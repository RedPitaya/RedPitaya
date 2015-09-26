Red Pitaya SDK
==============

## Content ##

| File                  | Description
|-----------------------|-----------------------------------------
| `install.sh`          | Red Pitaya sdk install script
| `create_template.sh`  | Simple C program template creator script
| `run.sh`              | Run and execute script
| `Makefile`            | Red Pitaya compile Makefile


## Instalation procedure  ##

First rune the install script using the command below:
```bash
sudo ./install.sh
```

This will install a few programs including the linaro hf compiler needed to compile Red Pitaya
compatible programs. It will also set all the needed environmental variables.

### Installed programs ###

- Nano
- Curl
- Putty-tools
- Linaro compiler

### Modified variables ###

- PATH
- CROSS_COMPILE

## Template creation and sample program run ##

First we need to have a .c program ready to be run on a Red Pitaya. You can make one yourself, or you can
use the template creator to create a sample template that can be then modified to suit your needs. In this example
we will create a .c template called test.c. We run the create_template.sh cript and give it a name as the first
argument.

```bash
./create_template.sh test.c
```

This will create a file called test.c. Now let's run the run.sh script with the test.c file just created.

```bash
./run 192.168.1.100 test.c
```

NOTE: You need to replace the IP address given with the IP address of your redpitaya.

The program will take the IP address and the given file. It will compile our test.c to an executable, send it to a Red
Pitaya with the given IP and execute it. You will see a sample output in your console. You can also see
the output given in /var/log/sdk_log/debug.
