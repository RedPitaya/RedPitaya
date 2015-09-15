"""SCPI access to Red Pitaya."""

import socket

__author__ = "Luka Golinar, Iztok Jeras"
__copyright__ = "Copyright 2015, Red Pitaya"

class scpi (object):
    """SCPI class used to access Red Pitaya over an IP network."""
    delimiter = '\r\n'

    def __init__(self, host, timeout=None, port=5000):
        """Initialize object and open IP connection.
        Host IP should be a string in parentheses, like '192.168.1.100'.
        """
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

    def rx_txt(self, chunksize = 4096):
        """Receive text string and return it after removing the delimiter."""
        msg = ''
        while 1:
            chunk = self._socket.recv(chunksize + len(self.delimiter)) # Receive chunk size of 2^n preferably
            msg += chunk
            if (len(chunk) and chunk[-2:] == self.delimiter):
                break
        return msg[:-2]

    def tx_txt(self, msg):
        """Send text string ending and append delimiter."""
        return self._socket.send(msg + self.delimiter)

    #RP help functions
    def choose_state(self, led, state):
        return 'DIG:PIN LED' + str(led) + ', ' + str(state) + self.delimiter

    def close(self):
        """Close IP connection."""
        self.__del__()

    def __del__(self):
        if self._socket is not None:
            self._socket.close()
        self._socket = None

