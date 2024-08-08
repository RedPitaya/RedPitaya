APP=$(notdir $(CURDIR:%/=%))

# Versioning system
BUILD_NUMBER ?= 0
REVISION ?= devbuild

INSTALL_DIR ?= /opt/redpitaya/
LINUX_VER ?= dev
CONTROLLERHF = controllerhf.so

CFLAGS += -DVERSION=$(LINUX_VER)-$(BUILD_NUMBER) -DREVISION=$(REVISION)
export CFLAGS

all: $(CONTROLLERHF)

$(CONTROLLERHF):
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean
	-$(RM) *.so
