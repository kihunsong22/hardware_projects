-- SIR decoder for slow IrDA

library IEEE;
use IEEE.std_logic_1164.all;

entity sirDec is 
port (
      clk           :  in std_logic;
      reset         :  in std_logic;
      rx_pad_i      :  in std_logic;
      negate_rx     :  in std_logic;
      baud_enable   :  in std_logic;
      tx_select     :  in std_logic;
      IrDec         :  out std_logic);
end sirDec;

architecture sirDecode of sirDec is

signal IrDecs     : std_logic;
signal zero       : std_logic;
signal rx_i       : std_logic;
signal inv_rx     : std_logic;
signal this       : std_logic;
signal last       : std_logic;

begin
   inv_rx <= '1' when (rx_pad_i='0') else '0';
   rx_i <= rx_pad_i when (negate_rx='0') else inv_rx;
   
   IrDec <= IrDecs;

process( clk, reset)
   variable cnt16 : integer ;
    begin
        if(reset='1') then
          zero <= '0';
          cnt16 := 0;
          IrDecs <= '1';
        ELSIF (clk'EVENT AND clk = '1' ) THEN
           if (tx_select = '0') and (baud_enable='1') then
              cnt16 := cnt16 + 1;
              if ((this = '1') and (last = '0'))  then 
                  cnt16 := 0;
                  IrDecs <= '0';
              elsif (cnt16 = 16) then
                 cnt16 := 0;
                 IrDecs <= '1';
              end if;
           end if;
        end if;
    end process;
process (clk) -- to allow detection of start bit
     begin
        IF (clk'EVENT AND clk = '1') THEN
          if (baud_enable = '1') then
             this <= rx_i;
             last <= this;
          end if;
        end if;
     end process;
end sirDecode;




