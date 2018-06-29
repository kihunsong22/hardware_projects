--2003-12-10

library IEEE;
use IEEE.std_logic_1164.all;
use ieee.std_logic_unsigned.all;


ENTITY owm_machine IS
   PORT (
      cmd_reg                 : IN std_logic_vector(7 DOWNTO 0);   
      DQ_IN                   : IN std_logic;   
      read_owm_status         : IN std_logic;   
      read_owm_data           : IN std_logic;   
      write_owm_data          : IN std_logic;   
      owm_interrupt           : OUT std_logic;   
      interrupt_enbl          : IN std_logic_vector(7 DOWNTO 0);   
      DQ_CONTROL              : OUT std_logic;   
      reset                   : IN std_logic;   
      clk_1us                 : IN std_logic;   
      owm_rcvr_buffer         : OUT std_logic_vector(7 DOWNTO 0);
      owr                     : IN std_logic;   
      xmit_buffer             : IN std_logic_vector(7 DOWNTO 0);   
      owm_status              : OUT std_logic_vector(7 DOWNTO 0);   
      sr_a                    : OUT std_logic);   
END ENTITY owm_machine;

ARCHITECTURE translated OF owm_machine IS


   -- Setting up my states 

   CONSTANT  OWM_OD_RESET_TC       :  std_logic_vector(11 DOWNTO 0) := "000001110000";    --  112us
   CONSTANT  OWM_RESET_TC          :  std_logic_vector(11 DOWNTO 0) := "001111001101";    --  973us
   CONSTANT  OWM_OD_RESET_WAIT     :  std_logic_vector(11 DOWNTO 0) := "000000111100";    --  60us
   CONSTANT  OWM_RESET_WAIT        :  std_logic_vector(11 DOWNTO 0) := "000111100111";    --  487us
   CONSTANT  OWM_OD_RESET_SAMPLE   :  std_logic_vector(11 DOWNTO 0) := "000001000000";    -- 64us
   CONSTANT  OWM_RESET_SAMPLE      :  std_logic_vector(11 DOWNTO 0) := "001000100001";    --  545us
   CONSTANT  OWM_RESET_SM_TC       :  std_logic_vector(11 DOWNTO 0) := "000000011100";    --  28us
   CONSTANT  OWM_OD_RESET_SM_TC    :  std_logic_vector(11 DOWNTO 0) := "000000000010";    --  2us
   CONSTANT  T_WRITE_ZERO_TIME     :  std_logic_vector(8 DOWNTO 0) := "000111110";    --  62us
   CONSTANT  T_WRITE_ONE_TIME      :  std_logic_vector(8 DOWNTO 0) := "000111110";    --  62us
   CONSTANT  T_OD_MODE_WRITE_ONE_ZERO_SAMPLE_TIME: std_logic_vector(8 DOWNTO 0) := "000000011";    --  3us
   CONSTANT  T_OD_MODE_WRITE_ONE_ZERO_TIME  :  std_logic_vector(8 DOWNTO 0) := "000000111";    --  7us
   CONSTANT  Idle                  :  std_logic_vector(3 DOWNTO 0) := "0000";    --  Idle
   CONSTANT  CheckOWR              :  std_logic_vector(3 DOWNTO 0) := "0001";    --  Check for shorted OW
   CONSTANT  Reset_Low             :  std_logic_vector(3 DOWNTO 0) := "0010";    --  Start reset
   CONSTANT  PD_Wait               :  std_logic_vector(3 DOWNTO 0) := "0011";    --  release line for 1T
   CONSTANT  PD_Sample             :  std_logic_vector(3 DOWNTO 0) := "0100";    --  sample line after slowest 1T over
   CONSTANT  Reset_High            :  std_logic_vector(3 DOWNTO 0) := "0101";    --  recover OW line level
   CONSTANT  PD_Force              :  std_logic_vector(3 DOWNTO 0) := "0110";    --  mask the presence pulse  
   CONSTANT  PD_Release            :  std_logic_vector(3 DOWNTO 0) := "0111";    --  recover OW line level
   CONSTANT  SI_Idle               :  std_logic_vector(3 DOWNTO 0) := "1000";    --  slave interrupt idle
   CONSTANT  SI_Wait               :  std_logic_vector(3 DOWNTO 0) := "1011";    --  Slave Waitfor
   CONSTANT  SI_PD_Sample          :  std_logic_vector(3 DOWNTO 0) := "1100";    --  Slave PD sample
   CONSTANT  SI_Reset_High         :  std_logic_vector(3 DOWNTO 0) := "1101"; 
   CONSTANT  IdleS                 :  std_logic_vector(3 DOWNTO 0) := "0000";    --  Idle state
   CONSTANT  Load                  :  std_logic_vector(3 DOWNTO 0) := "0001";    --  Load byte
   CONSTANT  CheckOW               :  std_logic_vector(3 DOWNTO 0) := "1111";    --  Check for shorted line
   CONSTANT  DQLOW                 :  std_logic_vector(3 DOWNTO 0) := "0010";    --  Start of timeslot
   CONSTANT  WriteZero             :  std_logic_vector(3 DOWNTO 0) := "0011";    --  Write a zero to the 1-wire
   CONSTANT  WriteOne              :  std_logic_vector(3 DOWNTO 0) := "0100";    --  Write a one to the 1-wire
   CONSTANT  ReadBit               :  std_logic_vector(3 DOWNTO 0) := "0101";    --  Search Rom Accelerator read bit
   CONSTANT  FirstPassSR           :  std_logic_vector(3 DOWNTO 0) := "0110";    --  Used by SRA
   CONSTANT  WriteBitSR            :  std_logic_vector(3 DOWNTO 0) := "0111";    --  Decide what bit value to write in SRA
   CONSTANT  WriteBit              :  std_logic_vector(3 DOWNTO 0) := "1000";    --  Writes the chosen bit in SRA
   CONSTANT  WaitTS                :  std_logic_vector(3 DOWNTO 0) := "1001";    --  Wait for end of time slot
   CONSTANT  IndexInc              :  std_logic_vector(3 DOWNTO 0) := "1010";    --  Increments bit index
   CONSTANT  UpdateBuff            :  std_logic_vector(3 DOWNTO 0) := "1011";    --  Updates states of rbf
   CONSTANT  ODWriteZero           :  std_logic_vector(3 DOWNTO 0) := "1100";    --  Write a zero @ OD speed to OW
   CONSTANT  ODWriteOne            :  std_logic_vector(3 DOWNTO 0) := "1101";    --  Write a one @ OD speed to OW
   CONSTANT  ClrLowDone            :  std_logic_vector(3 DOWNTO 0) := "1110";    --  disable stpupz before pulldown
   SIGNAL dqz                      :  std_logic;   
   SIGNAL dqoe                     :  std_logic;   
   -- interrupt register

   SIGNAL pd                       :  std_logic;   --  presence detect done flag
   SIGNAL pdr                      :  std_logic;   --  presence detect result
   SIGNAL tbe                      :  std_logic;   --  transmit buffer empty flag
   SIGNAL temt                     :  std_logic;   --  transmit shift register empty flag
   SIGNAL temt_ext                 :  std_logic;   --  temt extended flag
   SIGNAL rbf                      :  std_logic;   --  receive buffer full flag
   SIGNAL rsrf                     :  std_logic;   --  receive shift register full flag
   -- interrupt enable register

   SIGNAL epd                      :  std_logic;   --  enable presence detect interrupt
   SIGNAL etbe                     :  std_logic;   --  enable transmit buffer empty interrupt
   SIGNAL etmt                     :  std_logic;   --  enable transmit shift register empty int.
   SIGNAL erbf                     :  std_logic;   --  enable receive buffer full interrupt

   SIGNAL eslave_int               :  std_logic;   --  enable slave interrupt
   SIGNAL enbsy                    :  std_logic;   --  enable not busy  int.
   SIGNAL reset_owr                :  std_logic;   
   SIGNAL OneWireReset_eq_Idle     :  std_logic;   
   SIGNAL OneWireReset_eq_Reset_High      :  std_logic;   
   SIGNAL owm_state_eq_Load        :  std_logic;
   SIGNAL owm_state_eq_UpdateBuff  :  std_logic;

   SIGNAL clear_pd                 :  std_logic;   
   SIGNAL clear_interrupts         :  std_logic;   
   SIGNAL ias                      :  std_logic;   
   SIGNAL set_tbe                  :  std_logic;   
   SIGNAL clear_tbe                :  std_logic;   
   SIGNAL set_rbf                  :  std_logic;   
   SIGNAL clear_rbf                :  std_logic;   
   SIGNAL set_interrupts           :  std_logic;   
   SIGNAL activate_intr            :  std_logic;   
   SIGNAL xmit_shiftreg            :  std_logic_vector(7 DOWNTO 0);   --  transmit shift register
   SIGNAL rcvr_shiftreg            :  std_logic_vector(7 DOWNTO 0);   --  receive shift register
   SIGNAL last_rcvr_bit            :  std_logic;   --  active on index = 7 to begin shift to rbe
   SIGNAL byte_done                :  std_logic;   
   SIGNAL byte_done_flag           :  std_logic;   --  signals to stretch byte_done    
   SIGNAL bdext1                   :  std_logic;   --  signals to stretch byte_done    
   SIGNAL First                    :  std_logic;   --  for Search ROM accelerator
   SIGNAL BitRead1                 :  std_logic;   
   SIGNAL BitRead2                 :  std_logic;   
   SIGNAL BitWrite                 :  std_logic;   
   SIGNAL OneWireReset             :  std_logic_vector(3 DOWNTO 0);   
   SIGNAL owm_state                :  std_logic_vector(3 DOWNTO 0);
   SIGNAL owm_one_wire_reset_count :  std_logic_vector(11 DOWNTO 0);   
   SIGNAL smCnt                    :  std_logic_vector(6 DOWNTO 0);   
   SIGNAL owm_index                :  std_logic_vector(3 DOWNTO 0);   
   SIGNAL owm_TimeSlotCnt          :  std_logic_vector(8 DOWNTO 0);   
   SIGNAL pd_set                   :  std_logic;   
   SIGNAL owm_longline_mode        :  std_logic;   
   SIGNAL owm_od_mode              :  std_logic;   
   SIGNAL owr_reset                :  std_logic;   
   SIGNAL owr_mode                 :  std_logic;   
   SIGNAL owm_reset_goto_pdwait    :  std_logic;   
   SIGNAL owm_reset_goto_pdsample  :  std_logic;   
   SIGNAL owm_reset_goto_resethigh :  std_logic;   
   SIGNAL owm_reset_goto_idle      :  std_logic;   
   SIGNAL owm_state_write_zero     :  std_logic_vector(3 DOWNTO 0);
   SIGNAL owm_state_write_one      :  std_logic_vector(3 DOWNTO 0);
   SIGNAL owm_timeslot_bitread     :  std_logic;
   SIGNAL owm_timeslot_readbit_transition :  std_logic;   
   SIGNAL owm_timeslot_firstpass_transition:  std_logic;   
   SIGNAL owm_timeslot_waitts_transition  :  std_logic;   
   SIGNAL owm_timeslot_writezero_sample   :  std_logic;   
   SIGNAL owm_timeslot_writeone_sample    :  std_logic;
   SIGNAL owm_TimeSlot_dqlow_transition   :  std_logic;   
   SIGNAL not_busy                 :  std_logic;   
   SIGNAL dq_in_d                  :  std_logic;   
   SIGNAL slave_int                :  std_logic;   
   SIGNAL clear_slave_int          :  std_logic;   
   SIGNAL set_slave_int            :  std_logic;   
   --   wire SI_Wait_goto;

   SIGNAL SI_Wait_goto             :  std_logic;   
   SIGNAL owm_reset_goto_si_resethigh     :  std_logic;   
   SIGNAL owm_reset_goto_si_pdsample      :  std_logic;   
   --   wire owm_reset_goto_si_idle;

   SIGNAL owm_reset_goto_si_idle   :  std_logic;   
   CONSTANT  OWM_SI_OD_PD_WAIT     :  std_logic_vector(11 DOWNTO 0) := "000000000110"; 
      CONSTANT  OWM_SI_PD_WAIT        :  std_logic_vector(11 DOWNTO 0) := "000000101000"; 
      CONSTANT  OWM_SI_OD_RESET_HIGH  :  std_logic_vector(11 DOWNTO 0) := "000001100000"; 
      CONSTANT  OWM_SI_RESET_HIGH     :  std_logic_vector(11 DOWNTO 0) := "001111000000"; 
      CONSTANT  OWM_SI_OD_PD_SAMPLE   :  std_logic_vector(11 DOWNTO 0) := "000000001000"; 
      CONSTANT  OWM_SI_PD_SAMPLE      :  std_logic_vector(11 DOWNTO 0) := "000000101010"; 
      --  1 wire control

   SIGNAL clk                      :  std_logic;   
   -- These strings allow for easy debugging of the rtl.  They dissapear at synthesis time.


   SIGNAL owm_reset_string         :  std_logic_vector(8 * 20 DOWNTO 1);   
   -- case(OneWireReset)

   SIGNAL owm_state_string         :  std_logic_vector(8 * 20 DOWNTO 1);
   SIGNAL owm_status_tmp1         :  std_logic_vector(7 DOWNTO 0);   
   SIGNAL DQ_CONTROL_s         :  std_logic;   
   SIGNAL owm_rcvr_buffer_tmp3    :  std_logic_vector(7 DOWNTO 0);   
   SIGNAL owm_interrupt_tmp4      :  std_logic;   
   SIGNAL sr_a_tmp5               :  std_logic;   
