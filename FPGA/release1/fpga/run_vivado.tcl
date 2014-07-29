#*******************************************************************************
# Simple script to control vivado project via command line
#
# It takes two argument
#   clean   cleans project files
#   build   build all output files



#*******************************************************************************
# Open project
open_project ./vivado/red_pitaya.xpr


#*******************************************************************************
# Update sources
update_compile_order -fileset sources_1


##*******************************************************************************
## Clean project

if {[lindex $argv 0] == "clean"} {
   ## clean implementation
   set_property incremental_checkpoint {} [get_run impl_1]
   file delete -force ./vivado/red_pitaya.runs/impl_1_previous
   reset_run impl_1

   ## clean synthesis
   reset_run synth_1

   ## clean PS project configuration
   reset_target all [get_files  ./vivado/red_pitaya.srcs/sources_1/bd/system/system.bd]
}




##*******************************************************************************
## Make output files
# This is really not involved in generation of the bit file
if {[lindex $argv 0] == "build_system_xml"} {
   ## export PS configuration
   # TODO: check system.bd timestamp and the xml timestamp to see if we actually need to update anything
   # This takes like 1 minute, I want to save that minute....
   generate_target all [get_files  ./vivado/red_pitaya.srcs/sources_1/bd/system/system.bd]
   open_bd_design ./vivado/red_pitaya.srcs/sources_1/bd/system/system.bd
   export_hardware [get_files ./vivado/red_pitaya.srcs/sources_1/bd/system/system.bd]
   close_bd_design system
}

# We add logic to only conditionally remake the design,
# The logic is the following, we check two properties of the run
# NEEDS_REFRESH is set to 1 if the run needs to be rerun, based on the
# modifications you made to the project files
#
# the STATUS property can have a few values
#
# For synthesis:
# "Not started"
# "synth_design Complete!"
#
# For implementation:
# "Not started"
# "Not started phys_opt_design" <- after place
# "Running Design Initialization..."
# "route_design Complete!" <- There seems to be 3 steps opt_design, place_design, and route_design
# "write_bitstream Complete!"
if {[lindex $argv 0] == "build"} {
   # Check if the synthesis/implementation run needs to be refreshed
   # NEEDS_REFRESH will be 1 if this run needs to be redone
   # so first you need to reset it
   ## do implementation
   if {[get_property NEEDS_REFRESH [get_runs impl_1]] == 1} {
      # we keep the old run to incrementally compile from it :D
      reset_run -noclean_dir impl_1
      file delete -force ./vivado/red_pitaya.runs/impl_1_previous
      file rename ./vivado/red_pitaya.runs/impl_1 ./vivado/red_pitaya.runs/impl_1_previous
   }
   # resetting the synth_1 run will acutally reset the impl_1 run
   # putting the logic in this order decreases code duplication while keeping
   # the old implemented run
   if {[get_property NEEDS_REFRESH [get_runs synth_1]] == 1} {
      reset_run synth_1
   }

   ## do synthesis

   # TODO: Vivado seems to be able to check for syntax errors,
   # It would be REALLY awesome if we could use that check and speed up the detection
   # of syntax errors instead of trying to build it and waiting for errors

   # TODO:
   # Make sure that TCL_OK is returned, and exit 1
   # see:
   # https://www.tcl.tk/man/tcl/TclCmd/catch.htm
   # TODO: run with wait_on_runs -quiet, this will cause the run to NEVER FAIL
   # therefore, we will have to check the status for complete and NOT failure after wait_on_runs finishes
   #
   # TODO: also check if it is already running.... it doesn't like to run something twoce
   #
   # TODO: find some kind of progress bar.... because if something takes 10 minutes, I want to see a progress bar....
   # TODO: Output the summary report at the end of the synthesis

   # Check if the design is complete
   if {[string equal [get_property STATUS [get_runs synth_1]] "synth_design Complete!"] == 0} {
      puts "Launching synth_1, this may take a while..."
      launch_runs synth_1
      wait_on_run -quiet synth_1
      if {[string equal [get_property STATUS [get_runs synth_1]] "synth_design Complete!"] == 0} {
         puts "Error in synthesis."
         puts "Open project in Vivado for more information."
         exit 1
      }
   }
   puts [get_property STATUS [get_runs synth_1]]


   # placement and routing can be done incrementally, therefore try to do them incrementally
   # FIXME: I assume the route checkpoint holds the place information too. Maybe not :S.
   # This needs further testing
   if {[file exists ./vivado/red_pitaya.runs/impl_1_previous/red_pitaya_top_routed.dcp] == 1} {
      set_property incremental_checkpoint ./vivado/red_pitaya.runs/impl_1_previous/red_pitaya_top_routed.dcp [get_run impl_1]
   } elseif {[file exists ./vivado/red_pitaya.runs/impl_1_previous/red_pitaya_top_placed.dcp] == 1} {
      set_property incremental_checkpoint ./vivado/red_pitaya.runs/impl_1_previous/red_pitaya_top_placed.dcp [get_run impl_1]
   } else {
      set_property incremental_checkpoint {} [get_run impl_1]
   }

   # check if the bistream has been written
   # TODO: does vivado check if the bitstream file actually exists checking if the bistream file exists
   # if it doesn't we should rebuild it....
   if {[string equal [get_property STATUS [get_runs impl_1]] "write_bitstream Complete!"] == 0} {

      # TODO: maybe we can break this apart into the smaller steps, OPT, place, route, bitstream
      # that way we get some kind of progress bar...
      # unfortunately, the logic is kinda crazy
      puts "Launching impl_1, this may take a while... (like a really long time....)"
      launch_runs impl_1 -to_step write_bitstream

      # I don't want to have this part of the compilation be quiet until I get some kind of progress bar...
      wait_on_run impl_1
      # TODO: how do you catch an error in TCL....
      #wait_on_run impl_1 -quiet
      if {[string equal [get_property STATUS [get_runs impl_1]] "write_bitstream Complete!"] == 0} {
         puts "Error in implementation."
         puts "Open project in Vivado for more information."
         exit 1
      }
   }
   puts [get_property STATUS [get_runs impl_1]]
}




#*******************************************************************************
# Close opened project
close_project



exit

