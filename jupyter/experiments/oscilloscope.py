from mercury import overlay, osc

import time
import numpy as np

from bokeh.io import push_notebook, show, output_notebook
from bokeh.models import HoverTool, Range1d
from bokeh.plotting import figure
from bokeh.layouts import widgetbox
from bokeh.resources import INLINE 

class oscilloscope (object):
    def __init__ (self, channels = [0, 1]):
        """Oscilloscope application"""

        # instantiate both oscilloscopes
        self.channels = channels

        # this will load the FPGA
        try:
            self.ovl = fpga.overlay("mercury")
        except ResourceWarning:
            print ("FPGA bitstream is already loaded")
        # wait a bit for the overlay to be properly applied
        # TODO it should be automated in the library
        time.sleep(0.5)

        # generators
        self.osc = [self.channel(ch) for ch in self.channels]

        # display widgets
        for ch in self.channels:
            self.osc[ch].display()

    def __del__ (self):
        # close widgets
        for ch in self.channels:
            self.osc[ch].close()
        # delete generator and overlay objects
        del (self.osc)
        # TODO: overlay should not be removed if a different app added it
        del (self.ovl)

    class channel (fpga.gen):

        def __init__ (self, ch):

            super().__init__(ch)

            self.reset()
            # TODO: separate masks
            sh = 5*ch
            self.mask = [0x1 << sh, 0x2 << sh, 0x4 << sh]
            self.amplitude = 0
            self.offset    = 0
            self.set_waveform()
            self.frequency = self.f_one
            self.phase     = 0
            self.start()
            self.trigger()

            # create widgets
            self.w_enable     = ipw.ToggleButton     (value=False, description='input enable')
            self.w_x_scale    = ipw.FloatSlider      (value=0,      min=-1.0, max=+1.0, step=0.02, description='X scale')
            self.w_x_position = ipw.FloatSlider      (value=0,      min=-1.0, max=+1.0, step=0.02, description='X position')
            self.w_y_scale    = ipw.FloatSlider      (value=0,      min=-1.0, max=+1.0, step=0.02, description='Y scale')
            self.w_y_position = ipw.FloatSlider      (value=0,      min=-1.0, max=+1.0, step=0.02, description='Y position')
            self.w_t_position = ipw.FloatRangeSlider (value=[0, 0], min=-1.0, max=+1.0, step=0.02, description='T position')
            self.w_t_holdoff  = ipw.FloatSlider      (value=0,      min=0,    max=1,    step=0.1,  description='T hold off')
            self.w_t_edge     = ipw.RadioButtons     (value='pos', options=['pos', 'neg'],         description='T edge')
            
            # style widgets
            self.w_enable.layout     = ipw.Layout(width='100%')
            self.w_x_scale.layout    = ipw.Layout(width='100%')
            self.w_x_position.layout = ipw.Layout(width='100%')
            self.w_y_scale.layout    = ipw.Layout(width='100%')
            self.w_y_position.layout = ipw.Layout(width='100%')
            self.w_t_position.layout = ipw.Layout(width='100%')
            self.w_t_holdoff.layout  = ipw.Layout(width='100%')

            self.w_enable.observe     (self.clb_enable    , names='value')
            self.w_x_scale.observe    (self.clb_x_scale   , names='value')
            self.w_x_position.observe (self.clb_x_position, names='value')
            self.w_y_scale.observe    (self.clb_y_scale   , names='value')
            self.w_y_position.observe (self.clb_y_position, names='value')
            self.w_t_position.observe (self.clb_t_position, names='value')
            self.w_t_holdoff.observe  (self.clb_t_holdoff , names='value')

        def clb_enable (self, change):
            i=0

        def clb_x_scale (self, change):
            i=0

        def clb_x_position (self, change):
            i=0

        def clb_y_scale (self, change):
            i=0

        def clb_y_position (self, change):
            i=0

        def clb_t_position (self, change):
            self.level = change['new']

        def clb_t_holdoff (self, change):
            self.holdoff = change['new']

        def display (self):
            layout = ipw.Layout(border='solid 2px', margin='2px 2px 2px 2px')
            # labels
            self.lbl_waveform  = ipw.Label(value="waveform shapes"               , layout = ipw.Layout(width='100%'))
            self.lbl_output    = ipw.Label(value="amplitude [V] and offset [V]"  , layout = ipw.Layout(width='100%'))
            self.lbl_frequency = ipw.Label(value="frequency [Hz] and phase [DEG]", layout = ipw.Layout(width='100%'))
            self.lbl_channel   = ipw.Label(value="generator"                     , layout = ipw.Layout(width='100%'))
            # boxes
            self.box_enable    = ipw.VBox([self.w_enable], layout = ipw.Layout(margin='2px 2px 2px 2px'))
            self.box_waveform  = ipw.VBox([self.lbl_waveform,  self.w_waveform, self.w_duty]   , layout = layout)
            self.box_output    = ipw.VBox([self.lbl_output,    self.w_amplitude, self.w_offset], layout = layout)
            self.box_frequency = ipw.VBox([self.lbl_frequency, self.w_frequency, self.w_phase] , layout = layout)
            self.box_channel = ipw.VBox([self.lbl_channel,
                                         self.box_enable,
                                         self.box_waveform,
                                         self.box_output,
                                         self.box_frequency],
                                        layout = ipw.Layout(border='solid 2px',
                                                            padding='2px 2px 2px 2px',
                                                            margin='4px 0px 4px 0px') )
            display (self.box_channel)

        def close(self):
            # boxes
            self.box_channel.close()
            self.box_enable.close()
            self.box_waveform.close()
            self.box_output.close()
            self.box_frequency.close()
            # labels
            self.lbl_waveform.close()
            self.lbl_output.close()
            self.lbl_frequency.close()
            self.lbl_channel.close()
            # widgets
            self.w_enable.close()
            self.w_waveform.close()
            self.w_duty.close()
            self.w_amplitude.close()
            self.w_offset.close()
            self.w_frequency.close()
            self.w_phase.close()