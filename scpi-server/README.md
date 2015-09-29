# SCPI SERVER #

## Contents ##

| paths                         | contents
|-------------------------------|---------
| `scpi-server/src/`            |
| `scpi-server/scpi-parser`     |
| `scpi-server/Makefile`        |


## How to build Red Pitaya scpi-server ##
Before proceeding follow the [instructions](http://wiki.redpitaya.com/index.php?title=Red_Pitaya_OS) on how to set up working evironment.
Then proceed by simply running the following command.
```bash
make clean all
``` 

## Starting Red Pitaya Scpi server ##

Before starting scpi server, make sure nginx is not running. Both of these services use the same hardware configuration, thus
resulting in one disturbing the functioniong of the other.
```bash
systemctl stop nginx
```
Now we can try and start red pitaya scpi server
```bash
systemctl start redpitaya_scpi
```
