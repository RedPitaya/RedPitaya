(function(OSC, $, undefined) {

    OSC.initCursors = function(){
         // Y cursor arrows dragging
        $('#cur_y1_arrow, #cur_y2_arrow').draggable({
            axis: 'y',
            containment: 'parent',
            start: function(ev, ui) {
                OSC.state.cursor_dragging_measure = true;
            },
            drag: function(ev, ui) {
                OSC.updateYCursorElems(ui, false);
            },
            stop: function(ev, ui) {
                OSC.updateYCursorElems(ui, true);
                OSC.state.cursor_dragging_measure = false;
            }
        });

        $('#cur_x1_arrow, #cur_x2_arrow, #cur_y1_arrow, #cur_y2_arrow').mouseenter(function(event) {
            OSC.state.mouseover = true;
        });

        $('#cur_x1_arrow, #cur_x2_arrow, #cur_y1_arrow, #cur_y2_arrow').mouseleave(function(event) {
            OSC.state.mouseover = false;
        });

        // X cursor arrows dragging
        $('#cur_x1_arrow, #cur_x2_arrow').draggable({
            axis: 'x',
            containment: 'parent',
            start: function(ev, ui) {
                OSC.state.cursor_dragging_measure = true;
            },
            drag: function(ev, ui) {
                OSC.updateXCursorElems(ui, false);
            },
            stop: function(ev, ui) {
                OSC.updateXCursorElems(ui, true);
                OSC.state.cursor_dragging_measure = false;
            }
        });
    }

    OSC.cursorY = function() {
        if (!OSC.state.cursor_dragging_measure) {
            var ocs = OSC.params.orig['OSC_CURSOR_SRC'] !== undefined ? OSC.params.orig['OSC_CURSOR_SRC'].value : undefined

            if (ocs === undefined) return;
            var ref_show = ocs == OSC.adc_channes ? 'MATH_SHOW' : 'CH'+ (ocs + 1) +'_SHOW';
            var ref_scale = ocs == OSC.adc_channes ? 'OSC_MATH_SCALE' : 'OSC_CH'+ (ocs + 1) +'_SCALE';
            var ref_offset = ocs == OSC.adc_channes ? 'OSC_MATH_OFFSET' : 'OSC_CH'+ (ocs + 1) +'_OFFSET';

            var ref_scale_val = OSC.params.orig[ref_scale] != undefined ?  OSC.params.orig[ref_scale].value : undefined
            var ref_offset_val = OSC.params.orig[ref_offset] != undefined ?  OSC.params.orig[ref_offset].value : undefined
            var ref_show_val = OSC.params.orig[ref_show] != undefined ?  OSC.params.orig[ref_show].value : undefined

            if (ref_scale_val !== undefined && ref_offset_val !=  undefined && ref_show_val !== undefined){
                for(var i = 1 ; i <= 2; i++){
                    var y = 'y'+i;
                    var c_out_name = 'OSC_CURSOR_Y' + i;
                    var c_pos_name = 'OSC_CUR'+i+'_V';

                    var y_cur = OSC.params.orig[c_out_name] != undefined ?  OSC.params.orig[c_out_name].value : undefined
                    var y_cur_v = OSC.params.orig[c_pos_name] != undefined ?  OSC.params.orig[c_pos_name].value : undefined

                    if (y_cur && ref_show_val) {
                        // var ocs = OSC.params.orig['OSC_CURSOR_SRC'].value;

                        // var ref_scale = ocs == OSC.adc_channes ? 'OSC_MATH_SCALE' : 'OSC_CH'+ (ocs + 1) +'_SCALE';
                        // var ref_offset = ocs == OSC.adc_channes ? 'OSC_MATH_OFFSET' : 'OSC_CH'+ (ocs + 1) +'_OFFSET';

                        var source_offset = ref_offset_val;
                        var graph_height = $('#graph_grid').height();
                        var volt_per_px = (ref_scale_val * 10) / graph_height;
                        var px_offset = -((y_cur_v + source_offset) / volt_per_px - parseInt($('#cur_' + y + '_arrow').css('margin-top')) / 2);
                        var top = (graph_height + 7) / 2 + px_offset;
                        var overflow = false;

                        if (top < 0) {
                            top = 0;
                            overflow = true;
                        }
                        if (top > graph_height) {
                            top = graph_height;
                            overflow = true;
                        }

                        $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').css('top', top).show();
                        $('#cur_' + y + '_info')
                            .html(OSC.convertVoltage(+y_cur_v))
                            .data('cleanval', +y_cur_v)
                            .css('margin-top', (top < 16 ? 3 : ''));
                        if (overflow)
                            $('#cur_' + y + '_info').hide();
                    } else {
                        $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').hide();
                    }

                    var field = $('#' + c_out_name);
                    if (field.is('button')) {
                        field[y_cur === true? 'addClass' : 'removeClass']('active');
                    }
                }
                OSC.updateYCursorDiff();
            }
        }
    }

    OSC.setCursorSrc = function(new_params) {
        var field = $('#OSC_CURSOR_SRC');
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.val(OSC.params.orig['OSC_CURSOR_SRC'].value);
        }
        OSC.cursorY();
    }

    OSC.cursorY1 = function(new_params) {
        OSC.cursorY();
    }

    OSC.cursorY2 = function(new_params) {
        OSC.cursorY();
    }


    OSC.cursorX = function() {
        if (!OSC.state.cursor_dragging_measure) {
            var ts = OSC.params.orig['OSC_TIME_SCALE'] !== undefined ? OSC.params.orig['OSC_TIME_SCALE'].value : undefined
            var toff = OSC.params.orig['OSC_TIME_OFFSET'] !== undefined ? OSC.params.orig['OSC_TIME_OFFSET'].value : undefined

            if (ts !== undefined && toff !=  undefined ){
                for(var i = 1 ; i <= 2; i++){
                    var x = 'x'+i
                    var c_out_name = 'OSC_CURSOR_X' + i;
                    var c_pos_name = 'OSC_CUR'+i+'_T';

                    var x_cur = OSC.params.orig[c_out_name] != undefined ?  OSC.params.orig[c_out_name].value : undefined
                    var x_cur_t = OSC.params.orig[c_pos_name] != undefined ?  OSC.params.orig[c_pos_name].value : undefined


                    if (x_cur !== undefined && x_cur_t !== undefined){
                        if (x_cur) {

                            var graph_width = $('#graph_grid').width();
                            var ms_per_px = (ts * 10.0) / graph_width;
                            var px_offset = -((x_cur_t + toff) / ms_per_px - parseInt($('#cur_' + x + '_arrow').css('margin-left')) / 2 - 2.5);
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
                                .html(OSC.convertTime(-x_cur_t))
                                .data('cleanval', -x_cur_t)
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

    OSC.cursorX1 = function(new_params) {
        OSC.cursorX();
    }

    OSC.cursorX2 = function(new_params) {
        OSC.cursorX();
    }

     // Updates all elements related to a Y cursor
     OSC.updateYCursorElems = function(ui, save) {
        var y = (ui.helper[0].id == 'cur_y1_arrow' ? 'y1' : 'y2');
        var ocs = OSC.params.orig['OSC_CURSOR_SRC'].value;
        var ref_scale = ocs == OSC.adc_channes ? 'OSC_MATH_SCALE' : 'OSC_CH'+ (ocs + 1) +'_SCALE';
        var ref_offset = ocs == OSC.adc_channes ? 'OSC_MATH_OFFSET' : 'OSC_CH'+ (ocs + 1) +'_OFFSET';
        var source_offset = OSC.params.orig[ref_offset].value;
        var graph_height = $('#graph_grid').height();
        var volt_per_px = (OSC.params.orig[ref_scale].value * 10) / graph_height;
        var new_value = (graph_height / 2 - ui.position.top - (ui.helper.height() - 2) / 2 - parseInt(ui.helper.css('margin-top'))) * volt_per_px - source_offset;

        $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').show();
        $('#cur_' + y + ', #cur_' + y + '_info').css('top', ui.position.top);
        $('#cur_' + y + '_info')
            .html(OSC.convertVoltage(+new_value))
            .data('cleanval', +new_value)
            .css('margin-top', (ui.position.top < 16 ? 3 : ''));

        OSC.updateYCursorDiff();

        if (save) {
            OSC.params.local[y == 'y1' ? 'OSC_CUR1_V' : 'OSC_CUR2_V'] = { value: new_value };
            OSC.sendParams();
        }
    };

    // Updates all elements related to a X cursor
    OSC.updateXCursorElems = function(ui, save) {
        var x = (ui.helper[0].id == 'cur_x1_arrow' ? 'x1' : 'x2');
        var graph_width = $('#graph_grid').width();
        var ms_per_px = (OSC.params.orig['OSC_TIME_SCALE'].value * 10) / graph_width;
        var msg_width = $('#cur_' + x + '_info').outerWidth();
        var new_value = (graph_width / 2 - ui.position.left - (ui.helper.width() - 2) / 2 - parseInt(ui.helper.css('margin-left'))) * ms_per_px - OSC.params.orig['OSC_TIME_OFFSET'].value;

        $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').show();
        $('#cur_' + x + ', #cur_' + x + '_info').css('left', ui.position.left);
        $('#cur_' + x + '_info')
            .html(OSC.convertTime(-new_value))
            .data('cleanval', -new_value)
            .css('margin-left', (ui.position.left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));

        OSC.updateXCursorDiff();

        if (save) {
            OSC.params.local[x == 'x1' ? 'OSC_CUR1_T' : 'OSC_CUR2_T'] = { value: new_value };
            OSC.sendParams();
        }
    };

    // Resizes double-headed arrow showing the difference between Y cursors
    OSC.updateYCursorDiff = function() {
        var y1 = $('#cur_y1_info');
        var y2 = $('#cur_y2_info');
        var y1_top = parseInt(y1.css('top'));
        var y2_top = parseInt(y2.css('top'));
        var diff_px = Math.abs(y1_top - y2_top) - 6;

        if (y1.is(':visible') && y2.is(':visible') && diff_px > 12) {
            var top = Math.min(y1_top, y2_top);
            var value = $('#cur_y1_info').data('cleanval') - $('#cur_y2_info').data('cleanval');

            $('#cur_y_diff')
                .css('top', top + 5)
                .height(diff_px)
                .show();
            $('#cur_y_diff_info')
                .html(OSC.convertVoltage(Math.abs(value)))
                .css('top', top + diff_px / 2 - 2)
                .show();
        } else {
            $('#cur_y_diff, #cur_y_diff_info').hide();
        }
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

}(window.OSC = window.OSC || {}, jQuery));