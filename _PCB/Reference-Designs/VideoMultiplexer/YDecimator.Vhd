--
-- YDecimator.vhd
--
--  This file describes the vertical (Y) decimator for the Video Multiplexer.
--  It vertically sums groups of four 180-pixel lines and outputs the results.
--  This reduces a 288 (244) line image to 72 (61) lines for picture-in-picture.
--  The implementation assumes that the input timing is that of the output
--  of the XDecimator, ie. "ien" is high for 4 consecutive clocks out of 16.
--  Each output byte is held valid for 4 consecutive clocks to allow the
--  RAM controller time to write the data to external SRAM.
--
--     Author: Peter Allworth (Linear Solutions Pty Ltd)
--

library IEEE;
use IEEE.std_logic_1164.all, IEEE.std_logic_arith.all;

entity YDecimator is
    port (ck, rst : in std_logic; -- clock (27MHz) and reset inputs
          v : in std_logic;       -- BT656 vertical blanking flag
          ien : in std_logic;     -- input enable, indicates vpi valid
          vpi : in std_logic_vector(9 downto 0); -- decimated video data
          mem_di : in std_logic_vector(11 downto 0); -- memory data in
          mem_do : out std_logic_vector(11 downto 0); -- memory data out
          mem_addr : out std_logic_vector(8 downto 0); -- memory address
          mem_rd : out std_logic; -- active high memory read enable
          mem_wr : out std_logic; -- active high memory write enable
          vpo : out std_logic_vector(7 downto 0); -- BT656 active video data
          valid : out std_logic); -- high while vpo data is valid
end YDecimator;

architecture behaviour of YDecimator is
    subtype accumulator is unsigned(11 downto 0);
    subtype xcounter is unsigned(8 downto 0);
    subtype ycounter is unsigned(2 downto 0);
    type state_value is (GetCb0, GetY0, GetCr0, GetY1,
                         AccCb0, AccY0, AccCr0, AccY1,
                         PutCb0a, PutCb0b, PutY0a, PutY0b,
                         PutCr0a, PutCr0b, PutY1a, PutY1b);
    signal state : state_value;        -- state of the data stream
    signal xpos : xcounter;            -- current x position in BT656 samples
    signal ypos : ycounter;            -- current line number modulo 4
    signal cb_acc, cr_acc : accumulator;       -- chrominance totals
    signal y0_acc, y1_acc : accumulator;       -- luminance totals
    signal cb_out, y0_out, cr_out, y1_out : std_logic_vector(7 downto 0);
    signal sending : std_logic;        -- high while output data is valid
begin
    -- subcircuit to accept XDecimator output and perform Y decimation
    receiver: process
       variable x_next : xcounter;
       variable y_next : ycounter;
       variable sample, total : accumulator;
    begin
       wait until ck'event and ck = '1';
       if (rst = '1') or (v = '1') then
           state <= GetCb0;
           mem_rd <= '0';
           mem_wr <= '0';
           mem_do <= (others => '0');
           xpos <= (others => '0');
           ypos <= (others => '0');
       else
           x_next := xpos + '1';
           y_next := ypos + '1';
           -- sign-extend the input words to 12 bits
           sample := conv_unsigned(unsigned(vpi), 12);
           if (ypos = 0) then
               -- initialise total for the first in a group of four lines
               -- bit 3 corresponds to 1/2 LSB of the final, 8-bit result so
               -- setting it here means the vpo output is correctly rounded
               total := (3 => '1', others => '0');
           else
               -- otherwise retrieve the previously saved value
               total := unsigned(mem_di);
           end if;
           case state is
               when GetCb0 =>
                   if ien = '1' then
                       cb_acc <= sample;
                       state <= GetY0;
                   else
                       state <= GetCb0;
                   end if;
               when GetY0 =>
                   y0_acc <= sample;
                   state <= GetCr0;
               when GetCr0 =>
                   cr_acc <= sample;
                   mem_rd <= '1';
                   state <= GetY1;
               when GetY1 =>
                   y1_acc <= sample;
                   xpos <= x_next;
                   state <= AccCb0;
               when AccCb0 =>
                   cb_acc <= cb_acc + total;
                   xpos <= x_next;
                   state <= AccY0;
               when AccY0 =>
                   y0_acc <= y0_acc + total;
                   xpos <= x_next;
                   state <= AccCr0;
               when AccCr0 =>
                   cr_acc <= cr_acc + total;
                   -- reset pointer for writing back the sums
                   xpos <= xpos(8 downto 2) & "00";
                   mem_rd <= '0';
                   mem_wr <= '1';
                   mem_do <= std_logic_vector(cb_acc);
                   state <= AccY1;
               when AccY1 =>
                   y1_acc <= y1_acc + total;
                   cb_out <= std_logic_vector(cb_acc(11 downto 4));
                   state <= PutCb0a;
               when PutCb0a =>
                   xpos <= x_next;
                   mem_do <= std_logic_vector(y0_acc);
                   state <= PutCb0b;
               when PutCb0b =>
                   y0_out <= std_logic_vector(y0_acc(11 downto 4));
                   state <= PutY0a;
               when PutY0a =>
                   xpos <= x_next;
                   mem_do <= std_logic_vector(cr_acc);
                   state <= PutY0b;
               when PutY0b =>
                   cr_out <= std_logic_vector(cr_acc(11 downto 4));
                   state <= PutCr0a;
               when PutCr0a =>
                   xpos <= x_next;
                   mem_do <= std_logic_vector(y1_acc);
                   state <= PutCr0b;
               when PutCr0b =>
                   y1_out <= std_logic_vector(y1_acc(11 downto 4));
                   state <= PutY1a;
               when PutY1a =>
                   mem_wr <= '0';
                   state <= PutY1b;
               when PutY1b =>
                   if x_next /= 360 then
                       -- xpos runs from 0 to 359
                       xpos <= x_next;
                   elsif y_next /= 4 then
                       -- ypos runs from 0 to 3
                       ypos <= y_next;
                       xpos <= (others => '0');
                   else
                       -- start of a new group of lines
                       ypos <= (others => '0');
                       xpos <= (others => '0');
                   end if;
                   state <= GetCb0;
               when others =>
                   state <= GetCb0;
           end case;
       end if;
    end process;
    -- subcircuit to sequence the output values for the DMA & RAM controllers
    sender : process
        variable count : unsigned(3 downto 0); -- clock cycle counter
    begin
        wait until ck'event and ck = '1';
        if rst = '1' then
            sending <= '0';
            count := conv_unsigned(0, count'length);
        else
            if (state = PutY1b) and (ypos = 3) then
                -- last line in a group of four
                sending <= '1';
                count := conv_unsigned(0, count'length);
                vpo <= cb_out;
            elsif sending = '1' then
                case conv_integer(count) is
                    when 3 =>
                        vpo <= y0_out;
                    when 7 =>
                        vpo <= cr_out;
                    when 11 =>
                        vpo <= y1_out;
                    when 15 =>
                        sending <= '0';
                    when others =>
                end case;
                count := count + 1;
            end if;
        end if;
    end process;
    mem_addr <= std_logic_vector(xpos);
    valid <= sending;
end;
