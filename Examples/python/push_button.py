
import socket

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
        print(chunks)
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

rp_s = SCPI('192.168.178.36', 0.5)
rp_s.rp_write('DIG:PIN:DIR OUT,DIO5_N' + rp_s.delimiter)


i = 1000 #ms
while 1:
    rp_s.rp_write('DIG:PIN? DIO5_N' + rp_s.delimiter)
    if rp_s.rp_rcv(1)[0] == '1':
        rp_s.rp_write('DIG:PIN LED5,0' + rp_s.delimiter)
    else:
        rp_s.rp_write('DIG:PIN LED5,1' + rp_s.delimiter)