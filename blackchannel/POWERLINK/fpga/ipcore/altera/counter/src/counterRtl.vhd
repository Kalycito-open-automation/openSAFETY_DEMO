-------------------------------------------------------------------------------
--! @file counter.vhd
--
--! @brief A simple counter
--
--! @details This address decoder generates a select signal depending on the
--! provided base- and high-addresses by using smaller/greater logic.
--! Additionally a strob is generated if the base or high address is selected.
-------------------------------------------------------------------------------
--
--    (c) B&R, 2014
--
--    Redistribution and use in source and binary forms, with or without
--    modification, are permitted provided that the following conditions
--    are met:
--
--    1. Redistributions of source code must retain the above copyright
--       notice, this list of conditions and the following disclaimer.
--
--    2. Redistributions in binary form must reproduce the above copyright
--       notice, this list of conditions and the following disclaimer in the
--       documentation and/or other materials provided with the distribution.
--
--    3. Neither the name of B&R nor the names of its
--       contributors may be used to endorse or promote products derived
--       from this software without prior written permission. For written
--       permission, please contact office@br-automation.com
--
--    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
--    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
--    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
--    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
--    COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
--    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
--    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
--    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
--    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
--    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
--    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
--    POSSIBILITY OF SUCH DAMAGE.
--
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity counter is
    port
    (
        clk                          : in  std_logic;
        reset                        : in  std_logic;
        avs_s0_address               : in  std_logic;
        avs_s0_read                  : in  std_logic;
        avs_s0_readdata              : out std_logic_vector(31 downto 0);
        avs_s0_write                 : in  std_logic;
        avs_s0_writedata             : in  std_logic_vector(18 downto 0);
        avs_s0_waitrequest           : out std_logic
    );
end entity counter;

------------------------------------------------------------------------------
-- Architecture section
------------------------------------------------------------------------------

architecture rtl of counter is
    signal modulus : std_logic := '0';
    signal buswait_reg, buswait_next : std_logic := '0';
    signal tickslimit_reg, tickslimit_next : std_logic_vector(avs_s0_writedata'length-1 downto 0) := (others => '0');

    signal ticksCnt : integer range 0 to ((2**avs_s0_writedata'length)-1) := 0;
    signal counter : integer := 0;
begin

    process(clk)
    begin
        if clk = '1' and clk'event then
            if reset = '1' then
                ticksCnt <= 0;
            else
                if modulus = '0' and unsigned(tickslimit_reg) > 0 then
                    ticksCnt <= ticksCnt + 1;
                else
                    ticksCnt <= 0;
                end if;
            end if;
        end if;
    end process;

    modulus <= '1' when ticksCnt = unsigned(tickslimit_reg) - 1 else '0';

    process(clk)
    begin
        if clk = '1' and clk'event then
            if reset = '1' then
                counter <= 0;
            else
                if modulus = '1' and unsigned(tickslimit_reg) > 0 then
                    counter <= counter + 1;
                end if;
            end if;
        end if;
    end process;

    process(clk)
    begin
        if clk = '1' and clk'event then
            if reset = '1' then
                avs_s0_readdata <= (others => '0');
                tickslimit_reg <= (others => '0');
                buswait_reg <= '0';
            else
                avs_s0_readdata <= (others => '0');
                tickslimit_reg <= tickslimit_next;
                buswait_reg <= buswait_next;

                if avs_s0_read = '1' and avs_s0_address = '0' and buswait_next = '1' then
                     buswait_reg <= '0';
                     avs_s0_readdata <= std_logic_vector(to_unsigned(counter, avs_s0_readdata'length));
                end if;
            end if;
        end if;
    end process;

    process(avs_s0_write, avs_s0_address, buswait_reg, tickslimit_reg, avs_s0_writedata)
    begin
        tickslimit_next <= tickslimit_reg;
        buswait_next <= '1';

        if avs_s0_write = '1' and avs_s0_address = '1' and buswait_reg = '1' then
            tickslimit_next <= avs_s0_writedata;
            buswait_next <= '0';
        end if;
    end process;

    -- output logic
    avs_s0_waitrequest <= buswait_reg;

end rtl;
