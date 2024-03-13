(function(CURSORS, $, undefined) {

    CURSORS.cursorsRelative = {
        x1: 0.33,
        x2: 0.66,
        y1: 0.33,
        y2: 0.66
    };

    CURSORS.state = {
        cursor_dragging: false,
        mouseover: false,
    };

    CURSORS.getPoltRect = function(){
        var plot = MAIN.getPlot();
        if (plot === undefined) {
            return {l:0,t:0,w:0,h:0};
        }
        var gPosition = $('#graph_bode_grid').offset();
        var gLeft = gPosition.left;
        var gTop = gPosition.top;
        var gWidth = $('#graph_bode_grid').width();
        var gHeight = $('#graph_bode_grid').height();
        var plotOffset = plot.getPlotOffset();

        gLeft += plotOffset.left
        gTop += plotOffset.top
        gWidth = gWidth - plotOffset.left - plotOffset.right
        gHeight = gHeight - plotOffset.top - plotOffset.bottom
        return {l:gLeft,t:gTop,w:gWidth,h:gHeight}
    }

    CURSORS.moveX = function(ui) {
        var plot = MAIN.getPlot();
        if (plot === undefined) {
            return;
        }

        var graphSize = CURSORS.getPoltRect();

        if (ui.helper[0].id == 'cur_x1_arrow') {
            var c_pos = $("#cur_x1_arrow").offset().left + $("#cur_x1_arrow").width() / 2 - 1
            CURSORS.cursorsRelative.x1 = (c_pos - graphSize.l) / graphSize.w
            console.log(CURSORS.cursorsRelative.x1)
            if (CURSORS.cursorsRelative.x1 < 0)
                CURSORS.cursorsRelative.x1 = 0;
            if (CURSORS.cursorsRelative.x1 > 1)
                CURSORS.cursorsRelative.x1 = 1;
            CLIENT.parametersCache["IA_CURSOR_X1"] = { value: CURSORS.cursorsRelative.x1 };
            CLIENT.sendParameters();
        } else if (ui.helper[0].id == 'cur_x2_arrow') {
            var c_pos = $("#cur_x2_arrow").offset().left + $("#cur_x2_arrow").width() / 2 - 1
            CURSORS.cursorsRelative.x2 = (c_pos - graphSize.l) / graphSize.w
            if (CURSORS.cursorsRelative.x2 < 0)
                CURSORS.cursorsRelative.x2 = 0;
            if (CURSORS.cursorsRelative.x2 > 1)
                CURSORS.cursorsRelative.x2 = 1;
            CLIENT.parametersCache["IA_CURSOR_X2"] = { value: CURSORS.cursorsRelative.x2 };
            CLIENT.sendParameters();
        }
        CURSORS.updateXLinesAndArrows();
    }

    CURSORS.moveY = function(ui) {

        var plot = MAIN.getPlot();
        if (plot === undefined) {
            return;
        }

        var graphSize = CURSORS.getPoltRect();

        if (ui.helper[0].id == 'cur_y1_arrow') {
            var c_pos = $("#cur_y1_arrow").offset().top + $("#cur_y1_arrow").height() / 2 - 1
            CURSORS.cursorsRelative.y1 = (c_pos - graphSize.t) / graphSize.h
            if (CURSORS.cursorsRelative.y1 < 0)
                CURSORS.cursorsRelative.y1 = 0;
            if (CURSORS.cursorsRelative.y1 > 1)
                CURSORS.cursorsRelative.y1 = 1;
            CLIENT.parametersCache["IA_CURSOR_Y1"] = { value: CURSORS.cursorsRelative.y1 };
            CLIENT.sendParameters();
        } else if (ui.helper[0].id == 'cur_y2_arrow') {
            var c_pos = $("#cur_y2_arrow").offset().top + $("#cur_y2_arrow").height() / 2 - 1
            CURSORS.cursorsRelative.y2 = (c_pos - graphSize.t) / graphSize.h
            if (CURSORS.cursorsRelative.y2 < 0)
                CURSORS.cursorsRelative.y2 = 0;
            if (CURSORS.cursorsRelative.y2 > 1)
                CURSORS.cursorsRelative.y2 = 1;
            CLIENT.parametersCache["IA_CURSOR_Y2"] = { value: CURSORS.cursorsRelative.y2 };
            CLIENT.sendParameters();
        }
        CURSORS.updateYLinesAndArrows();
    }

    CURSORS.enableX = function(cursor_name, new_params) {
        if (!CURSORS.state.cursor_dragging && !CURSORS.state.mouseover) {
            var x = (cursor_name == 'IA_CURSOR_X1_ENABLE' ? 'x1' : 'x2');

            if (new_params[cursor_name].value) {
                $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').show();
            } else {
                $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').hide();
            }

            if ($('cur_x1').is(':visible') && $('cur_x2').is(':visible')) {
                $('cur_x_diff').show();
                $('cur_x_diff_info').show();
            }else{
                $('cur_x_diff').hide();
                $('cur_x_diff_info').hide();
            }

            CURSORS.updateXLinesAndArrows();
            CURSORS.installCursorsHandlers();
        }
    }

    CURSORS.enableY = function(cursor_name, new_params) {
        if (!CURSORS.state.cursor_dragging && !CURSORS.state.mouseover) {
            var y = (cursor_name == 'IA_CURSOR_Y1_ENABLE' ? 'y1' : 'y2');

            if (new_params[cursor_name].value) {
                $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').show();
            } else {
                $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').hide();
            }

            if ($('cur_y1').is(':visible') && $('cur_y2').is(':visible')) {
                $('cur_y_diff').show();
                $('cur_y_diff_info').show();
            }else{
                $('cur_y_diff').hide();
                $('cur_y_diff_info').hide();
            }

            CURSORS.updateYLinesAndArrows();
            CURSORS.installCursorsHandlers();
        }
    }



    // X-cursors
    CURSORS.updateXLinesAndArrows = function() {

        var plot = MAIN.getPlot();
        if (plot === undefined) {
            return;
        }

        var graphSize = CURSORS.getPoltRect();

        var axes = plot.getAxes();
        var scale = MAIN.scale;

        var diff_px = 0;
        let unit = ' Hz';
        let min = axes.xaxis.min;
        let max = axes.xaxis.max;
        let new_val = 0;

        if (min < 1)
            min = 1;


        if (scale) {
            var a = Math.log10(min);
            var b = Math.log10(max);
            var c = (b - a) * CURSORS.cursorsRelative.x1
            new_val = Math.pow(10, a + c);
        } else {
            new_val = (max - min) * CURSORS.cursorsRelative.x1 + min;
        }

        $('#cur_x1_arrow').offset({
            left: graphSize.l + (graphSize.w * CURSORS.cursorsRelative.x1) - $("#cur_x1_arrow").width() / 2 - 1,
            top: graphSize.t + graphSize.h - $("#cur_x1_arrow").height()
        });
        $('#cur_x1, #cur_x1_info').offset({
            left: graphSize.l + graphSize.w * CURSORS.cursorsRelative.x1 - 2
        });

        $('#cur_x1').offset({
            top: graphSize.t
        });
        $('#cur_x1_info').offset({ top: graphSize.t + $('#cur_x1_info').outerHeight() + 30 });
        $('#cur_x1_info').html(new_val.toFixed(2) + unit).data('cleanval', +new_val);
        $('#cur_x1').height(graphSize.h);


        if (scale) {
            var a = Math.log10(min);
            var b = Math.log10(max);
            var c = (b - a) * CURSORS.cursorsRelative.x2
            new_val = Math.pow(10, a + c);

        } else {
            new_val = (max - min) * CURSORS.cursorsRelative.x2 + min;
        }

        $('#cur_x2_arrow').offset({
            left: graphSize.l + (graphSize.w * CURSORS.cursorsRelative.x2) - $("#cur_x2_arrow").width() / 2 - 1,
            top: graphSize.t + graphSize.h - $("#cur_x2_arrow").height()
        });
        $('#cur_x2, #cur_x2_info').offset({
            left: graphSize.l + graphSize.w * CURSORS.cursorsRelative.x2 - 2
        });
        $('#cur_x2').offset({
            top: graphSize.t
        });
        $('#cur_x2_info').offset({ top: graphSize.t + $('#cur_x2_info').outerHeight() + 30 });
        $('#cur_x2_info').html(new_val.toFixed(2) + unit).data('cleanval', +new_val);
        $('#cur_x2').height(graphSize.h);


        diff_px = Math.abs( CURSORS.cursorsRelative.x1 -  CURSORS.cursorsRelative.x2) * graphSize.w - 6;
        if ($('#cur_x1').is(':visible') && $('#cur_x2').is(':visible') && diff_px > 12) {
            var left_pos = Math.min(CURSORS.cursorsRelative.x1, CURSORS.cursorsRelative.x2) * graphSize.w + 3 + graphSize.l;
            var value = $('#cur_x1_info').data('cleanval') - $('#cur_x2_info').data('cleanval');

            $("#cur_x_diff").show()
            $("#cur_x_diff").offset({left: left_pos}).width(diff_px)

            $('#cur_x_diff_info').show();
            $('#cur_x_diff_info').html(Math.abs(value).toFixed(2) + unit)
                .offset({ left: left_pos + diff_px / 2 - 2 - $('#cur_x_diff_info').outerWidth() / 2 })

        } else {
            $('#cur_x_diff, #cur_x_diff_info').hide();
        }
    }


    // Y-cursors
    CURSORS.updateYLinesAndArrows = function() {

        var plot = MAIN.getPlot();
        if (plot === undefined) {
            return;
        }

        var graphSize = CURSORS.getPoltRect();

        var diff_px = 0;
        var axes = plot.getAxes();
        let min = axes.yaxis.min;
        let max = axes.yaxis.max;
        let new_val = 0;


        new_val = (min - max) * CURSORS.cursorsRelative.y1 + max;

        $('#cur_y1_arrow').offset({
            left: graphSize.l + graphSize.w - $('#cur_y1_arrow').width(),
            top: graphSize.t + (graphSize.h * CURSORS.cursorsRelative.y1) -  $("#cur_y1_arrow").height() / 2 - 1
        });

        $('#cur_y1, #cur_y1_info').offset({
            top: graphSize.t + graphSize.h * CURSORS.cursorsRelative.y1 - 2
        });
        $('#cur_y1_info').offset({ left: graphSize.l + $('#cur_y1_info').outerWidth() });
        $('#cur_y1_info').html(new_val.toFixed(2)).data('cleanval', + new_val);
        $('#cur_y1').width(graphSize.w);


        new_val = (min - max) * CURSORS.cursorsRelative.y2 + max;

        $('#cur_y2_arrow').offset({
            left: graphSize.l + graphSize.w - $('#cur_y2_arrow').width(),
            top: graphSize.t + (graphSize.h * CURSORS.cursorsRelative.y2) -  $("#cur_y2_arrow").height() / 2 - 1
        });

        $('#cur_y2, #cur_y2_info').offset({
            top: graphSize.t + graphSize.h * CURSORS.cursorsRelative.y2 - 2
        });
        $('#cur_y2_info').offset({ left: graphSize.l + $('#cur_y2_info').outerWidth() });
        $('#cur_y2_info').html(new_val.toFixed(2)).data('cleanval', +new_val);;
        $('#cur_y2').width(graphSize.w);


        diff_px = Math.abs($('#cur_y1').offset().top - $('#cur_y2').offset().top) - 6;
        if ($('#cur_y1').is(':visible') && $('#cur_y2').is(':visible') && diff_px > 12) {
            var top = Math.min($('#cur_y1').offset().top, $('#cur_y2').offset().top) + 3;
            var value = $('#cur_y1_info').data('cleanval') - $('#cur_y2_info').data('cleanval');
            value = Math.abs(value).toFixed(2)
            // value = MAIN.yAxisSuffix !== undefined ? value + "⋅" + MAIN.yAxisSuffix.value : value
            $('#cur_y_diff').show()
            $('#cur_y_diff')
                .offset({ left: graphSize.l + 50, top: top })
                .height(diff_px)

            $('#cur_y_diff_info').show()
            $('#cur_y_diff_info')
                .html(value)
                .offset({ left: $('#cur_y_diff').offset().l, top: top + diff_px / 2 - 2 })

        } else {
            $('#cur_y_diff, #cur_y_diff_info').hide();
        }
    }

    CURSORS.calculateLimits = function(){
        var plot = MAIN.getPlot();
        if (!plot) {
            return [0,0,0,0];
        }

        var r = CURSORS.getPoltRect();
        return [r.l - 2, r.t - 2, r.w + r.l - 2, r.h + r.t - 2]
    }

    CURSORS.installCursorsHandlers = function(){
        // X cursor arrows dragging
        $('#cur_x1_arrow, #cur_x2_arrow').draggable({
            axis: 'x',
            containment: CURSORS.calculateLimits(),
            start: function(ev, ui) {
                CURSORS.state.line_moving = true;
                CURSORS.state.cursor_dragging = true;
            },
            drag: function(ev, ui) {
                CURSORS.moveX(ui);
            },
            stop: function(ev, ui) {
                CURSORS.moveX(ui);
                CURSORS.state.line_moving = false;
                CURSORS.state.cursor_dragging = false;
            }
        });


        // Y cursor arrows dragging
        $('#cur_y1_arrow, #cur_y2_arrow').draggable({
            axis: 'y',
            containment: CURSORS.calculateLimits(),
            start: function(ev, ui) {
                CURSORS.state.cursor_dragging = true;
            },
            drag: function(ev, ui) {
                CURSORS.moveY(ui);
            },
            stop: function(ev, ui) {
                CURSORS.moveY(ui);
                CURSORS.state.cursor_dragging = false;
            }
        });

    }

    CURSORS.sendCursorState = function(x,state){
        CLIENT.parametersCache["IA_CURSOR_" + x.toUpperCase() + "_ENABLE"] = { value: state };
        CLIENT.sendParameters();
    };


    CURSORS.enableCursorX1Callback = function(new_params){
        var param_name = "IA_CURSOR_X1_ENABLE"
        if (new_params[param_name].value){
            $('#'+param_name).addClass('active');
        }else{
            $('#'+param_name).removeClass('active');
        }
        CURSORS.enableX(param_name,new_params)
    }

    CURSORS.enableCursorX2Callback = function(new_params){
        var param_name = "IA_CURSOR_X2_ENABLE"
        if (new_params[param_name].value){
            $('#'+param_name).addClass('active');
        }else{
            $('#'+param_name).removeClass('active');
        }
        CURSORS.enableX(param_name,new_params)
    }

    CURSORS.enableCursorY1Callback = function(new_params){
        var param_name = "IA_CURSOR_Y1_ENABLE"
        if (new_params[param_name].value){
            $('#'+param_name).addClass('active');
        }else{
            $('#'+param_name).removeClass('active');
        }
        CURSORS.enableY(param_name,new_params)
    }

    CURSORS.enableCursorY2Callback = function(new_params){
        var param_name = "IA_CURSOR_Y2_ENABLE"
        if (new_params[param_name].value){
            $('#'+param_name).addClass('active');
        }else{
            $('#'+param_name).removeClass('active');
        }
        CURSORS.enableY(param_name,new_params)
    }

    CURSORS.cursorX1Callback = function(new_params){
        CURSORS.cursorsRelative.x1 = new_params["IA_CURSOR_X1"].value;
        CURSORS.updateXLinesAndArrows();
    }

    CURSORS.cursorX2Callback = function(new_params){
        CURSORS.cursorsRelative.x2 = new_params["IA_CURSOR_X2"].value;
        CURSORS.updateXLinesAndArrows();
    }

    CURSORS.cursorY1Callback = function(new_params){
        CURSORS.cursorsRelative.y1 = new_params["IA_CURSOR_Y1"].value;
        CURSORS.updateYLinesAndArrows();
    }

    CURSORS.cursorY2Callback = function(new_params){
        CURSORS.cursorsRelative.y2 = new_params["IA_CURSOR_Y2"].value;
        CURSORS.updateYLinesAndArrows();
    }


}(window.CURSORS = window.CURSORS || {}, jQuery));


$(function() {

    $('#IA_CURSOR_X1_ENABLE').click(function() {
        var btn = $(this);
        if (!btn.hasClass('active'))
            CURSORS.sendCursorState('x1',true);
        else
            CURSORS.sendCursorState('x1',false);
    });

    $('#IA_CURSOR_X2_ENABLE').click(function() {
        var btn = $(this);
        if (!btn.hasClass('active'))
            CURSORS.sendCursorState('x2',true);
        else
            CURSORS.sendCursorState('x2',false);
    });

    $('#IA_CURSOR_Y1_ENABLE').click(function() {
        var btn = $(this);
        if (!btn.hasClass('active'))
            CURSORS.sendCursorState('y1',true);
        else
            CURSORS.sendCursorState('y1',false);
    });

    $('#IA_CURSOR_Y2_ENABLE').click(function() {
        var btn = $(this);
        if (!btn.hasClass('active'))
            CURSORS.sendCursorState('y2',true);
        else
            CURSORS.sendCursorState('y2',false);
    });

    CURSORS.installCursorsHandlers();
})