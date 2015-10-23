#
# Red Pitaya Install Makefile
#

INSTALL_DIR ?= ../
INSTALL_NAME = sdk.zip

install:
	mkdir -p target
	cp -r install.sh run.sh create_template.sh ../api/include Makefile_target ./target/
	cd target; mv Makefile_target Makefile; zip $(INSTALL_NAME) *
	cd target; cp $(INSTALL_NAME) $(INSTALL_DIR)

clean:
	-$(RM) $(TARGET) *.o ~*
	-$(RM) -r target
	-$(RM) $(INSTALL_NAME)
