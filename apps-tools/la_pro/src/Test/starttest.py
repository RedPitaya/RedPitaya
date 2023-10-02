import os
import sys
import dumplist
import time


## Constants
sigrokOutputFile = "sigrok-cli_output"
decoderOutputFile = "decoder-cli_output"


## Clean directory, if need
if len(sys.argv) > 1:
	if sys.argv[1] == "clean":
		os.system("python test.py clean")
		exit(0)

## Excecute for all files
time_list = []

for item in dumplist.uart_file:
	start_time = time.time()
	os.system("python test.py %s uart %s" % (item['file'], item['params']))
	stop_time = time.time()
	time_list.append({item['file']: stop_time - start_time})
	# Diff
	os.system("cat %s | grep '[0-9A-F]' | sed -e '/\(Frame\|Parity\|Start\|Stop\)/d' | sed -n '/^.\{2\}/p' | sed '1d' > %s" % (sigrokOutputFile, "result_sigrok"))
	os.system("cat %s | sed -e '1,5d' > %s" % (decoderOutputFile, "result_decoder"))
	result = os.system("diff %s %s > diff" % ("result_sigrok", "result_decoder"))
	if result != 0:
		print("UART decoding error!")
		exit(-1)

for item in dumplist.spi_file:
	start_time = time.time()
	os.system("python test.py %s spi %s" % (item['file'], item['params']))
	stop_time = time.time()
	time_list.append({item['file']: stop_time - start_time})
	# Diff
	os.system("cat %s | grep '[0-9A-F]' | sed -n '/^.\{2\}/p' > %s" % (sigrokOutputFile, "result_sigrok"))
	os.system("cat %s | sed -e '1,5d' > %s" % (decoderOutputFile, "result_decoder"))
	result = os.system("diff %s %s > diff" % ("result_sigrok", "result_decoder"))
	if result != 0:
		print("SPI decoding error!")
		exit(-1)

for item in dumplist.i2c_file:
	start_time = time.time()
	os.system("python test.py %s i2c %s" % (item['file'], item['params']))
	stop_time = time.time()
	time_list.append({item['file']: stop_time - start_time})
	# Diff
	os.system("cat %s | grep '[A-Za-z]' | sed -e '/\(Read\|Write\)/d' > %s" % (sigrokOutputFile, "result_sigrok"))
	os.system("cat %s | sed -e '1,6d' > %s" % (decoderOutputFile, "result_decoder"))
	result = os.system("diff %s %s > diff" % ("result_sigrok", "result_decoder"))
	if result != 0:
		print("I2C decoding error!")
		exit(-1)

if len(sys.argv) > 1:
	if sys.argv[1] == "-v":
		for item in time_list:
			print(item)

# Exit with OK code
exit(0)
