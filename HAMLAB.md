Applications (userspace drivers) that were developed for EXTM HAMLAB v1.0 hardware are;

new applications:

gpiorelay for sdr outputs/ instruments outputs, ext-tr / logic
hamrf - for instruments acdc, hvlv and sdr in

old applications:

hvlv 
	this application is able to switch between 
	SDR mode with "hvlv -s" and 
	INSTRUMENTS mode with "hvlv -i"
	and additionaly on instuments mode we are able to switch between 
	LV and HV jumpers on redpitaya with 
	"hvlv -i -1 X -2 Y" where X and Y are 0 for LV or 1 for HV 
	-1 and -2 are channel designatos and can be used together or only one 
	at a time. 

acdc
	this application is able to switch between 
        SDR mode with acdc -s and 
        INSTRUMENTS mode with acdc -i
        and additionaly on instuments mode we are able to switch between 
        AC and DC coupling inside osciloscope app
	Additional api functions were added as well...
	acdc settings must be used with -i argument and are set with 
	acdc -i -1 X -2 Y where X and Y are 0 for AC and 1 for DC
	-1 and -2 specify channel and can be used together or only one at a time.  
	
	
laosc	(in next hw revision this application becomes irelavent since 
	we must use kernel api for accesing gpios)

	this application can switch relay between osciloscope external 
	trigger and logic analizer input 1 line... 
	-l set device in logic analizer mode and 
	-o set device in osciloscope external trigger mode
	additional functions are to set leds and gpios:
	-4 for led4 
	-5 for led5 
	-6 for led6 
	-1 for gpio1 
	-2 for gpio2 
	-0 for setting all pins on pca9555 device pins to 0  
	
After boot hamlab.sh script must be run for above applications to work.
This can be done automaticly with systemd, by copiing 
OS/debian/overlay/etc/systemd/system/hamlab.service into /etc/systemd/system/hamlab.service 
and running "systemctl enable hamlab.service"



# debugging I2C devices

## Extension Board

```
$ # PCA9547 (I2C mux)
$ i2cdump -y -f              0 0x70
$ # PCA9557 (ID)
$ i2cdump -y -f -r 0x00-0x03 0 0x1f
$ # PCA9535 (GPIO expander)
$ i2cdump -y -f -r 0x00-0x07 8 0x20
```

## RF Board

```
$ # PCA9557 (ID)
$ i2cdump -y -f -r 0x00-0x03 7 0x1a
$ # PCA9535 (GPIO expander)
$ i2cdump -y -f -r 0x00-0x07 7 0x20
$ # 24LC64FT
$ i2cdump -y -f              7 0x51
```

## SDR Board

```
$ # PCA9557 (ID)
$ i2cdump -y -f -r 0x00-0x03 1 0x18
$ # PCA9555 (GPIO expander)
$ i2cdump -y -f -r 0x00-0x07 1 0x20
```

