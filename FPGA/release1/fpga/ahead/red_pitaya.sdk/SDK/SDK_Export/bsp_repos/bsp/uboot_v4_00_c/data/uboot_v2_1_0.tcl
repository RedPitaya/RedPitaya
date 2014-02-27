#
# EDK BSP board generation for U-boot supporting Microblaze and PPC
#
# (C) Copyright 2007-2012 Michal Simek
# (C)		2009-2010 John Williams <john.williams@petalogix.com>
# (C)		2009-2012 PetaLogix Qld Pty Ltd
# (C)		2012 Xilinx
#
# Michal SIMEK <monstr@monstr.eu>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#
# Project description at http://www.monstr.eu/uboot/
#

# Globals variable
variable version "U-BOOT v4.00.c"

variable proctype ""

proc bsp_drc {os_handle} {
	puts "\#--------------------------------------"
	puts "\# uboot BSP DRC..."
	puts "\#--------------------------------------"
}

proc generate {os_handle} {
	generate_uboot $os_handle
}

# procedure post_generate
# This generates the drivers directory for uboot
# and runs the ltypes script

proc post_generate {lib_handle} {
}

# Return the clock frequency attribute of the port of the given ip core.
proc get_clock_frequency {ip_handle portname} {
	set clk ""
	set clkhandle [xget_hw_port_handle $ip_handle $portname]
	if {[string compare -nocase $clkhandle ""] != 0} {
		set clk [xget_hw_subproperty_value $clkhandle "CLK_FREQ_HZ"]
	}
	return $clk
}

proc generate_uboot {os_handle} {
	global proctype
	global board_name
	puts "\#--------------------------------------"
	puts "\# uboot BSP generate..."
	puts "\#--------------------------------------"

	# Open files and print GPL licence
	set config_file2 [open "config.mk" w]
	headerm $config_file2
	set config_file [open "xparameters.h" w]
	headerc $config_file

	if { [info exists board_name] } {
		puts $config_file "#define XILINX_BOARD_NAME\t\"$board_name\"\n"
	} else {
		puts $config_file "#define XILINX_BOARD_NAME\t\U-BOOT_BSP\n"
	}

	# ******************************************************************************
	# print system clock
	set proc_handle [xget_libgen_proc_handle]
	# FIXME: this does not work with Zynq but generate later
	set cpu_freq [clock_val [xget_handle $proc_handle "IPINST"]]

	# Microblaze
	set hwproc_handle [xget_handle $proc_handle "IPINST"]
	set args [xget_hw_parameter_handle $hwproc_handle "*"]
	set proctype [xget_value $hwproc_handle "OPTION" "IPNAME"]
	switch $proctype {
		"microblaze" {
			# write only name of instance
			puts $config_file "/* Microblaze is [xget_hw_parameter_value $hwproc_handle "INSTANCE"] */"

			foreach arg $args {
				set arg_name [xget_value $arg "NAME"]
				set arg_name [string map -nocase {C_ ""} $arg_name]
				set arg_value [xget_value $arg "VALUE"]
				#	puts $config_file "DEBUG $arg_name $arg_value"
				switch $arg_name {
					USE_MSR_INSTR {
						puts $config_file "#define XILINX_USE_MSR_INSTR\t$arg_value"
					}
					FSL_LINKS {
						puts $config_file "#define XILINX_FSL_NUMBER\t$arg_value"
					}
					USE_ICACHE {
						if {[string match $arg_value "1"]} {
							puts $config_file "#define XILINX_USE_ICACHE\t$arg_value"
						}
					}
					#ICACHE_BASEADDR {
					#	puts $config_file "#define XILINX_ICACHE_BASEADDR\t$arg_value"
					#}
					#ICACHE_HIGHADDR {
					#	puts $config_file "#define XILINX_ICACHE_HIGHADDR\t$arg_value"
					#}
					USE_DCACHE {
						if {[string match $arg_value "1"]} {
							puts $config_file "#define XILINX_USE_DCACHE\t$arg_value"
						}
					}
					DCACHE_BYTE_SIZE {
						puts $config_file "#define XILINX_DCACHE_BYTE_SIZE\t$arg_value"
					}
					#DCACHE_BASEADDR {
					#	puts $config_file "#define XILINX_DCACHE_BASEADDR\t$arg_value"
					#}
					#DCACHE_HIGHADDR {
					#	puts $config_file "#define XILINX_DCACHE_HIGHADDR\t$arg_value"
					#}
					HW_VER {
						puts $config_file2 "PLATFORM_CPPFLAGS += -mcpu=v$arg_value"
					}
					USE_BARREL {
						if {[string match $arg_value "1"]} {
							puts $config_file2 "PLATFORM_CPPFLAGS += -mxl-barrel-shift"
						} else {
							puts $config_file2 "PLATFORM_CPPFLAGS += -mno-xl-barrel-shift"
						}
					}
					USE_DIV {
						if {[string match $arg_value "1"]} {
							puts $config_file2 "PLATFORM_CPPFLAGS += -mno-xl-soft-div"
						} else {
							puts $config_file2 "PLATFORM_CPPFLAGS += -mxl-soft-div"
						}
						#FIXME What is -mno-xl-hard-div
					}
					USE_HW_MUL {
						case $arg_value in {
							"2" {
								puts $config_file2 "PLATFORM_CPPFLAGS += -mxl-multiply-high"
								puts $config_file2 "PLATFORM_CPPFLAGS += -mno-xl-soft-mul"
							}
							"1" {
								puts $config_file2 "PLATFORM_CPPFLAGS += -mno-xl-multiply-high"
								puts $config_file2 "PLATFORM_CPPFLAGS += -mno-xl-soft-mul"
							}
							default {
								puts $config_file2 "PLATFORM_CPPFLAGS += -mxl-soft-mul"
							}
						}
						#FIXME What is -mxl-multiply-high???
					}
					USE_PCMP_INSTR {
						if {[string match $arg_value "1"]} {
							puts $config_file2 "PLATFORM_CPPFLAGS += -mxl-pattern-compare"
						} else {
							puts $config_file2 "PLATFORM_CPPFLAGS += -mno-xl-pattern-compare"
						}
					}
					USE_FPU {
						if {[string match $arg_value "1"]} {
							puts $config_file2 "PLATFORM_CPPFLAGS += -mhard-float"
						}
					}
					PVR {
						puts $config_file "#define XILINX_PVR\t\t$arg_value"
					}
					FAMILY {
					}
					default {}
				}
			}

		}
		"ppc405" -
		"ppc405_virtex4" -
		"ppc440_virtex5" {
			# Write only name of instance
			puts $config_file "/* PPC is [xget_hw_parameter_value $hwproc_handle "INSTANCE"] */"
		}
		"ps7_cortexa9" {
			# Write only name of instance
			puts $config_file "/* ARM is [xget_hw_parameter_value $hwproc_handle "INSTANCE"] */"
			set cpu_freq [uboot_value $hwproc_handle C_CPU_CLK_FREQ_HZ]
			# FIXME: check if we can change this name as this value maybe used by other drivers
			puts $config_file  "#define XPAR_CPU_CORTEXA9_CORE_CLOCK_FREQ_HZ\t$cpu_freq"
		}
		default {
			error "This type of CPU is not supported by U-BOOT yet"
		}
	}

	puts $config_file ""
	# Note final param (system_bus) is not used, just blank it off for now
	uboot_intc $os_handle $proc_handle $config_file $config_file2 $cpu_freq ""

	close $config_file
	close $config_file2
}

