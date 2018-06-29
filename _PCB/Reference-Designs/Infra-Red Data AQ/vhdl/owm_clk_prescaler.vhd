
library IEEE;
use IEEE.std_logic_1164.all;
use ieee.std_logic_unsigned.all;


ENTITY owm_clk_prescaler IS
   PORT (
      clk_1us                 : OUT std_logic;   
      CLK                     : IN std_logic;   
      MR                      : IN std_logic;   
      clkdiv_reg              : IN std_logic_vector(7 DOWNTO 0));   
END owm_clk_prescaler;

ARCHITECTURE translated OF owm_clk_prescaler IS


   -- clock divisor register

   SIGNAL pre_0                    :  std_logic;   
   SIGNAL pre_1                    :  std_logic;   
   SIGNAL div_1                    :  std_logic;   
   SIGNAL div_2                    :  std_logic;   
   SIGNAL div_3                    :  std_logic;   
   SIGNAL clk_prescaled            :  std_logic;   
   SIGNAL div_cnt                  :  std_logic_vector(6 DOWNTO 0);
   SIGNAL clk_prescaled_reg        :  std_logic;   
   SIGNAL ClkPrescale              :  std_logic_vector(2 DOWNTO 0);   
   CONSTANT  s0                    :  std_logic_vector(2 DOWNTO 0) := "000";
      CONSTANT  s1                    :  std_logic_vector(2 DOWNTO 0) := "001"; 
      CONSTANT  s2                    :  std_logic_vector(2 DOWNTO 0) := "010"; 
      CONSTANT  s3                    :  std_logic_vector(2 DOWNTO 0) := "011"; 
      CONSTANT  s4                    :  std_logic_vector(2 DOWNTO 0) := "100"; 
      CONSTANT  s5                    :  std_logic_vector(2 DOWNTO 0) := "101"; 
      CONSTANT  s6                    :  std_logic_vector(2 DOWNTO 0) := "110"; 
      SIGNAL clk_1us_tmp1            :  std_logic;   

BEGIN
   clk_1us <= clk_1us_tmp1;
   div_3 <= clkdiv_reg(4);
   div_2 <= clkdiv_reg(3);
   div_1 <= clkdiv_reg(2);
   pre_1 <= clkdiv_reg(1);
   pre_0 <= clkdiv_reg(0);


   clk_prescaled <= CLK WHEN (NOT pre_0 AND NOT pre_1) = '1' ELSE clk_prescaled_reg ;

   PROCESS (MR, CLK)
   BEGIN
      IF (MR = '1') THEN
         clk_prescaled_reg <= '0';    
      ELSIF (CLK'EVENT AND CLK = '1') THEN
         clk_prescaled_reg <= (NOT ClkPrescale(0) AND NOT ClkPrescale(1)) 
         AND NOT ClkPrescale(2);    
      END IF;
   END PROCESS;

   PROCESS (div_1, div_2, div_3, div_cnt, clk_prescaled)
      VARIABLE clk_1us_tmp1_tmp2  : std_logic;
      VARIABLE temp_tmp3  : std_logic_vector(2 DOWNTO 0);
   BEGIN
      temp_tmp3 := div_3 & div_2 & div_1;
      CASE temp_tmp3 IS
         WHEN "000" =>
                  clk_1us_tmp1_tmp2 := clk_prescaled;    
         WHEN "001" =>
                  clk_1us_tmp1_tmp2 := div_cnt(0);    
         WHEN "010" =>
                  clk_1us_tmp1_tmp2 := div_cnt(1);    
         WHEN "011" =>
                  clk_1us_tmp1_tmp2 := div_cnt(2);    
         WHEN "100" =>
                  clk_1us_tmp1_tmp2 := div_cnt(3);    
         WHEN "101" =>
                  clk_1us_tmp1_tmp2 := div_cnt(4);    
         WHEN "110" =>
                  clk_1us_tmp1_tmp2 := div_cnt(5);    
         WHEN OTHERS  =>
                  clk_1us_tmp1_tmp2 := div_cnt(6);    
         
      END CASE;
      clk_1us_tmp1 <= clk_1us_tmp1_tmp2;
   END PROCESS;

   PROCESS (clk_prescaled, MR)
   BEGIN
      IF (MR = '1') THEN
         div_cnt <= "0000000";    
      ELSIF (clk_prescaled'EVENT AND clk_prescaled = '1') THEN
         div_cnt <= div_cnt + "0000001";    
      END IF;
   END PROCESS;

   PROCESS (MR, CLK)
   BEGIN
      IF (MR = '1') THEN
         ClkPrescale <= s0;    
      ELSIF (CLK'EVENT AND CLK = '1') THEN
         CASE ClkPrescale IS
            WHEN s0 =>
                     ClkPrescale <= s1;    
            WHEN s1 =>
                     ClkPrescale <= s2;    
            WHEN s2 =>
                     IF ((pre_0 AND NOT pre_1) = '1') THEN
                        ClkPrescale <= s0;    
                     ELSE
                        ClkPrescale <= s3;    
                     END IF;
            WHEN s3 =>
                     ClkPrescale <= s4;    
            WHEN s4 =>
                     IF ((NOT pre_0 AND pre_1) = '1') THEN
                        ClkPrescale <= s0;    
                     ELSE
                        ClkPrescale <= s5;    
                     END IF;
            WHEN s5 =>
                     ClkPrescale <= s6;    
            WHEN s6 =>
                     ClkPrescale <= s0;    
            WHEN OTHERS  =>
                     ClkPrescale <= s0;    
            
         END CASE;
      END IF;
   END PROCESS;

END translated;

