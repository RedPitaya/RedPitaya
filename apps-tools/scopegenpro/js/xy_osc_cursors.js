(function(OSC, $, undefined) {

    OSC.initCursorsXY = function(){
        // Y cursor arrows dragging
       $('#xy_cur_y1_arrow, #xy_cur_y2_arrow').draggable({
           axis: 'y',
           containment: 'parent',
           start: function(ev, ui) {
               OSC.state.xy_cursor_dragging_measure = true;
           },
           drag: function(ev, ui) {
               OSC.updateYCursorElemsXY(ui, false);
           },
           stop: function(ev, ui) {
               OSC.updateYCursorElemsXY(ui, true);
               OSC.state.xy_cursor_dragging_measure = false;
           }
       });

       $('#xy_cur_x1_arrow, #xy_cur_x2_arrow, #xy_cur_y1_arrow, #xy_cur_y2_arrow').mouseenter(function(event) {
           OSC.state.xy_mouseover = true;
       });

       $('#xy_cur_x1_arrow, #xy_cur_x2_arrow, #xy_cur_y1_arrow, #xy_cur_y2_arrow').mouseleave(function(event) {
           OSC.state.xy_mouseover = false;
       });

       // X cursor arrows dragging
       $('#xy_cur_x1_arrow, #xy_cur_x2_arrow').draggable({
           axis: 'x',
           containment: 'parent',
           start: function(ev, ui) {
               OSC.state.xy_cursor_dragging_measure = true;
           },
           drag: function(ev, ui) {
               OSC.updateXCursorElemsXY(ui, false);
           },
           stop: function(ev, ui) {
               OSC.updateXCursorElemsXY(ui, true);
               OSC.state.xy_cursor_dragging_measure = false;
           }
       });
   }

    OSC.xyGetCh = function(val){
        if (val === 0) return "CH1"
        if (val === 1) return "CH2"
        if (val === 2) return "CH3"
        if (val === 3) return "CH4"
        if (val === 4) return "MATH"
        return ""
    }



    OSC.xyCursorY = function() {
        if (!OSC.state.xy_cursor_dragging_measure) {
            for(var i = 1 ; i <= 2; i++){
                var y = 'y'+i;
                var c_out_name = 'OSC_XY_CURSOR_Y' + i;
                var c_pos_name = 'OSC_XY_CUR'+i+'_Y';

                var y_cur = OSC.params.orig[c_out_name] != undefined ?  OSC.params.orig[c_out_name].value : undefined
                var y_cur_v = OSC.params.orig[c_pos_name] != undefined ?  OSC.params.orig[c_pos_name].value : undefined

                if (y_cur) {
                    // var ocs = OSC.params.orig['OSC_CURSOR_SRC'].value;

                    // var ref_scale = ocs == OSC.adc_channes ? 'OSC_MATH_SCALE' : 'OSC_CH'+ (ocs + 1) +'_SCALE';
                    // var ref_offset = ocs == OSC.adc_channes ? 'OSC_MATH_OFFSET' : 'OSC_CH'+ (ocs + 1) +'_OFFSET';

                    var graph_height = $('#xy_graph_grid').height();
                    var volt_per_px = 10.0 / graph_height;
                    var px_offset = -(y_cur_v / volt_per_px - parseInt($('#xy_cur_' + y + '_arrow').css('margin-top')) / 2);
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

                    $('#xy_cur_' + y + '_arrow, #xy_cur_' + y).css('top', top).show();
                    if (OSC.params.orig['Y_AXIS_SOURCE']){
                        var xsrc = OSC.params.orig['Y_AXIS_SOURCE'].value;
                        var srcName = OSC.xyGetCh(xsrc)
                        if (srcName !== ""){
                            var scale = OSC.params.orig["OSC_"+srcName+"_SCALE"] ? OSC.params.orig["OSC_"+srcName+"_SCALE"].value : 0;
                            var y_cur_volt = y_cur_v * scale
                            $('#xy_cur_' + y + '_info').css('top', top - 14).show();
                            $('#xy_cur_' + y + '_info')
                                .html(OSC.convertVoltage(y_cur_volt)+(srcName == "MATH" ? OSC.xyMathSuffix() : ""))
                                .data('cleanval', y_cur_volt)
                                .css('margin-top', (top < 16 ? 3 : ''));
                        }
                    }
                    if (overflow)
                        $('#xy_cur_' + y + '_info').hide();
                } else {
                    $('#xy_cur_' + y + '_arrow, #xy_cur_' + y + ', #xy_cur_' + y + '_info').hide();
                }

                var field = $('#' + c_out_name);
                if (field.is('button')) {
                    field[y_cur === true? 'addClass' : 'removeClass']('active');
                }
            }
            OSC.updateYCursorDiffXY();
       }
   }

   OSC.xyCursorY1 = function(new_params) {
       OSC.xyCursorY();
   }

   OSC.xyCursorY2 = function(new_params) {
       OSC.xyCursorY();
   }


   OSC.xyCursorX = function() {
       if (!OSC.state.xy_cursor_dragging_measure) {



            for(var i = 1 ; i <= 2; i++){
                var x = 'x'+i
                var c_out_name = 'OSC_XY_CURSOR_X' + i;
                var c_pos_name = 'OSC_XY_CUR'+i+'_X';

                var x_cur = OSC.params.orig[c_out_name] != undefined ?  OSC.params.orig[c_out_name].value : undefined
                var x_cur_t = OSC.params.orig[c_pos_name] != undefined ?  OSC.params.orig[c_pos_name].value : undefined


                if (x_cur !== undefined && x_cur_t !== undefined){
                    if (x_cur) {

                        var graph_width = $('#xy_graph_grid').width();
                        var per_px = 10.0 / graph_width;
                        var px_offset = -(x_cur_t / per_px - parseInt($('#xy_cur_' + x + '_arrow').css('margin-left')) / 2 - 2.5);
                        var msg_width = $('#xy_cur_' + x + '_info').outerWidth();
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

                        $('#xy_cur_' + x + '_arrow, #xy_cur_' + x).css('left', left).show();

                        if (OSC.params.orig['X_AXIS_SOURCE']){
                            var xsrc = OSC.params.orig['X_AXIS_SOURCE'].value;
                            var srcName = OSC.xyGetCh(xsrc)
                            if (srcName !== ""){
                                var scale = OSC.params.orig["OSC_"+srcName+"_SCALE"] ? OSC.params.orig["OSC_"+srcName+"_SCALE"].value : 0;
                                var x_cur_volt = -x_cur_t * scale
                                $('#xy_cur_' + x + '_info').css('left', left + 2).show();
                                $('#xy_cur_' + x + '_info')
                                                .html(OSC.convertVoltage(x_cur_volt)+(srcName == "MATH" ? OSC.xyMathSuffix() : ""))
                                                .data('cleanval', x_cur_volt)
                                                .css('margin-left', (left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));
                            }
                        }

                        if (overflow)
                            $('#xy_cur_' + x + '_info').hide();
                    } else {
                        $('#xy_cur_' + x + '_arrow, #xy_cur_' + x + ', #xy_cur_' + x + '_info').hide();
                    }

                    var field = $('#' + c_out_name);
                    if (field.is('button')) {
                        field[x_cur === true? 'addClass' : 'removeClass']('active');
                    }
                }
            }
            OSC.updateXCursorDiffXY();
        }
    }

    OSC.xyCursorX1 = function(new_params) {
        OSC.xyCursorX();
    }

    OSC.xyCursorX2 = function(new_params) {
        OSC.xyCursorX();
    }

    // Updates all elements related to a Y cursor
    OSC.updateYCursorElemsXY = function(ui, save) {
        if (OSC.params.orig['Y_AXIS_SOURCE']){
            var xsrc = OSC.params.orig['Y_AXIS_SOURCE'].value;
            var srcName = OSC.xyGetCh(xsrc)
            if (srcName !== ""){
                var scale = OSC.params.orig["OSC_"+srcName+"_SCALE"] ? OSC.params.orig["OSC_"+srcName+"_SCALE"].value : 0;
                var y = (ui.helper[0].id == 'xy_cur_y1_arrow' ? 'y1' : 'y2');
                var graph_height = $('#xy_graph_grid').height();
                var volt_per_px = 10.0 / graph_height;
                var new_value = (graph_height / 2 - ui.position.top - (ui.helper.height() - 2) / 2 - parseInt(ui.helper.css('margin-top'))) * volt_per_px;

                $('#xy_cur_' + y + '_arrow, #xy_cur_' + y + ', #xy_cur_' + y + '_info').show();
                $('#xy_cur_' + y).css('top', ui.position.top);
                $('#xy_cur_' + y + '_info').css('top', ui.position.top - 14).show();


                $('#xy_cur_' + y + '_info')
                    .html(OSC.convertVoltage(+new_value * scale)+(srcName == "MATH" ? OSC.xyMathSuffix() : ""))
                    .data('cleanval', +new_value * scale)
                    .css('margin-top', (ui.position.top < 16 ? 3 : ''));
            }
        }

        OSC.updateYCursorDiffXY();

        if (save) {
            OSC.params.local[y == 'y1' ? 'OSC_XY_CUR1_Y' : 'OSC_XY_CUR2_Y'] = { value: new_value };
            OSC.sendParams();
        }
   };

   // Updates all elements related to a X cursor
   OSC.updateXCursorElemsXY = function(ui, save) {
        if (OSC.params.orig['X_AXIS_SOURCE']){
            var xsrc = OSC.params.orig['X_AXIS_SOURCE'].value;
            var srcName = OSC.xyGetCh(xsrc)
            if (srcName !== ""){
                var scale = OSC.params.orig["OSC_"+srcName+"_SCALE"] ? OSC.params.orig["OSC_"+srcName+"_SCALE"].value : 0;
                var x = (ui.helper[0].id == 'xy_cur_x1_arrow' ? 'x1' : 'x2');
                var graph_width = $('#xy_graph_grid').width();
                var ms_per_px = 10.0 / graph_width;
                var msg_width = $('#xy_cur_' + x + '_info').outerWidth();
                var new_value = (graph_width / 2 - ui.position.left - (ui.helper.width() - 2) / 2 - parseInt(ui.helper.css('margin-left'))) * ms_per_px
                console.log("TEST",scale,new_value)
                $('#xy_cur_' + x + '_arrow, #xy_cur_' + x + ', #xy_cur_' + x + '_info').show();
                $('#xy_cur_' + x).css('left', ui.position.left);
                $('#xy_cur_' + x + '_info').css('left', ui.position.left + 2).show();
                $('#xy_cur_' + x + '_info')
                    .html(OSC.convertVoltage(-new_value * scale)+(srcName == "MATH" ? OSC.xyMathSuffix() : ""))
                    .data('cleanval', -new_value * scale)
                    .css('margin-left', (ui.position.left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));
            }
        }

        OSC.updateXCursorDiffXY();

        if (save) {
            OSC.params.local[x == 'x1' ? 'OSC_XY_CUR1_X' : 'OSC_XY_CUR2_X'] = { value: new_value };
            OSC.sendParams();
        }
   };

   // Resizes double-headed arrow showing the difference between Y cursors
   OSC.updateYCursorDiffXY = function() {
        if (OSC.params.orig['Y_AXIS_SOURCE']){
            var xsrc = OSC.params.orig['Y_AXIS_SOURCE'].value;
            var srcName = OSC.xyGetCh(xsrc)
            if (srcName !== ""){
                var y1 = $('#xy_cur_y1_info');
                var y2 = $('#xy_cur_y2_info');
                var y1_top = parseInt(y1.css('top'));
                var y2_top = parseInt(y2.css('top'));
                var diff_px = Math.abs(y1_top - y2_top) - 6;

                if (y1.is(':visible') && y2.is(':visible') && diff_px > 12) {
                    var top = Math.min(y1_top, y2_top);
                    var value = $('#xy_cur_y1_info').data('cleanval') - $('#xy_cur_y2_info').data('cleanval');

                    $('#xy_cur_y_diff')
                        .css('top', top + 18)
                        .height(diff_px)
                        .show();
                    $('#xy_cur_y_diff_info')
                        .html(OSC.convertVoltage(Math.abs(value))+(srcName == "MATH" ? OSC.xyMathSuffix() : ""))
                        .css('top', top + diff_px / 2 - 2)
                        .show();
                } else {
                    $('#xy_cur_y_diff, #xy_cur_y_diff_info').hide();
                }
            }
        }
    };

    // Resizes double-headed arrow showing the difference between X cursors
    OSC.updateXCursorDiffXY = function() {
        if (OSC.params.orig['X_AXIS_SOURCE']){
            var xsrc = OSC.params.orig['X_AXIS_SOURCE'].value;
            var srcName = OSC.xyGetCh(xsrc)
            if (srcName !== ""){
                var x1 = $('#xy_cur_x1_info');
                var x2 = $('#xy_cur_x2_info');
                var x1_left = parseInt(x1.css('left'));
                var x2_left = parseInt(x2.css('left'));
                var diff_px = Math.abs(x1_left - x2_left) - 9;

                if (x1.is(':visible') && x2.is(':visible') && diff_px > 12) {
                    var left = Math.min(x1_left, x2_left);
                    var value = $('#xy_cur_x1_info').data('cleanval') - $('#xy_cur_x2_info').data('cleanval');
                    $('#xy_cur_x_diff')
                        .css('left', left + 1)
                        .width(diff_px)
                        .show();
                    $('#xy_cur_x_diff_info')
                        .html(OSC.convertVoltage(Math.abs(value))+(srcName == "MATH" ? OSC.xyMathSuffix() : ""))
                        .show()
                        .css('left', left + diff_px / 2 - $('#cur_x_diff_info').width() / 2 + 3);
                } else {
                    $('#xy_cur_x_diff, #xy_cur_x_diff_info').hide();
                }
            }
        }
    };

}(window.OSC = window.OSC || {}, jQuery));