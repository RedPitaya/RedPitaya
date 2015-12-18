
__author__ = "Luka Golinar <luka.golinar@gmail.com>"

#Imports
import redpitaya_scpi as scpi
import unittest

#Scpi declaration
rp_scpi = scpi.scpi('192.168.1.241')

#Global variables
rp_dpin_p  = {i: 'DIO'+str(i)+'_P' for i in range(8)}
rp_dpin_n  = {i: 'DIO'+str(i)+'_N' for i in range(8)}
rp_a_pin_o = {i: 'AOUT'+str(i) for i in range(4)}
rp_a_pin_i = {i: 'AIN'+str(i) for i in range(4)}
rp_leds    = {i: 'LED'+str(i) for i in range(8)}

rp_freq_range  = [100, 1000, 10000, 100000, 1e+06, 1e+07, 3e+07]
rp_volt_range  = [0.25, 0.5, 0.75, 1.0]
rp_phase_range = [-360, -180, -90, -30, 30, 90, 180, 360]
rp_offs_range  = [-0.75, -0.5, -0.25, 0.25, 0.5 , 0.75]
rp_dcyc_range  = [0.5, 1, 10, 50, 75, 100]
rp_ncyc_range  = [0, 1, 10, 100, 1000, 10000, 50000]
rp_nor_range   = rp_ncyc_range[:]
rp_inp_range   = [i * 100 for i in range(1, 6)]
rp_channels    = ['CH1', 'CH2', 'MATH']
rp_scales      = [0.5, 1, 2, 3, 10]
rp_decimation  = ['1', '8', '64', '1024', '8192', '65536']
rp_sampling    = ['100', '10000', '100000', '1000000', '10000000', '125000000']
rp_trig_dly    = ['10', '1000', '10000', '16500']
rp_trig_dly_ns = ['10', '100', '250', '500']
rp_trig_level  = ['10', '100', '1000'] #TODO: Add more options

