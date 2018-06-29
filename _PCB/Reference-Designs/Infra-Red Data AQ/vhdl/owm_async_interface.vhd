

library IEEE;
use IEEE.std_logic_1164.all;



ENTITY owm_async_interface IS
   PORT (
      DIN                     : IN std_logic_vector(7 DOWNTO 0);   
      DDIR                    : OUT std_logic;   
      DQ_IN                   : IN std_logic;   
      ADDRESS                 : IN std_logic_vector(2 DOWNTO 0);   
      ADS_n                 : IN std_logic;
      EN_n                  : IN std_logic;   
      RD_n                  : IN std_logic;   
      WR_n                  : IN std_logic;
      MR                      : IN std_logic;   
      cmd_reg_write_flag_sync : IN std_logic;   
      xmit_reg_write_flag_sync: IN std_logic;   
      rx_reg_read_flag_sync   : IN std_logic;
      interrupt_reg_read_flag_sync: IN std_logic;   
      xmit_buffer             : OUT std_logic_vector(7 DOWNTO 0);   
      interrupt_enbl          : OUT std_logic_vector(7 DOWNTO 0);   
      clkdiv_reg              : OUT std_logic_vector(7 DOWNTO 0);   
      cmd_reg_write_flag      : OUT std_logic;   
      xmit_reg_write_flag     : OUT std_logic;   
      interrupt_reg_read_flag : OUT std_logic;   
      rx_reg_read_flag        : OUT std_logic;   
      sel_addr                : OUT std_logic_vector(2 DOWNTO 0);   
      cmd_reg                 : OUT std_logic_vector(7 DOWNTO 0));   
END  ENTITY owm_async_interface;

ARCHITECTURE translated OF owm_async_interface IS


   SIGNAL read_op                  :  std_logic;   
   SIGNAL  COMMAND_REG           :  std_logic_vector(4 DOWNTO 0):= "00000";    
   SIGNAL  XMIT_BUF              :  std_logic_vector(4 DOWNTO 0) := "00001";    
   SIGNAL  RX_BUF                :  std_logic_vector(4 DOWNTO 0) := "00001";    
   SIGNAL  INT_REG               :  std_logic_vector(4 DOWNTO 0) := "00010";    
   SIGNAL  INT_ENBL_REG          :  std_logic_vector(4 DOWNTO 0) := "00011";
   SIGNAL  CLK_DIV_REG           :  std_logic_vector(4 DOWNTO 0) := "10001";
   SIGNAL DDIR_tmp1               :  std_logic;
   SIGNAL cmd_reg_tmp2            :  std_logic_vector(7 DOWNTO 0);
   SIGNAL xmit_buffer_tmp3        :  std_logic_vector(7 DOWNTO 0);
   SIGNAL interrupt_enbl_tmp4     :  std_logic_vector(7 DOWNTO 0);
   SIGNAL clkdiv_reg_tmp5         :  std_logic_vector(7 DOWNTO 0);
   SIGNAL cmd_reg_write_flag_tmp6 :  std_logic;
   SIGNAL xmit_reg_write_flag_tmp7:  std_logic;
   SIGNAL interrupt_reg_read_flag_tmp8   :  std_logic;
   SIGNAL rx_reg_read_flag_tmp9   :  std_logic;   
   SIGNAL sel_addr_tmp10          :  std_logic_vector(2 DOWNTO 0);   

BEGIN
   DDIR <= DDIR_tmp1;
   cmd_reg <= cmd_reg_tmp2;
   xmit_buffer <= xmit_buffer_tmp3;
   interrupt_enbl <= interrupt_enbl_tmp4;
   clkdiv_reg <= clkdiv_reg_tmp5;
   cmd_reg_write_flag <= cmd_reg_write_flag_tmp6;
   xmit_reg_write_flag <= xmit_reg_write_flag_tmp7;
   interrupt_reg_read_flag <= interrupt_reg_read_flag_tmp8;
   rx_reg_read_flag <= rx_reg_read_flag_tmp9;
   sel_addr <= sel_addr_tmp10;


   ----------------------------------------------------------------------------
   
   --  Transparent address latch
   
   ----------------------------------------------------------------------------
   
   PROCESS (ADS_n, ADDRESS, EN_n)
      VARIABLE sel_addr_tmp10_tmp11  : std_logic_vector(2 DOWNTO 0);
   BEGIN
      ----------------------------------------------------------------------------
      
      --  write process
      
      ----------------------------------------------------------------------------
      
      IF ((NOT ADS_n AND NOT EN_n) = '1') THEN
         sel_addr_tmp10_tmp11 := ADDRESS;    
      END IF;
      sel_addr_tmp10 <= sel_addr_tmp10_tmp11;
   END PROCESS;
   read_op <= ((NOT EN_n AND NOT MR) AND NOT RD_n) AND WR_n ;
   DDIR_tmp1 <= read_op ;

   PROCESS (WR_n, MR)
      VARIABLE temp_tmp12  : std_logic_vector(4 DOWNTO 0);
   BEGIN
      IF (MR = '1') THEN
         cmd_reg_write_flag_tmp6 <= '0';    
         xmit_reg_write_flag_tmp7 <= '0';    
         xmit_buffer_tmp3 <= "00000000";    
         interrupt_enbl_tmp4 <= "00000000";
         clkdiv_reg_tmp5 <= "10010001";
         cmd_reg_tmp2 <= "00000000";
      ELSIF (WR_n'EVENT AND WR_n = '1') THEN
         temp_tmp12 := NOT RD_n & EN_n & sel_addr_tmp10;
         CASE temp_tmp12 IS
             WHEN "00000" =>
                     cmd_reg_tmp2 <= DIN;    
                     cmd_reg_write_flag_tmp6 <= NOT cmd_reg_write_flag_sync; 
             WHEN "00001" =>
                     xmit_buffer_tmp3 <= DIN;    
                     xmit_reg_write_flag_tmp7 <= NOT xmit_reg_write_flag_sync; 
             WHEN "00011" =>
                     interrupt_enbl_tmp4 <= DIN;    
             WHEN "00100" =>
                     clkdiv_reg_tmp5 <= DIN;    
             WHEN OTHERS =>
                     NULL;
            
         END CASE;
      END IF;
   END PROCESS;

   -- case({EN_n,sel_addr})
   
   ----------------------------------------------------------------------------
   
   --  read process
   
   ----------------------------------------------------------------------------
   
   PROCESS (RD_n, MR)
      VARIABLE temp_tmp13  : std_logic_vector(4 DOWNTO 0);
   BEGIN
      IF (MR = '1') THEN
         rx_reg_read_flag_tmp9 <= '0';    
         interrupt_reg_read_flag_tmp8 <= '0';    
      ELSIF (RD_n'EVENT AND RD_n = '1') THEN
         temp_tmp13 := NOT WR_n & EN_n & sel_addr_tmp10;
         CASE temp_tmp13 IS
            WHEN "00001" =>
                     rx_reg_read_flag_tmp9 <= NOT rx_reg_read_flag_sync;    
            WHEN "00010" =>
                     interrupt_reg_read_flag_tmp8 <= NOT interrupt_reg_read_flag_sync; 
                                 WHEN OTHERS =>
                     NULL;
            
         END CASE;
      END IF;
   END PROCESS;
   -- case({EN_n,sel_addr})

END ARCHITECTURE translated;

