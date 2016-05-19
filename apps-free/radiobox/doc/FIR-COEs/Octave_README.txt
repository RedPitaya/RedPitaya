===================================
Octave calculation for coefficients
===================================


1)  Enter the transfer function followed by the plot command

	hn=fir1(254, 3000/200000, 'low', 'barthannwin'); freqz(hn);


2)  Write out coefficients to a file

	dlmwrite("OUTPUT_FILENAME.txt", hn, "delimiter", ", ", "newline", "pc", "precision", "%0.25f");



<EOF>
