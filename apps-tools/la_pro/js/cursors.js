/*
 * Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function(LA, $, undefined) {
    // App state
    LA.state = {
        cursor_dragging: false,
        line_moving: false,
        resized: false,
    };

    LA.initCursors = function() {
        var plot = LA.getPlot();
        if (plot === undefined) {
            return;
        }
        var offset = plot.getPlotOffset();
        var left = offset.left + 1 + 'px';
        var right = offset.right + 1 + 'px';
        var top = offset.top + 1 + 'px';
        var bottom = offset.bottom - 1 + 'px';

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

    LA.updateCursors = function(){
        LA.cursorX(CLIENT.params.orig)
    }

    LA.updateXInfo = function(){
        LA.updateXCursorElemsTop('x1',$('#cur_x1_arrow').position().left,false);
        LA.updateXCursorElemsTop('x2',$('#cur_x2_arrow').position().left,false);
    }

    LA.cursorX = function(new_params){
        for(var i = 1 ; i <= 2; i++){
            var x = CLIENT.getValue("LA_CURSOR_X" + i)
            var x_pos = CLIENT.getValue("LA_CURSOR_X"+i+"_POS")
            if (x !== undefined && x_pos !== undefined) {
                LA.cursorXUpdate(i, x, x_pos);
            }
        }
    }

    LA.cursorXUpdate = function(cursor,visible,value) {
        if (!LA.state.cursor_dragging) {
            var x = 'x'+cursor;

            if (visible) {
                var plot = LA.getPlot();
                if (plot !== undefined) {
                    var offset = plot.getPlotOffset();
                    var graph_width = $('#graph_grid').width() - offset.left - offset.right;
                    var left = graph_width * value;
                    $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').css('left', left).show();
                    LA.updateXCursorElemsTop(x,left,false);
                }
                $('#LA_CURSOR_X'+cursor).addClass('active')
            } else {
                $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').hide();
                $('#LA_CURSOR_X'+cursor).removeClass('active')
                LA.updateXCursorDiff()
            }
        }
    }

    LA.updateXCursorElems = function(ui, save) {
        var x = (ui.helper[0].id == 'cur_x1_arrow' ? 'x1' : 'x2');
        LA.updateXCursorElemsTop(x,ui.position.left,save);
    }

    LA.updateXCursorElemsTop = function(x,left, save) {
        var plot = LA.getPlot();
        if (plot === undefined) {
            return;
        }
        var samplerate = CLIENT.getValue("LA_CUR_FREQ")
        var scale = CLIENT.getValue("LA_SCALE")
        if (samplerate !== undefined && scale !== undefined){
            var axes = plot.getAxes();
            var offset = plot.getPlotOffset();
            var graph_width = $('#graph_grid').width() - offset.left - offset.right;
            var mul = 1000;
            var dev_num = 10;
            var timePerDevInMs = (((graph_width / scale) / samplerate) * mul) / dev_num;
            var ms_per_px = (timePerDevInMs * 10) / graph_width;
            var msg_width = $('#cur_' + x + '_info').outerWidth();
            var new_value = axes.xaxis.min + left * ms_per_px;
            $('#cur_' + x + ', #cur_' + x + '_info').css('left', left);

            $('#cur_' + x + '_info')
                .html(OSC.convertTime(new_value))
                .attr('value',new_value)
                .css('margin-left', (left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));

            LA.updateXCursorDiff();
        }

        if (save) {
            new_value = left / graph_width;
            CLIENT.parametersCache[x == 'x1' ? 'LA_CURSOR_X1_POS' : 'LA_CURSOR_X2_POS'] = { value: new_value };
            CLIENT.sendParameters();
        }
    };

    LA.updateXCursorDiff = function() {
        var x1 = $('#cur_x1');
        var x2 = $('#cur_x2');
        var x1_left = parseInt(x1.css('left'));
        var x2_left = parseInt(x2.css('left'));
        var diff_px = Math.abs(x1_left - x2_left) - 9;

        if (x1.is(':visible') && x2.is(':visible') && diff_px > 30) {
            var left = Math.min(x1_left, x2_left);
            var value = parseFloat($('#cur_x1_info').attr('value')) - parseFloat($('#cur_x2_info').attr('value'));
            // var plot = LA.getPlot();

            $('#cur_x_diff')
                .css('left', left + 1)
                .width(diff_px)
                .show();
            $('#cur_x_diff_info')
                .html(OSC.convertTime(Math.abs(value)))
                .show()
                .css('left', left + diff_px / 2 - $('#cur_x_diff_info').width() / 2 + 3);
        } else {
            $('#cur_x_diff, #cur_x_diff_info').hide();
        }
    };

    LA.enableCursor = function(x) {
        CLIENT.parametersCache['LA_CURSOR_X'+x] = { value: true };
        CLIENT.sendParameters();
    };

    LA.disableCursor = function(x) {
        CLIENT.parametersCache['LA_CURSOR_X'+x] = { value: false };
        CLIENT.sendParameters();
    };

    LA.updateChannels = function(){
        for(var i = 0 ; i < 8; i++){
            LA.updateChVisibility(i)
        }
    }

    LA.updateChVisibility = function(ch) {
        var getTName = function(t){
            if (t == 1) return '(0)'
            if (t == 2) return '(1)'
            if (t == 3) return '(R)'
            if (t == 4) return '(F)'
            if (t == 5) return '(E)'
            return ''
        }
        var visible = CLIENT.getValue("LA_DIN_" + (ch + 1))
        var trigger = CLIENT.getValue("LA_DIN_" + (ch + 1) + "_TRIGGER")
        var ch_name = CLIENT.getValue("LA_DIN_NAME_" + (ch+1))
        var arrow = $('#ch' + (ch + 1) + '_offset_arrow');
        var arrow_img = $('#ch' + (ch + 1) + '_offset_arrow .ch_arrow');
        var arrow_img_info = $('#ch' + (ch + 1) + '_offset_arrow .ch_info_arrow');
        var arrow_img_info_span = $('#ch' + (ch + 1) + '_offset_arrow #ch' + (ch + 1) + '_info');

        if (visible === true) {
            var pos = CLIENT.getValue("LA_DIN_" +(ch+1)+ "_POS")
            if (pos !== undefined){
                var grid = $('#graph_grid');
                var volt_per_px = grid.height() / 9;
                var px_offset = grid.height() - (pos * volt_per_px);
                OSC.state.graph_grid_height = grid.height();
                arrow.css('top', px_offset).show();
            }
        }
        if (ch_name == "") {
            ch_name = "DIN" + ch;
        }
        var ch_name_l = ch_name
        if (trigger !== undefined && trigger !== 0){
            arrow_img.attr("src","img/ch" + (ch + 1) + "-offset-arrow_long.png");
            arrow_img_info.css('left','41px')
            arrow_img_info_span.css('left','53px')
            arrow.find('#CH' + (ch + 1) + '_LABEL').css('width','50px')
            ch_name_l += ' ' + getTName(trigger)
        }else{
            arrow_img.attr("src","img/ch" + (ch + 1) + "-offset-arrow.png");
            arrow_img_info.css('left','25px')
            arrow_img_info_span.css('left','40px')
            arrow.find('#CH' + (ch + 1) + '_LABEL').css('width','30px')
        }

        $('#CH' + (ch + 1) + '_NAME').val(ch_name);
        arrow.find('#CH' + (ch + 1) + '_LABEL').text(ch_name_l);

        LA.showInfoArrow(ch);
    }

    OSC.updateYOffset = function(ui, save) {
        var graph_height = $('#graph_grid').height();
        var arrows = ["ch1_offset_arrow", "ch2_offset_arrow", "ch3_offset_arrow", "ch4_offset_arrow",
            "ch5_offset_arrow", "ch6_offset_arrow", "ch7_offset_arrow", "ch8_offset_arrow"
        ];
        var ch = arrows.indexOf(ui.helper[0].id);
        if (ch != -1) {
            var mtop = parseFloat(ui.helper.css('top')) * 9.0 / graph_height
            var new_value = 9 - mtop

            OSC.guiHandler(); // Update signals
            if (new_value !== undefined && save) {
                CLIENT.parametersCache["LA_DIN_" +(ch+1)+ "_POS"] = { value : new_value }
                CLIENT.sendParameters();
            }else{
                CLIENT.params.orig["LA_DIN_" +(ch+1)+ "_POS"] = { value : new_value }
            }
            LA.setupDataToGraph()
        }
    };

}(window.LA = window.LA || {}, jQuery));
