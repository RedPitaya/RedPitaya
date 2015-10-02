
__author__ = "Luka Golinar <luka.golinar@gmail.com>"

#Imports
import sys
import redpitaya_scpi as scpi
import unittest
import collections


#Scpi declaration
rp_scpi = scpi.scpi('192.168.178.115')

#Global variables
rp_dpin_p  = {i: 'DIO'+str(i)+'_P' for i in range(8)}
rp_dpin_n  = {i: 'DIO'+str(i)+'_N' for i in range(8)}
rp_a_pin_o = {i: 'AOUT'+str(i) for i in range(4)}
rp_a_pin_i = {i: 'AIN'+str(i) for i in range(4)}
rp_leds    = {i: 'LED'+str(i) for i in range(8)}

rp_freq_range  = [100, 1000, 10000, 100000, 1e+06, 1e+07, 3e+07]
rp_volt_range  = [0.25, 0.5, 0.75, 1.0]
rp_phase_range = [360, -180, -90, -30, 30, 90, 180, 360]
rp_offs_range  = [-0.75, -0.5, -0.25, 0.25, 0.5 , 0.75]
rp_dcyc_range  = [0.5, 1, 10, 50, 75, 100]
rp_ncyc_range  = [1, 10, 100, 1000, 10000, 50000, 'INF']
rp_nor_range   = rp_ncyc_range[:]
rp_inp_range   = [i * 100 for i in range(1, 6)]


rp_wave_forms  = ['SINE', 'SQUARE', 'TRIANGLE', 'PWM', 'SAWU', 'SAWD']

