/*
 * Red Pitaya Spectrum Analizator client
 *
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function(SPEC, $, undefined) {

    SPEC.initCursors = function() {
        var plot = SPEC.getPlot();
        if (!(plot)) {
            return;
        }
        var offset = plot.getPlotOffset();
        var left = offset.left + 1 + 'px';
        var right = offset.right + 1 + 'px';
        var top = offset.top + 1 + 'px';
        var bottom = offset.bottom + 1 + 'px';

        //update lines length
        $('.hline').css('left', left);
        $('.hline').css('right', right);
        $('.vline').css('top', top);
        $('.vline').css('bottom', bottom);

        //update arrows positions
        var diff_left = offset.left + 2 + 'px';
        var diff_top = offset.top - 2 + 'px';

        // Y cursor arrows dragging
        $('#cur_y1_arrow, #cur_y2_arrow').draggable({
            axis: 'y',
            containment: UI_GRAPH.getPlotLimits(),
            start: function(ev, ui) {
                SPEC.state.cursor_dragging = true;
            },
            drag: function(ev, ui) {
                SPEC.updateYCursorElems(ui, false);
            },
            stop: function(ev, ui) {
                SPEC.updateYCursorElems(ui, true);
                SPEC.state.cursor_dragging = false;
            }
        });

        // X cursor arrows dragging
        $('#cur_x1_arrow, #cur_x2_arrow').draggable({
            axis: 'x',
            containment: UI_GRAPH.getPlotLimits(),
            start: function(ev, ui) {
                SPEC.state.cursor_dragging = true;
            },
            drag: function(ev, ui) {
                SPEC.updateXCursorElems(ui, false);
            },
            stop: function(ev, ui) {
                SPEC.updateXCursorElems(ui, true);
                SPEC.state.cursor_dragging = false;
            }
        });

    };

    SPEC.updateYInfo = function(){
        if ($('#cur_y1_arrow').position() === undefined) return
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

    SPEC.updateCursors = function(){
        SPEC.cursorX(CLIENT.params.orig)
        SPEC.cursorY(CLIENT.params.orig)
    }

    SPEC.cursorYUpdate = function(cursor,visible,value) {
        if (!SPEC.state.cursor_dragging) {
            var y = cursor;
            if (visible) {
                var plot = SPEC.getPlot();
                if (plot) {
                    var offset = plot.getPlotOffset();
                    var graph_height = $('#graph_grid').height() - offset.top - offset.bottom;
                    let left = $('#graph_grid').width() - offset.left - offset.right + 26;
                    var top = graph_height * value + offset.top - 2;
                    $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').css('top', top).show();
                    $('#cur_' + y + '_arrow').css('left', left).show();
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
        if (!(plot)) {
            return;
        }
        let key = y == 'y1' ? 'SPEC_CUR1_V' : 'SPEC_CUR2_V'
        let cur2 = y == 'y1' ? 'y2' : 'y1'
        var axes = plot.getAxes();
        var offset = plot.getPlotOffset();
        var graph_h = $('#graph_grid').height();
        var tickSize = axes.yaxis.tickSize;
        var new_value = axes.yaxis.c2p(top - offset.top + 2)
        var dBlabel = SPEC.y_axis_label();

        $('#cur_' + y).css('top', top + 1);
        let v = (new_value.toFixed(Math.abs(tickSize) >= 0.1 ? 2 : 3)) + " " + dBlabel
        $('#cur_' + y + '_info').attr('raw',new_value).html(v);
        let oh = $('#cur_' + y + '_info').outerHeight()
        let laboff = 0;
        let cur2item = $('#cur_' + cur2)
        let top2  =cur2item.css('top')
        let vis = cur2item.is(":visible")
        if (top > parseInt(top2) && vis){
            laboff += oh + 8
        }
        $('#cur_' + y + '_info').css('top', top - 1 + laboff);
        $('#cur_' + y + '_info').css('left', offset.left + 5);

        SPEC.updateYCursorDiff();

        if (save) {
            new_value = (top - offset.top + 2) / (graph_h - offset.top  - offset.bottom);
            CLIENT.parametersCache[key] = { value: new_value };
            CLIENT.params.orig[key] = { value: new_value };
            CLIENT.sendParameters()
        }
    };

    SPEC.updateYCursorDiff = function() {
        var plot = SPEC.getPlot();
        if (!(plot)) {
            return;
        }
        var y1 = $('#cur_y1');
        var y2 = $('#cur_y2');
        var y1_top = parseInt(y1.css('top'));
        var y2_top = parseInt(y2.css('top'));
        var diff_px = Math.abs(y1_top - y2_top) - 6;

        if (y1.is(':visible') && y2.is(':visible') && diff_px > 12) {
            var offset = plot.getPlotOffset();
            var top = Math.min(y1_top, y2_top);
            var value = parseFloat($('#cur_y1_info').html()) - parseFloat($('#cur_y2_info').html());

            $('#cur_y_diff')
                .css('top', top + 5)
                .css('left',offset.left + 15)
                .height(diff_px)
                .show();
            $('#cur_y_diff_info')
                .html(Math.abs(+(value.toFixed(Math.abs(value) >= 0.1 ? 2 : 3))) + " " + SPEC.y_axis_diff_label())
                .css('top', top + diff_px / 2 - 2)
                .css('left',offset.left + 25)
                .show();
        } else {
            $('#cur_y_diff, #cur_y_diff_info').hide();
        }
    };

    SPEC.updateXInfo = function(){
        if ($('#cur_x1_arrow').position() === undefined) return
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
                if (plot) {
                    UI_GRAPH.getPlotLimits()
                    var offset = plot.getPlotOffset();
                    var graph_width = $('#graph_grid').width() - offset.left  - offset.right
                    var top = $('#graph_grid').height() - offset.top - offset.bottom - 18;
                    var left = graph_width * value + offset.left - 2;
                    $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').css('left', left).show();
                    $('#cur_' + x + '_arrow').css('top', top)
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
        if (!(plot)) {
            return;
        }
        var axes = plot.getAxes();
        var offset = plot.getPlotOffset();

        var graph_width = $('#graph_grid').width();
        var new_value = axes.xaxis.c2p(left - offset.left + 2)

        $('#cur_' + x).css('left', left);

        $('#cur_' + x + '_info')
            .attr('raw',new_value)
            .html(SPEC.convertFreqToText(new_value))
        let left_orig = left
        left += 2
        var msg_width = $('#cur_' + x + '_info').outerWidth();
        if (msg_width + left + 12 > graph_width) left = graph_width - msg_width - 12
        $('#cur_' + x + '_info').css('left', left);

        SPEC.updateXCursorDiff();

        if (save) {
            new_value = (left_orig - offset.left + 2) / (graph_width - offset.left  - offset.right);
            let key = x == 'x1' ? 'SPEC_CUR1_T' : 'SPEC_CUR2_T'
            CLIENT.parametersCache[key] = { value: new_value };
            CLIENT.params.orig[key] = { value: new_value };
            CLIENT.sendParameters()
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
            var value = parseFloat($('#cur_x1_info').attr('raw')) - parseFloat($('#cur_x2_info').attr('raw'));
            value = Math.abs(value)
            $('#cur_x_diff')
                .css('left', left + 1)
                .width(diff_px)
                .show();
            $('#cur_x_diff_info')
                .html(SPEC.convertFreqToText(value))
                .show()
                .css('left', left + diff_px / 2 - $('#cur_x_diff_info').width() / 2 + 3);
        } else {
            $('#cur_x_diff, #cur_x_diff_info').hide();
        }
    };

}(window.SPEC = window.SPEC || {}, jQuery));