rp_gen_mode    = ['CONTINUOUS', 'BURST']
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
        rp_scpi.tx_txt('SOUR' + str(channel) + ':PHAS ' + str(phase) + ' DEG')
        rp_scpi.tx_txt('SOUR' + str(channel) + ':PHAS?')
        return rp_scpi.rx_txt()

    def rp_dcyc(self, channel, dcyc):
        rp_scpi.tx_txt('SOUR' + str(channel) + ':DCYC ' + str(dcyc))
        rp_scpi.tx_txt('SOUR' + str(channel) + ':DCYC?')
        return rp_scpi.rx_txt()

    def rp_burst_ncyc(self, channel, ncyc):
        rp_scpi.tx_txt('SOUR' + str(channel) + ':BURS:NCYC ' + str(ncyc))
        rp_scpi.tx_txt('SOUR' + str(channel) + ':BURS:NCYC?')
        return rp_scpi.rx_txt()

    def rp_burst_nor(self, channel, nor):
        rp_scpi.tx_txt('SOUR' + str(channel) + ':BURS:NCYC ' + str(nor))
        rp_scpi.tx_txt('SOUR' + str(channel) + ':BURS:NCYC?')
        return rp_scpi.rx_txt()

    def rp_burst_intp(self, channel, intp):
        #Set number of cycles to 0, for period repeatibilty (period = signal_time * burst_count + delay_time)
        rp_scpi.tx_txt('SOUR' + str(channel) + ':BURS:NCYC 0')
        rp_scpi.tx_txt('SOUR' + str(channel) + ':BURS:INT:PER ' + str(intp))
        rp_scpi.tx_txt('SOUR' + str(channel) + ':BURS:INT:PER?')
        return rp_scpi.rx_txt()

    def rp_gen_trig_src(self, channel, source):
        rp_scpi.tx_txt('SOUR' + str(channel) + ':TRIG:SOUR ' + source)
        rp_scpi.tx_txt('SOUR' + str(channel) + ':TRIG:SOUR?')
        return rp_scpi.rx_txt()

    ## ACQUIRE
    def rp_smpl_dec(self, decimation):
        #Decimation
        rp_scpi.tx_txt('ACQ:DEC ' + decimation)
        rp_scpi.tx_txt('ACQ:DEC?')
        if(rp_scpi.rx_txt() != decimation): return False
        #Smpl-rate
        '''
        rp_scpi.tx_txt('ACQ:SRAT?')
        sample_rate = rp_scpi.rx_txt().replace('\n\r', '')[:-3]
        if(rp_scpi.rx_txt() != (decimation) / )
        '''
        return True

    def rp_sampling(self, rate):
        rp_scpi.tx_txt('ACQ:SRAT ' + rate)
        rp_scpi.tx_txt('ACQ:SRAT?')
        return rp_scpi.rx_txt()

    def rp_averaging(self, averaging):
        rp_scpi.tx_txt('ACQ:AVG ' + averaging)
        rp_scpi.tx_txt('ACQ:AVG?')
        return rp_scpi.rx_txt()

    def rp_trigger_delay(self, delay):
        rp_scpi.tx_txt('ACQ:TRIG:DLY ' + delay)
        rp_scpi.tx_txt('ACQ:TRIG:DLY?')
        return rp_scpi.rx_txt()

    def rp_trigger_delay_ns(self, delay_ns):
        rp_scpi.tx_txt('ACQ:TRIG:DLY:NS ' + delay_ns)
        rp_scpi.tx_txt('ACQ:TRIG:DLY:NS?')
        return rp_scpi.rx_txt()

    def rp_trigger_hyst(self, hyst):
        rp_scpi.tx_txt('ACQ:TRIG:HYST ' + hyst)
        rp_scpi.tx_txt('ACQ:TRIG:HYST?')
        return rp_scpi.rx_txt()

    def rp_trigger_level(self, level):
        rp_scpi.tx_txt('ACQ:TRIG:LEV ' + level)
        rp_scpi.tx_txt('ACQ:TRIG:LEV?')
        return rp_scpi.rx_txt()

    def rp_data_units(self, units):
        rp_scpi.tx_txt('ACQ:DATA:UNITS ' + units)
        rp_scpi.tx_txt('ACQ:DATA:UNITS?')
        return rp_scpi.rx_txt()

    def rp_buffer_size(self):
        rp_scpi.tx_txt('ACQ:BUF:SIZE?')
        return rp_scpi.rx_txt()

    #Burst state must also set burst counts to something other than 0.
    #Api checks, if burst count is not equal to 0 or yes and with the latter being true
    #returns continious mode, therefore rp_burst_state will always return OFF and fail.
    #To prevent this, we set Burts count to 1 before starting the burst state test.
    #TODO: Make a better commentary.
    def rp_burst_state(self, channel):
        rp_scpi.tx_txt('SOUR' + str(channel) + ':BURS:NCYC 1')
        for i in range(len(rp_gen_mode)):
            rp_scpi.tx_txt('SOUR' + str(channel) + ':BURS:STAT ' + rp_gen_mode[i])
            rp_scpi.tx_txt('SOUR' + str(channel) + ':BURS:STAT?')
            if(rp_scpi.rx_txt().strip('\n') != rp_gen_mode[i]):
                return False
        return True

    def generate_wform(self, channel):

        buff = []
        buff_ctrl = []

        #Do not change these values!
        freq = 7629.39453125
        ampl = 0.8
        wave_form = 'SINE'

        #Init Red Pitaya resources
        rp_scpi.tx_txt('RP:INIT')

        #Enable Red Pitaya digital loop
        rp_scpi.tx_txt('RP:DIG')
        rp_scpi.tx_txt('ACQ:START')

        #Set generator options
        rp_scpi.tx_txt('SOUR' + str(channel) + ':VOLT ' + str(ampl))
        rp_scpi.tx_txt('SOUR' + str(channel) + ':FUNC ' + str(wave_form))
        rp_scpi.tx_txt('OUTPUT' + str(channel) + ':STATE ON')
        rp_scpi.tx_txt('SOUR' + str(channel) + ':FREQ:FIX ' + str(freq))
        rp_scpi.tx_txt('ACQ:TRIG CH' + str(channel) + '_PE')

        rp_scpi.tx_txt('ACQ:SOUR' + str(channel) + ':DATA?')

        buff_string = rp_scpi.rx_txt()
        buff_string = buff_string.strip('{}\n\r').replace("  ", "").split(',')
        buff = map(float, buff_string)

        if(channel == 1):
            buff_ctrl = open('./ctrl_data/gen_ctrl_ch1', 'r').readlines()
        else:
           buff_ctrl = open('./ctrl_data/gen_ctrl_ch2', 'r').readlines()

        for i in range(len(buff_ctrl)):
            buff_ctrl[i] = float(buff_ctrl[i].strip('\n'))

        rp_scpi.tx_txt('RP:RESET')
        return (buff[:] == buff_ctrl[:])

