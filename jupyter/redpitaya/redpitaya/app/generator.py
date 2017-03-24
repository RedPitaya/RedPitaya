# FPGA configuration and API
import mercury as fpga

# system and mathematics libraries
import time
import math
import numpy as np

# visualization
from bokeh.io import push_notebook, show, output_notebook
from bokeh.models import HoverTool, Range1d
from bokeh.plotting import figure
from bokeh.layouts import widgetbox
from bokeh.resources import INLINE

# widgets
from IPython.display import display
import ipywidgets as ipw

class generator (object):
    def __init__ (self, channels = [0, 1]):
        """Generator application"""

        # instantiate both generators
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
        self.gen = [self.channel(ch) for ch in self.channels]

        # display widgets
        for ch in self.channels:
            self.gen[ch].display()

    def __del__ (self):
        # close widgets
        for ch in self.channels:
            self.gen[ch].close()
        # delete generator and overlay objects
        del (self.gen)
        # TODO: overlay should not be removed if a different app added it
        del (self.ovl)

    class channel (fpga.gen):
        # waveform defaults
        form = 'sine'
        duty = 0.5

        def set_waveform (self):
            if   self.form is 'sine':
                self.waveform = self.sine()
            elif self.form is 'square':
                self.waveform = self.square(self.duty)
            elif self.form is 'sawtooth':
                self.waveform = self.sawtooth(self.duty)

        def __init__ (self, ch):

            super().__init__(ch)

            self.reset()
            # TODO: separate masks
            sh = 6*ch
            self.mask = [0x1 << sh, 0x2 << sh, 0x4 << sh, 0x8 << sh]
            self.amplitude = 0
            self.offset    = 0
            self.set_waveform()
            self.frequency = self.f_one
            self.phase     = 0
            self.start()
            self.trigger()

            # create widgets
            self.w_enable    = ipw.ToggleButton (value=False, description='output enable')
            self.w_waveform  = ipw.ToggleButtons(value='sine', options=['sine', 'square', 'sawtooth'], description='waveform')
            self.w_duty      = ipw.FloatSlider  (value=0.5, min=0.0, max=1.0, step=0.01, description='duty')
            self.w_amplitude = ipw.FloatSlider  (value=0, min=-1.0, max=+1.0, step=0.02, description='amplitude')
            self.w_offset    = ipw.FloatSlider  (value=0, min=-1.0, max=+1.0, step=0.02, description='offset')
            self.w_frequency = ipw.FloatSlider  (value=self.fl_one, min=self.fl_min, max=self.fl_max, step=0.02, description='frequency')
            self.w_phase     = ipw.FloatSlider (value=0, min=0, max=360, step=1, description='phase')

            # style widgets
            self.w_enable.layout    = ipw.Layout(width='100%')
            self.w_duty.layout      = ipw.Layout(width='100%')
            self.w_amplitude.layout = ipw.Layout(width='100%')
            self.w_offset.layout    = ipw.Layout(width='100%')
            self.w_frequency.layout = ipw.Layout(width='100%')
            self.w_phase.layout     = ipw.Layout(width='100%')

            self.w_enable.observe   (self.clb_enable   , names='value')
            self.w_waveform.observe (self.clb_waveform , names='value')
            self.w_duty.observe     (self.clb_duty     , names='value')
            self.w_amplitude.observe(self.clb_amplitude, names='value')
            self.w_offset.observe   (self.clb_offset   , names='value')
            self.w_frequency.observe(self.clb_frequency, names='value')
            self.w_phase.observe    (self.clb_phase    , names='value')
            
        def clb_enable (self, change):
            self.enable = change['new']

        def clb_waveform (self, change):
            self.form = change['new']
            self.set_waveform()

        def clb_duty (self, change):
            self.duty = change['new']
            self.set_waveform()

        def clb_amplitude (self, change):
            self.amplitude = change['new']

        def clb_offset (self, change):
            self.offset = change['new']

        def clb_frequency (self, change):
            self.frequency = math.pow(10, change['new'])

        def clb_phase (self, change):
            self.phase = change['new']

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

        def __del__ (self):
            # widgets
            del(w_enable)
            del(w_waveform)
            del(w_duty)
            del(w_amplitude)
            del(w_offset)
            del(w_frequency)
            del(w_phase)
            # calling super class
            super().__del__()
