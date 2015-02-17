
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


#----------------------------------------------------- SCPI EXAMPLES PYTHON -----------------------------------------------------#


    #Example X+1
    #This example generates N pulses
    #This example acquires a whole signal buffer

    def acq_buff(self):
        buff = []
        buff_string = ''
        self.rp_write('ACQ:START' + self.delimiter)
        self.rp_write('ACQ:TRIG NOW' + self.delimiter)
        self.rp_write('ACQ:TRIG:STAT?' + self.delimiter)
        self.rp_rcv(2)
        self.rp_write('ACQ:SOUR1:DATA?' + self.delimiter)
        buff_string = self.rp_rcv(self.BUFF_SIZE)
        buff_string = buff_string.strip('{}\n\r').replace("  ", "").split(',')
        buff = map(float, buff_string)

        return 0


    def close(self):
        self.__del__()

    def __del__(self):
        if self._socket is not None:
            self._socket.close()
        self._socket = None


