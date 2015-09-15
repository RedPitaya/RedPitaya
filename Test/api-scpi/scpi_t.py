
__author__ = "Luka Golinar <luka.golinar@gmail.com>"

#Imports
import sys
import redpitaya_scpi as scpi
import unittest
import collections


#Scpi declaration
rp_scpi = scpi.scpi('192.168.178.111')

#Global variables
rp_dpin_p  = {1: 'DIO1_P', 2: 'DIO2_P', 3: 'DIO3_P', 4: 'DIO4_P', 5: 'DIO5_P', 6: 'DIO6_P', 7: 'DIO7_P', 8: 'DIO8_P'}
rp_dpin_n  = {0: 'DIO0_N', 1: 'DIO1_N', 2: 'DIO2_N', 3: 'DIO3_N', 4: 'DIO4_N', 5: 'DIO5_N', 6: 'DIO6_N', 7: 'DIO7_N', 8:'DIO8_N'}
rp_a_pin_o = {0: 'AOUT0', 1: 'AOUT1', 2: 'AOUT2', 3: 'AOUT3'}
rp_a_pin_i = {0: 'AIN0', 1: 'AIN1', 2: 'AIN2', 3: 'AIN3'}
rp_leds    = {1: 'LED1', 2: 'LED2', 3: 'LED3', 4: 'LED4', 5: 'LED5', 6: 'LED6', 7: 'LED7', 8: 'LED8'}

rp_freq_range  = [100, 1000, 10000, 100000, 1e+06, 1e+07, 3e+07]
rp_volt_range  = [0.25, 0.5, 0.75, 1.0]
rp_phase_range = [360, -180, -90, -30, 30, 90, 180, 360]
rp_offs_range  = [-0.75, -0.5, -0.25, 0.25, 0.5 , 0.75]

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

    def generate_wform(self, channel):

        buff = []
        buff_ctrl = []

        #Sample data
        freq = 100
        ampl = 1
        wave_form = rp_wave_forms[0]

        #Enable Red Pitaya digital loop
        rp_scpi.tx_txt('OSC:RUN:DIGLOOP')

        #Set generator options
        rp_scpi.tx_txt('SOUR' + str(channel) + ':FREQ:FIX ' + str(freq))
        rp_scpi.tx_txt('SOUR' + str(channel) + ':VOLT ' + str(ampl))
        rp_scpi.tx_txt('SOUR' + str(channel) + ':FUNC ' + str(wave_form))
        rp_scpi.tx_txt('OUTPUT' + str(channel) + ':STATE ON')

        #Acquire bufferrp_s.tx_txt('ACQ:START')
        rp_scpi.tx_txt('ACQ:TRIG NOW')
        rp_scpi.tx_txt('ACQ:TRIG:STAT?')
        rp_scpi.rx_txt()
        rp_scpi.tx_txt('ACQ:SOUR' str(channel) + ':DATA?')

        buff_string = rp_scpi.rx_txt()
        buff_string = buff_string.strip('{}\n\r').replace("  ", "").split(',')
        buff = map(float, buff_string)
        buff_ctrl = open('./ctrl_data/gen_ctrl', 'r').readlines(len(buff))

        for i in range(0, len(buff_ctrl)):
            buff_ctrl[i] = buff_ctrl[i].strip('\n')

        #Compare the two buffers
        f = lambda x1, x2: x1[0:] == x2[:-1]

        return f(buff, buff_ctrl)

# Main test class
class MainTest(unittest.TestCase):

    ############### LEDS and GPIOs ###############
    def test_led(self):
        for led in range(1, 8):
            self.assertEquals(Base().rp_led(rp_leds[led], '1'), '1')
            self.assertEquals(Base().rp_led(rp_leds[led], '0'), '0')


    def test_dpin(self):
        #Test pos state
        for pin in range(1, 8):
            self.assertEquals(Base().rp_dpin_state(rp_dpin_p[pin], '1'), '1')
            self.assertEquals(Base().rp_dpin_state(rp_dpin_p[pin], '0'), '0')

        #Test neg state
        for pin in range(0, 8):
            self.assertEquals(Base().rp_dpin_state(rp_dpin_n[pin], '1'), '1')
            self.assertEquals(Base().rp_dpin_state(rp_dpin_n[pin], '0'), '0')

    def test_analog_pin(self):
        for a_pin in range(0, 3):
            self.assertTrue(1.2 <= float(Base().rp_analog_pin(rp_a_pin_o[a_pin], '1.34', True)) <= 1.4)
            self.assertTrue(0 <= float(Base().rp_analog_pin(rp_a_pin_i[a_pin], None, False)) <= 0.1)

    ############### SIGNAL GENERATOR ###############
    def test_freq(self):
        for i in range(0, len(rp_freq_range)):
            freq = rp_freq_range[i]
            self.assertEquals(float(Base().rp_freq(1, freq)), freq)
            self.assertEquals(float(Base().rp_freq(2, freq)), freq)

    def test_volt(self):
        for i in range(0, len(rp_volt_range)):
            volt = rp_volt_range[i]
            self.assertAlmostEquals(float(Base().rp_ampl(1, volt)), volt)
            self.assertAlmostEquals(float(Base().rp_ampl(2, volt)), volt)

    def test_w_form(self):
        for i in range(0, len(rp_wave_forms)):
            w_form = rp_wave_forms[i]
            self.assertEquals(Base().rp_w_form(1, w_form), w_form)
            self.assertEquals(Base().rp_w_form(2, w_form), w_form)

    def test_offs(self):
        for i in range(0, len(rp_offs_range)):
            offs = rp_offs_range[i]
            self.assertAlmostEquals(float(Base().rp_offs(1, offs)), offs)
            self.assertAlmostEquals(float(Base().rp_offs(2, offs)), offs)

    def test_phase(self):
        for i in range(0, len(rp_phase_range)):
            phase = rp_phase_range[i]
            if(phase < 0):
                phase_new = phase + 360
                self.assertAlmostEquals(float(Base().rp_phase(1, phase)), phase_new)
                self.assertAlmostEquals(float(Base().rp_phase(2, phase)), phase_new)
            else:
                self.assertAlmostEquals(float(Base().rp_phase(1, phase)), phase)
                self.assertAlmostEquals(float(Base().rp_phase(2, phase)), phase)

    #Test generate
    def test_generate(self):
        for form in rp_wave_forms:
            assert Base().generate_wform(form) is True


if __name__ == '__main__':
    #TODO: Implement specific tests
    unittest.main(verbosity=2)







