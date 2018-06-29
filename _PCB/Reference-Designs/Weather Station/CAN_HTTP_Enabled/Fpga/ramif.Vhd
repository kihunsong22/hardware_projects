-----------------------------------------------------------------
--- Submodule dual_mem_if.vhdl (RAM interface)
------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
Use IEEE.std_logic_unsigned.all;

entity RAMIF is
port (
-- CPU interface
    DATAI           : in    std_logic_vector(7 downto 0);
    DATAO           : out   std_logic_vector(7 downto 0);
    ADDR            : in    std_logic_vector(17 downto 0);
    WE              : in    std_logic;
    OE              : in    std_logic;
    CS              : in    std_logic;

-- RAM interface
    RAM0_DATA       : inout std_logic_vector(7 downto 0);
    RAM1_DATA       : inout std_logic_vector(7 downto 0);
    RAM_ADDR        : out   std_logic_vector(16 downto 0);
    RAM0_WE         : out   std_logic;
    RAM1_WE         : out   std_logic;
    RAM0_OE         : out   std_logic;
    RAM1_OE         : out   std_logic;
    RAM_CS          : out   std_logic
);
end RAMIF;


architecture RAMIF_Arch of RAMIF is
begin

    RAM_ADDR <= ADDR(16 downto 0);
    RAM_CS <= not CS;


    DATA_TO_RAM: process( DATAI, ADDR, WE )
    begin
        if WE = '1' then
            if ADDR(17) = '0' then
                RAM0_DATA <= DATAI;
                RAM1_DATA <= (others => 'Z');
            else
                RAM1_DATA <= DATAI;
                RAM0_DATA <= (others => 'Z');
            end if;
        else
            RAM0_DATA <= (others => 'Z');
            RAM1_DATA <= (others => 'Z');
        end if;
    end process;


    DATA_FROM_RAM: process( RAM0_DATA, RAM1_DATA, OE, ADDR )
    begin
        if OE = '1' then
            if ADDR(17) = '0' then
                DATAO <= RAM0_DATA;
            else
                DATAO <= RAM1_DATA;
            end if;
        else
            DATAO <= (others => '0' );
        end if;
    end process;


    WE_TO_RAM: process( ADDR, WE )
    begin
        if ADDR(17) = '0' then
            RAM0_WE <= not WE;
            RAM1_WE <= '1';
        else
            RAM1_WE <= not WE;
            RAM0_WE <= '1';
        end if;
    end process;


    OE_TO_RAM: process( ADDR, OE )
    begin
        if ADDR(17) = '0' then
            RAM0_OE <= not OE;
            RAM1_OE <= '1';
        else
            RAM1_OE <= not OE;
            RAM0_OE <= '1';
        end if;
    end process;

end RAMIF_Arch;



