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

    UI_GRAPH.minMaxChange = undefined;

    UI_GRAPH.time_steps = [
        // Hz
        1/20 , 1 / 10, 2 / 10, 5 / 10, 1, 2, 5, 10, 20, 50, 100, 200, 500
    ];

    // Voltage scale steps in volts
    UI_GRAPH.voltage_steps = [
        // dBm
        1 / 100, 1 / 20, 1 / 10, 2 / 10, 5 / 10, 1, 2, 5, 10, 20, 50, 100
    ];

    UI_GRAPH.getPoltRect = function(){
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
            console.log(newPos)
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
        UI_GRAPH.changeX(x * range_x / $(this).width());
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
        console.log("mouseup")
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
        console.log(p1,p2)
        var min_x = Math.min(p1.x,p2.x);
        var max_x = Math.max(p1.x,p2.x);
        var min_y = Math.min(p1.y,p2.y);
        var max_y = Math.max(p1.y,p2.y);
        var options = plot.getOptions();
        var range_x   = options.xaxes[0].max - options.xaxes[0].min;
        var range_y   = options.yaxes[0].max - options.yaxes[0].min;

        var new_y_axis_min = (1 - min_y / rect.h) * range_y + options.yaxes[0].min;
        var new_y_axis_max = (1 - max_y / rect.h) * range_y + options.yaxes[0].min;
        var new_x_axis_min = min_x / rect.w * range_x + options.xaxes[0].min;
        var new_x_axis_max = max_x / rect.w * range_x + options.xaxes[0].min;
        if (UI_GRAPH.x_axis_mode === 1) {
            new_x_axis_min = UI_GRAPH.convertLog(new_x_axis_min);
            new_x_axis_max = UI_GRAPH.convertLog(new_x_axis_max);
        }
        console.log("new_y_axis_min " + new_y_axis_min + " new_y_axis_max " +new_y_axis_max )
        console.log("new_x_axis_min " + new_x_axis_min + " new_x_axis_max " +new_x_axis_max )

        options.xaxes[0].min = new_x_axis_min;
        options.xaxes[0].max = new_x_axis_max;
        options.yaxes[0].min = new_y_axis_max
        options.yaxes[0].max = new_y_axis_min
        SPEC.sendParameters({
            'view_port_start': options.xaxes[0].min * Math.pow(1000,SPEC.config.unit),
            'view_port_end': options.xaxes[0].max * Math.pow(1000,SPEC.config.unit),
        });
        UI_GRAPH.lockUpdateYLimit();
        UI_GRAPH.zoom_used_x = true;
        UI_GRAPH.zoom_used_y = true;
        plot.setupGrid();
        plot.draw();
        SPEC.updateWaterfallWidth();
        SPEC.updateXInfo();
        SPEC.updateYInfo();
        if (UI_GRAPH.minMaxChange !== undefined){
            UI_GRAPH.minMaxChange(options.xaxes[0].min,options.xaxes[0].max)
        }
    }

     // Changes Y zoom/scale for the selected signal
    UI_GRAPH.changeYZoom = function(direction) {
        if (!(SPEC.graphs && SPEC.graphs.elem))
            return null;

        if (UI_GRAPH.y_axis_mode === 1) return;

        var plot_elem = SPEC.graphs.elem;

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
        var value_min = value;
        var value_max = value;

        if ( options.xaxes[0].min + value < SPEC.config.xmin / Math.pow(1000,SPEC.config.unit)) {
            return;
        }

        if ( options.xaxes[0].max + value > SPEC.config.xmax / Math.pow(1000,SPEC.config.unit)) {
            return;
        }
        var cur_min = options.xaxes[0].min + value
        var cur_max = options.xaxes[0].max + value

        options.xaxes[0].min = cur_min;
        options.xaxes[0].max = cur_max;
        SPEC.sendParameters({
            'view_port_start': cur_min * Math.pow(1000,SPEC.config.unit),
            'view_port_end': cur_max * Math.pow(1000,SPEC.config.unit),
        });
        SPEC.graphs.plot.setupGrid();
        SPEC.graphs.plot.draw();
        SPEC.updateWaterfallWidth();
        SPEC.updateXInfo();
        if (UI_GRAPH.minMaxChange !== undefined){
            UI_GRAPH.minMaxChange(options.xaxes[0].min,options.xaxes[0].max)
        }
    };

    // Changes X zoom/scale for all signals
    UI_GRAPH.changeXZoom = function(direction) {
        if (!(SPEC.graphs && SPEC.graphs.elem))
            return null;


        var options = SPEC.graphs.plot.getOptions();
        var axes = SPEC.graphs.plot.getAxes();
        var curr_scale = axes.xaxis.tickSize;

        if ((curr_scale >= UI_GRAPH.time_steps[UI_GRAPH.time_steps.length - 1] && direction == '-') || (curr_scale <= UI_GRAPH.time_steps[0] && direction == '+')) {
            if (direction == '-') UI_GRAPH.zoom_used_x = false;
            return null;
        }
        UI_GRAPH.zoom_used_x = true;
        var range = axes.xaxis.max - axes.xaxis.min;
        var delta = direction == '+' ? 1 : -1

        options.xaxes[0].min = Math.max(SPEC.config.xmin / Math.pow(1000,SPEC.config.unit), axes.xaxis.min + delta * range * 0.1);
        options.xaxes[0].max = Math.min(SPEC.config.xmax / Math.pow(1000,SPEC.config.unit), axes.xaxis.max - delta * range * 0.1);
        SPEC.sendParameters({
            'view_port_start': options.xaxes[0].min * Math.pow(1000,SPEC.config.unit),
            'view_port_end': options.xaxes[0].max * Math.pow(1000,SPEC.config.unit),
        });
        SPEC.graphs.plot.setupGrid();
        SPEC.graphs.plot.draw();
        SPEC.updateWaterfallWidth();
        SPEC.updateXInfo();
        if (UI_GRAPH.minMaxChange !== undefined){
            UI_GRAPH.minMaxChange(options.xaxes[0].min,options.xaxes[0].max)
        }
    };

    UI_GRAPH.resetZoom = function() {
        if (!(SPEC.graphs && SPEC.graphs.elem))
            return;
        if (CLIENT.params.orig['xmin'] == undefined) return;
        if (CLIENT.params.orig['xmax'] == undefined) return;
        // UI_GRAPH.updateYAxis(); // Reset min max for Y-axis
        var plot = SPEC.graphs.plot;
        var curr_options = plot.getOptions();
        curr_options.xaxes[0].min = CLIENT.params.orig['xmin'].value / Math.pow(1000,SPEC.config.unit);
        curr_options.xaxes[0].max = CLIENT.params.orig['xmax'].value / Math.pow(1000,SPEC.config.unit);
        curr_options.yaxes[0].min = UI_GRAPH.ymin;
        curr_options.yaxes[0].max = UI_GRAPH.ymax;

        SPEC.sendParameters({'xmin':CLIENT.params.orig['xmin'].value,
                             'xmax':CLIENT.params.orig['xmax'].value,
                             'view_port_start': SPEC.config.xmin,
                             'view_port_end': SPEC.config.xmax,
                            });
        plot.setupGrid();
        plot.draw();
        SPEC.updateWaterfallWidth();
        SPEC.updateYInfo();
        SPEC.updateXInfo();
        SPEC.update
        UI_GRAPH.zoom_used_x = false;
        UI_GRAPH.zoom_used_y = false;
        if (UI_GRAPH.minMaxChange !== undefined){
            UI_GRAPH.minMaxChange(curr_options.xaxes[0].min,curr_options.xaxes[0].max)
        }
    };

    UI_GRAPH.updateZoom = function() {

        if (SPEC.graphs && SPEC.graphs.elem) {
            var plot_elem = SPEC.graphs.elem;

                if ((CLIENT.params.orig['xmin'] === undefined) || (CLIENT.params.orig['xmax'] === undefined)) return
                var plot = SPEC.graphs.plot;
                CLIENT.params.local['xmin'] = { value: CLIENT.params.orig['xmin'].value };
                CLIENT.params.local['xmax'] = { value: CLIENT.params.orig['xmax'].value };

                var axes = plot.getAxes();
                var options = plot.getOptions();

                options.xaxes[0].min = CLIENT.params.local['xmin'].value / Math.pow(1000,SPEC.config.unit);
                options.xaxes[0].max = CLIENT.params.local['xmax'].value / Math.pow(1000,SPEC.config.unit);
                options.yaxes[0].min = axes.yaxis.min;
                options.yaxes[0].max = axes.yaxis.max;
                SPEC.sendParameters({
                    'view_port_start': CLIENT.params.local['xmin'].value ,
                    'view_port_end': CLIENT.params.local['xmax'].value ,
                });
                plot.setupGrid();
                plot.draw();
                SPEC.updateWaterfallWidth();
                SPEC.updateYInfo();
                SPEC.updateXInfo();
                if (UI_GRAPH.minMaxChange !== undefined){
                    UI_GRAPH.minMaxChange(options.xaxes[0].min,options.xaxes[0].max)
                }
        }
    };

    UI_GRAPH.checkYAxisLimit = function(value){
        if (isFinite(value) && (UI_GRAPH.lock_limit_change <= 0)) {
            var offset = Math.abs(value / 10);
            if (SPEC.config.y_axis_mode === "v"){
                 if ((SPEC.config.attenuator_ch1 === "1") || (SPEC.config.attenuator_ch2 === "1")){
                    offset = 1;
                 }else{
                    offset = 0.1;
                 }
            }

            if (UI_GRAPH.ymax < (value + offset)) {
                UI_GRAPH.ymax = value + offset;
                return true;
            }

            if (UI_GRAPH.ymin > Math.max(value - offset,-130)) {
                UI_GRAPH.ymin = Math.max(value - offset,-130);
                return true;
            }
        }
        return false;
    }

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
        SPEC.sendParameters({'y_axis_mode':mode_value});
        UI_GRAPH.resetZoom();
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
            UI_GRAPH.ymin = -130.0;
        }

        if (mode === "dbuV") {
            UI_GRAPH.ymax = 130.0;
            UI_GRAPH.ymin = -10.0;
        }

        if (mode === "dbV") {
            UI_GRAPH.ymax = 10.0;
            UI_GRAPH.ymin = -130.0;
        }

        if (mode === "v") {
            UI_GRAPH.ymax = 1.5;
            UI_GRAPH.ymin = -0.1;
        }

        if (mode === "mW") {
            UI_GRAPH.ymax = 10.0;
            UI_GRAPH.ymin = -0.1;
        }

        if (mode === "dBW") {
            UI_GRAPH.ymax = 10.0;
            UI_GRAPH.ymin = -130.0;
        }
    }

    UI_GRAPH.updateMinMaxXAxis = function(values){
        if (!(SPEC.graphs && SPEC.graphs.elem)) return ;
        if (values.length == 0) return;
        if (UI_GRAPH.x_axis_mode !== 1) return;
        var needSetup = false;
        var i = 0;
        while(i < values.length && values[i] === 0){
            i++;
        }
        if (i >= values.length) return;
        var start = values[i] / Math.pow(1000,SPEC.config.unit);

        if (start !== UI_GRAPH.x_axis_min) {
            UI_GRAPH.x_axis_min = start;
            needSetup = true;
        }

        var end = values[values.length - 1] / Math.pow(1000,SPEC.config.unit);
        if (end !== UI_GRAPH.x_axis_max) {
            UI_GRAPH.x_axis_max = end
            needSetup = true;
        }
        if (needSetup) {
            SPEC.graphs.plot.setupGrid();
        }
    }

    UI_GRAPH.convertLog = function(v){
        if (UI_GRAPH.x_axis_min === 0) return v;
        var a = UI_GRAPH.x_axis_min;
        var b = UI_GRAPH.x_axis_max;
        v = v > b ? b : v
        var x = Math.log10(b/a)/(b-a);
        var y = b / Math.pow(10,x * b);
        v =  y * Math.pow(10, x * v);
        return v;
    }

    UI_GRAPH.convertXLog = function(v){
        if (UI_GRAPH.x_axis_min === 0) return v;
        var a = UI_GRAPH.x_axis_min;
        var b = UI_GRAPH.x_axis_max;
        var x = Math.log10(b/a)/(b-a);
        v =  Math.log10(v/b)/x + b;
        return v;
    }

    UI_GRAPH.setXAxisMode = function(mode){
        if (!(SPEC.graphs && SPEC.graphs.elem))
            return ;
        UI_GRAPH.x_axis_mode = mode;
        UI_GRAPH.resetZoom();
        SPEC.graphs.plot.setupGrid();
    }


}(window.UI_GRAPH = window.UI_GRAPH || {}, jQuery));