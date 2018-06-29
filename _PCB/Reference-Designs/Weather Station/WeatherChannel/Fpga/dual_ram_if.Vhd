-----------------------------------------------------------------
--- Submodule dual_mem_if.vhdl (RAM interface)
-------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
Use IEEE.std_logic_unsigned.all;

entity DUAL_RAM_IF is 
port (
-- interface 0
    DATAI0         : in     std_logic_vector(7 downto 0);
    DATAO0         : out    std_logic_vector(7 downto 0);
    ADDR0          : in     std_logic_vector(17 downto 0);
    WE0            : in     std_logic;
    OE0            : in     std_logic;
    CS0            : in     std_logic;

-- interface 1
    DATAI1         : in     std_logic_vector(7 downto 0);
    DATAO1         : out    std_logic_vector(7 downto 0);
    ADDR1          : in     std_logic_vector(17 downto 0);
    WE1            : in     std_logic;
    OE1            : in     std_logic;
    CS1            : in     std_logic;
    
-- RAM interface
    RAM0_DATA      : inout  std_logic_vector(7 downto 0);
    RAM1_DATA      : inout  std_logic_vector(7 downto 0);
    RAM_ADDR       : out    std_logic_vector(16 downto 0);
    RAM0_WE        : out    std_logic;
    RAM1_WE        : out    std_logic;
    RAM0_OE        : out    std_logic;
    RAM1_OE        : out    std_logic;
    RAM_CS         : out    std_logic;
    
-- Control lines
    CLK            : in     std_logic;
    SEL            : in     std_logic
);
end DUAL_RAM_IF;


architecture RTL of DUAL_RAM_IF is

    signal   HOLD_CTRL0  : std_logic;
    signal   HOLD_CTRL1  : std_logic;
    signal   HOLD_CTRL2  : std_logic;
    signal   RAM0_WR     : std_logic;
    signal   RAM1_WR     : std_logic;

begin

    ADDR_TO_RAM: process( SEL, ADDR0, ADDR1, RAM0_WR, RAM1_WR )
    begin
       if SEL = '0' then
          if RAM0_WR = '1' and RAM1_WR = '1' then
             RAM_ADDR <= ADDR0(16 downto 0);
          end if;
       else
          if RAM0_WR = '1' and RAM1_WR = '1' then
              RAM_ADDR <= ADDR1(16 downto 0);
          end if;
       end if;
    end process;


    DATA_TO_RAM: process( SEL, DATAI0, DATAI1, ADDR0, ADDR1, WE0, WE1 )
    begin
       if SEL = '0' then
          if WE0 = '1' then
             if ADDR0(17) = '0' then
                RAM0_DATA <= DATAI0;
                RAM1_DATA <= (others => 'Z');
             else
                RAM1_DATA <= DATAI0;
                RAM0_DATA <= (others => 'Z');
             end if;
          else
             RAM0_DATA <= (others => 'Z');
             RAM1_DATA <= (others => 'Z');
          end if;
       else
          if WE1 = '1' then
             if ADDR1(17) = '0' then
                RAM0_DATA <= DATAI1;
                RAM1_DATA <= (others => 'Z');
             else
                RAM1_DATA <= DATAI1;
                RAM0_DATA <= (others => 'Z');
             end if;
          else
             RAM0_DATA <= (others => 'Z');
             RAM1_DATA <= (others => 'Z');
          end if;
       end if;
  end process;



    DATA_FROM_RAM: process( SEL, RAM0_DATA, RAM1_DATA, OE0, OE1, ADDR0, ADDR1 )
    begin
       if SEL = '0' then
          if OE0 = '1' then
             if ADDR0(17) = '0' then
                DATAO0 <= RAM0_DATA;
             else
                DATAO0 <= RAM1_DATA;
             end if;
          else
             DATAO0 <= (others => '0' );
          end if;
          DATAO1 <= (others => '0' );
       else
          if OE1 = '1' then
             if ADDR1(17) = '0' then
                DATAO1 <= RAM0_DATA;
             else
                DATAO1 <= RAM1_DATA;
             end if;
          else
             DATAO1 <= (others => '0' );
          end if;
          DATAO0 <= (others => '0' );
       end if;
    end process;


    WE_TO_RAM: process( SEL, ADDR0, ADDR1, WE0, WE1, HOLD_CTRL0, HOLD_CTRL1 )
    begin
       if SEL = '0' then
          if ADDR0(17) = '0' then
             RAM0_WR <= not WE0 or HOLD_CTRL0;
             RAM1_WR <= '1';
          else
             RAM1_WR <= not WE0 or HOLD_CTRL0;
             RAM0_WR <= '1';
          end if;
       else
          if ADDR1(17) = '0' then
             RAM0_WR <= not WE1 or HOLD_CTRL1;
             RAM1_WR <= '1';
          else
             RAM1_WR <= not WE1 or HOLD_CTRL1;
             RAM0_WR <= '1';
          end if;
       end if;
    end process;



    OE_TO_RAM: process( SEL, ADDR0, ADDR1, OE0, OE1, HOLD_CTRL0, HOLD_CTRL1 )
    begin
       if SEL = '0' then
          if ADDR0(17) = '0' then
             RAM0_OE <= not OE0 or HOLD_CTRL0;
             RAM1_OE <= '1';
          else
             RAM1_OE <= not OE0 or HOLD_CTRL0;
             RAM0_OE <= '1';
          end if;
       else
          if ADDR1(17) = '0' then
             RAM0_OE <= not OE1 or HOLD_CTRL1;
             RAM1_OE <= '1';
          else
             RAM1_OE <= not OE1 or HOLD_CTRL1;
             RAM0_OE <= '1';
          end if;
       end if;
    end process;


    CS_TO_RAM: process( SEL, ADDR0, ADDR1, CS0, CS1 )
    begin
       if SEL = '0' then
         RAM_CS <= not CS0;
       else
         RAM_CS <= not CS1;
       end if;
    end process;


    HOLD_CONTROL0: process( SEL, CLK )
    begin
       if SEL = '1' then
          HOLD_CTRL0 <= '1';
       elsif falling_edge(CLK) then
          HOLD_CTRL0 <= '0';
       end if;
    end process;


    HOLD_CONTROL1: process( SEL, CLK )
    begin
       if SEL = '0' then
          HOLD_CTRL1 <= '1';
       elsif falling_edge(CLK) then
          HOLD_CTRL1 <= '0';
       end if;
    end process;
    

    RAM0_WE <= RAM0_WR;
    RAM1_WE <= RAM1_WR;


end RTL;
    

    
