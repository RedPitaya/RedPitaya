#!/usr/bin/python3

from rp_updater import *
import numpy as np
import time

class Callback(CUpdaterCallback):
    def downloadProgress(self,fileName,now,total,stop):
        print("Download ", fileName, " progress = ", round(now / total * 100), end='\r')

    def downloadDone(self,fileName,succes):
        print("")
        print("Download done ",fileName, " state ",succes)
        if (succes):
            res = rp_UpdaterGetMD5('/home/redpitaya/ecosystems/'+fileName)
            print("MD5 ",fileName, " = ",res[1])

    def unzipProgress(self,i, total ,fileName):
        print("Unzip ",i, " / ",total, fileName)

    def installProgress(self,i, total ,fileName):
        print("Install ",i, " / ",total, fileName)


print("rp_UpdaterDownloadFile('https://downloads.redpitaya.com/downloads/Unify/nightly_builds/ecosystem-2.06-488-a5718bfe42!.zip')")
res = rp_UpdaterDownloadFile('https://downloads.redpitaya.com/downloads/Unify/nightly_builds/ecosystem-2.06-488-a5718bfe42!.zip')
print(res)

callback = Callback()
print("rp_UpdaterSetCallback(callback.__disown__())")
res = rp_UpdaterSetCallback(callback.__disown__())
print(res)

print("rp_UpdaterDownloadFileAsync('https://downloads.redpitaya.com/downloads/Unify/nightly_builds/ecosystem-2.06-487-f4bff67e5!.zip')")
res = rp_UpdaterDownloadFileAsync('https://downloads.redpitaya.com/downloads/Unify/nightly_builds/ecosystem-2.06-487-f4bff67e5!.zip')
print(res)

print("rp_UpdaterStopDownloadFile()")
res = rp_UpdaterStopDownloadFile()
print(res)

print("rp_UpdaterDownloadFileAsync('https://downloads.redpitaya.com/downloads/Unify/nightly_builds/ecosystem-2.06-487-f4bff67e5!.zip')")
res = rp_UpdaterDownloadFileAsync('https://downloads.redpitaya.com/downloads/Unify/nightly_builds/ecosystem-2.06-487-f4bff67e5!.zip')
print(res)


print("rp_UpdaterInit()")
res = rp_UpdaterInit()
print(res)

print("rp_UpdaterGetFreeSapce('/home/redpitaya/ecosystems')")
res = rp_UpdaterGetFreeSapce('/home/redpitaya/ecosystems')
print(res)

print("rp_UpdaterGetDownloadedFiles()")
res = rp_UpdaterGetDownloadedFiles()
print(res)

print("rp_UpdaterGetDownloadedCount()")
res = rp_UpdaterGetDownloadedCount()
print(res)

print("rp_UpdaterGetDownloadedFile(0)")
res = rp_UpdaterGetDownloadedFile(0)
print(res)

print("rp_UpdaterGetDownloadedFilesList()")
res = rp_UpdaterGetDownloadedFilesList()
print(res)

print("rp_UpdaterWaitDownloadFile()")
res = rp_UpdaterWaitDownloadFile()
print(res)

print("rp_UpdaterIsValidDownloadedFile('ecosystem-2.06-490-dcd31a4e9.zip')")
res = rp_UpdaterIsValidDownloadedFile('ecosystem-2.06-490-dcd31a4e9.zip')
print(res)

print("rp_UpdaterIsValidDownloadedFile('ecosystem-2.06-490-dcd31a4e9!.zip')")
res = rp_UpdaterIsValidDownloadedFile('ecosystem-2.06-490-dcd31a4e9!.zip')
print(res)

print("rp_UpdaterDownloadNBFile(500)")
res = rp_UpdaterDownloadNBFile(500)
print(res)

print("rp_UpdaterDownloadNBFileAsync(500)")
res = rp_UpdaterDownloadNBFileAsync(500)
print(res)

print("rp_UpdaterGetNBAvailableFilesList()")
res = rp_UpdaterGetNBAvailableFilesList()
print(res)

print("rp_UpdaterUpdateBoardEcosystem('ecosystem-2.06-490-dcd31a4e9.zip')")
res = rp_UpdaterUpdateBoardEcosystem('ecosystem-2.06-490-dcd31a4e9.zip')
print(res)

print("rp_UpdaterRelease()")
res = rp_UpdaterRelease()
print(res)