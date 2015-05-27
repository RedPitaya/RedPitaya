Building tools:


 - idgen

idgen - id file generator
build.sh - script to build idgen tool

Go to licmng folder and run script
	./build.sh

it uses arm-xilinx-linux-gnueabi-g++ compiler, to change it - change it in both scripts: idgen/build.sh and in cryptopp/build.sh 

It also builds cryptopp library and libjson library required by idgen tool.


 - licmng

licmng - license generator
build.sh - script to build licmng tool

Go to licmng folder and run script
	./build.sh

it uses arm-xilinx-linux-gnueabi-g++ compiler, to change it - change it in both scripts: idgen/build.sh and in cryptopp/build.sh 

It also builds cryptopp library and libjson library required by licmng tool.
