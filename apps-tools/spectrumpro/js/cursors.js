(function(SPEC, $, undefined) {

    SPEC.initCursors = function() {
        var plot = SPEC.getPlot();
        if (!(SPEC.isVisibleChannels() && plot)) {
            return;
        }
        var offset = plot.getPlotOffset();
        var left = offset.left + 1 + 'px';
        var right = offset.right + 1 + 'px';
        var top = offset.top + 1 + 'px';
        var bottom = offset.bottom + 9 + 'px';

        //update lines length
        $('.hline').css('left', left);
        $('.hline').css('right', right);
        $('.vline').css('top', top);
        $('.vline').css('bottom', bottom);

        //update arrows positions
        var diff_left = offset.left + 2 + 'px';
        var diff_top = offset.top - 2 + 'px';
        var margin_left = offset.left - 7 - 2 + 'px';
        var margin_top = -7 + offset.top - 2 + 'px';
        var margin_bottom = -2 + offset.bottom + 'px';
        var line_margin_left = offset.left - 2 + 'px';
        var line_margin_top = offset.top - 2 + 'px';
        var line_margin_bottom = offset.bottom - 2 + 'px';

        $('.varrow').css('margin-left', margin_left);
        $('.harrow').css('margin-top', margin_top);
        $('.harrow').css('margin-bottom', margin_bottom);
        $('.vline').css('margin-left', line_margin_left);
        $('.hline').css('margin-top', line_margin_top);
        $('.hline').css('margin-bottom', line_margin_bottom);

        $('#cur_x_diff').css('margin-left', diff_left);
        $('#cur_y_diff').css('margin-top', diff_top);
        $('#cur_x_diff_info').css('margin-left', diff_left);
        $('#cur_y_diff_info').css('margin-top', diff_top);
    };

    SPEC.updateYInfo = function(){
        SPEC.updateYCursorElemsTop('y1',$('#cur_y1_arrow').position().top,false);
        SPEC.updateYCursorElemsTop('y2',$('#cur_y2_arrow').position().top,false);
    }

    SPEC.cursorY = function(new_params){
        if ('SPEC_CURSOR_Y1' in new_params && 'SPEC_CUR1_V' in new_params) {
            SPEC.cursorYUpdate('y1',new_params['SPEC_CURSOR_Y1'].value, new_params['SPEC_CUR1_V'].value);
        }
        if ('SPEC_CURSOR_Y2' in new_params && 'SPEC_CUR2_V' in new_params) {
            SPEC.cursorYUpdate('y2',new_params['SPEC_CURSOR_Y2'].value, new_params['SPEC_CUR2_V'].value);
        }
    }

    SPEC.cursorYUpdate = function(cursor,visible,value) {
        if (!SPEC.state.cursor_dragging) {
            var y = cursor;
            if (visible) {
                var plot = SPEC.getPlot();
                if (SPEC.isVisibleChannels() && plot) {
                    var offset = plot.getPlotOffset();
                    var graph_height = $('#graph_grid').height() - offset.top - offset.bottom;
                    var top = (graph_height + 7) * value;
                    $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').css('top', top).show();
                    SPEC.updateYCursorElemsTop(y,top,false);
                }
            } else {
                $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').hide();
            }
        }
    }

    SPEC.updateYCursorElems = function(ui, save) {
        var y = (ui.helper[0].id == 'cur_y1_arrow' ? 'y1' : 'y2');
        SPEC.updateYCursorElemsTop(y,ui.position.top,save);
    }

    SPEC.updateYCursorElemsTop = function(y, top, save) {
        var plot = SPEC.getPlot();
        if (!(SPEC.isVisibleChannels() && plot)) {
            return;
        }
        var axes = plot.getAxes();

        var volt_per_px = 1 / axes.yaxis.scale;
        var tickSize = axes.yaxis.tickSize;
        var new_value = axes.yaxis.max - top * volt_per_px;
        var dBlabel = SPEC.y_axis_label();

        $('#cur_' + y + ', #cur_' + y + '_info').css('top', top);
        $('#cur_' + y + '_info').html((new_value.toFixed(Math.abs(tickSize) >= 0.1 ? 2 : 3)) + " " + dBlabel).css('margin-top', (top < 16 ? 3 : ''));

        SPEC.updateYCursorDiff();

        if (save) {
            new_value = 1.0 - (new_value  - axes.yaxis.min) / (axes.yaxis.max - axes.yaxis.min);
            SPEC.params.local[y == 'y1' ? 'SPEC_CUR1_V' : 'SPEC_CUR2_V'] = { value: new_value };
            SPEC.sendParams();
        }
    };

    SPEC.updateYCursorDiff = function() {
        var y1 = $('#cur_y1');
        var y2 = $('#cur_y2');
        var y1_top = parseInt(y1.css('top'));
        var y2_top = parseInt(y2.css('top'));
        var diff_px = Math.abs(y1_top - y2_top) - 6;

        if (y1.is(':visible') && y2.is(':visible') && diff_px > 12) {
            var top = Math.min(y1_top, y2_top);
            var value = parseFloat($('#cur_y1_info').html()) - parseFloat($('#cur_y2_info').html());

            $('#cur_y_diff')
                .css('top', top + 5)
                .height(diff_px)
                .show();
            $('#cur_y_diff_info')
                .html(Math.abs(+(value.toFixed(Math.abs(value) >= 0.1 ? 2 : 3))) + " " + SPEC.y_axis_diff_label())
                .css('top', top + diff_px / 2 - 2)
                .show();
        } else {
            $('#cur_y_diff, #cur_y_diff_info').hide();
        }
    };

    SPEC.updateXInfo = function(){
        SPEC.updateXCursorElemsTop('x1',$('#cur_x1_arrow').position().left,false);
        SPEC.updateXCursorElemsTop('x2',$('#cur_x2_arrow').position().left,false);
    }

    SPEC.cursorX = function(new_params){
        if ('SPEC_CURSOR_X1' in new_params && 'SPEC_CUR1_T' in new_params) {
            SPEC.cursorXUpdate('x1',new_params['SPEC_CURSOR_X1'].value, new_params['SPEC_CUR1_T'].value);
        }
        if ('SPEC_CURSOR_X2' in new_params && 'SPEC_CUR2_T' in new_params) {
            SPEC.cursorXUpdate('x2',new_params['SPEC_CURSOR_X2'].value, new_params['SPEC_CUR2_T'].value);
        }
    }

    SPEC.cursorXUpdate = function(cursor,visible,value) {
        if (!SPEC.state.cursor_dragging) {
            var x = cursor;

            if (visible) {

                var plot = SPEC.getPlot();
                if (SPEC.isVisibleChannels() && plot) {
                    var offset = plot.getPlotOffset();
                    var graph_width = $('#graph_grid').width() - offset.left - offset.right;
                    var left = graph_width * value;
                    $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').css('left', left).show();
                    SPEC.updateXCursorElemsTop(x,left,false);
                }
            } else {
                $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').hide();
            }
        }
    }

    SPEC.updateXCursorElems = function(ui, save) {
        var x = (ui.helper[0].id == 'cur_x1_arrow' ? 'x1' : 'x2');
        SPEC.updateXCursorElemsTop(x,ui.position.left,save);
    }

    SPEC.updateXCursorElemsTop = function(x,left, save) {
        var plot = SPEC.getPlot();
        if (!(SPEC.isVisibleChannels() && plot)) {
            return;
        }
        var axes = plot.getAxes();
        var offset = plot.getPlotOffset();
        var tickDec = axes.xaxis.tickDecimals  + 1;

        var graph_width = $('#graph_grid').width() - offset.left - offset.right;
        var ms_per_px = 1 / axes.xaxis.scale;
        var msg_width = $('#cur_' + x + '_info').outerWidth();
        var new_value = axes.xaxis.min + left * ms_per_px;

        $('#cur_' + x + ', #cur_' + x + '_info').css('left', left);

        var unit = SPEC.freq_unit[SPEC.params.orig['freq_unit'].value];
        if (UI_GRAPH.x_axis_mode === 1){
            new_value = UI_GRAPH.convertLog(new_value);
        }
        console.log(new_value)
        $('#cur_' + x + '_info')
            .html((new_value.toFixed(tickDec) + ' ' + unit))
            .css('margin-left', (left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));

        SPEC.updateXCursorDiff();

        if (save) {
            // new_value = (new_value  - axes.xaxis.min) / (axes.xaxis.max - axes.xaxis.min);
            new_value = left / graph_width;
            SPEC.params.local[x == 'x1' ? 'SPEC_CUR1_T' : 'SPEC_CUR2_T'] = { value: new_value };
            SPEC.sendParams();
        }
    };

    SPEC.updateXCursorDiff = function() {
        var x1 = $('#cur_x1');
        var x2 = $('#cur_x2');
        var x1_left = parseInt(x1.css('left'));
        var x2_left = parseInt(x2.css('left'));
        var diff_px = Math.abs(x1_left - x2_left) - 9;

        if (x1.is(':visible') && x2.is(':visible') && diff_px > 30) {
            var left = Math.min(x1_left, x2_left);
            var value = parseFloat($('#cur_x1_info').html()) - parseFloat($('#cur_x2_info').html());
            var unit = SPEC.freq_unit[SPEC.params.orig['freq_unit'].value];
            var plot = SPEC.getPlot();
            var axes = plot.getAxes();
            var tickDec = axes.xaxis.tickDecimals + 1;

            $('#cur_x_diff')
                .css('left', left + 1)
                .width(diff_px)
                .show();
            $('#cur_x_diff_info')
                .html((value.toFixed(tickDec) + ' ' + unit))
                .show()
                .css('left', left + diff_px / 2 - $('#cur_x_diff_info').width() / 2 + 3);
        } else {
            $('#cur_x_diff, #cur_x_diff_info').hide();
        }
    };

}(window.SPEC = window.SPEC || {}, jQuery));