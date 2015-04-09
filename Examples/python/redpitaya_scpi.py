
__author__ = 'infused'

import socket
import time

class scpi(object):

    # Socket info
    BUFF_SIZE = 16384 * 32
    # SCPI delimiter
    delimiter = '\r\n'

    def __init__(self, host, timeout=None, port=5000):
        self.host    = host
        self.port    = port
        self.timeout = time_out

        try:
            self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

            if timeout is not None:
                self._socket.settimeout(timeout)

            self._socket.connect((host, port))

        except socket.error as e:
            print 'SCPI >> connect({:s}:{:d}) failed: {:s}'.format(host, port, e)

    # Receive function
    def recv(self, buff_size):
        chunks = []
        bytes_read = 0
        while 1:
            chunk = (self._socket.recv(buff_size)) # Receive chunk size of 2^n preferably
            chunks.append(chunk)
            bytes_read += len(chunk)
            if(len(chunk) and chunk[-1] == '\n'):
                break
        return b''.join(chunks)

    def send(self, message):
        return self._socket.send(message)

    # RP help functions

    def choose_state(self, led, state):
        return 'DIG:PIN LED' + str(led) + ', ' + str(state) + self.delimiter
