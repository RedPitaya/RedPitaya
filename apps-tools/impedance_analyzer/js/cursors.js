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

    CURSORS.updateXLinesPosition = function() {
        const w = $('#cursors_holder').width() - 2;
        const h = $('#cursors_holder').height();
        $('#cur_x1_arrow').css({
            left: w * CURSORS.cursorsRelative.x1,
            top: h - 27
        });

        $('#cur_x2_arrow').css({
            left: w * CURSORS.cursorsRelative.x2,
            top: h - 27
        });

        $('#cur_x1').height(h);
        $('#cur_x2').height(h);
    }

    CURSORS.updateYLinesPosition = function() {
        const w = $('#cursors_holder').width();
        const h = $('#cursors_holder').height() - 2;
        $('#cur_y1_arrow').css({
            left: w - 27 ,
            top: h * CURSORS.cursorsRelative.y1
        });

        $('#cur_y2_arrow').css({
            left: w - 27 ,
            top: h * CURSORS.cursorsRelative.y2
        });

        $('#cur_y1').width(w);
        $('#cur_y2').width(w);
    }


    CURSORS.moveX = function(ui) {
        const w = $('#cursors_holder').width() - 2;
        if (ui.helper[0].id == 'cur_x1_arrow') {
            const l = $("#cur_x1_arrow").position().left
            CURSORS.cursorsRelative.x1 = l / w;
            if (CURSORS.cursorsRelative.x1 < 0)
                CURSORS.cursorsRelative.x1 = 0;
            if (CURSORS.cursorsRelative.x1 > 1)
                CURSORS.cursorsRelative.x1 = 1;

        } else if (ui.helper[0].id == 'cur_x2_arrow') {
            const l = $("#cur_x2_arrow").position().left
            CURSORS.cursorsRelative.x2 = l / w;
            if (CURSORS.cursorsRelative.x2 < 0)
                CURSORS.cursorsRelative.x2 = 0;
            if (CURSORS.cursorsRelative.x2 > 1)
                CURSORS.cursorsRelative.x2 = 1;
        }
        CURSORS.updateXLinesAndArrows();
    }


    CURSORS.moveY = function(ui) {
        const h = $('#cursors_holder').height() - 2;
        if (ui.helper[0].id == 'cur_y1_arrow') {
            const t = $("#cur_y1_arrow").position().top
            CURSORS.cursorsRelative.y1 = t / h;
            if (CURSORS.cursorsRelative.y1 < 0)
                CURSORS.cursorsRelative.y1 = 0;
            if (CURSORS.cursorsRelative.y1 > 1)
                CURSORS.cursorsRelative.y1 = 1;

        } else if (ui.helper[0].id == 'cur_y2_arrow') {
            const t = $("#cur_y2_arrow").position().top
            CURSORS.cursorsRelative.y2 = t / h;
            if (CURSORS.cursorsRelative.y2 < 0)
                CURSORS.cursorsRelative.y2 = 0;
            if (CURSORS.cursorsRelative.y2 > 1)
                CURSORS.cursorsRelative.y2 = 1;
        }
        CURSORS.updateYLinesAndArrows();
    }

    CURSORS.enableX = function(cursor_name, new_params) {
        if (!CURSORS.state.cursor_dragging && !CURSORS.state.mouseover) {
            var x = (cursor_name == 'IA_CURSOR_X1_ENABLE' ? 'x1' : 'x2');
            if (new_params) {
                $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').show();
                $('#'+cursor_name).addClass('active');
            } else {
                $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').hide();
                $('#'+cursor_name).removeClass('active');
            }
            CURSORS.updateXLinesAndArrows();
        }
    }

    CURSORS.enableY = function(cursor_name, new_params) {
        if (!CURSORS.state.cursor_dragging && !CURSORS.state.mouseover) {
            var x = (cursor_name == 'IA_CURSOR_Y1_ENABLE' ? 'y1' : 'y2');
            if (new_params) {
                $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').show();
                $('#'+cursor_name).addClass('active');
            } else {
                $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').hide();
                $('#'+cursor_name).removeClass('active');
            }
            CURSORS.updateYLinesAndArrows();
        }
    }

    // X-cursors
    CURSORS.updateXLinesAndArrows = function() {

        const cl = $('#cursors_holder').offset().left;
        const w = $('#cursors_holder').width();

        var axes = MAIN.graphCache.plot.getAxes();
        var diff_px = 0;
        let unit = ' Hz';
        let min = axes.xaxis.min;
        let max = axes.xaxis.max;
        let new_val = 0;

        const setCursor = (cur, min,max,t) => {

            if (MAIN.scale) {
                var a = Math.log10(min);
                var b = Math.log10(max);
                var c = (b - a) * t
                new_val = Math.pow(10, a + c);
            } else {
                new_val = (max - min) * t + min;
            }

            // Update line
            $('#cur_' + cur).offset({
                left: $('#cur_' + cur + '_arrow').offset().left + $('#cur_' + cur + '_arrow').width() / 2 - 1
            });

            $('#cur_' + cur + '_info').html(new_val.toFixed(2) + unit).data('cleanval', +new_val);
            var arr_left = $('#cur_' + cur + '_arrow').position().left + 10
            if (w - (arr_left + $('#cur_' + cur + '_info').width()) < 10)
                arr_left = arr_left - $('#cur_' + cur + '_info').width() - 20
            $('#cur_' + cur + '_info').offset({left: cl + arr_left});
        };

        if (min < 1) min = 1;

        setCursor('x1',min,max,CURSORS.cursorsRelative.x1)
        setCursor('x2',min,max,CURSORS.cursorsRelative.x2)


        diff_px = Math.abs($('#cur_x1').offset().left - $('#cur_x2').offset().left) - 6;
        if ($('#cur_x1').is(':visible') && $('#cur_x2').is(':visible') && diff_px > 12) {
            var left = Math.min($('#cur_x1').offset().left, $('#cur_x2').offset().left) + 3;
            var value = $('#cur_x1_info').data('cleanval') - $('#cur_x2_info').data('cleanval');
            $('#cur_x_diff').show()
            $('#cur_x_diff_info').show()

            $('#cur_x_diff')
                .offset({ left: left })
                .width(diff_px)

            $('#cur_x_diff_info')
                .html(Math.abs(value).toFixed(2) + unit)
                .offset({ left: left + diff_px / 2 - 2 - $('#cur_x_diff_info').width() / 2 })

        } else {
            $('#cur_x_diff, #cur_x_diff_info').hide();
        }
    }


    // Y-cursors
    CURSORS.updateYLinesAndArrows = function() {

        const ct = $('#cursors_holder').offset().top;
        const h = $('#cursors_holder').height();

        var diff_px = 0;
        let unit = '';

        var axes = MAIN.graphCache.plot.getAxes();
        let min = axes.yaxis.min;
        let max = axes.yaxis.max;
        let new_val = 0;



        const setCursor = (cur, min,max,t) => {

           new_val = (min - max) * t + max;

            // Update line
            $('#cur_' + cur).offset({
                top: $('#cur_' + cur + '_arrow').offset().top + $('#cur_' + cur + '_arrow').height() / 2 - 1
            });

            $('#cur_' + cur + '_info').html(new_val.toFixed(2) + unit).data('cleanval', +new_val);
            var arr_top = $('#cur_' + cur + '_arrow').position().top + 10
            if (h - (arr_top + $('#cur_' + cur + '_info').height()) < 10)
                arr_top = arr_top - $('#cur_' + cur + '_info').height() - 20
            $('#cur_' + cur + '_info').offset({top: ct + arr_top});
        };

        setCursor('y1',min,max,CURSORS.cursorsRelative.y1)
        setCursor('y2',min,max,CURSORS.cursorsRelative.y2)


        diff_px = Math.abs($('#cur_y1').offset().top - $('#cur_y2').offset().top) - 6;
        if ($('#cur_y1').is(':visible') && $('#cur_y2').is(':visible') && diff_px > 12) {
            var top = Math.min($('#cur_y1').offset().top, $('#cur_y2').offset().top) + 3;
            var value = $('#cur_y1_info').data('cleanval') - $('#cur_y2_info').data('cleanval');
            $('#cur_y_diff').show()
            $('#cur_y_diff_info').show()

            $('#cur_y_diff')
                .offset({top: top })
                .height(diff_px)

            $('#cur_y_diff_info')
                .html(Math.abs(value).toFixed(2) + unit)
                .offset({ top: top + diff_px / 2  - $('#cur_y_diff_info').height() / 2 })

        } else {
            $('#cur_y_diff, #cur_y_diff_info').hide();
        }
    }

    CURSORS.enableCursor = function(x) {
        var x2 = (x[1] == '1') ? x[0] + '2' : x[0] + '1';
        var d = (x[1] == '1') ? '1' : '2';
        $('cur_' + x).show();
        $('cur_' + x + '_info').show();
        $('cur_' + x + '_arrow').show();

        if ($('cur_' + x2).is(':visible')) {
            $('cur_' + x[0] + '_diff').show();
            $('cur_' + x[0] + '_diff_info').show();
        }

        if (x[0] == 'x') {
            CURSORS.enableX('IA_CURSOR_' + x[0].toUpperCase() + d + '_ENABLE', true);
        } else if (x[0] == 'y') {
            CURSORS.enableY('IA_CURSOR_' + x[0].toUpperCase() + d + '_ENABLE', true);
        }
    };

    CURSORS.disableCursor = function(x) {
        var d = (x[1] == '1') ? '1' : '2';

        $('cur_' + x).hide();
        $('cur_' + x + '_info').hide();
        $('cur_' + x + '_arrow').hide();
        $('cur_' + x[0] + '_diff').hide();
        $('cur_' + x[0] + '_diff_info').hide();

        if (x[0] == 'x') {
            CURSORS.enableX('IA_CURSOR_' + x[0].toUpperCase() + d + '_ENABLE', false);
        } else if (x[0] == 'y') {
            CURSORS.enableY('IA_CURSOR_' + x[0].toUpperCase() + d + '_ENABLE', false);
        }
    };

    CURSORS.updateCursors = function() {
        if (MAIN.graphCache === undefined)
            return;
        var plot = MAIN.graphCache.plot;
        var offset = plot.getPlotOffset();
        var diff_left = offset.left + 2 + 'px';
        var diff_top = offset.top - 2 + 'px';

        $('.harrow').css('margin-top', -7);
        $('.harrow').css('margin-bottom', -7);

        $('#cur_x_diff').css('margin-left', diff_left);
        $('#cur_y_diff').css('margin-top', diff_top);
        $('#cur_z_diff').css('margin-top', diff_top);

        $('#cur_x1_info').css({ top: 55});
        $('#cur_x2_info').css({ top: 55});

        $('#cur_y1_info').css({ left: 40});
        $('#cur_y2_info').css({ left: 40});

        CURSORS.updateXLinesPosition()
        CURSORS.updateYLinesPosition()
    };

    CURSORS.updateLinesAndArrows = function() {
        CURSORS.updateXLinesAndArrows();
        CURSORS.updateYLinesAndArrows();
    }

    CURSORS.enableCursorX1Callback = function(new_params){
        var param_name = "IA_CURSOR_X1_ENABLE"
        if (new_params[param_name].value === true){
            CURSORS.enableCursor('x1');
        }else{
            CURSORS.disableCursor('x1');
        }
    }

    CURSORS.enableCursorX2Callback = function(new_params){
        var param_name = "IA_CURSOR_X2_ENABLE"
        if (new_params[param_name].value === true){
            CURSORS.enableCursor('x2');
        }else{
            CURSORS.disableCursor('x2');
        }
    }

    CURSORS.enableCursorY1Callback = function(new_params){
        var param_name = "IA_CURSOR_Y1_ENABLE"
        if (new_params[param_name].value === true){
            CURSORS.enableCursor('y1');
        }else{
            CURSORS.disableCursor('y1');
        }
    }

    CURSORS.enableCursorY2Callback = function(new_params){
        var param_name = "IA_CURSOR_Y2_ENABLE"
        if (new_params[param_name].value === true){
            CURSORS.enableCursor('y2');
        }else{
            CURSORS.disableCursor('y2');
        }
    }

    CURSORS.cursorX1Callback = function(new_params){
        CURSORS.cursorsRelative.x1 = new_params["IA_CURSOR_X1"].value;
        CURSORS.updateXLinesPosition()
        CURSORS.updateXLinesAndArrows()
    }

    CURSORS.cursorX2Callback = function(new_params){
        CURSORS.cursorsRelative.x2 = new_params["IA_CURSOR_X2"].value;
        CURSORS.updateXLinesPosition()
        CURSORS.updateXLinesAndArrows()
    }

    CURSORS.cursorY1Callback = function(new_params){
        CURSORS.cursorsRelative.y1 = new_params["IA_CURSOR_Y1"].value;
        CURSORS.updateYLinesPosition()
        CURSORS.updateYLinesAndArrows();
    }

    CURSORS.cursorY2Callback = function(new_params){
        CURSORS.cursorsRelative.y2 = new_params["IA_CURSOR_Y2"].value;
        CURSORS.updateYLinesPosition()
        CURSORS.updateYLinesAndArrows();
    }


}(window.CURSORS = window.CURSORS || {}, jQuery));


