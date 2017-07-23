LIBRP=lib/librp1.so

all: $(LIBRP)

$(LIBRP):
	$(MAKE) -C src

docapi:
	$(MAKE) -C doc

clean:
	$(MAKE) -C src clean
	$(MAKE) -C doc clean

install:
	$(MAKE) -C src install INSTALL_DIR=$(abspath $(INSTALL_DIR))