--

--   FUNCTION conv_std_logic (
--      val      : IN boolean) RETURN std_logic IS
--      variable result: std_logic;
--   BEGIN
      --result := '0';
      --if val then
      --   result := '1';
      --end if;
 --     return result;
--   END function conv_std_logic;
  
 
--



BEGIN
   owm_status <= owm_status_tmp1;
   DQ_CONTROL <= DQ_CONTROL_s;
   owm_rcvr_buffer <= owm_rcvr_buffer_tmp3;
   owm_interrupt <= owm_interrupt_tmp4;
   sr_a <= sr_a_tmp5;
 
   PROCESS (owm_one_wire_reset_count, owm_od_mode)
      VARIABLE owm_reset_goto_si_idle_tmp6  : std_logic;
   BEGIN
      IF (owm_one_wire_reset_count = OWM_SI_OD_RESET_HIGH AND owm_od_mode = '1') 
      THEN
         owm_reset_goto_si_idle_tmp6 := '1';    
      ELSE
         IF (owm_one_wire_reset_count = OWM_SI_RESET_HIGH AND owm_od_mode = '0') 
         THEN
            owm_reset_goto_si_idle_tmp6 := '1';    
         ELSE
            owm_reset_goto_si_idle_tmp6 := '0';    
         END IF;
      END IF;
      owm_reset_goto_si_idle <= owm_reset_goto_si_idle_tmp6;
   END PROCESS;

   
   PROCESS (owm_one_wire_reset_count, owm_od_mode)
      VARIABLE owm_reset_goto_si_resethigh_tmp7  : std_logic;
   BEGIN
      IF (owm_one_wire_reset_count = OWM_SI_OD_PD_SAMPLE AND owm_od_mode = '1') 
      THEN
         owm_reset_goto_si_resethigh_tmp7 := '1';    
      ELSE
         IF (owm_one_wire_reset_count = OWM_SI_PD_SAMPLE AND owm_od_mode = '0') 
         THEN
            owm_reset_goto_si_resethigh_tmp7 := '1';    
         ELSE
            owm_reset_goto_si_resethigh_tmp7 := '0';    
         END IF;
      END IF;
      owm_reset_goto_si_resethigh <= owm_reset_goto_si_resethigh_tmp7;
   END PROCESS;

   
   PROCESS (owm_one_wire_reset_count, owm_od_mode)
      VARIABLE owm_reset_goto_si_pdsample_tmp8  : std_logic;
   BEGIN
      IF (owm_one_wire_reset_count = OWM_SI_OD_PD_WAIT AND owm_od_mode = '1') 
      THEN
         owm_reset_goto_si_pdsample_tmp8 := '0';    
      ELSE
         IF (owm_one_wire_reset_count = OWM_SI_PD_WAIT AND owm_od_mode = '0') 
         THEN
            owm_reset_goto_si_pdsample_tmp8 := '0';    
         ELSE
            owm_reset_goto_si_pdsample_tmp8 := '0';    
         END IF;
      END IF;
      owm_reset_goto_si_pdsample <= owm_reset_goto_si_pdsample_tmp8;
   END PROCESS;

   
   PROCESS (dq_in_d, slave_int, owm_state, pd)
      VARIABLE SI_Wait_goto_tmp9  : std_logic;
   BEGIN
      IF (((dq_in_d = '0' AND slave_int = '0') AND owm_state = IdleS) AND
      pd = '0') THEN
         SI_Wait_goto_tmp9 := '1';    
      ELSE
         SI_Wait_goto_tmp9 := '0';    
      END IF;
      SI_Wait_goto <= SI_Wait_goto_tmp9;
   END PROCESS;
   set_slave_int <= '1' when((owm_reset_goto_si_idle = '1' AND dq_in_d = '0') 
   AND owm_state = IdleS) else '0';
   clear_slave_int <= read_owm_status  ;
   dqz <= cmd_reg(2) ;
   owm_od_mode <= cmd_reg(7) ;
   owm_longline_mode <= cmd_reg(6) ;
   sr_a_tmp5 <= cmd_reg(1) ;
   owr_reset <= cmd_reg(5) ;
   owr_mode <= cmd_reg(4) ;
   -- mask register assignments
   (dqoe, enbsy, eslave_int, erbf, etmt, etbe, ias, epd) <= interrupt_enbl ;
   -- status register assignments
   owm_status_tmp1 <= dq_in_d & not_busy & slave_int & rbf & temt & tbe & 
   pdr & pd ;
   not_busy <= '1' when(NOT (owm_state /= IdleS OR OneWireReset /=
   Idle)) else '0' ;
   -- state control assignments  These bits are used in if then else flow control state logic
   clear_pd <= '1' when(read_owm_status = '1')else '0' ;
   clear_interrupts <= read_owm_data OR read_owm_status ;
   OneWireReset_eq_Idle <= '1' when(OneWireReset = Idle)else '0' ;
   OneWireReset_eq_Reset_High <= '1' when(OneWireReset = Reset_High)else '0' ;
   owm_state_eq_Load <= '1' when(owm_state = Load)else '0' ;
   owm_state_eq_UpdateBuff <= '1' when(owm_state = UpdateBuff)else '0' ;
   set_interrupts <= (pd AND epd) OR ((tbe AND etbe) AND NOT temt) OR (temt_ext 
   AND etmt) OR (rbf AND erbf) OR (slave_int AND eslave_int) OR (enbsy AND not_busy) ;
   set_tbe <= owm_state_eq_Load ;
   clear_tbe <= '1' when(write_owm_data = '1' AND tbe = '1')else '0' ;
   set_rbf <= '1' when((rsrf = '1' AND owm_state_eq_UpdateBuff = '1')
   AND rbf = '0')else '0' ;
   clear_rbf <=  '1' when ((read_owm_data='1') AND (rbf = '1'))else '0' ;
   owm_interrupt_tmp4 <= activate_intr XOR NOT ias ;
   reset_owr <= '1' when  (OneWireReset = Reset_High)else '0' ;
   owm_reset_goto_pdwait <= '1' when  ((owm_one_wire_reset_count = OWM_OD_RESET_WAIT)and (owm_od_mode = '1')) ELSE
      '1' when  ((owm_one_wire_reset_count = OWM_RESET_WAIT)and(owm_od_mode = '0')) else '0' ;
      
   owm_reset_goto_pdsample <=  '1' when ((owm_one_wire_reset_count = OWM_OD_RESET_SAMPLE)and (owm_od_mode = '1')) ELSE
        '1' when ((owm_one_wire_reset_count = OWM_RESET_SAMPLE)and (owm_od_mode = '0')) else '0' ;
   
   owm_reset_goto_resethigh <=  '1' when ((smCnt = OWM_OD_RESET_SM_TC)and (owm_od_mode = '1')) ELSE
       '1' when ((smCnt = OWM_RESET_SM_TC)and(owm_od_mode = '0'))else '0' ;
       
   owm_reset_goto_idle <= '1' when ((owm_one_wire_reset_count = OWM_OD_RESET_TC )
                               OR (clear_pd = '1')) and (owm_od_mode = '1') ELSE 
   '1' when((owm_one_wire_reset_count =  OWM_RESET_TC) OR (clear_pd = '1'))and (owm_od_mode = '0') else '0' ;
   owm_state_write_zero <= ODWriteZero WHEN owm_od_mode = '1' ELSE WriteZero ;
   owm_state_write_one <= ODWriteOne WHEN owm_od_mode = '1' ELSE WriteOne ;
   --1us
   --8uS
   --5us
   owm_TimeSlot_dqlow_transition <= '1' when((owm_TimeSlotCnt = "000000000") 
   and (owm_od_mode = '1')) ELSE 
   '1' when((owm_TimeSlotCnt = "000001000")and (owm_longline_mode = '1')) ELSE 
   '1' when (owm_TimeSlotCnt = "000000101") else '0';
   --2us
   --22uS
   --15uS
   owm_timeslot_bitread <= '1' when((owm_TimeSlotCnt = "000000011")and (owm_od_mode = '1')) ELSE
   '1' when((owm_TimeSlotCnt = "000010110")and (owm_longline_mode = '1')) ELSE 
   '1' when(owm_TimeSlotCnt = "000001111") else '0';
   --6us
   --61uS
   owm_timeslot_readbit_transition <= '1' when((owm_TimeSlotCnt = "000000110") and (owm_od_mode = '1')) ELSE 
   '1' when(owm_TimeSlotCnt = "000111101") else '0';
   --11uS
   --75uS
   --71uS
   owm_timeslot_firstpass_transition <= '1' when(owm_TimeSlotCnt = "000001011")and(owm_od_mode = '1') ELSE 
   '1' when(owm_TimeSlotCnt = "001001011")and(owm_longline_mode = '1') ELSE 
   '1' when(owm_TimeSlotCnt = "001000111")else '0' ;
   --4us
   --75uS
   --71uS
   owm_timeslot_waitts_transition <= '1' when(owm_TimeSlotCnt = "000001001")and(owm_od_mode = '1') ELSE 
   '1' when(owm_TimeSlotCnt = "001001011")and(owm_longline_mode = '1') ELSE 
   '1' when(owm_TimeSlotCnt = "001000111")else '0' ;
   --22uS
   --15uS
   owm_timeslot_writezero_sample <= '1' when(owm_TimeSlotCnt = "000010110") 
   AND (sr_a_tmp5 = '0') and (owm_longline_mode = '1') ELSE 
   '1' when(owm_TimeSlotCnt = "000001111") AND (sr_a_tmp5 = '0')else '0' ;
   
   owm_timeslot_writeone_sample <= owm_timeslot_writezero_sample ;
   clk <= clk_1us ;

   PROCESS (clk, reset)
   BEGIN
      IF (reset = '1') THEN
         DQ_CONTROL_s <= '1';    
      ELSIF (clk'EVENT AND clk = '1') THEN
         IF (owr_reset = '1') THEN
            DQ_CONTROL_s <= '1';    
         ELSE
            IF dqoe = '1' THEN
               DQ_CONTROL_s <= NOT dqz;
            ELSE
               IF OneWireReset = Reset_Low THEN
                  DQ_CONTROL_s <= '0';
               ELSE
                  IF OneWireReset = PD_Wait THEN
                     DQ_CONTROL_s <= '1';
                  ELSE
                     IF OneWireReset = PD_Force THEN
                        DQ_CONTROL_s <= '0';
                     ELSE
                        IF owm_state = DQLOW THEN
                           DQ_CONTROL_s <= '0';
                        ELSE
                           IF owm_state = WriteZero THEN
                              DQ_CONTROL_s <= '0';
                           ELSE
                              IF owm_state = ODWriteZero THEN
                                 DQ_CONTROL_s <= '0';
                              ELSE
                                 IF owm_state = WriteBit THEN
                                    DQ_CONTROL_s <= '0';
                                 ELSE
                                    DQ_CONTROL_s <= '1';
                                 END IF;
                              END IF;
                           END IF;
                        END IF;
                     END IF;
                  END IF;
               END IF;
            END IF;    
         END IF;
      END IF;
   END PROCESS;

   PROCESS (clk, reset)
   BEGIN
      IF (reset = '1') THEN
         dq_in_d <= '0';    
      ELSIF (clk'EVENT AND clk = '1') THEN
         dq_in_d <= DQ_IN;    
      END IF;
   END PROCESS;

   PROCESS (clk, reset)
   BEGIN
      IF (reset = '1') THEN
         pd <= '0';    
         pd_set <= '0';    
      ELSIF (clk'EVENT AND clk = '1') THEN
         IF (owr_reset = '1') THEN
            pd <= '0';    
            pd_set <= '0';    
         ELSE
            IF (OneWireReset_eq_Reset_High = '1' AND pd = '0') THEN
               pd_set <= '1';    
            ELSE
               IF ((clear_pd = '1' AND pd = '1') AND pd_set = '1') THEN
                  pd_set <= '0';    
                  pd <= '0';    
               END IF;
            END IF;
            IF ((dq_in_d = '1' AND pd_set = '1') AND pd = '0') THEN
               pd <= '1';    
            END IF;
         END IF;
      END IF;
   END PROCESS;

   PROCESS (clk, reset)
   BEGIN
      IF (reset = '1') THEN
         slave_int <= '0';    
      ELSIF (clk'EVENT AND clk = '1') THEN
         IF (owr_reset = '1') THEN
            slave_int <= '0';    
         ELSE
            IF (clear_slave_int = '1') THEN
               slave_int <= '0';    
            ELSE
               IF (set_slave_int = '1') THEN
                  slave_int <= '1';    
               END IF;
            END IF;
         END IF;
      END IF;
   END PROCESS;

   PROCESS (clk, reset)
   BEGIN
      IF (reset = '1') THEN
         tbe <= '1';    
      ELSIF (clk'EVENT AND clk = '1') THEN
         IF (owr_reset = '1') THEN
            tbe <= '1';    
         ELSE
            IF (clear_tbe = '1') THEN
               tbe <= '0';    
            ELSE
               IF (set_tbe = '1') THEN
                  tbe <= '1';    
               END IF;
            END IF;
         END IF;
      END IF;
   END PROCESS;

   PROCESS (clk, reset)
   BEGIN
      IF (reset = '1') THEN
         rbf <= '0';    
      ELSIF (clk'EVENT AND clk = '1') THEN
         IF (owr_reset = '1') THEN
            rbf <= '0';    
         ELSE
            IF (clear_rbf = '1') THEN
               rbf <= '0';    
            ELSE
               IF (set_rbf = '1') THEN
                  rbf <= '1';    
               END IF;
            END IF;
         END IF;
      END IF;
   END PROCESS;

   PROCESS (clk, reset)
   BEGIN
      IF (reset = '1') THEN
         owm_rcvr_buffer_tmp3 <= "00000000";    
      ELSIF (clk'EVENT AND clk = '1') THEN
         IF (owr_reset = '1') THEN
            owm_rcvr_buffer_tmp3 <= "00000000";    
         ELSE
            IF ((rbf AND rsrf) = '1') THEN
               owm_rcvr_buffer_tmp3 <= rcvr_shiftreg;    
            END IF;
         END IF;
      END IF;
   END PROCESS;

   PROCESS (clk, reset)
   BEGIN
      IF (reset = '1') THEN
         rsrf <= '0';    
      ELSIF (clk'EVENT AND clk = '1') THEN
         IF (owr_reset = '1') THEN
            rsrf <= '0';    
         ELSE
            IF (last_rcvr_bit = '1') THEN
               IF (owm_state = IndexInc) THEN
                  rsrf <= '1';    
               ELSE
                  IF (rbf = '1') THEN
                     rsrf <= '0';    
                  END IF;
               END IF;
            END IF;
         END IF;
      END IF;
   END PROCESS;

   PROCESS (clk, reset)
   BEGIN
      IF (reset = '1') THEN
         activate_intr <= '0';    
      ELSIF (clk'EVENT AND clk = '1') THEN
         IF (owr_reset = '1') THEN
            activate_intr <= '0';    
         ELSE
            IF (set_interrupts = '1') THEN
               activate_intr <= '1';    
            ELSE
               activate_intr <= '0';    
            END IF;
         END IF;
      END IF;
   END PROCESS;

   ----------------------------------------------------------------------------
   
   --
   
   --  OneWireReset
   
   --
   
   --  this state machine performs the 1-wire reset and presence detect
   
   --  - Added OD for overdrive speed presence detect
   
   --  - Added PD_LOW bit for strong pullup control
   
   --
   
   --  Idle       : OW high - waiting to issue a PD
   
   --  CheckOWR   : OW high - checks for shorted OW line
   
   --  Reset_Low  : OW low - held down for GT8 OW osc periods
   
   --  PD_Wait    : OW high - released and waits for 1T 
   
   --  PD_Sample  : OW high - checks to see if a slave is out there pulling 
   
   --                         OW low for 4T
   
   --  Reset_High : OW high - slave, if any, release OW and host lets it recover
   
   ----------------------------------------------------------------------------
   
   PROCESS (clk, reset)
   BEGIN
      IF (reset = '1') THEN
         pdr <= '1';    
         OneWireReset <= Idle;    
         smCnt <= "0000000";    --  added to init simulations
         owm_one_wire_reset_count <= "000000000000";    
      ELSIF (clk'EVENT AND clk = '1') THEN
         IF (owr_reset = '1') THEN
            pdr <= '1';    
            OneWireReset <= Idle;    
            smCnt <= "0000000";    --  added to init simulations
            owm_one_wire_reset_count <= "000000000000";    
         ELSE
            CASE OneWireReset IS
               WHEN Idle =>
                        owm_one_wire_reset_count <= "000000000000";    
                        smCnt <= "0000000";    
                        IF (owr = '1') THEN
                           OneWireReset <= CheckOWR;    
                        ELSE
                           IF (SI_Wait_goto = '1') THEN
                              OneWireReset <= SI_Wait;    
                           END IF;
                        END IF;
               WHEN SI_Wait =>
                        IF (owr = '1') THEN
                           OneWireReset <= CheckOWR;    
                           owm_one_wire_reset_count <= "000000000000";    
                        ELSE
                           IF (dq_in_d = '1') THEN
                              OneWireReset <= Idle;    
                           ELSE
                              owm_one_wire_reset_count <= owm_one_wire_reset_count + "000000000001"; 
                                                            IF ((NOT dq_in_d AND DQ_CONTROL_s) = '1') 
                              THEN
                                 OneWireReset <= SI_PD_Sample;    
                                 smCnt <= "0000000";    
                              ELSE
                                 IF (owm_reset_goto_si_pdsample = '1') THEN
                                    OneWireReset <= SI_PD_Sample;    
                                    smCnt <= "0000000";    
                                 END IF;
                              END IF;
                           END IF;
                        END IF;
                        -- else: !if(owr)
                        
                        
               -- case: SI_Wait
               
               WHEN SI_PD_Sample =>
                        IF (owr = '1') THEN
                           OneWireReset <= CheckOWR;    
                           owm_one_wire_reset_count <= "000000000000";    
                        ELSE
                           IF (dq_in_d = '1') THEN
                              OneWireReset <= Idle;    
                           ELSE
                              owm_one_wire_reset_count <= owm_one_wire_reset_count + "000000000001"; 
                                                            smCnt <= smCnt + "0000001";    
                              IF (owm_reset_goto_si_resethigh = '1') THEN
                                 pdr <= dq_in_d;    
                                 OneWireReset <= SI_Reset_High;    
                              END IF;
                           END IF;
                        END IF;
                        -- else: !if(owr)
                        
                        
               -- case: SI_PD_Sample
               
               WHEN SI_Reset_High =>
                        IF (owr = '1') THEN
                           OneWireReset <= CheckOWR;    
                           owm_one_wire_reset_count <= "000000000000";    
                        ELSE
                           IF (dq_in_d = '1') THEN
                              OneWireReset <= Idle;    
                           ELSE
                              owm_one_wire_reset_count <= owm_one_wire_reset_count + "000000000001"; 
                                                            IF (owm_reset_goto_si_idle = '1') THEN
                                 OneWireReset <= Idle;    
                              END IF;
                           END IF;
                        END IF;
                        -- else: !if(owr)
                        
                        
               -- case: SI_Reset_High
               
               WHEN CheckOWR =>
                        OneWireReset <= Reset_Low;    
               WHEN Reset_Low =>
                        owm_one_wire_reset_count <= owm_one_wire_reset_count + "000000000001"; 
                                                IF (owm_reset_goto_pdwait = '1') THEN
                           OneWireReset <= PD_Wait;    
                        END IF;
               WHEN PD_Wait =>
                        owm_one_wire_reset_count <= owm_one_wire_reset_count + "000000000001"; 
                                                IF ((NOT dq_in_d AND DQ_CONTROL_s) = '1') THEN
                           OneWireReset <= PD_Sample;    
                           smCnt <= "0000000";    
                        ELSE
                           IF (owm_reset_goto_pdsample = '1') THEN
                              OneWireReset <= PD_Sample;    
                              smCnt <= "0000000";    
                           END IF;
                        END IF;
               WHEN PD_Sample =>
                        owm_one_wire_reset_count <= owm_one_wire_reset_count + "000000000001"; 
                                                smCnt <= smCnt + "0000001";    
                        IF (owm_reset_goto_resethigh = '1') THEN
                           pdr <= dq_in_d;    
                           OneWireReset <= Reset_High;    
                        END IF;
               WHEN Reset_High =>
                        owm_one_wire_reset_count <= owm_one_wire_reset_count + "000000000001"; 
                                                IF (owm_reset_goto_idle = '1') THEN
                           OneWireReset <= Idle;    
                        END IF;
               WHEN OTHERS  =>
                        OneWireReset <= Idle;    
               
            END CASE;
         END IF;
      END IF;
   END PROCESS;

   ----------------------------------------------------------------------------
   
   --  owm_state
   
   --  this state machine performs the 1-wire writing and reading
   
   ----------------------------------------------------------------------------
   
   -- The following 2 registers are to stretch the temt signal to catch the
   
   -- temt interrupt source - SDS
   
   PROCESS (clk, reset)
   BEGIN
      IF (reset = '1') THEN
         bdext1 <= '0';    
      ELSIF (clk'EVENT AND clk = '1') THEN
         IF (owr_reset = '1') THEN
            bdext1 <= '0';    
         ELSE
            bdext1 <= byte_done;    
         END IF;
      END IF;
   END PROCESS;

   PROCESS (clk, reset)
   BEGIN
      IF (reset = '1') THEN
         byte_done_flag <= '0';    
      ELSIF (clk'EVENT AND clk = '1') THEN
         byte_done_flag <= bdext1;    
      END IF;
   END PROCESS;
   temt_ext <= temt AND byte_done_flag ;

   -- The index variable has been decoded explicitly in this state machine
   
   -- so that the code would compile on the Cypress warp compiler - SDS
   
   PROCESS (clk, reset)
      VARIABLE temp_tmp10  : std_logic_vector(1 DOWNTO 0);
   BEGIN
      IF (reset = '1') THEN
         owm_index <= "0000";    
         owm_TimeSlotCnt <= "000000000";    
         temt <= '1';    
         last_rcvr_bit <= '0';    
         owm_state <= IdleS;
         BitRead1 <= '0';    
         BitRead2 <= '0';    
         BitWrite <= '0';    
         First <= '0';    
         byte_done <= '0';    
         xmit_shiftreg <= "00000000";    
         rcvr_shiftreg <= "00000000";    
      -- if (reset)
      
      ELSIF (clk'EVENT AND clk = '1') THEN
         IF (owr_reset = '1') THEN
            owm_index <= "0000";    
            owm_TimeSlotCnt <= "000000000";    
            temt <= '1';    
            last_rcvr_bit <= '0';    
            owm_state <= IdleS;
            BitRead1 <= '0';    
            BitRead2 <= '0';    
            BitWrite <= '0';    
            First <= '0';    
            byte_done <= '0';    
            xmit_shiftreg <= "00000000";    
            rcvr_shiftreg <= "00000000";    
         -- if (owr_reset)
         
         ELSE
            CASE owm_state IS
               -- IdleS state clears variables and waits for something to be
               
               -- deposited in the transmit buffer. When something is there,
               
               -- the next state is Load.
               
               WHEN IdleS =>
                        byte_done <= '0';    
                        owm_index <= "0000";    
                        last_rcvr_bit <= '0';    
                        First <= '0';    
                        owm_TimeSlotCnt <= "000000000";    
                        temt <= '1';    
                        IF (NOT tbe = '1') THEN
                           owm_state <= Load;
                        END IF;
               -- Load transfers the transmit buffer to the transmit shift register,
               
               -- then clears the transmit shift register empty interrupt. The next
               
               -- state is then DQLOW.
               
               WHEN Load =>
                        xmit_shiftreg <= xmit_buffer;    
                        temt <= '0';    
                        owm_state <= DQLOW;
               -- DQLOW pulls the DQ line low for 1us, beginning a timeslot.
               
               -- If sr_a is 0, it is a normal write/read operation. If sr_a
               
               -- is a 1, then you must go into Search ROM accelerator mode.
               
               WHEN DQLOW =>
                        owm_TimeSlotCnt <= owm_TimeSlotCnt + "000000001";    
                        IF (owm_TimeSlot_dqlow_transition = '1') THEN
                           IF (NOT sr_a_tmp5 = '1') THEN
                              -- Normal write
                              
                              CASE owm_index IS
                                 WHEN "0000" =>
                                          IF (NOT xmit_shiftreg(0) = '1') THEN
                                             owm_state <= owm_state_write_zero;
                                                                                       ELSE
                                             owm_state <= owm_state_write_one;
                                                                                       END IF;
                                 WHEN "0001" =>
                                          IF (NOT xmit_shiftreg(1) = '1') THEN
                                             owm_state <= owm_state_write_zero;
                                                                                       ELSE
                                             owm_state <= owm_state_write_one;
                                                                                       END IF;
                                 WHEN "0010" =>
                                          IF (NOT xmit_shiftreg(2) = '1') THEN
                                             owm_state <= owm_state_write_zero;
                                                                                       ELSE
                                             owm_state <= owm_state_write_one;
                                                                                       END IF;
                                 WHEN "0011" =>
                                          IF (NOT xmit_shiftreg(3) = '1') THEN
                                             owm_state <= owm_state_write_zero;
                                                                                       ELSE
                                             owm_state <= owm_state_write_one;
                                                                                       END IF;
                                 WHEN "0100" =>
                                          IF (NOT xmit_shiftreg(4) = '1') THEN
                                             owm_state <= owm_state_write_zero;
                                                                                       ELSE
                                             owm_state <= owm_state_write_one;
                                                                                       END IF;
                                 WHEN "0101" =>
                                          IF (NOT xmit_shiftreg(5) = '1') THEN
                                             owm_state <= owm_state_write_zero;
                                                                                       ELSE
                                             owm_state <= owm_state_write_one;
                                                                                       END IF;
                                 WHEN "0110" =>
                                          IF (NOT xmit_shiftreg(6) = '1') THEN
                                             owm_state <= owm_state_write_zero;
                                                                                       ELSE
                                             owm_state <= owm_state_write_one;
                                                                                       END IF;
                                 WHEN "0111" =>
                                          IF (NOT xmit_shiftreg(7) = '1') THEN
                                             owm_state <= owm_state_write_zero;
                                                                                       ELSE
                                             owm_state <= owm_state_write_one;
                                                                                       END IF;
                                 WHEN OTHERS =>
                                          NULL;
                                 
                              END CASE;
                           -- case(owm_index)
                           
                           ELSE
                              -- Search Rom Accelerator mode
                              
                              owm_state <= ReadBit;
                           END IF;
                        END IF;
               -- WriteZero and WriteOne are identical, except for what they do to
               
               -- DQ (assigned in concurrent assignments). They both read DQ after
               
               -- 11us, then move on to wait for the end of the timeslot.
               
               WHEN WriteZero =>
                        owm_TimeSlotCnt <= owm_TimeSlotCnt + "000000001";    
                        IF (owm_timeslot_writezero_sample = '1') THEN
                           CASE owm_index IS
                              WHEN "0000" =>
                                       rcvr_shiftreg(0) <= dq_in_d;    
                              WHEN "0001" =>
                                       rcvr_shiftreg(1) <= dq_in_d;    
                              WHEN "0010" =>
                                       rcvr_shiftreg(2) <= dq_in_d;    
                              WHEN "0011" =>
                                       rcvr_shiftreg(3) <= dq_in_d;    
                              WHEN "0100" =>
                                       rcvr_shiftreg(4) <= dq_in_d;    
                              WHEN "0101" =>
                                       rcvr_shiftreg(5) <= dq_in_d;    
                              WHEN "0110" =>
                                       rcvr_shiftreg(6) <= dq_in_d;    
                              WHEN "0111" =>
                                       rcvr_shiftreg(7) <= dq_in_d;    
                              WHEN OTHERS =>
                                       NULL;
                              
                           END CASE;
                        END IF;
                        IF (owm_TimeSlotCnt = (T_WRITE_ZERO_TIME)) THEN
                           owm_state <= WaitTS;
                        END IF;
               WHEN WriteOne =>
                        owm_TimeSlotCnt <= owm_TimeSlotCnt + "000000001";    
                        IF (owm_timeslot_writeone_sample = '1') THEN
                           CASE owm_index IS
                              WHEN "0000" =>
                                       rcvr_shiftreg(0) <= dq_in_d;    
                              WHEN "0001" =>
                                       rcvr_shiftreg(1) <= dq_in_d;    
                              WHEN "0010" =>
                                       rcvr_shiftreg(2) <= dq_in_d;    
                              WHEN "0011" =>
                                       rcvr_shiftreg(3) <= dq_in_d;    
                              WHEN "0100" =>
                                       rcvr_shiftreg(4) <= dq_in_d;    
                              WHEN "0101" =>
                                       rcvr_shiftreg(5) <= dq_in_d;    
                              WHEN "0110" =>
                                       rcvr_shiftreg(6) <= dq_in_d;    
                              WHEN "0111" =>
                                       rcvr_shiftreg(7) <= dq_in_d;    
                              WHEN OTHERS =>
                                       NULL;
                              
                           END CASE;
                        END IF;
                        IF (owm_TimeSlotCnt = (T_WRITE_ONE_TIME)) THEN
                           owm_state <= WaitTS;
                        END IF;
               -- case: WriteOne
               
               -- ADDED ODWRITE states here GAG
               
               -- ODWriteZero and ODWriteOne are identical, except for what they 
               
               -- do to DQ (assigned in concurrent assignments). They both read 
               
               -- DQ after 3us, then move on to wait for the end of the timeslot.
               
               WHEN ODWriteZero =>
                        owm_TimeSlotCnt <= owm_TimeSlotCnt + "000000001";    
                        IF ((owm_TimeSlotCnt = T_OD_MODE_WRITE_ONE_ZERO_SAMPLE_TIME) 
                        AND (sr_a_tmp5 = '0')) THEN
                           CASE owm_index IS
                              WHEN "0000" =>
                                       rcvr_shiftreg(0) <= dq_in_d;    
                              WHEN "0001" =>
                                       rcvr_shiftreg(1) <= dq_in_d;    
                              WHEN "0010" =>
                                       rcvr_shiftreg(2) <= dq_in_d;    
                              WHEN "0011" =>
                                       rcvr_shiftreg(3) <= dq_in_d;    
                              WHEN "0100" =>
                                       rcvr_shiftreg(4) <= dq_in_d;    
                              WHEN "0101" =>
                                       rcvr_shiftreg(5) <= dq_in_d;    
                              WHEN "0110" =>
                                       rcvr_shiftreg(6) <= dq_in_d;    
                              WHEN "0111" =>
                                       rcvr_shiftreg(7) <= dq_in_d;    
                              WHEN OTHERS =>
                                       NULL;
                              
                           END CASE;
                        END IF;
                        -- case(owm_index)
                        
                        IF (owm_TimeSlotCnt = T_OD_MODE_WRITE_ONE_ZERO_TIME) 
                        THEN
                           owm_state <= WaitTS;    
                        END IF;
               WHEN ODWriteOne =>
                        owm_TimeSlotCnt <= owm_TimeSlotCnt + "000000001";    
                        IF ((owm_TimeSlotCnt = T_OD_MODE_WRITE_ONE_ZERO_SAMPLE_TIME) 
                        AND (sr_a_tmp5 = '0')) THEN
                           CASE owm_index IS
                              WHEN "0000" =>
                                       rcvr_shiftreg(0) <= dq_in_d;    
                              WHEN "0001" =>
                                       rcvr_shiftreg(1) <= dq_in_d;    
                              WHEN "0010" =>
                                       rcvr_shiftreg(2) <= dq_in_d;    
                              WHEN "0011" =>
                                       rcvr_shiftreg(3) <= dq_in_d;    
                              WHEN "0100" =>
                                       rcvr_shiftreg(4) <= dq_in_d;    
                              WHEN "0101" =>
                                       rcvr_shiftreg(5) <= dq_in_d;    
                              WHEN "0110" =>
                                       rcvr_shiftreg(6) <= dq_in_d;    
                              WHEN "0111" =>
                                       rcvr_shiftreg(7) <= dq_in_d;    
                              WHEN OTHERS =>
                                       NULL;
                              
                           END CASE;
                        END IF;
                        IF (owm_TimeSlotCnt = T_OD_MODE_WRITE_ONE_ZERO_TIME) 
                        THEN
                           owm_state <= WaitTS;    
                        END IF;
               -- case: ODWriteOne
               
               -- ReadBit used by the SRA to do the required bit reads
               
               WHEN ReadBit =>
                        owm_TimeSlotCnt <= owm_TimeSlotCnt + "000000001";    
                        IF (owm_timeslot_bitread = '1') THEN
                           IF (NOT First = '1') THEN
                              BitRead1 <= dq_in_d;    
                           ELSE
                              BitRead2 <= dq_in_d;    
                           END IF;
                        END IF;
                        IF (owm_timeslot_readbit_transition = '1') THEN
                           owm_state <= FirstPassSR;    
                        END IF;
               -- FirstPassSR decides whether to do another read or to do the bit write.
               
               WHEN FirstPassSR =>
                        owm_TimeSlotCnt <= owm_TimeSlotCnt + "000000001";    
                        IF (owm_timeslot_firstpass_transition = '1') THEN
                           owm_TimeSlotCnt <= "000000000";    
                           IF (NOT First = '1') THEN
                              First <= '1';    
                              owm_state <= DQLOW;    
                           ELSE
                              owm_state <= WriteBitSR;    
                           END IF;
                           -- else: !if(!First)
                           
                           
                        END IF;
               -- WriteBitSR will now determine the bit necessary to write
               
               -- for the Search ROM to proceed.
               
               WHEN WriteBitSR =>
                        temp_tmp10 := BitRead1 & BitRead2;
                        CASE temp_tmp10 IS
                           WHEN "00" =>
                                    CASE owm_index IS
                                       WHEN "0000" =>
                                                BitWrite <= xmit_shiftreg(1); 
                                                                                                rcvr_shiftreg(0) <= '1';    
                                       WHEN "0001" =>
                                                BitWrite <= xmit_shiftreg(2); 
                                                                                                rcvr_shiftreg(1) <= '1';    
                                       WHEN "0010" =>
                                                BitWrite <= xmit_shiftreg(3); 
                                                                                                rcvr_shiftreg(2) <= '1';    
                                       WHEN "0011" =>
                                                BitWrite <= xmit_shiftreg(4); 
                                                                                                rcvr_shiftreg(3) <= '1';    
                                       WHEN "0100" =>
                                                BitWrite <= xmit_shiftreg(5); 
                                                                                                rcvr_shiftreg(4) <= '1';    
                                       WHEN "0101" =>
                                                BitWrite <= xmit_shiftreg(6); 
                                                                                                rcvr_shiftreg(5) <= '1';    
                                       WHEN "0110" =>
                                                BitWrite <= xmit_shiftreg(7); 
                                                                                                rcvr_shiftreg(6) <= '1';    
                                       WHEN "0111" =>
                                                BitWrite <= xmit_shiftreg(0); 
                                                                                                rcvr_shiftreg(7) <= '1';    
                                       WHEN OTHERS =>
                                                NULL;
                                       
                                    END CASE;
                           WHEN "01" =>
                                    BitWrite <= '0';    
                                    CASE owm_index IS
                                       WHEN "0000" =>
                                                rcvr_shiftreg(0) <= '0';    
                                       WHEN "0001" =>
                                                rcvr_shiftreg(1) <= '0';    
                                       WHEN "0010" =>
                                                rcvr_shiftreg(2) <= '0';    
                                       WHEN "0011" =>
                                                rcvr_shiftreg(3) <= '0';    
                                       WHEN "0100" =>
                                                rcvr_shiftreg(4) <= '0';    
                                       WHEN "0101" =>
                                                rcvr_shiftreg(5) <= '0';    
                                       WHEN "0110" =>
                                                rcvr_shiftreg(6) <= '0';    
                                       WHEN "0111" =>
                                                rcvr_shiftreg(7) <= '0';    
                                       WHEN OTHERS =>
                                                NULL;
                                       
                                    END CASE;
                           WHEN "10" =>
                                    BitWrite <= '1';    
                                    CASE owm_index IS
                                       WHEN "0000" =>
                                                rcvr_shiftreg(0) <= '0';    
                                       WHEN "0001" =>
                                                rcvr_shiftreg(1) <= '0';    
                                       WHEN "0010" =>
                                                rcvr_shiftreg(2) <= '0';    
                                       WHEN "0011" =>
                                                rcvr_shiftreg(3) <= '0';    
                                       WHEN "0100" =>
                                                rcvr_shiftreg(4) <= '0';    
                                       WHEN "0101" =>
                                                rcvr_shiftreg(5) <= '0';    
                                       WHEN "0110" =>
                                                rcvr_shiftreg(6) <= '0';    
                                       WHEN "0111" =>
                                                rcvr_shiftreg(7) <= '0';    
                                       WHEN OTHERS =>
                                                NULL;
                                       
                                    END CASE;
                           WHEN "11" =>
                                    BitWrite <= '1';    
                                    CASE owm_index IS
                                       WHEN "0000" =>
                                                rcvr_shiftreg(0) <= '1';    
                                                rcvr_shiftreg(1) <= '1';    
                                       WHEN "0001" =>
                                                rcvr_shiftreg(1) <= '1';    
                                                rcvr_shiftreg(2) <= '1';    
                                       WHEN "0010" =>
                                                rcvr_shiftreg(2) <= '1';    
                                                rcvr_shiftreg(3) <= '1';    
                                       WHEN "0011" =>
                                                rcvr_shiftreg(3) <= '1';    
                                                rcvr_shiftreg(4) <= '1';    
                                       WHEN "0100" =>
                                                rcvr_shiftreg(4) <= '1';    
                                                rcvr_shiftreg(5) <= '1';    
                                       WHEN "0101" =>
                                                rcvr_shiftreg(5) <= '1';    
                                                rcvr_shiftreg(6) <= '1';    
                                       WHEN "0110" =>
                                                rcvr_shiftreg(6) <= '1';    
                                                rcvr_shiftreg(7) <= '1';    
                                       WHEN "0111" =>
                                                rcvr_shiftreg(7) <= '1';    
                                                rcvr_shiftreg(0) <= '1';    
                                       WHEN OTHERS =>
                                                NULL;
                                       
                                    END CASE;
                           WHEN OTHERS =>
                                    NULL;
                           
                        END CASE;
                        -- case({BitRead1,BitRead2})
                        
                        owm_state <= WriteBit;    
               -- WriteBit actually writes the chosen bit to the One Wire bus.
               
               WHEN WriteBit =>
                        owm_TimeSlotCnt <= owm_TimeSlotCnt + "000000001";    
                        CASE owm_index IS
                           WHEN "0000" =>
                                    rcvr_shiftreg(1) <= BitWrite;    
                           WHEN "0001" =>
                                    rcvr_shiftreg(2) <= BitWrite;    
                           WHEN "0010" =>
                                    rcvr_shiftreg(3) <= BitWrite;    
                           WHEN "0011" =>
                                    rcvr_shiftreg(4) <= BitWrite;    
                           WHEN "0100" =>
                                    rcvr_shiftreg(5) <= BitWrite;    
                           WHEN "0101" =>
                                    rcvr_shiftreg(6) <= BitWrite;    
                           WHEN "0110" =>
                                    rcvr_shiftreg(7) <= BitWrite;    
                           WHEN "0111" =>
                                    rcvr_shiftreg(0) <= BitWrite;    
                           WHEN OTHERS =>
                                    NULL;
                           
                        END CASE;
                        IF (NOT BitWrite = '1') THEN
                           owm_state <= owm_state_write_zero;    
                        ELSE
                           owm_state <= owm_state_write_one;    
                        END IF;
               -- WaitTS waits until the timeslot is completed, 80us. When done with
               
               -- that timeslot, the owm_index will be incremented.
               
               WHEN WaitTS =>
                        owm_TimeSlotCnt <= owm_TimeSlotCnt + "000000001";    
                        IF (owm_timeslot_waitts_transition = '1') THEN
                           --if(owm_TimeSlotCnt == (194))
                           
                           owm_state <= IndexInc;    
                        END IF;
               -- IndexInc incs the owm_index by 1 if normal write, by 2 if in SRA
               
               WHEN IndexInc =>
                        IF (NOT sr_a_tmp5 = '1') THEN
                           owm_index <= owm_index + "0001";    
                        ELSE
                           owm_index <= owm_index + "0010";    
                           First <= '0';    
                        END IF;
                        -- else: !if(!sr_a)
                        
                        IF ((owm_index = "0111" AND sr_a_tmp5 = '0') OR (owm_index = "0110" 
                        AND sr_a_tmp5 = '1')) THEN
                           byte_done <= '1';    
                           owm_state <= UpdateBuff;    
                        -- if ((owm_index == 8-1 && !sr_a) || (owm_index == 8-2 && sr_a)  )
                        
                        ELSE
                           IF (owm_index = "0110" AND sr_a_tmp5 = '0') THEN
                              last_rcvr_bit <= '1';    
                           ELSE
                              IF (owm_index = "0100" AND sr_a_tmp5 = '1') THEN
                                 last_rcvr_bit <= '1';    
                              END IF;
                           END IF;
                           owm_state <= DQLOW;    
                           owm_TimeSlotCnt <= "000000000";    
                        END IF;
               WHEN UpdateBuff =>
                        owm_state <= IdleS;    
               WHEN OTHERS =>
                        NULL;
               
            END CASE;
         END IF;
      END IF;
   END PROCESS;


END translated;



