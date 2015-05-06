#export CROSS_COMPILE=arm-xilinx-eabi-
#source /usr/local/xilinx/Vivado/2013.3/settings32.sh
#export PATH=$PATH:home/user/websocketpp/config/
#export PATH=$PATH:/home/user/work/RedPitaya-master/OS/u-boot/u-boot-xlnx/tools
#make > my.log 2>&1
#export PATH=$PATH:home/user/websocketpp/
#gcc telemetry_server.cpp -Iinclude-path /home/user/websocketpp
export BOOST_INCLUDES=/usr/include/boost
export BOOST_LIBS=/usr/lib
#export PATH=$PATH:home/user/work/jsoncpp-master/build/debug/src/lib-json
scons examples/telemetry_server/
