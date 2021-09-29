library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;

entity picture is
    Port (
        clk50: in STD_LOGIC;
        hst: in unsigned(10 downto 0);
        vst: in unsigned(9 downto 0);
		data_position: in unsigned(16 downto 0);
        offset: in unsigned(15 downto 0);
        size: in unsigned(15 downto 0);
        data_in: in STD_LOGIC;
        rgb: out STD_LOGIC_VECTOR(2 downto 0));
end picture;

architecture Behavioral of picture is

type logo is array(0 to 65535) of std_logic;
signal picture_array: logo;


--screen size
constant Hp: integer := 800;
constant Vp: integer := 600;

--picture part
constant Hpicture: integer := 80;
constant Vpicture: integer := 20;

--picture size signal
signal Hp_size: unsigned(7 downto 0) := (others => '1');
signal Vp_size: unsigned(7 downto 0) := (others => '1');


signal hst_sig: unsigned(10 downto 0) := (others => '0');
signal vst_sig: unsigned(9 downto 0) := (others => '0');


signal cx_sig: UNSIGNED(9 downto 0); 
signal cy_sig: UNSIGNED (9 downto 0);

-- change picture position on screen
signal Xoff: UNSIGNED (7 downto 0) := (others => '0');
signal Yoff: UNSIGNED (7 downto 0) := (others => '0'); 

--write enable
signal wen: STD_LOGIC;

--data from array
signal data_out: STD_LOGIC;

--if we are on picture position
signal picture_en: STD_LOGIC; 

begin

hst_sig <= hst;
vst_sig <= vst;

cx_sig <= hst_sig(9 downto 0);
cy_sig <= vst_sig;

wen <= data_position(0);
Xoff <= offset(15 downto 8);
Yoff <= offset(7 downto 0);
Hp_size <= size(15 downto 8);
Vp_size <= size (7 downto 0);

--fill ROM

P1: process (clk50)
begin
   if rising_edge(clk50) then
        if  wen = '1' then
            picture_array(to_integer(data_position(16 downto 1))) <= data_in;   

        end if;
          
          if ((hst_sig >= resize(Xoff, 11)) and (hst_sig < (resize(Hp_size,11) +resize(Xoff, 11)))) and ((vst_sig >= resize(Yoff,10)) and (vst_sig < (resize(Vp_size,10) + resize(Yoff, 10)))) then
                picture_en <= '1';
          else 
                picture_en <= '0';
          end if;
     end if;
    
end process;

--picture
P3: process (clk50)
begin

--check the position on the screen, 
--only if we are on right position 
--draw picture

if rising_edge(clk50) then
     if (hst_sig < Hp) and  (vst_sig < Vp) then
         if picture_en = '1' then
             data_out <= picture_array(to_integer(resize(cx_sig - Xoff, 8) & resize(cy_sig - Yoff, 8)));   
                 if data_out = '1' then
                   rgb <= "111";
                 else
                   rgb <= "000"; 
                end if;
             
          else
             rgb <= "010";
          end if;
     else
         rgb <= "000";
     end if;
end if;   
end process;
end Behavioral;  