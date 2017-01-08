Python applications should be executed with the device IP address in the command line.
```bash
$ python example-script.py 192.168.1.100
```

This example is written and completely supported by Python 2.7
TODO:Export to python 3.4

Coding in python directly on th RedPitaya:

Some scrips need aditional packages installed:
```bash
$ apt-get update
$ apt-get install python-matplotlib python-tk
```

Hardcode IP addres in the script:
```bash
rp_s = scpi.scpi('192.168.1.100')
```

Prevent error: _tkinter.TclError: no display name and no $DISPLAY environment variable
```bash
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plot
```
