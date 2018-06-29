-------------------------------------------------------
--- Submodule vgaif.vhdl (VGA)
-------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
Use IEEE.std_logic_unsigned.all;

entity VGAIF is 
port
(
    CLK         : in    std_logic;
    ALE         : in    std_logic;
    RST         : in    std_logic;

    VGA_RD      : in    std_logic;
    VGA_DATA    : out   std_logic_vector(7 downto 0);
    VGA_PXLADDR : in    std_logic_vector(18 downto 0);

    RAM_ADDR    : out   std_logic_vector(17 downto 0);
    RAM_DI      : in    std_logic_vector(7 downto 0);
    RAM_DO      : out   std_logic_vector(7 downto 0);
    RAM_RD      : out   std_logic;
    RAM_WR      : out   std_logic;
    RAM_EN      : out   std_logic;

    SFR_ADDR    : in    std_logic_vector(6 downto 0);
    SFR_DATAI   : in    std_logic_vector(7 downto 0);
    SFR_DATAO   : out   std_logic_vector(7 downto 0);
    SFR_DATAL   : in    std_logic_vector(7 downto 0);
    SFR_WR      : in    std_logic;
    SFR_RD      : in    std_logic;
    
    C0_DATAO    : out   std_logic_vector(7 downto 0);
    C0_DATAI    : in    std_logic_vector(7 downto 0);
    C0_ADDR     : out   std_logic_vector(8 downto 0);
    C0_WR       : out   std_logic;
    
    C1_DATAO    : out   std_logic_vector(7 downto 0);
    C1_DATAI    : in    std_logic_vector(7 downto 0);
    C1_ADDR     : out   std_logic_vector(8 downto 0);
    C1_WR       : out   std_logic

) ;
end VGAIF;

architecture RTL of VGAIF is

    signal CLK2             : std_logic;
    signal VGA_CACHE        : std_logic_vector(7 downto 0);
    signal RST_PXL          : std_logic;
    signal PXL              : std_logic_vector(8 downto 0);
    signal LN               : std_logic_vector(8 downto 0);
    signal MEM_READ_ADDR    : std_logic_vector(16 downto 0);
    signal MEM_WRITE_ADDR   : std_logic_vector(16 downto 0);
    signal MEM_DATA         : std_logic_vector(7 downto 0);
    signal WRITE_RAM        : std_logic;
    signal READ_RAM         : std_logic;
    signal CACHE_FULL       : std_logic;
    signal CACHE_REQUEST    : std_logic;
    signal VGA_CMD          : std_logic_vector(7 downto 0) := "00000000";           -- VGA command
    signal RST_CMD          : std_logic_vector(7 downto 0) := "00000000";           -- Reset VGA command
    signal READ_PXL         : std_logic := '0';
    signal PXL_VALID        : std_logic;
    signal WRITE_DATA       : std_logic;
    signal WRITE_DONE       : std_logic;
    signal INC_X            : std_logic_vector(1 downto 0) := "00";
    signal X_RDY            : std_logic;
    signal INC_Y            : std_logic_vector(1 downto 0) := "00";
    signal Y_RDY            : std_logic;

    signal XPOS             : std_logic_vector(15 downto 0) := "0000000000000000";  -- X-Position low byte
    signal YPOS             : std_logic_vector(15 downto 0) := "0000000000000000";  -- Y-Position low byte
    signal COLOR            : std_logic_vector(7 downto 0) := "00000000";           -- Pixel color
    signal COMMAND          : std_logic_vector(7 downto 0);

begin

    DIVIDE_CLK: process( CLK )
    begin
        if falling_edge( CLK ) then
            CLK2 <= not CLK2;
        end if;
    end process;

