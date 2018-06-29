--
-- XDecimator.vhd
--
--  This file describes the horizontal (X) decimator for the Video Multiplexer.
--  It accumulates groups of four pixels and outputs the resulting sums.
--  This reduces a 720 pixel line to 180 pixels for picture-in-picture.
--  Because the chrominance samples are at half the spatial resolution of
--  the luminance samples, a pair of pixels (Cb,Y,Cr,Y) is output during the
--  last 4 clocks of a group of 16 clocks.
--
--     Author: Peter Allworth (Linear Solutions Pty Ltd)
--

library IEEE;
use IEEE.std_logic_1164.all, IEEE.std_logic_arith.all;

entity XDecimator is
    port (ck, rst : in std_logic; -- clock (27MHz) and reset inputs
          v, h : in std_logic; -- BT656 blanking flags
          vpi : in std_logic_vector(7 downto 0); -- BT656 active video data
          vpo : out std_logic_vector(9 downto 0); -- decimated video data
          valid : out std_logic); -- high while vpo data is valid
end XDecimator;

architecture behaviour of XDecimator is
    subtype accumulator is unsigned(9 downto 0);
    subtype pixel_offset is unsigned(9 downto 0);
    type input_state is (Cb0, Y0, Cr0, Y1, Cb1, Y2, Cr1, Y3,
                         Cb2, Y4, Cr2, Y5, Cb3, Y6, Cr3, Y7);
    signal state : input_state;            -- state of input data stream
    signal x : pixel_offset;               -- current output pixel number
    signal cb_acc, cr_acc : accumulator;   -- chrominance totals
    signal y_acc, y_tmp : accumulator;     -- luminance totals
begin
    process
       variable x_next : pixel_offset;
       variable sample, cb_sum, cr_sum, y_sum : accumulator;
       variable tempR : std_logic_vector(9 downto 0);
    begin
       wait until ck'event and ck = '1';
       if (rst = '1') or (v = '1') then
           state <= Cb0;
           valid <= '0';
           x <= (others => '0');
       else
          x_next := x + '1';
          tempR  := "00" & vpi;
          sample := unsigned(tempR);
          cb_sum := cb_acc + sample;
          cr_sum := cr_acc + sample;
          y_sum := y_acc + sample;
          case state is
               when Cb0 =>
                    valid <= '0';
                    if (h = '1') then
                        -- horizontal blanking period, reset x count
                        state <= Cb0;
                        x <= (others => '0');
                    elsif (x = 180) then
                        -- end of line so wait for rising H
                        state <= Cb0;
                    else
                        -- falling edge of H means start of line
                        state <= Y0;
                        cb_acc <= sample;
                    end if;
               when Y0 =>
                    y_acc <= sample;
                    state <= Cr0;
               when Cr0 =>
                    cr_acc <= sample;
                    state <= Y1;
               when Y1 =>
                    y_acc <= y_sum;
                    state <= Cb1;
               when Cb1 =>
                    cb_acc <= cb_sum;
                    state <= Y2;
               when Y2 =>
                    y_acc <= y_sum;
                    state <= Cr1;
               when Cr1 =>
                    cr_acc <= cr_sum;
                    state <= Y3;
               when Y3 =>
                    y_tmp <= y_sum;
                    state <= Cb2;
               when Cb2 =>
                    cb_acc <= cb_sum;
                    state <= Y4;
               when Y4 =>
                    y_acc <= sample;
                    state <= Cr2;
               when Cr2 =>
                    cr_acc <= cr_sum;
                    state <= Y5;
               when Y5 =>
                    y_acc <= y_sum;
                    state <= Cb3;
               when Cb3 =>
                    vpo <= std_logic_vector(cb_sum);
                    valid <= '1';
                    state <= Y6;
               when Y6 =>
                    y_acc <= y_sum;
                    vpo <= std_logic_vector(y_tmp);
                    x <= x_next;
                    state <= Cr3;
               when Cr3 =>
                    vpo <= std_logic_vector(cr_sum);
                    state <= Y7;
               when Y7 =>
                    vpo <= std_logic_vector(y_sum);
                    x <= x_next;
                    state <= Cb0;
               when others =>
                    state <= Cb0;
          end case;
       end if;
    end process;
end;