#function for handling adress
proc uboot_addr_hex {handle name} {
	return [format "0x%08x" [uboot_value $handle $name]]
}

proc uboot_value {handle name} {
	set addr [xget_sw_parameter_value "$handle" "$name"]
	if {![llength $addr]} {
		error "Request for undefined value [xget_hw_name $handle]:$name"
	}
	return $addr
}

proc uboot_set_phy_addr {os_handle_name ethernet config_file} {
	set overrides [xget_sw_parameter_value $os_handle_name "periph_type_overrides"]

	foreach over $overrides {
		if { [lindex $over 0] == "phy" &&  [lindex $over 1] == $ethernet } {
			puts $config_file "#define CONFIG_PHY_ADDR\t\t\t[format "0x%x" [lindex $over 2]]"
			break
		}
	}
}

proc uboot_intc {os_handle proc_handle config_file config_file2 freq system_bus} {
	global proctype
# ******************************************************************************
# Interrupt controler
	switch $proctype {
		"microblaze" {
			set intc_handle [get_handle_to_intc $proc_handle "Interrupt"]
		}

		"ppc405" -
                "ppc405_virtex4" {
			set intc_handle [get_handle_to_intc $proc_handle "EICC405EXTINPUTIRQ"]
		}

		"ppc440_virtex5" {
			set intc_handle [get_handle_to_intc $proc_handle "EICC440EXTIRQ"]
		}

		"ps7_cortexa9" {
			set intc_handle [get_handle_to_ps7_core $proc_handle "ps7_scugic"]
		}
	}


	if {[string match "" $intc_handle] || [string match -nocase "none" $intc_handle]} {
		puts $config_file "/* Interrupt controller not defined */"
	} else {
		puts $config_file "/* Interrupt controller is [xget_hw_name $intc_handle] */"
		switch $proctype {
			# FIXME: this is hacky to detect the interrupt controller
			"ps7_cortexa9" {
				puts $config_file "#define XILINX_PS7_INTC_BASEADDR\t\t[uboot_addr_hex $intc_handle "C_S_AXI_BASEADDR"]"
			}
			default {
				#FIXME redesign test_buses - give name of buses from IP
				test_buses $system_bus $intc_handle "SOPB"
				#Interrupt controller address
				puts $config_file "#define XILINX_INTC_BASEADDR\t\t[uboot_addr_hex $intc_handle "C_BASEADDR"]"

				set intc_value [uboot_value $intc_handle "C_NUM_INTR_INPUTS"]
				puts $config_file "#define XILINX_INTC_NUM_INTR_INPUTS\t$intc_value"
				set intc_value [expr $intc_value - 1]

				set port_list [xget_port_by_subtype $intc_handle "*"]
				foreach port $port_list {
					set name [xget_value $port "NAME"]
					if {[string match -nocase $name "intr"]} {
						set intc_irq [xget_value $port "VALUE"]
#DEBUG						puts $config_file "/* pripojene interrupty $name=$intc_irq */"
						set intc_signals [split $intc_irq "&"]
#						split the signals
#					DEBUG	puts $config_file "$signals"
					}
				}
			}
		}
		puts $config_file ""

# ****************************************************************************
# Timer part
# handle timer if exists intc
		if { "$proctype" != "ps7_cortexa9" } {
			set timer [xget_sw_parameter_value $os_handle "timer"]
			if {[string match "" $timer] || [string match -nocase "none" $timer]} {
				# FIXME: hack to find the ttc and scutimer
				puts $config_file "/* Timer not defined */"
			} else {
				set timer_handle [xget_sw_ipinst_handle_from_processor $proc_handle $timer]
				#test for correct system bus
				test_buses $system_bus $timer_handle "SOPB"
#				set timer_base [xget_sw_parameter_value $timer_handle "C_BASEADDR"]
#				set timer_end [xget_sw_parameter_value $timer_handle "C_HIGHADDR"]
#				set timer_base [format "0x%08x" $timer_base]
				puts $config_file "/* Timer pheriphery is $timer */"
				puts $config_file "#define XILINX_TIMER_BASEADDR\t[uboot_addr_hex $timer_handle "C_BASEADDR"]"

#				puts "$timer_handle [xget_hw_value $timer_handle]"
				set intr [get_intr $timer_handle $intc_handle $intc_value "Interrupt"]
				puts $config_file "#define XILINX_TIMER_IRQ\t$intr"

				if { [xget_hw_value $timer_handle] == "axi_timer"} {
					set clkhandle [xget_hw_port_handle $timer_handle "S_AXI_ACLK"]
					if {[string compare -nocase $clkhandle ""] != 0} {
						set freq [xget_hw_subproperty_value $clkhandle "CLK_FREQ_HZ"]
					}
				}
			}
			puts $config_file ""
		}
	}

	puts $config_file "/* System Timer Clock Frequency */"
	if {[string match ps7_cortexa9 $proctype]} {
		puts $config_file "#define XILINX_PS7_CLOCK_FREQ\t[expr $freq/2]\n"
	} else {
		puts $config_file "#define XILINX_CLOCK_FREQ\t$freq\n"
	}

# ******************************************************************************
# UartLite driver - I suppose, only uartlite driver
	set uart [xget_sw_parameter_value $os_handle "stdout"]
	if {[string match "" $uart] || [string match -nocase "none" $uart]} {
		puts $config_file "/* Uart not defined */"
		puts "ERROR Uart not specified. Please specific console"
	} else {
		set uart_handle [xget_sw_ipinst_handle_from_processor $proc_handle $uart]
		# count uartlite/uart16500 ips for serial multi support
		set uartlite_count 0
		set uart16550_count 0
		set ps7uart_count 0

		puts $config_file "/* Uart console is $uart */"
		set type [xget_value $uart_handle "VALUE"]
		switch $type {
			"axi_uart16550" -
			"opb_uart16550" -
			"xps_uart16550" {
				puts $config_file "#define XILINX_UART16550"
				puts $config_file "#define XILINX_UART16550_BASEADDR\t[uboot_addr_hex $uart_handle "C_BASEADDR"]"
				puts $config_file "#define XILINX_UART16550_CLOCK_HZ\t[clock_val $uart_handle]"
				incr uart16550_count
			}
			"ps7_uart" {
				puts $config_file "#define XILINX_PS7_UART"
				puts $config_file "#define XILINX_PS7_UART_BASEADDR\t[uboot_addr_hex $uart_handle "C_S_AXI_BASEADDR"]"
				puts $config_file "#define XILINX_PS7_UART_CLOCK_HZ\t[uboot_value $uart_handle "C_UART_CLK_FREQ_HZ"]"
				incr ps7uart_count
			}
			"opb_uartlite" -
			"xps_uartlite" -
			"axi_uartlite" {
				set args [xget_sw_parameter_handle $uart_handle "*"]
				foreach arg $args {
					set arg_name [xget_value $arg "NAME"]
					set arg_value [xget_value $arg "VALUE"]
					set arg_name [string map -nocase {C_ ""} $arg_name]
					case $arg_name in {
						"BASEADDR" {
							puts $config_file "#define XILINX_UARTLITE_${arg_name}\t$arg_value"
							set uart_base $arg_value
						}
						"BAUDRATE" {
							puts $config_file "#define XILINX_UARTLITE_${arg_name}\t$arg_value"
						}
						"HIGHADDR" {
							set uart_end $arg_value
						}
						default	{}
					}
				}
				incr uartlite_count
			}
			"opb_mdm" -
			"mdm" -
			"xps_mdm" {
				set args [xget_sw_parameter_handle $uart_handle "*"]
				foreach arg $args {
					set arg_name [xget_value $arg "NAME"]
					set arg_value [xget_value $arg "VALUE"]
					set arg_name [string map -nocase {C_ ""} $arg_name]
					case $arg_name in {
						"BASEADDR" {
							puts $config_file "#define XILINX_UARTLITE_${arg_name}\t$arg_value"
							set uart_base $arg_value
						}
						default	{}
					}
				}
				puts $config_file "#define XILINX_UARTLITE_BAUDRATE\t115200"
				incr uartlite_count
			}
			default {
				error "Unsupported type of console - $type"
			}
		}

		# Load all IPs connected to cpu BUS, ips variable stores all of them
		set mhs_handle [xget_hw_parent_handle $uart_handle]
		set hwproc_handle [xget_handle $proc_handle "IPINST"]
		set ips ""
		set bus_name [xget_hw_busif_value $hwproc_handle "M_AXI_DP"]
		if { [string compare -nocase $bus_name ""] != 0 } {
			set ips [xget_hw_connected_busifs_handle $mhs_handle $bus_name "slave"]
		}
		set bus_name [xget_hw_busif_value $hwproc_handle "DPLB"]
		if { [string compare -nocase $bus_name ""] != 0 } {
			set ips [xget_hw_connected_busifs_handle $mhs_handle $bus_name "slave"]
		}
		set bus_name [xget_hw_busif_value $hwproc_handle "DOPB"]
		if { [string compare -nocase $bus_name ""] != 0 } {
			set ips [xget_hw_connected_busifs_handle $mhs_handle $bus_name "slave"]
		}

		foreach ip $ips {
			set ip_handle [xget_hw_parent_handle $ip]
			# do not generate console setting
			if {[ string match -nocase $ip_handle $uart_handle ]} {
				continue;
			}
			set type [xget_value $ip_handle "VALUE"]
			switch $type {
				"axi_uart16550" -
				"opb_uart16550" -
				"xps_uart16550" {
					set addr [expr [uboot_addr_hex $ip_handle "C_BASEADDR"] + 0x1000]
					set val [expr $uart16550_count + 1]
					puts $config_file "#define CONFIG_SYS_NS16550_COM$val\t\t[format "0x%08x" $addr]"
					if {[ string match -nocase $uart16550_count "0" ]} {
						puts $config_file "#define CONFIG_SYS_NS16550_CLK\t\t[clock_val $ip_handle]"
					}
					incr uart16550_count
				}
				"opb_uartlite" -
				"xps_uartlite" -
				"axi_uartlite" -
				"opb_mdm" -
				"mdm" -
				"xps_mdm" {
					if {[ string match -nocase $uartlite_count "0" ]} {
						puts $config_file "#define XILINX_UARTLITE_BASEADDR\t[uboot_addr_hex $ip_handle "C_BASEADDR"]"
					} else {
						puts $config_file "#define XILINX_UARTLITE_BASEADDR$uartlite_count\t[uboot_addr_hex $ip_handle "C_BASEADDR"]"
					}
					incr uartlite_count
				}
				default { }
			}
		}
		if { $uart16550_count > 0 } {
			puts $config_file "#define CONFIG_CONS_INDEX\t\t1"
		}
		if { $uartlite_count > 4 } {
			error "Unsupported number of uartlite IPs : $uartlite_count"
		}
		if { $uart16550_count > 4 } {
			error "Unsupported number of uart16550 IPs : $uart16550_count"
		}
	}
	puts $config_file ""
	# ******************************************************************************
	# IIC driver - I suppose, only uartlite driver
	set iic [xget_sw_parameter_value $os_handle "iic"]
	if {[string match "" $iic] || [string match -nocase "none" $iic]} {
		puts $config_file "/* IIC doesn't exist */"
	} else {
		switch -regexp $iic {
			"ps7_i2c" {
				puts $config_file "/* IIC pheriphery is $iic */"
				set iic_handle [xget_sw_ipinst_handle_from_processor $proc_handle $iic]
				puts $config_file "#define XILINX_PS7_IIC_BASEADDR\t\t[uboot_addr_hex $iic_handle "C_S_AXI_BASEADDR"]"
			}
			default {
				set iic_handle [xget_sw_ipinst_handle_from_processor $proc_handle $iic]
				set iic_baseaddr [xget_sw_parameter_value $iic_handle "C_BASEADDR"]
				set iic_baseaddr [format "0x%08x" $iic_baseaddr]
				set iic_freq [xget_sw_parameter_value $iic_handle "C_IIC_FREQ"]
				set iic_bit [xget_sw_parameter_value $iic_handle "C_TEN_BIT_ADR"]
				puts $config_file "/* IIC pheriphery is $iic */"
				puts $config_file "#define XILINX_IIC_0_BASEADDR\t$iic_baseaddr"
				puts $config_file "#define XILINX_IIC_0_FREQ\t$iic_freq"
				puts $config_file "#define XILINX_IIC_0_BIT\t$iic_bit"
			}
		}
	}
	puts $config_file ""
	# ******************************************************************************
	# GPIO configuration
	set gpio [xget_sw_parameter_value $os_handle "gpio"]
	if {[string match "" $gpio] || [string match "none" $gpio]} {
		puts $config_file "/* GPIO doesn't exist */"
	} else {
		switch -regexp $gpio {
			"ps7_gpio" {
				puts $config_file "/* GPIO is $gpio */"
				set gpio_handle [xget_sw_ipinst_handle_from_processor $proc_handle $gpio]
				puts $config_file "#define XILINX_PS7_GPIO_BASEADDR\t\t[uboot_addr_hex $gpio_handle "C_S_AXI_BASEADDR"]"
			}
			default {
				set base_param_name [format "C_BASEADDR" $gpio]
				set gpio_handle [xget_sw_ipinst_handle_from_processor $proc_handle $gpio]
				set gpio_base [xget_sw_parameter_value $gpio_handle $base_param_name]
				set gpio_base [format "0x%08x" $gpio_base]

				set gpio_end [xget_sw_parameter_value $gpio_handle "C_HIGHADDR"]
				set gpio_end [format "0x%08x" $gpio_end]

				puts $config_file "/* GPIO is $gpio*/"
				puts $config_file "#define XILINX_GPIO_BASEADDR\t$gpio_base"
			}
		}
	}
	puts $config_file ""

	# ******************************************************************************
	# SDIO driver
	set sdio [xget_sw_parameter_value $os_handle "sdio"]
	if {[string match "" $sdio] || [string match -nocase "none" $sdio]} {
		puts $config_file "/* SDIO doesn't exist */"
	} else {
		switch -regexp $sdio {
			"ps7_sd" {
				puts $config_file "/* SDIO controller is $sdio */"
				set sdio_handle [xget_sw_ipinst_handle_from_processor $proc_handle $sdio]
				puts $config_file "#define XILINX_PS7_SDIO_BASEADDR\t\t[uboot_addr_hex $sdio_handle "C_S_AXI_BASEADDR"]"
			}
			default {
				error "Unknown SDIO core $sdio"
			}
		}
	}
	puts $config_file ""

	# ******************************************************************************
	# System memory
	set main_mem [xget_sw_parameter_value $os_handle "main_memory"]
	set eram_base -1
	set eram_size -1
	# Try manual memory setup
	set main_memory_start [xget_sw_parameter_value $os_handle "main_memory_start"]
	set main_memory_size [xget_sw_parameter_value $os_handle "main_memory_size"]
	if {[llength $main_memory_start] == 0} {
		set main_memory_start -1
	}
	if {[llength $main_memory_size] == 0} {
		set main_memory_size 0
	}
	set eram_base [expr ${main_memory_start}]
	set eram_size [expr ${main_memory_size}]
	if { $eram_base >= 0 && $eram_size > 0 } {
		set eram_base [format "0x%08x" $eram_base]
		set eram_size [format "0x%08x" $eram_size]
	} elseif {[string match "" $main_mem] || [string match "none" $main_mem]} {
		puts "ERROR main_memory not specified. Please specific main_memory"
		puts $config_file "/* Main Memory doesn't exist */"
	} else {
		set main_mem_bank [xget_sw_parameter_value $os_handle "main_memory_bank"]
		if {[llength $main_mem_bank] == 0} {
			set main_mem_bank 0
		}
		set main_mem_handle [xget_sw_ipinst_handle_from_processor $proc_handle $main_mem]
		if {[string compare -nocase $main_mem_handle ""] != 0} {
			switch [xget_hw_value $main_mem_handle] {
				"mpmc" {
					set base_param_name "C_MPMC_BASEADDR"
					set high_param_name "C_MPMC_HIGHADDR"
				}
				"axi_v6_ddrx" -
				"axi_7series_ddrx" {
					set base_param_name "C_S_AXI_BASEADDR"
					set high_param_name "C_S_AXI_HIGHADDR"
				}
				"axi_s6_ddrx" {
					set base_param_name [format "C_S%i_AXI_BASEADDR" $main_mem_bank]
					set high_param_name [format "C_S%i_AXI_HIGHADDR" $main_mem_bank]
				}
				"ppc440mc_ddr2" {
					set base_param_name "C_MEM_BASEADDR"
					set high_param_name "C_MEM_HIGHADDR"
				}
				"axi_emc" {
					set base_param_name [format "C_S_AXI_MEM%i_BASEADDR" $main_mem_bank]
					set high_param_name [format "C_S_AXI_MEM%i_HIGHADDR" $main_mem_bank]
				}
				"ps7_ddr" {
					set base_param_name "C_S_AXI_BASEADDR"
					set high_param_name "C_S_AXI_HIGHADDR"
				}
				default {
					if {$main_mem_bank >= 0} {
						set base_param_name [format "C_MEM%i_BASEADDR" $main_mem_bank]
						set high_param_name [format "C_MEM%i_HIGHADDR" $main_mem_bank]
					} else {
						# bankless memory
						set base_param_name "C_BASEADDR"
						set high_param_name "C_HIGHADDR"
					}
				}
			}

			set eram_base [xget_sw_parameter_value $main_mem_handle $base_param_name]
			set eram_end [xget_sw_parameter_value $main_mem_handle $high_param_name]
			# FIXME: this is workaround for Xilinx 14.1/14.2, This can be removed on 14.3
			set XIL_VER [xget_swverandbld]
			if { [regexp -all -- 14.\[1|2\] $XIL_VER] && [xget_hw_value $main_mem_handle] == "ps7_ddr"} {
				set eram_base 0
				set eram_high [expr $eram_end]
			}
			set eram_size [expr $eram_end - $eram_base + 1]
			set eram_base [format "0x%08x" $eram_base]
			set eram_size [format "0x%08x" $eram_size]
		}
	}
	if { [expr $eram_base] >= 0 && [expr $eram_size] > 0 } {
		set eram_high [expr $eram_base + $eram_size]
		set eram_high [format "0x%08x" $eram_high]
		puts "/* Main Memory is $main_mem */"
		puts $config_file "/* Main Memory is $main_mem */"
		puts $config_file "#define XILINX_RAM_START\t$eram_base"
		puts $config_file "#define XILINX_RAM_SIZE\t\t$eram_size"
	}
	puts $config_file ""
	# ******************************************************************************
	# Flash memory
	set flash_mem [xget_sw_parameter_value $os_handle "flash_memory"]
	if {[string match "" $flash_mem] || [string match "none" $flash_mem]} {
		puts $config_file "/* FLASH doesn't exist $flash_mem */"
		puts "FLASH doesn't exists"
	} else {
		set flash_mem_handle [xget_sw_ipinst_handle_from_processor $proc_handle $flash_mem]
		set flash_mem_bank [xget_sw_parameter_value $os_handle "flash_memory_bank"]
		if {[llength $flash_mem_bank] == 0} {
			set flash_mem_bank 0
		}
		set flash_type [xget_hw_value $flash_mem_handle];
		puts $config_file "/* Flash Memory is $flash_mem */"

		# Handle different FLASHs differently
		switch -exact $flash_type {
			"axi_spi" -
			"axi_quad_spi" -
			"xps_spi" {
				# SPI FLASH
				# Set the SPI FLASH's SPI controller's base address.
				set spi_start [xget_sw_parameter_value $flash_mem_handle "C_BASEADDR"]
				puts $config_file "#define XILINX_SPI_FLASH_BASEADDR\t$spi_start"
				# Set the SPI FLASH clock frequency
				if { $flash_type == "xps_spi" } {
					set sys_clk [get_clock_frequency $flash_mem_handle "SPLB_Clk"]
				} else {
					set sys_clk [get_clock_frequency $flash_mem_handle "S_AXI_ACLK"]
				}
				set sck_ratio [xget_sw_parameter_value $flash_mem_handle "C_SCK_RATIO"]
				set sck [expr { $sys_clk / $sck_ratio }]
				puts $config_file "#define XILINX_SPI_FLASH_MAX_FREQ\t$sck"
				# Set the SPI FLASH chip select
				global flash_memory_bank
				puts $config_file "#define XILINX_SPI_FLASH_CS\t$flash_memory_bank"
			}
			"axi_emc" -
			"xps_mch_emc" {
				# Parallel Flash
				if { $flash_type == "xps_mch_emc" } {
					set base_param_name [format "C_MEM%i_BASEADDR" $flash_mem_bank]
					set high_param_name [format "C_MEM%i_HIGHADDR" $flash_mem_bank]
				} else {
					set base_param_name [format "C_S_AXI_MEM%i_BASEADDR" $flash_mem_bank]
					set high_param_name [format "C_S_AXI_MEM%i_HIGHADDR" $flash_mem_bank]
				}
				set flash_start [xget_sw_parameter_value $flash_mem_handle $base_param_name]
				set flash_end [xget_sw_parameter_value $flash_mem_handle $high_param_name]
				set flash_size [expr $flash_end - $flash_start + 1]
				set flash_start [format "0x%08x" $flash_start]
				set flash_size [format "0x%08x" $flash_size]
				puts $config_file "#define XILINX_FLASH_START\t$flash_start"
				puts $config_file "#define XILINX_FLASH_SIZE\t$flash_size"
			}
			"ps7_emc" {
				# FIXME: add ps7_emc
				puts "========== find ps7_emc"
			}
			"ps7_spi" {
				# FIXME: add ps7_spi
				puts "========== find ps7_spi"
			}
			"ps7_qspi" {
				# ZYNQ QSPI FLASH
				# Set the SPI FLASH's SPI controller's base address.
				set spi_start [xget_sw_parameter_value $flash_mem_handle "C_S_AXI_BASEADDR"]
				puts $config_file "#define XILINX_PS7_QSPI_FLASH_BASEADDR\t$spi_start"
				# Set the SPI Flash clock frequency, assume it will be
				# 1/4 of the QSPI controller frequency.
				# Note this is not the actual maximum SPI flash frequency
				# as we can't know.
				set qspi_clk [expr [uboot_value $flash_mem_handle "C_QSPI_CLK_FREQ_HZ"]/4]
				puts $config_file "#define XILINX_SPI_FLASH_MAX_FREQ\t$qspi_clk"
				# Set the SPI FLASH chip select
				global flash_memory_bank
				puts $config_file "#define XILINX_SPI_FLASH_CS\t$flash_memory_bank"
			}
			default {
				error "Unknown flash memory interface type $flash_type"
			}
		}
	}
	puts $config_file ""
	# ******************************************************************************
	# Sysace
	set sysace [xget_sw_parameter_value $os_handle "sysace"]
	if {[string match "" $sysace] || [string match "none" $sysace]} {
		puts $config_file "/* Sysace doesn't exist */"
	} else {
		puts $config_file "/* Sysace Controller is $sysace */"
		set sysace_handle [xget_sw_ipinst_handle_from_processor $proc_handle $sysace]
		set args [xget_sw_parameter_handle $sysace_handle "*"]
		foreach arg $args {
			set arg_name [xget_value $arg "NAME"]
			set arg_name [string map -nocase {C_ ""} $arg_name]
			set arg_value [xget_value $arg "VALUE"]
			switch $arg_name {
				"BASEADDR" {
					puts $config_file "#define XILINX_SYSACE_${arg_name}\t$arg_value"
					set sysace_base $arg_value
				}
				"MEM_WIDTH" {
					puts $config_file "#define XILINX_SYSACE_${arg_name}\t$arg_value"
				}
				"HIGHADDR" {
					set sysace_end $arg_value
				}
				"HW_VER" {
				}
				default {}
			}
		}
	}
	puts $config_file ""
	# ******************************************************************************
	# Ethernet
	set ethernet [xget_sw_parameter_value $os_handle "ethernet"]
	if {[string match "" $ethernet] || [string match -nocase "none" $ethernet]} {
		puts $config_file "/* Ethernet doesn't exist */"
	} else {
		set ethernet_handle [xget_sw_ipinst_handle_from_processor $proc_handle $ethernet]
		set ethernet_name [xget_value $ethernet_handle "VALUE"]
		puts $config_file "/* Ethernet controller is $ethernet */"

		switch $ethernet_name {
			"opb_ethernet" -
			"xps_ethernet" {
				set args [xget_sw_parameter_handle $ethernet_handle "*"]
				foreach arg $args {
					set arg_name [xget_value $arg "NAME"]
					set arg_value [xget_value $arg "VALUE"]
					set arg_name [string map -nocase {C_ ""} $arg_name]
#						{"MII_EXIST" "DMA_PRESENT" "HALF_DUPLEX_EXIST"} {
#							puts $config_file "#define XILINX_EMAC_${arg_name}\t$arg_value"
#						}
					case $arg_name in {
						"BASEADDR" {
							puts $config_file "#define XILINX_EMAC_${arg_name}\t$arg_value"
							set ethernet_base $arg_value
						}
#						"HIGHADDR" {
#							set ethernet_end $arg_value
#						}
						default {}
					}
				}
			}
			"xps_ll_temac" {
				puts $config_file "#define XILINX_LLTEMAC_BASEADDR\t\t\t[uboot_addr_hex $ethernet_handle "C_BASEADDR"]"
#get mhs_handle
				set mhs_handle [xget_hw_parent_handle $ethernet_handle]

				set bus_handle [xget_handle $ethernet_handle "BUS_INTERFACE" "LLINK0"]
				set bus_type [xget_hw_value $bus_handle]
#				debug 8 "$bus_handle --$bus_type "
#initiator is ll_temac
#				set slave_ips [xget_hw_connected_busifs_handle $mhs_handle $bus_type "INITIATOR"]
#				puts "$slave_ips"
#target is mpmc
				set llink_bus [xget_hw_connected_busifs_handle $mhs_handle $bus_type "TARGET"]
#				debug 8 "handle of parent bus is $llink_bus"
#name of bus interface
				set llink_name [xget_hw_name $llink_bus]
#				debug 8 "Name of parent interface: $llink_name"
#get mpmc handle
				set llink_handle [xget_hw_parent_handle $llink_bus]

				set connected_ip_name [xget_hw_name $llink_handle]
				set connected_ip_type [xget_hw_value $llink_handle]

#				debug 8 "connected ip_name: $connected_ip_name"
#				debug 8 "connected ip_type: $connected_ip_type"

				if {$connected_ip_type == "mpmc" } {
					set sdma [xget_sw_parameter_handle $llink_handle "C_SDMA_CTRL_BASEADDR"]
#					debug 8 "sdma: $sdma"
					if {[llength $sdma] != 0 } {
						set mpmc [xget_hw_name $llink_handle]
#						debug 8 "mpmc is $mpmc"
	#I need to separate number of interface
						set sdma_channel [string index "$llink_name" [expr [string length $llink_name] - 1]]
						set sdma_name [xget_value $sdma "NAME"]
						set sdma_name [string map -nocase {C_ ""} $sdma_name]
						set sdma_base [xget_value $sdma "VALUE"]
#						debug 8 "$sdma_name $sdma_base"
	#channel count
						set sdma_base [expr $sdma_base + [expr $sdma_channel * 0x80]]
						set sdma_base [format "0x%08x" $sdma_base]
						puts $config_file "#define XILINX_LLTEMAC_${sdma_name}\t${sdma_base}"
					} else {
						set fifo [xget_sw_parameter_handle $llink_handle "C_BASEADDR"]
						if {[llength $fifo] != 0 } {
							set ll_fifo [xget_hw_name $llink_handle]
#							debug 8 "ll_fifo is $ll_fifo, $fifo"
							set fifo_name [xget_value $fifo "NAME"]
							set fifo_name [string map -nocase {C_ ""} $fifo_name]
							set fifo_base [xget_value $fifo "VALUE"]
#							debug 8 "$fifo_name $fifo_base"
							puts $config_file "#define XILINX_LLTEMAC_FIFO_${fifo_name}\t${fifo_base}"
						} else {
							warning "your ll_temac is no connected properly"
						}
					}
				} elseif {$connected_ip_type == "ppc440_virtex5"} {
			                # Assumes only one PPC.
			                if {[string match LLDMA? $llink_name]} {
			                        set port_number [string range $llink_name 5 5]
			                        set sdma_name "DMA$port_number"

						# DCR regs for SDMA ports are 0x80, 0x98, ...
						set baseaddr [expr 0x80 + 0x18*$port_number]

						puts $config_file "#define XILINX_LLTEMAC_SDMA_USE_DCR\t1"
						puts $config_file "#define XILINX_LLTEMAC_SDMA_CTRL_BASEADDR\t[format "0x%02x" ${baseaddr}]"

			                } else {
			                        error "found ll_temac connected to ppc440_virtex5, but can't find the port number!"
			                }
				} elseif {$connected_ip_type == "xps_ll_fifo"} {
					set fifo  [xget_sw_parameter_handle $llink_handle "C_BASEADDR"]
					set fifo_base [xget_value $fifo "VALUE"]
					puts $config_file "#define XILINX_LLTEMAC_FIFO_BASEADDR\t${fifo_base}"
				}
			}
			"opb_ethernetlite" -
			"axi_ethernetlite" -
			"xps_ethernetlite" {
				set args [xget_sw_parameter_handle $ethernet_handle "*"]
				foreach arg $args {
					set arg_name [xget_value $arg "NAME"]
					set arg_value [xget_value $arg "VALUE"]
					set arg_name [string map -nocase {C_ ""} $arg_name]
					case $arg_name in {
						"BASEADDR" {
							puts $config_file "#define XILINX_EMACLITE_${arg_name}\t$arg_value"
						}
						{"TX_PING_PONG" "RX_PING_PONG"} {
							if { "$arg_value" == "1" } {
								puts $config_file "#define CONFIG_XILINX_EMACLITE_${arg_name}\t$arg_value"
							}
						}
						default {}
					}
				}
			}
			"axi_ethernet" {
				set mhs_handle [xget_hw_parent_handle $ethernet_handle]
				puts $config_file "#define XILINX_AXIEMAC_BASEADDR\t\t\t[uboot_addr_hex $ethernet_handle "C_BASEADDR"]"
				# Find out AXI DMA addr
				set axiethernet_busif_handle [xget_hw_busif_handle $ethernet_handle "AXI_STR_TXD"]
				set axiethernet_name [xget_hw_value $axiethernet_busif_handle]
				set axiethernet_ip_handle [xget_hw_connected_busifs_handle $mhs_handle $axiethernet_name "INITIATOR"]
				set connected_ip_handle [xget_hw_parent_handle $axiethernet_ip_handle]
				puts $config_file "#define XILINX_AXIDMA_BASEADDR\t\t\t[uboot_addr_hex $connected_ip_handle "C_BASEADDR"]"
				uboot_set_phy_addr $os_handle $ethernet $config_file
			}
			"ps7_ethernet" {
				set mhs_handle [xget_hw_parent_handle $ethernet_handle]
				puts $config_file "#define XILINX_PS7_GEM_BASEADDR\t\t\t[uboot_addr_hex $ethernet_handle "C_S_AXI_BASEADDR"]"
				uboot_set_phy_addr $os_handle $ethernet $config_file
			}
			default {
				error "Unsupported ethernet periphery - $ethernet_name"
			}
		}
	}

	#*******************************************************************************
	# U-BOOT position in memory
# FIXME I think that generation via setting don't work corectly
	set text_base [xget_sw_parameter_value $os_handle "uboot_position"]
	set text_base [format "0x%08x" $text_base]
	puts $config_file2 ""
	if {$text_base == 0} {
		if {[llength $eram_base] != 0 } {
			set half [format "0x%08x" [expr $eram_high - 0x400000 ]]
			puts $config_file2 "TEXT_BASE = $half"
			puts $config_file2 "CONFIG_SYS_TEXT_BASE = $half"
			puts "INFO automatic U-BOOT position = $half"
		} else {
			error "Main memory is not defined"
		}
	} else {
		if {$eram_base < $text_base && $eram_high > $text_base} {
			#			puts $config_file2 "# TEXT BASE "
			puts $config_file2 "TEXT_BASE = $text_base"
			puts $config_file2 "CONFIG_SYS_TEXT_BASE = $text_base"
			puts $config_file2 ""
			# print system clock
			#	set sw [xget_sw_parameter_value $proc_handle "HW_INSTANCE"]
			# FIXME Parameters for Microblaze from MHS files
			# look at Microblaze SW manual
			# print microblaze params
			#	set hwproc_handle [xget_handle $proc_handle "IPINST"]
			#	set args [xget_hw_parameter_handle $hwproc_handle "*"]
			#	set proctype [xget_value $hwproc_handle "OPTION" "IPNAME"]
		} else {
			error "ERROR u-boot position is out of range $eram_base - $eram_high"
		}
	}
}