# Base functions
class Base(object):

    def rp_led(self, led, state):
        rp_scpi.tx_txt('DIG:PIN ' + led + ', ' + state)
        rp_scpi.tx_txt('DIG:PIN? ' + led)
        return rp_scpi.rx_txt()

    #TODO: Direction
    def rp_dpin_state(self, pin, state):
        rp_scpi.tx_txt('DIG:PIN ' + pin + ', ' + state)
        rp_scpi.tx_txt('DIG:PIN? ' + pin)
        return rp_scpi.rx_txt()

    def rp_analog_pin(self, pin, state, out):
        if(out is True):
            rp_scpi.tx_txt('ANALOG:PIN ' + pin + ', ' + state)
        rp_scpi.tx_txt('ANALOG:PIN? ' + pin)
        return rp_scpi.rx_txt()

    def rp_freq(self, channel, freq):
        rp_scpi.tx_txt('SOUR' + str(channel) + ':FREQ:FIX ' + str(freq))
        rp_scpi.tx_txt('SOUR' + str(channel) + ':FREQ:FIX?')
        return rp_scpi.rx_txt()

    def rp_ampl(self, channel, ampl):
        rp_scpi.tx_txt('SOUR' + str(channel) + ':VOLT ' + str(ampl))
        rp_scpi.tx_txt('SOUR' + str(channel) + ':VOLT?')
        return rp_scpi.rx_txt()

    def rp_w_form(self, channel, form):
        rp_scpi.tx_txt('SOUR' + str(channel) + ':FUNC ' + str(form))
        rp_scpi.tx_txt('SOUR' + str(channel) + ':FUNC?')
        return rp_scpi.rx_txt()


    def rp_offs(self, channel, offs):
        #AMPL + OFFS <= |1V|
        rp_scpi.tx_txt('SOUR' + str(channel) + ':VOLT 0.01')

        rp_scpi.tx_txt('SOUR' + str(channel) + ':VOLT:OFFS ' + str(offs))
        rp_scpi.tx_txt('SOUR' + str(channel) + ':VOLT:OFFS?')
        return rp_scpi.rx_txt()

    def rp_phase(self, channel, phase):
        rp_scpi.tx_txt('SOUR' + str(channel) + ':PHAS ' + str(phase))
        rp_scpi.tx_txt('SOUR' + str(channel) + ':PHAS?')
        return rp_scpi.rx_txt()

    def rp_dcyc(self, channel, dcyc):
        rp_scpi.tx_txt('SOUR' + str(channel) + ':DCYC ' + str(dcyc))
        rp_scpi.tx_txt('SOUR' + str(channel) + ':DCYC?')
        return rp_scpi.rx_txt()

    #TODO: Scpi server returns 1 for ON and 0 for off here. It should be ON and OFF. Fix that!
    def rp_burst_state(self, channel):
        rp_scpi.tx_txt('SOUR' + str(channel) + ':BURS:STAT ON')
        rp_scpi.tx_txt('SOUR' + str(channel) + ':BURS:STAT?')
        if(rp_scpi.rx_txt().strip('\n') is not '1'):
            return False
        rp_scpi.tx_txt('SOUR' + str(channel) + ':BURS:STAT OFF')
        rp_scpi.tx_txt('SOUR' + str(channel) + ':BURS:STAT?')
        if(rp_scpi.rx_txt().strip('\n') is not '0'):
            return False
        return True

    def rp_burst_ncyc(self, channel, ncyc):
        rp_scpi.tx_txt('SOUR' + str(channel) + ':BURS:NCYC ' + str(ncyc))
        rp_scpi.tx_txt('SOUR' + str(channel) + ':BURS:NCYC?')
        return rp_scpi.rx_txt()

    def rp_burst_nor(self, channel, nor):
        rp_scpi.tx_txt('SOUR' + str(channel) + ':BURS:NCYC ' + str(nor))
        rp_scpi.tx_txt('SOUR' + str(channel) + ':BURS:NCYC?')
        return rp_scpi.rx_txt()

    def rp_burst_intp(self, channel, intp):
        rp_scpi.tx_txt('SOUR' + str(channel) + ':BURS:INT:PER ' + str(intp))
        rp_scpi.tx_txt('SOUR' + str(channel) + ':BURS:INT:PER?')
        return rp_scpi.rx_txt()

    def generate_wform(self, channel):

        buff = []
        buff_ctrl = []

        #Do not change these values!
        freq = 7629.39453125
        ampl = 0.8
        wave_form = 'SINE'


        rp_scpi.tx_txt('ACQ:START')


        #Enable Red Pitaya digital loop
        rp_scpi.tx_txt('OSC:RUN:DIGLOOP')

        #Set generator options
        rp_scpi.tx_txt('SOUR' + str(channel) + ':FREQ:FIX ' + str(freq))
        rp_scpi.tx_txt('SOUR' + str(channel) + ':VOLT ' + str(ampl))
        rp_scpi.tx_txt('SOUR' + str(channel) + ':FUNC ' + str(wave_form))

        rp_scpi.tx_txt('ACQ:TRIG CH1_PE')
        rp_scpi.tx_txt('OUTPUT' + str(channel) + ':STATE ON')

        #rp_scpi.tx_txt('ACQ:TRIG:STAT?')
        #rp_scpi.rx_txt()
        rp_scpi.tx_txt('ACQ:SOUR' + str(channel) + ':DATA?')

        buff_string = rp_scpi.rx_txt()
        buff_string = buff_string.strip('{}\n\r').replace("  ", "").split(',')
        buff = map(float, buff_string)

        buff_ctrl = open('./ctrl_data/gen_ctrl', 'r').readlines()

        for i in range(len(buff_ctrl)):
            buff_ctrl[i] = (float)(buff_ctrl[i].strip('\n'))
            buff[i] = (float)(buff[i])
        rp_scpi.tx_txt('ACQ:RST')

        #Compare the two buffers
        cmp = lambda x, y: collections.Counter(x) == collections.Counter(y)
        return cmp(buff, buff_ctrl)

