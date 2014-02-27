#
# EDK BSP board generation for device trees supporting Microblaze and PPC
#
# (C) Copyright 2007-2013 Xilinx, Inc.
# Based on original code:
# (C) Copyright 2007-2013 Michal Simek
# (C) Copyright 2007-2012 PetaLogix Qld Pty Ltd
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

# Debug mechanism.
variable debug_level {}
# Uncomment the line below to get general progress messages.
lappend debug_level [list "info"]
# Uncomment the line below to get warnings about IP core usage.
lappend debug_level [list "warning"]
# Uncomment the line below to get a summary of clock analysis.
lappend debug_level [list "clock"]
# Uncomment the line below to get verbose IP information.
lappend debug_level [list "ip"]
# Uncomment the line below to get debugging information about EDK handles.
# lappend debug_level [list "handles"]


# Globals variable
variable device_tree_generator_version "1.1"
variable cpunumber 0
variable periphery_array ""
variable buses {}
variable bus_count 0
variable mac_count 0
variable gpio_names {}
variable overrides {}
variable microblaze_system_timer ""

variable serial_count 0
variable sysace_count 0
variable ethernet_count 0
variable alias_node_list {}
variable phy_count 0

variable vdma_device_id 0
variable dma_device_id 0

variable ps7_spi_count 0
variable ps7_i2c_count 0

# FIXME it will be better not to use it
variable ps7_cortexa9_clk 0
variable ps7_cortexa9_1x_clk 0

variable ps7_smcc_list {}

variable simple_version 0

#
# How to use generate_device_tree() from another MLD
#
# Include the following proc in your MLD tcl script:
#     proc device_tree_bspscript {device_tree_mld_version} {
#         global env
#
#         set device_tree_mld_script "data/device-tree_v2_1_0.tcl"
#
#         set edk_install_dir $env(XILINX_EDK)
#         set device_tree_bspdir_install "$edk_install_dir/sw/ThirdParty/bsp/$device_tree_mld_version"
#         set device_tree_bspdir_proj "../../../bsp/$device_tree_mld_version"
#
#         if {[file isfile $device_tree_bspdir_proj/$device_tree_mld_script]} {
#             return $device_tree_bspdir_proj/$device_tree_mld_script
#         } elseif {[file isfile $device_tree_bspdir_install/$device_tree_mld_script]} {
#             return $device_tree_bspdir_install/$device_tree_mld_script
#         } else {
#             return ""
#         }
#     }
#
# Inside your generate() or post_generate() routine, add something like the following:
#     set device_tree_mld_name_version "device-tree_v1_00_a"
#     set dts_name "virtex440.dts"
#
#     # kcmd_line is set or constructed however you want. The following is
#     # just an example
#     set kcmd_line "console=ttyUL0 ip=on root=/dev/xsysace2 rw"
#
#     namespace eval device_tree source "[device_tree_bspscript $device_tree_mld_name_version]"
#     device_tree::generate_device_tree "linux/arch/powerpc/boot/dts/$dts_name" $kcmd_line
#

# For calling from top level BSP
proc bsp_drc {os_handle} {
	debug info "\#--------------------------------------"
	debug info "\# device-tree BSP DRC..."
	debug info "\#--------------------------------------"
}

# If standalone purpose
proc device_tree_drc {os_handle} {
	bsp_drc $os_handle
}

proc generate {os_handle} {
	variable  device_tree_generator_version
	variable simple_version

	debug info "\#--------------------------------------"
	debug info "\# device-tree BSP generate..."
	debug info "\#--------------------------------------"

	set bootargs [xget_sw_parameter_value $os_handle "bootargs"]
	global consoleip
	set consoleip [xget_sw_parameter_value $os_handle "stdout"]
	if {[llength $consoleip] == 0} {
		set consoleip [xget_sw_parameter_value $os_handle "console device"]
		variable simple_version
		set simple_version "1"
	}

	global overrides
	set overrides [xget_sw_parameter_value $os_handle "periph_type_overrides"]
	# Format override string to list format
	set overrides [string map { "\}\{" "\} \{" } $overrides]
	edk_override_update

	global main_memory
	set main_memory [xget_sw_parameter_value $os_handle "main_memory"]
	global main_memory_bank
	set main_memory_bank [xget_sw_parameter_value $os_handle "main_memory_bank"]
	if {[llength $main_memory_bank] == 0} {
		set main_memory_bank 0
	}
	global main_memory_start
	set main_memory_start [xget_sw_parameter_value $os_handle "main_memory_start"]
	global main_memory_size
	set main_memory_size [xget_sw_parameter_value $os_handle "main_memory_size"]
	global main_memory_offset
	set main_memory_offset [xget_sw_parameter_value $os_handle "main_memory_offset"]
	global flash_memory
	set flash_memory [xget_sw_parameter_value $os_handle "flash_memory"]
	global flash_memory_bank
	set flash_memory_bank [xget_sw_parameter_value $os_handle "flash_memory_bank"]
	global timer
	set timer [xget_sw_parameter_value $os_handle "timer"]

	if { "$simple_version" == "1" } {
		set main_memory_start -1
		set main_memory_size 0
	}

	global buses
	set buses {}

	generate_device_tree "xilinx.dts" $bootargs $consoleip
}

proc edk_override_update {} {
	global overrides

	# FIXME - Xilinx 14.2 changed the TCL API and IP names are returned
	# lowercase.  Must lowercase the override string to match
	if { [xget_swverandbld]  >= "14.2" } {
		set allover $overrides
		set overrides ""
		foreach over $allover {
			if { "[string first "-" [lindex $over 1]]" == "0" } {
				set ipname [string tolower [lindex $over 2]]
				lset over 2 $ipname
			} else {
				set ipname [string tolower [lindex $over 1]]
				lset over 1 $ipname
			}
			lappend overrides $over
		}
	}
}