proc headerm {ufile} {
	variable version
	puts $ufile "\#"
	puts $ufile "\# (C) Copyright 2007-2008 Michal Simek"
	puts $ufile "\#"
	puts $ufile "\# Michal SIMEK <monstr@monstr.eu>"
	puts $ufile "\#"
	puts $ufile "\# This program is free software; you can redistribute it and/or"
	puts $ufile "\# modify it under the terms of the GNU General Public License as"
	puts $ufile "\# published by the Free Software Foundation; either version 2 of"
	puts $ufile "\# the License, or (at your option) any later version."
	puts $ufile "\#"
	puts $ufile "\# This program is distributed in the hope that it will be useful,"
	puts $ufile "\# but WITHOUT ANY WARRANTY; without even the implied warranty of"
	puts $ufile "\# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the"
	puts $ufile "\# GNU General Public License for more details."
	puts $ufile "\#"
	puts $ufile "\# You should have received a copy of the GNU General Public License"
	puts $ufile "\# along with this program; if not, write to the Free Software"
	puts $ufile "\# Foundation, Inc., 59 Temple Place, Suite 330, Boston,"
	puts $ufile "\# MA 02111-1307 USA"
	puts $ufile "\#"
	puts $ufile "\# CAUTION: This file is automatically generated by libgen."
	puts $ufile "\# Version: [xget_swverandbld]"
	puts $ufile "\# Generate by $version"
	puts $ufile "\# Project description at http://www.monstr.eu/uboot/"
	puts $ufile "\#"
	puts $ufile ""
}

