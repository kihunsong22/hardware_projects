--
-- PipControl.vhd
--
-- This file describes the Picture-in-Picture controller
-- for the Video Multiplexer.
-- Within a field, active pixels are numbered (x, y)
-- from upper left (1, 1) to lower right (720, 288).
--
--     Author: Peter Allworth (Linear Solutions Pty Ltd)
--

library IEEE;
use IEEE.std_logic_1164.all, IEEE.std_logic_arith.all;

entity PipControl is
    port (ck, rst : in std_logic; -- clock (27MHz) and reset inputs
          f, v, h : in std_logic; -- BT656 field and blanking flags
          pip_en : in std_logic; -- enable picture-in-picture
          reg_we : in std_logic;  -- enable for register writes
          reg_addr : in std_logic_vector(3 downto 0); -- register address
          reg_data : in std_logic_vector(7 downto 0); -- register data
          alt_img : out std_logic; -- when high, use alternate image
          field_start : out std_logic); -- pulses high at start of field
end PipControl;

architecture behaviour of PipControl is
    subtype pixel_offset is unsigned(9 downto 0);
    type byte_array is array(0 to 7) of std_logic_vector(7 downto 0);
    type state_value is (start, blanked_video, active_video);
    signal reg : byte_array;                  -- register file
    signal index : natural range reg'low to reg'high;
    signal xul, yul, xlr, ylr : pixel_offset; -- bounding box of PIP
    signal x, y : pixel_offset;               -- current pixel
    signal state : state_value;               -- video state
    signal x_win, y_win : std_logic;          -- high when x, y are in window
    signal c_word : std_logic;                -- high during chrominance word
begin
    index <= conv_integer(unsigned(reg_addr(2 downto 0))); -- MSB not used
    process
       variable x_next, y_next : pixel_offset;
       variable tempR10a, tempR10b, tempR10c, tempR10d : std_logic_vector(9 downto 0);
    begin
       wait until ck'event and ck = '1';
       if rst = '1' then
          state <= start;
          x_win <= '0';
          y_win <= '0';
          field_start <= '0';
       else
          x_next := x + '1';
          y_next := y + '1';
          case state is
               when start =>
                    if v = '0' then  -- falling edge of V (start of field)
                        x <= (others => '0');
                        y <= (others => '0');
                        tempR10a := reg(1)(1 downto 0) & reg(0);
                        xul <= unsigned(tempR10a);
                        tempR10b := reg(3)(1 downto 0) & reg(2);
                        yul <= unsigned(tempR10b);
                        tempR10c := reg(5)(1 downto 0) & reg(4);
                        xlr <= unsigned(tempR10c);
                        tempR10d := reg(7)(1 downto 0) & reg(6);
                        ylr <= unsigned(tempR10d);
                        field_start <= '1';
                        state <= blanked_video;
                    else
                        state <= start;
                    end if;
               when blanked_video =>
                    field_start <= '0';
                    c_word <= '0'; -- 1st word _after_ this one is luminance
                    if h = '0' then  -- falling edge of H means start of line
                        x <= x_next;
                        state <= active_video;
                    else
                        state <= blanked_video;
                    end if;
               when active_video =>
                    if v = '1' then -- rising edge of V (end of field)
                        state <= start;
                    elsif h = '1' then -- rising H means end of line
                        x <= (others => '0');
                        y <= y_next;
                        state <= blanked_video;
                    elsif c_word = '1' then -- first word of pixel (C,Y)
                        c_word <= '0';
                        x <= x_next;
                        state <= active_video;
                    else                    -- second word of pixel
                        c_word <= '1';
                        state <= active_video;
                    end if;
               when others =>
                    state <= start;
          end case;
          -- look for entry to or exit from window in x dimension
          if (x_win = '0') and (x_next = xul) and (c_word = '0') then
              x_win <= '1';
          elsif (x_win = '1') and (x = xlr) and (c_word = '0') then
              x_win <= '0';
          end if;
          -- look for entry to or exit from window in y dimension
          if (y_win = '0') and (y_next = yul) then
              y_win <= '1';
          elsif (y_win = '1') and (y = ylr) then
              y_win <= '0';
          end if;
          -- handle register writes
          if reg_we = '1' then
              reg(index) <= reg_data;
          end if;
       end if;
    end process;
    alt_img <= '1' when (pip_en = '1')
                        and (h = '0') and (x_win = '1')
                        and (v = '0') and (y_win = '1') else '0';
end;

