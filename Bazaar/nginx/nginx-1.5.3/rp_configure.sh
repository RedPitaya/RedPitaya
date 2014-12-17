#!/bin/sh
#
# $Id: rp_configure.sh 1233 2014-02-21 15:44:47Z tomaz.beltram $
#
# Configure nginx for Red Pitaya.
#
# version and revision passed as parameters
#
# The objs/Makefile is modified in script at the end.
#


# Remove everything we can.
# We will add things one-by-one accoording to our needs in add_modules.
without=$(cat configure_withouts.txt)

add_modules="--add-module=../ngx_ext_modules/lua-nginx-module --add-module=../ngx_ext_modules/ngx_http_rp_module"
add_conf_params="--with-zlib=../../../OS/buildroot/buildroot-2014.02/output/build/zlib-1.2.8/ --with-pcre=../../../OS/buildroot/buildroot-2014.02/output/build/pcre-8.34/"
export VERSION=$1
export REVISION=$2
echo ${without} | xargs ./configure ${add_conf_params} ${add_modules}


# Make it cross-compilable
cat objs/Makefile | tail -n +3 > objs/Makefile.tmp
echo "CROSS_COMPILE=arm-xilinx-linux-gnueabi-" > objs/Makefile
echo "CC =	\$(CROSS_COMPILE)gcc" >> objs/Makefile
cat objs/Makefile.tmp >> objs/Makefile
rm objs/Makefile.tmp
