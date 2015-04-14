"""SCPI access to Red Pitaya."""

import socket

__author__ = "Luka Golinar, Iztok Jeras"
__copyright__ = "Copyright 2015, Red Pitaya"

class scpi (object):
    """SCPI class used to access Red Pitaya over an IP network."""

    # SCPI delimiter
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

    def recv_txt(self, chunksize = 4096):
        """Receive text string and return it after removing the delimiter."""
        msg = ''
        while 1:
            chunk = self._socket.recv(chunksize + len(self.delimiter)) # Receive chunk size of 2^n preferably
            msg += chunk
            if (len(chunk) and chunk[-2:] == self.delimiter):
                break
        return msg[:-2]

    def recv_raw(self):
        """Receive RAW binary data, parse the header, and return only the payload."""
        # receive message header
        msg = self._socket.recv(6)
        # parse header to get the length of binary data (number of bytes)
        num = int(msg)
        # receive binary data
        msg = self._socket.recv(num)
        # return binary data
        return msg

    def send_txt(self, msg):
        """Send text string ending and append delimiter."""
        return self._socket.send(msg + self.delimiter)

    def send_raw(self, msg):
        """Send RAW binary data."""
        return self._socket.send(msg)

    def close(self):
        """Close IP connection."""
        self.__del__()

    def __del__(self):
        if self._socket is not None:
            self._socket.close()
        self._socket = None



    def gen(self, src, waveform, freq, ampl):
        """Generate signal."""
        self.send_txt('SOUR'+str(src)+':FUNC '      + str(waveform).upper())
        self.send_txt('SOUR'+str(src)+':FREQ:FIX '  + str(freq)            )
        self.send_txt('SOUR'+str(src)+':VOLT '      + str(ampl)            )
        self.send_txt('OUTPUT'+str(src)+':STATE ON'                        )
