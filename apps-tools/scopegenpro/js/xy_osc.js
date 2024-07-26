(function(OSC, $, undefined) {



    OSC.chShowXY = function(new_params) {
        // Set button state in sub dialog
        var param_name = "X_Y_SHOW";
        var state = new_params[param_name].value;
        var field = $('#' + param_name);
        if (field.is('button')) {
            field[state === true? 'addClass' : 'removeClass']('active');
        }
        if (state === true){
            if ($('#xy_main').css("display") === "none"){
                OSC.resize()
            }
            $('#xy_main').css("display", "");
            $('#right_menu .menu-btn.x_y').removeAttr('disabled');
            OSC.drawGraphGridXY()
        }else{
            if ($('#xy_main').css("display") !== "none"){
                OSC.resize()
            }
            $('#xy_main').css("display", "none");
            $('#right_menu .menu-btn.x_y').attr('disabled', 'disabled');
        }
        // OSC.resize(false)
        // OSC.showInArrow(ch,state);
        // OSC.cursorY();
        // OSC.triggerParam();
    }

    OSC.updateXYSrcX = function(new_params,param_name) {
        var field = $('#' + param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.val(new_params[param_name].value);
        }
        OSC.setXYAxisScale()
        OSC.updateTitileXAxisTicksXY()
    }

    OSC.updateXYSrcY = function(new_params,param_name) {
        var field = $('#' + param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.val(new_params[param_name].value);
        }
        OSC.setXYAxisScale()
        OSC.updateTitileYAxisTicksXY()
    }

    OSC.drawSignalXY = function(signals) {
        var gh = $('#xy_graph_grid').height()
        var gw = $('#xy_graph_grid').width()
        if (gh == 0 || gw == 0) return
        var pointArr = []
        var colorsArr = []
        var points = []
        var sig_name = 'xy'
        var color = OSC.config.graph_colors[sig_name];

        if (OSC.params.orig['OSC_VIEW_START_POS'] && OSC.params.orig['OSC_VIEW_END_POS']) {
            for (var i = OSC.params.orig['OSC_VIEW_START_POS'].value; i < OSC.params.orig['OSC_VIEW_END_POS'].value; i++)
                points.push([signals['X_AXIS_VALUES'].value[i], signals['Y_AXIS_VALUES'].value[i]]);
        } else {
            for (var i = 0; i < new_signals[sig_name].size; i++) {
                points.push([signals['X_AXIS_VALUES'].value[i], signals['Y_AXIS_VALUES'].value[i]]);
            }
        }

        pointArr.push(points);
        colorsArr.push(color);

        if (OSC.graphs["xy"]) {
            OSC.graphs["xy"].elem.show();
            OSC.graphs["xy"].plot.setColors(colorsArr);
            OSC.graphs["xy"].plot.resize();
            OSC.graphs["xy"].plot.setupGrid();
            OSC.graphs["xy"].plot.setData(pointArr);
            OSC.graphs["xy"].plot.draw();
        } else {
            OSC.graphs["xy"] = {};
            OSC.graphs["xy"].elem = $('<div class="xy_plot" />').css($('#xy_graph_grid').css(['height', 'width'])).appendTo('#xy_graphs');
            OSC.graphs["xy"].plot = $.plot(OSC.graphs["xy"].elem, [pointArr], {
                name: "xy",
                series: {
                    shadowSize: 0, // Drawing is faster without shadows
                },
                yaxis: {
                    min: -5,
                    max: 5
                },
                xaxis: {
                    min: -5,
                    max: 5
                },
                grid: {
                    show: false
                },
                colors: [
                    '#FF2A68', '#FF9500', '#FFDB4C', '#87FC70', '#22EDC7', '#1AD6FD', '#C644FC', '#52EDC7', '#EF4DB6'
                ]
            });
            // If page not full loaded
            if (OSC.graphs["xy"].elem === undefined){
                OSC.graphs = {};
            }

            $('.xy_plot').css($('#xy_graph_grid').css(['height', 'width']));

        }
    }

    OSC.setXYAxisScale = function(){

        if (OSC.params.orig['X_AXIS_SOURCE']){
            var xsrc = OSC.params.orig['X_AXIS_SOURCE'].value;
            var srcName = OSC.xyGetCh(xsrc)
            if (srcName !== ""){
                var value = OSC.params.orig["GPOS_SCALE_"+srcName] ? OSC.params.orig["GPOS_SCALE_"+srcName].value : 0;
                var unit = ' V';
                if (Math.abs(value) < 1.0) {
                    value *= 1000;
                    unit = ' mV';
                } else if (Math.abs(value) >= 1000000) {
                    value /= 1000000;
                    unit = ' MV';
                } else if (Math.abs(value) >= 1000) {
                    value /= 1000;
                    unit = ' kV';
                }
                $('#OSC_XY_X_AXIS_SCALE , #OSC_XY_X_AXIS_SCALE_UNIT, #OSC_XY_X_AXIS_NAME').css('color', OSC.config.graph_colors[srcName.toLowerCase()])
                $('#OSC_XY_X_AXIS_SCALE').html(value)
                $("#OSC_XY_X_AXIS_SCALE_UNIT").html(unit + (srcName == "MATH" ? OSC.mathSuffix() : "") +"/div")
                OSC.xyCursorX();
                OSC.updateXCursorDiffXY()
            }else{
                console.log("Error channel!!!")
            }
        }

        if (OSC.params.orig['Y_AXIS_SOURCE']){
            var xsrc = OSC.params.orig['Y_AXIS_SOURCE'].value;
            var srcName = OSC.xyGetCh(xsrc)
            if (srcName !== ""){
                var value = OSC.params.orig["GPOS_SCALE_"+srcName] ? OSC.params.orig["GPOS_SCALE_"+srcName].value : 0;
                var unit = ' V';
                OSC.div = 1;
                if (Math.abs(value) < 1.0) {
                    value *= 1000;
                    unit = ' mV';
                } else if (Math.abs(value) >= 1000000) {
                    value /= 1000000;
                    unit = ' MV';
                } else if (Math.abs(value) >= 1000) {
                    value /= 1000;
                    unit = ' kV';
                }
                $('#OSC_XY_Y_AXIS_SCALE , #OSC_XY_Y_AXIS_SCALE_UNIT, #OSC_XY_Y_AXIS_NAME').css('color', OSC.config.graph_colors[srcName.toLowerCase()])
                $('#OSC_XY_Y_AXIS_SCALE').html(value)
                $("#OSC_XY_Y_AXIS_SCALE_UNIT").html(unit + (srcName == "MATH" ? OSC.mathSuffix() : "") + "/div")
                OSC.xyCursorY();
                OSC.updateYCursorDiffXY()
            }else{
                console.log("Error channel!!!")
            }
        }
        OSC.upda
    }

    OSC.createAxisTicksXY = function(){
        OSC.createXAxisTicksXY()
        OSC.createYAxisTicksXY()
    }

    OSC.createXAxisTicksXY = function(){
        var graphs = document.getElementById("xy_main");
        for(var i = -5; i <= 5; i++){
            var tick = document.createElement('div');
            tick.id = "xy_xaxis_tick" + (i + 5)
            tick.className = "x_axis_ticks"
            tick.innerText = i;
            graphs.appendChild(tick)
        }
        OSC.moveTitileXAxisTicksXY()
    }

    OSC.updateTitileXAxisTicksXY = function(){
        var scale = 0
        var srcName = ""
        if (OSC.params.orig['X_AXIS_SOURCE']){
            var xsrc = OSC.params.orig['X_AXIS_SOURCE'].value;
            srcName = OSC.xyGetCh(xsrc)
            if (srcName !== ""){
                scale = OSC.params.orig["GPOS_SCALE_"+srcName] ? OSC.params.orig["GPOS_SCALE_"+srcName].value : 0;
            }
        }
        for(var i = -5; i <= 5; i++){
            $("#xy_xaxis_tick" + (i + 5)).html(OSC.convertVoltageForAxis(-i * scale)+(srcName == "MATH" ? OSC.mathSuffix() : ""))
        }
        OSC.moveTitileXAxisTicksXY()
    }

    OSC.moveTitileXAxisTicksXY = function(){
        var gh = $('#xy_main').height()
        var gw = $('#xy_main').width()
        for(var i = -5; i <= 5; i++){
            var ws = $("#xy_xaxis_tick" + (i + 5)).width() / 2
            if (i == -5) ws = 0
            if (i ==  5) ws *= 2
            $("#xy_xaxis_tick" + (i + 5)).css('top',gh + 41).css('left', gw / 2.0 + (gw / 2.0) * i/5.0 + 48 - ws)
        }
    }

    OSC.createYAxisTicksXY = function(){
        var graphs = document.getElementById("xy_main");
        for(var i = -5; i <= 5; i++){
            var tick = document.createElement('div');
            tick.id = "xy_yaxis_tick" + (i + 5)
            tick.className = "y_axis_ticks rotate"
            tick.innerText = i;
            graphs.appendChild(tick)
        }
        OSC.moveTitileYAxisTicksXY()
    }

    OSC.updateTitileYAxisTicksXY = function(){
        var scale = 0
        var srcName = ""
        if (OSC.params.orig['Y_AXIS_SOURCE']){
            var xsrc = OSC.params.orig['Y_AXIS_SOURCE'].value;
            srcName = OSC.xyGetCh(xsrc)
            if (srcName !== ""){
                scale = OSC.params.orig["GPOS_SCALE_"+srcName] ? OSC.params.orig["GPOS_SCALE_"+srcName].value : 0;
            }
        }
        for(var i = -5; i <= 5; i++){
            $("#xy_yaxis_tick" + (i + 5)).html(OSC.convertVoltageForAxis(-i * scale)+(srcName == "MATH" ? OSC.mathSuffix() : ""))
        }
        OSC.moveTitileYAxisTicksXY()
    }

    OSC.moveTitileYAxisTicksXY = function(){
        var gh = $('#xy_main').height()
        for(var i = -5; i <= 5; i++){
            var ws = $("#xy_yaxis_tick" + (i + 5)).height() / 2
            if (i == -5) ws = 0
            if (i ==  5) ws *= 2
            $("#xy_yaxis_tick" + (i + 5)).css('left',32).css('top', gh / 2.0 + (gh / 2.0) * i/5.0 + 33 - ws)
        }
    }

}(window.OSC = window.OSC || {}, jQuery));