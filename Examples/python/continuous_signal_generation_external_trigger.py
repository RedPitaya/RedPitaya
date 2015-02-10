
__author__ = 'infused'

import socket
import time

class SCPI(object):

    #Socket info
    HOST = None
    PORT = 5000
    TIME_OUT = None
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

rp_s.rp_write('SOUR1:FUNC ' + str(wave_form).upper() + rp_s.delimiter)
rp_s.rp_write('SOUR1:FREQ:FIX ' + str(freq) + rp_s.delimiter)
rp_s.rp_write('SOUR1:VOLT ' + str(ampl) + rp_s.delimiter)
rp_s.rp_write('SOUR1:TRIG:SOUR EXT' + rp_s.delimiter)

#Enable output
rp_s.rp_write('OUTPUT1:STATE ON' + rp_s.delimiter)


