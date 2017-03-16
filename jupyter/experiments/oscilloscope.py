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

class oscilloscope (object):

    size = 1024

    def __init__ (self, channels = [0, 1], input_range = [1.0, 1.0]):
        """Oscilloscope application"""

        # instantiate both oscilloscopes
        self.channels = channels
        self.input_range = input_range

        # this will load the FPGA
        try:
            self.ovl = fpga.overlay("mercury")
        except ResourceWarning:
            print ("FPGA bitstream is already loaded")
        # wait a bit for the overlay to be properly applied
        # TODO it should be automated in the library
        time.sleep(0.5)

        # ocsilloscope channels
        self.osc = [self.channel(ch, self.input_range[ch]) for ch in self.channels]
        
        # set parameters common to all channels
        for ch in channels:
            # trigger timing is in the middle of the screen
            self.osc[ch].regset.cfg_pre = self.size/2
            self.osc[ch].regset.cfg_pst = self.size/2

    def __del__ (self):
        # close widgets
        for ch in self.channels:
            self.osc[ch].close()
        # delete generator and overlay objects
        del (self.osc)
        # TODO: overlay should not be removed if a different app added it
        del (self.ovl)

    def display (self):
        ch = 0
        self.x = (np.arange(self.size) - self.osc[ch].regset.cfg_pre) / fpga.osc.FS
        buff = [np.zeros(self.size) for ch in self.channels]

        #output_notebook(resources=INLINE)
        output_notebook()

        colors = ('red', 'blue')
        tools = "pan,wheel_zoom,box_zoom,reset,crosshair"
        p = figure(plot_height=500, plot_width=900, title="oscilloscope", toolbar_location="above", tools=(tools))
        p.xaxis.axis_label='time [s]'
        p.yaxis.axis_label='voltage [V]'
        p.y_range=Range1d(-1.2, +1.2)
        self.r = [p.line(self.x, buff[ch], line_width=1, line_alpha=0.7, color=colors[ch]) for ch in self.channels]

        # trigger time/amplitude
        ch = 0
        if self.osc[ch].edg is 'pos': level = self.osc[ch].level[1]
        else                        : level = self.osc[ch].level[0]
        self.h_trigger_t = [p.line ([0,0], [-1.2, +1.2], color="black", line_width=1, line_alpha=0.75),
                            p.quad(left=[0], right=[self.osc[ch].holdoff],
                                   bottom=[-1.2], top=[+1.2], color="grey", alpha=0.25)]
        self.h_trigger_a = [p.line ([self.x[0], self.x[-1]], [level]*2, color="black", line_width=1, line_alpha=0.75),
                            p.quad(bottom=[self.osc[ch].level[0]], top=[self.osc[ch].level[1]],
                                   left=[self.x[0]], right=[self.x[-1]], color="grey", alpha=0.25)]

        # configure hover tool
        hover = HoverTool(mode = 'vline', tooltips=[("T", "@x"), ("V", "@y")], renderers=self.r)
        p.add_tools(hover)

        # get an explicit handle to update the next show cell with
        self.target = show(p, notebook_handle=True)

        # create widgets
        self.w_enable     = ipw.ToggleButton     (value=False, description='input enable')
        self.w_x_scale    = ipw.FloatSlider      (value=0,      min=-1.0, max=+1.0, step=0.02, description='X scale')
        self.w_x_position = ipw.FloatSlider      (value=0,      min=-1.0, max=+1.0, step=0.02, description='X position')
        self.w_y_scale    = ipw.FloatSlider      (value=0,      min=-1.0, max=+1.0, step=0.02, description='Y scale')
        self.w_y_position = ipw.FloatSlider      (value=0,      min=-1.0, max=+1.0, step=0.02, description='Y position')

        self.w_t_source   = ipw.RadioButtons     (value=0, options=[0, 1], description='T source')
        self.w_t_edge     = ipw.RadioButtons     (value='pos', options=['pos', 'neg'], description='T edge')
        self.w_t_position = ipw.FloatRangeSlider (value=self.osc[ch].level, min=-1.0, max=+1.0, step=0.02, description='T position')
        self.w_t_holdoff  = ipw.FloatSlider      (value=0,      min=0,    max=1,    step=0.1,  description='T hold off')

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

        self.w_t_edge.observe     (self.clb_t_edge    , names='value')
        self.w_t_position.observe (self.clb_t_position, names='value')
        self.w_t_holdoff.observe  (self.clb_t_holdoff , names='value')

        display(self.w_t_source, self.w_t_edge, self.w_t_position, self.w_t_holdoff, )

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

    def clb_t_edge (self, change):
        self.osc[0].edge = change['new']
        if   (change['new'] == 'pos'):
            self.h_trigger_a[0].data_source.data['y'] = [self.osc[0].level[1]]*2
        elif (change['new'] == 'neg'):
            self.h_trigger_a[0].data_source.data['y'] = [self.osc[0].level[0]]*2
        push_notebook(handle=self.target)

    def clb_t_position (self, change):
        self.osc[0].level = change['new']
        self.h_trigger_a[0].data_source.data['y']      = [change['new'][1]]*2
        self.h_trigger_a[1].data_source.data['bottom'] = [change['new'][0]]
        self.h_trigger_a[1].data_source.data['top']    = [change['new'][1]]
        push_notebook(handle=self.target)

    def clb_t_holdoff (self, change):
        self.osc[0].holdoff = change['new']
        self.h_trigger_t[0].data_source.data['x']     = [change['new']]*2
        self.h_trigger_t[1].data_source.data['right'] = [change['new']]
        push_notebook(handle=self.target)

    def run (self):
        while True:
            ch = 0
            self.osc[ch].reset()
            self.osc[ch].start()
            while self.osc[ch].status_run(): pass
            #buff = np.absolute(np.fft.fft(buff))
            for ch in self.channels:
                self.r[ch].data_source.data['y'] = self.osc[ch].data(self.size)
            # push updates to the plot continuously using the handle (intererrupt the notebook kernel to stop)
            push_notebook(handle=self.target)
            #time.sleep(0.05)

    class channel (fpga.osc):

        def __init__ (self, ch, input_range = 1.0):

            super().__init__(ch, input_range)

            self.reset()
            # TODO: for now bypass input filter
            self.filter_bypass = True
            # decimation rate
            self.decimation = 1

            # trigger level [V], edge and holdoff [periods]
            self.level = [-0.1, +0.1]
            self.edg = 'pos'
            self.holdoff = 0

            # trigger source mask
            #sh = 5*(ch+2)
            sh = 5*(0+2)
            self.mask = [(0x1<<sh), (0x2<<sh), (0x4<<sh) | (0x8<<10)]