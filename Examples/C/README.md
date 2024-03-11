# Compiling and running on target

When compiling on the target no special preparations are needed. A native toolchain is available directly on the Debian system.

First connect to your board over SSH (replace the IP, the default password is `root`).
```bash
ssh root@192.168.0.100
```

Now on the target, make a clone of the Red Pitaya Git repository and enter the project directory.
```bash
git clone https://github.com/RedPitaya/RedPitaya.git
cd RedPitaya
```

To compile one example just use the source file name without the `.c` extension.
```bash
cd Examples/C
make digital_led_blink
```

```bash
cd Examples/C
make acquire_signal_check
```


Applications based on the API require a specific FPGA image to be loaded:
```bash
overlay.sh v0.94
```

Execute the application. The path to Red Pitaya shared libraries must be provided explicitly. Some applications run in a continuous loop, press `CTRL+C` to stop them.
```bash
LD_LIBRARY_PATH=/opt/redpitaya/lib ./digital_led_blink
```
