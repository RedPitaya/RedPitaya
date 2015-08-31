# Free applications

## Application list:
- Scope
- Scope+gen
- Scope+pid
- spectrum
- freqanalyzer


## Application structure

Scope, scope+gen, scope+pid have all similar structure

|  path                           | contents
|---------------------------------|---------
| `apps-free/Makefile`            | Main Makefile used to build all applications listed here.
| `apps-free/app_name/index.html` | Main client GUI file. It is used for graphical view of the application in the web-browser
| `apps-free/app_name/Makefile`   | Application Makefile, used to build src intro controller.so and controllerhf.so
| `apps-free/app_name/info`       | Application meta-data in the application list of a red pitaya
| `apps-free/app_name/src`        | Main source directory. Most of C code resides here.
| `apps-free/app_name/fpga.conf`  | File containing the fpga.bit file location for each specific application.
| `apps-free/app_name/doc`        | Documentation directory

Spectrum and Freqanalyzer
-------------------------

These applications have a slightly different structure in the src directory.

| path                              | contents
|-----------------------------------|---------
| `apps_name/src/external/kiss_fft` | Fast fourier transform directory; kiss distribution.


# Build process

Before building the applications, you need or should set your working environment.

You need to set the CROSS_COMPILE variable. Check to see if is is already set by using the following command:
```bash
env | grep CROSS_COMPILE
```
The output should be something like: CROSS_COMPILE=arm-linux-gnueabihf-. 
If this gives you a blank result, you should either set the CROSS_COMPILE variable to be pointing at the gnu cross
compiler or check our how to build red pitaya OS [wiki-page](http://wiki.redpitaya.com/index.php?title=Red_Pitaya_OS)

If you don't have gcc cross compiler installed, you can install it with the following command:
```bash
sudo wget https://releases.linaro.org/14.11/components/toolchain/binaries/arm-linux-gnueabihf/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf.tar.xz
```
Extract the content of this .tar.xz file you just downloaded
```bash
tar xvf gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf.tar.xz
```
Now all you need to do, is set the PATH variable to include the linaro directory.
```bash
export PATH=$PATH:./gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf/bin
```
NOTE: You can copy the extracted linaro directory where ever you want, but remember to set the PATH variable accordingly.

Now export the CROSS_COMPILE variable.
```bash
export CROSS_COMPILE=arm-linux-gnueabihf-
```
Once you did all that, you can use the same command as before, to check you completed these steps successfuly.
```bash
env | grep CROSS_COMPILE
```
which should now, in term, give you the desired output.


Now it's time to build our applications. You can run the Makefile from the apps-free directory building all
the applications listed all at once. Or you can navigate to the specific application. Either way, the command remains the same:
```bash
make clean all
```
This way we are going to first clean some old artifacts, remaining from older builds and build a fresh, new copy.

