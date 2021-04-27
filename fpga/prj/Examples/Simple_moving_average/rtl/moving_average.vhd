----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 06.03.2021 17:18:22
-- Design Name: 
-- Module Name: moving_average - Behavioral
-- Project Name: 
-- Target Devices: 
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
use IEEE.NUMERIC_STD.all;

entity moving_average is
    Port ( data_i   : in std_logic_vector (13 downto 0);    -- adc input data
           clk_i    : in std_logic;                         -- bus clock 
           rstn_i   : in std_logic;                         -- bus reset - active low
           tag_i    : in unsigned (1 downto 0);             -- filter window size
           data_o   : out std_logic_vector (13 downto 0));  -- filtered data
end moving_average;

architecture Behavioral of moving_average is
    type mem_t is array (0 to 2) of signed (13 downto 0);
    
    signal regs: mem_t; -- buffer for moving average algorithm
    signal sum: signed(13 downto 0); -- register for storing the sum of register values
begin

regs(0) <= signed(data_i);

process (clk_i)
begin
    if(rising_edge(clk_i)) then
        if (rstn_i = '0') then
            sum <= "00000000000000";
        else            
            case tag_i is
                -- regs
                when "01" => data_o <= std_logic_vector(sum);
                
                -- regs / 2
                when "10" => data_o <= std_logic_vector(shift_right(sum, 1));
                
                -- (regs * 85) / 256
                when "11" => data_o <= std_logic_vector(resize(shift_right(sum * 85, 8), 14));
                     
                -- (regs * 85) / 256
                when others => data_o <= std_logic_vector(resize(shift_right(sum * 85, 8), 14));
            end case;
            
            if (tag_i(1) = '1') then
                regs(1) <= regs(0);
            else
                regs(1) <= "00000000000000";
            end if;
            
            if (tag_i(0) = '1') then
                regs(2) <= regs(1);
            else
                regs(2) <= "00000000000000";
            end if;
                        
            sum <= regs(0) + regs(1) + regs(2);
        end if;
    end if;
end process;

end Behavioral;