--***********
-- WRITE/READ Command in SFR
--***********
    WRITE_CMD: process( VGA_CMD, RST_CMD, CLK, RST, SFR_WR, INC_X, INC_Y )
        variable X_DONE : std_logic;
        variable Y_DONE : std_logic;
        variable STATE : std_logic;
    begin
        if RST = '1' then
            VGA_CMD <= ( others => '0');
            XPOS <= ( others => '0');
            YPOS <= ( others => '0');
            COLOR <= "00000000";
            X_DONE := '0';
            Y_DONE := '0';
            X_RDY <= '0';
            Y_RDY <= '0';
        elsif rising_edge( CLK ) then
            if RST_CMD /= "11111111" then
                VGA_CMD <= VGA_CMD and RST_CMD;
            elsif SFR_WR = '1' then
                if SFR_ADDR = "1011001" then    -- VCMD at SFR address D9h
                    VGA_CMD <= SFR_DATAI;
                elsif SFR_ADDR = "1011010" then -- X-Pos high at SFR address DAh
                    XPOS(15 downto 8) <= SFR_DATAI;
                elsif SFR_ADDR = "1011011" then -- X-Pos low at SFR address DBh
                    XPOS( 7 downto 0) <= SFR_DATAI;
                elsif SFR_ADDR = "1011100" then -- Y-Pos high at SFR address DCh
                    YPOS(15 downto 8) <= SFR_DATAI;
                elsif SFR_ADDR = "1011101" then -- Y-Pos low at SFR address DDh
                    YPOS( 7 downto 0) <= SFR_DATAI;
                elsif SFR_ADDR = "1011110" then -- Pixel color at SFR address DEh
                    COLOR <= SFR_DATAI;
                end if;
            else
                if INC_X /= 0 then
                    if X_DONE = '0' then
                        XPOS <= XPOS + INC_X;
                    end if;
                    X_DONE := '1';
                    X_RDY <= '1';
                else
                    X_RDY <= '0';
                    X_DONE := '0';
                end if;

                if INC_Y /= 0 then
                    if Y_DONE = '0' then
                        YPOS <= YPOS + INC_Y;
                    end if;
                    Y_DONE := '1';
                    Y_RDY <= '1';
                else
                    Y_RDY <= '0';
                    Y_DONE := '0';
                end if;
            end if;
        end if;
    end process;


    process( VGA_CMD, CLK2 )
    begin
        if falling_edge( CLK2 ) then
            COMMAND <= VGA_CMD;
        end if;
    end process;

    READ_SFR: process( SFR_RD, SFR_ADDR, COMMAND, SFR_DATAL )   -- DO NOT USE CLK!! read SFR as unclocked memory.
    begin
        if ( SFR_RD = '1' ) then
            if ( SFR_ADDR = "1011001" ) then    -- VCMD at SFR address D9h
                SFR_DATAO <= COMMAND;
            else
                SFR_DATAO <= SFR_DATAL;
            end if;
        else
            SFR_DATAO <= "00000000";
        end if;
    end process;



    RESET_PIXEL_CNT: process( CLK )
    begin
        if falling_edge(CLK) then
            if ( VGA_PXLADDR( 8 downto 0 ) < 4 ) then
                CACHE_REQUEST <= '1';
                RST_PXL <= '1';
            elsif ( VGA_PXLADDR( 8 downto 0 ) > 500 ) then
                CACHE_REQUEST <= '1';
                RST_PXL <= '0';
            elsif ALE = '0' then
                CACHE_REQUEST <= '0';
                RST_PXL <= '0';
            end if;
        end if;
    end process;


    PIXEL_CNT: process( CLK, RST_PXL )
    begin
        if RST_PXL = '1' then
            PXL <= (others => '0');
        elsif falling_edge( CLK ) then
            if ( ALE = '1' and PXL < 511 ) then
                PXL <= PXL + 1;
                CACHE_FULL <= '0';
            elsif ALE = '1' and PXL = 511 then
                CACHE_FULL <= '1';
            end if;
        end if;
    end process;


    process( VGA_PXLADDR, CLK )
    begin
        if VGA_PXLADDR(18 downto 9) < 249 then
            LN <= VGA_PXLADDR( 17 downto 9 ) + 1;
        else
            LN <= ( others => '0' );
        end if;
    end process;


-- Address for reading from RAM to cache
    READ_ADDRESS: process( CLK )
        variable X256 : std_logic_vector( 16 downto 0 );
        variable X64  : std_logic_vector( 16 downto 0 );
    begin
        if falling_edge( CLK ) then
            MEM_READ_ADDR(16 downto 9) <= LN(7 downto 0);
            MEM_READ_ADDR( 8 downto 0) <= PXL; 
        end if;        
    end process;