# Main test class
class MainTest(unittest.TestCase):

    ############### STATE COMMANDS ###############


    ############### LEDS and GPIOs ###############
    def test0200_led(self):
        for led in range(1, 8):
            self.assertEquals(Base().rp_led(rp_leds[led], '1'), '1')
            self.assertEquals(Base().rp_led(rp_leds[led], '0'), '0')


    def test0201_dpin(self):
        #Test pos state
        for pin in range(1, 8):
            self.assertEquals(Base().rp_dpin_state(rp_dpin_p[pin], '1'), '1')
            self.assertEquals(Base().rp_dpin_state(rp_dpin_p[pin], '0'), '0')

        #Test neg state
        for pin in range(len(rp_dpin_p)):
            self.assertEquals(Base().rp_dpin_state(rp_dpin_n[pin], '1'), '1')
            self.assertEquals(Base().rp_dpin_state(rp_dpin_n[pin], '0'), '0')

    def test0202_analog_pin(self):
        for a_pin in range(0, 3):
            self.assertTrue(1.2 <= float(Base().rp_analog_pin(rp_a_pin_o[a_pin], '1.34', True)) <= 1.4)
            self.assertTrue(0 <= float(Base().rp_analog_pin(rp_a_pin_i[a_pin], None, False)) <= 0.1)

    ############### SIGNAL GENERATOR ###############
    def test0300_freq(self):
        for i in range(len(rp_freq_range)):
            freq = rp_freq_range[i]
            self.assertEquals(float(Base().rp_freq(1, freq)), freq)
            self.assertEquals(float(Base().rp_freq(2, freq)), freq)

    def test0301_volt(self):
        for i in range(len(rp_volt_range)):
            volt = rp_volt_range[i]
            self.assertAlmostEquals(float(Base().rp_ampl(1, volt)), volt)
            self.assertAlmostEquals(float(Base().rp_ampl(2, volt)), volt)

    def test0302_w_form(self):
        for i in range(len(rp_wave_forms)):
            w_form = rp_wave_forms[i]
            self.assertEquals(Base().rp_w_form(1, w_form), w_form)
            self.assertEquals(Base().rp_w_form(2, w_form), w_form)

    def tes0303_offs(self):
        for i in range(len(rp_offs_range)):
            offs = rp_offs_range[i]
            self.assertAlmostEquals(float(Base().rp_offs(1, offs)), offs)
            self.assertAlmostEquals(float(Base().rp_offs(2, offs)), offs)

    def test0304_phase(self):
        for i in range(len(rp_phase_range)):
            phase = rp_phase_range[i]
            if(phase < 0):
                phase_new = phase + 360
                self.assertAlmostEquals(float(Base().rp_phase(1, phase)), phase_new)
                self.assertAlmostEquals(float(Base().rp_phase(2, phase)), phase_new)
            else:
                self.assertAlmostEquals(float(Base().rp_phase(1, phase)), phase)
                self.assertAlmostEquals(float(Base().rp_phase(2, phase)), phase)

    def test0305_dcyc(self):
        for i in range(len(rp_dcyc_range)):
            dcyc = rp_dcyc_range[i]
            self.assertEquals(float(Base().rp_dcyc(1, dcyc)), dcyc)
            self.assertEquals(float(Base().rp_dcyc(2, dcyc)), dcyc)

    #TODO: Scpi server returns 0 on INF given. This should also be fixed.
    def test0306_ncyc(self):
        for i in range(len(rp_ncyc_range)):
            ncyc = rp_ncyc_range[i]
            self.assertEquals(float(Base().rp_burst_ncyc(1, ncyc)), ncyc) if i != (len(rp_ncyc_range) - 1) else  self.assertEquals(Base().rp_burst_ncyc(1, ncyc), 'INF')
            self.assertEquals(float(Base().rp_burst_ncyc(2, ncyc)), ncyc) if i != (len(rp_ncyc_range) - 1) else  self.assertEquals(Base().rp_burst_ncyc(2, ncyc), 'INF')

    def test0307_nor(self):
        for i in range(len(rp_nor_range)):
            nor = rp_nor_range[i]
            self.assertEquals(float(Base().rp_burst_nor(1, nor)), nor) if i != (len(rp_nor_range) - 1) else  self.assertEquals(Base().rp_burst_nor(1, nor), 'INF')
            self.assertEquals(float(Base().rp_burst_nor(2, nor)), nor) if i != (len(rp_nor_range) - 1) else  self.assertEquals(Base().rp_burst_nor(2, nor), 'INF')

    def test0308_intp(self):
        for i in range(len(rp_inp_range)):
            intp = rp_inp_range[i]
            self.assertEquals(float(Base().rp_burst_intp(1, intp)), intp)
            self.assertEquals(float(Base().rp_burst_intp(2, intp)), intp)

    def test0309_burst_state(self):
        self.assertTrue(Base().rp_burst_state(1))
        self.assertTrue(Base().rp_burst_state(1))

    #TODO: Arbitrary-waveform. TRAC-DATA


    #Test generate
    def test0310_generate(self):
            assert (Base().generate_wform(1)) is True
            assert (Base().generate_wform(2)) is True

    ############### SIGNAL ACQUISITION TOOL ###############


if __name__ == '__main__':
    #TODO: Implement specific tests
    unittest.main(verbosity=2)







