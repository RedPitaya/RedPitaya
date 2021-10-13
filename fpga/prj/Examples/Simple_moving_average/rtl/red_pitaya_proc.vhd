library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity red_pitaya_proc is  
  port (
    clk_i   : in  std_logic;                      -- bus clock 
    rstn_i  : in  std_logic;                      -- bus reset - active low
    addr_i  : in  std_logic_vector(31 downto 0);  -- bus address
    wdata_i : in  std_logic_vector(31 downto 0);  -- bus write data          
    wen_i   : in  std_logic;                      -- bus write enable
    ren_i   : in  std_logic;                      -- bus read enable
    rdata_o : out std_logic_vector(31 downto 0);  -- bus read data
    err_o   : out std_logic;                      -- bus error indicator
    ack_o   : out std_logic;                      -- bus acknowledge signal

    adc_i : in  std_logic_vector(13 downto 0);
    adc_o : out std_logic_vector(13 downto 0)
    );
end red_pitaya_proc;

architecture Behavioral of red_pitaya_proc is
    component moving_average
        port ( 
            data_i   : in std_logic_vector (13 downto 0);
            clk_i    : in std_logic;
            rstn_i   : in std_logic;                    
            tag_i    : in unsigned (1 downto 0);
            data_o   : out std_logic_vector (13 downto 0));
    end component;
  
<<<<<<< HEAD
    signal tag_i: unsigned(1 downto 0) := "01";
begin

=======
begin


>>>>>>> dev-250-12
pbusr: process(clk_i)
begin
    if(rising_edge(clk_i)) then
      if (wen_i or ren_i)='1' then
        ack_o <= '1';
      end if;   
      
      if (rstn_i = '0') then
        tag_i <= "01";
      else
        case addr_i(19 downto 0) is
            when X"00000" => rdata_o <= X"00000001";
<<<<<<< HEAD
            when X"00008" => tag_i <= unsigned(wdata_i(1 downto 0));
=======
            
>>>>>>> dev-250-12
            when others => rdata_o <= X"00000000";
        end case;
      end if;
    end if;
end process;

<<<<<<< HEAD
rp_average:
    moving_average
        port map (
            data_i => adc_i,
            clk_i => clk_i,
            rstn_i => rstn_i,
            tag_i => tag_i,
            data_o => adc_o
        );
        
=======
>>>>>>> dev-250-12
end Behavioral;
