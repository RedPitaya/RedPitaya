# Compiling and running on host

When compiling on a PC host, a cross compiler must be used. Please read [the instructions](../../README.md) for installing the required tools and setting up the environment variables.

To compile the code just use the source file name without the `.c` extension.
```bash
cd Examples/C
make digital_led_blink
```

This will create an executable, which should be copied to your board using `scp` (replace the IP).
```bash
scp digital_led_blink root@192.168.0.100
```

Connect to your board over SSH (replace the IP).
```bash
ssh root@192.168.0.100
```

Now on target execute the application. The path to Red Pitaya shared libraries must be provided explicitly. Some applications run in a continuous loop, press `CTRL+C` to stop them.
```bash
LD_LIBRARY_PATH=/opt/redpitaya/lib ./digital_led_blink
```

# Compiling and running on target

When compiling on the target no special preparations are needed. A native toolchain is available directly on the Debian system.

First connect to your board over SSH (replace the IP).
```bash
ssh root@192.168.0.100
```

Now on the target, make a clone of the Red Pitaya Git repository and enter the project directory.
```bash
git clone https://github.com/RedPitaya/RedPitaya.git
cd RedPitaya
```

To compile the code just use the source file name without the `.c` extension.
```bash
cd Examples/C
make digital_led_blink
```

Execute the application. The path to Red Pitaya shared libraries must be provided explicitly. Some applications run in a continuous loop, press `CTRL+C` to stop them.
```bash
LD_LIBRARY_PATH=/opt/redpitaya/lib ./digital_led_blink
```
