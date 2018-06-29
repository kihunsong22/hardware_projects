--
-- RamControl.vhd
--
-- This file describes the External SRAM Controller for the Video Multiplexer.
--
--     Author: Peter Allworth (Linear Solutions Pty Ltd)
--

library IEEE;
use IEEE.std_logic_1164.all;

entity RamControl is 
    port (ck, rst : in std_logic; -- clock and reset inputs
          rd_ack, wr_ack : in std_logic; -- DMA transfer acknowledge
          rd_addr : in std_logic_vector(16 downto 0); -- address for reads
          wr_addr : in std_logic_vector(16 downto 0); -- address for writes
          wr_data : in std_logic_vector(7 downto 0); -- data for writes
          rd_data : out std_logic_vector(7 downto 0); -- data from reads
          rd_req, wr_req : out std_logic; -- DMA transfer request
          ram_data : inout std_logic_vector(7 downto 0); -- SRAM data bus
          ram_addr : out std_logic_vector(16 downto 0); -- SRAM address bus
          ram_we_bar : out std_logic; -- write enable for external SRAM
          ram_oe_bar : out std_logic; -- output enable for external SRAM
          ram_cs_bar : out std_logic); -- chip select for external SRAM
end RamControl;

architecture behaviour of RamControl is
    type state_value is (rd_setup, rd_xfer, wr_setup, wr_xfer);
    signal state : state_value;
    signal wr_acked, writing : std_logic;
    signal waddr : std_logic_vector(16 downto 0); -- latched wr_addr
    signal wdata : std_logic_vector(7 downto 0); -- latched wr_data
begin
    -- read/write interleaver (each SRAM read or write takes 2 cycles)
    process
    begin
        wait until ck'event and ck = '1';
        if rst = '1' then
            state <= rd_setup;
            rd_req <= '0';
            wr_req <= '0';
            wr_acked <= '0';
            ram_we_bar <= '1';
        else
            case state is
                when rd_setup =>
                    rd_req <= '1';
                    wr_req <= '1';
                    state <= rd_xfer;
                when rd_xfer =>
                    rd_req <= '0';
                    wr_req <= '0';
                    waddr <= wr_addr;
                    wdata <= wr_data;
                    wr_acked <= wr_ack;
                    state <= wr_setup;
                when wr_setup =>
                    ram_we_bar <= not wr_acked;
                    state <= wr_xfer;
                when wr_xfer =>
                    ram_we_bar <= '1';
                    state <= rd_setup;
                when others =>
                    state <= rd_setup;
            end case;
        end if;
    end process;
    -- latched multiplexer to drive the external address bus
    process
    begin
        -- change the address on the falling edge of the clock
        wait until ck'event and ck = '0';
        if rst = '1' then
            ram_addr <= (others => '0');
            writing <= '0';
        else
            case state is
                when rd_setup =>
                    ram_addr <= rd_addr;
                    writing <= '0';
                when wr_setup =>
                    ram_addr <= waddr;
                    writing <= '1';
                when others =>
            end case;
        end if;
    end process;
    -- tristate buffer to drive the external data bus
    process (wdata, writing)
    begin
        if writing = '1' then
            ram_data <= wdata;
        else
            ram_data <= (others => 'Z');
        end if;
    end process;
    ram_cs_bar <= '0' ;
    ram_oe_bar <= not rd_ack;
    rd_data <= ram_data;
end behaviour;

