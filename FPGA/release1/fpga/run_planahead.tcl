

#*******************************************************************************
# Simple script to control PlanAhead project via command line
#
# It takes two argument
#   clean   cleans project files
#   build   build all output files



#*******************************************************************************
# Open project
open_project ./ahead/red_pitaya.ppr


#*******************************************************************************
# Update sources
update_compile_order -fileset sources_1


##*******************************************************************************
## Clean project

if {[lindex $argv 0] == "clean"} {
   ## clean implementation
   reset_run impl_1

   ## clean synthesis
   reset_run synth_1

   ## clean PS project configuration
   reset_target all [get_files  ./ahead/red_pitaya.srcs/sources_1/edk/system/system.xmp]
}




##*******************************************************************************
## Make output files

if {[lindex $argv 0] == "build"} {

   ## export PS configuration
   generate_target all [get_files  ./ahead/red_pitaya.srcs/sources_1/edk/system/system.xmp]
   export_hardware [get_files ./ahead/red_pitaya.srcs/sources_1/edk/system/system.xmp]

   ## do synthesis
   launch_runs synth_1
   wait_on_run synth_1

   ## do implementation
   launch_runs impl_1
   wait_on_run impl_1

   ## make bit file
   launch_runs impl_1 -to_step Bitgen
   wait_on_run impl_1
}




#*******************************************************************************
# Close opened project
close_project



exit