proc headerc {ufile} {
	variable version
	puts $ufile "/*"
	puts $ufile " * (C) Copyright 2007-2008 Michal Simek"
	puts $ufile " *"
	puts $ufile " * Michal SIMEK <monstr@monstr.eu>"
	puts $ufile " *"
	puts $ufile " * This program is free software; you can redistribute it and/or"
	puts $ufile " * modify it under the terms of the GNU General Public License as"
	puts $ufile " * published by the Free Software Foundation; either version 2 of"
	puts $ufile " * the License, or (at your option) any later version."
	puts $ufile " *"
	puts $ufile " * This program is distributed in the hope that it will be useful,"
	puts $ufile " * but WITHOUT ANY WARRANTY; without even the implied warranty of"
	puts $ufile " * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the"
	puts $ufile " * GNU General Public License for more details."
	puts $ufile " *"
	puts $ufile " * You should have received a copy of the GNU General Public License"
	puts $ufile " * along with this program; if not, write to the Free Software"
	puts $ufile " * Foundation, Inc., 59 Temple Place, Suite 330, Boston,"
	puts $ufile " * MA 02111-1307 USA"
	puts $ufile " *"
	puts $ufile " * CAUTION: This file is automatically generated by libgen."
	puts $ufile " * Version: [xget_swverandbld]"
	puts $ufile " * Generate by $version"
	puts $ufile " * Project description at http://www.monstr.eu/uboot/"
	puts $ufile " */"
	puts $ufile ""
}


