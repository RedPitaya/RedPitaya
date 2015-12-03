# Makefile to build python with some package 
# Author : Pierre Cladé


VERSION ?= 0.00-0000
REVISION ?= devbuild

DESTDIR ?= tmpdir
TMPDIR = tmpdir

# Main GCC executable (used for compiling and linking)
CC=$(CROSS_COMPILE)gcc
LDSHARED=$(CROSS_COMPILE)gcc -shared
CXX=$(CROSS_COMPILE)cpp 
AR=$(CROSS_COMPILE)ar 
RANLIB=$(CROSS_COMPILE)ranlib

BASE = ../OS/buildroot/buildroot-2014.02/output/target/
LDFLAGS=-L${BASE}/lib -L${BASE}/usr/lib 

# Installation directory
INSTALL_DIR ?= .

PYTHON_SOURCE = Python-2.7.3.tgz
HOSTPYTHON = hostpython/Python-2.7.3/python
HOSTPGEN = hostpython/Python-2.7.3/Parser/pgen
ABS_HOSTPYTHON=$(abspath $(HOSTPYTHON))
HOSTPYTHON_PATH=$(abspath hostpython/Python-2.7.3/)
ABS_HOSTPGEN=$(abspath $(HOSTPGEN))
ARMPYTHON = $(TMPDIR)/usr/bin/python

NUMPY=$(TMPDIR)/usr/lib/python2.7/site-packages/numpy
NUMPYSRC = numpy-1.7.1

IPYTHON=$(TMPDIR)/usr/bin/ipython
IPYTHONSRC = ipython-3.0.0

RPYC=$(TMPDIR)/usr/lib/python2.7/site-packages/rpyc
RPYCSRC = rpyc-3.3.0

CACHED_PROPERTY=$(TMPDIR)/usr/lib/python2.7/site-packages/cached_property-0.1.5-py2.7.egg-info
CACHED_PROPERTYSRC=cached-property-0.1.5

MYHDL=$(TMPDIR)/usr/lib/python2.7/site-packages/myhdl
MYHDLSRC=myhdl-0.8.1

PYREDPITAYA=$(TMPDIR)/usr/lib/python2.7/site-packages/PyRedPitaya
PYREDPITAYASRC=PyRedPitaya-1.0

PYTHON_PATCH = Python-2.7.3-xcompile.patch

PYTHON_XCOMPILE_DEPENDENCIES_PREFIX=$(abspath ../OS/buildroot/buildroot-2014.02/output/target/)
#PYTHON_XCOMPILE_DEPENDENCIES_PREFIX=$(abspath ../OS/buildroot/buildroot-2014.02/output/host/usr/armeb-buildroot-linux-gnueabi/sysroot/usr/)
PYPREFIX=$(abspath $(TMPDIR)/usr)

CFLAGS= -I$(PYTHON_XCOMPILE_DEPENDENCIES_PREFIX)/../host/usr/armeb-buildroot-linux-gnueabi/sysroot/usr/include -I${PYPREFIX}/include/python2.7

$(PYTHON_SOURCE):
	wget https://www.python.org/ftp/python/2.7.3/Python-2.7.3.tgz

$(HOSTPYTHON): $(PYTHON_SOURCE)
	mkdir -p hostpython
	cd hostpython; 	tar -xvzf ../Python-2.7.3.tgz; 	cd Python-2.7.3; ./configure; make -j 10 python Parser/pgen 
	cd hostpython/Python-2.7.3; make install prefix="./install" DESTDIR=$(HOSTPYTHON_PATH) 
	
#$(PYTHON_PATCH):
#	wget http://randomsplat.com/wp-content/uploads/2012/10/Python-2.7.3-xcompile.patch
$(DESTDIR):
	mkdir -p $(DESTDIR)

$(TMPDIR):
	mkdir -p $(TMPDIR)

$(ARMPYTHON): $(HOSTPYTHON) $(TMPDIR) 
	mkdir -p python
	cd python; tar -xvzf ../Python-2.7.3.tgz 
	cd python/Python-2.7.3; patch -p1 < ../../Python-2.7.3-xcompile.patch
	cd python/Python-2.7.3; CC="$(CC)" CXX="$(CXX)" AR="$(AR)" RANLIB="$(RANLIB)" ./configure --host=arm-linux --build=x86_64-linux-gnu --prefix=/usr
	cd python/Python-2.7.3; PYTHON_XCOMPILE_DEPENDENCIES_PREFIX=$(PYTHON_XCOMPILE_DEPENDENCIES_PREFIX) make -j 10 HOSTPYTHON="${ABS_HOSTPYTHON}" HOSTPGEN="${ABS_HOSTPGEN}" BLDSHARED="${LDSHARED}" CROSS_COMPILE="$(CROSS_COMPILE)" CROSS_COMPILE_TARGET=yes HOSTARCH=arm-linux BUILDARCH=x86_64-linux-gnu 
	cd python/Python-2.7.3; make install HOSTPYTHON="${ABS_HOSTPYTHON}" BLDSHARED="${LDSHARED}" CROSS_COMPILE="$(CROSS_COMPILE)" CROSS_COMPILE_TARGET=yes DESTDIR=../../$(TMPDIR)

