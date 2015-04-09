
__author__ = 'infused'

import socket
import time

class scpi(object):

    # Socket info
    # SCPI delimiter
    delimiter = '\r\n'
    chunksize = 4096

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
    def recv(self, chunksize = self.chunksize):
        msg = ''
        bytes_read = 0
        while 1:
            chunk = (self._socket.recv(chunksize)) # Receive chunk size of 2^n preferably
            msg.append(chunk)
            bytes_read += len(chunk)
            if(len(chunk) and chunk[-1] == '\n'):
                break

        return msg

    def send(self, message):
        return self._socket.send(message)


    def close(self):
        self.__del__()

    def __del__(self):
        if self._socket is not None:
            self._socket.close()
        self._socket = None
