# SCPI SERVER

## Contents

| paths                         | contents
|-------------------------------|---------
| `scpi-server/src/`            |
| `scpi-server/scpi-parser`     |
| `scpi-server/Makefile`        |


## How to build Red Pitaya `scpi-server`
Before proceeding follow the [instructions](http://wiki.redpitaya.com/index.php?title=Red_Pitaya_OS) on how to set up working environment.
Then proceed by simply running the following command.
```bash
make clean all
``` 

## Starting Red Pitaya SCPI server

Before starting SCPI service, make sure Nginx and Wyliodrin services is not running. Running them at the same time will cause conflicts, since they access the same hardware.
```bash
systemctl stop redpitaya_nginx
systemctl stop redpitaya_wyliodrin
```
Now we can try and start Red Pitaya SCPI server.
```bash
systemctl start redpitaya_scpi
```

## Starting Red Pitaya SCPI server at boot time

The next commands will enable running SCPI service at boot time and disable Nginx and Wyliodrin services.
```bash
systemctl disable redpitaya_nginx
systemctl disable redpitaya_wyliodrin
systemctl enable  redpitaya_scpi
```
