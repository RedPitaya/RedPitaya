The previous README file is available [here](doc/developer.rst).

Work in progress documentation <http://redpitaya.readthedocs.io/en/latest/index.html>.

# CALL FOR DEVELOPERS

Our internal development is shifting toward [Jupyter](http://jupyter.org/).
Jupyter with a set of Python modules provides a great tool for quick prototyping:
1. UIO drivers written directly in Python,
2. dynamic and interactive data visualization with [bokeh](http://bokeh.pydata.org),
3. data storage and processing with [numpy](http://www.numpy.org/), [scipy](https://www.scipy.org/), ...
4. source code management with Git and [GitHub](https://github.com/blog/1995-github-jupyter-notebooks-3),
5. and under development is support for device tree overlays and FPGA manager.

## 1. UIO (Userspace IO) drivers in Python
Drivers for FPGA memory mapped peripherals can be written directly in Python using
[mmap](https://docs.python.org/2/library/mmap.html), [ctypes](https://docs.python.org/3/library/ctypes.html)
and [numpy](http://www.numpy.org/).
[UIO description in a device tree](https://github.com/RedPitaya/RedPitaya/blob/mercury/jupyter/experiments/mercury.dts)
in combination with [UDEV rules](https://github.com/RedPitaya/RedPitaya/blob/mercury/OS/debian/overlay/etc/udev/rules.d/10-redpitaya.rules#L7)
provides named UIO devices as `/dev/uio/name`.
Each device can be mapped into memory space and locked separately with
[mmap](https://docs.python.org/2/library/mmap.html) and
[fcntl](https://docs.python.org/3.6/library/fcntl.html).
Register sets can be written using Python ctypes and/or
[numpy.dtpye](https://docs.scipy.org/doc/numpy/reference/generated/numpy.dtype.html).
Python offers language features for the creation of elegant APIs.

## 2. Dynamic and interactive data visualization
As a base [Matplotlib](http://matplotlib.org/) provides a vast array of features for data visualization.
With the addition of [bokeh](http://bokeh.pydata.org) (JavaScript based library) visualization become dynamic
with good frame rates (up to about 16fps depending on data size).
There are widget libraries available for making interactive applications.

## 3. Data storage and digital signal processing
Python makes it easy to store data into a file, since the application is also written in Python,
it easy to access data from various processing changes not just a filtered output.
A simple data logger for example can write data file onto the SD card,
the file can be later loaded onto a PC using the Jupyter file browser.
Python is interpreted in Jupyter, which makes it slow.
Fortunately most data processing can be done on arrays with dedicated libraries.
Numpy and Scipy provide processing functions for a great spectrum of applications.
They are well optimized although not very fast on the ZYNQ ARM CPU.
Python wrappers can be written around optimized DSP libraries like [Ne10](https://projectne10.github.io/Ne10/).
Processing can also be offloaded to the FPGA, project [PYNQ](https://github.com/Xilinx/PYNQ/) is making progress there.

## 4. Git and GitHub
Applications can be developed directly on the board and edited in the Jupyter editor.
Git is now installed on SD card images.
In combination with Github it provides a great tool for version control,
publishing and distribution of Python applications and libraries.
TODO: under Welcome instructions for Git SSH keys, maybe they should be created at first boot and displayed.

## 5. Device tree overlays and FPGA manager
Device tree overlays are already supported on our current 4.4 based kernel.
We are working on a kernel 4.9 based version, which would also support FPGA manager.
FPGA manager enables loading a FPGA bitstream with an overlay,
which enables proper loading and unloading of kernel drivers (GPIO, LED, XADC, DMA, ...)
needed by the overlay.
Most of our problems are related to backward compatibility.
