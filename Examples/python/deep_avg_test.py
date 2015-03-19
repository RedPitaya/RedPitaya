import socket
import matplotlib.pyplot as plt

import math

class SCPI(object):

    #Socket info
    HOST = None
    PORT = 5000
    TIME_OUT = None
    #SCPI delimiter
    delimiter = '; \r\n\t'
    _chunk = 4096

    def __init__(self, host, time_out, port=PORT):
        self.HOST = host
        self.TIME_OUT = time_out

        try:
            self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

            if self.TIME_OUT is not None:
                self._socket.settimeout(self.TIME_OUT)

            self._socket.connect((self.HOST, self.PORT))

        except socket.error as e:
            print 'SCPI >> connect({:s}:{:d}) failed: {:s}'.format(host, port, e)

    def rp_rcv(self):
        chunks = ''
        bytes_read = 0
        while 1:
            chunk = (self._socket.recv(self._chunk)) #Receive chunk size of 2^n preferably
            chunks += chunk
            bytes_read += len(chunk)
            #print chunks
            if chunk[-1] == '\n':
                break

        return chunks

    def rp_write(self, message):
        return self._socket.send(message)


    def close(self):
        self.__del__()

    def __del__(self):
        if self._socket is not None:
            self._socket.close()
        self._socket = None

#Input your Pitaya's IP address and set a timeout. Example: rp_s = SCPI('192.168.1.100', 1)
rp_s = SCPI('192.168.178.56', 1)
time_out = 1

wave_form = 'sine'
freq = 100000
ampl = 1

shift = 2
count = 10000

#Generate signal
rp_s.rp_write('SOUR1:FUNC ' + str(wave_form).upper() + rp_s.delimiter)
rp_s.rp_write('SOUR1:FREQ:FIX ' + str(freq) + rp_s.delimiter)
rp_s.rp_write('SOUR1:VOLT ' + str(ampl) + rp_s.delimiter)
rp_s.rp_write('OUTPUT1:STATE ON' + rp_s.delimiter)

#Deep averaging
rp_s.rp_write('ACQ:DP:LEN 16383' + rp_s.delimiter)
rp_s.rp_write('ACQ:DP:COUNT ' + str(count) + rp_s.delimiter)
rp_s.rp_write('ACQ:DP:SHIFT ' + str(shift) + rp_s.delimiter)
rp_s.rp_write('ACQ:DP:DEBTIM 0' + rp_s.delimiter)
rp_s.rp_write('ACQ:TRIG CH1_NE' + rp_s.delimiter)
rp_s.rp_write('ACQ:TRIG:LEV 0' + rp_s.delimiter)

#Hysteresis rp_s.rp_write('')

#Applying software based offset. Not user defined.
rp_s.rp_write('ACQ:DP:OFFSET' + rp_s.delimiter)
rp_s.rp_write('ACQ:DP:START' + rp_s.delimiter)


run = 1
while run:
    rp_s.rp_write('ACQ:DP:RUN?' + rp_s.delimiter)
    run = int(rp_s.rp_rcv())
    #print run

rp_s.rp_write('ACQ:DP:SOUR1:DATA?' + rp_s.delimiter)
buff_string = rp_s.rp_rcv()

buff_string = buff_string.strip('{\n\r}').split(',')
buff = map(float, buff_string)

#Applying scaling to accumulated data.
for i in range(0, len(buff)):
    buff[i] = (buff[i] * (math.pow(2, shift) / count)) / 0x1fff

plt.plot(buff)
plt.ylabel('Deep acq test')
plt.show()

