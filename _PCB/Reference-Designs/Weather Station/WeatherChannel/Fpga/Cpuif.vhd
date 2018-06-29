-------------------------------------------------------
--- Submodule ramif.vhdl (RAM interface)
-------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
Use IEEE.std_logic_unsigned.all;

entity CPUIF is 
port (
    CLK             : in    std_logic;

    CPUDATAI        : in    std_logic_vector(7 downto 0);
    CPUDATAO        : out   std_logic_vector(7 downto 0);
    MEMRD           : in    std_logic;
    MEMWR           : in    std_logic;
    MEMADDR         : in    std_logic_vector(15 downto 0);
    PSRD            : in    std_logic;
    PSWR            : in    std_logic;
    
    SFR_ADDR        : in    std_logic_vector(6 downto 0);
    SFR_DATAI       : in    std_logic_vector(7 downto 0);
    SFR_DATAO       : out   std_logic_vector(7 downto 0);
    SFR_WR          : in    std_logic;
    SFR_RD          : in    std_logic;

    RAM_DIN         : in    std_logic_vector(7 downto 0);
    RAM_DOUT        : out   std_logic_vector(7 downto 0);
    RAM_ADDR        : out   std_logic_vector(17 downto 0);
    RAM_EN          : out   std_logic;
    RAM_WR          : out   std_logic;
    RAM_RD          : out   std_logic;
    
    LCD_EN          : out   std_logic
) ;
end CPUIF;

architecture rambus of CPUIF is

    signal LCD_SEL      : std_logic;
    signal RAM_SEL      : std_logic;
    signal ADDR_LOW     : std_logic_vector(7 downto 0);
    signal A15          : std_logic;
    signal A16          : std_logic;
    signal A17          : std_logic;
    signal BANK         : std_logic_vector(2 downto 0);

begin

--***********
-- LATCH BANK Select
--***********
    WRITE_BANK: process( CLK, SFR_WR, SFR_ADDR, BANK )
    begin
        if rising_edge( CLK ) then
            if ( SFR_WR = '1' ) then
                if ( SFR_ADDR = "1010001" ) then    -- BANK at SFR address D1h
                    BANK <= SFR_DATAI(2 downto 0);
                end if;
            end if;
        end if;
    end process;


    READ_BANK: process( CLK, SFR_RD, SFR_ADDR, BANK )
    begin
--        if rising_edge( CLK ) then
            if ( SFR_RD = '1' ) then
                if ( SFR_ADDR = "1010001" ) then   -- BANK at SFR address D1h
                    SFR_DATAO(7 downto 3) <= "00000";
                    SFR_DATAO(2 downto 0) <= BANK(2 downto 0);
                else
                    SFR_DATAO <= "00000000";
                end if;
            else
                SFR_DATAO <= "00000000";
            end if;
--        end if;
    end process;


    RAM_ADDR(14 downto 0) <= MEMADDR(14 downto 0);
    RAM_ADDR(15) <= A15;
    RAM_ADDR(16) <= A16;
    RAM_ADDR(17) <= A17;

--***********
-- DATA directions 
--***********

-- TO RAM
    RAM_DOUT <= CPUDATAI;
    
-- TO CPU
    CPUDATAO <= RAM_DIN;


--***********
-- SELECTION
--***********

-- DEVICE selection
    LCD: process( MEMRD, MEMWR, MEMADDR )
    begin
       if (MEMRD = '1' or MEMWR = '1' ) then
          if ( MEMADDR(15 downto 8) = "00000010" ) then    -- LCD at 200h
             LCD_SEL <= '1';
          else
             LCD_SEL <= '0';
          end if;
       else
          LCD_SEL <= '0';
       end if;
    end process;         


-- MEM selection

    RAM_SEL <= PSRD or PSWR or MEMRD or MEMWR ;
    RAM_EN <= RAM_SEL and not LCD_SEL;
    


--***********
-- RAM read/write access
--***********

    RAM_RD <= PSRD or MEMRD;
    RAM_WR <= MEMWR or PSWR;
    


--***********
-- SELECT BANK in RAM
--***********
    process( BANK, PSRD, PSWR, MEMADDR )
    begin
        if PSWR = '1' or PSRD = '1' then    -- CODE 
            A15 <= MEMADDR(15);
            A16 <= '0';
            A17 <= '0';
        elsif MEMADDR(15) = '0' then        -- DATA 
            A15 <= '0';
            A16 <= '1';
            A17 <= '0';
        else
            if BANK = "000" then            -- BANK 0
                A15 <= '1';
                A16 <= '1';
                A17 <= '0';
            elsif BANK = "001" then         -- BANK 1
                A15 <= '0';
                A16 <= '0';
                A17 <= '1';
            elsif BANK = "010" then         -- BANK 2
                A15 <= '1';
                A16 <= '0';
                A17 <= '1';
            elsif BANK = "011" then         -- BANK 3
                A15 <= '0';
                A16 <= '1';
                A17 <= '1';
            elsif BANK = "100" then         -- BANK 4
                A15 <= '1';
                A16 <= '1';
                A17 <= '1';
            else                            -- Default BANK 0
                A15 <= '1';
                A16 <= '1';
                A17 <= '0';
            end if;
        end if;
    end process;


-- DEVICE select lines
    LCD_EN <= LCD_SEL;

end rambus;


