--
-- SyncFifo.vhd
--
-- This file describes a Synchronous Fifo Controller for the Video Multiplexer.
-- Memory for the FIFO is supplied externally to this block.
-- This version assumes the memory is 512 words deep.
--
--     Author: Peter Allworth (Linear Solutions Pty Ltd)
--

library IEEE;
use IEEE.std_logic_1164.all, IEEE.std_logic_arith.all;

entity SyncFifo is
    port (ck, rst: in std_logic; -- clock and reset inputs
          rd, wr : in std_logic; -- enable a read from or write to the FIFO
          empty, half, full : out std_logic; -- status of FIFO
          we_out : out std_logic; -- write enable for external RAM
          rd_addr, wr_addr : out std_logic_vector(8 downto 0));
end SyncFifo;

architecture behaviour of SyncFifo is
    signal head, head_next, tail, tail_next, count : unsigned(8 downto 0);
    signal high_tide : std_logic; -- readable copy of "full"
begin
    head_next <= head + 1; -- implicitly mod 512 as it's 9-bit
    tail_next <= tail + 1;
    process
    begin
       wait until ck'event and ck = '1';
       if rst = '1' then
          count <= (others => '0');
          head <= (others => '0');
          tail <= (others => '0');
       else
           if (rd = '1') and (wr = '1') then
              head <= head_next;
              tail <= tail_next;
           elsif (rd = '1') and (count /= 0) then
              head <= head_next;
              count <= count - 1;
           elsif (wr = '1') and (count /= 511) then
              tail <= tail_next;
              count <= count + 1;
           end if;
       end if;
    end process;
    empty <= '1' when (count = 0) else '0';
    half <= '1' when (count > 256) else '0';
    high_tide <= '1' when (count = 511) else '0';
    full <= high_tide;
    we_out <= '1' when (wr = '1') and (high_tide = '0') else '0';
    -- read address is latched by RAM so need to look ahead
    rd_addr <= std_logic_vector(head_next) when (rd = '1')
               else std_logic_vector(head);
    wr_addr <= std_logic_vector(tail);
end;
