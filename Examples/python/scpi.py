
__author__ = 'infused'

import socket
import time

class SCPI(object):

    #Socket info
    HOST = None
    PORT = 5000
    TIME_OUT = None
    BUFF_SIZE = 16384 * 32 #TODO Fix size

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


    #TODO: Make it write data chunk by chunk
    def rp_write(self, message):
        return self._socket.send(message)

    #RP help functions

    def choose_state(self, led, state):
        return 'DIG:PIN LED' + str(led) + ', ' + str(state) + self.delimiter


#----------------------------------------------------- SCPI EXAMPLES PYTHON -----------------------------------------------------#

    #Example 1
    #This functions blinks diode with a period of time_out(seconds)
    def blink_diode(self, diode, time_out):
        while 1:
            time.sleep(time_out)
            self.rp_write(self.choose_state(diode, 1))
            time.sleep(time_out)
            self.rp_write(self.choose_state(diode, 0))
        return 0

    #Example 2
    #This example turns on leds from 1 to 7 depending on the given test_parameter

    def bar_led_graphs(self, test_parameter):
        for i in range(1, 7):
            if(test_parameter >= ((100/7) * i)):
                self.rp_write(self.choose_state(str(i), 1))
            else:
                self.rp_write(self.choose_state(str(i), 0))
        return 0
    #Example X
    #This example generates a simple signal

    def continuous_sig_gen(self, wave_form, freq, ampl):
        self.rp_write('SOUR1:FUNC ' + str(wave_form).upper() + self.delimiter)
        self.rp_write('SOUR1:FREQ:FIX ' + str(freq) + self.delimiter)
        self.rp_write('SOUR1:VOLT ' + str(ampl) + self.delimiter)

        #Enable output
        self.rp_write('OUTPUT1:STATE ON' + self.delimiter)

    #Example X+1
    #This example generates N pulses
    def cont_sig_gen_pulse(self, wave_form, freq, ampl, pulses):
        for i in range(0, 1000):
            self.rp_write('SOUR1:FUNC ' + str(wave_form).upper() + self.delimiter)
            self.rp_write('SOUR1:FREQ:FIX ' + str(freq) + self.delimiter)
            self.rp_write('SOUR1:VOLT ' + str(ampl) + self.delimiter)

            self.rp_write('SOUR1:BURS:NCYC 1' + self.delimiter)
            self.rp_write('SOUR1:TRIG:IMM' + self.delimiter)

            #Enable output
            self.rp_write('OUTPUT1:STATE ON' + self.delimiter)
            self.rp_write('SOUR1:TRIG:IMM' + self.delimiter)

            #Sleep for N time before generating another pulse
            time.sleep(10)

    #Example X + Y
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


