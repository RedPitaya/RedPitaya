----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 15.05.2021 13:05:18
-- Design Name: 
-- Module Name: picture - Behavioral
-- Project Name: VGA_image
-- Target Devices: Redpitaya
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity picture is
    Port ( clk50 : in STD_LOGIC;
           hst : in unsigned (10 downto 0);
           vst : in unsigned (9 downto 0);
           rgb : out STD_LOGIC_VECTOR (2 downto 0));
           
end picture;

architecture Behavioral of picture is
    type logo is array(0 to 19) of STD_LOGIC_VECTOR(0 to 79);
    signal slika: logo := (
    "00000000000000000000000000000000000000000000000000000000000000000000000000000000",
    "00000000100000000000000000000000000000000000000000000000000000000000000000000000",
    "00000001100000000000000000000000000000000000000000000000000000000000000000000000",
    "00000001000000000000000000000000000000000000000000000000000000000000000000000000",
    "00000001001000000000000000000000000000000000000000000000000000000000000000000000",
    "00000001001000000000000000000000000000100000000000000000000000000000000000000000",
    "00000001111000000000000000000000000000100000000001001111111111111111111111111111",
    "00001001111000000000000000000000000000100000000000001000000000000000000000000000",
    "00010011111001000001011001111100011111100011110001011111011111100100000101111110",
    "00011111111111000001100010000010100000100100001001001000000000010100000100000001",
    "00000000000000000001000010000010100000100100001001001000000000010100000100000001",
    "00000000000000000001000010000010100000100100001001001000001111110100000100111111",
    "01111110000000000001000011111000100000100100001001001000010000010100000101000001",
    "00111110011001100001000010000000100000100100001001001000010000010100000101000001",
    "00111100011001100001000010000000100000100100001001001000010000010100000101000001",
    "00011110000000000001000010000000100000100100001001001000010000010100000101000001",
    "00011111111111000001000001111100011111100111110001000111001111110011111100111111",
    "00011111111110000000000000000000000000000100000000000000000000000000000100000000",
    "00000000000000000000000000000000000000000100000000000000000000000000000100000000",
    "00000000000000000000000000000000000000000000000000000000000000000000000000000000");
    
    signal hst_sig: unsigned(10 downto 0) := (others => '0');
    signal vst_sig: unsigned(9 downto 0) := (others => '0');
    
    signal cx_sig: UNSIGNED(9 downto 0); 
    signal cy_sig: UNSIGNED (9 downto 0);
    
    --screen size
    constant Hp: integer := 79;
    constant Vp: integer := 19;
    constant Hslika: integer := 79;
    constant Vslika: integer := 19;
    
    --data from array
    signal data_out: STD_LOGIC;
    
begin


hst_sig <= hst;
vst_sig <= vst;

cx_sig <= hst_sig(9 downto 0);
cy_sig <= vst_sig;
    
P2: process (hst_sig, vst_sig, cx_sig, cy_sig)
begin
    if (hst_sig < Hp) and (vst_sig < Vp) then -- and en = '1' then
        if(cx_sig < Hslika) and (cy_sig < Vslika) then
            if slika(to_integer(cy_sig))( to_integer(cx_sig)) = '1' then
                rgb <= "111";
            else
                rgb <= "000"; 
            end if;
        else
            rgb <= "001";
        end if;
    else
        rgb <= "000";
    end if;
end process;

end Behavioral;
