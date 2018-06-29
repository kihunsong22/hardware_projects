

library IEEE;
use IEEE.std_logic_1164.all;

ENTITY owm_io IS
   PORT (
      DIN                     : OUT std_logic_vector(7 DOWNTO 0);   
      DOUT                    : IN std_logic_vector(7 DOWNTO 0);   
      DDIR                    : IN std_logic;   
      DATA_IN                 : OUT std_logic_vector(7 DOWNTO 0);
      DATA_OUT                : IN std_logic_vector(7 DOWNTO 0);
      DQ                      : IN std_logic;
      --DQ_CONTROL              : IN std_logic;   
      DQ_IN                   : OUT std_logic;   
      ADDRESS                 : IN std_logic_vector(2 DOWNTO 0));   
END owm_io;

ARCHITECTURE One_Wire_Master OF owm_io IS


   SIGNAL DIN_xhdl1                :  std_logic_vector(7 DOWNTO 0);
   SIGNAL DQ_IN_xhdl2              :  std_logic;   

BEGIN
   DIN <= DIN_xhdl1;
   DQ_IN <= DQ_IN_xhdl2;
   --DATA <= DOUT WHEN DDIR = '1'  AND ADDRESS < "101"  ELSE "ZZZZZZZZ" ;
   DATA_IN <= DOUT;          --MJR
   --DIN_xhdl1 <= DATA ;
   DIN_xhdl1 <= DATA_OUT ;   --MJR
   --DQ <= 'Z' WHEN DQ_CONTROL = '1' ELSE '0' ;
   DQ_IN_xhdl2 <= DQ ;

END One_Wire_Master;