#test for peripheral - if is correct setting system bus
proc test_buses {system_bus handle bus_type} {
	set bus [xget_handle $handle "BUS_INTERFACE" $bus_type]
	if { [llength $bus] == 0 } {
		return 1
	}
	set bus [xget_value $bus "VALUE"]
	if { $bus != $system_bus} {
		error "Periphery $handle is connected to another system bus $bus ----"
		return 0
	} else {
		set name [xget_value $handle "NAME"]
		puts "$name has correct system_bus $system_bus"
	}
	return 1
}

proc get_intc_signals {intc} {
	set signals [split [xget_hw_port_value $intc "intr"] "&"]
	set intc_signals {}
	foreach signal $signals {
		lappend intc_signals [string trim $signal]
	}
	return $intc_signals
}

# Get interrupt number
proc get_intr {per_handle intc intc_value port_name} {
	if {![string match "" $intc] && ![string match -nocase "none" $intc]} {
		set intc_signals [get_intc_signals $intc]
		set port_handle [xget_hw_port_handle $per_handle "$port_name"]
		set interrupt_signal [xget_value $port_handle "VALUE"]
		set index [lsearch $intc_signals $interrupt_signal]
		if {$index == -1} {
			return -1
		} else {
			# interrupt 0 is last in list.
			return [expr [llength $intc_signals] - $index - 1]
		}
	} else {
		return -1
	}
}

