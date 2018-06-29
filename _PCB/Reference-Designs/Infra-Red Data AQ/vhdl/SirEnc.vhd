-- SIR encoder for slow IrDA

library IEEE;
use IEEE.std_logic_1164.all;

entity sirEnc is 
port (
      clk           :  in std_logic;
      reset         :  in std_logic;
      baud_enable   :  in std_logic;
      tx_select     :  in std_logic;
      stx_pad       :  in std_logic; 
      IrEnc         :  out std_logic);
end sirEnc;

architecture sirEncode of sirEnc is

signal IrEncs      : std_logic;
signal latch       : std_logic;
signal inv_latch   : std_logic;
signal this       : std_logic;
signal last       : std_logic;

begin
   inv_latch <= '1' when (latch='0') else '0';
   IrEnc <= IrEncs;

process( clk, reset)
   variable cnt16 : integer;
    begin
        if(reset='1') then
          latch <= '0';
          cnt16 := 0;
          IrEncs <= '1';
        ELSIF (clk'EVENT AND clk = '1') THEN
           if (tx_select = '1') and (baud_enable='1') then
              cnt16 := cnt16 + 1;
              if ((this = '0') and (last = '1')) or (cnt16 = 16) then 
                  cnt16 := 0;
              end if;
              case cnt16 is
                 when 7 =>
                    latch <=  stx_pad ;
                 when 8|9|10 =>
                    IrEncs <= inv_latch;
                 when others =>
                   IrEncs <= '0';
              end case;
           end if;
        end if;
    end process;
process (clk) -- to allow detection of start bit
     begin
        IF (clk'EVENT AND clk = '1') THEN
          if (baud_enable = '1') then
             this <= stx_pad;
             last <= this;
          end if;
        end if;
     end process;
end sirEncode;


