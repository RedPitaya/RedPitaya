
__author__ = 'infused'

import socket

class scpi (object):

    # Socket info
    # SCPI delimiter
    delimiter = '\r\n'

    def __init__(self, host, timeout=None, port=5000):
        self.host    = host
        self.port    = port
        self.timeout = timeout

        try:
            self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

            if timeout is not None:
                self._socket.settimeout(timeout)

            self._socket.connect((host, port))

        except socket.error as e:
            print 'SCPI >> connect({:s}:{:d}) failed: {:s}'.format(host, port, e)

    # Receive function
    def recv(self, chunksize = 4096):
        msg = ''
        while 1:
            chunk = (self._socket.recv(chunksize)) # Receive chunk size of 2^n preferably
            msg += chunk
            if(len(chunk) and chunk[-1] == '\n'):
                break

        return msg

    def send(self, msg):
        return self._socket.send(msg)


    def close(self):
        self.__del__()

    def __del__(self):
        if self._socket is not None:
            self._socket.close()
        self._socket = None
