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

            var unit = ""//SPEC.freq_unit[CLIENT.params.orig['freq_unit'].value];

            $('#cur_' + x + '_info')
                .html(OSC.convertTime(new_value))
                .attr('value',new_value)
                .css('margin-left', (left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));

            LA.updateXCursorDiff();
        }

        if (save) {
            // new_value = (new_value  - axes.xaxis.min) / (axes.xaxis.max - axes.xaxis.min);
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
/*
    OSC.cursor_x = function() {
        if (!OSC.state.cursor_dragging) {
            var scale = CLIENT.getValue("LA_SCALE")
            var speed = CLIENT.getValue("LA_CUR_FREQ")
            if (scale !== undefined && speed !== undefined){
                for(var i = 1 ; i <= 2; i++){
                    var x = 'x' + i;
                    var c_out_name = 'LA_CURSOR_X' + i;
                    var c_pos_name = 'LA_CURSOR_X'+i+'_POS';

                    var x_cur = CLIENT.getValue(c_out_name)
                    var x_cur_pos = CLIENT.getValue(c_pos_name)


                    // if (x_cur && x_cur_pos) {
                    //     var new_value = new_params[x == 'x1' ? 'LA_CURSOR_X1_POS' : 'LA_CURSOR_X2_POS'].value;
                    //     var graph_width = $('#graph_grid').width();

                    //     // Calculate time per division
                    //     // var samplerate = OSC.state.acq_speed;
                    //     // var samples = 1024;
                    //     // var mul = 1000;
                    //     // var scale = OSC.time_scale;
                    //     // var timePerDevInMs = ((samples / scale) / samplerate) * mul;
                    //     // var ms_per_px = timePerDevInMs / graph_width;

                    //     var px_offset = -(parseInt($('#cur_' + x + '_arrow').css('margin-left')) / 2 - 2.5);
                    //     var msg_width = $('#cur_' + x + '_info').outerWidth();
                    //     var left = (graph_width + 2) / 2 + px_offset;

                    //     var overflow = false;
                    //     if (left < 0) {
                    //         left = 0;
                    //         overflow = true;
                    //     }
                    //     if (left > graph_width) {
                    //         left = graph_width;
                    //         overflow = true;
                    //     }

                    //     $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').css('left', left).show();
                    //     $('#cur_' + x + '_info')
                    //         .html(OSC.convertTime(-new_value))
                    //         .data('cleanval', -new_value)
                    //         .css('margin-left', (left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));

                    //     if (overflow)
                    //         $('#cur_' + x + '_info').hide();
                    // } else {
                    //     $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').hide();
                    // }
                    if (x_cur !== undefined && x_cur_pos !== undefined){
                        if (x_cur) {

                            var graph_width = $('#graph_grid').width();
                            var samples = 1024;
                            var mul = 1000;
                            var timePerDevInMs = ((graph_width / scale) / speed) * mul;
                            var ms_per_px = timePerDevInMs / graph_width;

                            var ms_per_px = (ts * 10.0) / graph_width;
                            var px_offset = -(x_cur_pos / ms_per_px - parseInt($('#cur_' + x + '_arrow').css('margin-left')) / 2 - 2.5);
                            var msg_width = $('#cur_' + x + '_info').outerWidth();
                            var left = (graph_width + 2) / 2 + px_offset;

                            var overflow = false;
                            if (left < 0) {
                                left = 0;
                                overflow = true;
                            }
                            if (left > graph_width) {
                                left = graph_width;
                                overflow = true;
                            }
                            $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').css('left', left).show();
                            $('#cur_' + x + '_info')
                                .html(OSC.convertTime(-x_cur_pos))
                                .data('cleanval', -x_cur_pos)
                                .css('margin-left', (left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));

                            if (overflow)
                                $('#cur_' + x + '_info').hide();
                        } else {
                            $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').hide();
                        }

                        var field = $('#' + c_out_name);
                        if (field.is('button')) {
                            field[x_cur === true? 'addClass' : 'removeClass']('active');
                        }
                    }
                }
                OSC.updateXCursorDiff();
            }
        }
    }

    OSC.cursor_x1 = function(new_params) {
        OSC.cursor_x();
    }

    OSC.cursor_x2 = function(new_params) {
        OSC.cursor_x();
    }

    OSC.checkAndShowArrows = function() {
        var trigPosInPoints = (OSC.samples_sum * OSC.time_scale - OSC.counts_offset) * $('#graphs').width() / 1024 - $('#time_offset_arrow').width() / 2;
        $('#time_offset_arrow').css('left', trigPosInPoints);
        $("#time_offset_arrow").show();
        if ($('#OSC_CURSOR_X1').hasClass('active'))
            $('#cur_x1_arrow').show();
        if ($('#OSC_CURSOR_X2').hasClass('active'))
            $('#cur_x2_arrow').show();
    }


    // Sets default values for cursors, if values not yet defined
    OSC.updateCursorWithNewScale = function() {
        var graph_height = $('#graph_grid').height();
        var graph_width = $('#graph_grid').width();

        // Calculate time per division
        var samplerate = OSC.state.acq_speed;
        var samples = 1024;
        var mul = 1000;
        var scale = OSC.time_scale;
        var timePerDevInMs = (((samples / scale) / samplerate) * mul);
        var ms_per_px = timePerDevInMs / graph_width;

        // Default value for X1 cursor is 1/4 from graph width
        if ($('#cur_x1').is(':visible')) {
            var left = parseInt($('#cur_x1').css('left'));

            var msg_width = $('#cur_x1' + '_info').outerWidth();
            var new_value = (graph_width / 2 - left - (10 - 2) / 2 - parseInt($('#graph_grid').css('margin-left'))) * ms_per_px;
            $('#cur_x1' + '_info')
                .html(OSC.convertTime(-new_value))
                .data('cleanval', -new_value)
                .css('margin-left', (left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));
        }

        // Default value for X2 cursor is 1/3 from graph width
        if ($('#cur_x2').is(':visible')) {
            var left = parseInt($('#cur_x2').css('left'));

            var msg_width = $('#cur_x2' + '_info').outerWidth();
            var new_value = (graph_width / 2 - left - (10 - 2) / 2 - parseInt($('#graph_grid').css('margin-left'))) * ms_per_px;
            $('#cur_x2' + '_info')
                .html(OSC.convertTime(-new_value))
                .data('cleanval', -new_value)
                .css('margin-left', (left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));
        }

        OSC.updateXCursorDiff();
    };

    // Sets default values for cursors, if values not yet defined
    OSC.setDefCursorVals = function() {
        var graph_height = $('#graph_grid').height();
        var graph_width = $('#graph_grid').width();

        // Calculate time per division
        var samplerate = OSC.state.acq_speed;
        var samples = 1024;
        var mul = 1000;
        var scale = OSC.time_scale;
        var timePerDevInMs = (((samples / scale) / samplerate) * mul);
        var ms_per_px = timePerDevInMs / graph_width;

        // Default value for X1 cursor is 1/4 from graph width
        if ($('#cur_x1').is(':visible')) {
            var left = graph_width * 0.25;

            var msg_width = $('#cur_x1' + '_info').outerWidth();
            var new_value = (graph_width / 2 - left - (10 - 2) / 2 - parseInt($('#graph_grid').css('margin-left'))) * ms_per_px;
            $('#cur_x1' + '_arrow, #cur_x1' + ', #cur_x1' + '_info').show();
            $('#cur_x1' + ', #cur_x1' + '_info').css('left', left);
            $('#cur_x1' + '_info')
                .html(OSC.convertTime(-new_value))
                .data('cleanval', -new_value)
                .css('margin-left', (left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));

            $('#cur_x1_arrow, #cur_x1').css('left', left).show();
            $('#cur_x1').data('init', true);
        }

        // Default value for X2 cursor is 1/3 from graph width
        if ($('#cur_x2').is(':visible')) {
            var left = graph_width * 0.33;

            var msg_width = $('#cur_x2' + '_info').outerWidth();
            var new_value = (graph_width / 2 - left - (10 - 2) / 2 - parseInt($('#graph_grid').css('margin-left'))) * ms_per_px;
            $('#cur_x2' + '_arrow, #cur_x2' + ', #cur_x2' + '_info').show();
            $('#cur_x2' + ', #cur_x2' + '_info').css('left', left);
            $('#cur_x2' + '_info')
                .html(OSC.convertTime(-new_value))
                .data('cleanval', -new_value)
                .css('margin-left', (left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));

            $('#cur_x2_arrow, #cur_x2').css('left', left).show();
            $('#cur_x2').data('init', true);
        }

        OSC.updateXCursorDiff();
    };

    // Updates all elements related to a X cursor
    OSC.updateXCursorElems = function(ui, save) {
        var x = (ui.helper[0].id == 'cur_x1_arrow' ? 'x1' : 'x2');
        var graph_width = $('#graph_grid').width();

        // Calculate time per division
        var samplerate = OSC.state.acq_speed;
        var samples = 1024;
        var mul = 1000;
        var scale = OSC.time_scale;
        var timePerDevInMs = (((samples / scale) / samplerate) * mul);

        var ms_per_px = timePerDevInMs / graph_width;
        var msg_width = $('#cur_' + x + '_info').outerWidth();
        var new_value = (graph_width / 2 - ui.position.left - (ui.helper.width() - 2) / 2 - parseInt(ui.helper.css('margin-left'))) * ms_per_px;

        $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').show();
        $('#cur_' + x + ', #cur_' + x + '_info').css('left', ui.position.left);
        $('#cur_' + x + '_info')
            .html(OSC.convertTime(-new_value))
            .data('cleanval', -new_value)
            .css('margin-left', (ui.position.left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));

        OSC.updateXCursorDiff();
    };

    // Resizes double-headed arrow showing the difference between X cursors
    OSC.updateXCursorDiff = function() {
        var x1 = $('#cur_x1_info');
        var x2 = $('#cur_x2_info');
        var x1_left = parseInt(x1.css('left'));
        var x2_left = parseInt(x2.css('left'));
        var diff_px = Math.abs(x1_left - x2_left) - 9;

        if (x1.is(':visible') && x2.is(':visible') && diff_px > 12) {
            var left = Math.min(x1_left, x2_left);
            var value = $('#cur_x1_info').data('cleanval') - $('#cur_x2_info').data('cleanval');

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

    // Updates Y offset in the signal config dialog, if opened, or saves new value




    */

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
        var visible = CLIENT.getValue("LA_DIN_" + (ch + 1))
        var arrow = $('#ch' + (ch + 1) + '_offset_arrow');
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
        var txt = $('#CH' + (ch + 1) + '_NAME').val();
        if (txt == "") {
            txt = "DIN" + ch;
        }
        $('#CH' + (ch + 1) + '_NAME').val(txt);
        arrow.find('#CH' + (ch + 1) + '_LABEL').text(txt);
        OSC.ch_names[ch] = txt;

        LA.showInfoArrow(ch);
    }

    LA.showInfoArrow = function(ch) {
        var arrow_info = $('#ch' + (ch + 1) + '_info');
        var arrow_img = $('#img_info_arrow' + (ch + 1));
        var info_text = OSC.accordingChanName(ch + 1);

        if (info_text !== "") {
            arrow_info.text(info_text);
            arrow_img.show();
        }
    }

    LA.hideInfoArrow = function(ch) {
        var arrow_info = $('#ch' + (ch + 1) + '_info');
        var arrow_img = $('#img_info_arrow' + (ch + 1));

        arrow_info.text("");
        arrow_img.hide();
    }

    OSC.updateYOffset = function(ui, save) {
        var graph_height = $('#graph_grid').height();
        // var new_value = 0;

        var arrows = ["ch1_offset_arrow", "ch2_offset_arrow", "ch3_offset_arrow", "ch4_offset_arrow",
            "ch5_offset_arrow", "ch6_offset_arrow", "ch7_offset_arrow", "ch8_offset_arrow"
        ];
        var ch = arrows.indexOf(ui.helper[0].id);
        if (ch != -1) {
            // var volt_per_px = 9 / graph_height;
            var mtop = parseFloat(ui.helper.css('top')) * 9.0 / graph_height
            var new_value = 9 - mtop ///ui.position.top + parseInt(ui.helper.css('margin-top')) / 2 * volt_per_px;

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
