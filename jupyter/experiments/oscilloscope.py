# FPGA configuration and API
import mercury as fpga

# system and mathematics libraries
import time
import math
import numpy as np

# visualization
from bokeh.io import push_notebook, show, output_notebook
from bokeh.models import HoverTool, Range1d, LinearAxis
from bokeh.plotting import figure
from bokeh.layouts import widgetbox
from bokeh.resources import INLINE

# widgets
from IPython.display import display
import ipywidgets as ipw

class oscilloscope (object):

    size = 1250
    range_max = 1.2

    # time per division (10 divisions on screen)
    x_scales = [  '5s' ,   '2s' ,   '1s' ,
                '500ms', '200ms', '100ms',
                 '50ms',  '20ms',  '10ms',
                  '5ms',   '2ms',   '1ms',
                '500us', '200us', '100us',
                 '50us',  '20us',  '10us',
                  '5us',   '2us',   '1us']
    x_scales_dict = {  '5s' :   5000000,   '2s' :   2000000,   '1s' :   1000000,
                     '500ms':    500000, '200ms':    200000, '100ms':    100000,
                      '50ms':     50000,  '20ms':     20000,  '10ms':     10000,
                       '5ms':      5000,   '2ms':      2000,   '1ms':      1000,
                     '500us':       500, '200us':       200, '100us':       100,
                      '50us':        50,  '20us':        20,  '10us':        10,
                       '5us':         5,   '2us':         2,   '1us':         1}

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
        self.osc = [self.channel(top = self, ch = ch, input_range = self.input_range[ch]) for ch in self.channels]
        
        # set parameters common to all channels
        for ch in channels:
            # trigger timing is in the middle of the screen
            self.osc[ch].regset.cfg_pre = self.size/2
            self.osc[ch].regset.cfg_pst = self.size/2

        # default trigger source
        self.t_source = 0
