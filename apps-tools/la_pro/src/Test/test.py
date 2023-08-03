import os
import sys
from building import *
import clog


## Constants
signal_tmp_dir = "signal_tmp"
rlePath = "../rle"
decoderPath = "../decoder-cli"
logicTmpFile = "logic_data_file"
rleTmpFile = "rle_logic_data_file"
sigrokOutputFile = "sigrok-cli_output"
decoderOutputFile = "decoder-cli_output"
logger = clog.Clog()
notScriptsFilesList = [rleModuleName, decodercliModuleName, 'log.txt', logicTmpFile, rleTmpFile, sigrokOutputFile, decoderOutputFile, 
'result_sigrok', 'result_decoder', 'out', "diff", '*.pyc']


## Functions
def CleanThisDir():
	for item in notScriptsFilesList:
		os.system("rm -f %s" % item)


def CleanSignalTmpDir():
	os.system("rm -f %s" % "./" + signal_tmp_dir + "/*")


def DeleteSignalTmpDir():
	os.system("rmdir %s" % signal_tmp_dir)


def PrintHelp():
	print("Usage:")
	print("python test.py clean")
	print("python test.py *.sr (uart baudrate samplerate bitorder databits parity)|")
	print("                    (spi cs clk data)|i2c")


def DecodeByDecoderCli(input_file, prot):
	if prot == "uart":
		os.system("./%s -vi uart -r %s -b %s -S %s -o %s -D %s -p %s -n %s -K %s %s %s > %s" % 
			(decodercliModuleName, sys.argv[9], sys.argv[3], sys.argv[4], sys.argv[5], sys.argv[6], sys.argv[7], sys.argv[8], sys.argv[10], input_file, "out", decoderOutputFile))
	elif prot == "i2c":
		os.system("./%s -vi i2c --scl %s --sda %s %s %s > %s" % (decodercliModuleName, sys.argv[3], sys.argv[4], input_file, "out", decoderOutputFile))
	elif prot == "spi":
		os.system("./%s -vi spi --cs %s --clk %s --data %s --cspol %s --cpol %s --cpha %s -o %s %s %s > %s" % 
			(decodercliModuleName, sys.argv[3], sys.argv[4], sys.argv[5], sys.argv[6], sys.argv[7], sys.argv[8], sys.argv[9], input_file, "out", decoderOutputFile))

def DecodeBySigrokCli(input_file, prot):
	if prot == "uart":
		os.system("sigrok-cli -i %s -P uart:baudrate=%s:parity_type=%s:num_data_bits=%s:format=hex:invert_rx=%s -A uart > %s" % (input_file, sys.argv[3], 
			sys.argv[7], sys.argv[6], sys.argv[11], sigrokOutputFile))
	elif prot == "i2c":
		os.system("sigrok-cli -i %s -P i2c -A i2c > %s" % (input_file, sigrokOutputFile))
	elif prot == "spi":
		os.system("sigrok-cli -i %s -P spi:cs=%s:clk=%s:mosi=%s:cs_polarity=%s:cpol=%s:cpha=%s:bitorder=%s -A spi > %s" % 
			(input_file, sys.argv[10], sys.argv[11], sys.argv[12], sys.argv[13], sys.argv[14], sys.argv[15], sys.argv[16], sigrokOutputFile))

def GetAccordingDump(input_file):
	os.system("cp ./dumps/%s ./%s" % (os.path.split(input_file)[1]+".dump", sigrokOutputFile))

def SaveAccordingDump(input_file):
	os.system("rm -f ./dumps/%s" % (os.path.split(input_file)[1]+".dump"))
	os.system("cp ./%s ./dumps/%s" % (sigrokOutputFile, os.path.split(input_file)[1]+".dump"))


## Script body
if len(sys.argv) == 1:
	PrintHelp()
	exit(0)

if len(sys.argv) > 1 and sys.argv[1] == "clean":
	CleanThisDir()
	CleanSignalTmpDir()
	DeleteSignalTmpDir()
	exit(0)

