--
-- BT656Decoder.vhd
--
-- This file describes an ITU-BT656 timing reference decoder
-- for the Video Multiplexer. SAV and EAV codes are detected
-- within the input ITU-BT656 format data stream and the
-- F, V and H bits are extracted and output.
--
--     Author: Peter Allworth (Linear Solutions Pty Ltd)
--

library IEEE;
use IEEE.std_logic_1164.all;

entity BT656Decoder is
    port (ck, rst: in std_logic; -- clock and reset inputs
          data : in std_logic_vector(7 downto 0); -- BT656 data stream
          f, v, h : out std_logic); -- field id, v and h blanking state
end BT656Decoder;

architecture behaviour of BT656Decoder is
    type state_value is (start, found_ff, first_00, second_00);
    signal state : state_value;
begin
    process
    begin
       wait until ck'event and ck = '1';
       if rst = '1' then
           state <= start;
           f <= '0';
           v <= '0';
           h <= '0';
       else
           case state is
                when start =>
                     if data = "11111111" then
                         state <= found_ff;
                     else
                         state <= start;
                     end if;
                when found_ff =>
                     if data = "00000000" then
                         state <= first_00;
                     elsif data = "11111111" then
                         state <= found_ff;
                     else
                         state <= start;
                     end if;
                when first_00 =>
                     if data = "00000000" then
                         state <= second_00;
                     elsif data = "11111111" then
                         state <= found_ff;
                     else
                         state <= start;
                     end if;
                when second_00 =>
                     if data(7) = '1' then
                         f <= data(6);
                         v <= data(5);
                         h <= data(4);
                     end if;
                     state <= start;
                when others =>
                     state <= start;
           end case;
       end if;
    end process;
end;