proc generate_device_tree {filepath bootargs {consoleip ""}} {
	variable  device_tree_generator_version
	global board_name
	debug info "--- device tree generator version: v$device_tree_generator_version ---"
	debug info "generating $filepath"

	set toplevel {}
	set ip_tree {}

	set proc_handle [xget_libgen_proc_handle]
	set hwproc_handle [xget_handle $proc_handle "IPINST"]

# Clock port summary
	debug clock "Clock Port Summary:"
	set mhs_handle [xget_hw_parent_handle $hwproc_handle]
	set ips [xget_hw_ipinst_handle $mhs_handle "*"]
	foreach ip $ips {
		set ipname [xget_hw_name $ip]
		set ports [xget_hw_port_handle $ip "*"]
		foreach port $ports {
			set sigis [xget_hw_subproperty_value $port "SIGIS"]
			if {[string toupper $sigis] == "CLK"} {
				set portname [xget_hw_name $port]
				# EDK doesn't compute clocks for ports that aren't connected.
				set connected_port [xget_hw_port_value $ip $portname]
				if {[llength $connected_port] != 0} {
					set frequency [get_clock_frequency $ip $portname]
					if {$frequency == ""} {
						set connected_bus [get_clock_frequency $ip $portname]
						set frequency "WARNING: no frequency found!"
					}
					debug clock "$ipname.$portname connected to $connected_port:"
					debug clock "    CLK_FREQ_HZ = $frequency"
					set dir [xget_hw_subproperty_value $port "DIR"]
					set inport [xget_hw_subproperty_value $port "CLK_INPORT"]
					set factor [xget_hw_subproperty_value $port "CLK_FACTOR"]
					if {[string toupper $dir] == "O"} {
						debug clock "    CLK_INPORT = $inport"
						debug clock "    CLK_FACTOR = $factor"
					}
				}
			}
		}
	}

	set proctype [xget_value $hwproc_handle "OPTION" "IPNAME"]
	switch $proctype {
		"microblaze" {
			# Microblaze linux system requires dual-channel timer
			global timer
			variable simple_version

			if { "$simple_version" != "1" } {
				if { [string match "" $timer] || [string match "none" $timer] } {
					error "No timer is specified in the system. Linux requires dual channel timer."
				}
			}

			set intc [get_handle_to_intc $proc_handle "Interrupt"]
			set toplevel [gen_microblaze $toplevel $hwproc_handle [default_parameters $hwproc_handle]]


			# Microblaze v8 has AXI and/or PLB. xget_hw_busif_handle returns
			# a valid handle for both these bus ifs, even if they are not
			# connected. The better way of checking if a bus is connected
			# or not is to check it's value.
			set bus_name [xget_hw_busif_value $hwproc_handle "M_AXI_DC"]
			if { [string compare -nocase $bus_name ""] != 0 } {
				set tree [bus_bridge $hwproc_handle $intc 0 "M_AXI_DC"]
				if { [llength $tree] != 0 } {
					set tree [tree_append $tree [list ranges empty empty]]
					lappend ip_tree $tree
				}
			}
			set bus_name [xget_hw_busif_value $hwproc_handle "M_AXI_DP"]
			if { [string compare -nocase $bus_name ""] != 0 } {
				set tree [bus_bridge $hwproc_handle $intc 0 "M_AXI_DP"]
				if { [llength $tree] != 0 } {
					set tree [tree_append $tree [list ranges empty empty]]
					lappend ip_tree $tree
				}
			}
			set bus_name [xget_hw_busif_value $hwproc_handle "DPLB"]
			if { [string compare -nocase $bus_name ""] != 0 } {
				# Microblaze v7 has PLB.
				set tree [bus_bridge $hwproc_handle $intc 0 "DPLB"]
				if { [llength $tree] != 0 } {
					set tree [tree_append $tree [list ranges empty empty]]
					lappend ip_tree $tree
				}
			}
			set bus_name [xget_hw_busif_value $hwproc_handle "DOPB"]
			if { [string compare -nocase $bus_name ""] != 0 } {
				# Older microblazes have OPB.
				set tree [bus_bridge $hwproc_handle $intc 0 "DOPB"]
				if { [llength $tree] != 0 } {
					set tree [tree_append $tree [list ranges empty empty]]
					lappend ip_tree $tree
				}
			}
			lappend toplevel [list "compatible" stringtuple [list "xlnx,microblaze"] ]
			if { ![info exists board_name] } {
				lappend toplevel [list model string "Xilinx MicroBlaze"]
			}

			variable microblaze_system_timer
			if { "$simple_version" != "1" } {
				if { [llength $microblaze_system_timer] == 0 } {
					error "Microblaze requires to setup system timer. Please setup it!"
				}
			}
		}
		"ppc405" -
		"ppc405_virtex4" {
			global timer
			set timer ""

			set intc [get_handle_to_intc $proc_handle "EICC405EXTINPUTIRQ"]
			set toplevel [gen_ppc405 $toplevel $hwproc_handle [default_parameters $hwproc_handle]]
			set busif_handle [xget_hw_busif_handle $hwproc_handle "DPLB"]
			if {[llength $busif_handle] != 0} {
				# older ppc405s have a single PLB interface.
				set tree [bus_bridge $hwproc_handle $intc 0 "DPLB"]
				if { [llength $tree] != 0 } {
					set tree [tree_append $tree [list ranges empty empty]]
					lappend ip_tree $tree
				}
			} else {
				# newer ppc405s since edk9.2 have two plb interfaces, with
				# DPLB1 only being used for memory.
				set tree [bus_bridge $hwproc_handle $intc 0 "DPLB0"]
				if { [llength $tree] != 0 } {
					set tree [tree_append $tree [list ranges empty empty]]
					lappend ip_tree $tree
				}
				set tree [bus_bridge $hwproc_handle $intc 1 "DPLB1"]
				if { [llength $tree] != 0 } {
					set tree [tree_append $tree [list ranges empty empty]]
					lappend ip_tree $tree
				}
			}
			# pickup things which are only on the dcr bus.
			if {[bus_is_connected $hwproc_handle "MDCR"]} {
				set tree [bus_bridge $hwproc_handle $intc 0 "MDCR"]
				if { [llength $tree] != 0 } {
					lappend ip_tree $tree
				}
			}

			lappend toplevel [list "compatible" stringtuple [list "xlnx,virtex405" "xlnx,virtex"] ]
			if { ![info exists board_name] } {
				lappend toplevel [list model string "Xilinx PPC Virtex405"]
			}
		}
		"ppc440_virtex5" {
			global timer
			set timer ""

			set intc [get_handle_to_intc $proc_handle "EICC440EXTIRQ"]
			set toplevel [gen_ppc440 $toplevel $hwproc_handle $intc [default_parameters $hwproc_handle]]
			set tree [bus_bridge $hwproc_handle $intc 0 "MPLB"]
			if { [llength $tree] != 0 } {
				set tree [tree_append $tree [list ranges empty empty]]
				lappend ip_tree $tree
			}
			# pickup things which are only on the dcr bus.
			if {[bus_is_connected $hwproc_handle "MDCR"]} {
				set tree [bus_bridge $hwproc_handle $intc 0 "MDCR"]
				if { [llength $tree] != 0 } {
					lappend ip_tree $tree
				}
			}

# 			set tree [bus_bridge $hwproc_handle $intc 0 "PPC440MC"]
# 			set tree [tree_append $tree [list ranges empty empty]]
# 			lappend ip_tree $tree

			lappend toplevel [list "compatible" stringtuple [list "xlnx,virtex440" "xlnx,virtex"] ]
			set cpu_name [xget_hw_name $hwproc_handle]
			lappend toplevel [list "dcr-parent" labelref $cpu_name]
			if { ![info exists board_name] } {
				lappend toplevel [list model string "Xilinx PPC Virtex440"]
			}
		}
		"ps7_cortexa9" {
			global timer
			set timer ""

			# MS: This is nasty hack how to get all slave IPs
			# What I do is that load all IPs from M_AXI_DP and then pass all IPs
			# in bus_bridge then handle the rest of IPs
			set ips [xget_hw_proc_slave_periphs $hwproc_handle]

			# FIXME uses axi_ifs instead of ips and remove that param from bus_bridge
			global axi_ifs
			set axi_ifs ""

			# Find out GIC
			foreach i $ips {
				if { "[xget_hw_value $i]" == "ps7_scugic" } {
					set intc "$i"
				}
			}

			set toplevel [gen_cortexa9 $toplevel $hwproc_handle $intc [default_parameters $hwproc_handle]]

			set bus_name [xget_hw_busif_value $hwproc_handle "M_AXI_DP"]
			if { [string compare -nocase $bus_name ""] != 0 } {
				set tree [bus_bridge $hwproc_handle $intc 0 "M_AXI_DP" "" $ips "ps7_pl310 ps7_xadc"]
				set tree [tree_append $tree [list ranges empty empty]]
				lappend ip_tree $tree
			}
			lappend toplevel [list "compatible" stringtuple [list "xlnx,zynq-zc770" "xlnx,zynq-7000"] ]
			if { ![info exists board_name] } {
				lappend toplevel [list model string "Xilinx Zynq"]
			}
		}
		default {
			error "unsupported CPU"
		}
	}

	variable alias_node_list
	puts "$alias_node_list"

	if {[llength $bootargs] == 0} {
		# generate default string for uart16550 or uartlite if specified
		if {![string match "" $consoleip] && ![string match -nocase "none" $consoleip] } {
			set uart_handle [xget_sw_ipinst_handle_from_processor [xget_libgen_proc_handle] $consoleip]
			switch -exact [xget_value $uart_handle "VALUE"] {
				"axi_uart16550" -
				"xps_uart16550" -
				"plb_uart16550" -
				"opb_uart16550" {
					# for uart16550 is default string 115200
					set bootargs "console=ttyS0,115200"
				}
				"axi_uartlite" -
				"xps_uartlite" -
				"opb_uartlite" {
					set bootargs "console=ttyUL0,[xget_sw_parameter_value $uart_handle "C_BAUDRATE"]"
				}
				"mdm" {
					set bootargs "console=ttyUL0,115200"
				}
				"ps7_uart" {
					set bootargs "console=ttyPS0,115200"
				}
				default {
					debug warning "WARNING: Unsupported console ip $consoleip. Can't generate bootargs."
				}
			}
		}
	}

	set chosen {}
	lappend chosen [list bootargs string $bootargs]

	set dev_tree [concat $toplevel $ip_tree]
	if {$consoleip != ""} {
		set consolepath [get_pathname_for_label $dev_tree $consoleip]
		if {$consolepath != ""} {
			lappend chosen [list "linux,stdout-path" string $consolepath]
		} else {
			debug warning "WARNING: console ip $consoleip was not found.  This may prevent output from appearing on the boot console."
		}
	} else {
		debug warning "WARNING: no console ip was specified.  This may prevent output from appearing on the boot console."
	}

	lappend toplevel [list \#size-cells int 1]
	lappend toplevel [list \#address-cells int 1]

	if { [info exists board_name] } {
		lappend toplevel [list model string [prj_dir]]
	}

	set reset [reset_gpio]
	if { "$reset" != "" } {
		lappend toplevel $reset
	}
	lappend toplevel [list chosen tree $chosen]

	#
	# Add the alias section to toplevel
	#
	lappend toplevel [list aliases tree $alias_node_list]

	set toplevel [gen_memories $toplevel $hwproc_handle]

	set toplevel_file [open $filepath w]
	headerc $toplevel_file $device_tree_generator_version
	puts $toplevel_file "/dts-v1/;"
	puts $toplevel_file "/ {"
	write_tree 0 $toplevel_file $toplevel
	write_tree 0 $toplevel_file $ip_tree
	puts $toplevel_file "} ;"
	close $toplevel_file
}

proc post_generate {lib_handle} {
}

proc prj_dir {} {
	# board_name comes from toplevel BSP context
	global board_name

	if { [info exists board_name] } {

		return $board_name
	}
	return [file tail [file normalize [file join .. .. ..]]]
}

proc headerc {ufile generator_version} {
	puts $ufile "/*"
	puts $ufile " * Device Tree Generator version: $generator_version"
	puts $ufile " *"
	puts $ufile " * (C) Copyright 2007-2013 Xilinx, Inc."
	puts $ufile " * (C) Copyright 2007-2013 Michal Simek"
	puts $ufile " * (C) Copyright 2007-2012 PetaLogix Qld Pty Ltd"
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
	puts $ufile " * [clock format [clock seconds] -format {Today is: %A, the %d of %B, %Y; %H:%M:%S}]"
	puts $ufile " *"
	puts $ufile " * XPS project directory: [prj_dir]"
	puts $ufile " */"
	puts $ufile ""
}

# generate structure for reset gpio.
# mss description - first pin of Reset_GPIO ip is used for system reset
# {key-word IP_name gpio_pin size_of_pin}
# for reset-gpio is used only size equals 1
#
# PARAMETER periph_type_overrides = {hard-reset-gpios Reset_GPIO 1 1}
proc reset_gpio {} {
	global overrides
	# ignore size parameter
	set reset {}
	foreach over $overrides {
		# parse hard-reset-gpio keyword
		if {[lindex $over 0] == "hard-reset-gpios"} {
			# search if that gpio name is valid IP core in system
			set desc [valid_gpio [lindex $over 1]]
			if { "$desc" != "" } {
				# check if is pin larger then gpio width
				if {[lindex $desc 1] > [lindex $over 2]} {
					set k [ list [lindex $over 1] [lindex $over 2] 1]
					set reset "hard-reset-gpios labelref-ext {{$k}}"
					return $reset
				} else {
					puts "RESET-GPIO: Requested pin is greater than number of GPIO pins: $over"
				}
			} else {
				puts "RESET-GPIO: Not valid IP name: $over"
			}
		}
	}
	return
}

# For generation of gpio led description
# this function is called from bus code because linux needs to have this description in the same node as is IP
# FIXME there could be maybe problem if system contains bridge and gpio is after it - needs test
#
# PARAMETER periph_type_overrides = {led heartbeat LEDs_8Bit 5 5} {led yellow LEDs_8Bit 7 2} {led green LEDs_8Bit 4 1}
proc led_gpio {} {
	global overrides
	set tree {}
	foreach over $overrides {
		# parse hard-reset-gpio keyword
		if {[lindex $over 0] == "led"} {
			# clear trigger
			set trigger ""
			set desc [valid_gpio [lindex $over 2]]
			if { "$desc" != "" } {
				# check if is pin larger then gpio width
				if { [lindex $desc 1] > [lindex $over 3]} {
					# check if the size exceed number of pins
					if { [lindex $desc 1] >= [expr [lindex $over 3] + [lindex $over 4]] } {
						# assemble led node
						set label_desc "{label string [lindex $over 1]}"
						set led_pins "{[lindex $over 2] [lindex $over 3] [lindex $over 4]}"
						if { [string match -nocase "heartbeat" [lindex $over 1]] } {
							set trigger "{linux,default-trigger string heartbeat}"
						}
						set tree "{[lindex $over 1] tree { $label_desc $trigger { gpios labelref-ext $led_pins }}} $tree"
					} else {
						puts "LED-GPIO: Requested pin size reach out of GPIO pins width: $over"
					}

				} else {
					puts "LED-GPIO: Requested pin is greater than number of GPIO pins: $over"
				}
			} else {
				puts "LED-GPIO: Not valid IP name $over"
			}
		}
	}
	# it is a complex node that's why I have to assemble it
	if { "$tree" != "" } {
		set tree "gpio-leds tree { {compatible string gpio-leds} $tree }"
	}
	return $tree
}

# Check if gpio name is valid or not
proc valid_gpio {name} {
	global gpio_names
	foreach gpio_desc $gpio_names {
		if { [string match -nocase [lindex $gpio_desc 0] "$name" ] } {
			return $gpio_desc
		}
	}
	return
}

proc get_intc_signals {intc} {

	# MS the simplest way to detect ARM is through intc type
	if { "[xget_hw_value $intc]" == "ps7_scugic" } {
		# MS here is small complication because INTC from FPGA
		# are divided to two separate segments. That's why
		# I generate two silly offsets to setup correct location
		# for both segments.
		# FPGA 7-0 - irq 61 - 68
		# FPGA 15-8 - irq 84 - 91

		set int_lines "[split [xget_hw_port_value $intc "IRQ_F2P"] "&"]"

		set fpga_irq_id 0
		set irq_signals {}
		for {set x 0} {$x < [llength $int_lines]} {incr x} {
				set e [string trim [lindex ${int_lines} $x]]
				if { [string range $e 0 1] == "0b" } {
					# Sometimes there could be 0 instead of a physical interrupt signal
					set siglength [ expr [string length $e] - 2 ]
					for {set y 0} {$y < ${siglength}} {incr y} {
						lappend irq_signals 0
					}
				} elseif { [string range $e 0 1] == "0x" } {
					# Sometimes there could be 0 instead of a physical interrupt signal
					error "This interrupt signal is a hex digit, cannot detect the length of it"
				} else {
					# actual interrupt signal
					lappend irq_signals $e
				}
		}
		if { [llength $irq_signals] > 16 } {
			error "Too many interrupt lines connected to Zynq GIC"
		}

		# append the missing irq bits
		for {set x [llength ${irq_signals}]} {$x < 16} {incr x} {
			lappend irq_signals 0
		}

		# skip the first 32 interrupts because of Linux
		# and generate numbers till the first fpga area
		# top to down 60 - 32
		set linux_irq_offset 32
		for {set x 60} {$x >= $linux_irq_offset} { set x [expr $x - 1] } {
			lappend pl1 $x
		}

		# offset between fpga 7-0 and 15-8 is fixed
		# top to down 83 - 69
		for {set x 83} {$x >= 69} { set x [expr $x - 1] } {
			lappend pl2 $x
		}

		# Compose signal string with this layout from top to down
		set signals "[lrange ${irq_signals} 0 7] $pl2 [lrange ${irq_signals} 8 15] $pl1"
	} else {
		set signals [split [xget_hw_port_value $intc "intr"] "&"]
	}

	set intc_signals {}
	foreach signal $signals {
		lappend intc_signals [string trim $signal]
	}
	return $intc_signals
}

# Get interrupt number
proc get_intr {ip_handle intc port_name} {
	if {![string match "" $intc] && ![string match -nocase "none" $intc]} {
		set intc_signals [get_intc_signals $intc]
		set port_handle [xget_hw_port_handle $ip_handle "$port_name"]
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

proc get_intr_type {intc ip_handle port_name} {
	set ip_name [xget_hw_name $ip_handle]
	set port_handle [xget_hw_port_handle $ip_handle "$port_name"]
	set sensitivity [xget_hw_subproperty_value $port_handle "SENSITIVITY"];

	if { "[xget_hw_value $intc]" == "ps7_scugic" } {
		# Follow the openpic specification
		if { [string compare -nocase $sensitivity "EDGE_FALLING"] == 0 } {
			return 2;
		} elseif { [string compare -nocase $sensitivity "EDGE_RISING"] == 0 } {
			return 1;
		} elseif { [string compare -nocase $sensitivity "LEVEL_HIGH"] == 0 } {
			return 4;
		} elseif { [string compare -nocase $sensitivity "LEVEL_LOW"] == 0 } {
			return 8;
		}
	} else {
		# Follow the openpic specification
		if { [string compare -nocase $sensitivity "EDGE_FALLING"] == 0 } {
			return 3;
		} elseif { [string compare -nocase $sensitivity "EDGE_RISING"] == 0 } {
			return 0;
		} elseif { [string compare -nocase $sensitivity "LEVEL_HIGH"] == 0 } {
			return 2;
		} elseif { [string compare -nocase $sensitivity "LEVEL_LOW"] == 0 } {
			return 1;
		}
	}

	error "Unknown interrupt sensitivity on port $port_name of $ip_name was $sensitivity"
}

# Generate a template for a compound slave, such as the ll_temac or
# the opb_ps2_dual_ref
proc compound_slave {slave {baseaddrname "C_BASEADDR"}} {
	set baseaddr [scan_int_parameter_value $slave ${baseaddrname}]
	set ip_name [xget_hw_name $slave]
	set ip_type [xget_hw_value $slave]
	set tree [list [format_ip_name $ip_type $baseaddr $ip_name] tree {}]
	set tree [tree_append $tree [list \#size-cells int 1]]
	set tree [tree_append $tree [list \#address-cells int 1]]
	set tree [tree_append $tree [list ranges empty empty]]
	set tree [tree_append $tree [list compatible stringtuple [list "xlnx,compound"]]]
	return $tree
}

proc slaveip_intr {slave intc interrupt_port_list devicetype params {baseaddr_prefix ""} {dcr_baseaddr_prefix ""} {other_compatibles {}} } {
	set tree [slaveip $slave $intc $devicetype $params $baseaddr_prefix $other_compatibles]
	return [gen_interrupt_property $tree $slave $intc $interrupt_port_list]
}

proc get_dcr_parent_name {slave face} {
	set busif_handle [xget_hw_busif_handle $slave $face]
	if {[llength $busif_handle] == 0} {
		error "Bus handle $face not found!"
	}
	set bus_name [xget_hw_value $busif_handle]

	debug ip "IP on DCR bus $bus_name"
	debug handles "  bus_handle: $busif_handle"
	set mhs_handle [xget_hw_parent_handle $slave]
	set bus_handle [xget_hw_ipinst_handle $mhs_handle $bus_name]

	set master_ifs [xget_hw_connected_busifs_handle $mhs_handle $bus_name "master"]
	if {[llength $master_ifs] == 1} {
		set ip_handle [xget_hw_parent_handle [lindex $master_ifs 0 0]]
		set ip_name [xget_hw_name $ip_handle]
		return $ip_name
	} else {
		error "DCR bus found which does not have exactly one master.  Masters were $master_ifs"
	}
}

proc append_dcr_interface {tree slave {dcr_baseaddr_prefix ""} } {
	set name [xget_hw_name $slave]
	set baseaddr [scan_int_parameter_value $slave [format "C_DCR%s_BASEADDR" $dcr_baseaddr_prefix]]
	set highaddr [scan_int_parameter_value $slave [format "C_DCR%s_HIGHADDR" $dcr_baseaddr_prefix]]
	set tree [tree_append $tree [gen_reg_property $name $baseaddr $highaddr "dcr-reg"]]
	set name [get_dcr_parent_name $slave "SDCR"]
	set tree [tree_append $tree [list "dcr-parent" labelref $name]]
	return $tree
}

# Many IP's (e.g. xps_tft) can be connected to dcr or plb control busses.
proc slaveip_dcr_or_plb {slave intc devicetype params {baseaddr_prefix ""} {other_compatibles {}} } {
	# Get the value of the parameter which indicates about the interface
	# on which the core is connected.
	if {[parameter_exists $slave "C_DCR_SPLB_SLAVE_IF"] != 0} {
		set bus_name  [scan_int_parameter_value $slave "C_DCR_SPLB_SLAVE_IF"]
	} else {
		# Default case is connected to the bus - for axi_tftcd
		set bus_name "1"
	}
	# '1' indicates core connected on PLB bus directly
	# '0' indicates core connected on DCR bus directly
	if {$bus_name == "1"} {
		if {[parameter_exists $slave "C_PLB_BASEADDR"] != 0} {
			return [slaveip $slave $intc $devicetype $params "PLB_" $other_compatibles]
		} else {
			if {[parameter_exists $slave "C_SPLB_BASEADDR"] != 0} {
				return [slaveip $slave $intc $devicetype $params "SPLB_" $other_compatibles]
			} else {
				set ip_tree [slaveip_intr $slave $intc [interrupt_list $slave] $devicetype $params "S_AXI_" "" $other_compatibles]
				# Necessary for linux driver probing because driver was designed for PPC DCR
				set ip_tree [tree_append $ip_tree [list "xlnx,dcr-splb-slave-if" int $bus_name]]
				return $ip_tree
			}
		}
	} else {
		# When the core is connected directly on the DCR bus
		return [slaveip_dcr $slave $intc "tft" [default_parameters $slave] "DCR_" $other_compatibles]
	}
}

# DCR addresses are usually word addresses, not byte addresses.  The
# device tree always handles byte address.
proc slaveip_dcr {slave intc devicetype params {baseaddr_prefix ""} {other_compatibles {}} } {
	set dcr_baseaddr [scan_int_parameter_value $slave [format "C_%sBASEADDR" $baseaddr_prefix]]
	set name [xget_hw_name $slave]
	set type [xget_hw_value $slave]
	if {$devicetype == ""} {
		set devicetype $type
	}
	set tree [slaveip_basic $slave $intc $params [format_ip_name $devicetype $dcr_baseaddr $name] $other_compatibles]
	set dcr_busif_handle [xget_hw_busif_handle $slave "SDCR"]
	if {[llength $dcr_busif_handle] != 0} {
		# Hmm.. looks like there's a dcr interface.
		set tree [append_dcr_interface $tree $slave]
	}

	# Backward compatibility to not break older style tft driver
	# connected through opb2dcr bridge.
	set dcr_highaddr [scan_int_parameter_value $slave [format "C_%sHIGHADDR" $baseaddr_prefix]]
	# DCR addresses are word-based addresses.  Here we convert to the
	# correct byte ranges that subsume the word ranges.  This is tricky
	# in the case of the high address, since we multiply the length by
	# 4 and then have to convert back to the correct address.
	set scaled_baseaddr [expr $dcr_baseaddr * 4]
	set scaled_highaddr [expr ($dcr_highaddr + 1) * 4 - 1]
	set tree [tree_append $tree [gen_reg_property $name $scaled_baseaddr $scaled_highaddr]]

	return $tree
}

proc slaveip {slave intc devicetype params {baseaddr_prefix ""} {other_compatibles {}} } {
	set baseaddr [scan_int_parameter_value $slave [format "C_%sBASEADDR" $baseaddr_prefix]]
	set highaddr [scan_int_parameter_value $slave [format "C_%sHIGHADDR" $baseaddr_prefix]]
	set tree [slaveip_explicit_baseaddr $slave $intc $devicetype $params $baseaddr $highaddr $other_compatibles]
	set dcr_busif_handle [xget_hw_busif_handle $slave "SDCR"]
	if {[llength $dcr_busif_handle] != 0} {
		if {[bus_is_connected $slave "SDCR"] != 0} {
			# Hmm.. looks like there's a dcr interface.
			set tree [append_dcr_interface $tree $slave]
		}
	}
	return $tree
}

proc slaveip_pcie_ipif_slave {slave intc devicetype params {baseaddr_prefix ""} {other_compatibles {}} } {
	set baseaddr [scan_int_parameter_value $slave [format "C_%sMEM0_BASEADDR" $baseaddr_prefix]]
	set highaddr [scan_int_parameter_value $slave [format "C_%sMEM0_HIGHADDR" $baseaddr_prefix]]
	set tree [slaveip_explicit_baseaddr $slave $intc $devicetype $params $baseaddr $highaddr $other_compatibles]
	return $tree
}

proc slaveip_explicit_baseaddr {slave intc devicetype params baseaddr highaddr {other_compatibles {}} } {
	set name [xget_hw_name $slave]
	set type [xget_hw_value $slave]
	if {$devicetype == ""} {
		set devicetype $type
	}
	set tree [slaveip_basic $slave $intc $params [format_ip_name $devicetype $baseaddr $name] $other_compatibles]
	return [tree_append $tree [gen_reg_property $name $baseaddr $highaddr]]
}

proc slaveip_basic {slave intc params nodename {other_compatibles {}} } {
	set name [xget_hw_name $slave]
	set type [xget_hw_value $slave]

	set hw_ver [xget_hw_parameter_value $slave "HW_VER"]

	set ip_node {}
	lappend ip_node [gen_compatible_property $name $type $hw_ver $other_compatibles]

	# Generate the parameters
	set ip_node [gen_params $ip_node $slave $params]

	return [list $nodename tree $ip_node]
}

proc gen_intc {slave intc devicetype param {prefix ""} {other_compatibles {}}} {
	set tree [slaveip $slave $intc $devicetype $param $prefix $other_compatibles]
	set intc_name [lindex $tree 0]
	set intc_node [lindex $tree 2]

	# Tack on the interrupt-specific tags.
	lappend intc_node [list \#interrupt-cells hexint 2]
	lappend intc_node [list interrupt-controller empty empty]
	return [list $intc_name tree $intc_node]
}

proc ll_temac_parameters {ip_handle index} {
	set params {}
	foreach param [default_parameters $ip_handle] {
		set pattern [format "C_TEMAC%d*" $index]
		if {[string match $pattern $param]} {
			lappend params $param
		}
	}
	return $params
}

# Generate a slaveip, assuming it is inside a compound that has a
# baseaddress and reasonable ranges.
# index: The index of this slave
# stride: The distance between instances of the slave inside the container
# size: The size of the address space for the slave
proc slaveip_in_compound_intr {slave intc interrupt_port_list devicetype parameter_list index stride size} {
	set name [xget_hw_name $slave]
	set type [xget_hw_value $slave]
	if {$devicetype == ""} {
		set devicetype $type
	}
	set baseaddr [expr $index * $stride]
	set highaddr [expr $baseaddr + $size - 1]
	set ip_tree [slaveip_basic $slave $intc $parameter_list [format_ip_name $devicetype $baseaddr]]
	set ip_tree [tree_append $ip_tree [gen_reg_property $name $baseaddr $highaddr]]
	set ip_tree [gen_interrupt_property $ip_tree $slave $intc $interrupt_port_list]
	return $ip_tree
}

proc slave_ll_temac_port {slave intc index} {
	set name [xget_hw_name $slave]
	set type [xget_hw_value $slave]
	set baseaddr [scan_int_parameter_value $slave "C_BASEADDR"]
	set baseaddr [expr $baseaddr + $index * 0x40]
	set highaddr [expr $baseaddr + 0x3f]

	#
	# Add this temac channel to the alias list
	#
	variable ethernet_count
	variable alias_node_list
	set subnode_name [format "%s_%s" $name "ETHERNET"]
	set alias_node [list ethernet$ethernet_count aliasref $subnode_name $ethernet_count]
	lappend alias_node_list $alias_node
	incr ethernet_count

	set ip_tree [slaveip_basic $slave $intc "" [format_ip_name "ethernet" $baseaddr $subnode_name]]
	set ip_tree [tree_append $ip_tree [list "device_type" string "network"]]
	variable mac_count
	set ip_tree [tree_append $ip_tree [list "local-mac-address" bytesequence [list 0x00 0x0a 0x35 0x00 0x00 $mac_count]]]
	incr mac_count

	set ip_tree [tree_append $ip_tree [gen_reg_property $name $baseaddr $highaddr]]
	set ip_tree [gen_interrupt_property $ip_tree $slave $intc [format "TemacIntc%d_Irpt" $index]]
	set ip_name [lindex $ip_tree 0]
	set ip_node [lindex $ip_tree 2]
	# Generate the parameters, stripping off the right prefix.
	set ip_node [gen_params $ip_node $slave [ll_temac_parameters $slave $index] [format "C_TEMAC%i_" $index]]
	# Generate the common parameters.
	set ip_node [gen_params $ip_node $slave [list "C_PHY_TYPE" "C_TEMAC_TYPE" "C_BUS2CORE_CLK_RATIO"]]
	set ip_tree [list $ip_name tree $ip_node]
	set mhs_handle [xget_hw_parent_handle $slave]
	# See what the temac is connected to.
	set ll_busif_handle [xget_hw_busif_handle $slave "LLINK$index"]
	set ll_name [xget_hw_value $ll_busif_handle]
	set ll_ip_handle [xget_hw_connected_busifs_handle $mhs_handle $ll_name "target"]
	set ll_ip_handle_name [xget_hw_name $ll_ip_handle]
	set connected_ip_handle [xget_hw_parent_handle $ll_ip_handle]
	set connected_ip_name [xget_hw_name $connected_ip_handle]
	set connected_ip_type [xget_hw_value $connected_ip_handle]
	if {$connected_ip_type == "mpmc"} {
		# Assumes only one MPMC.
		if {[string match SDMA_LL? $ll_ip_handle_name]} {
			set port_number [string range $ll_ip_handle_name 7 7]
			set sdma_name "PIM$port_number"
			set ip_tree [tree_append $ip_tree [list "llink-connected" labelref $sdma_name]]
		} else {
			error "found ll_temac connected to mpmc, but can't find the port number!"
		}
	} elseif {$connected_ip_type == "ppc440_virtex5"} {
		# Assumes only one PPC.
		if {[string match LLDMA? $ll_ip_handle_name]} {
			set port_number [string range $ll_ip_handle_name 5 5]
			set sdma_name "DMA$port_number"
			set ip_tree [tree_append $ip_tree [list "llink-connected" labelref $sdma_name]]
		} else {
			error "found ll_temac connected to ppc440_virtex5, but can't find the port number!"
		}
	} else {
		# Hope it's something that only has one locallink
		# connection. Most likely an xps_ll_fifo
		set ip_tree [tree_append $ip_tree [list "llink-connected" labelref "$connected_ip_name"]]
	}
	return $ip_tree
}
proc slave_ll_temac {slave intc} {
	set tree [compound_slave $slave]
	set tree [tree_append $tree [slave_ll_temac_port $slave $intc 0] ]
	set port1_enabled  [scan_int_parameter_value $slave "C_TEMAC1_ENABLED"]
	if {$port1_enabled == "1"} {
		set tree [tree_append $tree [slave_ll_temac_port $slave $intc 1] ]
	}
	return $tree
}
proc slave_mpmc {slave intc} {
	set share_addresses [scan_int_parameter_value $slave "C_ALL_PIMS_SHARE_ADDRESSES"]
	if {[catch {
		# Found control port for ECC and performance monitors
		set tree [slaveip $slave $intc "" "" "MPMC_CTRL_"]
		set ip_name [lindex $tree 0]
		set mpmc_node [lindex $tree 2]
	}]} {
		# No control port
		if {$share_addresses == 0} {
			set baseaddr [scan_int_parameter_value $slave "C_PIM0_BASEADDR"]
		} else {
			set baseaddr [scan_int_parameter_value $slave "C_MPMC_BASEADDR"]
		}
		set tree [slaveip_basic $slave $intc "" [format_ip_name "mpmc" $baseaddr] ]
		set ip_name [lindex $tree 0]
		set mpmc_node [lindex $tree 2]

		# Generate the parameters
		# set mpmc_node [gen_params $mpmc_node $slave [default_parameters $slave] ]

	}
	lappend mpmc_node [list \#size-cells int 1]
	lappend mpmc_node [list \#address-cells int 1]
	lappend mpmc_node [list ranges empty empty]

	set num_ports [scan_int_parameter_value $slave "C_NUM_PORTS"]
	for {set x 0} {$x < $num_ports} {incr x} {
		set pim_type [scan_int_parameter_value $slave [format "C_PIM%d_BASETYPE" $x]]
		if {$pim_type == 3} {
			# Found an SDMA port
			if {$share_addresses == 0} {
				set baseaddr [scan_int_parameter_value $slave [format "C_SDMA_CTRL%d_BASEADDR" $x]]
				set highaddr [scan_int_parameter_value $slave [format "C_SDMA_CTRL%d_HIGHADDR" $x]]
			} else {
				set baseaddr [scan_int_parameter_value $slave "C_SDMA_CTRL_BASEADDR"]
				set baseaddr [expr $baseaddr + $x * 0x80]
				set highaddr [expr $baseaddr + 0x7f]
			}

			set sdma_name [format_ip_name sdma $baseaddr "PIM$x"]
			set sdma_tree [list $sdma_name tree {}]
			set sdma_tree [tree_append $sdma_tree [gen_reg_property $sdma_name $baseaddr $highaddr]]
			set sdma_tree [tree_append $sdma_tree [gen_compatible_property $sdma_name "ll_dma" "1.00.a"]]
			set sdma_tree [gen_interrupt_property $sdma_tree $slave $intc [list [format "SDMA%d_Rx_IntOut" $x] [format "SDMA%d_Tx_IntOut" $x]]]

			lappend mpmc_node $sdma_tree

		}
	}
	return [list $ip_name tree $mpmc_node]
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
		error "CPU has not connection to Interrupt controller"
	}
	#	set sink_port [xget_hw_connected_ports_handle $mhs_handle $intr_port "sink"]
	#	set sink_name [xget_hw_name $sink_port]
	#get source port periphery handle - on interrupt controller
	set source_port [xget_hw_connected_ports_handle $mhs_handle $intr_port "source"]
	#get interrupt controller handle
	set intc [xget_hw_parent_handle $source_port]
	set name [xget_hw_name $intc]
	debug handles "Interrupt Controller: $name $intc"
	return $intc
}

#return number of tabulator
proc tt {number} {
	set tab ""
	for {set x 0} {$x < $number} {incr x} {
		set tab "$tab\t"
	}
	return $tab
}

# Change the name of a node.
proc change_nodename {nodetochange oldname newname} {
	if {[llength $nodetochange] == 0} {
		error "Tried to change the name of an empty node: $oldname with $newname"
	}
	# The name of a node is in the first element of the node
	set lineofname [lindex $nodetochange 0]
	set substart [string first $oldname $lineofname]
	set subend [expr {$substart + [string length $oldname] - 1}]
	set lineofname [string replace $lineofname $substart $subend $newname]
	return [lreplace $nodetochange 0 0 "$lineofname"]
}

proc check_console_irq {slave intc} {
	global consoleip
	set name [xget_hw_name $slave]

	set irq [get_intr $slave $intc [interrupt_list $slave]]
	if { $irq == "-1" } {
		if {[string match -nocase $name $consoleip]} {
			error "Console($name) interrupt line is not connected to the interrupt controller [xget_hw_name $intc]. Please connect it or choose different console IP."
		} else {
			debug warning "Warning!: Serial IP ($name) has no interrupt connected!"
		}
	}
	return $irq
}

proc zynq_irq {ip_tree intc name } {
	array set zynq_irq_list [ list \
		{cpu_timerFIXME} {1 11 1} \
		{nFIQFIXME} {1 12 8} \
		{ps7_scutimer_0} {1 13 0x301} \
		{ps7_scuwdt_0} {1 14 0x301} \
		{nIRQFIXME} {1 15 8} \
		{ps7_core_parity} {0 0 1 0 1 1} \
		{ps7_pl310} {0 2 4} \
		{ps7_l2cc} {0 3 4} \
		{ps7_ocm} {0 4 4} \
		{ps7_pmu} {0 5 4 0 6 4} \
		{ps7_xadc} {0 7 4} \
		{ps7_dev_cfg_0} {0 8 4} \
		{ps7_wdt_0} {0 9 1} \
		{ps7_ttc_0} {0 10 4 0 11 4 0 12 4} \
		{ps7_dma_s} {0 13 4 0 14 4 0 15 4 0 16 4 0 17 4 0 40 4 0 41 4 0 42 4 0 43 4} \
		{ps7_dma_ns} {0 13 4 0 14 4 0 15 4 0 16 4 0 17 4 0 40 4 0 41 4 0 42 4 0 43 4} \
		{ps7_smcc} {0 18 4} \
		{ps7_qspi_0} {0 19 4} \
		{ps7_gpio_0} {0 20 4} \
		{ps7_usb_0} {0 21 4} \
		{ps7_ethernet_0} {0 22 4} \
		{ps7_ethernet_wake0FIXME} {0 23 1} \
		{ps7_sd_0} {0 24 4} \
		{ps7_i2c_0} {0 25 4} \
		{ps7_spi_0} {0 26 4} \
		{ps7_uart_0} {0 27 4} \
		{ps7_can_0} {0 28 4} \
		{ps7_fpga_7_0FIXME} {0 29 4 0 34 0 0 31 4 0 32 4 0 33 4 0 34 4 0 35 4 0 36 4} \
		{ps7_ttc_1} {0 37 4 0 38 4 0 39 4} \
		{ps7_usb_1} {0 44 4} \
		{ps7_ethernet_1} {0 45 4} \
		{ps7_ethernet_wake1FIXME} {0 46 1} \
		{ps7_sd_1} {0 47 4} \
		{ps7_i2c_1} {0 48 4} \
		{ps7_spi_1} {0 49 4} \
		{ps7_uart_1} {0 50 4} \
		{ps7_can_1} {0 51 4} \
		{ps7_fpga_irq_15_8FIXME} {0 52 4 0 53 4 0 54 4 0 55 4 0 56 4 0 57 4 0 58 4 0 59 4} \
		{scu_parityFIXME} {0 60 1} \
	]

	if { [info exists zynq_irq_list($name)] } {
		set irq "$zynq_irq_list($name)"
		set ip_tree [tree_append $ip_tree [list "interrupts" inttuple "$irq"]]
		set intc_name [xget_hw_name $intc]
		set ip_tree [tree_append $ip_tree [list "interrupt-parent" labelref $intc_name]]
	}
	return $ip_tree
}

proc gener_slave {node slave intc {force_type ""}} {
	variable phy_count
	variable mac_count

	if { [llength $force_type] != 0 } {
		set name $force_type
		set type $force_type
	} else {
		set name [xget_hw_name $slave]
		set type [xget_hw_value $slave]

		# Ignore IP through overides
		# Command: "ip -ignore <IP name> "
		global overrides
		foreach i $overrides {
			# skip others overrides
			if { [lindex "$i" 0] != "ip" } {
				continue;
			}
			# Compatible command have at least 4 elements in the list
			if { [llength $i] != 3 } {
				error "Wrong compatible override command string - $i"
			}
			# Check command and then IP name
			if { [string match [lindex "$i" 1] "-ignore"] } {
				if { [string match [lindex "$i" 2] "$name"] } {
					puts "Ignoring $node"
					return $node
				}
			}
		}
	}

	switch -exact $type {
		"opb_intc" -
		"xps_intc" -
		"axi_intc" {
			# Interrupt controllers
			lappend node [gen_intc $slave $intc "interrupt-controller" "C_NUM_INTR_INPUTS C_KIND_OF_INTR"]
		}
		"mdm" -
		"opb_mdm" {
			# Microblaze debug

			# Check if uart feature is enabled
			set use_uart [xget_hw_parameter_value $slave "C_USE_UART"]
			if { "$use_uart" == "1" } {
				set irq [check_console_irq $slave $intc]

				variable alias_node_list
				global consoleip
				if { $irq != "-1"} {
					set ip_tree [slaveip_intr $slave $intc [interrupt_list $slave] "serial" [default_parameters $slave] "" "" "xlnx,xps-uartlite-1.00.a" ]
					if {[string match -nocase $name $consoleip]} {
						lappend alias_node_list [list serial0 aliasref $name 0]
						set ip_tree [tree_append $ip_tree [list "port-number" int 0]]
					} else {
						variable serial_count
						incr serial_count
						lappend alias_node_list [list serial$serial_count aliasref $name $serial_count]
						set ip_tree [tree_append $ip_tree [list "port-number" int $serial_count]]
					}
				} else {
					set ip_tree [slaveip_intr $slave $intc [interrupt_list $slave] "serial" [default_parameters $slave] "" "" "xlnx,xps-uartlite-1.00.a" ]
				}
			} else {
				# EDK 11.4 disables PLB connection when USE_UART is disabled that's why whole node won't be generated
				# Only bus connected IPs are generated
				set ip_tree [slaveip_intr $slave $intc [interrupt_list $slave] "debug" [default_parameters $slave] "" "" "" ]
				#"C_MB_DBG_PORTS C_UART_WIDTH C_USE_UART"
			}
			lappend node $ip_tree
		}
		"xps_uartlite" -
		"opb_uartlite" -
		"axi_uartlite" {
			#
			# Add this uartlite device to the alias list
			#
			check_console_irq $slave $intc

			set ip_tree [slaveip_intr $slave $intc [interrupt_list $slave] "serial" [default_parameters $slave] ]
			set ip_tree [tree_append $ip_tree [list "device_type" string "serial"]]

			variable alias_node_list
			global consoleip
			if {[string match -nocase $name $consoleip]} {
				lappend alias_node_list [list serial0 aliasref $name 0]
				set ip_tree [tree_append $ip_tree [list "port-number" int 0]]
			} else {
				variable serial_count
				incr serial_count
				lappend alias_node_list [list serial$serial_count aliasref $name $serial_count]
				set ip_tree [tree_append $ip_tree [list "port-number" int $serial_count]]
			}

			set ip_tree [tree_append $ip_tree [list "current-speed" int [xget_sw_parameter_value $slave "C_BAUDRATE"]]]
			if { $type == "opb_uartlite"} {
				set ip_tree [tree_append $ip_tree [list "clock-frequency" int [get_clock_frequency $slave "SOPB_Clk"]]]
			} elseif { $type == "xps_uartlite" } {
				set ip_tree [tree_append $ip_tree [list "clock-frequency" int [get_clock_frequency $slave "SPLB_Clk"]]]
			} elseif { $type == "axi_uartlite" } {
				set ip_tree [tree_append $ip_tree [list "clock-frequency" int [get_clock_frequency $slave "S_AXI_ACLK"]]]
			}
			lappend node $ip_tree
			#"BAUDRATE DATA_BITS CLK_FREQ ODD_PARITY USE_PARITY"]
		}
		"xps_uart16550" -
		"plb_uart16550" -
		"opb_uart16550" -
		"axi_uart16550" {
			#
			# Add this uart device to the alias list
			#
			check_console_irq $slave $intc

			variable alias_node_list
			global consoleip
			if {[string match -nocase $name $consoleip]} {
				lappend alias_node_list [list serial0 aliasref $name 0]
			} else {
				variable serial_count
				incr serial_count
				lappend alias_node_list [list serial$serial_count aliasref $name $serial_count]
			}

			set ip_tree [slaveip_intr $slave $intc [interrupt_list $slave] "serial" [default_parameters $slave] "" "" [list "ns16550a"] ]
			set ip_tree [tree_append $ip_tree [list "device_type" string "serial"]]
			set ip_tree [tree_append $ip_tree [list "current-speed" int "115200"]]

			# The 16550 cores usually use the bus clock as the baud
			# reference, but can also take an external reference clock.
			if { $type == "opb_uart16550"} {
				set freq [get_clock_frequency $slave "OPB_Clk"]
			} elseif { $type == "plb_uart16550"} {
				set freq [get_clock_frequency $slave "PLB_Clk"]
			} elseif { $type == "xps_uart16550"} {
				set freq [get_clock_frequency $slave "SPLB_Clk"]
			} elseif { $type == "axi_uart16550"} {
				set freq [get_clock_frequency $slave "S_AXI_ACLK"]
			}
			set has_xin [scan_int_parameter_value $slave "C_HAS_EXTERNAL_XIN"]
			if { $has_xin == "1" } {
				set freq [get_clock_frequency $slave "xin"]
			}
			set ip_tree [tree_append $ip_tree [list "clock-frequency" int $freq]]

			set ip_tree [tree_append $ip_tree [list "reg-shift" int "2"]]
			if { $type == "axi_uart16550"} {
				set ip_tree [tree_append $ip_tree [list "reg-offset" hexint [expr 0x1000]]]
			} else {
				set ip_tree [tree_append $ip_tree [list "reg-offset" hexint [expr 0x1003]]]
			}
			lappend node $ip_tree
			#"BAUDRATE DATA_BITS CLK_FREQ ODD_PARITY USE_PARITY"]
		}
		"ps7_uart" {
			set ip_tree [slaveip $slave $intc "serial" [default_parameters $slave] "S_AXI_" "xlnx,xuartps"]

			variable alias_node_list
			global consoleip
			if {[string match -nocase $name $consoleip]} {
				lappend alias_node_list [list serial0 aliasref $name 0]
				set ip_tree [tree_append $ip_tree [list "port-number" int 0]]
			} else {
				variable serial_count
				incr serial_count
				lappend alias_node_list [list serial$serial_count aliasref $name $serial_count]
				set ip_tree [tree_append $ip_tree [list "port-number" int $serial_count]]
			}

			# MS silly use just clock-frequency which is standard
			set ip_tree [tree_append $ip_tree [list "device_type" string "serial"]]
			set ip_tree [tree_append $ip_tree [list "current-speed" int "115200"]]
			set ip_tree [zynq_irq $ip_tree $intc $name]

			lappend node $ip_tree
		}
		"xps_timebase_wdt" -
		"axi_timebase_wdt" {
			set ip_tree [slaveip_intr $slave $intc [interrupt_list $slave] "" [default_parameters $slave] ]
			if { $type == "xps_timebase_wdt" } {
				set ip_tree [tree_append $ip_tree [list "clock-frequency" int [get_clock_frequency $slave "SPLB_Clk"]]]
			} elseif { $type == "axi_timebase_wdt" } {
				set ip_tree [tree_append $ip_tree [list "clock-frequency" int [get_clock_frequency $slave "S_AXI_ACLK"]]]
			}
			lappend node $ip_tree
		}
		"xps_timer" -
		"opb_timer" -
		"axi_timer" {
			global timer
			if {[ string match -nocase $name $timer ]} {
				set ip_tree [slaveip_intr $slave $intc [interrupt_list $slave] "system_timer" [default_parameters $slave] ]
				set one_timer_only [xget_hw_parameter_value $slave "C_ONE_TIMER_ONLY"]
				if { $one_timer_only == "1" } {
					error "Linux requires dual channel timer, but $name is set to single channel. Please configure the $name to dual channel"
				}
				set irq [get_intr $slave $intc "Interrupt"]
				if { $irq == "-1" } {
					error "Linux requires dual channel timer with interrupt connected. Please configure the $name to interrupt"
				}
				variable microblaze_system_timer
				set microblaze_system_timer $timer
			} else {
				set ip_tree [slaveip_intr $slave $intc [interrupt_list $slave] "timer" [default_parameters $slave] ]
			}

			# for version 1.01b of the xps timer, make sure that it has the patch applied to the h/w
			# so that it's using an edge interrupt rather than a falling as described in AR 33880
			# this is tracking a h/w bug in EDK 11.4 that should be fixed in the future

			set hw_ver [xget_hw_parameter_value $slave "HW_VER"]
			if { $hw_ver == "1.01.b" && $type == "xps_timer" } {
				set port_handle [xget_hw_port_handle $slave "Interrupt"]
				set sensitivity [xget_hw_subproperty_value $port_handle "SENSITIVITY"];
				if { [string compare -nocase $sensitivity "EDGE_RISING"] != 0 } {
					error "xps_timer version 1.01b must be patched to rising edge IRQ sensitivity. \
						Please see Xilinx Answer Record 33880 at http://www.xilinx.com/support/answers/33880.htm \
						and follow the instructions there."
				}
			}
			#"C_COUNT_WIDTH C_ONE_TIMER_ONLY"]

			# axi_timer runs at bus frequency, whereas plb and opb timers run at cpu fruquency. The timer driver
			# in microblaze kernel uses the 'clock-frequency' property, if there is one available; otherwise it
			# uses cpu frequency. For axi_timer, generate the 'clock-frequency' property with bus frequency as
			# it's value
			if { $type == "axi_timer"} {
				set freq [get_clock_frequency $slave "S_AXI_ACLK"]
				set ip_tree [tree_append $ip_tree [list "clock-frequency" int $freq]]
			}
			lappend node $ip_tree
		}
		"axi_sysace" -
		"xps_sysace" -
		"opb_sysace" {
			set ip_tree [slaveip_intr $slave $intc [interrupt_list $slave] "sysace" [default_parameters $slave] ]
			#"MEM_WIDTH"]
			set sysace_width [xget_hw_parameter_value $slave "C_MEM_WIDTH"]
			if { $sysace_width == "8" } {
				set ip_tree [tree_append $ip_tree [list "8-bit" empty empty]]
			} elseif { $sysace_width == "16" } {
				set ip_tree [tree_append $ip_tree [list "16-bit" empty empty]]
			} else {
				error "Unsuported Systemace memory width"
			}
			variable sysace_count
			set ip_tree [tree_append $ip_tree [list "port-number" int $sysace_count]]
			incr sysace_count
			lappend node $ip_tree
		}
		"opb_ethernet" -
		"plb_ethernet" -
		"opb_ethernetlite" -
		"xps_ethernetlite" -
		"axi_ethernetlite" -
		"plb_temac" {
			#
			# Add this temac channel to the alias list
			#
			variable ethernet_count
			variable alias_node_list
			lappend alias_node_list [list ethernet$ethernet_count aliasref $name $ethernet_count]
			incr ethernet_count

			# 'network' type
			set ip_tree [slaveip_intr $slave $intc [interrupt_list $slave] "ethernet" [default_parameters $slave]]
			set ip_tree [tree_append $ip_tree [list "device_type" string "network"]]
			set ip_tree [tree_append $ip_tree [list "local-mac-address" bytesequence [list 0x00 0x0a 0x35 0x00 0x00 $mac_count]]]
			incr mac_count

			if {$type == "xps_ethernetlite" || $type == "axi_ethernetlite"} {
				if {[parameter_exists $slave "C_INCLUDE_MDIO"]} {
					set has_mdio [scan_int_parameter_value $slave "C_INCLUDE_MDIO"]
					if {$has_mdio == 1} {
						set phy_name "phy$phy_count"
						set ip_tree [tree_append $ip_tree [list "phy-handle" labelref $phy_name]]
						set ip_tree [tree_append $ip_tree [gen_mdiotree $slave]]
					}
				}
			}

			lappend node $ip_tree
		}
		"xps_ll_temac" {
			# We need to handle this specially, to notify the driver
			# about the connected LL connection, and the dual cores.
			lappend node [slave_ll_temac $slave $intc]
		}
		"axi_ethernet_buffer" -
		"axi_ethernet" {
			set name [xget_hw_name $slave]
			set type [xget_hw_value $slave]
			set baseaddr [scan_int_parameter_value $slave "C_BASEADDR"]
			set highaddr [expr $baseaddr + 0x3ffff]

			variable ethernet_count
			variable alias_node_list
			set alias_node [list ethernet$ethernet_count aliasref $name $ethernet_count]
			lappend alias_node_list $alias_node
			incr ethernet_count

			set ip_tree [slaveip_basic $slave $intc "" [format_ip_name "axi-ethernet" $baseaddr $name]]
			set ip_tree [tree_append $ip_tree [list "device_type" string "network"]]
			set ip_tree [tree_append $ip_tree [list "local-mac-address" bytesequence [list 0x00 0x0a 0x35 0x00 0x00 $mac_count]]]
			incr mac_count
			set phy_name "phy$phy_count"
			set ip_tree [tree_append $ip_tree [list "phy-handle" labelref $phy_name]]

			set ip_tree [tree_append $ip_tree [gen_reg_property $name $baseaddr $highaddr]]
			set ip_tree [gen_interrupt_property $ip_tree $slave $intc [format "INTERRUPT"]]
			set ip_name [lindex $ip_tree 0]
			set ip_node [lindex $ip_tree 2]
			# Generate the common parameters.
			set ip_node [gen_params $ip_node $slave [list "C_PHY_TYPE" "C_TYPE" "C_PHYADDR" "C_INCLUDE_IO" "C_HALFDUP"]]
			set ip_node [gen_params $ip_node $slave [list "C_TXMEM" "C_RXMEM" "C_TXCSUM" "C_RXCSUM" "C_MCAST_EXTEND" "C_STATS" "C_AVB"]]
			set ip_node [gen_params $ip_node $slave [list "C_TXVLAN_TRAN" "C_RXVLAN_TRAN" "C_TXVLAN_TAG" "C_RXVLAN_TAG" "C_TXVLAN_STRP" "C_RXVLAN_STRP"]]
			set ip_tree [list $ip_name tree $ip_node]
			set mhs_handle [xget_hw_parent_handle $slave]
			# See what the axi ethernet is connected to.
			set axiethernet_busif_handle [xget_hw_busif_handle $slave "AXI_STR_RXD"]
			set axiethernet_name [xget_hw_value $axiethernet_busif_handle]
			set axiethernet_ip_handle [xget_hw_connected_busifs_handle $mhs_handle $axiethernet_name "TARGET"]
			set axiethernet_ip_handle_name [xget_hw_name $axiethernet_ip_handle]
			set connected_ip_handle [xget_hw_parent_handle $axiethernet_ip_handle]
			set connected_ip_name [xget_hw_name $connected_ip_handle]
			set connected_ip_type [xget_hw_value $connected_ip_handle]
			set ip_tree [tree_append $ip_tree [list "axistream-connected" labelref $connected_ip_name]]
			set ip_tree [tree_append $ip_tree [list "axistream-control-connected" labelref $connected_ip_name]]

			set freq [get_clock_frequency $slave "S_AXI_ACLK"]
			set ip_tree [tree_append $ip_tree [list "clock-frequency" int $freq]]

			set ip_tree [tree_append $ip_tree [gen_mdiotree $slave]]

			lappend node $ip_tree
		}
		"axi_dma" {
			set ip_tree [slaveip_intr $slave $intc [interrupt_list $slave] "" [default_parameters $slave]]
			set mhs_handle [xget_hw_parent_handle $slave]
			# See what the axi dma is connected to.
			set axidma_busif_handle [xget_hw_busif_handle $slave "M_AXIS_MM2S"]
			set axidma_name [xget_hw_value $axidma_busif_handle]
			set axidma_ip_handle [xget_hw_connected_busifs_handle $mhs_handle $axidma_name "TARGET"]
			set axidma_ip_handle_name [xget_hw_name $axidma_ip_handle]
			set connected_ip_handle [xget_hw_parent_handle $axidma_ip_handle]
			set connected_ip_name [xget_hw_name $connected_ip_handle]
			set connected_ip_type [xget_hw_value $connected_ip_handle]
			set ip_tree [tree_append $ip_tree [list "axistream-connected" labelref $connected_ip_name]]
			set ip_tree [tree_append $ip_tree [list "axistream-control-connected" labelref $connected_ip_name]]
			lappend node $ip_tree
		}
		# FIXME - this need to be check because can break axi ethernet implementation
		"axi_dma-merged" {
			set axiethernetfound 0
			variable dma_device_id
			set xdma "axi-dma"
			set mhs_handle [xget_hw_parent_handle $slave]
			set axidma_busif_handle [xget_hw_busif_handle $slave "M_AXIS_MM2S"]
			set axidma_name [xget_hw_value $axidma_busif_handle]
			set axidma_ip_handle [xget_hw_connected_busifs_handle $mhs_handle $axidma_name "TARGET"]
			set axidma_ip_handle_name [xget_hw_name $axidma_ip_handle]
			set connected_ip_handle [xget_hw_parent_handle $axidma_ip_handle]
			set connected_ip_name [xget_hw_name $connected_ip_handle]
			set connected_ip_type [xget_hw_value $connected_ip_handle]
			if {[string compare $connected_ip_type "axi_ethernet"] == 0} {
				set axiethernetfound 1
			}
			if {$axiethernetfound != 1} {
				set hw_name [xget_hw_name $slave]

				set baseaddr [scan_int_parameter_value $slave "C_BASEADDR"]
				set highaddr [scan_int_parameter_value $slave "C_HIGHADDR"]

				set mytree [list [format_ip_name "axidma" $baseaddr $hw_name] tree {}]

				set tx_chan [scan_int_parameter_value $slave "C_INCLUDE_MM2S"]
				if {$tx_chan == 1} {
					set chantree [dma_channel_config $xdma $baseaddr "MM2S" $intc $slave $dma_device_id]
					set mytree [tree_append $mytree $chantree]
				}

				set rx_chan [scan_int_parameter_value $slave "C_INCLUDE_S2MM"]
				if {$rx_chan == 1} {
					set chantree [dma_channel_config $xdma [expr $baseaddr + 0x30] "S2MM" $intc $slave $dma_device_id]
					set mytree [tree_append $mytree $chantree]
				}

				set mytree [tree_append $mytree [list \#size-cells int 1]]
				set mytree [tree_append $mytree [list \#address-cells int 1]]
				set mytree [tree_append $mytree [list compatible stringtuple [list "xlnx,axi-dma"]]]

				set stsctrl 1
				set sgdmamode1 1
				set sgdmamode [xget_hw_parameter_handle $slave "C_INCLUDE_SG"]
				if {$sgdmamode != ""} {
					set sgdmamode1 [scan_int_parameter_value $slave "C_INCLUDE_SG"]
					if {$sgdmamode1 == 0} {
						set stsctrl 0
						set mytree [tree_append $mytree [list "xlnx,sg-include-stscntrl-strm" hexint $stsctrl]]
					} else {
						set stsctrl [xget_hw_parameter_handle $slave "C_SG_INCLUDE_STSCNTRL_STRM"]
						if {$stsctrl != ""} {
							set stsctrl [scan_int_parameter_value $slave "C_SG_INCLUDE_STSCNTRL_STRM"]
						} else {
							set stsctrl 0
						}
						set mytree [tree_append $mytree [list "xlnx,sg-include-stscntrl-strm" hexint $stsctrl]]
					}
				} else {
					set stsctrl [xget_hw_parameter_handle $slave "C_SG_INCLUDE_STSCNTRL_STRM"]
					if {$stsctrl != ""} {
						set stsctrl [scan_int_parameter_value $slave "C_SG_INCLUDE_STSCNTRL_STRM"]
					} else {
						set stsctrl 0
					}
					set mytree [tree_append $mytree [list "xlnx,sg-include-stscntrl-strm" hexint $stsctrl]]
				}
				set mytree [tree_append $mytree [gen_ranges_property $slave $baseaddr $highaddr $baseaddr]]
				set mytree [tree_append $mytree [gen_reg_property $hw_name $baseaddr $highaddr]]

				lappend node $mytree
			}

			if {$axiethernetfound == 1} {
				if {[catch {lappend node [slaveip_intr $slave $intc [interrupt_list $slave] "" [default_parameters $slave] "" ]} {error}]} {
					debug warning $error
				}
			}
			incr dma_device_id
		}
		"axi_vdma" {
			variable vdma_device_id
			set xdma "axi-vdma"
			set hw_name [xget_hw_name $slave]

			set baseaddr [scan_int_parameter_value $slave "C_BASEADDR"]
			set highaddr [scan_int_parameter_value $slave "C_HIGHADDR"]

			set mytree [list [format_ip_name "axivdma" $baseaddr $hw_name] tree {}]
			set tx_chan [scan_int_parameter_value $slave "C_INCLUDE_MM2S"]
			if {$tx_chan == 1} {
				set chantree [dma_channel_config $xdma $baseaddr "MM2S" $intc $slave $vdma_device_id]
				set mytree [tree_append $mytree $chantree]
			}

			set rx_chan [scan_int_parameter_value $slave "C_INCLUDE_S2MM"]
			if {$rx_chan == 1} {
				set chantree [dma_channel_config $xdma [expr $baseaddr + 0x30] "S2MM" $intc $slave $vdma_device_id]
				set mytree [tree_append $mytree $chantree]
			}

			set mytree [tree_append $mytree [list \#size-cells int 1]]
			set mytree [tree_append $mytree [list \#address-cells int 1]]
			set mytree [tree_append $mytree [list compatible stringtuple [list "xlnx,axi-vdma"]]]

			set tmp [xget_hw_parameter_handle $slave "C_INCLUDE_SG"]

			if {$tmp != ""} {
				set tmp [scan_int_parameter_value $slave "C_INCLUDE_SG"]
				set mytree [tree_append $mytree [list "xlnx,include-sg" hexint $tmp]]
			} else {
				# older core always has SG
				set mytree [tree_append $mytree [list "xlnx,include-sg" hexint 1]]
			}

			set tmp [scan_int_parameter_value $slave "C_NUM_FSTORES"]
			set mytree [tree_append $mytree [list "xlnx,num-fstores" hexint $tmp]]

			set tmp [scan_int_parameter_value $slave "C_FLUSH_ON_FSYNC"]
			set mytree [tree_append $mytree [list "xlnx,flush-fsync" hexint $tmp]]

			set mytree [tree_append $mytree [gen_ranges_property $slave $baseaddr $highaddr $baseaddr]]
			set mytree [tree_append $mytree [gen_reg_property $hw_name $baseaddr $highaddr]]

			lappend node $mytree
			incr vdma_device_id
		}
		"axi_cdma" {
			set hw_name [xget_hw_name $slave]

			set baseaddr [scan_int_parameter_value $slave "C_BASEADDR"]
			set highaddr [scan_int_parameter_value $slave "C_HIGHADDR"]

			set mytree [list [format_ip_name "axicdma" $baseaddr $hw_name] tree {}]
			set namestring "dma-channel"
			set channame [format_name [format "%s@%x" $namestring $baseaddr]]

			set chan {}
			lappend chan [list compatible stringtuple [list "xlnx,axi-cdma-channel"]]
			set tmp [scan_int_parameter_value $slave "C_INCLUDE_DRE"]
			lappend chan [list "xlnx,include-dre" hexint $tmp]

			set tmp [scan_int_parameter_value $slave "C_USE_DATAMOVER_LITE"]
			lappend chan [list "xlnx,lite-mode" hexint $tmp]

			set tmp [scan_int_parameter_value $slave "C_M_AXI_DATA_WIDTH"]
			lappend chan [list "xlnx,datawidth" hexint $tmp]

			set tmp [scan_int_parameter_value $slave "C_M_AXI_MAX_BURST_LEN"]
			lappend chan [list "xlnx,max-burst-len" hexint $tmp]


			set chantree [list $channame tree $chan]
			set chantree [gen_interrupt_property $chantree $slave $intc [list "cdma_introut"]]

			set mytree [tree_append $mytree $chantree]

			set mytree [tree_append $mytree [list \#size-cells int 1]]
			set mytree [tree_append $mytree [list \#address-cells int 1]]
			set mytree [tree_append $mytree [list compatible stringtuple [list "xlnx,axi-cdma"]]]

			set tmp [scan_int_parameter_value $slave "C_INCLUDE_SG"]
			set mytree [tree_append $mytree [list "xlnx,include-sg" hexint $tmp]]

			set mytree [tree_append $mytree [gen_ranges_property $slave $baseaddr $highaddr $baseaddr]]
			set mytree [tree_append $mytree [gen_reg_property $hw_name $baseaddr $highaddr]]

			lappend node $mytree
		}
		"axi_tft" -
		"xps_tft" {
			lappend node [slaveip_dcr_or_plb $slave $intc "tft" [default_parameters $slave]]
		}
		"logi3d" -
		"logiwin" -
		"logibmp" {
			lappend node [slaveip_intr $slave $intc [interrupt_list $slave] "" "[default_parameters $slave]" "REGS_"]
		}
		"logibayer" -
		"logicvc" {
			set params "C_VMEM_BASEADDR C_VMEM_HIGHADDR"
			lappend node [slaveip_intr $slave $intc [interrupt_list $slave] "" "[default_parameters $slave] $params" "REGS_"]
		}
		"logibitblt" {
			set params "C_BB_BASEADDR C_BB_HIGHADDR"
			lappend node [slaveip_intr $slave $intc [interrupt_list $slave] "" "[default_parameters $slave] $params" "REGS_"]
		}

		"plb_tft_cntlr_ref" -
		"plb_dvi_cntlr_ref" {
			# We handle this specially, since it is a DCR slave.
			lappend node [slaveip_dcr $slave $intc "tft" [default_parameters $slave] "DCR_"]
		}
		"opb_ps2_dual_ref" {
			# We handle this specially, to report the two independent
			# ports.
			set tree [compound_slave $slave]
			set baseaddr [scan_int_parameter_value $slave "C_BASEADDR"]
			set highaddr [scan_int_parameter_value $slave "C_HIGHADDR"]
			set tree [tree_append $tree [gen_ranges_property $slave $baseaddr $highaddr 0]]
			set tree [tree_append $tree [slaveip_in_compound_intr $slave $intc "Sys_Intr1" "ps2" "" 0 0x1000 0x40]]
			set tree [tree_append $tree [slaveip_in_compound_intr $slave $intc "Sys_Intr2" "ps2" "" 1 0x1000 0x40]]
			lappend node $tree
		}
		"xps_ps2" {
			set baseaddr [scan_int_parameter_value $slave "C_BASEADDR"]
			set highaddr [scan_int_parameter_value $slave "C_HIGHADDR"]
			set is_dual [scan_int_parameter_value $slave "C_IS_DUAL"]

			if {$is_dual == 1} {
				# We handle this specially, to report the two independent
				# ports.
				set tree [compound_slave $slave]
				set tree [tree_append $tree [gen_ranges_property $slave $baseaddr $highaddr 0]]
				set tree [tree_append $tree [slaveip_in_compound_intr $slave $intc "IP2INTC_Irpt_1" "ps2" "" 0 0x1000 0x40]]
				set tree [tree_append $tree [slaveip_in_compound_intr $slave $intc "IP2INTC_Irpt_2" "ps2" "" 1 0x1000 0x40]]
				lappend node $tree
			} else {
				lappend node [slaveip_intr $slave $intc "IP2INTC_Irpt_1" "ps2" ""]
			}
		}
		"opb_ac97_controller_ref" {
			# We should handle this specially, to report the two
			# interrupts in the right order.
			lappend node [slaveip_intr $slave $intc "Playback_Interrupt Record_Interrupt" "ac97" ""]
		}
		"opb_gpio" -
		"xps_gpio" -
		"axi_gpio" {
			# save gpio names and width for gpio reset code
			global gpio_names
			lappend gpio_names [list [xget_hw_name $slave] [scan_int_parameter_value $slave "C_GPIO_WIDTH"]]
			# We should handle this specially, to report two ports.
			set ip_tree [slaveip_intr $slave $intc [interrupt_list $slave] "gpio" [default_parameters $slave]]
			set ip_tree [tree_append $ip_tree [list "#gpio-cells" int "2"]]
			set ip_tree [tree_append $ip_tree [list "gpio-controller" empty empty]]
			lappend node $ip_tree
		}
		"opb_iic" -
		"xps_iic" -
		"axi_iic" {
			# We should handle this specially, to report two ports.
			lappend node [slaveip_intr $slave $intc [interrupt_list $slave] "i2c" [default_parameters $slave]]
		}
		"xps_spi" -
		"axi_quad_spi" -
		"axi_spi" {
			# We will handle SPI FLASH here
			global flash_memory flash_memory_bank
			set tree [slaveip_intr $slave $intc [interrupt_list $slave] "spi" [default_parameters $slave] "" ]

			if {[string match -nocase $flash_memory $name]} {
				# Add the address-cells and size-cells to make the DTC compiler stop outputing warning
				set tree [tree_append $tree [list "#address-cells" int "1"]]
				set tree [tree_append $tree [list "#size-cells" int "0"]]
				# If it is a SPI FLASH, we will add a SPI Flash
				# subnode to the SPI controller
				set subnode {}
				# Set the SPI Flash chip select
				lappend subnode [list "reg" hexinttuple [list $flash_memory_bank]]
				# Set the SPI Flash clock freqeuncy
				if { $type == "xps_spi" } {
					set sys_clk [get_clock_frequency $slave "SPLB_Clk"]
				} else {
					set sys_clk [get_clock_frequency $slave "S_AXI_ACLK"]
				}
				set sck_ratio [scan_int_parameter_value $slave "C_SCK_RATIO"]
				set sck [expr { $sys_clk / $sck_ratio }]
				lappend subnode [list [format_name "spi-max-frequency"] int $sck]
				set tree [tree_append $tree [list [format_ip_name $type $flash_memory_bank "primary_flash"] tree $subnode]]
			}
			lappend node $tree
		}
		"xps_usb_host" {
			lappend node [slaveip_intr $slave $intc [interrupt_list $slave] "usb" [default_parameters $slave] "SPLB_" "" [list "usb-ehci"]]
		}
		"ps7_dma" {
			set ip_tree [slaveip $slave $intc "" [default_parameters $slave] "S_AXI_" "arm,primecell arm,pl330"]
			# use TCL table
			set ip_tree [zynq_irq $ip_tree $intc $name]
			set ip_tree [tree_append $ip_tree [list "#dma-cells" int "1"]]
			set ip_tree [tree_append $ip_tree [list "#dma-channels" int "8"]]
			set ip_tree [tree_append $ip_tree [list "#dma-requests" int "4"]]
			set ip_tree [tree_append $ip_tree [list "arm,primecell-periphid" hexint "0x00041330"]];

			lappend node $ip_tree
		}
		"ps7_slcr" {
			set ip_tree [slaveip $slave $intc "" [default_parameters $slave] "S_AXI_" "xlnx,zynq-slcr"]
			# use TCL table
			set ip_tree [zynq_irq $ip_tree $intc $name]

			set clock_tree [list "clocks" tree {}]
			set clock_tree [tree_append $clock_tree [list "#address-cells" int "1"]]
			set clock_tree [tree_append $clock_tree [list "#size-cells" int "0"]]

			# PS_CLK node creation
			set subclk_tree [list "ps_clk: ps_clk" tree {}]
			set subclk_tree [tree_append $subclk_tree [list "#clock-cells" int "0"]]
			set subclk_tree [tree_append $subclk_tree [list "compatible" stringtuple "fixed-clock"]]
			set subclk_tree [tree_append $subclk_tree [list "clock-output-names" stringtuple "ps_clk"]]
			set subclk_tree [tree_append $subclk_tree [list "clock-frequency" int "33333333"]]
			set clock_tree [tree_append $clock_tree $subclk_tree]

			set subclk_tree [list "armpll: armpll" tree {}]
			set subclk_tree [tree_append $subclk_tree [list "#clock-cells" int "0"]]
			set subclk_tree [tree_append $subclk_tree [list "compatible" stringtuple "xlnx,zynq-pll"]]
			set subclk_tree [tree_append $subclk_tree [list "clocks" labelref "ps_clk"]]
			set subclk_tree [tree_append $subclk_tree [list "reg" hexinttuple [list "0x100" "0x110" "0x10c"]]]
			set subclk_tree [tree_append $subclk_tree [list "lockbit" int "0"]]
			set subclk_tree [tree_append $subclk_tree [list "clock-output-names" stringtuple "armpll"]]
			set clock_tree [tree_append $clock_tree $subclk_tree]

			set subclk_tree [list "ddrpll: ddrpll" tree {}]
			set subclk_tree [tree_append $subclk_tree [list "#clock-cells" int "0"]]
			set subclk_tree [tree_append $subclk_tree [list "compatible" stringtuple "xlnx,zynq-pll"]]
			set subclk_tree [tree_append $subclk_tree [list "clocks" labelref "ps_clk"]]
			set subclk_tree [tree_append $subclk_tree [list "reg" hexinttuple [list "0x104" "0x114" "0x10c"]]]
			set subclk_tree [tree_append $subclk_tree [list "lockbit" int "1"]]
			set subclk_tree [tree_append $subclk_tree [list "clock-output-names" stringtuple "ddrpll"]]
			set clock_tree [tree_append $clock_tree $subclk_tree]

			set subclk_tree [list "iopll: iopll" tree {}]
			set subclk_tree [tree_append $subclk_tree [list "#clock-cells" int "0"]]
			set subclk_tree [tree_append $subclk_tree [list "compatible" stringtuple "xlnx,zynq-pll"]]
			set subclk_tree [tree_append $subclk_tree [list "clocks" labelref "ps_clk"]]
			set subclk_tree [tree_append $subclk_tree [list "reg" hexinttuple [list "0x108" "0x118" "0x10c"]]]
			set subclk_tree [tree_append $subclk_tree [list "lockbit" int "2"]]
			set subclk_tree [tree_append $subclk_tree [list "clock-output-names" stringtuple "iopll"]]
			set clock_tree [tree_append $clock_tree $subclk_tree]

			set ip_tree [tree_append $ip_tree $clock_tree]

			lappend node $ip_tree
		}
		"ps7_can" -
		"ps7_iop_bus_config" -
		"ps7_qspi_linear" -
		"ps7_ddrc" -
		"ps7_dev_cfg" {
			set ip_tree [slaveip $slave $intc "" [default_parameters $slave] "S_AXI_" ""]
			# use TCL table
			set ip_tree [zynq_irq $ip_tree $intc $name]

			lappend node $ip_tree
		}
		"ps7_gpio" {
			set count 32
			set ip_tree [slaveip $slave $intc "" "" "S_AXI_" ""]
			set ip_tree [tree_append $ip_tree [list "emio-gpio-width" int [xget_sw_parameter_value $slave "C_EMIO_GPIO_WIDTH"]]]
			set gpiomask [xget_sw_parameter_value $slave "C_MIO_GPIO_MASK"]
			set mask [expr {$gpiomask & 0xffffffff}]
			set ip_tree [tree_append $ip_tree [list "gpio-mask-low" hexint $mask]]
			set mask [expr {$gpiomask>>$count}]
			set mask [expr {$mask & 0xffffffff}]
			set ip_tree [tree_append $ip_tree [list "gpio-mask-high" hexint $mask]]
			set ip_tree [zynq_irq $ip_tree $intc $name]

			set ip_tree [tree_append $ip_tree [list "#gpio-cells" int "2"]]
			set ip_tree [tree_append $ip_tree [list "gpio-controller" empty empty]]

			lappend node $ip_tree
		}
		"ps7_i2c" {
			set ip_tree [slaveip $slave $intc "" [default_parameters $slave] "S_AXI_" ""]
			# use TCL table
			set ip_tree [zynq_irq $ip_tree $intc $name]

			variable ps7_i2c_count
			variable ps7_cortexa9_clk
			set ip_tree [tree_append $ip_tree [list "input-clk" int [expr $ps7_cortexa9_clk/6]]]
			set ip_tree [tree_append $ip_tree [list "i2c-clk" int 400000]]
			set ip_tree [tree_append $ip_tree [list "bus-id" int $ps7_i2c_count]]
			incr ps7_i2c_count

			lappend node $ip_tree
		}
		"ps7_ttc" {
			set ip_tree [slaveip $slave $intc "" "" "S_AXI_" "cdns,ttc"]
			# use TCL table
			set ip_tree [zynq_irq $ip_tree $intc $name]

			lappend node $ip_tree
		}
		"ps7_scutimer" {
			set ip_tree [slaveip $slave $intc "" [default_parameters $slave] "S_AXI_" ""]
			# use TCL table
			set ip_tree [zynq_irq $ip_tree $intc $name]

			set ip_tree [slaveip $slave $intc "" [default_parameters $slave] "S_AXI_" "arm,cortex-a9-twd-timer"]
			set ip_tree [zynq_irq $ip_tree $intc $name]

			lappend node $ip_tree
		}
		"ps7_qspi" {
			set ip_tree [slaveip $slave $intc "" [default_parameters $slave] "S_AXI_" ""]
			# use TCL table
			set ip_tree [zynq_irq $ip_tree $intc $name]

			variable ps7_spi_count
			set ip_tree [tree_append $ip_tree [list "speed-hz" int [xget_sw_parameter_value $slave "C_QSPI_CLK_FREQ_HZ"]]]
			set ip_tree [tree_append $ip_tree [list "bus-num" int $ps7_spi_count]]
			set ip_tree [tree_append $ip_tree [list "num-chip-select" int 1]]
			set qspi_mode [xget_sw_parameter_value $slave "C_QSPI_MODE"]
			if { $qspi_mode == 2} {
				set is_dual 1
			} else {
				set is_dual 0
			}
			set ip_tree [tree_append $ip_tree [list "is-dual" int $is_dual]]
			incr ps7_spi_count

			# We will handle SPI FLASH here
			global flash_memory flash_memory_bank

			if {[string match -nocase $flash_memory $name]} {
				# Add the address-cells and size-cells to make the DTC compiler stop outputing warning
				set ip_tree [tree_append $ip_tree [list "#address-cells" int "1"]]
				set ip_tree [tree_append $ip_tree [list "#size-cells" int "0"]]
				# If it is a SPI FLASH, we will add a SPI Flash
				# subnode to the SPI controller
				set subnode {}
				# Set the SPI Flash chip select
				lappend subnode [list "reg" hexinttuple [list $flash_memory_bank]]
				# Set the SPI Flash clock frequency, assume it will be
				# 1/4 of the QSPI controller frequency.
				# Note this is not the actual maximum SPI flash frequency
				# as we can't know.
				lappend subnode [list [format_name "spi-max-frequency"] int [expr [xget_sw_parameter_value $slave "C_QSPI_CLK_FREQ_HZ"]/4]]
				set ip_tree [tree_append $ip_tree [list [format_ip_name $type $flash_memory_bank "primary_flash"] tree $subnode]]
			}

			lappend node $ip_tree
		}
		"ps7_wdt" {
			set ip_tree [slaveip $slave $intc "" [default_parameters $slave] "S_AXI_" ""]
			# use TCL table
			set ip_tree [zynq_irq $ip_tree $intc $name]

			set ip_tree [tree_append $ip_tree [list "device_type" string "watchdog"]]
			set ip_tree [tree_append $ip_tree [list "reset" int 0]]
			set ip_tree [tree_append $ip_tree [list "timeout" int 10]]

			lappend node $ip_tree
		}
		"ps7_scuwdt" {
			set ip_tree [slaveip $slave $intc "" [default_parameters $slave] "S_AXI_" ""]
			# use TCL table
			set ip_tree [zynq_irq $ip_tree $intc $name]

			set ip_tree [tree_append $ip_tree [list "device_type" string "watchdog"]]

			lappend node $ip_tree
		}
		"ps7_usb" {
			set ip_tree [slaveip $slave $intc "" [default_parameters $slave] "S_AXI_" ""]
			# use TCL table
			set ip_tree [zynq_irq $ip_tree $intc $name]

			set ip_tree [tree_append $ip_tree [list "dr_mode" string "host"]]
			set ip_tree [tree_append $ip_tree [list "phy_type" string "ulpi"]]

			lappend node $ip_tree
		}
		"ps7_spi" {
			set ip_tree [slaveip $slave $intc "" [default_parameters $slave] "S_AXI_" ""]
			# use TCL table
			set ip_tree [zynq_irq $ip_tree $intc $name]

			variable ps7_spi_count
			set ip_tree [tree_append $ip_tree [list "speed-hz" int [xget_sw_parameter_value $slave "C_SPI_CLK_FREQ_HZ"]]]
			set ip_tree [tree_append $ip_tree [list "bus-num" int $ps7_spi_count]]
			set ip_tree [tree_append $ip_tree [list "num-chip-select" int 4]]
			incr ps7_spi_count
			# We will handle SPI FLASH here
			global flash_memory flash_memory_bank

			if {[string match -nocase $flash_memory $name]} {
				# Add the address-cells and size-cells to make the DTC compiler stop outputing warning
				set ip_tree [tree_append $ip_tree [list "#address-cells" int "1"]]
				set ip_tree [tree_append $ip_tree [list "#size-cells" int "0"]]
				# If it is a SPI FLASH, we will add a SPI Flash
				# subnode to the SPI controller
				set subnode {}
				# Set the SPI Flash chip select
				lappend subnode [list "reg" hexinttuple [list $flash_memory_bank]]
				# Set the SPI Flash clock freqeuncy
				# hardcode this spi-max-frequency (based on board_zc770_xm010.c)
				lappend subnode [list [format_name "spi-max-frequency"] int 75000000]
				set ip_tree [tree_append $ip_tree [list [format_ip_name $type $flash_memory_bank "primary_flash"] tree $subnode]]
			}

			lappend node $ip_tree
		}
		"ps7_sdio" {
			set ip_tree [slaveip $slave $intc "" [default_parameters $slave] "S_AXI_" "generic-sdhci"]
			# FIXME linux sdhci requires clock-frequency even if we use common clock framework
			set ip_tree [tree_append $ip_tree [list "clock-frequency" int [xget_sw_parameter_value $slave "C_SDIO_CLK_FREQ_HZ"]]]
			set ip_tree [zynq_irq $ip_tree $intc $name]
			lappend node $ip_tree
		}
		"ps7_smcc" {
			set ip_tree [slaveip $slave $intc "" [default_parameters $slave] "S_AXI_" "xlnx,ps7-smc"]

			# use TCL table
			set ip_tree [zynq_irq $ip_tree $intc $type]

			variable ps7_smcc_list
			if {![string match "" $ps7_smcc_list]} {
				set ip_tree [tree_append $ip_tree [list "#address-cells" int "1"]]
				set ip_tree [tree_append $ip_tree [list "#size-cells" int "1"]]
				set ip_tree [tree_append $ip_tree [list ranges empty empty]]

				set ip_tree [tree_append $ip_tree $ps7_smcc_list]
			}

			lappend node $ip_tree
		}
		"ps7_nand" {
			# just C_S_AXI_BASEADDR  C_S_AXI_HIGHADDR C_NAND_CLK_FREQ_HZ C_NAND_MODE C_INTERCONNECT_S_AXI_MASTERS HW_VER INSTANCE
			set ip_tree [slaveip $slave $intc "" [default_parameters $slave] "S_AXI_" ""]

			# FIXME: set reg size to 16MB. This is a workaround for 14.4
			# tools provides the wrong high address of NAND
			set baseaddr [scan_int_parameter_value $slave "C_S_AXI_BASEADDR"]
			set ip_tree [tree_node_update $ip_tree "reg" [list "reg" hexinttuple [list $baseaddr "16777216" ]]]

			global flash_memory
			if {[ string match -nocase $name $flash_memory ]} {
				set ip_tree [change_nodename $ip_tree $name "primary_flash"]
			}

			variable ps7_smcc_list

			set ps7_smcc_list "$ps7_smcc_list $ip_tree"
		}
		"ps7_nor" -
		"ps7_sram" {
			# NOTE: For 14.4, the ps7_sram_* is refer to NOR flash not SRAM
			global flash_memory
			if {[ string match -nocase $name $flash_memory ]} {
				set ip_tree [slaveip $slave $intc "flash" [default_parameters $slave] "S_AXI_" "cfi-flash"]
				set ip_tree [change_nodename $ip_tree $name "primary_flash"]
			} else {
				set ip_tree [slaveip $slave $intc "" [default_parameters $slave] "S_AXI_" "cfi-flash"]
			}

			set ip_tree [tree_append $ip_tree [list "bank-width" int 1]]

			regsub -all "ps7_sram" $ip_tree "ps7_nor" ip_tree
			regsub -all "ps7-sram" $ip_tree "ps7-nor" ip_tree

			variable ps7_smcc_list
			set ps7_smcc_list "$ps7_smcc_list $ip_tree"
		}
		"ps7_scugic" {
			# FIXME this node should be provided by SDK and not to compose it by hand

			# just test code to show all interrupts
#			set port_handles [xget_hw_port_handle $slave "*"]
#			foreach i $port_handles {
#				set signals [xget_hw_port_value $slave [xget_hw_name $i]]
#				puts "$i [xget_hw_name $i] -- $signals --"
#			}

			# Replace _ with - in type to be compatible
			regsub -all "_" $type "-" type

			# Add interrupt distributor because it is not detected
			set tree [list "$name: $type@f8f01000" tree \
					[list \
						[gen_compatible_property $name $type [xget_hw_parameter_value $slave "HW_VER"] "arm,cortex-a9-gic arm,gic" ] \
						[list "reg" hexinttuple [list "0xF8F01000" "0x1000" "0xF8F00100" "0x100"] ] \
						[list "#interrupt-cells" inttuple "3" ] \
						[list "#address-cells" inttuple "2" ] \
						[list "#size-cells" inttuple "1" ] \
						[list "interrupt-controller" empty empty ] \
						[list "linux,phandle" hexinttuple "0x1" ] \
						[list "phandle" hexinttuple "0x1" ] \
					] \
				]
			lappend node $tree

#			lappend node [gen_intc $slave "" "interrupt-controller" [default_parameters $slave] "S_AXI_" "arm,gic"]
		}
		"ps7_pl310" {
			set tree [list "ps7_pl310_0: ps7-pl310@f8f02000" tree \
					[list \
						[gen_compatible_property "ps7_pl310" "ps7_pl310" "1.00.a" "arm,pl310-cache" ] \
						[list "cache-unified" empty empty ] \
						[list "cache-level" inttuple "2" ] \
						[list "reg" hexinttuple [list "0xF8F02000" "0x1000"] ] \
						[list "arm,data-latency" inttuple [list "3" "2" "2"] ] \
						[list "arm,tag-latency" inttuple [list "2" "2" "2"] ] \
					] \
				]
			set tree [zynq_irq $tree $intc $name]
			lappend node $tree
		}
		"ps7_xadc" {
			set tree [list "ps7_xadc: ps7-xadc@f8007100" tree \
					[list \
						[gen_compatible_property "ps7_xadc" "ps7_xadc" "1.00.a" ] \
						[list "reg" hexinttuple [list "0xF8007100" "0x20"] ] \
					] \
				]
			set tree [zynq_irq $tree $intc $name]
			lappend node $tree
		}
		"ps7_trace" -
		"ps7_ddr" {
			# Do nothing
		}
		"ps7_ethernet" {
			variable ethernet_count
			variable alias_node_list
			set alias_node [list ethernet$ethernet_count aliasref $name $ethernet_count]
			lappend alias_node_list $alias_node
			incr ethernet_count

			set ip_tree [slaveip $slave $intc "" [default_parameters $slave] "S_AXI_" ""]
			set ip_tree [zynq_irq $ip_tree $intc $name]
			set ip_tree [tree_append $ip_tree [list "local-mac-address" bytesequence [list 0x00 0x0a 0x35 0x00 0x00 $mac_count]]]
			incr mac_count

			set ip_tree [tree_append $ip_tree [list "#address-cells" int "1"]]
			set ip_tree [tree_append $ip_tree [list "#size-cells" int "0"]]
			set phy_name "phy$phy_count"
			set ip_tree [tree_append $ip_tree [list "phy-handle" labelref $phy_name]]

			set mdio_tree [list "mdio" tree {}]
			set mdio_tree [tree_append $mdio_tree [list \#size-cells int 0]]
			set mdio_tree [tree_append $mdio_tree [list \#address-cells int 1]]
			set phya 7
			set phy_chip "marvell,88e1116r"
			set mdio_tree [tree_append $mdio_tree [gen_phytree $slave $phya $phy_chip]]

			set phya [is_gmii2rgmii_conv_present $slave]
			if { $phya != "-1" } {
				set phy_name "phy$phy_count"
				set ip_tree [tree_append $ip_tree [list "gmii2rgmii-phy-handle" labelref $phy_name]]
				set phy_chip "xlnx,gmii2rgmii"
				set mdio_tree [tree_append $mdio_tree [gen_phytree $slave $phya $phy_chip]]
			}
			set ip_tree [tree_append $ip_tree $mdio_tree]

			variable ps7_cortexa9_1x_clk
			set ip_tree [tree_append $ip_tree [list "xlnx,ptp-enet-clock" int $ps7_cortexa9_1x_clk]]

			set phymode [scan_int_parameter_value $slave "C_ETH_MODE"]
			if { $phymode == 0 } {
				set ip_tree [tree_append $ip_tree [list "phy-mode" string "gmii"]]
			} else {
				set ip_tree [tree_append $ip_tree [list "phy-mode" string "rgmii-id"]]
			}

			lappend node $ip_tree
		}
		"axi_fifo_mm_s" {
			set ip_tree [slaveip_intr $slave $intc [interrupt_list $slave] "" [default_parameters $slave]]
			lappend node $ip_tree
		}
		"ps7_ram" {
			if {"$name" == "ps7_ram_0"} {
				set ip_tree [slaveip $slave $intc "" "" "S_AXI_" "xlnx,ps7-ocm"]
				set ip_tree [tree_node_update $ip_tree "reg" [list "reg" hexinttuple [list "0xfffc0000" "262144" ]]]
				# use TCL table
				set ip_tree [zynq_irq $ip_tree $intc $name]

				lappend node $ip_tree
			}
		}
		"plb_bram_if_cntlr" -
		"opb_bram_if_cntlr" -
		"axi_bram_ctrl" -
		"opb_cypress_usb" -
		"plb_ddr" -
		"plb_ddr2" -
		"opb_sdram" -
		"opb_ddr" -
		"mch_opb_ddr" -
		"mch_opb_ddr2" -
		"mch_opb_sdram" -
		"ppc440mc_ddr2" -
		"axi_s6_ddrx" -
		"axi_v6_ddrx" -
		"axi_7series_ddrx" -
		"mig_7series" {
			# Do nothing..  this is handled by the 'memory' special case.
		}
		"opb_emc" -
		"plb_emc" -
		"mch_opb_emc" -
		"xps_mch_emc" {
			global main_memory main_memory_bank
			# Handle flash memories with 'banks'. Generate one flash node
			# for each bank, if necessary.  If not connected to flash,
			# then do nothing.
			set count [scan_int_parameter_value $slave "C_NUM_BANKS_MEM"]
			if { [llength $count] == 0 } {
				set count 1
			}
			for {set x 0} {$x < $count} {incr x} {

				# Make sure we didn't already register this guy as the main memory.
				# see main handling in gen_memories
				if {[ string match -nocase $name $main_memory ] && $x == $main_memory_bank } {
					continue;
				}
				global flash_memory flash_memory_bank
				set baseaddr_prefix [format "MEM%d_" $x]
				set tree [slaveip_intr $slave $intc [interrupt_list $slave] "flash" [default_parameters $slave] $baseaddr_prefix "" "cfi-flash"]

				# Flash needs a bank-width attribute.
				set datawidth [scan_int_parameter_value $slave [format "C_%sWIDTH" $baseaddr_prefix]]
				set tree [tree_append $tree [list "bank-width" int "[expr ($datawidth/8)]"]]

				# If it is a set as the system Flash memory, change the name of this node to PetaLinux standard system Flash emmory name
				if {[ string match -nocase $name $flash_memory ] && $x == $flash_memory_bank} {
					set tree [change_nodename $tree $name "primary_flash"]
				}
				lappend node $tree
			}
		}
		"axi_emc" {
			# Handle flash memories with 'banks'. Generate one flash node
			# for each bank, if necessary.  If not connected to flash,
			# then do nothing.
			set count [scan_int_parameter_value $slave "C_NUM_BANKS_MEM"]
			if { [llength $count] == 0 } {
				set count 1
			}
			for {set x 0} {$x < $count} {incr x} {

				set synch_mem [scan_int_parameter_value $slave [format "C_MEM%d_TYPE" $x]]
				# C_MEM$x_TYPE = 2 or 3 indicates the bank handles
				# a flash device and it should be listed as a
				# slave in fdt.
				# C_MEM$x_TYPE = 0, 1 or 4 indicates the bank handles
				# SRAM and it should be listed as a memory in
				# fdt.

				global main_memory main_memory_bank
				# Make sure we didn't already register this guy as the main memory.
				# see main handling in gen_memories
				if {[ string match -nocase $name $main_memory ] && $x == $main_memory_bank } {
					if { $synch_mem == 0 || $synch_mem == 1 || $synch_mem == 4 } {
						continue;
					}
				}

				set baseaddr_prefix [format "S_AXI_MEM%d_" $x]
				if { $synch_mem == 2 || $synch_mem == 3 } {
					set tree [slaveip_intr $slave $intc [interrupt_list $slave] "flash" [default_parameters $slave] $baseaddr_prefix "" "cfi-flash"]
				} else {
					set tree [slaveip_intr $slave $intc [interrupt_list $slave] "memory" [default_parameters $slave] $baseaddr_prefix "" ""]
				}

				# Flash needs a bank-width attribute.
				set datawidth [scan_int_parameter_value $slave [format "C_MEM%d_WIDTH" $x]]
				set tree [tree_append $tree [list "bank-width" int "[expr ($datawidth/8)]"]]

				# If it is a set as the system Flash memory, change the name of this node to PetaLinux standard system Flash emmory name
				global flash_memory flash_memory_bank
				if {[ string match -nocase $name $flash_memory ] && $x == $flash_memory_bank} {
					set tree [change_nodename $tree $name "primary_flash"]
				}
				lappend node $tree
			}
		}
		"mpmc" {
			# We should handle this specially, to report the DMA
			# ports.  This is a hack that happens to work for the
			# design I have.  Note that we don't use the default
			# parameters here because of the slew of parameters the
			# mpmc has.
			lappend node [slave_mpmc $slave $intc]
		}
		"opb2plb_bridge" {
			# Hmm.. how do we represent this?
			#	lappend node [bus_bridge $slave $intc "MPLB" "C_RNG"]
		}
		"plb2opb_bridge" -
		"plbv46_opb_bridge" {
			set baseaddr [scan_int_parameter_value $slave "C_RNG0_BASEADDR"]
			set tree [bus_bridge $slave $intc $baseaddr "MOPB"]
			set ranges_list [default_ranges $slave "C_NUM_ADDR_RNG" "C_RNG%d_BASEADDR" "C_RNG%d_HIGHADDR"]
			set tree [tree_append $tree [gen_ranges_property_list $slave $ranges_list]]
			lappend node $tree
		}
		"plbv46_axi_bridge" {
			# Fix me -- how do we represent this?
		}
		"axi_plbv46_bridge" {
			set baseaddr [scan_int_parameter_value $slave "C_S_AXI_RNG1_BASEADDR"]
			set tree [bus_bridge $slave $intc $baseaddr "MPLB"]
			set ranges_list [default_ranges $slave "C_S_AXI_NUM_ADDR_RANGES" "C_S_AXI_RNG%d_BASEADDR" "C_S_AXI_RNG%d_HIGHADDR" "1"]
			set tree [tree_append $tree [gen_ranges_property_list $slave $ranges_list]]
			lappend node $tree
		}
		"plbv46_plbv46_bridge" {
			# FIXME: multiple ranges!
			set baseaddr [scan_int_parameter_value $slave "C_RNG0_BASEADDR"]
			set tree [bus_bridge $slave $intc $baseaddr "MPLB"]
			set ranges_list [default_ranges $slave "C_NUM_ADDR_RNG" "C_RNG%d_BASEADDR" "C_RNG%d_HIGHADDR"]
			set tree [tree_append $tree [gen_ranges_property_list $slave $ranges_list]]
			lappend node $tree
		}
		"opb_opb_lite" {
			# FIXME: multiple ranges!
			set baseaddr [scan_int_parameter_value $slave "C_DEC0_BASEADDR"]
			set tree [bus_bridge $slave $intc $baseaddr "MOPB"]
			set ranges_list [default_ranges $slave "C_NUM_DECODES" "C_DEC%d_BASEADDR" "C_DEC%d_HIGHADDR"]
			set tree [tree_append $tree [gen_ranges_property_list $slave $ranges_list]]
			lappend node $tree
		}
		"opb2dcr_bridge" -
		"plbv46_dcr_bridge" {
			set baseaddr [scan_int_parameter_value $slave "C_BASEADDR"]
			set highaddr [scan_int_parameter_value $slave "C_HIGHADDR"]
			set slavetree [slaveip_intr $slave $intc [interrupt_list $slave] "" [default_parameters $slave] ""]
			set slavetree [tree_append $slavetree [list dcr-controller empty empty]]
			set slavetree [tree_append $slavetree [list dcr-access-method string mmio]]
			set slavetree [tree_append $slavetree [list dcr-mmio-stride int 4]]
			set slavetree [tree_append $slavetree [gen_reg_property $name $baseaddr $highaddr "dcr-mmio-range"]]
			lappend node $slavetree
			set tree [bus_bridge $slave $intc 0 "MDCR"]

			# Backward compatibility to not break older style tft driver
			# connected through opb2dcr bridge.
			set ranges [gen_ranges_property $slave $baseaddr $highaddr 0]
			set tree [tree_append $tree $ranges]

			lappend node $tree
		}
		"axi_pcie" {
			set ip_tree [slaveip_intr $slave $intc [interrupt_list $slave] "" [default_parameters $slave] ]
			set ip_tree [tree_append $ip_tree [list \#address-cells int 3]]
			set ip_tree [tree_append $ip_tree [list \#size-cells int 2]]
			# 64-bit high address.
			set high_64bit 0x00000000
			set ranges {}
			set ranges_list [axipcie_ranges $slave "C_AXIBAR_NUM" "C_AXIBAR_%d" "C_AXIBAR2PCIEBAR_%d" "C_AXIBAR_HIGHADDR_%d"]
			foreach range $ranges_list {
				set range_type [lindex $range 0]
				set axi_baseaddr [lindex $range 1]
				set child_baseaddr [lindex $range 1]
				set pcie_baseaddr [lindex $range 2]
				set axi_highaddr [lindex $range 3]
				set size [validate_ranges_property $slave $axi_baseaddr $axi_highaddr $child_baseaddr]
				lappend ranges $range_type $high_64bit $pcie_baseaddr $axi_baseaddr $high_64bit $size
			}
			set ip_tree [tree_append $ip_tree [list "ranges" hexinttuple $ranges]]
			lappend node $ip_tree
		}
		"pcie_ipif_slave" {
			# We can automatically generate the ranges property, but that's about it
			# the interrupt-map encodes board-level info that cannot be
			# derived from the MHS.
			# Default handling for all params first
			set ip_tree [slaveip_pcie_ipif_slave $slave $intc "pcie_ipif_slave" [default_parameters $slave]]

			# Standard stuff required fror the pci OF bindings
			set ip_tree [tree_append $ip_tree [list "#size-cells" int "2"]]
			set ip_tree [tree_append $ip_tree [list "#address-cells" int "3"]]
			set ip_tree [tree_append $ip_tree [list "#interrupt-cells" int "1"]]
			set ip_tree [tree_append $ip_tree [list "device_type" string "pci"]]
			# Generate ranges property.  Lots of assumptions here - 32 bit address space being the main one
			set ranges ""

			set ipifbar [ scan_int_parameter_value $slave "C_MEM1_BASEADDR" ]
			set ipif_highaddr [ scan_int_parameter_value $slave "C_MEM1_HIGHADDR" ]
			set space_code "0x02000000"

			set ranges [lappend ranges $space_code 0 $ipifbar $ipifbar 0 [ expr $ipif_highaddr - $ipifbar + 1 ]]

			set ip_tree [tree_append $ip_tree [ list "ranges" hexinttuple $ranges ]]

			# Now the interrupt-map-mask etc
			set ip_tree [tree_append $ip_tree [ list "interrupt-map-mask" hexinttuple "0xff00 0x0 0x0 0x7" ]]

			# Make sure the user knows they've still got more work to do
			# If we were prepared to add a custom PARAMETER to the MLD then we could do moer here, but for now this is
			# the best we can do
			debug warning "WARNING: Cannot automatically populate PCI interrupt-map property - this must be completed manually"
			lappend node $ip_tree
		}
		"plbv46_pci" {
			# We can automatically generate the ranges property, but that's about it
			# the interrupt-map encodes board-level info that cannot be
			# derived from the MHS.
			# Default handling for all params first
			set ip_tree [slaveip_intr $slave $intc [interrupt_list $slave] "plbv46-pci" [default_parameters $slave]]

			# Standard stuff required fror the pci OF bindings
			set ip_tree [tree_append $ip_tree [list "#size-cells" int "2"]]
			set ip_tree [tree_append $ip_tree [list "#address-cells" int "3"]]
			set ip_tree [tree_append $ip_tree [list "#interrupt-cells" int "1"]]
			set ip_tree [tree_append $ip_tree [list "device_type" string "pci"]]
			# Generate ranges property.  Lots of assumptions here - 32 bit address space being the main one
			set ranges ""
			set ipifbar_num [ scan_int_parameter_value $slave "C_IPIFBAR_NUM"]
			for {set i 0} {$i < $ipifbar_num} {incr i} {
				set ipif_spacetype [ scan_int_parameter_value $slave [ format "C_IPIF_SPACETYPE_%i" $i ] ]
				set ipifbar [ scan_int_parameter_value $slave [ format "C_IPIFBAR_%i" $i ] ]
				set ipif_highaddr [ scan_int_parameter_value $slave [ format "C_IPIF_HIGHADDR_%i" $i ] ]
				set ipifbar2pcibar [ scan_int_parameter_value $slave [ format "C_IPIFBAR2PCIBAR_%i" $i ] ]
				# A quick DRC to make sure the IPIFBAR and IPIFBAR2PCIBAR match
				# This is a limitation of the kernel PCI layer rather than anything else
				if { $ipifbar != $ipifbar2pcibar } {
					debug warning "WARNING: $name:  C_IPIFBAR_$i and C_IPIBAR2PCIBAR_$i don't match"
				}
				# Different magic number depending upon the type of address space
				switch $ipif_spacetype {
					"0" {
						# IO space
						set space_code "0x01000000"
						debug warning "WARNING: $name BAR $i: PCI I/O spaces not supported in Linux kernel PCI drivers"
					}
					"1" {
						# mem space
						set space_code "0x02000000"
					}
				}
				set ranges [lappend ranges $space_code 0 $ipifbar2pcibar $ipifbar 0 [ expr $ipif_highaddr - $ipifbar + 1 ]]
			}
			set ip_tree [tree_append $ip_tree [ list "ranges" hexinttuple $ranges ]]

			# Now the interrupt-map-mask etc
			set ip_tree [tree_append $ip_tree [ list "interrupt-map-mask" hexinttuple "0xff00 0x0 0x0 0x7" ]]

			# Make sure the user knows they've still got more work to do
			# If we were prepared to add a custom PARAMETER to the MLD then we could do moer here, but for now this is
			# the best we can do
			debug warning "WARNING: Cannot automatically populate PCI interrupt-map property - this must be completed manually"
			lappend node $ip_tree
		}
		"axi2axi_connector" {
			# FIXME: multiple ranges!
			set baseaddr [scan_int_parameter_value $slave "C_S_AXI_RNG00_BASEADDR"]
			set tree [bus_bridge $slave $intc $baseaddr "M_AXI"]

			if {[llength $tree] != 0} {
				set ranges_list [default_ranges $slave "C_S_AXI_NUM_ADDR_RANGES" "C_S_AXI_RNG%02d_BASEADDR" "C_S_AXI_RNG%02d_HIGHADDR"]
				set tree [tree_append $tree [gen_ranges_property_list $slave $ranges_list]]
				lappend node $tree
			}
		}
		"microblaze" {
			debug ip "Other Microblaze CPU $name=$type"
			lappend node [gen_microblaze $slave [default_parameters $slave]]
		}
		"ppc405" {
			debug ip "Other PowerPC405 CPU $name=$type"
			lappend node [gen_ppc405 $slave [default_parameters $slave]]
		}
		"axi_epc" -
		"xps_epc" {
			set tree [compound_slave $slave "C_PRH0_BASEADDR"]

			set epc_peripheral_num [xget_hw_parameter_value $slave "C_NUM_PERIPHERALS"]
			for {set x 0} {$x < ${epc_peripheral_num}} {incr x} {
				set subnode [slaveip_intr $slave $intc [interrupt_list $slave] "" "" "PRH${x}_" ]
				set subnode [change_nodename $subnode $name "${name}_p${x}"]
				set tree [tree_append $tree $subnode]
			}
			lappend node $tree
		}
		default {
			# *Most* IP should be handled by this default case.
			# check if is any parameter BASEADDR
			set ip_params [xget_hw_parameter_handle $slave "*"]
			set address_array {}
			set ranges_list {}
			puts [xget_hw_name $slave]
			foreach par_name $ip_params {
				# check all
				set addrtype [xget_hw_subproperty_value $par_name "ADDRESS"]
				if {[string compare -nocase $addrtype "BASE"] == 0} {
					set base [xget_hw_name $par_name]
					set high [xget_hw_subproperty_value $par_name "PAIR"]
					set baseaddr [scan_int_parameter_value $slave $base]
					set highaddr [scan_int_parameter_value $slave $high]
					if { "${baseaddr}" < "${highaddr}" } {
						lappend address_array $par_name
						# Also compound ranges list with all BASEADDR and HIGHADDR pairs
						lappend ranges_list [list $baseaddr $highaddr $baseaddr]
					}
				}
			}

			switch [llength $address_array] {
				"0" {
					# maybe just IP just with interrupt line
					set name [xget_hw_name $slave]
					set type [xget_hw_value $slave]
					set tree [slaveip_basic $slave $intc [default_parameters $slave] [format_ip_name $type "0" $name] ""]
					set tree [gen_interrupt_property $tree $slave $intc [interrupt_list $slave]]
					lappend node $tree
				}
				"1" {
					# address_array has only one baseaddr which means that it is single node
					set par_name $address_array
					set base [xget_hw_name $par_name]
					set high [xget_hw_subproperty_value $par_name "PAIR"]
					set baseaddr [scan_int_parameter_value $slave $base]
					set highaddr [scan_int_parameter_value $slave $high]
					set tree [slaveip_explicit_baseaddr $slave $intc "" [default_parameters $slave] $baseaddr $highaddr ""]
					set tree [gen_interrupt_property $tree $slave $intc [interrupt_list $slave]]
					lappend node $tree
				}
				default {
					# Use the first BASEADDR parameter to be in node name - order is directed by mpd
					set tree [slaveip_basic $slave $intc [default_parameters $slave] [format_ip_name $type [lindex $ranges_list 0 0] $name] ""]
					set tree [tree_append $tree [list \#size-cells int 1]]
					set tree [tree_append $tree [list \#address-cells int 1]]
					set tree [tree_append $tree [gen_ranges_property_list $slave $ranges_list]]
					set tree [gen_interrupt_property $tree $slave $intc [interrupt_list $slave]]
					lappend node $tree
				}
			}
		}
	}
	return [dts_override $node]
}

proc memory {slave baseaddr_prefix params} {
	set name [xget_hw_name $slave]
	set type [xget_hw_value $slave]
	set par [xget_hw_parameter_handle $slave "*"]
	set hw_ver [xget_hw_parameter_value $slave "HW_VER"]

	set ip_node {}

	set baseaddr [scan_int_parameter_value $slave [format "C_%sBASEADDR" $baseaddr_prefix]]
	set highaddr [scan_int_parameter_value $slave [format "C_%sHIGHADDR" $baseaddr_prefix]]

	lappend ip_node [gen_reg_property $name $baseaddr $highaddr]
	lappend ip_node [list "device_type" string "memory"]
	set ip_node [gen_params $ip_node $slave $params]
	return [list [format_ip_name memory $baseaddr $name] tree $ip_node]
}

proc gen_cortexa9 {tree hwproc_handle intc params} {
	set out ""
	variable cpunumber
	variable ps7_cortexa9_clk
	variable ps7_cortexa9_1x_clk
	set cpus_node {}

	set mhs_handle [xget_hw_parent_handle $hwproc_handle]
	set lprocs [xget_cortexa9_handles $mhs_handle]

	# add both the cortex a9 processors to the cpus node
	foreach hw_proc $lprocs {
		set cpu_name [xget_hw_name $hw_proc]
		set cpu_type [xget_hw_value $hw_proc]
		set hw_ver [xget_hw_parameter_value $hw_proc "HW_VER"]

		set proc_node {}
		lappend proc_node [list "device_type" string "cpu"]
		lappend proc_node [list model string "$cpu_type,$hw_ver"]
		lappend proc_node [gen_compatible_property $cpu_type $cpu_type $hw_ver]

		set ps7_cortexa9_clk [xget_sw_parameter_value $hwproc_handle "C_CPU_CLK_FREQ_HZ"]
		set ps7_cortexa9_1x_clk [xget_sw_parameter_value $hwproc_handle "C_CPU_1X_CLK_FREQ_HZ"]
		lappend proc_node [list "reg" int $cpunumber]
		lappend proc_node [list "i-cache-size" hexint [expr 0x8000]]
		lappend proc_node [list "i-cache-line-size" hexint 32]
		lappend proc_node [list "d-cache-size" hexint [expr 0x8000]]
		lappend proc_node [list "d-cache-line-size" hexint 32]
		set proc_node [gen_params $proc_node $hw_proc $params]
		lappend cpus_node [list [format_ip_name "cpu" $cpunumber $cpu_name] "tree" "$proc_node"]

		set cpunumber [expr $cpunumber + 1]
	}
	lappend cpus_node [list \#size-cells int 0]
	lappend cpus_node [list \#address-cells int 1]
	lappend cpus_node [list \#cpus hexint "$cpunumber" ]
	lappend tree [list cpus tree "$cpus_node"]

	# Add PMU node
	set ip_tree [list "pmu" tree ""]
	set ip_tree [zynq_irq $ip_tree $intc "ps7_pmu"]
	set ip_tree [tree_append $ip_tree [list "reg" hexinttuple [list "0xF8891000" "0x1000" "0xF8893000" "0x1000"] ] ]
	set ip_tree [tree_append $ip_tree [list "compatible" stringtuple "arm,cortex-a9-pmu"]]
	lappend tree "$ip_tree"

	return $tree
}

proc xget_cortexa9_handles { mhs_handle } {
	set ipinst_list [xget_hw_ipinst_handle $mhs_handle "*"]
	set lprocs ""
	foreach ipinst $ipinst_list {
		set ipname [xget_value $ipinst "OPTION" "IPNAME"]
		if {[string compare -nocase $ipname "ps7_cortexa9"] == 0} {
			lappend lprocs $ipinst
		}
	}

	return $lprocs
}

proc gen_ppc405 {tree hwproc_handle params} {
	set out ""
	variable cpunumber

	set cpu_name [xget_hw_name $hwproc_handle]
	set cpu_type [xget_hw_value $hwproc_handle]
	set hw_ver [xget_hw_parameter_value $hwproc_handle "HW_VER"]

	set cpus_node {}
	set proc_node {}
	lappend proc_node [list "device_type" string "cpu"]
	lappend proc_node [list model string "PowerPC,405"]
	lappend proc_node [list compatible stringtuple [list "PowerPC,405" "ibm,ppc405"]]

	# Get the clock frequency from the processor
	set clk [get_clock_frequency $hwproc_handle "CPMC405CLOCK"]
	if {$clk == ""} {
		set proc_handle [xget_libgen_proc_handle]
		set clk [xget_sw_parameter_value $proc_handle "CORE_CLOCK_FREQ_HZ"]
	}
	debug clock "Clock Frequency: $clk"

	lappend proc_node [list clock-frequency int $clk]
	# Assume that the CPMC405TIMERENABLE is always high, so the
	# timebase is the same as the processor clock.
	lappend proc_node [list timebase-frequency int $clk]
	lappend proc_node [list reg int $cpunumber]
	lappend proc_node [list i-cache-size hexint [expr 0x4000]]
	lappend proc_node [list i-cache-line-size hexint 32]
	lappend proc_node [list d-cache-size hexint [expr 0x4000]]
	lappend proc_node [list d-cache-line-size hexint 32]
	set proc_node [gen_params $proc_node $hwproc_handle $params]
	lappend proc_node [list dcr-controller empty empty]
	lappend proc_node [list dcr-access-method string native]

	lappend cpus_node [list [format_ip_name "cpu" $cpunumber $cpu_name] "tree" "$proc_node"]
	lappend cpus_node [list \#size-cells int 0]
	lappend cpus_node [list \#address-cells int 1]
	incr cpunumber
	lappend cpus_node [list \#cpus hexint "$cpunumber" ]
	lappend tree [list cpus tree "$cpus_node"]
	return $tree
}

proc gen_ppc440 {tree hwproc_handle intc params} {
	set out ""
	variable cpunumber

	set cpu_name [xget_hw_name $hwproc_handle]
	set cpu_type [xget_hw_value $hwproc_handle]
	set hw_ver [xget_hw_parameter_value $hwproc_handle "HW_VER"]

	set cpus_node {}
	set proc_node {}
	lappend proc_node [list "device_type" string "cpu"]
	lappend proc_node [list model string "PowerPC,440"]
	lappend proc_node [list compatible stringtuple [list "PowerPC,440" "ibm,ppc440"]]

	# Get the clock frequency from the processor
	set clk [get_clock_frequency $hwproc_handle "CPMC440CLK"]
	if {$clk == ""} {
		set proc_handle [xget_libgen_proc_handle]
		set clk [xget_sw_parameter_value $proc_handle "CORE_CLOCK_FREQ_HZ"]
	}
	debug clock "Clock Frequency: $clk"

	lappend proc_node [list clock-frequency int $clk]
	# Assume that the CPMC440TIMERENABLE is always high, so the
	# timebase is the same as the processor clock.
	lappend proc_node [list timebase-frequency int $clk]
	lappend proc_node [list reg int $cpunumber]
	lappend proc_node [list i-cache-size hexint [expr 0x8000]]
	lappend proc_node [list i-cache-line-size hexint 32]
	lappend proc_node [list d-cache-size hexint [expr 0x8000]]
	lappend proc_node [list d-cache-line-size hexint 32]
	set proc_node [gen_params $proc_node $hwproc_handle $params]
	lappend proc_node [list dcr-controller empty empty]
	lappend proc_node [list dcr-access-method string native]

	lappend proc_node [list \#size-cells int 1]
	lappend proc_node [list \#address-cells int 1]

	set num_ports [scan_int_parameter_value $hwproc_handle "C_NUM_DMA"]
	for {set x 0} {$x < $num_ports} {incr x} {
		set idcr_baseaddr [scan_int_parameter_value $hwproc_handle [format "C_IDCR_BASEADDR" $x]]
		# This expression comes out of the V5FX user guide.
		# 0x80, 0x98, 0xb0, 0xc8
		set baseaddr [expr $idcr_baseaddr + [expr 0x80 + 0x18*$x]]
		# Yes, apparently there really are 17 registers!
		set highaddr [expr $baseaddr + 0x10]

		set sdma_name [format_ip_name sdma $baseaddr "DMA$x"]
		set sdma_tree [list $sdma_name tree {}]
		set sdma_tree [tree_append $sdma_tree [gen_reg_property $sdma_name $baseaddr $highaddr "dcr-reg"]]
		set sdma_tree [tree_append $sdma_tree [gen_compatible_property $sdma_name "ll_dma" "1.00.a"]]
		set sdma_tree [gen_interrupt_property $sdma_tree $hwproc_handle $intc [list [format "DMA%dRXIRQ" $x] [format "DMA%dTXIRQ" $x]]]

		lappend proc_node $sdma_tree
	}

	lappend cpus_node [list [format_ip_name "cpu" $cpunumber $cpu_name] "tree" "$proc_node"]
	lappend cpus_node [list \#size-cells int 0]
	lappend cpus_node [list \#address-cells int 1]
	incr cpunumber
	lappend cpus_node [list \#cpus hexint "$cpunumber" ]
	lappend tree [list cpus tree "$cpus_node"]
	return $tree
}

proc gen_microblaze {tree hwproc_handle params} {
	set out ""
	variable cpunumber

	set cpu_name [xget_hw_name $hwproc_handle]
	set cpu_type [xget_hw_value $hwproc_handle]

	set icache_size [scan_int_parameter_value $hwproc_handle "C_CACHE_BYTE_SIZE"]
	set icache_base [scan_int_parameter_value $hwproc_handle "C_ICACHE_BASEADDR"]
	set icache_high [scan_int_parameter_value $hwproc_handle "C_ICACHE_HIGHADDR"]
	set dcache_size [scan_int_parameter_value $hwproc_handle "C_DCACHE_BYTE_SIZE"]
	set dcache_base [scan_int_parameter_value $hwproc_handle "C_DCACHE_BASEADDR"]
	set dcache_high [scan_int_parameter_value $hwproc_handle "C_DCACHE_HIGHADDR"]
	# The Microblaze parameters are in *words*, while the device tree
	# is in bytes.
	set icache_line_size [expr 4*[scan_int_parameter_value $hwproc_handle "C_ICACHE_LINE_LEN"]]
	set dcache_line_size [expr 4*[scan_int_parameter_value $hwproc_handle "C_DCACHE_LINE_LEN"]]
	set hw_ver [xget_hw_parameter_value $hwproc_handle "HW_VER"]

	set cpus_node {}
	set proc_node {}
	lappend proc_node [list "device_type" string "cpu"]
	lappend proc_node [list model string "$cpu_type,$hw_ver"]
	lappend proc_node [gen_compatible_property $cpu_type $cpu_type $hw_ver]

	# Get the clock frequency from the processor
	set clk [get_clock_frequency $hwproc_handle "CLK"]
	debug clock "Clock Frequency: $clk"
	lappend proc_node [list clock-frequency int $clk]
	lappend proc_node [list timebase-frequency int $clk]
	lappend proc_node [list reg int 0]
	if { [llength $icache_size] != 0 } {
		lappend proc_node [list i-cache-baseaddr hexint $icache_base]
		lappend proc_node [list i-cache-highaddr hexint $icache_high]
		lappend proc_node [list i-cache-size hexint $icache_size]
		lappend proc_node [list i-cache-line-size hexint $icache_line_size]
	}
	if { [llength $dcache_size] != 0 } {
		lappend proc_node [list d-cache-baseaddr hexint $dcache_base]
		lappend proc_node [list d-cache-highaddr hexint $dcache_high]
		lappend proc_node [list d-cache-size hexint $dcache_size]
		lappend proc_node [list d-cache-line-size hexint $dcache_line_size]
	}

	#-----------------------------
	# generating additional parameters
	# the list of Microblaze parameters
	set proc_node [gen_params $proc_node $hwproc_handle $params]

	#-----------------------------
	lappend cpus_node [list [format_ip_name "cpu" $cpunumber  $cpu_name] "tree" "$proc_node"]
	lappend cpus_node [list \#size-cells int 0]
	lappend cpus_node [list \#address-cells int 1]
	incr cpunumber
	lappend cpus_node [list \#cpus hexint "$cpunumber" ]
	lappend tree [list cpus tree "$cpus_node"]
	return $tree
}

proc get_first_mem_controller { memory_nodes } {
	foreach order "ps7_ddr axi_v6_ddrx axi_7series_ddrx axi_s6_ddrx mpmc" {
		foreach node $memory_nodes {
			if { "[lindex $node 0]" == "$order" } {
				return $node
			}
		}
	}
}

proc gen_memories {tree hwproc_handle} {
	global main_memory main_memory_bank
	global main_memory_start main_memory_size
	set mhs_handle [xget_hw_parent_handle $hwproc_handle]
	set ip_handles [xget_hw_ipinst_handle $mhs_handle "*"]
	set memory_count 0
	set baseaddr [expr ${main_memory_start}]
	set memsize [expr ${main_memory_size}]
	if {$baseaddr >= 0 && $memsize > 0} {
		# Manual memory setup
		set subnode {}
		set devtype "memory"
		lappend subnode [list "device_type" string "${devtype}"]
		lappend subnode [list "reg" hexinttuple [list $baseaddr $memsize]]
		lappend tree [list [format_ip_name "${devtype}" $baseaddr "system_memory"] tree $subnode]
		incr memory_count
		return $tree
	}
	set mhs_handle [xget_hw_parent_handle $hwproc_handle]
	set ip_handles [xget_hw_ipinst_handle $mhs_handle "*"]
	set memory_count 0
	set memory_nodes {}
	foreach slave $ip_handles {
		set name [xget_hw_name $slave]
		set type [xget_hw_value $slave]

		if {![string match "" $main_memory] && ![string match -nocase "none" $main_memory]} {
			if {![string match $name $main_memory]} {
				continue;
			}
		}
		set node $type
		switch $type {
			"lmb_bram_if_cntlr" {
				if { "$name" == "microblaze_0_i_bram_ctrl" } {
					lappend node [memory $slave "" ""]
					lappend memory_nodes $node
					incr memory_count
				}
			}
			"axi_bram_ctrl" -
			"plb_bram_if_cntlr" -
			"opb_bram_if_cntlr" {
				# Ignore these, since they aren't big enough to be main
				# memory, and we can't currently handle non-contiguous memory
				# regions.
			}
			"opb_sdram" -
			"mig_7series" {
				# Handle bankless memories.
				lappend node [memory $slave "" ""]
				lappend memory_nodes $node
				incr memory_count
			}
			"ppc440mc_ddr2" {
				# Handle bankless memories.
				lappend node [memory $slave "MEM_" ""]
				lappend memory_nodes $node
				incr memory_count
			}
			"axi_s6_ddrx" {
				for {set x 0} {$x < 6} {incr x} {
					set baseaddr [scan_int_parameter_value $slave [format "C_S%d_AXI_BASEADDR" $x]]
					set highaddr [scan_int_parameter_value $slave [format "C_S%d_AXI_HIGHADDR" $x]]
					if {$highaddr < $baseaddr} {
						continue;
					}
					lappend node [memory $slave [format "S%d_AXI_" $x] ""]
					lappend memory_nodes $node
					break;
				}
				incr memory_count
			}
			"ps7_ddr" {
				# FIXME: this is workaround for Xilinx tools to
				# generate correct base memory address for ps7_ddr
				set subnode {}
				set baseaddr 0
				set highaddr [scan_int_parameter_value $slave "C_S_AXI_HIGHADDR"]
				set highaddr [expr $highaddr + 1]
				lappend subnode [list "device_type" string "memory"]
				lappend subnode [list "reg" hexinttuple [list $baseaddr $highaddr]]
				lappend node [list [format_ip_name "memory" $baseaddr $name] tree $subnode]
				lappend memory_nodes $node
				incr memory_count
			}
			"axi_v6_ddrx" -
			"axi_7series_ddrx" {
				lappend node [memory $slave "S_AXI_" ""]
				lappend memory_nodes $node
				incr memory_count
			}
			"opb_cypress_usb" -
			"plb_ddr" -
			"plb_ddr2" -
			"plb_emc" -
			"opb_sdram" -
			"opb_ddr" -
			"opb_emc" -
			"mch_opb_ddr" -
			"mch_opb_ddr2" -
			"mch_opb_emc" -
			"mch_opb_sdram" -
			"xps_mch_emc" {
				# Handle memories with 'banks'. Generate one memory
				# node for each bank.
				set count [scan_int_parameter_value $slave "C_NUM_BANKS_MEM"]
				if { [llength $count] == 0 } {
					set count 1
				}
				for {set x 0} {$x < $count} {incr x} {
					if { $x == $main_memory_bank } {
						lappend node [memory $slave [format "MEM%d_" $x] ""]
						lappend memory_nodes $node
						incr memory_count
					}
				}
			}
			"axi_emc" {
				# Handle memories with 'banks'. Generate one memory
				# node for each bank.
				set count [scan_int_parameter_value $slave "C_NUM_BANKS_MEM"]
				if { [llength $count] == 0 } {
					set count 1
				}
				for {set x 0} {$x < $count} {incr x} {
					set synch_mem [scan_int_parameter_value $slave [format "C_MEM%d_TYPE" $x]]
					# C_MEM$x_TYPE = 2 or 3 indicates the bank handles
					# a flash device and it should be listed as a
					# slave in fdt.
					# C_MEM$x_TYPE = 0, 1 or 4 indicates the bank handles
					# SRAM and it should be listed as a memory in
					# fdt.
					if { $synch_mem == 2 || $synch_mem == 3 } {
						continue;
					}
					lappend node [memory $slave [format "S_AXI_MEM%d_" $x] ""]
					lappend memory_nodes $node
					incr memory_count
				}
			}
			"mpmc" {
				set share_addresses [scan_int_parameter_value $slave "C_ALL_PIMS_SHARE_ADDRESSES"]
				if {$share_addresses != 0} {
					lappend node [memory $slave "MPMC_" ""]
					lappend memory_nodes $node
				} else {
					set old_baseaddr [scan_int_parameter_value $slave [format "C_PIM0_BASEADDR" $x]]
					set old_offset [scan_int_parameter_value $slave [format "C_PIM0_OFFSET" $x]]
					set safe_addresses 1
					set num_ports [scan_int_parameter_value $slave "C_NUM_PORTS"]
					for {set x 1} {$x < $num_ports} {incr x} {
						set baseaddr [scan_int_parameter_value $slave [format "C_PIM%d_BASEADDR" $x]]
						set baseaddr [scan_int_parameter_value $slave [format "C_PIM%d_OFFSET" $x]]
						if {$baseaddr != $old_baseaddr} {
							debug warning "Warning!: mpmc is configured with different baseaddresses on different ports!  Since this is a potentially hazardous configuration, a device tree node describing the memory will not be generated."
							set safe_addresses 0
						}
						if {$offset != $old_offset} {
							debug warning "Warning!: mpmc is configured with different offsets on different ports!  Since this is a potentially hazardous configuration, a device tree node describing the memory will not be generated."
						}
					}
					if {$safe_addresses == 1} {
						lappend node [memory $slave "PIM0_" ""]
						lappend memory_nodes $node
					}
				}

				incr memory_count
			}
		}
	}
	if {$memory_count == 0} {
		error "No memory nodes found!"
	}
	if {$memory_count > 1} {
		debug warning "Warning!: More than one memory found.  Note that most platforms don't support non-contiguous memory maps!"
		debug warning "Warning!: Try to find out the main memory controller!"
		set memory_node [get_first_mem_controller $memory_nodes]
	} else {
		set memory_node [lindex $memory_nodes 0]
	}

	# Skip type because only one memory node is selected
	lappend tree [lindex $memory_node 1]

	return $tree
}

# Return 1 if the given interface of the given slave is connected to a bus.
proc bus_is_connected {slave face} {
	set busif_handle [xget_hw_busif_handle $slave $face]
	if {[llength $busif_handle] == 0} {
		error "Bus handle $face not found!"
	}
	set bus_name [xget_hw_value $busif_handle]

	set mhs_handle [xget_hw_parent_handle $slave]
	set bus_handle [xget_hw_ipinst_handle $mhs_handle $bus_name]

	return [llength $bus_handle]
}

# Populates a bus node with components connected to the given slave
# and adds it to the given tree
#
# tree         : Tree to populate
# slave_handle : The slave to use as a starting point, this is
# typically the root processor or a previously traversed bus bridge.
# intc_handle	: The interrupt controller associated with the
# processor. Slave will have an interrupts node relative to this
# controller.
# baseaddr     : The base address of the address range of this bus.
# face : The name of the port of the slave that is connected to the
# bus.
proc bus_bridge {slave intc_handle baseaddr face {handle ""} {ps_ifs ""} {force_ips ""}} {
	debug handles "+++++++++++ $slave ++++++++"
	set busif_handle [xget_hw_busif_handle $slave $face]
	if {[llength $handle] != 0} {
		set busif_handle $handle
	}
 	if {[llength $busif_handle] == 0} {
		error "Bus handle $face not found!"
	}
	set bus_name [xget_hw_value $busif_handle]
	global buses
	if {[lsearch $buses $bus_name] >= 0} {
		return {}
	}
	lappend buses $bus_name
	debug ip "IP connected to bus: $bus_name"
	debug handles "bus_handle: $busif_handle"

	set mhs_handle [xget_hw_parent_handle $slave]
	set bus_handle [xget_hw_ipinst_handle $mhs_handle $bus_name]

#FIXME remove compatible_list property and add simple-bus in  gen_compatible_property function
	set compatible_list {}
	if {[llength $bus_handle] == 0} {
		debug handles "Bus handle $face connected directly..."
		set slave_ifs [xget_hw_connected_busifs_handle $mhs_handle $bus_name "target"]
		set bus_type "xlnx,compound"
		set hw_ver ""
		set devicetype $bus_type
	} else {
		debug handles "Bus handle $face connected through a bus..."
		set bus_type [xget_hw_value $bus_handle]
		switch $bus_type {
			"plb_v34" -
			"plb_v46" {
				set devicetype "plb"
				set compatible_list [list "simple-bus"]
			}
			"opb_v20" {
				set devicetype "opb"
				set compatible_list [list "simple-bus"]
			}
			"dcr_v29" {
				set devicetype "dcr"
				set compatible_list [list "simple-bus"]
			}
			"axi_crossbar" -
			"axi_interconnect" {
				set devicetype "axi"
				set compatible_list [list "simple-bus"]
			}
			"ps7_axi_interconnect" {
				set devicetype "amba"
				set compatible_list [list "simple-bus"]
			}
			default {
				set devicetype $bus_type
			}
		}
		set hw_ver [xget_hw_parameter_value $bus_handle "HW_VER"]

		set master_ifs [xget_hw_connected_busifs_handle $mhs_handle $bus_name "master"]
		foreach if $master_ifs {
			set ip_handle [xget_hw_parent_handle $if]
			debug ip "-master [xget_hw_name $if] [xget_hw_value $if] [xget_hw_name $ip_handle]"
			debug handles "  handle: $ip_handle"

			# Note that bus masters do not need to be traversed, so we don't
			# add them to the list of ip.
		}
		set slave_ifs [xget_hw_connected_busifs_handle $mhs_handle $bus_name "slave"]
	}

	set bus_ip_handles {}
	# Compose peripherals & cleaning

	foreach if $slave_ifs {
		set ip_handle [xget_hw_parent_handle $if]
		debug ip "-slave [xget_hw_name $if] [xget_hw_value $if] [xget_hw_name $ip_handle]"
		debug handles "  handle: $ip_handle"

		# Do not generate ps7_dma type with name ps7_dma_ns
		if { "[xget_hw_value $ip_handle]" == "ps7_dma" &&  "[xget_hw_name $ip_handle]" == "ps7_dma_ns" } {
			continue
		}

		# If its not already in the list, and its not the bridge, then
		# append it.
		if {$ip_handle != $slave} {
			if {[lsearch $bus_ip_handles $ip_handle] == -1} {
				lappend bus_ip_handles $ip_handle
			}
		}
	}

	# MS This is specific function for AXI zynq IPs - I hope it will be removed
	# soon by providing M_AXI_GP0 interface
	foreach if $ps_ifs {
		debug ip "-slave [xget_hw_name $if]"
		debug handles "  handle: $if"

		# Do not generate ps7_dma type with name ps7_dma_ns
		if { "[xget_hw_value $if]" == "ps7_dma" &&  "[xget_hw_name $if]" == "ps7_dma_ns" } {
			continue
		}

		# If its not already in the list, and its not the bridge, then
		# append it.
		if {$if != $slave} {
			if {[lsearch $bus_ip_handles $if] == -1} {
				lappend bus_ip_handles $if
			} else {
				debug ip "IP $if [xget_hw_name $if] is already appended - skip it"
			}
		}
	}

	# A list of all the IP that have been generated already.
	variable periphery_array

	set mdm {}
	set uartlite {}
	set fulluart {}
	set ps_smcc {}
	set sorted_ip {}
	set console_type ""

	global consoleip
	# Sort all serial IP to be nice in the alias list
	foreach ip $bus_ip_handles {
		set name [xget_hw_name $ip]
		set type [xget_hw_value $ip]

		# Save console type for alias sorting
		if { [string match "$name" "$consoleip"] } {
			set console_type "$type"
		}

		# Add all uarts to own class
		if { [string first "uartlite" "$type"] != -1 } {
			lappend uartlite $ip
		} elseif { [string first "uart16550" "$type"] != -1 } {
			lappend fulluart $ip
		} elseif { [string first "mdm" "$type"] != -1 } {
			lappend mdm $ip
		} elseif { [string first "smcc" "$type"] != -1 } {
			lappend ps_smcc $ip
		} else {
			lappend sorted_ip $ip
		}
	}

	# This order will be in alias list
	if { [string first "uart16550" "$console_type"] != -1 } {
		set sorted_ip "$sorted_ip $fulluart $uartlite $mdm $ps_smcc"
	} else {
		set sorted_ip "$sorted_ip $uartlite $fulluart $mdm $ps_smcc"
	}

	# Start generating the node for the bus.
	set bus_node {}

	# Populate with all the slaves.
	foreach ip $sorted_ip {
		# If we haven't already generated this ip
		if {[lsearch $periphery_array $ip] == -1} {
			set bus_node [gener_slave $bus_node $ip $intc_handle]
			lappend periphery_array $ip
		}
	}

	# Force nodes to bus $force_ips is list of IP types
	foreach ip $force_ips {
		set bus_node [gener_slave $bus_node "" $intc_handle $ip]
	}

	# I have to generate led description on the same level as gpio node is
	# we are using designs with one plb - that's why is ok to have it here
	set led [led_gpio]
	if { "$led" != "" } {
		lappend bus_node $led
	}

	lappend bus_node [list \#size-cells int 1]
	lappend bus_node [list \#address-cells int 1]
	lappend bus_node [gen_compatible_property $bus_name $bus_type $hw_ver $compatible_list]

	variable bus_count
	set baseaddr $bus_count
	incr bus_count
	return [list [format_ip_name $devicetype $baseaddr $bus_name] tree $bus_node]
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

# Return a sorted list of all the port names that we think are
# interrupts (i.e. those tagged in the mpd with SIGIS=INTERRUPT)
proc interrupt_list {ip_handle} {
	set port_handles [xget_hw_port_handle $ip_handle "*"]
	set interrupt_ports {}
	foreach port $port_handles {
		set name [xget_value $port "NAME"]
		set sigis [xget_hw_subproperty_value $port "SIGIS"]
		if {[string match $sigis "INTERRUPT"]} {
			lappend interrupt_ports $name
		}
	}
	return [lsort $interrupt_ports]
}

# Return a list of translation ranges for bridges which support
# multiple ranges with identity translation.
# ip_handle: handle to the bridge
# num_ranges_name: name of the bridge parameter which gives the number
# of active ranges.
# range_base_name_template: parameter name for the base address of
# each range, with a %d in place of the range number.
# range_high_name_template: parameter name for the high address of
# each range, with a %d in place of the range number.
proc default_ranges {ip_handle num_ranges_name range_base_name_template range_high_name_template {range_start "0"}} {
	set count [scan_int_parameter_value $ip_handle $num_ranges_name]
	if { [llength $count] == 0 } {
		set count 1
	}
	set ranges_list {}
	for {set x ${range_start}} {$x < [expr $count + ${range_start}]} {incr x} {
		set baseaddr [scan_int_parameter_value $ip_handle [format $range_base_name_template $x]]
		set highaddr [scan_int_parameter_value $ip_handle [format $range_high_name_template $x]]
		lappend ranges_list [list $baseaddr $highaddr $baseaddr]
	}
	return $ranges_list
}

proc axipcie_ranges {ip_handle num_ranges_name axi_base_name_template pcie_base_name_template axi_high_name_template} {
	set count [scan_int_parameter_value $ip_handle $num_ranges_name]
	if { [llength $count] == 0 } {
		set count 1
	}
	set ranges_list {}
	for {set x 0} {$x < $count} {incr x} {
		set range_type 0x02000000
		set axi_baseaddr [scan_int_parameter_value $ip_handle [format $axi_base_name_template $x]]
		set pcie_baseaddr [scan_int_parameter_value $ip_handle [format $pcie_base_name_template $x]]
		set axi_highaddr [scan_int_parameter_value $ip_handle [format $axi_high_name_template $x]]
		lappend ranges_list [list $range_type $axi_baseaddr $pcie_baseaddr $axi_highaddr]
	}
	return $ranges_list
}

# Return a list of all the parameter names for the given ip that
# should be reported in the device tree for generic IP. This list
# includes all the parameter names, except those that are handled
# specially, such as the instance name, baseaddr, etc.
proc default_parameters {ip_handle} {
	set par_handles [xget_hw_parameter_handle $ip_handle "*"]
	set params {}
	foreach par $par_handles {
		set par_name [xget_hw_name $par]
		# Ignore some parameters that are always handled specially
		switch -glob $par_name {
			"INSTANCE" -
			"*BASEADDR" -
			"*HIGHADDR" -
			"C_SPLB*" -
			"C_OPB*" -
			"C_DPLB*" -
			"C_IPLB*" -
			"C_PLB*" -
			"C_M_AXI*" -
			"C_S_AXI_ADDR_WIDTH" -
			"C_S_AXI_DATA_WIDTH" -
			"C_S_AXI_ACLK_FREQ_HZ" -
			"C_S_AXI_LITE*" -
			"C_S_AXI_PROTOCOL" -
			"C_INTERCONNECT_?_AXI*" -
			"C_S_AXI_ACLK_PERIOD_PS" -
			"C_M*_AXIS*" -
			"C_S*_AXIS*" -
			"C_PRH*" -
			"C_FAMILY" -
			"HW_VER" {}
			default {
				if { [ regexp {^C_.+} $par_name ] } {
					lappend params $par_name
				}
			}
		}
	}
	return $params
}

proc parameter_exists {ip_handle name} {
	set param_handle [xget_hw_parameter_handle $ip_handle $name]
	if {$param_handle == ""} {
		return 0
	}
	return 1
}

proc scan_int_parameter_value {ip_handle name} {
	set param_handle [xget_hw_parameter_handle $ip_handle $name]
	if {$param_handle == ""} {
		error "Can't find parameter $name in [xget_hw_name $ip_handle]"
		return 0
	}
	set value [xget_hw_value $param_handle]
	# tcl 8.4 doesn't handle binary literals..
	if {[string match 0b* $value]} {
		# Chop off the 0b
		set tail [string range $value 2 [expr [string length $value]-1]]
		# Pad to 32 bits, because binary scan ignores incomplete words
		set list [split $tail ""]
		for {} {[llength $list] < 32} {} {
			set list [linsert $list 0 0]
		}
		set tail [join $list ""]
		# Convert the remainder back to decimal
		binary scan [binary format "B*" $tail] "I*" value
	}
	return [expr $value]
}

# generate structure for phy.
# PARAMETER periph_type_overrides = {phy <IP_name> <phy_addr> <compatible>}
proc gen_phytree {ip phya phy_chip} {
	variable phy_count

	global overrides

	set name [xget_hw_name $ip]
	set type [xget_hw_value $ip]

	foreach over $overrides {
		if {[lindex $over 0] == "phy"} {
			if { [xget_hw_name $ip] == [lindex $over 1] } {
				set phya [lindex $over 2]
				set phy_chip [lindex $over 3]
			} else {
				puts "PHY: Not valid PHY addr for this ip: $name/$type"
			}
		}
	}

	set phy_name [format_ip_name phy $phya "phy$phy_count"]
	set phy_tree [list $phy_name tree {}]
	set phy_tree [tree_append $phy_tree [list "reg" int $phya]]
	set phy_tree [tree_append $phy_tree [list "device_type" string "ethernet-phy"]]
	set phy_tree [tree_append $phy_tree [list "compatible" string "$phy_chip"]]

	incr phy_count
	return $phy_tree
}

proc gen_mdiotree {ip} {
	# set default to 7
	set phya 7
	set phy_chip "marvell,88e1111"
	set mdio_tree [list "mdio" tree {}]
	set mdio_tree [tree_append $mdio_tree [list \#size-cells int 0]]
	set mdio_tree [tree_append $mdio_tree [list \#address-cells int 1]]
	return [tree_append $mdio_tree [gen_phytree $ip $phya $phy_chip]]
}

proc format_name {par_name} {
	set par_name [string tolower $par_name]
	set par_name [string map -nocase {"_" "-"} $par_name]
	return $par_name
}

proc format_xilinx_name {name} {
	return "xlnx,[format_name $name]"
}

proc format_param_name {name trimprefix} {
	if {[string match [string range $name 0 [expr [string length $trimprefix] - 1]] $trimprefix]} {
		set name [string range $name [string length $trimprefix] [string length $name]]
	}
	return [format_xilinx_name $name]
}

proc format_ip_name {devicetype baseaddr {label ""}} {
	set node_name [format_name [format "%s@%x" $devicetype $baseaddr]]
	if {[string match $label ""]} {
		return $node_name
	} else {
		return [format "%s: %s" $label $node_name]
	}
}

# TODO: remove next two lines which is a temporary HACK for CR 532315
set num_intr_inputs -1

proc gen_params {node_list handle params {trimprefix "C_"} } {
	foreach par_name $params {
		if {[catch {
			set par_value [scan_int_parameter_value $handle $par_name]
			# TODO: remove next if elseif block which is a temporary HACK for CR 532315
			if {[string match C_NUM_INTR_INPUTS $par_name]} {
				set num_intr_inputs $par_value
			} elseif {[string match C_KIND_OF_INTR $par_name]} {
				# Pad to 32 bits - num_intr_inputs
				if {$num_intr_inputs != -1} {
					set count 0
					set mask 0
					set par_mask 0
					while {$count < $num_intr_inputs} {
						set mask [expr {1<<$count}]
						set new_mask [expr {$mask | $par_mask}]
						set par_mask $new_mask
						incr count
					}
					set par_value_32 $par_value
					set par_value [expr {$par_value_32 & $par_mask}]
				} else {
					debug warning "Warning: num-intr-inputs not set yet, kind-of-intr will be set to zero"
					set par_value 0
				}
			}
			lappend node_list [list [format_param_name $par_name $trimprefix] hexint $par_value]
		} {err}]} {
			set par_handle [xget_hw_parameter_handle $handle $par_name]
			if {$par_handle == ""} {
				debug warning "Warning: Unknown parameter name $par_name"
			} else {
				set par_value [xget_hw_value $par_handle]
			}
			lappend node_list [list [format_param_name $par_name $trimprefix] string $par_value]
		}
	}
	return $node_list
}

proc gen_compatible_property {nodename type hw_ver {other_compatibles {}} } {
	array set compatible_list [ list \
		{opb_intc} {xps_intc_1.00.a} \
		{opb_timer} {xps_timer_1.00.a} \
		{xps_timer} {xps_timer_1.00.a} \
		{axi_timer} {xps_timer_1.00.a} \
		{mpmc} {mpmc_3.00.a} \
		{plb_v46} {plb_v46_1.00.a} \
		{plbv46_pci} {plbv46_pci_1.03.a} \
		{xps_bram_if_cntlr} {xps_bram_if_cntlr_1.00.a} \
		{axi_bram_ctrl} {xps_bram_if_cntlr_1.00.a} \
		{xps_ethernetlite} {xps_ethernetlite_1.00.a} \
		{axi_ethernetlite} {xps_ethernetlite_1.00.a} \
		{xps_gpio} {xps_gpio_1.00.a} \
		{axi_gpio} {xps_gpio_1.00.a} \
		{xps_hwicap} {xps_hwicap_1.00.a} \
		{xps_tft} {xps_tft_1.00.a} \
		{axi_tft} {xps_tft_1.00.a} \
		{xps_iic} {xps_iic_2.00.a} \
		{axi_iic} {xps_iic_2.00.a} \
		{xps_intc} {xps_intc_1.00.a} \
		{axi_intc} {xps_intc_1.00.a} \
		{xps_ll_temac} {xps_ll_temac_1.01.b xps_ll_temac_1.00.a} \
		{xps_ll_fifo} {xps_ll_fifo_1.00.a} \
		{axi_ethernet} {axi_ethernet_1.00.a} \
		{axi_ethernet_buffer} {axi_ethernet_1.00.a} \
		{axi_dma} {axi_dma_1.00.a} \
		{xps_ps2} {xps_ps2_1.00.a} \
		{xps_spi_2} {xps_spi_2.00.a} \
		{axi_spi} {xps_spi_2.00.a} \
		{axi_quad_spi} {xps_spi_2.00.a} \
		{xps_uart16550_2} {xps_uart16550_2.00.a} \
		{axi_uart16550} {xps_uart16550_2.00.a} \
		{xps_uartlite} {xps_uartlite_1.00.a} \
		{axi_uartlite} {xps_uartlite_1.00.a} \
		{xps_timebase_wdt} {xps_timebase_wdt_1.00.a} \
		{axi_timebase_wdt} {xps_timebase_wdt_1.00.a} \
		{xps_can} {xps_can_1.00.a} \
		{axi_can} {xps_can_1.00.a} \
		{xps_sysace} {xps_sysace_1.00.a} \
		{axi_sysace} {xps_sysace_1.00.a} \
		{xps_usb_host} {xps_usb_host_1.00.a} \
		{xps_usb2_device} {xps_usb2_device_4.00.a} \
		{axi_usb2_device} {xps_usb2_device_4.00.a} \
		{axi_pcie} {axi_pcie_1.05.a} \
		{ps7_ddrc} {ps7-ddrc} \
	]

	if {$hw_ver != ""} {
		set namewithver [format "%s_%s" $type $hw_ver]
		set clist [list [format_xilinx_name "$namewithver"]]
		regexp {([^\.]*)} $hw_ver hw_ver_wildcard
		set namewithwildcard [format "%s_%s" $type $hw_ver_wildcard]
		if { [info exists compatible_list($namewithver)] } {              # Check exact match
			set add_clist [list [format_xilinx_name "$compatible_list($namewithver)"]]
			set clist [concat $clist $add_clist]
		} elseif { [info exists compatible_list($namewithwildcard)] } {   # Check major wildcard match
			set add_clist [list [format_xilinx_name "$compatible_list($namewithwildcard)"]]
			set clist [concat $clist $add_clist]
		} elseif { [info exists compatible_list($type)] } {               # Check type wildcard match
			# Extended compatible property - for example ll_temac
			foreach single "$compatible_list($type)" {
				set add_clist [list [format_xilinx_name "$single"]]
				if { ![string match $clist $add_clist] } {
					set clist [concat $clist $add_clist]
				}
			}
		}
	} else {
		set clist [list [format_xilinx_name "$type"]]
	}
	set clist [concat $clist $other_compatibles]

	# Command: "compatible -replace/-append <IP name> <compatible list>"
	# or: "compatible <IP name> <compatible list>" where replace is used
	global overrides
	foreach i $overrides {
		# skip others overrides
		if { [lindex "$i" 0] != "compatible" } {
			continue;
		}
		# Compatible command have at least 4 elements in the list
		if { [llength $i] < 3 } {
			error "Wrong compatible override command string - $i"
		}
		# Check command and then IP name
		if { [string match [lindex "$i" 1] "-append"] } {
	                if { [string match [lindex "$i" 2] "$nodename"] } {
				# Append it to the list
				set compact [lrange "$i" 3 end]
				set clist [concat $clist $compact]
				break;
			}
		} elseif { [string match [lindex "$i" 1] "-replace"] } {
	                if { [string match [lindex "$i" 2] "$nodename"] } {
				# Replace the whole compatible property list
				set clist [lrange "$i" 3 end]
				break;
			}
		} else {
	                if { [string match [lindex "$i" 1] "$nodename"] } {
				# Replace behavior
				set clist [lrange "$i" 2 end]
				break;
			}
		}
	}

	return [list "compatible" stringtuple $clist]
}

proc validate_ranges_property {slave parent_baseaddr parent_highaddr child_baseaddr} {
	set nodename [xget_hw_name $slave]
	if { ![llength $parent_baseaddr] || ![llength $parent_highaddr] } {
		error "Bad address range $nodename"
	}
	if {[string match $parent_highaddr "0x00000000"]} {
		error "Bad highaddr for $nodename"
	}
	set size [expr $parent_highaddr - $parent_baseaddr + 1]
	if { $size < 0 } {
		error "Bad highaddr for $nodename"
	}
	return $size
}

proc gen_ranges_property {slave parent_baseaddr parent_highaddr child_baseaddr} {
	set size [validate_ranges_property $slave $parent_baseaddr $parent_highaddr $child_baseaddr]
	return [list "ranges" hexinttuple [list $child_baseaddr $parent_baseaddr $size]]
}

proc gen_ranges_property_list {slave rangelist} {
	set ranges {}
	foreach range $rangelist {
		set parent_baseaddr [lindex $range 0]
		set parent_highaddr [lindex $range 1]
		set child_baseaddr [lindex $range 2]
		set size [validate_ranges_property $slave $parent_baseaddr $parent_highaddr $child_baseaddr]
		lappend ranges $child_baseaddr $parent_baseaddr $size
	}
	return [list "ranges" hexinttuple $ranges]
}

proc gen_interrupt_property {tree slave intc interrupt_port_list} {
	set intc_name [xget_hw_name $intc]
	set interrupt_list {}
	foreach in $interrupt_port_list {
		set irq [get_intr $slave $intc $in]

		if {![string match $irq "-1"]} {
			set irq_type [get_intr_type $intc $slave $in]
			if { "[xget_hw_value $intc]" == "ps7_scugic" } {
				lappend interrupt_list 0 $irq $irq_type
			} else {
				lappend interrupt_list $irq $irq_type
			}
		}
	}
	if {[llength $interrupt_list] != 0} {
		set tree [tree_append $tree [list "interrupts" inttuple $interrupt_list]]
		set tree [tree_append $tree [list "interrupt-parent" labelref $intc_name]]
	}
	return $tree
}

proc gen_reg_property {nodename baseaddr highaddr {name "reg"}} {
	if { ![llength $baseaddr] || ![llength $highaddr] } {
		error "Bad address range $nodename"
	}
	if {[string match $highaddr "0x00000000"]} {
		error "No high address for $nodename"
	}
	# Detect undefined baseaddr for MPMC CTRL
	if {[string match "0x[format %x $baseaddr]" "0xffffffff"]} {
		error "No base address for $nodename"
	}
	set size [expr $highaddr - $baseaddr + 1]
	if { [format %x $size] < 0 } {
		error "Bad highaddr for $nodename"
	}
	return [list $name hexinttuple [list $baseaddr $size]]
}

proc dts_override {root} {
	#PARAMETER periph_type_overrides = {dts <IP_name> <parameter> <value_type> <value>}
	global overrides

	foreach iptree $root {
		if {[lindex $iptree 1] != "tree"} {
			error {"tree_append called on $iptree, which is not a tree."}
		}

		set name [lindex $iptree 0]
		set name_list [split $name ":"]
		set hw_name [lindex $name_list 0]
		set node [lindex $iptree 2]

		foreach over $overrides {
			if {[lindex $over 0] == "dts"} {
				if { [llength $over] != 5 } {
					error "Wrong compatible override command string - $over"
				}

				if { $hw_name == [lindex $over 1] } {
					set over_parameter [lindex $over 2]
					set over_type [lindex $over 3]
					set over_value [lindex $over 4]
					set idx 0
					set node_found 0
					set new_node ""
					foreach list $node {
						set node_parameter [lindex $list 0]
						if { $over_parameter == $node_parameter } {
							set new_node "$over_parameter $over_type $over_value"
							set node [lreplace $node $idx $idx $new_node ]
							set node_found 1
						}
						incr idx
					}

					if { $node_found == 0 } {
						set new_node "$over_parameter $over_type $over_value"
						set node [linsert $node $idx $new_node ]
					}
				}
			}
		}
		set new_tree [list $name tree $node]
		if { [info exists new_root] } {
			lappend new_root $new_tree
		} else {
			set new_root [list $new_tree]
		}
	}

	if { [info exists new_root] } {
		return $new_root
	} else {
		return $root
	}
}

proc write_value {file indent type value} {
	if {[catch {
		if {$type == "int"} {
			puts -nonewline $file "= <[format %d $value]>"
		} elseif {$type == "hexint"} {
			# Mask down to 32-bits
			puts -nonewline $file "= <0x[format %x [expr $value & 0xffffffff]]>"
		} elseif {$type == "empty"} {
		} elseif {$type == "inttuple"} {
			puts -nonewline $file "= < "
			foreach element $value {
				puts -nonewline $file "[format %d $element] "
			}
			puts -nonewline $file ">"
		} elseif {$type == "hexinttuple"} {
			puts -nonewline $file "= < "
			foreach element $value {
				# Mask down to 32-bits
				puts -nonewline $file "0x[format %x [expr $element & 0xffffffff]] "
			}
			puts -nonewline $file ">"
		} elseif {$type == "bytesequence"} {
			puts -nonewline $file "= \[ "
			foreach element $value {
				if {[expr $element > 255]} {
					error {"Value $element is not a byte!"}
				}
				puts -nonewline $file "[format %02x $element] "
			}
			puts -nonewline $file "\]"
		} elseif {$type == "labelref"} {
			puts -nonewline $file "= <&$value>"
		} elseif {$type == "labelref-ext"} {
			puts -nonewline $file "= < &"
			foreach element $value {
				puts -nonewline $file "$element "
			}
			puts -nonewline $file ">"
		} elseif {$type == "aliasref"} {
			puts -nonewline $file "= &$value"
		} elseif {$type == "string"} {
			puts -nonewline $file "= \"$value\""
		} elseif {$type == "stringtuple"} {
			puts -nonewline $file "= "
			set first true
			foreach element $value {
				if {$first != true} { puts -nonewline $file ", " }
				puts -nonewline $file "\"$element\""
				set first false
			}
		} elseif {$type == "tree"} {
			puts $file "{"
			write_tree $indent $file $value
			puts -nonewline $file "} "
		} else {
			puts "unknown type $type"
		}
	} {error}]} {
		puts $error
		puts -nonewline $file "= \"$value\""
	}
	puts $file ";"
}

# tree: a tree triple
# child_node: a tree triple
# returns: tree with child_node appended to the list of child nodes
proc tree_append {tree child_node} {
	if {[lindex $tree 1] != "tree"} {
		error {"tree_append called on $tree, which is not a tree."}
	}
	set name [lindex $tree 0]
	set node [lindex $tree 2]
	lappend node $child_node
	return [list $name tree $node]
}

# tree: a tree triple
# child_node_name: name of the childe node that will be updated
# new_child_node: the new child_node node
proc tree_node_update {tree child_node_name new_child_node} {
	if {[lindex $tree 1] != "tree"} {
		error {"tree_append called on $tree, which is not a tree."}
	}
	set name [lindex $tree 0]
	set node [lindex $tree 2]
	set new_node []

	foreach p [lindex $tree 2] {
		set node_name [lindex $p 0]
		if { "[string compare $node_name $child_node_name ]" == "0" } {
			lappend new_node $new_child_node
		} else {
			lappend new_node $p
		}
	}
	return [list $name tree $new_node]
}

proc write_nodes {indent file tree} {
	set tree [lsort -index 0 $tree]
	foreach node $tree {
		if { [string match [expr [llength $node] % 3]  "0"] && [expr [llength $node] > 0]} {
			set loop_count [expr [llength $node] / 3 ]
			for { set i 0} { $i < $loop_count } { incr i } {
				set name [lindex $node [expr $i * 3 ]]
				set type [lindex $node [expr $i * 3 + 1]]
				set value [lindex $node [expr $i * 3 + 2]]
				puts -nonewline $file "[tt [expr $indent + 1]]$name "
				write_value $file [expr $indent + 1] $type $value
			}
		} elseif { [string match [llength $node] "4"] && [string match [lindex $node 1] "aliasref"] } {
			set name [lindex $node 0]
			set type [lindex $node 1]
			set value [lindex $node 2]
			puts -nonewline $file "[tt [expr $indent + 1]]$name "
			write_value $file [expr $indent + 1] $type $value
		} else {
			puts "Error_bad_tree_node length = [llength $node], $node"
		}
	}
}

proc write_tree {indent file tree} {
	set trees {}
	set nontrees {}
	foreach node $tree {
		if { [string match [lindex $node 1] "tree"]} {
			lappend trees $node
		} else {
			lappend nontrees $node
		}
	}
	write_nodes $indent $file $nontrees
	write_nodes $indent $file $trees

	puts -nonewline $file "[tt $indent]"
}

proc get_pathname_for_label {tree label {path /}} {
	foreach node $tree {
		set fullname [lindex $node 0]
		set type [lindex $node 1]
		set value [lindex $node 2]
		set nodelabel [string trim [lindex [split $fullname ":"] 0]]
		set nodename [string trim [lindex [split $fullname ":"] 1]]
		if {[string equal $label $nodelabel]} {
			return $path$nodename
		}
		if {$type == "tree"} {
			set p [get_pathname_for_label $value $label "$path$nodename/"]
			if {$p != ""} {return $p}
		}
	}
	return ""
}

# help function for debug purpose
proc debug {level string} {
	variable debug_level
	if {[lsearch $debug_level $level] != -1} {
		puts $string
	}
}

proc dma_channel_config {xdma addr mode intc slave devid} {
	set modelow [string tolower $mode]
	set namestring "dma-channel"
	set channame [format_name [format "%s@%x" $namestring $addr]]

	set chan {}
	lappend chan [list compatible stringtuple [list [format "xlnx,%s-%s-channel" $xdma $modelow]]]
	set tmp [scan_int_parameter_value $slave [format "C_INCLUDE_%s_DRE" $mode]]
	lappend chan [list "xlnx,include-dre" hexint $tmp]

	lappend chan [list "xlnx,device-id" hexint $devid]
	set tmp [xget_hw_parameter_handle $slave [format "C_%s_AXIS_%s_TDATA_WIDTH" [string index $mode 0] $mode]]
	if {$tmp != ""} {
		set tmp [scan_int_parameter_value $slave [format "C_%s_AXIS_%s_TDATA_WIDTH" [string index $mode 0] $mode]]
		lappend chan [list "xlnx,datawidth" hexint $tmp]
	}

	set tmp [xget_hw_parameter_handle $slave [format "C_%s_AXIS_%s_DATA_WIDTH" [string index $mode 0] $mode]]
	if {$tmp != ""} {
		set tmp [scan_int_parameter_value $slave [format "C_%s_AXIS_%s_DATA_WIDTH" [string index $mode 0] $mode]]
		lappend chan [list "xlnx,datawidth" hexint $mode]
	}

	if { [string compare -nocase $xdma "axi-dma"] != 0} {
		set tmp [scan_int_parameter_value $slave [format "C_%s_GENLOCK_MODE" $mode]]
		lappend chan [list "xlnx,genlock-mode" hexint $tmp]
	}

	set chantree [list $channame tree $chan]
	set chantree [gen_interrupt_property $chantree $slave $intc [list [format "%s_introut" $modelow]]]

	return $chantree
}

proc is_gmii2rgmii_conv_present {slave} {
	set port_value 0
	set phy_addr -1
	set ipconv 0

	# No any other way how to detect this convertor
	set mhs_handle [xget_hw_parent_handle $slave]
	set ips [xget_hw_ipinst_handle $mhs_handle "*"]
	set ip_name [xget_hw_name $slave]

	foreach ip $ips {
		set periph [xget_value $ip "value"]
		if { [string compare -nocase $periph "gmii_to_rgmii"] == 0} {
			set ipconv $ip
			break
		}
	}
	if { $ipconv != 0 }  {
		set port_value [xget_hw_port_value $ipconv "gmii_txd"]
		if { $port_value != 0 } {
			set tmp [string first "ENET0" $port_value]
			if { $tmp >= 0 } {
				if { [string compare -nocase $ip_name "ps7_ethernet_0"] == 0} {
					set phy_addr [scan_int_parameter_value $ipconv "C_PHYADDR"]
				}
			} else {
				set tmp0 [string first "ENET1" $port_value]
				if { $tmp0 >= 0 } {
					if { [string compare -nocase $ip_name "ps7_ethernet_1"] == 0} {
						set phy_addr [scan_int_parameter_value $ipconv "C_PHYADDR"]
					}
				}
			}
		}
	}
	return $phy_addr
}
