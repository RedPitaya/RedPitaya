#
# $Id: $
#
# Red Pitaya documentation for librp Makefile.
#


DOXY_CONF=$(CURDIR)/conf/doxyfile.inc
DOXY_STARTUP_DIR=../src/
DOXY_RUN=doxygen

OUT_DIR=./doxygen

DOC=doxygen/html

all: $(DOC)

$(DOC):  
	cd $(DOXY_STARTUP_DIR); $(DOXY_RUN) $(DOXY_CONF)

clean:
	rm -f -r $(OUT_DIR)/*

