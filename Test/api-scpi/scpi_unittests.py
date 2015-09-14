
__author__ = "Luka Golinar <luka.golinar@gmail.com>"

#Imports
import sys
import redpitaya_scpi as scpi
import unittest


#Scpi declaration
rp_scpi = scpi.scpi('192.168.178.111')

#Global variables
rp_dpin_p = {1: 'DIO1_P', 2: 'DIO2_P', 3: 'DIO3_P', 4: 'DIO4_P', 5: 'DIO5_P', 6: 'DIO6_P', 7: 'DIO7_P', 8: 'DIO8_P'}
rp_dpin_n = {0: 'DIO0_N', 1: 'DIO1_N', 2: 'DIO2_N', 3: 'DIO3_N', 4: 'DIO4_N', 5: 'DIO5_N', 6: 'DIO6_N', 7: 'DIO7_N', 8:'DIO8_N'}
rp_a_pin_o = {0: 'AOUT0', 1: 'AOUT1', 2: 'AOUT2', 3: 'AOUT3'}
rp_a_pin_i = {0: 'AIN0', 1: 'AIN1', 2: 'AIN2', 3: 'AIN3'}
rp_leds = {1: 'LED1', 2: 'LED2', 3: 'LED3', 4: 'LED4', 5: 'LED5', 6: 'LED6', 7: 'LED7', 8: 'LED8'}

rp_freq_range = ['100', '1000', '10000', '100000', '1000000', '10000000', '30000000']
rp_ampl_range = ['0.1', '0.25', '0.5', '0.75', '1.0']

# Base functions
class Base(object):

    def rp_led(self, led, state):
        rp_scpi.tx_txt('DIG:PIN ' + led + ', ' + state)
        rp_scpi.tx_txt('DIG:PIN? ' + led)
        state = rp_scpi.rx_txt()
        return state

    #TODO: Direction

    def rp_dpin_state(self, pin, state):
        rp_scpi.tx_txt('DIG:PIN ' + pin + ', ' + state)
        rp_scpi.tx_txt('DIG:PIN? ' + pin)
        state = rp_scpi.rx_txt()
        return state

    def rp_analog_pin(self, pin, state, out):
        if(out is True):
            rp_scpi.tx_txt('ANALOG:PIN ' + pin + ', ' + state)
        rp_scpi.tx_txt('ANALOG:PIN? ' + pin)
        state = rp_scpi.rx_txt()
        return state

    def rp_freq(self, channel, freq):
        rp_scpi.tx_txt('SOUR1' + ':FREQ:FIX ' + freq)
        rp_scpi.tx_txt('SOUR1' + ':FREQ:FIX?')
        state = rp_scpi.rx_txt()
        print(state)
        return state


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
            self.assertEquals(Base().rp_freq(1, freq), freq)


if __name__ == '__main__':
    unittest.main(verbosity=3)







