what comes free comes without guarantee.....

Gain/phase/impedance analyser on RedPitaya 
by Martin Ossmann, ossmann@fh-aachen.de
20.th August 2014


This software is as it is....
Here some comments for people who want to understand it or make it better.

If someone plans to make a web-app out of it with graphical
user interface a good source for uideas is the manual of
the Hewlett Packard 4194A Impedance/Gain-Phase Analyzer
(available on the net). You may also contact me for discussion.

Now som comments about the program:

The main program first scans the arguments and sets the options in the 
"theOptions" structure. 

Then the oscilloscope-worker is started. This is used for sampling  of signals.
It is the software also used by the "acquire" software delivered with the RedPitaya.

Then the main-routine sweeps over the frequencies. For each frequency the signal 
generator is set to the wanted frequency.  This is done by the software borrowed 
from the "genertor" application supplied with the RedPitaya. Also
the signal sampling parameters (decimation factor) is set according to the frequency.

(I have tested the design starting with frequencies at 1kHz. Using  higher decimation 
factors and longer measurement times it should be possible to use lower frequencies also)


After waittng a setup time a first set of samples is sampled as dummy valus.
Then a second set is sampled and used for measuring the wanted parameters
Sampling and (ooptional) measuring is done by the routine
   doOneMeasurement(s,fSample,frq,0,outfp, theOptions) ;

(This is a weak point in the design. It would be better to synchronize signal 
generation and signal sampling such that the samples are taken when enough
periods of the signal have been generated)

doOneMeasurement() gets the samples from the oscilloscope-worker.
If it succedds, it uses the routine
    analyseSignal(size,s,fSample,fMeasure,outfp, theOptions) ;
to do the analysis. This routine uses a least-squares approach
to estimate phase and amplitude of the sinusoidal input sampled ADC signals.
It estimates the coefficients a,b,c in the Ansatz 
  s(Tk)=a+b*sin(2*pi*f*Tk)+c*cos(2*pi*f*Tk)
where Tk are the sampling times of the samples.
It uses a simle linear-algebra subroutine package to solve the 
3x3 linear equation systems. After this estimation
the phase and amplitude of the both input signals are known.

Then gain and phase are computed and the routine
   impedanceAnalyse(fMeasure,uXX,uYY,theOptions) ;
is called to do the impedance analysis. the routine
  impedanceAnalyse(double fMeasure, complex uXX, complex uYY, options_t theOptions)
uses the complex form of the voltages to compute voltage and current
accross the device under test DUT. Then the complex impedance z is computed
and this is converted into the quantities the user has selected.

The following routines have been programmed by me:
  linAlg.h linAlg.c     simple linear algebra routines
  genGtrl.c             interafce to signal generator control
  GPIanalyse.c          all gain/phase/impedance stuff
All other modules are taken from the original RedPitaya software
form the signal generator application and from the oscilloscope application.



