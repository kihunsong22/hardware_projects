--
-- DMA.vhd
--
--  This file describes a DMA controller for the Video Multiplexer.
--  Five values are required to specify the DMA: xlim, ylim, xinc, yinc
--  and start_addr. These values are fetched, least significant byte first,
--  from the following registers...
--
--       xlim: low byte in reg(0), high bits in reg(1)
--       ylim: low byte in reg(2), high bits in reg(3)
--       xinc: low byte in reg(4), high bits in reg(5)
--       yinc: low byte in reg(6), high bits in reg(7)
--       start_addr: low byte reg(8), mid byte reg(9), high bits in reg(10)
--
--  Once a start pulse is received, transfers occur in a raster fashion
--  for each "location" in a 0..ylim-1 by 0..xlim-1 matrix. The initial
--  transfer address is specified by start_addr. After each transfer,
--  this address is incremented by xinc and additionally, at the end of
--  each "line", by yinc. This allows transfers to and from frame buffers
--  in interlaced or non-interlaced order where each pixel is a specified
--  number of bytes.
--
--     Author: Peter Allworth (Linear Solutions Pty Ltd)
--

library IEEE;
use IEEE.std_logic_1164.all, IEEE.std_logic_arith.all;

entity DMA is
    port (ck, rst : in std_logic; -- clock and reset inputs
          start : in std_logic; -- pulse high to begin DMA
          dma_req : in std_logic; -- suspend DMA while low
          reg_we : in std_logic;  -- enable for register writes
          reg_addr : in std_logic_vector(3 downto 0); -- register address
          reg_data : in std_logic_vector(7 downto 0); -- register data
          dma_addr : out std_logic_vector(16 downto 0); -- src/dst of DMA
          dma_ack : out std_logic; -- goes high to enable a DMA transfer
          done : out std_logic); -- goes high when final transfer is complete
end DMA;

architecture behaviour of DMA is
    subtype offset is unsigned(9 downto 0);
    type byte_array is array(0 to 15) of std_logic_vector(7 downto 0);
    signal reg : byte_array;                  -- register file
    signal index : natural range reg'low to reg'high;
    signal p : unsigned(16 downto 0);         -- DMA address pointer
    signal x, y : offset;                     -- current "location"
    signal xl, yl : offset;                   -- limits on x and y
    signal xi, yi : offset;                   -- address increments
    signal finished, ready : std_logic;
begin
    index <= conv_integer(unsigned(reg_addr));
    ready <= '1' when (finished = '0') and (dma_req = '1') else '0';
    process
       variable x_next, y_next, xinc, yinc : offset;
       variable tempR16 : std_logic_vector(16 downto 0);
       variable tempR10a, tempR10b, tempR10c, tempR10d : std_logic_vector(9 downto 0);
    begin
       wait until ck'event and ck = '1';
       if rst = '1' then
           finished <= '1';
       else
           if start = '1' then
               -- initialise from the register bank
               tempR10a := reg(1)(1 downto 0) & reg(0);
               xl <= unsigned(tempR10a);
               tempR10b := reg(3)(1 downto 0) & reg(2);
               yl <= unsigned(tempR10b);
               tempR10c := reg(5)(1 downto 0) & reg(4);
               xinc := unsigned(tempR10c);
               tempR10d := reg(7)(1 downto 0) & reg(6);
               yinc := unsigned(tempR10d);
               xi <= xinc;
               yi <= yinc + xinc;
               tempR16 := reg(10)(0) & reg(9) & reg(8);
               p <= unsigned(tempR16);
               x <= (others => '0');
               y <= (others => '0');
               finished <= '0';
           else
               x_next := x + '1';
               y_next := y + '1';
               if (ready = '1') then
                   if x_next /= xl then
                       p <= p + xi;
                       x <= x_next;
                   elsif y_next /= yl then
                       p <= p + yi;
                       y <= y_next;
                       x <= (others => '0');
                   else
                       finished <= '1';
                   end if;
               end if;
          end if;
          -- handle register writes
          if reg_we = '1' then
              reg(index) <= reg_data;
          end if;
       end if;
    end process;
    dma_addr <= std_logic_vector(p);
    dma_ack <= ready;
    done <= finished;
end;
