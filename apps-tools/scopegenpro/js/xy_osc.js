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
            $('#right_menu .menu-btn.x_y').prop('disabled', false);
            OSC.drawGraphGridXY()
        }else{
            if ($('#xy_main').css("display") !== "none"){
                OSC.resize()
            }
            $('#xy_main').css("display", "none");
            $('#right_menu .menu-btn.x_y').prop('disabled', true);
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
    }

    OSC.updateXYSrcY = function(new_params,param_name) {
        var field = $('#' + param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.val(new_params[param_name].value);
        }
        OSC.setXYAxisScale()
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

    OSC.xyMathSuffix = function(){
        var value_op = OSC.params.orig["OSC_MATH_OP"] ? OSC.params.orig["OSC_MATH_OP"].value : undefined;
        if (value_op !== undefined){
            var units = ['', '', '',  '^2', '', '',  '/s', 's'];
            return units[value_op]
        }
        return ""
    }

    OSC.setXYAxisScale = function(){

        if (OSC.params.orig['X_AXIS_SOURCE']){
            var xsrc = OSC.params.orig['X_AXIS_SOURCE'].value;
            var srcName = OSC.xyGetCh(xsrc)
            if (srcName !== ""){
                var value = OSC.params.orig["OSC_"+srcName+"_SCALE"] ? OSC.params.orig["OSC_"+srcName+"_SCALE"].value : 0;
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
                $("#OSC_XY_X_AXIS_SCALE_UNIT").html(unit + (srcName == "MATH" ? OSC.xyMathSuffix() : "") +"/div")
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
                var value = OSC.params.orig["OSC_"+srcName+"_SCALE"] ? OSC.params.orig["OSC_"+srcName+"_SCALE"].value : 0;
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
                $("#OSC_XY_Y_AXIS_SCALE_UNIT").html(unit + (srcName == "MATH" ? OSC.xyMathSuffix() : "") + "/div")
                OSC.xyCursorY();
                OSC.updateYCursorDiffXY()
            }else{
                console.log("Error channel!!!")
            }
        }
        OSC.upda
    }


}(window.OSC = window.OSC || {}, jQuery));