-- CACHE control-, address- and data-in lines
    ACCESS_CACHE: process( CLK, ALE, LN, VGA_PXLADDR, PXL )
    begin
        if falling_edge( CLK ) then
            if LN(0) = '0' then         -- Reading even line: Write odd cache
                C1_ADDR <= PXL;
                if ALE = '1' and CACHE_FULL = '0' then 
                    C1_WR <= '1';
                else
                    C1_WR <= '0';
                end if;

                C0_ADDR <= VGA_PXLADDR(8 downto 0);
                C0_WR <= '0';
            else                        -- Else, write even cache
                C0_ADDR <= PXL;
                if ALE = '1' and CACHE_FULL = '0' then
                    C0_WR <= '1';
                else
                    C0_WR <= '0';
                end if;

                C1_ADDR <= VGA_PXLADDR(8 downto 0);
                C1_WR <= '0';
            end if;
        end if;
    end process;
    

    RD_RAM: process ( ALE, CACHE_FULL, CLK )
    begin
        if falling_edge ( CLK ) then
            if ALE = '1' and CACHE_FULL = '0' then
                READ_RAM <= '1';
                PXL_VALID <= '0';
            elsif ALE = '1' and CACHE_FULL = '1' and READ_PXL = '1' then
                READ_RAM <= '1';
                PXL_VALID <= '1';
            else
                READ_RAM <= '0';
                PXL_VALID <= '0';
            end if;
        end if;
    end process;


    EXECUTE_CMD: process ( COMMAND, CLK2, RST )
        variable CMD_STATE  : std_logic_vector(3 downto 0) := "0000";
        variable OLD_PXL    : std_logic_vector(7 downto 0);
        variable Y_MULT256  : std_logic_vector( 16 downto 0 );
        variable Y_MULT64   : std_logic_vector( 16 downto 0 );
        variable X_DIV2     : std_logic_vector( 16 downto 0 );
        variable ADD1       : std_logic_vector( 16 downto 0 );
    begin
        if RST = '1' then
            RST_CMD <= "11111111";
            MEM_WRITE_ADDR <= ( others => '0');
            CMD_STATE := "0000";
            OLD_PXL := ( others => '0');
            Y_MULT256 := ( others => '0');
            Y_MULT64 := ( others => '0');
            X_DIV2 := ( others => '0');
            ADD1 := ( others => '0');
            INC_X <= "00";
            INC_Y <= "00";
            MEM_DATA <= ( others => '0');
            WRITE_DATA <= '0';
        elsif falling_edge( CLK2 ) then
            if COMMAND = "00000000" then         -- Clear screen
                RST_CMD <= "11111111";
                MEM_WRITE_ADDR <= ( others => '0');
                CMD_STATE := "0000";
                Y_MULT256 := ( others => '0');
            
            elsif COMMAND(3 downto 0) = "0001" then         -- Clear screen
                if CMD_STATE = "0000" then                -- Init signals
                    MEM_WRITE_ADDR <= ( others => '0');
                    MEM_DATA <= COLOR;
                    CMD_STATE := "0001";

                elsif CMD_STATE = "0001" then             -- Write pixel to mem
                    if ALE = '1' and CACHE_FULL = '1' and CACHE_REQUEST = '0' then
                         WRITE_DATA <= '1';
                         CMD_STATE := "0010";
                    end if;

                elsif CMD_STATE = "0010" then              -- Set next pixel
                    if ALE = '1' then
                        WRITE_DATA <= '0';
                        CMD_STATE := "0011";
                    end if;

                else
                    if MEM_WRITE_ADDR /= 128000 then
                        MEM_WRITE_ADDR <= MEM_WRITE_ADDR + 1;
                        MEM_DATA <= COLOR;
                        CMD_STATE := "0001";
                    else
                         RST_CMD <= "00000000";
                         CMD_STATE := "0000";
                    end if;
                end if;

            elsif COMMAND(3 downto 0) = "0010" then     -- Set Pixel
                if CMD_STATE = "0000" then
                    Y_MULT256(16 downto 8) := YPOS(8 downto 0);
                    Y_MULT256(7 downto 0) := ( others => '0' );
                    Y_MULT64(16 downto 15) := ( others => '0' );
                    Y_MULT64(14 downto 6) := YPOS(8 downto 0);
                    Y_MULT64(5  downto 0) := ( others => '0' );
                    CMD_STATE := "0001";

                elsif CMD_STATE = "0001" then
                    ADD1 := Y_MULT256 + Y_MULT64;
                    X_DIV2(16 downto 9) := ( others => '0' );
                    X_DIV2(8 downto 0) := XPOS(9 downto 1);
                    CMD_STATE := "0010";

                elsif CMD_STATE = "0010" then
                    MEM_WRITE_ADDR <= ADD1 + X_DIV2;
                    CMD_STATE := "0011";

                elsif CMD_STATE = "0011" then
                    if ALE = '1' and CACHE_FULL = '1' and CACHE_REQUEST = '0' then
                        -- read pixels
                        READ_PXL <= '1';
                        CMD_STATE := "0100";
                    end if;

                elsif CMD_STATE = "0100" then
                    if ALE = '1' then
                        if PXL_VALID = '1' then
                            OLD_PXL := RAM_DI;
                            CMD_STATE := "0101";   -- Goto next state
                        end if;
                    end if;

                elsif CMD_STATE = "0101" then
                   READ_PXL <= '0';
                    if ALE = '0' then
                        -- get new pixel value
                        if XPOS(0) = '0' then
                            -- put in odd pixel nibble
                            MEM_DATA(7 downto 4) <= OLD_PXL(7 downto 4);
                            MEM_DATA(3 downto 0) <= COLOR(3 downto 0);
                        else
                            -- put in even pixel nibble
                            MEM_DATA(7 downto 4) <= COLOR(3 downto 0);
                            MEM_DATA(3 downto 0) <= OLD_PXL(3 downto 0);
                        end if;
                        CMD_STATE := "0110";
                    end if;

                elsif CMD_STATE = "0110" then
                    if ALE = '1' and CACHE_FULL = '1' and CACHE_REQUEST = '0' then
                        -- write new pixel value to memory
                        WRITE_DATA <= '1';
                        CMD_STATE := "0111";

                    end if;

                elsif CMD_STATE = "0111" then
                    if ALE = '1' then
                        -- write new pixel value to memory
                        WRITE_DATA <= '0';
                        CMD_STATE := "1000";
                    end if;

                else
                    RST_CMD <= "11110000";
                    CMD_STATE := "0000";
                end if;
            
            elsif COMMAND(3 downto 0) = "0011" then     -- Set  Two Pixels
                if CMD_STATE = "0000" then
                    Y_MULT256(16 downto 8) := YPOS(8 downto 0);
                    Y_MULT256(7 downto 0) := ( others => '0' );
                    Y_MULT64(16 downto 15) := ( others => '0' );
                    Y_MULT64(14 downto 6) := YPOS(8 downto 0);
                    Y_MULT64(5  downto 0) := ( others => '0' );
                    X_DIV2(16 downto 9) := ( others => '0' );
                    X_DIV2(8 downto 0) := XPOS(9 downto 1);
                    CMD_STATE := "0001";

                elsif CMD_STATE = "0001" then
                    ADD1 := Y_MULT256 + Y_MULT64;
                    CMD_STATE := "0010";

                elsif CMD_STATE = "0010" then
                    MEM_WRITE_ADDR <= ADD1 + X_DIV2;
                    CMD_STATE := "0011";
                    
                elsif CMD_STATE = "0011" then
