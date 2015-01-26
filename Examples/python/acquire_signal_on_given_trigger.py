
__author__ = 'infused'

import socket
import time

class SCPI(object):

    #Socket info
    HOST = None
    PORT = 5000
    TIME_OUT = None
    BUFF_SIZE = 16384 * 32
    #SCPI delimiter
    delimiter = '\r\n'

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

    #Receive function
    def rp_rcv(self, buff_size):
        chunks = []
        bytes_read = 0
        while 1:
            chunk = (self._socket.recv(buff_size)) #Receive chunk size of 2^n preferably
            chunks.append(chunk)
            bytes_read += len(chunk)
            if(len(chunk) and chunk[-1] == '\n'):
                break
        return b''.join(chunks)

    def rp_write(self, message):
        return self._socket.send(message)

    #RP help functions

    def choose_state(self, led, state):
        return 'DIG:PIN LED' + str(led) + ', ' + str(state) + self.delimiter

    def close(self):
        self.__del__()

    def __del__(self):
        if self._socket is not None:
            self._socket.close()
        self._socket = None


rp_s = SCPI('IP', None)
wave_form = 'sine'
freq = 1000
ampl = 1

buff = []
buff_string = ''
rp_s.rp_write('ACQ:DEC 8' + rp_s.delimiter)
rp_s.rp_write('ACQ:TRIG:LEV 100' + rp_s.delimiter)
rp_s.rp_write('ACQ:START' + rp_s.delimiter)
rp_s.rp_write('ACQ:TRIG CH1_PE' + rp_s.delimiter)

while 1:
    rp_s.rp_write('ACQ:TRIG:STAT?' + rp_s.delimiter)
    if rp_s.rp_rcv(2) == 'TD':
        break

rp_s.rp_write('ACQ:SOUR1:DATA?' + rp_s.delimiter)
buff_string = rp_s.rp_rcv(rp_s.BUFF_SIZE)
buff_string = buff_string.strip('{}\n\r').replace("  ", "").split(',')
buff = map(float, buff_string)