#    def __del__ (self):
#        # close widgets
#        for ch in self.channels:
#            self.osc[ch].close()
#        # delete generator and overlay objects
#        del (self.osc)
#        # TODO: overlay should not be removed if a different app added it
#        del (self.ovl)

    def display (self):
        ch = 0
        self.x = (np.arange(self.size) - self.osc[ch].regset.cfg_pre) / self.osc[ch].sample_rate
        buff = [np.zeros(self.size) for ch in self.channels]
        rmax = 1.0
        
        #output_notebook(resources=INLINE)
        output_notebook()

        colors = ('red', 'blue')
        tools = "pan,wheel_zoom,box_zoom,reset,crosshair"
        self.p = figure(plot_height=500, plot_width=900, title="oscilloscope", toolbar_location="above", tools=(tools))
        self.p.xaxis.axis_label = 'time [s]'
        #self.p.yaxis.axis_label = 'voltage [V]'
        self.p.x_range = Range1d(self.x[0], self.x[-1])
        self.p.y_range = Range1d(-rmax, +rmax)
        self.p.extra_y_ranges = {str(ch): Range1d(-rmax, +rmax) for ch in self.channels}
        for ch in self.channels:
            self.p.add_layout(LinearAxis(y_range_name=str(ch).format(ch), axis_label = 'CH {} voltage [V]'.format(ch)), 'left')
        self.r = [self.p.line(self.x, buff[ch], line_width=1, line_alpha=0.7, color=colors[ch], y_range_name=str(ch)) for ch in self.channels]

        # trigger time/amplitude
        ch = 0
        if self.osc[ch].edg is 'pos': level = self.osc[ch].level[1]
        else                        : level = self.osc[ch].level[0]
        self.h_trigger_t = [self.p.line ([0,0], [-rmax, +rmax], color="black", line_width=1, line_alpha=0.75),
                            self.p.quad(left=[0], right=[self.osc[ch].holdoff],
                                        bottom=[-rmax], top=[+rmax], color="grey", alpha=0.25)]
        self.h_trigger_a = [self.p.line ([self.x[0], self.x[-1]], [level]*2, color="black", line_width=1, line_alpha=0.75),
                            self.p.quad(bottom=[self.osc[ch].level[0]], top=[self.osc[ch].level[1]],
                                        left=[self.x[0]], right=[self.x[-1]], color="grey", alpha=0.25)]

        # configure hover tool
        hover = HoverTool(mode = 'vline', tooltips=[("T", "@x"), ("V", "@y")], renderers=self.r)
        self.p.add_tools(hover)

        # style
        #self.p.yaxis[0].major_tick_line_color = None
        #self.p.yaxis[0].minor_tick_line_color = None
        self.p.yaxis[0].visible = False

        # get an explicit handle to update the next show cell with
        self.target = show(self.p, notebook_handle=True)

        # create widgets
        self.w_enable     = ipw.ToggleButton     (value=False, description='input enable')
        self.w_x_scale    = ipw.SelectionSlider  (value=self.x_scales[-1], options=self.x_scales, description='X scale')
        self.w_x_position = ipw.FloatSlider      (value=0, min=-rmax, max=+rmax, step=0.02, description='X position')
        self.w_t_source   = ipw.ToggleButtons    (value=self.t_source, options=[0, 1], description='T source')

        # style widgets
        self.w_enable.layout     = ipw.Layout(width='100%')
        self.w_x_scale.layout    = ipw.Layout(width='100%')
        self.w_x_position.layout = ipw.Layout(width='100%')

        self.w_enable.observe     (self.clb_enable    , names='value')
        self.w_x_scale.observe    (self.clb_x_scale   , names='value')
        self.w_x_position.observe (self.clb_x_position, names='value')
        self.w_t_source.observe   (self.clb_t_source  , names='value')
        
        display(self.w_x_scale, self.w_t_source)
        for ch in self.channels:
            self.osc[ch].display()

    def clb_enable (self, change):
            i=0

    def clb_x_scale (self, change):
        for ch in self.channels:
            self.osc[ch].decimation = self.x_scales_dict[change['new']]
        self.osc[self.channels[0]].reset()
        self.clb_x_update()

    def clb_x_position (self, change):
            i=0

    def clb_x_update (self):
        self.x = (np.arange(self.size) - self.osc[ch].regset.cfg_pre) / self.osc[ch].sample_rate
        for ch in self.channels:
            self.r[ch].data_source.data['x'] = self.x
        self.p.x_range.start = self.x[ 0]
        self.p.x_range.end   = self.x[-1]
        # trigger level and edge
        self.h_trigger_a[1].data_source.data['left']  = [self.x[ 0]]
        self.h_trigger_a[1].data_source.data['right'] = [self.x[-1]]
        self.h_trigger_a[0].data_source.data['x'] = [self.x[0], self.x[-1]]
        push_notebook(handle=self.target)

    def clb_t_source (self, change):
        self.t_source = change['new']
        sh   = 6*(0+2)
        sh_t = 6*(self.t_source+2)
        # TODO: handle trigger masks
        for ch in self.channels:
            self.osc[ch].mask = [(0x1<<sh), (0x2<<sh), (0x4<<sh), (0x8<<sh) | (0x10<<sh_t)]
        self.clb_t_update()
        self.clb_y_update()

    def clb_t_update (self):
        osc = self.osc[self.t_source]
        # trigger level and edge
        self.h_trigger_a[1].data_source.data['bottom'] = [osc.level[0]]
        self.h_trigger_a[1].data_source.data['top']    = [osc.level[1]]
        if   (osc.edge == 'pos'):
            self.h_trigger_a[0].data_source.data['y'] = [osc.level[1]]*2
        elif (osc.edge == 'neg'):
            self.h_trigger_a[0].data_source.data['y'] = [osc.level[0]]*2
        # trigger hold off time
        #self.h_trigger_t[0].data_source.data['x']     = [osc.holdoff]*2
        self.h_trigger_t[1].data_source.data['right'] = [osc.holdoff]
        push_notebook(handle=self.target)


    def clb_y_update (self):
        osc = self.osc[self.t_source]
        self.p.y_range.start = osc.y_position - osc.y_scale
        self.p.y_range.end   = osc.y_position + osc.y_scale

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

        # wigget related constants
        y_scales = [0.001, 0.002, 0.005, 0.01, 0.02, 0.05, 0.1, 0.2, 0.5, 1, 2]
        y_scale = 1
        y_position = 0

        def __init__ (self, top, ch, input_range = 1.0):

            super().__init__(ch, input_range)
            self.top = top
            self.ch = ch

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
            #sh = 6*(ch+2)
            sh = 6*(0+2)
            sh_t = 6*(0+2)
            self.mask = [(0x1<<sh), (0x2<<sh), (0x4<<sh), (0x8<<sh) | (0x10<<sh_t)]
            
            
            # create widgets
            self.w_t_edge     = ipw.ToggleButtons    (value=self.edge, options=['pos', 'neg'], description='T edge')
            self.w_t_position = ipw.FloatRangeSlider (value=self.level, min=-1.0, max=+1.0, step=0.02, description='T position')
            self.w_t_holdoff  = ipw.FloatSlider      (value=self.holdoff, min=0, max=1, step=0.1, description='T hold off')
            self.w_y_position = ipw.FloatSlider      (value=self.y_position, min=-1.0, max=+1.0, step=0.01, description='Y position')
            self.w_y_scale    = ipw.SelectionSlider  (value=self.y_scale, options=self.y_scales, description='Y scale')

            # style widgets
            self.w_t_position.layout = ipw.Layout(width='100%')
            self.w_t_holdoff.layout  = ipw.Layout(width='100%')
            self.w_y_scale.layout    = ipw.Layout(width='100%')
            self.w_y_position.layout = ipw.Layout(width='100%')

            self.w_t_edge.observe     (self.clb_t_edge    , names='value')
            self.w_t_position.observe (self.clb_t_position, names='value')
            self.w_t_holdoff.observe  (self.clb_t_holdoff , names='value')
            self.w_y_scale.observe    (self.clb_y_scale   , names='value')
            self.w_y_position.observe (self.clb_y_position, names='value')

        def clb_t_edge (self, change):
            self.edge = change['new']
            self.top.clb_t_update()

        def clb_t_position (self, change):
            self.level = change['new']
            self.top.clb_t_update()

        def clb_t_holdoff (self, change):
            self.holdoff = change['new']
            self.top.clb_t_update()

        def clb_y_position (self, change):
            self.y_position = change['new']
            self.clb_y_update()

        def clb_y_scale (self, change):
            self.y_scale = change['new']
            self.clb_y_update()

        def clb_y_update (self):
            # this does not work, probably a bug in bokeh
            # https://github.com/bokeh/bokeh/issues/4014#issuecomment-199147242
            #app.p.y_range = Range1d(-0.2, +0.2)
            self.top.p.extra_y_ranges[str(self.ch)].start = self.y_position - self.y_scale
            self.top.p.extra_y_ranges[str(self.ch)].end   = self.y_position + self.y_scale
            self.top.clb_y_update()

        def display (self):
            display(self.w_t_edge, self.w_t_position, self.w_t_holdoff, self.w_y_position, self.w_y_scale)
