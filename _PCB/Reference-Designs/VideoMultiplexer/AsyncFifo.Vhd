--
-- AsyncFifo.vhd
--
-- This file describes an Asynchronous Fifo Controller for
-- the Video Multiplexer.
-- Memory for the FIFO is supplied externally to this block.
-- This version assumes the memory is 256 words (128 pixels) deep.
-- The FIFO is designed specifically for use in the BT656 data synchronizer.
-- Data is input while the wr input is high. Data is not made available for
-- output until the FIFO has prefilled to halfway (64 pixels). At this
-- point, the ready output goes high and stays high for 1440 + 128 clocks
-- which is time for 720 pixels plus the contents of the FIFO to drain.
-- This setup means the relative speed of the two clocks can be 1.00 +/- 9%.
--
--     Author: Peter Allworth (Linear Solutions Pty Ltd)
--

library IEEE;
use IEEE.std_logic_1164.all, IEEE.std_logic_arith.all;

entity AsyncFifo is
    port (ckr, ckw, rst: in std_logic; -- read/write clocks and reset inputs
          rd, wr : in std_logic; -- enable a read from or write to the FIFO
          di : in std_logic_vector(7 downto 0); -- input data to FIFO
          do : out std_logic_vector(7 downto 0); -- output data from FIFO
          ram_di : in std_logic_vector(7 downto 0); -- data read from ram
          ram_do : out std_logic_vector(7 downto 0); -- data to write to ram
          ready : out std_logic; -- high while FIFO is read to be read
          valid : out std_logic; -- high when the output data is valid
          ram_we : out std_logic; -- write enable for external RAM
          rd_addr, wr_addr : out std_logic_vector(7 downto 0));
end AsyncFifo;

architecture behaviour of AsyncFifo is
    subtype counter is unsigned(11 downto 0);
    subtype pointer is unsigned(7 downto 0);
    type state_value is (start, filling, running);
    signal state : state_value;      -- status of the FIFO
    signal head, tail : pointer;     -- data pointers
    signal count : counter;          -- clock count for state machine
    signal reading : std_logic;      -- goes high after rising edge of rd
begin
    -- the reader process
    process
    begin
       wait until ckr'event and ckr = '1';
       do <= ram_di;         -- register ram data to ease timing constraint
       if rd = '1' then
           head <= head + '1'; -- implicitly mod 256 as it's 8-bit
           reading <= '1';
       else
           reading <= '0';
           head <= (others => '0');
       end if;
       valid <= reading;
    end process;

    -- the writer process handles the state machine
    process
        variable count_next : counter;
        variable tail_next : pointer;
    begin
       wait until ckw'event and ckw = '1';
       if rst = '1' then
           state <= start;
       else
           count_next := count + '1';
           tail_next := tail + '1';
           ram_do <= di;
           case state is
               when start =>
                   count <= (others => '0');
                   tail <= (others => '0');
                   if (wr = '1') then
                       ram_we <= '1';
                       state <= filling;
                   else
                       ready <= '0';
                       ram_we <= '0';
                       state <= start;
                   end if;
               when filling =>
                   count <= count_next;
                   tail <= tail_next;
                   if (count_next = 128) then
                       ready <= '1';
                       state <= running;
                   else
                       state <= filling;
                   end if;
               when running =>
                   if (count_next = 1568) then
                       state <= start;
                   else
                       count <= count_next;
                       if count_next = 1440 then
                           ram_we <= '0';
                       end if;
                       tail <= tail_next;
                       state <= running;
                   end if;
               when others =>
                   state <= start;
           end case;
       end if;
    end process;
    rd_addr <= std_logic_vector(head);
    wr_addr <= std_logic_vector(tail);
end;