proc clock_val {hw_handle} {
	set ipname [xget_hw_name $hw_handle]
	set ports [xget_hw_port_handle $hw_handle "*"]
	foreach port $ports {
		set sigis [xget_hw_subproperty_value $port "SIGIS"]
		if {[string toupper $sigis] == "CLK"} {
			set portname [xget_hw_name $port]
			# EDK doesn't compute clocks for ports that aren't connected.
			set connected_port [xget_hw_port_value $hw_handle $portname]
			if {[llength $connected_port] != 0} {
				set frequency [get_clock_frequency $hw_handle $portname]
				return "$frequency"
			}
		}
	}
	puts "Not find correct clock frequency"
}

#
#get handle to interrupt controller from CPU handle
#
proc get_handle_to_intc {proc_handle port_name} {
	#one CPU handle
	set hwproc_handle [xget_handle $proc_handle "IPINST"]
	#hangle to mhs file
	set mhs_handle [xget_hw_parent_handle $hwproc_handle]
	#get handle to interrupt port on Microblaze
	set intr_port [xget_value $hwproc_handle "PORT" $port_name]
	if { [llength $intr_port] == 0 } {
		puts "CPU has no connection to Interrupt controller"
		return
	}
	#	set sink_port [xget_hw_connected_ports_handle $mhs_handle $intr_port "sink"]
	#	set sink_name [xget_hw_name $sink_port]
	#get source port periphery handle - on interrupt controller
	set source_port [xget_hw_connected_ports_handle $mhs_handle $intr_port "source"]
	#get interrupt controller handle
	set intc [xget_hw_parent_handle $source_port]
	#	set name [xget_hw_name $intc]
	#	puts "$intc $name"
	return $intc
}

proc get_handle_to_ps7_core {proc_handle ip_instance} {
	#one CPU handle
	set hwproc_handle [xget_handle $proc_handle "IPINST"]
	#hangle to mhs file
	set mhs_handle [xget_hw_parent_handle $hwproc_handle]
	#get handle to interrupt port on Microblaze

	set ip_handles [xget_hw_ipinst_handle $mhs_handle "*"]
	# loop to find the ip instance

	set name ""
	foreach slave $ip_handles {
		set type [xget_hw_value $slave]
		if {[string match -nocase $type $ip_instance]} {
			set name [xget_hw_name $slave]
			set core $slave
			#puts "interrupt controller: $core - $name - $type"
		}
	}
	if { [llength $name] == 0 } {
		puts "CPU has no connection to Interrupt controller or $ip_instance doesn't exist in the system"
		return
	}
	return $core
}


proc debug {level message} {
	puts "$message"
}