# Checking for existing rle and decoder-cli tools in this directory.
# Build it if not
rleExist = CheckExistRLE("./")
decoderExist = CheckExistDECODER("./")

if not rleExist:
	logger.log_file("Main", "%s tool is not exists, try to build from sources..." % rleModuleName)
	result, log = BuildModule(rlePath)
	if result:
		logger.log_file("Main", "%s tool was successfully built, try to copy in this dir" % rleModuleName)
		CopyModuleRLE(rlePath)
		logger.log_file("Main", "%s tool was successfully copied" % rleModuleName)
	else:
		logger.log_file("Main", "%s tool was not built:\n %s" % (rleModuleName, log))
		print("Error! See the log.txt file for more details.")
		exit(-1)

if not decoderExist:
	logger.log_file("Main", "%s tool is not exists, try to build from sources..." % decodercliModuleName)
	result, log = BuildModule(decoderPath)
	if result:
		logger.log_file("Main", "%s tool was successfully built, try to copy in this dir" % decodercliModuleName)
		CopyModuleDECODER(decoderPath)
		logger.log_file("Main", "%s tool was successfully copied" % decodercliModuleName)
	else:
		logger.log_file("Main", "%s tool was not built:\n %s" % (decodercliModuleName, log))
		print("Error! See the log.txt file for more details.")
		exit(-1)

# Check that tmp directories is exists and create it if not
if not os.path.exists(signal_tmp_dir):
	os.mkdir(signal_tmp_dir)

logicListFile = []

# Check that first argument is the *.sr file
if len(sys.argv) > 1 and sys.argv[1].split('.')[1] != 'sr':
	print("First argument is not *.sr file!")
	exit(-2)
elif len(sys.argv) > 1 and not os.path.exists(sys.argv[1]):
	print("%s in this directory is not exists!" % sys.argv[1])
	exit(-3)
elif len(sys.argv) > 1:
	# Unzip *.sr file
	os.system("unzip %s -d %s" % (sys.argv[1], './' + signal_tmp_dir))

	# Get list of files in signal_tmp directory
	fileListSignalTmpDir = os.listdir("./" + signal_tmp_dir)

	# Get list of 'logic' files
	for item in fileListSignalTmpDir:
		if item[:len('logic')] == "logic":
			logicListFile.append(item)

	# Merge 'logic' to one big file
	with open(logicTmpFile, "wb") as logicFile:
		for item in logicListFile:
			with open("./%s/%s" % (signal_tmp_dir, item), "rb") as dataFile:
				logicFile.write(dataFile.read())

	# Encode logic file to RLE format
	rleResult = os.system("./rle s2r %s %s" % (logicTmpFile, rleTmpFile))
	if rleResult != 0:
		logger.log_file("Main", "Converting to RLE was failed.")
		print("Error! Converting to RLE failed!")
		exit(-4)

	# Decode protocol by sigrok-cli and decoder-cli
	if len(sys.argv) > 2:
		if sys.argv[2] == "uart":
			if len(sys.argv) < 8:
				logger.log_file("Main", "UART args error")
				print("Error! Args error for UART!")
			else:
				DecodeByDecoderCli(rleTmpFile, "uart")
				#DecodeBySigrokCli(sys.argv[1], "uart")
				GetAccordingDump(sys.argv[1])
		elif sys.argv[2] == "i2c":
			DecodeByDecoderCli(rleTmpFile, "i2c")
			#DecodeBySigrokCli(sys.argv[1], "i2c")
			GetAccordingDump(sys.argv[1])
		elif sys.argv[2] == "spi":
			if len(sys.argv) < 6:
				logger.log_file("Main", "SPI args error")
				print("Error! Args error for SPI!")
			else:
				DecodeByDecoderCli(rleTmpFile, "spi")
				#DecodeBySigrokCli(sys.argv[1], "spi")
				#SaveAccordingDump(sys.argv[1])
				GetAccordingDump(sys.argv[1])

# After working clean signal_tmp directory
CleanSignalTmpDir()
print("End. Signal tmp directory was cleaned.")
