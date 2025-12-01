/*
 * Red Pitaya Spectrum Analizator client
 *
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function(UI_GRAPH, $, undefined) {
    // Origin Y min and max. Need for reset
    UI_GRAPH.ymax = 20.0;
    UI_GRAPH.ymin = -130.0;
    UI_GRAPH.lock_limit_change = 0;

    UI_GRAPH.move_mode = undefined;
    UI_GRAPH.rect_mode = undefined;
    UI_GRAPH.rect_mode_last = undefined;
    UI_GRAPH.x_axis_mode = 0;
    UI_GRAPH.y_axis_mode = 0;
    UI_GRAPH.x_axis_min = undefined;
    UI_GRAPH.x_axis_max = undefined;
    UI_GRAPH.x_axis_min_full = undefined;
    UI_GRAPH.x_axis_max_full = undefined;
    UI_GRAPH.zoom_used_x = false;
    UI_GRAPH.zoom_used_y = false;

    UI_GRAPH.freq_unit = ['Hz', 'kHz', 'MHz'];
    UI_GRAPH.freq_unit_range = 1

    UI_GRAPH.graph_colors = {
        'ch1_view': '#f3ec1a',
        'ch2_view': '#31b44b',
        'ch3_view': '#ee3739',
        'ch4_view': '#3af7f7',
        'ch1_view_min': '#787407',
        'ch2_view_min': '#1b642a',
        'ch3_view_min': '#750a0c',
        'ch4_view_min': '#057a7a',

        'ch1_view_max': '#787407',
        'ch2_view_max': '#1b642a',
        'ch3_view_max': '#750a0c',
        'ch4_view_max': '#057a7a',
    };

    // UI_GRAPH.minMaxChange = undefined;

    // Voltage scale steps in volts
    UI_GRAPH.voltage_steps = [
        // dBm
        1 / 100, 1 / 20, 1 / 10, 2 / 10, 5 / 10, 1, 2, 5, 10, 20, 50, 100
    ];

    UI_GRAPH.getPoltRect = function(){
        if (!SPEC || !SPEC.getPlot) return
        var plot = SPEC.getPlot();
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

    UI_GRAPH.getPlotLimits = function(){
        if (!SPEC || !SPEC.getPlot) return 'parent'
        var plot = SPEC.getPlot();
        if (!plot) {
            return 'parent';
        }
        var gPosition = $('#graph_grid').offset();
        var gLeft = gPosition.left;
        var gTop = gPosition.top;
        var gWidth = $('#graph_grid').width();
        var gHeight = $('#graph_grid').height();
        var plotOffset = plot.getPlotOffset();

        gLeft += plotOffset.left
        gTop += plotOffset.top
        let z = [gLeft - 2, gTop - 2 , gWidth + plotOffset.left - plotOffset.right - 3, gHeight + gPosition.top - plotOffset.bottom - 2]
        return z
    }

    UI_GRAPH.boundCursor = function(rect,pos){
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

     // Touch events
    $(document).on('mousedown', '.plot', function(ev) {
        ev.preventDefault();
        if (!UI_GRAPH.zoom_used_x && !UI_GRAPH.zoom_used_y){
            var rect = UI_GRAPH.getPoltRect()
            var newPos = UI_GRAPH.boundCursor(rect,{ x: ev.clientX, y: ev.clientY })
            UI_GRAPH.rect_mode = newPos;
            UI_GRAPH.rect_mode_last  = newPos;
            return;
        }

        if (!UI_GRAPH.move_mode) {
            var rect = UI_GRAPH.getPoltRect()
            var newPos = UI_GRAPH.boundCursor(rect,{ x: ev.clientX, y: ev.clientY })
            UI_GRAPH.move_mode = newPos;
        }
    });

    $(document).on('mousemove', '.plot', function(ev) {
        ev.preventDefault();

        if (!UI_GRAPH.move_mode) {
            return;
        }

        var rect = UI_GRAPH.getPoltRect()
        var newPos = UI_GRAPH.boundCursor(rect,{ x: ev.clientX, y: ev.clientY })

        var x = UI_GRAPH.move_mode.x - newPos.x;
        var y = UI_GRAPH.move_mode.y - newPos.y;
        var options = SPEC.graphs.plot.getOptions();
        var range_x   = options.xaxes[0].max - options.xaxes[0].min;
        var range_y   = options.yaxes[0].max - options.yaxes[0].min;
        UI_GRAPH.move_mode  = newPos;
        UI_GRAPH.changeX(x * range_x / $(this).width(), x , $(this).width());
        UI_GRAPH.changeY(y * range_y / $(this).height());
    });

    $(document).on('mousemove',  function(ev) {
        ev.preventDefault();

        var rect = UI_GRAPH.getPoltRect()
        var newPos = UI_GRAPH.boundCursor(rect,{ x: ev.clientX, y: ev.clientY })

        if (!UI_GRAPH.zoom_used_x && !UI_GRAPH.zoom_used_y && UI_GRAPH.rect_mode != undefined){

            var x = Math.min(UI_GRAPH.rect_mode.x,newPos.x);
            var y = Math.min(UI_GRAPH.rect_mode.y,newPos.y);
            var w = Math.max(UI_GRAPH.rect_mode.x,newPos.x) - x;
            var h = Math.max(UI_GRAPH.rect_mode.y,newPos.y) - y;

            UI_GRAPH.rect_mode_last  = newPos;

            $("#cur_rectangle").show();
            $("#cur_rectangle").css("left",x)
            $("#cur_rectangle").css("top",y)
            $("#cur_rectangle").css("width",w)
            $("#cur_rectangle").css("height",h)
            return;
        }
    });

    $(document).on('mouseup', '.plot', function(ev) {
        ev.preventDefault();
        UI_GRAPH.move_mode = undefined;
        if (UI_GRAPH.rect_mode && UI_GRAPH.rect_mode_last){
            var rect = UI_GRAPH.getPoltRect()
            var p1 =  {x:UI_GRAPH.rect_mode.x - rect.l,y:UI_GRAPH.rect_mode.y - rect.t}
            var p2 =  {x:UI_GRAPH.rect_mode_last.x - rect.l,y:UI_GRAPH.rect_mode_last.y - rect.t}
            UI_GRAPH.setMouseZoom(p1,p2,rect)
        }
        UI_GRAPH.rect_mode_last = undefined;
        UI_GRAPH.rect_mode = undefined;
        $("#cur_rectangle").hide();
    });

    $(document).on('mouseup', function(ev) {
        ev.preventDefault();
        UI_GRAPH.rect_mode_last = undefined;
        UI_GRAPH.rect_mode = undefined;
        $("#cur_rectangle").hide();
    });

    UI_GRAPH.setMouseZoom = function(p1,p2,rect) {
        var plot = SPEC.getPlot();
        if (!plot) {
            return;
        }
        var min_x = Math.min(p1.x, p2.x);
        var max_x = Math.max(p1.x, p2.x);
        var min_y = Math.min(p1.y, p2.y);
        var max_y = Math.max(p1.y, p2.y);
        var options = plot.getOptions();
        var axes = SPEC.graphs.plot.getAxes();
        new_x_axis_min = axes.xaxis.c2p(min_x)
        new_x_axis_max = axes.xaxis.c2p(max_x)
        new_y_axis_min = axes.yaxis.c2p(min_y)
        new_y_axis_max = axes.yaxis.c2p(max_y)

        console.log("new_y_axis_min " + new_y_axis_min + " new_y_axis_max " +new_y_axis_max )
        console.log("new_x_axis_min " + new_x_axis_min + " new_x_axis_max " +new_x_axis_max )

        options.xaxes[0].min = new_x_axis_min;
        options.xaxes[0].max = new_x_axis_max;
        options.yaxes[0].min = new_y_axis_max
        options.yaxes[0].max = new_y_axis_min

        UI_GRAPH.lockUpdateYLimit();
        UI_GRAPH.zoom_used_x = true;
        UI_GRAPH.zoom_used_y = true;
        plot.setupGrid();
        plot.draw();
        SPEC.updateWaterfallWidth();
        SPEC.updateXInfo();
        SPEC.updateYInfo();
    }

     // Changes Y zoom/scale for the selected signal
    UI_GRAPH.changeYZoom = function(direction) {
        if (!(SPEC.graphs && SPEC.graphs.elem))
            return null;

        if (UI_GRAPH.y_axis_mode === 1) return;
        if (direction == undefined) return;

        var options = SPEC.graphs.plot.getOptions();
        var axes = SPEC.graphs.plot.getAxes();
        var curr_scale = axes.yaxis.tickSize;
        if ((curr_scale >= UI_GRAPH.voltage_steps[UI_GRAPH.voltage_steps.length - 1] && direction == '-') ||
            (curr_scale <= UI_GRAPH.voltage_steps[0] && direction == '+')){
            if (direction == '-') UI_GRAPH.zoom_used_y = false;
            return null;
        }
        UI_GRAPH.zoom_used_y = true;

        var range = axes.yaxis.max - axes.yaxis.min;
        var delta = direction == '+' ? 1 : -1
        options.yaxes[0].min = Math.max(axes.yaxis.min + delta * range * 0.1, UI_GRAPH.ymin)
        options.yaxes[0].max = Math.min(axes.yaxis.max - delta * range * 0.1 ,UI_GRAPH.ymax)
        if (options.yaxes[0].min == UI_GRAPH.ymin && options.yaxes[0].max == UI_GRAPH.ymax){
            UI_GRAPH.zoom_used_y = false;
        }
        SPEC.graphs.plot.setupGrid();
        SPEC.graphs.plot.draw();
        SPEC.updateWaterfallWidth();
        SPEC.updateYInfo();
    };

     // Changes Y zoom/scale for the selected signal
     UI_GRAPH.changeY = function(value) {
        if (!(SPEC.graphs && SPEC.graphs.elem))
            return null;

        if (UI_GRAPH.y_axis_mode === 1) return;


        var options = SPEC.graphs.plot.getOptions();
        if ( options.yaxes[0].min - value < UI_GRAPH.ymin) {
            return;
        }

        if ( options.yaxes[0].max - value > UI_GRAPH.ymax) {
            return;
        }
        var cur_min = options.yaxes[0].min - value
        var cur_max = options.yaxes[0].max - value

        options.yaxes[0].min = cur_min;
        options.yaxes[0].max = cur_max;

        SPEC.graphs.plot.setupGrid();
        SPEC.graphs.plot.draw();
        SPEC.updateWaterfallWidth();
        SPEC.updateYInfo();
    };

    UI_GRAPH.changeX = function(value) {
        if (!(SPEC.graphs && SPEC.graphs.elem))
            return null;


        var options = SPEC.graphs.plot.getOptions();

        if ( options.xaxes[0].min + value < CLIENT.getValue('xmin')) {
            value  = CLIENT.getValue('xmin') - options.xaxes[0].min
        }
        else
        if ( options.xaxes[0].max + value > CLIENT.getValue('xmax')) {
            value  = CLIENT.getValue('xmax') - options.xaxes[0].max
        }
        var cur_min = options.xaxes[0].min + value
        var cur_max = options.xaxes[0].max + value

        options.xaxes[0].min = cur_min;
        options.xaxes[0].max = cur_max;
        SPEC.graphs.plot.setupGrid();
        SPEC.graphs.plot.draw();
        SPEC.updateWaterfallWidth();
        SPEC.updateXInfo();
    };


    // Changes X zoom/scale for all signals
    UI_GRAPH.changeXZoom = function(direction) {
        if (!(SPEC.graphs && SPEC.graphs.elem))
            return null;


        var options = SPEC.graphs.plot.getOptions();
        var axes = SPEC.graphs.plot.getAxes();
        var curr_scale = axes.xaxis.scale;

        if ((curr_scale >= 1 && direction == '+') || (curr_scale <= 0 && direction == '-')) {
            if (direction == '-') UI_GRAPH.zoom_used_x = false;
            return null;
        }
        UI_GRAPH.zoom_used_x = true;
        var range = axes.xaxis.max - axes.xaxis.min;
        var delta = direction == '+' ? 1 : -1
        let xmin = CLIENT.getValue("xmin")
        let xmax = CLIENT.getValue("xmax")
        options.xaxes[0].min = Math.max(xmin, axes.xaxis.min + delta * range * 0.1);
        options.xaxes[0].max = Math.min(xmax, axes.xaxis.max - delta * range * 0.1);
        if (options.xaxes[0].min == xmin && options.xaxes[0].max == xmax){
            UI_GRAPH.zoom_used_x = false;
        }
        SPEC.graphs.plot.setupGrid();
        SPEC.graphs.plot.draw();
        SPEC.updateWaterfallWidth();
        SPEC.updateXInfo();
    };

    UI_GRAPH.resetZoom = function() {
        if (!(SPEC.graphs && SPEC.graphs.elem))
            return;
        let xmin = CLIENT.getValue('xmin')
        let xmax = CLIENT.getValue('xmax')
        if ((xmin === undefined) || (xmax === undefined)) return false
        // UI_GRAPH.updateYAxis(); // Reset min max for Y-axis
        var plot = SPEC.graphs.plot;
        var curr_options = plot.getOptions();
        curr_options.xaxes[0].min = xmin;
        curr_options.xaxes[0].max = xmax;
        curr_options.yaxes[0].min = UI_GRAPH.ymin;
        curr_options.yaxes[0].max = UI_GRAPH.ymax;

        plot.setupGrid();
        plot.draw();
        SPEC.updateWaterfallWidth();
        SPEC.updateYInfo();
        SPEC.updateXInfo();
        UI_GRAPH.zoom_used_x = false;
        UI_GRAPH.zoom_used_y = false;
    };

    UI_GRAPH.updateZoom = function() {
        if (SPEC.graphs && SPEC.graphs.elem) {
            let xmin = CLIENT.getValue('xmin')
            let xmax = CLIENT.getValue('xmax')
            if ((xmin === undefined) || (xmax === undefined)) return false

            var plot = SPEC.graphs.plot;
            var axes = plot.getAxes();
            var options = plot.getOptions();

            options.xaxes[0].min = UI_GRAPH.zoom_used_x ? UI_GRAPH.x_axis_min : xmin;
            options.xaxes[0].max = UI_GRAPH.zoom_used_x ? UI_GRAPH.x_axis_max : xmax;
            options.yaxes[0].min = axes.yaxis.min;
            options.yaxes[0].max = axes.yaxis.max;
            plot.setupGrid();
            plot.draw();
            SPEC.updateWaterfallWidth();
            SPEC.updateYInfo();
            SPEC.updateXInfo();
            return true
        }
        return false
    };

    UI_GRAPH.lockUpdateYLimit = function(){
        UI_GRAPH.lock_limit_change = 3;
    }

    UI_GRAPH.unlockUpdateYLimit = function(){
        UI_GRAPH.lock_limit_change--;
    }

    UI_GRAPH.changeYAxisMode = function(mode){
        UI_GRAPH.lockUpdateYLimit()
        var mode_value = 0;
        SPEC.config.y_axis_mode = mode;
        if (mode === "dbm") {
            mode_value = 0;
            $('.power-label').text('Amplitude [dBm]');
        }
        if (mode === "v") {
            mode_value = 1;
            $('.power-label').text('Amplitude [V]');
        }
        if (mode === "dbu") {
            mode_value = 2;
            $('.power-label').text('Amplitude [dBu]');
        }
        if (mode === "dbV") {
            mode_value = 3;
            $('.power-label').text('Amplitude [dBV]');
        }
        if (mode === "dbuV") {
            mode_value = 4;
            $('.power-label').text('Amplitude [dBµV]');
        }
        if (mode === "mW") {
            mode_value = 5;
            $('.power-label').text('Amplitude [mW]');
        }
        if (mode === "dBW") {
            mode_value = 6;
            $('.power-label').text('Amplitude [dBW]');
        }
        CLIENT.sendParametersEx({'y_axis_mode': {value : mode_value}});
        UI_GRAPH.resetZoom();
        SPEC.minMaxController.resetAll()
    }

    UI_GRAPH.updateYAxis = function() {
        UI_GRAPH.lockUpdateYLimit();
        var mode = $("#BDM_DBU_FUNC option:selected").val();
        if (mode === "dbm") {
            UI_GRAPH.ymax = 20.0;
            UI_GRAPH.ymin = -130.0;
        }

        if (mode === "dbu") {
            UI_GRAPH.ymax = 10.0;
            UI_GRAPH.ymin = -140.0;
        }

        if (mode === "dbuV") {
            UI_GRAPH.ymax = 130.0;
            UI_GRAPH.ymin = -20.0;
        }

        if (mode === "dbV") {
            UI_GRAPH.ymax = 10.0;
            UI_GRAPH.ymin = -140.0;
        }

        if (mode === "v") {
            UI_GRAPH.ymax = 1.5;
            UI_GRAPH.ymin = -0.1;
        }

        if (mode === "mW") {
            UI_GRAPH.ymax = 10.0;
            UI_GRAPH.ymin = -1;
        }

        if (mode === "dBW") {
            UI_GRAPH.ymax = 10.0;
            UI_GRAPH.ymin = -160.0;
        }
    }

    UI_GRAPH.autoScale = function(){
         var plot = SPEC.getPlot();
        if (!(plot)) {
            return
        }
        UI_GRAPH.updateYAxis()
        let min = SPEC.minMaxController.getMinOfVisible()
        let max = SPEC.minMaxController.getMaxOfVisible()
        if (min == undefined || max == undefined) return
        const margin = (max - min) * 0.1;
        min = min - margin;
        max = max + margin;
        if (UI_GRAPH.ymin < min && UI_GRAPH.ymax > max){
            min = UI_GRAPH.ymin
            max = UI_GRAPH.ymax
        }else{
            UI_GRAPH.ymin = min
            UI_GRAPH.ymax = max
        }

        var options = plot.getOptions();
        options.yaxes[0].min = min
        options.yaxes[0].max = max
        SPEC.processSignals(SPEC.latest_signal)
    }

    UI_GRAPH.convertLog = function(v){
        var a = UI_GRAPH.x_axis_min;
        var b = UI_GRAPH.x_axis_max;
        v = v > b ? b : v
        var x = Math.log10(b/a)/(b-a);
        var y = b / Math.pow(10,x * b);
        v =  y * Math.pow(10, x * v);
        return v;
    }

    UI_GRAPH.convertXLog = function(v,min,max){
        if (v == 0) return 0;
        var a = Math.max(1,min * 0.8);
        var b = max;
        var x = Math.log10(b/a)/(b-a);
        v =  Math.log10(v/b)/x + b;
        return v;
    }

    UI_GRAPH.reverseXLog = function(v, min, max) {
        if (v == 0) return 0;

        var a = Math.max(1, min * 0.8);
        var b = max;
        var x = Math.log10(b/a)/(b-a);

        var result = Math.pow(10, (v - b) * x) * b;
        return result;
    }

    UI_GRAPH.setXAxisMode = function(mode){
        UI_GRAPH.x_axis_mode = mode;
        if (!(SPEC.graphs && SPEC.graphs.elem))
            return ;
        UI_GRAPH.resetZoom();
        SPEC.graphs.plot.setupGrid();
    }

    UI_GRAPH.getLogValue = function(minVal, maxVal, steps, stepIndex) {
        if (minVal <= 0 || maxVal <= 0) {
            throw new Error("Error values");
        }

        const logMin = Math.log10(minVal);
        const logMax = Math.log10(maxVal);
        const stepSize = (logMax - logMin) / steps;

        const logVal = logMin + stepIndex * stepSize;
        return Math.pow(10, logVal);
    }

    function generateLogarithmicTicks(min,xMinNonZero, max) {
        const ticks = [];
        xMinNonZero = min != 0 ? min :xMinNonZero
        const min2 = Math.max(xMinNonZero,1)
        const startDecade = Math.floor(Math.log10(min2));
        const endDecade = Math.floor(Math.log10(max));

        // const multipliers = [1, 2, 3, 4 , 5, 6 , 7 , 8 ,9 ];
        const multipliers = [1, 2, 3,  5,  7 ];
        if (min == 0) ticks.push(0)
        for (let decade = startDecade; decade <= endDecade; decade++) {
            for (const mult of multipliers) {
                const freq = mult * Math.pow(10, decade);
                if (freq >= min2 && freq <= max) {
                    ticks.push(Math.round(freq));
                }
            }
        }
        if (!ticks.includes(Math.round((max)))) {
            ticks.push(Math.round(max));
        }

        return ticks.sort((a, b) => a - b);
    }

    function generateLogarithmicPower2Ticks(min, xMinNonZero, max) {
        const result = [];
        if (min == 0) result.push(0)
        const min2 = Math.max(min,1)
        let current = Math.pow(2, Math.ceil(Math.log2(min2)));
        if (current / 2 >= min2) {
            current = current / 2;
        }

        while (current <= max) {
            if (current >= xMinNonZero) {
                result.push(Math.round(current));
            }
            current *= 2
        }

        if (result[0] !== min2) result.unshift(Math.round(min2));
        if (result[result.length - 1] !== max) result.push(Math.round(max));

        return result;
    }

    function generatePowerOf10Scale(min,xMinNonZero, max) {
        const result = [];
        if (min == 0) result.push(0)
        const min2 = Math.max(xMinNonZero,1)
        let current = Math.pow(10, Math.ceil(Math.log10(min2)));
        if (current / 10 >= min2) {
            current = current / 10;
        }

        while (current <= max) {
            if (current >= min2) {
                result.push(Math.round(current));
            }
            current *= 2.5
        }

        if (result[0] !== min2) result.unshift(Math.round(min2));
        if (result[result.length - 1] !== max) result.push(Math.round(max));

        return result;
    }

    function generateRoundedLinearScale(min, max, targetSteps = 10) {
        if (min >= max) throw new Error("The minimum value must be less than the maximum");

        const range = max - min;
        let roughStep = range / targetSteps;

        const magnitude = Math.pow(10, Math.floor(Math.log10(roughStep)));
        const normalizedStep = roughStep / magnitude;

        let stepMultiplier;
        if (normalizedStep <= 1.5) {
            stepMultiplier = 1;
        } else if (normalizedStep <= 3) {
            stepMultiplier = 2;
        } else {
            stepMultiplier = 5;
        }

        const step = stepMultiplier * magnitude;

        const start = Math.floor(min / step) * step;
        const end = Math.ceil(max / step) * step;

        const result = [];
        for (let value = start; value <= end + Number.EPSILON; value += step) {
            if (value >= min - Number.EPSILON && value <= max + Number.EPSILON) {
                result.push(Math.round(value));
            }
        }

        if (result.length === 0 || result[0] > min) result.unshift(Math.round(min));
        if (result[result.length - 1] < max) result.push(Math.round(max));

        return result;
    }

    UI_GRAPH.getXAxisScale = function(axis) {
        var xMin = CLIENT.getValue("xmin")
        var xMinNonZero = axis.firstNonzero
        var xMax = CLIENT.getValue("xmax")
        var t = [];
        const xscale =  CLIENT.getValue("xAxisLogMode")
        if (xscale !== undefined && xMin !== undefined && xMax !== undefined){
            t = generateRoundedLinearScale(xMin,xMax)
            if (xscale === 1){
                t = generateLogarithmicTicks(xMin,xMinNonZero,xMax)
            }
            if (xscale === 2){
                t = generateLogarithmicPower2Ticks(xMin,xMinNonZero,xMax)
            }
            if (xscale === 3){
                t = generatePowerOf10Scale(xMin,xMinNonZero,xMax)
            }
        }
        return t
    }

    UI_GRAPH.funcxTickFormat = function(val, axis) {

        if (Math.abs(val) >= 0 && Math.abs(val) < 1000)
            return (val * 1).toFixed(0) + " ";
        else if (Math.abs(val) >= 1000 && Math.abs(val) < 1000000)
            return (val / 1000).toFixed(2) + "k";
        else if (Math.abs(val) >= 1000000)
            return (val / 1000000).toFixed(2) + "M";
    }


}(window.UI_GRAPH = window.UI_GRAPH || {}, jQuery));