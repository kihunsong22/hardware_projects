


library IEEE;
use IEEE.std_logic_1164.all;



ENTITY OWM IS
   PORT (
      TEST1                    : OUT std_logic;
      TEST2                    : OUT std_logic;
      TEST3                    : OUT std_logic;
      CLK                      : IN std_logic;
      DATA_IN                  : OUT std_logic_vector(7 DOWNTO 0);
      DATA_OUT                 : IN std_logic_vector(7 DOWNTO 0);

      RD_n                     : IN std_logic;
      ADS_n                    : IN std_logic;
      DQ                       : IN std_logic;
      DQ_CONTROL               : OUT std_logic;
      WR_n                     : IN std_logic;
      INTR                     : OUT std_logic;
      ADDRESS                  : IN std_logic_vector(2 DOWNTO 0);
      EN_n                     : IN std_logic;
      MR                       : IN std_logic);
END OWM;

ARCHITECTURE translated OF OWM IS

   COMPONENT owm_async_interface
      PORT (
         DIN                     : IN  std_logic_vector(7 DOWNTO 0);
         DDIR                    : OUT std_logic;
         DQ_IN                   : IN  std_logic;
         ADDRESS                 : IN  std_logic_vector(2 DOWNTO 0);
         ADS_n                 : IN  std_logic;
         EN_n                  : IN  std_logic;
         RD_n                  : IN  std_logic;
         WR_n                  : IN  std_logic;
         MR                      : IN  std_logic;
         cmd_reg_write_flag_sync : IN  std_logic;
         xmit_reg_write_flag_sync: IN  std_logic;
         rx_reg_read_flag_sync   : IN  std_logic;
         interrupt_reg_read_flag_sync: IN  std_logic;
         xmit_buffer             : OUT std_logic_vector(7 DOWNTO 0);
         interrupt_enbl          : OUT std_logic_vector(7 DOWNTO 0);
         clkdiv_reg              : OUT std_logic_vector(7 DOWNTO 0);
         cmd_reg_write_flag      : OUT std_logic;
         xmit_reg_write_flag     : OUT std_logic;
         interrupt_reg_read_flag : OUT std_logic;
         rx_reg_read_flag        : OUT std_logic;
         sel_addr                : OUT std_logic_vector(2 DOWNTO 0);
         cmd_reg                 : OUT std_logic_vector(7 DOWNTO 0));
   END COMPONENT;

   COMPONENT owm_clk_bridge
      PORT (
         clk_1us                 : IN  std_logic;
         MR                      : IN  std_logic;
         reset                   : OUT std_logic;
         cmd_reg_write_flag_sync : OUT std_logic;
         xmit_reg_write_flag_sync: OUT std_logic;
         rx_reg_read_flag_sync   : OUT std_logic;
         interrupt_reg_read_flag_sync: OUT std_logic;
         owr_trigger             : OUT std_logic;
         write_owm_data          : OUT std_logic;
         read_owm_data           : OUT std_logic;
         read_owm_status         : OUT std_logic;
         cmd_reg_write_flag      : IN  std_logic;
         xmit_reg_write_flag     : IN  std_logic;
         rx_reg_read_flag        : IN  std_logic;
         interrupt_reg_read_flag : IN  std_logic);
   END COMPONENT;

   COMPONENT owm_clk_prescaler
      PORT (
         clk_1us                 : OUT std_logic;
         CLK                     : IN  std_logic;
         MR                      : IN  std_logic;
         clkdiv_reg              : IN  std_logic_vector(7 DOWNTO 0));
   END COMPONENT;

   COMPONENT owm_data_sel
      PORT (
         cmd_reg                 : IN  std_logic_vector(7 DOWNTO 0);
         owm_rcvr_buffer         : IN  std_logic_vector(7 DOWNTO 0);
         DQ_IN                   : IN  std_logic;
         owm_status              : IN  std_logic_vector(7 DOWNTO 0);
         clkdiv_reg              : IN  std_logic_vector(7 DOWNTO 0);
         DOUT                    : OUT std_logic_vector(7 DOWNTO 0);
         sel_addr                : IN  std_logic_vector(2 DOWNTO 0);
         interrupt_enbl          : IN  std_logic_vector(7 DOWNTO 0);
         owr                     : OUT std_logic;
         owr_trigger             : IN  std_logic;
         owm_interrupt           : IN  std_logic;
         INTR                    : OUT std_logic);
   END COMPONENT;

   COMPONENT owm_io
      PORT (
         DIN                     : OUT std_logic_vector(7 DOWNTO 0);
         DOUT                    : IN  std_logic_vector(7 DOWNTO 0);
         DDIR                    : IN  std_logic;
      DATA_IN                    : OUT std_logic_vector(7 DOWNTO 0);
      DATA_OUT                   : IN std_logic_vector(7 DOWNTO 0);
         DQ                      : IN std_logic;
         DQ_IN                   : OUT std_logic;
         ADDRESS                 : IN  std_logic_vector(2 DOWNTO 0));
   END COMPONENT;

   COMPONENT owm_machine
      PORT (
         cmd_reg                 : IN  std_logic_vector(7 DOWNTO 0);
         DQ_IN                   : IN  std_logic;
         read_owm_status         : IN  std_logic;
         read_owm_data           : IN  std_logic;
         write_owm_data          : IN  std_logic;
         owm_interrupt           : OUT std_logic;
         interrupt_enbl          : IN  std_logic_vector(7 DOWNTO 0);
         DQ_CONTROL              : OUT std_logic;
         reset                   : IN  std_logic;
         clk_1us                 : IN  std_logic;
         owm_rcvr_buffer         : OUT std_logic_vector(7 DOWNTO 0);
         owr                     : IN  std_logic;
         xmit_buffer             : IN  std_logic_vector(7 DOWNTO 0);
         owm_status              : OUT std_logic_vector(7 DOWNTO 0);
         sr_a                    : OUT std_logic);
   END COMPONENT;


   SIGNAL DDIR                     :  std_logic;   
   SIGNAL interrupt_enbl           :  std_logic_vector(7 DOWNTO 0);   
   SIGNAL cmd_reg                  :  std_logic_vector(7 DOWNTO 0);   
   SIGNAL DQ_IN                    :  std_logic;   
   SIGNAL cmd_reg_write_flag_sync  :  std_logic;   
   SIGNAL read_owm_status          :  std_logic;   
   SIGNAL sel_addr                 :  std_logic_vector(2 DOWNTO 0);   
   SIGNAL DIN                      :  std_logic_vector(7 DOWNTO 0);
   SIGNAL cmd_reg_write_flag       :  std_logic;
   SIGNAL DOUT                     :  std_logic_vector(7 DOWNTO 0);   
   SIGNAL rx_reg_read_flag         :  std_logic;   
   SIGNAL xmit_reg_write_flag_sync :  std_logic;
   SIGNAL owm_rcvr_buffer          :  std_logic_vector(7 DOWNTO 0);
   SIGNAL xmit_reg_write_flag      :  std_logic;
   SIGNAL reset                    :  std_logic;   
   SIGNAL sr_a                     :  std_logic;
   SIGNAL read_owm_data            :  std_logic;   
   SIGNAL owr_trigger              :  std_logic;   
   SIGNAL owm_status               :  std_logic_vector(7 DOWNTO 0);   
   SIGNAL clk_1us                  :  std_logic;
   SIGNAL clkdiv_reg               :  std_logic_vector(7 DOWNTO 0);   
   SIGNAL xmit_buffer              :  std_logic_vector(7 DOWNTO 0);   
   SIGNAL DQ_CONTROL_s             :  std_logic;
   SIGNAL write_owm_data           :  std_logic;   
   SIGNAL rx_reg_read_flag_sync    :  std_logic;   
   SIGNAL owr                      :  std_logic;   
   SIGNAL interrupt_reg_read_flag_sync    :  std_logic;   
   SIGNAL interrupt_reg_read_flag  :  std_logic;   
   SIGNAL owm_interrupt            :  std_logic;   
   SIGNAL INTR_xhdl1               :  std_logic;   
   SIGNAL DATA_IN_s                :  std_logic_vector(7 DOWNTO 0);

