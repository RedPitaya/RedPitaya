/*
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


(function(LA, $, undefined) {

    LA.graphs = undefined;
    LA.graphs_buffer = undefined;
    LA.graph_colors = {
        'ch1': '#dc1809',
        'ch2': '#e6890c',
        'ch3': '#fed730',
        'ch4': '#00ae73',
        'ch5': '#0760be',
        'ch6': '#846167',
        'ch7': '#6b6f61',
        'ch8': '#ebf1e7'
    };

    LA.trigger_color = '#75cede'

    LA.lastData = undefined;
    LA.lastDataRepacked = undefined;
    LA.decodedData = {}
    LA.region_view_moving = false;
    LA.region_samples = {start:0 , end:0}

    LA.initGraph = function() {
        if (LA.graphs === undefined) {
            LA.graphs = {};
            LA.graphs.elem = $('<div class="plot" id="plot_main"/>').css($('#graph_grid').css(['height', 'width'])).appendTo('#graphs');
            LA.graphs.plot = $.plot(LA.graphs.elem, [], {
                name: "ch1",
                series: {
                    shadowSize: 0, // Drawing is faster without shadows
                },
                yaxis: {
                    min: 0,
                    max: 9
                },
                xaxis: {
                    min: 0,
                    max: 0
                },
                grid: {
                    show: false
                },
                colors: [
                ]
            });
        }
    }

    LA.initGraphBuffer = function() {
        if (LA.graphs_buffer === undefined) {
            LA.graphs_buffer = {};
            LA.graphs_buffer.elem = $('<div class="plot" id="plot_buff"/>').css($('#graphs_buffer').css(['height', 'width'])).appendTo('#graphs_buffer');
            LA.graphs_buffer.plot = $.plot(LA.graphs_buffer.elem, [], {
                name: "ch1",
                series: {
                    shadowSize: 0, // Drawing is faster without shadows
                    lineWidth: 1.5,
                    lines: {
                        lineWidth: 1.5
                    }
                },
                yaxis: {
                    min: 0,
                    max: 100
                },
                xaxis: {
                    min: 0
                },
                grid: {
                    show: false
                },
                colors: [
                ]
            });
        }
    }

    LA.resizePlots = function(){

        var g_h = $('#main').height() - $('#buffer').height() - $('#info').height()
        $('#graphs_holder').height(g_h)

        if (LA.graphs === undefined){
            LA.initGraph()
        }

        var canvas_width = $('#graphs').width() - 2;
        var canvas_height = $("#graphs").height() - 2;
        var ctx = $('#graph_grid')[0].getContext('2d');
        // Set canvas size
        ctx.canvas.width = canvas_width;
        ctx.canvas.height = canvas_height;

        $('#plot_main').css($('#graph_grid').css(['height', 'width']))
        LA.graphs.plot.resize();
        LA.graphs.plot.setupGrid();
        LA.graphs.plot.draw();

        if (LA.graphs_buffer === undefined){
            LA.initGraphBuffer()
        }
        $('#plot_buff').css($('#graphs_buffer').css(['height', 'width']))
        LA.graphs_buffer.plot.resize();
        LA.graphs_buffer.plot.setupGrid();
        LA.graphs_buffer.plot.draw();

        LA.updatePositionBufferViewport()
    }

    LA.resizeAxisGraphBufferFromCount = function(x_count) {
        if (LA.graphs_buffer === undefined) {
            LA.initGraphBuffer()
        }
        var plot = LA.getPlotBuffer()
        if (plot !== undefined){
            var axes = plot.getAxes();
            axes.xaxis.options.min = 0
            axes.xaxis.options.max = x_count
            plot.setupGrid();
            plot.draw();
        }
    }

    LA.setupDataToBufferGraph = function() {

        var preTriggerCount = CLIENT.getValue("LA_PRE_TRIGGER_SAMPLES")
        if (preTriggerCount === undefined) return
        if (LA.lastDataRepacked === undefined) return

        var channel_list = ['ch1', 'ch2', 'ch3', 'ch4', 'ch5', 'ch6', 'ch7', 'ch8'];
        var pointArr = [];
        var colorsArr = [];
        var sig_h = 5; // px

        for (var sig_name in LA.lastDataRepacked) {

            var index_channel = channel_list.indexOf(sig_name);

            // Ignore empty signals
            if (LA.lastDataRepacked[sig_name].size == 0)
                continue;

            var points = [];
            var color = LA.graph_colors[sig_name];
            var start_point = 0;
            var offset = 11;
            for (var u = 0; u < LA.lastDataRepacked[sig_name].value.length; u += 2) {
                var start_x = start_point;
                var start_y = (LA.lastDataRepacked[sig_name].value[u + 1] === 1 ? sig_h : 0);
                start_y += (index_channel + 1) * offset;
                points.push([start_x, start_y]);

                start_point += LA.lastDataRepacked[sig_name].value[u];

                var end_x = start_point;
                var end_y = start_y;
                points.push([end_x, end_y]);
            }
            pointArr.push(points);
            colorsArr.push(color);

        }


        // Added trigger v line
        if (pointArr.length > 0 && preTriggerCount !== 0){
            pointArr.push([[preTriggerCount + 1,0],[preTriggerCount + 1,100]])
            colorsArr.push(LA.trigger_color)
        }

        if (LA.graphs_buffer) {
            LA.graphs_buffer.elem.show();
            LA.graphs_buffer.plot.setColors(colorsArr);
            LA.graphs_buffer.plot.resize();
            LA.graphs_buffer.plot.setupGrid();
            LA.graphs_buffer.plot.setData(pointArr);
            LA.graphs_buffer.plot.draw();
        }
    }

    LA.setupDataToGraph = function() {

        var preTriggerCount = CLIENT.getValue("LA_PRE_TRIGGER_SAMPLES")
        if (preTriggerCount === undefined) return
        if (LA.lastDataRepacked === undefined) return

        var channel_list = ['ch1', 'ch2', 'ch3', 'ch4', 'ch5', 'ch6', 'ch7', 'ch8'];
        var pointArr = [];
        var colorsArr = [];
        var sig_h = 0.5;

        for (var sig_name in LA.lastDataRepacked) {

            var index_channel = channel_list.indexOf(sig_name);

            // Ignore empty signals
            if (LA.lastDataRepacked[sig_name].size == 0)
                continue;

            if (CLIENT.getValue('LA_DIN_' + (index_channel + 1)) !== true)
                continue;

            var points = [];
            var color = LA.graph_colors[sig_name];
            var start_point = 0;
            var offset = CLIENT.getValue("LA_DIN_" +(index_channel+1)+ "_POS")
            for (var u = 0; u < LA.lastDataRepacked[sig_name].value.length; u += 2) {
                var start_x = start_point;
                var start_y = (LA.lastDataRepacked[sig_name].value[u + 1] === 1 ? sig_h : 0);
                start_y += offset;
                points.push([start_x, start_y]);

                start_point += LA.lastDataRepacked[sig_name].value[u];

                var end_x = start_point;
                var end_y = start_y;
                points.push([end_x, end_y]);
            }
            pointArr.push(points);
            colorsArr.push(color);
        }


        // Added trigger v line
        if (pointArr.length > 0 && preTriggerCount !== 0){
            pointArr.push([[preTriggerCount + 1,0],[preTriggerCount + 1,9]])
            colorsArr.push(LA.trigger_color)
        }

        if (LA.graphs) {
            LA.graphs.elem.show();
            LA.graphs.plot.setColors(colorsArr);
            LA.graphs.plot.resize();
            LA.graphs.plot.setupGrid();
            LA.graphs.plot.setData(pointArr);
            LA.graphs.plot.draw();
        }
    }

    LA.updatePositionBufferViewportOnly = function(){
        var samples =  CLIENT.getValue('LA_TOTAL_SAMPLES')
        var pos = CLIENT.getValue('LA_VIEW_PORT_POS')
        var scale = CLIENT.getValue('LA_SCALE')
        var samplerate = CLIENT.getValue('LA_CUR_FREQ')
        var graph_width = $('#graph_grid').width()

        if (samples === undefined || samples === 0){
            $("#buffer_time_region").hide()
            return
        }

        if (pos !== undefined && samplerate !== undefined && scale !== undefined && samples !== undefined){
            var timePerDevInMs = (((graph_width / scale) / samplerate) * 1000.0); // Main view
            // var totalTime = samples / samplerate * 1000.0
            var totalWidthBuffer = $('#graphs_buffer').width()
            var viewPortWidth = timePerDevInMs * 10 // totalTime * totalWidth
            $("#buffer_time_region").width(viewPortWidth).height($('#buffer').height())
            $("#buffer_time_region").show()

            var centerPosX = totalWidthBuffer * pos
            if (centerPosX - viewPortWidth/2.0 < 0 && centerPosX + viewPortWidth/2.0 > totalWidthBuffer){
                centerPosX = totalWidthBuffer * 0.5
                $('#buffer_time_region').draggable('disable');
            }else{
                centerPosX = (centerPosX - viewPortWidth / 2.0 < 0 ? viewPortWidth / 2: centerPosX)
                centerPosX = (centerPosX + viewPortWidth / 2.0 > totalWidthBuffer ? totalWidthBuffer - viewPortWidth / 2.0: centerPosX)
                $('#buffer_time_region').draggable('enable');
            }
            var leftPos = centerPosX - viewPortWidth / 2.0
            $("#buffer_time_region").css({left: leftPos});
        }
    }

    LA.updatePositionBufferViewport = function(){
        var samples =  CLIENT.getValue('LA_TOTAL_SAMPLES')
        var pos = CLIENT.getValue('LA_VIEW_PORT_POS')
        var scale = CLIENT.getValue('LA_SCALE')
        var samplerate = CLIENT.getValue('LA_CUR_FREQ')
        var graph_width = $('#graph_grid').width()

        if (samples === undefined || samples === 0){
            $("#buffer_time_region").hide()
            return
        }

        if (pos !== undefined && samplerate !== undefined && scale !== undefined && samples !== undefined){
            var timePerDevInMs = (((graph_width / scale) / samplerate) * 1000.0); // Main view
            // var totalTime = samples / samplerate * 1000.0
            var totalWidthBuffer = $('#graphs_buffer').width()
            var viewPortWidth = timePerDevInMs * 10 // totalTime * totalWidth
            $("#buffer_time_region").width(viewPortWidth).height($('#buffer').height())
            $("#buffer_time_region").show()

            var centerPosX = totalWidthBuffer * pos
            if (centerPosX - viewPortWidth/2.0 < 0 && centerPosX + viewPortWidth/2.0 > totalWidthBuffer){
                centerPosX = totalWidthBuffer * 0.5
                $('#buffer_time_region').draggable('disable');
            }else{
                centerPosX = (centerPosX - viewPortWidth / 2.0 < 0 ? viewPortWidth / 2: centerPosX)
                centerPosX = (centerPosX + viewPortWidth / 2.0 > totalWidthBuffer ? totalWidthBuffer - viewPortWidth / 2.0: centerPosX)
                $('#buffer_time_region').draggable('enable');
            }
            var leftPos = centerPosX - viewPortWidth / 2.0
            var rightPos = centerPosX + viewPortWidth / 2.0
            $("#buffer_time_region").css({left: leftPos});

            var l = Math.floor(leftPos / graph_width * samples)
            var r = Math.ceil(rightPos / graph_width * samples)
            LA.region_samples = {start: l , end:r}

            LA.updateMainView()
            LA.updateTimeScale()
            LA.updateXInfo()
            if ((centerPosX / totalWidthBuffer) !== pos){
                CLIENT.parametersCache['LA_VIEW_PORT_POS'] = {value: centerPosX / totalWidthBuffer}
                CLIENT.sendParameters()
            }
        }
    }

    LA.updateMainView = function() {
        if (LA.graphs === undefined) {
            LA.initGraph()
        }

        var plot = LA.getPlot()
        if (plot !== undefined){
            var axes = plot.getAxes();
            var min = LA.region_samples.start
            var max = LA.region_samples.end
            axes.xaxis.options.min = min
            axes.xaxis.options.max = max
            plot.setupGrid();
            plot.draw();
            LA.drawAllSeries()
            LOGGER.scrollDataArea()
        }
    }

    LA.moveViewPort = function(dir){
        var pos = CLIENT.getValue('LA_VIEW_PORT_POS')
        var viewWidth = $('#buffer_time_region').width()
        var totalWidth = $('#graphs_buffer').width()
        var moveDelta = (viewWidth / ((LA.state.fine == false) ? 2 : 8)) / totalWidth * (dir == '+' ? 1 : -1)
        const clamp = (value, min, max) => {
            if (value < min) return min;
            if (value > max) return max;
            return value;
        }
        CLIENT.params.orig['LA_VIEW_PORT_POS'] = {value: clamp(pos + moveDelta,0,1)}
        LA.updatePositionBufferViewport()
    }

    // Draws the grid on the lowest canvas layer
    LA.drawGraphGrid = function() {

        var ctx = $('#graph_grid')[0].getContext('2d');
        var canvas_width = $('#graph_grid').width()
        var canvas_height = $('#graph_grid').height()
        var x_offset = 0;
        var y_offset = 0;

        // Set draw options
        ctx.beginPath();
        ctx.lineWidth = 1;
        ctx.strokeStyle = '#5d5d5c';

        // Draw ticks
        for (var i = 1; i < 45; i++) {
            x_offset = x_offset + (canvas_width / 45);
            y_offset = y_offset + (canvas_height / 45);

            ctx.moveTo(x_offset, canvas_height - 3);
            ctx.lineTo(x_offset, canvas_height);

            ctx.moveTo(0, y_offset);
            ctx.lineTo(3, y_offset);
        }

        // Draw lines
        x_offset = 0;
        y_offset = 0;

        for (var i = 1; i < 9; i++) {
            x_offset = x_offset + (canvas_height / 9);
            ctx.moveTo(0, x_offset);
            ctx.lineTo(canvas_width, x_offset);
        }
        ctx.stroke();
        for (var i = 1; i < 10; i++) {
            y_offset = y_offset + (canvas_width / 10);
            ctx.moveTo(y_offset, 0);
            ctx.lineTo(y_offset, canvas_height);
        }
        ctx.stroke();
    };

    LA.getPlot = function() {

        if (LA.graphs && LA.graphs.elem) {
            var plot = LA.graphs.plot;
            return plot;
        }
        return undefined;
    };

    LA.getPlotBuffer = function() {

        if (LA.graphs_buffer && LA.graphs_buffer.elem) {
            var plot = LA.graphs_buffer.plot;
            return plot;
        }
        return undefined;
    };

    // Changes X zoom/scale for all signals
    LA.changeXZoom = function(direction, curr_scale, send_changes) {
        // Calculate time per division
        var samplerate = CLIENT.getValue("LA_CUR_FREQ")
        var scale = CLIENT.getValue("LA_SCALE")

        if (samplerate !== undefined && scale !== undefined){
            var newScaleMul = 1;

            if (direction == '+') {
                newScaleMul = scale * ((LA.state.fine == false) ? 2 : 1.1);
                if (newScaleMul >= 1000) newScaleMul = 1000
            } else if (direction == '-') {
                newScaleMul = scale / ((LA.state.fine == false) ? 2 : 1.1);
                if (newScaleMul < 0.005) newScaleMul = 0.005
            } else if (direction == '1') {
                newScaleMul = 1;
            }
            if (newScaleMul !== CLIENT.getValue("LA_SCALE")){
                CLIENT.parametersCache["LA_SCALE"] = { value : newScaleMul }
                CLIENT.sendParameters();
                return true;
            }
        }
        return false;
    };

    LA.getPoltRect = function(){
        var plot = LA.getPlot();
        if (!plot) {
            return {l:0,t:0,w:0,h:0};
        }
        var gPosition = $('#graph_grid').offset();
        var gLeft = gPosition.left;
        var gTop = gPosition.top;
        var gWidth = $('#graph_grid').width();
        var gHeight = $('#graph_grid').height();
        var plotOffset = plot.getPlotOffset();

        gLeft += plotOffset.left
        gTop += plotOffset.top
        gWidth = gWidth - plotOffset.left - plotOffset.right
        gHeight = gHeight - plotOffset.top - plotOffset.bottom
        return {l:gLeft,t:gTop,w:gWidth,h:gHeight}
    }

    LA.boundCursor = function(rect,pos){
        if (pos.x < rect.l){
            pos.x = rect.l
        }
        if (pos.x > (rect.l + rect.w)){
            pos.x = (rect.l + rect.w)
        }
        if (pos.y < rect.t){
            pos.y = rect.t
        }
        if (pos.y > (rect.t + rect.h)){
            pos.y = (rect.t + rect.h)
        }
        return pos
    }

    LA.time_zoom = function(ev, offsetPx, byMouseWheel) {
        LA.changeXZoom(ev);
        LA.guiHandler();
    }

    // channel - 1..8
    LA.calculateOffset = function(channel){
        if (LA.graphs === undefined) {
            LA.initGraph()
        }
        var pos = CLIENT.getValue("LA_DIN_" + channel + "_POS")
        if (pos !== undefined){
            return pos
        }
        return -1
    }

    LA.calculateXBySamples = function(sample){
        var min = LA.region_samples.start
        var max = LA.region_samples.end
        var grid = $('#graph_grid');
        var sample_per_px = (max - min) / grid.width();
        return (sample - min) / sample_per_px;
    }

    LA.calculateSamplesToPixels = function(samples){
        var min = LA.region_samples.start
        var max = LA.region_samples.end
        var grid = $('#graph_grid');
        var sample_per_px = (max - min) / grid.width();
        if (sample_per_px == 0) return 0;
        return samples / sample_per_px;
    }

    LA.calculateScale = function(){
        var min = LA.region_samples.start
        var max = LA.region_samples.end
        var grid = $('#graph_grid');
        if ((max - min) == 0) return 0;
        return grid.width() / (max - min);
    }

    LA.calculatePixelInbound = function(point, size){
        if (point !== undefined && point.x !== undefined){
            var x = point.x
            var grid = $('#graph_grid')
            if ((x + size) < 0) return -1
            if ((x + size)  >= 0 && (x - size) <= grid.width()) return 0
            if ((x - size)  > grid.width()) return 1
        }else{
            console.error("Point not defined")
        }
        return 0
    }


    LA.drawSeries = function(idx, plot, canvascontext) {

        var data = LA.decodedData[idx]

        if (data.values !== undefined && data.name !== undefined){
            for (var line in data.values) {
                let sel_idx = -1
                for(let ixd = 1; idx <= 8; idx++){
                    if (CLIENT.getValue('LA_DIN_NAME_'+idx) == line){
                        sel_idx = idx
                        break
                    }
                }
                if (sel_idx == -1) continue;
                var enable_din = CLIENT.getValue('LA_DIN_' + sel_idx)
                var offset = LA.calculateOffset(line)
                if (offset == -1) continue;
                if (enable_din != true) continue;
                if (data.name == 'CAN'){
                    CAN.drawDecoded(plot, canvascontext, offset, data.values[line]);
                }else if (data.name == 'I2C'){
                    I2C.drawDecoded(plot, canvascontext, offset, data.values[line]);
                }else if (data.name == 'SPI'){
                    SPI.drawDecoded(plot, canvascontext, offset, data.values[line]);
                }else if (data.name == 'UART'){
                    UART.drawDecoded(plot, canvascontext, offset, data.values[line]);
                }
            }
        }
    }

    LA.drawAllSeries = function(){
        if (LA.graphs === undefined) {
            LA.initGraph()
        }
        var plot = LA.getPlot()
        if (plot !== undefined){
            plot.draw()
            for(var ch = 1; ch <= 4; ch++){
                var enable = CLIENT.getValue('DECODER_ENABLED_' + ch)
                if (enable){
                    LA.drawSeries(ch, plot, plot.getCanvas().getContext("2d"))
                }
            }
        }
    }

}(window.LA = window.LA || {}, jQuery));
