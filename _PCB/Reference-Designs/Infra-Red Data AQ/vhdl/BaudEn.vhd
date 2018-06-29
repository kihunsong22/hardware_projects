library IEEE;
use IEEE.std_logic_1164.all;
use ieee.std_logic_unsigned.all;


entity BAUDEN is
    port(
       clk    :  IN std_logic;    
       rst    :  IN std_logic; 
       baud_en   :  out std_logic);
end BAUDEN;

architecture baud_rtl of BAUDEN is

  constant MAX : std_logic_vector(8 DOWNTO 0) := "101000101"; -- 325 50MHz to 9600 baud
  signal termcnt : std_logic;
  signal count : std_logic_vector(8 DOWNTO 0);

  begin
  
     baud_en <= termcnt;

     process (clk,rst)
     begin
       if (rst = '1') then count<=MAX;
       elsif  (clk'EVENT AND clk = '1') THEN
         if count = "000000000" then
            count <= MAX;
         else
            count<=count-"000000001";
         end if;
         if (count = "000000000") then
            termcnt  <= '1';
         else
            termcnt <= '0';
         end if;
       end if;
      end process;  
  end baud_rtl;