BEGIN
   TEST1 <= owm_status(4);
   TEST2 <= clk_1us;
   TEST3 <= owm_status(2);

   INTR <= INTR_xhdl1;
   --DQ_CONTROL <= DQ_CONTROL_s;
   --DATA_IN <= DATA_IN_s;

   owm_async_interface_1 : owm_async_interface 
      PORT MAP (
         xmit_reg_write_flag => xmit_reg_write_flag,
         DDIR => DDIR,
         ADDRESS => ADDRESS,
         EN_n => EN_n,
         RD_n => RD_n,
         DQ_IN => DQ_IN,
         ADS_n => ADS_n,
         interrupt_enbl => interrupt_enbl,
         cmd_reg_write_flag_sync => cmd_reg_write_flag_sync,
         cmd_reg => cmd_reg,
         cmd_reg_write_flag => cmd_reg_write_flag,
         rx_reg_read_flag_sync => rx_reg_read_flag_sync,
         interrupt_reg_read_flag_sync => interrupt_reg_read_flag_sync,
         clkdiv_reg => clkdiv_reg,
         xmit_buffer => xmit_buffer,
         rx_reg_read_flag => rx_reg_read_flag,
         DIN => DIN,
         WR_n => WR_n,
         xmit_reg_write_flag_sync => xmit_reg_write_flag_sync,
         interrupt_reg_read_flag => interrupt_reg_read_flag,
         sel_addr => sel_addr,
         MR => MR);   
   
   owm_clk_bridge_1 : owm_clk_bridge 
      PORT MAP (
         cmd_reg_write_flag_sync => cmd_reg_write_flag_sync,
         read_owm_status => read_owm_status,
         xmit_reg_write_flag => xmit_reg_write_flag,
         clk_1us => clk_1us,
         reset => reset,
         cmd_reg_write_flag => cmd_reg_write_flag,
         write_owm_data => write_owm_data,
         read_owm_data => read_owm_data,
         rx_reg_read_flag_sync => rx_reg_read_flag_sync,
         interrupt_reg_read_flag_sync => interrupt_reg_read_flag_sync,
         rx_reg_read_flag => rx_reg_read_flag,
         owr_trigger => owr_trigger,
         interrupt_reg_read_flag => interrupt_reg_read_flag,
         xmit_reg_write_flag_sync => xmit_reg_write_flag_sync,
         MR => MR);   
   
   owm_io_1 : owm_io 
      PORT MAP (
         DDIR => DDIR,
         DQ => DQ,
         ADDRESS => ADDRESS,
         --DATA_IN => DATA_IN_s,           --MJR
         DATA_IN => DATA_IN,
         DATA_OUT => DATA_OUT,         --MJR
        DIN => DIN,
         DQ_IN => DQ_IN,
         DOUT => DOUT);   
   
   owm_clk_prescaler_1 : owm_clk_prescaler 
      PORT MAP (
         clkdiv_reg => clkdiv_reg,
         CLK => CLK,
         clk_1us => clk_1us,
         MR => MR);   
   
   owm_machine_1 : owm_machine 
      PORT MAP (
         read_owm_status => read_owm_status,
         cmd_reg => cmd_reg,
         clk_1us => clk_1us,
         reset => reset,
         sr_a => sr_a,
         --DQ_CONTROL => DQ_CONTROL_s,
         DQ_CONTROL => DQ_CONTROL,
         write_owm_data => write_owm_data,
         owm_status => owm_status,
         read_owm_data => read_owm_data,
         owr => owr,
         owm_rcvr_buffer => owm_rcvr_buffer,
         xmit_buffer => xmit_buffer,
         DQ_IN => DQ_IN,
         interrupt_enbl => interrupt_enbl,
         owm_interrupt => owm_interrupt);   
   
   owm_data_sel_1 : owm_data_sel
      PORT MAP (
         cmd_reg => cmd_reg,
         owm_status => owm_status,
         owr => owr,
         owm_rcvr_buffer => owm_rcvr_buffer,
         clkdiv_reg => clkdiv_reg,
         DQ_IN => DQ_IN,
         owr_trigger => owr_trigger,
         DOUT => DOUT,
         INTR => INTR_xhdl1,
         interrupt_enbl => interrupt_enbl,
         sel_addr => sel_addr,
         owm_interrupt => owm_interrupt);



END translated;