# Main test class
class MainTest(unittest.TestCase):

    ############### STATE COMMANDS ###############


    ############### LEDS and GPIOs ###############
    def test0200_led(self):
        for i in rp_leds:
            self.assertEquals(Base().rp_led(rp_leds[i], '1'), '1')
            self.assertEquals(Base().rp_led(rp_leds[i], '0'), '0')


    def test0201_dpin(self):
        #Test pos state
        for pin_p in range(1, 8):
            self.assertEquals(Base().rp_dpin_state(rp_dpin_p[pin_p], '1'), '1')
            self.assertEquals(Base().rp_dpin_state(rp_dpin_p[pin_p], '0'), '0')

        #Test neg state
        for pin_n in rp_dpin_p:
            self.assertEquals(Base().rp_dpin_state(rp_dpin_n[pin_n], '1'), '1')
            self.assertEquals(Base().rp_dpin_state(rp_dpin_n[pin_n], '0'), '0')

    def test0202_analog_pin(self):
        for a_pin in range(0, 3):
            self.assertTrue(1.2 <= float(Base().rp_analog_pin(rp_a_pin_o[a_pin], '1.34', True)) <= 1.4)
            self.assertTrue(0 <= float(Base().rp_analog_pin(rp_a_pin_i[a_pin], None, False)) <= 0.1)

    ############### SIGNAL GENERATOR ###############
    def test0300_freq(self):
        for freq in rp_freq_range:
            self.assertEquals(float(Base().rp_freq(1, freq)), freq)
            self.assertEquals(float(Base().rp_freq(2, freq)), freq)

    def test0301_volt(self):
        for volt in rp_volt_range:
            self.assertAlmostEquals(float(Base().rp_ampl(1, volt)), volt)
            self.assertAlmostEquals(float(Base().rp_ampl(2, volt)), volt)

    def test0302_w_form(self):
        for w_form in rp_wave_forms:
            self.assertEquals(Base().rp_w_form(1, w_form), w_form)
            self.assertEquals(Base().rp_w_form(2, w_form), w_form)

    def tes0303_offs(self):
        for offs in rp_offs_range:
            self.assertAlmostEquals(float(Base().rp_offs(1, offs)), offs)
            self.assertAlmostEquals(float(Base().rp_offs(2, offs)), offs)

    def test0304_phase(self):
        for phase in rp_phase_range:
            if(phase < 0):
                phase_new = phase + 360
                self.assertAlmostEquals(float(Base().rp_phase(1, phase)), phase_new)
                self.assertAlmostEquals(float(Base().rp_phase(2, phase)), phase_new)
            else:
                self.assertAlmostEquals(float(Base().rp_phase(1, phase)), phase)
                self.assertAlmostEquals(float(Base().rp_phase(2, phase)), phase)

    def test0305_dcyc(self):
        for dcyc in rp_dcyc_range:
            self.assertEquals(float(Base().rp_dcyc(1, dcyc)), dcyc)
            self.assertEquals(float(Base().rp_dcyc(2, dcyc)), dcyc)

    def test0306_ncyc(self):
        for ncyc in rp_ncyc_range:
            self.assertEquals(float(Base().rp_burst_ncyc(1, ncyc)), ncyc)
            self.assertEquals(float(Base().rp_burst_ncyc(2, ncyc)), ncyc)

    def test0307_nor(self):
        for nor in rp_nor_range:
            self.assertEquals(float(Base().rp_burst_nor(1, nor)), nor)
            self.assertEquals(float(Base().rp_burst_nor(2, nor)), nor)

    def test0308_intp(self):
        for intp in rp_inp_range:
            self.assertEquals(float(Base().rp_burst_intp(1, intp)), intp)
            self.assertEquals(float(Base().rp_burst_intp(2, intp)), intp)

    def test0309_burst_state(self):
        self.assertTrue(Base().rp_burst_state(1))
        self.assertTrue(Base().rp_burst_state(2))

    #Test generate
    def test000_generate(self):
        assert (Base().generate_wform(1)) is True
        assert (Base().generate_wform(2)) is True

    ############### SIGNAL ACQUISITION TOOL ###############
    def test0401_acq_decimation(self):
        for decimation in rp_decimation:
            assert (Base().rp_smpl_dec(decimation)) is True

    def test0402_acq_avg(self):
        self.assertEquals(Base().rp_averaging('ON'), 'ON')
        self.assertEquals(Base().rp_averaging('OFF'), 'OFF')

    def test0403_trig_dly(self):
        for delay in rp_trig_dly:
            self.assertEquals(Base().rp_trigger_delay(delay), delay)

        ''' TODO: Trigger delay in nano seconds
        for delay_ns in rp_trig_dly_ns:
            print(delay_ns)
            self.assertEquals(Base().rp_trigger_delay_ns(delay_ns), delay_ns)
        '''

    #TODO
    def test04040_trig_hyst(self):
        return 0

    def test04050_trig_level(self):
        return 0
        '''
        for lvl in rp_trig_level:
            self.assertEquals(Base().rp_trigger_level(lvl), lvl)
        '''

    def test04060_data_units(self):
        self.assertEquals(Base().rp_data_units('VOLTS'), 'VOLTS')
        self.assertEquals(Base().rp_data_units('RAW'), 'RAW')

    def test04070_buffer_size(self):
        self.assertEquals(Base().rp_buffer_size(), '16384')

    #TODO: ACQ:WPOS?  ACQ:TPOS?
    #TODO: Arbitrary-waveform. TRAC-DATA


if __name__ == '__main__':
    #TODO: Implement specific tests
    unittest.main(verbosity=2)







