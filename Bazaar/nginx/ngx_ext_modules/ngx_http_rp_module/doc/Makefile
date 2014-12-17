#
# Copyright (c) 2013 Instrumentation Technologies
# All Rights Reserved.
#
# $Id: Makefile 465 2013-09-03 11:47:30Z damijan.skvarc $
#

.PHONY : all clean

PLANTUML_JAR ?= /usr/share/java/plantuml.jar
PDF_FILENAME := RedPitaya_NGINX_module.pdf

all:
	java -jar $(PLANTUML_JAR) -forcecairo -o img *.dox
	java -jar $(PLANTUML_JAR) -forcecairo -o ./img ../src/*.c
	java -jar $(PLANTUML_JAR) -forcecairo -o ./img ../src/*.h
	doxygen
	make -C latex pdf
	cp latex/refman.pdf $(PDF_FILENAME)

clean:
	-rm -rf img html latex
	-rm $(PDF_FILENAME)

