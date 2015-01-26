__author__ = 'infused'


import scpi

host = '192.168.178.56'

rp_s = scpi.SCPI(host, None)

#rp_s.blink_diode(5, 1)

#rp_s.bar_led_graphs(67)

#rp_s.continuous_sig_gen('sine', 1000, 1)

#rp_s.cont_sig_gen_pulse('sine', 10, 1, 1)

rp_s.acq_buff()


rp_s.close()







