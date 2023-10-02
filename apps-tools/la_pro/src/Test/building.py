import sys
import os
import subprocess as sp


rleModuleName = "rle"
decodercliModuleName = "decoder-cli"


def CheckExistRLE(path):
	if os.path.exists(path + '/' + rleModuleName) and os.path.isfile(path + '/' + rleModuleName):
		return True
	return False

def CheckExistDECODER(path):
	if os.path.exists(path + '/' + decodercliModuleName) and os.path.isfile(path + '/' + decodercliModuleName):
		return True
	return False

def BuildModule(path):
	self_dir = os.getcwd()
	os.chdir(path)
	sp.call("make clean", shell=True)
	building = sp.Popen("make", shell=True, stdout=sp.PIPE)
	build_log = building.stdout.read()
	building.communicate()
	build_returncode = building.returncode
	print(build_returncode)
	os.chdir(self_dir)
	if build_returncode == 0:
		return True, build_log
	return False, build_log

def CopyModuleRLE(path):
	result = os.system("cp " + path + "/" + rleModuleName + " ./" + rleModuleName)
	if result == 0:
		return True
	return False

def CopyModuleDECODER(path):
	result = os.system("cp " + path + "/" + decodercliModuleName + " ./" + decodercliModuleName)
	if result == 0:
		return True
	return False
