# SCPI SERVER

## Contents

| paths                         | contents
|-------------------------------|---------
| `scpi-server/src/`            |
| `scpi-server/scpi-parser`     |
| `scpi-server/Makefile`        |


## How to build Red Pitaya `scpi-server`
Before proceeding follow the [instructions](https://github.com/RedPitaya/RedPitaya/blob/master/doc/developer.rst) on how to set up working environment.
Then proceed by simply running the following command.
```bash
make clean all
``` 

## Starting Red Pitaya SCPI server

Before starting SCPI service, make sure Nginx service is not running.
Running them at the same time will cause conflicts, since they access the same hardware.
```bash
systemctl stop redpitaya_nginx
```
Now we can try and start Red Pitaya SCPI server.
```bash
systemctl start redpitaya_scpi
```

## Starting Red Pitaya SCPI server at boot time

The next commands will enable running SCPI service at boot time and disable Nginx service.
```bash
systemctl disable redpitaya_nginx
systemctl enable  redpitaya_scpi
```
