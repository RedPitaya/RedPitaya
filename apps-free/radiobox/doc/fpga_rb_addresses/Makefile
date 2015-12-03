#
# Copyright (c) 2015 DF4IAH
# All Rights Reserved.
#
# Makefile 2015-09-30 10:30:00Z espero7757@gmx.net $
#

.PHONY : all pdf clean

DOCUMENTSRC  := fpga_rb_addresses
LYX_SRC      := $(DOCUMENTSRC).lyx
LATEX_SRC    := $(DOCUMENTSRC).tex
PDF_FILENAME := RedPitaya_RadioBox_FPGA-Register-Allocation_Manual.pdf

all: pdf

pdf $(PDF_FILENAME): clean $(LYX_SRC) $(LATEX_SRC)
	# LyX to LaTeX  (TODO)
	pdflatex $(DOCUMENTSRC)
	makeindex $(DOCUMENTSRC).idx
	pdflatex $(DOCUMENTSRC)
	latex_count=8 ; \
	while egrep -s 'Rerun (LaTeX to get cross-references right)' $(DOCUMENTSRC).log && [ $$latex_count -gt 0 ] ;\
	    do \
	      echo "Rerunning latex...." ;\
	      pdflatex $(DOCUMENTSRC) ;\
	      latex_count=`expr $$latex_count - 1` ;\
	    done
	makeindex $(DOCUMENTSRC).idx
	pdflatex $(DOCUMENTSRC)

clean:
	$(RM) *.pdf *.ps *.dvi *.aux *.toc *.idx *.ind *.ilg *.log *.out *.brf *.blg *.bbl
