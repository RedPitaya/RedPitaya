# Compiling and running on host PC

When compiling on a PC host, a cross compiler must be used. Please read [the instructions](../../README.md) for installing the required tools and setting up the environment variables. During make, you should see the cross compiler `arm-linux-gnueabihf-gcc` being used, otherwise some steps from instructions have been missed.

Make a clone of the Red Pitaya Git repository, enter the project directory, and setup the environment.
```bash
git clone https://github.com/RedPitaya/RedPitaya.git
cd RedPitaya
. settings.sh
```

Compile the API.
```bash
make api
```

To compile one example just use the source file name without the `.c` extension.
```bash
cd Examples/C
make digital_led_blink
```
To compile all examples and to clean run one of the following.
```bash
make all
make clean
```

This will create an executable, which should be copied to your board using `scp` (replace the IP, the default password is `root`).
```bash
scp digital_led_blink root@192.168.0.100
```

Connect to your board over SSH (replace the IP, the default password is `root`).
```bash
ssh root@192.168.0.100
```

Now on the target execute the application. The path to Red Pitaya shared libraries must be provided explicitly. Some applications run in a continuous loop, press `CTRL+C` to stop them.
```bash
LD_LIBRARY_PATH=/opt/redpitaya/lib ./digital_led_blink
```

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

Compile the API.
```bash
make api
```

To compile one example just use the source file name without the `.c` extension.
```bash
cd Examples/C
make digital_led_blink
```

Execute the application. The path to Red Pitaya shared libraries must be provided explicitly. Some applications run in a continuous loop, press `CTRL+C` to stop them.
```bash
LD_LIBRARY_PATH=/opt/redpitaya/lib ./digital_led_blink
```
