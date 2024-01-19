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

    CURSORS.moveX = function(ui) {
        var graphSize = MAIN.graphSize;

        if (ui.helper[0].id == 'cur_x1_arrow') {
            CURSORS.cursorsRelative.x1 = ($("#cur_x1_arrow").offset().left - graphSize.left) / (graphSize.right - graphSize.left);
            if (CURSORS.cursorsRelative.x1 < 0)
                CURSORS.cursorsRelative.x1 = 0;
            if (CURSORS.cursorsRelative.x1 > 1)
                CURSORS.cursorsRelative.x1 = 1;

        } else if (ui.helper[0].id == 'cur_x2_arrow') {
            CURSORS.cursorsRelative.x2 = ($("#cur_x2_arrow").offset().left - graphSize.left) / (graphSize.right - graphSize.left);
            if (CURSORS.cursorsRelative.x2 < 0)
                CURSORS.cursorsRelative.x2 = 0;
            if (CURSORS.cursorsRelative.x2 > 1)
                CURSORS.cursorsRelative.x2 = 1;
        }
        CURSORS.updateXLinesAndArrows();
    }

    CURSORS.enableX = function(cursor_name, new_params) {
        if (!CURSORS.state.cursor_dragging && !CURSORS.state.mouseover) {
            var x = (cursor_name == 'BA_CURSOR_X1' ? 'x1' : 'x2');

            if (new_params[cursor_name].value) {
                $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').show();
            } else {
                $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').hide();
            }
            CURSORS.updateXLinesAndArrows();
            CURSORS.installCursorsHandlers();
        }
    }




    CURSORS.enableY = function(cursor_name, new_params) {
        if (!CURSORS.state.cursor_dragging && !CURSORS.state.mouseover) {
            var y = (cursor_name == 'BA_CURSOR_Y1' ? 'y1' : 'y2');

            if (new_params[cursor_name].value) {
                $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').show();
            } else {
                $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').hide();
            }
            CURSORS.updateYLinesAndArrows();
            CURSORS.installCursorsHandlers();
        }
    }

    CURSORS.moveY = function(ui) {
        var graphSize = MAIN.graphSize;

        if (ui.helper[0].id == 'cur_y1_arrow') {
            CURSORS.cursorsRelative.y1 = ($("#cur_y1_arrow").offset().top - graphSize.top) / (graphSize.bottom - graphSize.top);
            if (CURSORS.cursorsRelative.y1 < 0)
                CURSORS.cursorsRelative.y1 = 0;
            if (CURSORS.cursorsRelative.y1 > 1)
                CURSORS.cursorsRelative.y1 = 1;

        } else if (ui.helper[0].id == 'cur_y2_arrow') {
            CURSORS.cursorsRelative.y2 = ($("#cur_y2_arrow").offset().top - graphSize.top) / (graphSize.bottom - graphSize.top);
            if (CURSORS.cursorsRelative.y2 < 0)
                CURSORS.cursorsRelative.y2 = 0;
            if (CURSORS.cursorsRelative.y2 > 1)
                CURSORS.cursorsRelative.y2 = 1;
        }
        CURSORS.updateYLinesAndArrows();
    }

    // X-cursors
    CURSORS.updateXLinesAndArrows = function() {

        var axes = MAIN.graphCache.plot.getAxes();
        var scale = MAIN.curGraphScale;
        var graphSize = MAIN.graphSize;

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
            left: graphSize.left + ((graphSize.right - graphSize.left) * CURSORS.cursorsRelative.x1),
            top: graphSize.bottom - 17
        });
        $('#cur_x1, #cur_x1_info').offset({
            left: $('#cur_x1_arrow').offset().left + $('#cur_x1_arrow').width() / 2
        });
        $('#cur_x1_info').offset({ top: graphSize.top + $('#cur_x1_info').outerHeight() + 30 });
        $('#cur_x1_info').html(new_val.toFixed(2) + unit).data('cleanval', +new_val);
        $('#cur_x1').height(graphSize.bottom - graphSize.top);


        if (scale) {
            var a = Math.log10(min);
            var b = Math.log10(max);
            var c = (b - a) * CURSORS.cursorsRelative.x2
            new_val = Math.pow(10, a + c);

        } else {
            new_val = (max - min) * CURSORS.cursorsRelative.x2 + min;
        }

        $('#cur_x2_arrow').offset({
            left: graphSize.left + ((graphSize.right - graphSize.left) * CURSORS.cursorsRelative.x2),
            top: graphSize.bottom - 17
        });
        $('#cur_x2, #cur_x2_info').offset({
            left: $('#cur_x2_arrow').offset().left + $('#cur_x2_arrow').width() / 2
        });
        $('#cur_x2_info').offset({ top: graphSize.top + $('#cur_x2_info').outerHeight() + 30 });
        $('#cur_x2_info').html(new_val.toFixed(2) + unit).data('cleanval', +new_val);
        $('#cur_x2').height(graphSize.bottom -graphSize.top);


        diff_px = Math.abs($('#cur_x1').offset().left - $('#cur_x2').offset().left) - 6;
        if ($('#cur_x1').is(':visible') && $('#cur_x2').is(':visible') && diff_px > 12) {
            var left = Math.min($('#cur_x1').offset().left, $('#cur_x2').offset().left) + 3;
            var value = $('#cur_x1_info').data('cleanval') - $('#cur_x2_info').data('cleanval');

            $('#cur_x_diff')
                .offset({ left: left })
                .width(diff_px)
                .show();
            $('#cur_x_diff_info')
                .html(Math.abs(value).toFixed(2) + unit)
                .offset({ left: left + diff_px / 2 - 2 - $('#cur_x_diff_info').outerWidth() / 2 })
                .show();
        } else {
            $('#cur_x_diff, #cur_x_diff_info').hide();
        }
    }


    // Y-cursors
    CURSORS.updateYLinesAndArrows = function() {
        var graphSize = MAIN.graphSize;

        var diff_px = 0;
        let unit = ' dB';
        let min = parseFloat($('#BA_GAIN_MIN').val());
        let max = parseFloat($('#BA_GAIN_MAX').val());
        let new_val = 0;


        new_val = (min - max) * CURSORS.cursorsRelative.y1 + max;
        console.log(CURSORS.cursorsRelative.y1)
        $('#cur_y1_arrow').offset({
            left: graphSize.right + $('#cur_y1_arrow').width() / 2 + 2,
            top: graphSize.top + ((graphSize.bottom - graphSize.top) * CURSORS.cursorsRelative.y1)
        });

        $('#cur_y1, #cur_y1_info').offset({
            top: $('#cur_y1_arrow').offset().top + $('#cur_y1_arrow').height() / 2
        });
        $('#cur_y1_info').offset({ left: graphSize.left + $('#cur_y1_info').outerWidth() });
        $('#cur_y1_info').html(new_val.toFixed(2) + unit).data('cleanval', +new_val);
        $('#cur_y1').width(graphSize.right - graphSize.left);


        new_val = (min - max) * CURSORS.cursorsRelative.y2 + max;

        $('#cur_y2_arrow').offset({
            left: graphSize.right + $('#cur_y2_arrow').width() / 2 + 2,
            top: graphSize.top + ((graphSize.bottom - graphSize.top) * CURSORS.cursorsRelative.y2)
        });

        $('#cur_y2, #cur_y2_info').offset({
            top: $('#cur_y2_arrow').offset().top + $('#cur_y2_arrow').height() / 2
        });
        $('#cur_y2_info').offset({ left: graphSize.left + $('#cur_y2_info').outerWidth() });
        $('#cur_y2_info').html(new_val.toFixed(2) + unit).data('cleanval', +new_val);;
        $('#cur_y2').width(graphSize.right - graphSize.left);


        diff_px = Math.abs($('#cur_y1').offset().top - $('#cur_y2').offset().top) - 6;
        if ($('#cur_y1').is(':visible') && $('#cur_y2').is(':visible') && diff_px > 12) {
            var top = Math.min($('#cur_y1').offset().top, $('#cur_y2').offset().top) + 3;
            var value = $('#cur_y1_info').data('cleanval') - $('#cur_y2_info').data('cleanval');

            $('#cur_y_diff')
                .offset({ left: graphSize.left + 50, top: top })
                .height(diff_px)
                .show();
            $('#cur_y_diff_info')
                .html(Math.abs(value).toFixed(2) + unit)
                .offset({ left: $('#cur_y_diff').offset().left, top: top + diff_px / 2 - 2 })
                .show();
        } else {
            $('#cur_y_diff, #cur_y_diff_info').hide();
        }
    }

    CURSORS.calculateYLimit = function(){
        if (MAIN.graphCache === undefined) return [0,0,0,0];
        var plot = MAIN.graphCache.plot;
        if (!plot) {
            return [0,0,0,0];
        }

        var gPosition = $('#graph_bode').offset();
        var gLeft = gPosition.left;
        var gTop = gPosition.top;
        var gWidth = $('#graph_bode').width();
        var gHeight = $('#graph_bode').height();
        var plotOffset = plot.getPlotOffset();

        gLeft += plotOffset.left
        gTop += plotOffset.top
        gWidth = gWidth - plotOffset.left - plotOffset.right
        gHeight = gHeight - plotOffset.bottom  + gTop
        return [gLeft,gTop,gWidth,gHeight]
    }

    CURSORS.installCursorsHandlers = function(){
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
            }
        });


        // Y cursor arrows dragging
        $('#cur_y1_arrow, #cur_y2_arrow').draggable({
            axis: 'y',
            containment: CURSORS.calculateYLimit(),
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

}(window.CURSORS = window.CURSORS || {}, jQuery));


$(function() {
    CURSORS.installCursorsHandlers();


})