--                    if ALE = '0' then
                        MEM_DATA <= COLOR;
                        CMD_STATE := "0100";
--                    end if;

                elsif CMD_STATE = "0100" then
                    if ALE = '1' and CACHE_FULL = '1' and CACHE_REQUEST = '0' then
                        WRITE_DATA <= '1';
                        CMD_STATE := "0101";
                    end if;
                    
                elsif CMD_STATE = "0101" then
                    if ALE = '1' then
                        WRITE_DATA <= '0';
                        CMD_STATE := "0110";
                    end if;
                    
                else
                    RST_CMD <= "11110000";
                    CMD_STATE := "0000";
                end if;
            







            elsif COMMAND(7 downto 6) /= 0 then     -- Increment X-position
                if CMD_STATE = "0000" then
                    INC_X <= COMMAND(7 downto 6);
                    RST_CMD <= "11111111";
                    CMD_STATE := "0001";
                
                elsif CMD_STATE = "0001" then
                    if X_RDY = '1' then 
                        INC_X <= "00";
                        CMD_STATE := "0010";
                    end if;

                elsif CMD_STATE = "0010" then
                    RST_CMD <= "00111111";
                    CMD_STATE := "0000";
                end if;
                    
            elsif COMMAND(5 downto 4) /= 0 then     -- Increment Y-position
                if CMD_STATE = "0000" then
--                    INC_Y <= COMMAND(5 downto 4);
                    RST_CMD <= "11111111";
                    CMD_STATE := "0001";
                
                elsif CMD_STATE = "0001" then
                    INC_Y <= COMMAND(5 downto 4);
--                    RST_CMD <= "11111111";
                    CMD_STATE := "0010";
                
                elsif CMD_STATE = "0010" then
                    if Y_RDY = '1' then 
                        INC_Y <= "00";
                        CMD_STATE := "0011";
                    end if;

                elsif CMD_STATE = "0011" then
                    RST_CMD <= "11001111";
                    CMD_STATE := "0000";
                end if;
                    
            else
                RST_CMD <= "00000000";
            end if;
        end if;
--        MEM_WRITE_ADDR <= PXL_ADDR;
    end process;
            

-- CACHE Data in
    process( VGA_PXLADDR, C0_DATAI, C1_DATAI )
    begin
        if VGA_PXLADDR(9) = '1' then         -- Reading even line, read even cache
            VGA_CACHE <= C0_DATAI;
        else                        -- Else, read odd cache
            VGA_CACHE <= C1_DATAI;
        end if;
    end process;
    


-- TO CACHE 0
    C0_DATAO <= RAM_DI;


-- TO CACHE 1
    C1_DATAO <= RAM_DI;


-- TO VGA
    VGA_DATA <= VGA_CACHE when VGA_RD = '1' else "00000000";

-- TO RAM
    RAM_ADDR(16 downto 0) <= MEM_READ_ADDR when CACHE_FULL = '0' else MEM_WRITE_ADDR;
    RAM_ADDR(17) <= '1';
    
    RAM_DO <= MEM_DATA;
    
    RAM_WR <= WRITE_DATA;
    RAM_RD <= READ_RAM;
    RAM_EN <= '1';



end RTL;