$(function() {

     // X cursor arrows dragging
    $('#cur_x1_arrow, #cur_x2_arrow').draggable({
        axis: 'x',
        containment: 'parent',
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
            CLIENT.parametersCache["IA_CURSOR_X1"] = { value: CURSORS.cursorsRelative.x1 };
            CLIENT.parametersCache["IA_CURSOR_X2"] = { value: CURSORS.cursorsRelative.x2 };
            CLIENT.sendParameters();
        }
    });


    // Y cursor arrows dragging
    $('#cur_y1_arrow, #cur_y2_arrow').draggable({
        axis: 'y',
        containment: 'parent',
        start: function(ev, ui) {
            CURSORS.state.cursor_dragging = true;
        },
        drag: function(ev, ui) {
            CURSORS.moveY(ui);
        },
        stop: function(ev, ui) {
            CURSORS.moveY(ui);
            CURSORS.state.cursor_dragging = false;
            CLIENT.parametersCache["IA_CURSOR_Y1"] = { value: CURSORS.cursorsRelative.y1 };
            CLIENT.parametersCache["IA_CURSOR_Y2"] = { value: CURSORS.cursorsRelative.y2 };
            CLIENT.sendParameters();
        }
    });

    $('#IA_CURSOR_X1_ENABLE').click(function() {
        CLIENT.parametersCache["IA_CURSOR_X1_ENABLE"] = { value: !$(this).hasClass('active') };
        CLIENT.sendParameters();
    });

    $('#IA_CURSOR_X2_ENABLE').click(function() {
        CLIENT.parametersCache["IA_CURSOR_X2_ENABLE"] = { value: !$(this).hasClass('active') };
        CLIENT.sendParameters();
    });

    $('#IA_CURSOR_Y1_ENABLE').click(function() {
        CLIENT.parametersCache["IA_CURSOR_Y1_ENABLE"] = { value: !$(this).hasClass('active') };
        CLIENT.sendParameters();
    });

    $('#IA_CURSOR_Y2_ENABLE').click(function() {
        CLIENT.parametersCache["IA_CURSOR_Y2_ENABLE"] = { value: !$(this).hasClass('active') };
        CLIENT.sendParameters();
    });

})