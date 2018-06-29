                   

library IEEE;
use IEEE.std_logic_1164.all;


ENTITY owm_clk_bridge IS
   PORT (
      clk_1us                 : IN std_logic;   
      MR                      : IN std_logic;   
      reset                   : OUT std_logic;   
      cmd_reg_write_flag_sync : OUT std_logic;   
      xmit_reg_write_flag_sync: OUT std_logic;   
      rx_reg_read_flag_sync   : OUT std_logic;   
      interrupt_reg_read_flag_sync: OUT std_logic;   
      owr_trigger             : OUT std_logic;   
      write_owm_data          : OUT std_logic;   
      read_owm_data           : OUT std_logic;   
      read_owm_status         : OUT std_logic;   
      cmd_reg_write_flag      : IN std_logic;   
      xmit_reg_write_flag     : IN std_logic;   
      rx_reg_read_flag        : IN std_logic;   
      interrupt_reg_read_flag : IN std_logic);   
END owm_clk_bridge;

ARCHITECTURE translated OF owm_clk_bridge IS


   SIGNAL cmd_reg_write_flag_sync_a:  std_logic;   
   SIGNAL xmit_reg_write_flag_sync_a      :  std_logic;   
   SIGNAL rx_reg_read_flag_sync_a  :  std_logic;   
   SIGNAL interrupt_reg_read_flag_sync_a  :  std_logic;   
   SIGNAL rst                      :  std_logic_vector(1 DOWNTO 0);   
   SIGNAL cmd_reg_write_flag_sync_tmp1   :  std_logic;   
   SIGNAL xmit_reg_write_flag_sync_tmp2  :  std_logic;   
   SIGNAL rx_reg_read_flag_sync_tmp3     :  std_logic;   
   SIGNAL interrupt_reg_read_flag_sync_tmp4:  std_logic;   
   SIGNAL owr_trigger_tmp5        :  std_logic;   
   SIGNAL write_owm_data_tmp6     :  std_logic;   
   SIGNAL read_owm_data_tmp7      :  std_logic;   
   SIGNAL read_owm_status_tmp8    :  std_logic;   
   SIGNAL reset_tmp9              :  std_logic;   

BEGIN
   cmd_reg_write_flag_sync <= cmd_reg_write_flag_sync_tmp1;
   xmit_reg_write_flag_sync <= xmit_reg_write_flag_sync_tmp2;
   rx_reg_read_flag_sync <= rx_reg_read_flag_sync_tmp3;
   interrupt_reg_read_flag_sync <= interrupt_reg_read_flag_sync_tmp4;
   owr_trigger <= owr_trigger_tmp5;
   write_owm_data <= write_owm_data_tmp6;
   read_owm_data <= read_owm_data_tmp7;
   read_owm_status <= read_owm_status_tmp8;
   reset <= reset_tmp9;
   reset_tmp9 <= rst(1) or rst(0) or MR ;
   owr_trigger_tmp5 <= cmd_reg_write_flag_sync_tmp1 XOR cmd_reg_write_flag_sync_a ;
   write_owm_data_tmp6 <= xmit_reg_write_flag_sync_tmp2 XOR xmit_reg_write_flag_sync_a ;
   read_owm_data_tmp7 <= rx_reg_read_flag_sync_tmp3 XOR rx_reg_read_flag_sync_a ;
   read_owm_status_tmp8 <= interrupt_reg_read_flag_sync_tmp4 XOR interrupt_reg_read_flag_sync_a ;

   -- reset flops.
   
   PROCESS (clk_1us)
   BEGIN
      IF (clk_1us'EVENT AND clk_1us = '1') THEN
         rst <= rst(0) & MR;    
      END IF;
   END PROCESS;

   PROCESS (clk_1us, reset_tmp9)
   BEGIN
      IF (reset_tmp9 = '1') THEN
         cmd_reg_write_flag_sync_tmp1 <= '0';    
         xmit_reg_write_flag_sync_tmp2 <= '0';    
         rx_reg_read_flag_sync_tmp3 <= '0';    
         interrupt_reg_read_flag_sync_tmp4 <= '0';    
         cmd_reg_write_flag_sync_a <= '0';    
         xmit_reg_write_flag_sync_a <= '0';    
         rx_reg_read_flag_sync_a <= '0';    
         interrupt_reg_read_flag_sync_a <= '0';    
      -- if (reset)
      
      ELSIF (clk_1us'EVENT AND clk_1us = '1') THEN
         cmd_reg_write_flag_sync_a <= cmd_reg_write_flag;    
         xmit_reg_write_flag_sync_a <= xmit_reg_write_flag;    
         rx_reg_read_flag_sync_a <= rx_reg_read_flag;    
         interrupt_reg_read_flag_sync_a <= interrupt_reg_read_flag;    
         cmd_reg_write_flag_sync_tmp1 <= cmd_reg_write_flag_sync_a;    
         xmit_reg_write_flag_sync_tmp2 <= xmit_reg_write_flag_sync_a;    
         rx_reg_read_flag_sync_tmp3 <= rx_reg_read_flag_sync_a;    
         interrupt_reg_read_flag_sync_tmp4 <= interrupt_reg_read_flag_sync_a; 
               END IF;
   END PROCESS;

END translated;

