#
# $Id: $
#
# Red Pitaya library librp and test app Makefile.
#

LIBRP=lib/librp1.so

all: $(LIBRP)

$(LIBRP):
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

install:
	$(MAKE) -C src install INSTALL_DIR=$(abspath $(INSTALL_DIR))
