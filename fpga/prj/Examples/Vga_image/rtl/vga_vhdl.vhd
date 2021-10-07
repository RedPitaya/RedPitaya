library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;

entity VGA is
  Port (clk50: in STD_LOGIC;
        hst: out unsigned(10 downto 0);
        vst: out unsigned(9 downto 0);
        hsync: out STD_LOGIC;
        vsync: out STD_LOGIC);
end VGA;

architecture Behavioral of VGA is
--monitor period
constant H: integer := 1040;
constant V: integer := 666;

--pulse start
constant Hf: integer := 856;
constant Vf: integer := 637;

--pulse duration
constant Hs: integer := 120;
constant Vs: integer := 6;

signal hst_sig: unsigned(10 downto 0);
signal vst_sig: unsigned(9 downto 0);

begin

P1: process (clk50)
begin
    if rising_edge(clk50) then 
        if hst_sig < (H-1) then
            hst_sig <= hst_sig +1;           
        else
            hst_sig <= (others => '0');
            if vst_sig < V-1 then
                vst_sig <= vst_sig +1;
            else 
                vst_sig <= (others => '0');
            end if;
        end if;
    end if;
end process;

hst <= hst_sig;
vst <= vst_sig;

--signals to synhronize the screen
hsync <= '1' when hst_sig >= Hf and hst_sig < Hf + Hs else '0';
vsync <= '1' when vst_sig >= Vf and vst_sig < Vf + Vs else '0';

end Behavioral;