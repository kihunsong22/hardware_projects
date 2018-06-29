

library IEEE;
use IEEE.std_logic_1164.all;

ENTITY owm_data_sel IS
   PORT (
      cmd_reg                 : IN std_logic_vector(7 DOWNTO 0);   
      owm_rcvr_buffer         : IN std_logic_vector(7 DOWNTO 0);
      DQ_IN                   : IN std_logic;
      owm_status              : IN std_logic_vector(7 DOWNTO 0);   
      clkdiv_reg              : IN std_logic_vector(7 DOWNTO 0);   
      DOUT                    : OUT std_logic_vector(7 DOWNTO 0);   
      sel_addr                : IN std_logic_vector(2 DOWNTO 0);   
      interrupt_enbl          : IN std_logic_vector(7 DOWNTO 0);   
      owr                     : OUT std_logic;   
      owr_trigger             : IN std_logic;   
      owm_interrupt           : IN std_logic;   
      INTR                    : OUT std_logic);   
END owm_data_sel;

ARCHITECTURE translated OF owm_data_sel IS


   SIGNAL dqoe                     :  std_logic;   
   SIGNAL ersf                     :  std_logic;   
   SIGNAL erbf                     :  std_logic;   
   SIGNAL etmt                     :  std_logic;   
   SIGNAL etbe                     :  std_logic;   
   SIGNAL ias                      :  std_logic;   
   SIGNAL epd                      :  std_logic;   
   CONSTANT  COMMAND_REG           :  std_logic_vector(2 DOWNTO 0) := "000";    
   CONSTANT  XMIT_BUF              :  std_logic_vector(2 DOWNTO 0) := "001";    
   CONSTANT  RX_BUF                :  std_logic_vector(2 DOWNTO 0) := "001";    
   CONSTANT  INT_REG               :  std_logic_vector(2 DOWNTO 0) := "010";    
   CONSTANT  INT_ENBL_REG          :  std_logic_vector(2 DOWNTO 0) := "011";    
   CONSTANT  CLK_DIV_REG           :  std_logic_vector(2 DOWNTO 0) := "100";    
   SIGNAL dstat0                   :  std_logic_vector(7 DOWNTO 0);   
   SIGNAL dstat1                   :  std_logic_vector(7 DOWNTO 0);   
   SIGNAL dstat2                   :  std_logic_vector(7 DOWNTO 0);   
   SIGNAL dstat3                   :  std_logic_vector(7 DOWNTO 0);   
   SIGNAL dstat4                   :  std_logic_vector(7 DOWNTO 0);   
   SIGNAL owr_tmp1                :  std_logic;   
   SIGNAL INTR_tmp2               :  std_logic;   
   SIGNAL DOUT_tmp3               :  std_logic_vector(7 DOWNTO 0);   

BEGIN
   owr <= owr_tmp1;
   INTR <= INTR_tmp2;
   DOUT <= DOUT_tmp3;
   INTR_tmp2 <= owm_interrupt ;
   owr_tmp1 <= cmd_reg(0) AND owr_trigger ;
   dstat0 <= cmd_reg(7 DOWNTO 4) & DQ_IN & cmd_reg(2 DOWNTO 0) ;
   dstat1 <= owm_rcvr_buffer ;
   dstat2 <= owm_status ;
   dstat3 <= interrupt_enbl ;
   dstat4 <= clkdiv_reg ;

   PROCESS (sel_addr, dstat0, dstat1, dstat2, dstat3, dstat4)
      VARIABLE DOUT_tmp3_tmp4  : std_logic_vector(7 DOWNTO 0);
   BEGIN
      CASE sel_addr IS
         WHEN COMMAND_REG =>
                  DOUT_tmp3_tmp4 := dstat0;    
         WHEN RX_BUF =>
                  DOUT_tmp3_tmp4 := dstat1;    
         WHEN INT_REG =>
                  DOUT_tmp3_tmp4 := dstat2;    
         WHEN INT_ENBL_REG =>
                  DOUT_tmp3_tmp4 := dstat3;    
         WHEN CLK_DIV_REG =>
                  DOUT_tmp3_tmp4 := dstat4;    
         WHEN OTHERS  =>
                  DOUT_tmp3_tmp4 := "00000000";    
         
      END CASE;
      DOUT_tmp3 <= DOUT_tmp3_tmp4;
   END PROCESS;
   -- case(sel_addr)

END translated;