$(NUMPYSRC):
	wget http://pypi.python.org/packages/source/n/numpy/$(NUMPYSRC).tar.gz
	tar -xvzf numpy-1.7.1.tar.gz
	cd $(NUMPYSRC)/; patch -p1 <../fix-numpy.patch

$(NUMPY): $(ARMPYTHON) $(NUMPYSRC)
	cd $(NUMPYSRC)/;CC="$(CC)" AR="$(AR)" RANLIB="$(RANLIB)" LDFLAGS="$(LDFLAGS) -L${PYPREFIX}/lib/python2.7" CFLAGS="${CFLAGS}" LDSHARED="$(LDSHARED)" $(ABS_HOSTPYTHON) setup.py install --prefix=$(PYPREFIX)

$(IPYTHONSRC):
	wget https://pypi.python.org/packages/source/i/ipython/$(IPYTHONSRC).tar.gz#md5=b3f00f3c0be036fafef3b0b9d663f27e
	tar -xzf $(IPYTHONSRC).tar.gz
	cd $(IPYTHONSRC)/; patch <../ipython-setupbase.patch

$(IPYTHON): $(ARMPYTHON) $(IPYTHONSRC)
	cd $(IPYTHONSRC)/;CC="$(CC)" AR="$(AR)" RANLIB="$(RANLIB)" LDFLAGS="$(LDFLAGS) -L${PYPREFIX}/lib/python2.7" CFLAGS="${CFLAGS}" LDSHARED="$(LDSHARED)" $(ABS_HOSTPYTHON) setup.py install --prefix=$(PYPREFIX)

$(RPYCSRC):
	wget https://pypi.python.org/packages/source/r/rpyc/$(RPYCSRC).tar.gz#md5=6931cb92c41f547591b525142ccaeef1
	tar -xzf $(RPYCSRC).tar.gz

$(RPYC): $(RPYCSRC) $(ARMPYTHON) 
	cd $(RPYCSRC)/; $(ABS_HOSTPYTHON) setup.py install --prefix=$(PYPREFIX)

$(CACHED_PROPERTYSRC):
	wget https://pypi.python.org/packages/source/c/cached-property/$(CACHED_PROPERTYSRC).tar.gz#md5=6e9132c59fcfa338a4d50b42da780983
	tar -xzf $(CACHED_PROPERTYSRC).tar.gz

$(CACHED_PROPERTY): $(CACHED_PROPERTYSRC) $(ARMPYTHON) 
	cd $(CACHED_PROPERTYSRC)/; $(ABS_HOSTPYTHON) setup.py install --prefix=$(PYPREFIX)

$(MYHDLSRC):
	wget https://pypi.python.org/packages/source/m/myhdl/$(MYHDLSRC).tar.gz#md5=f12ed83bd758ef35e9fe5052b6f5dbc7
	tar -xzf $(MYHDLSRC).tar.gz

$(MYHDL): $(MYHDLSRC) $(ARMPYTHON) 
	cd $(MYHDLSRC)/; $(ABS_HOSTPYTHON) setup.py install --prefix=$(PYPREFIX)

$(PYREDPITAYA):
	cd $(PYREDPITAYASRC)/monitor/;CC="$(CC)" AR="$(AR)" RANLIB="$(RANLIB)" LDFLAGS="$(LDFLAGS) -L${PYPREFIX}/lib/python2.7" CFLAGS="${CFLAGS}" LDSHARED="$(LDSHARED)" make all
	cd $(PYREDPITAYASRC)/monitor/;make install INSTALL_DIR=$(PYPREFIX)
	cd $(PYREDPITAYASRC); $(ABS_HOSTPYTHON) setup.py install --prefix=$(PYPREFIX)

all: $(ARMPYTHON) $(NUMPY) $(IPYTHON) $(MYHDL) $(RPYC) $(CACHED_PROPERTY) $(PYREDPITAYA) $(TMPDIR)/bin
	
install: $(DESTDIR)
	cp -rf $(TMPDIR)/* $(DESTDIR)

$(TMPDIR)/bin :
	mkdir -p $(TMPDIR)/bin
	cp example/led $(TMPDIR)/bin
	cp example/rpyc_server $(TMPDIR)/bin
	cp example/send_temperature_by_email $(TMPDIR)/bin

clean:
	rm -rf hostpython
	rm -rf python
	rm $(PYTHON_SOURCE)
	rm -rf $(NUMPYSRC)
	rm -r $(NUMPYSRC).tar.gz
	rm -rf $(IPYTHONSRC)
	rm -r $(IPYTHONSRC).tar.gz
	rm -rf $(RPYCSRC)
	rm -r $(RPYCSRC).tar.gz
	rm -rf $(MYHDLSRC)
	rm -r $(MYHDLSRC).tar.gz
	rm -rf $(CACHED_PROPERTYSRC)
	rm -r $(CACHED_PROPERTYSRC).tar.gz
	rm -rf $(TMPDIR)


