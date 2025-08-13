/*
 * Red Pitaya Bode analyzer client
 *
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


//Bode analyser
(function(BA, $, undefined) {


    BA.enableX = function(cursor_name, new_params) {
        if (!BA.state.cursor_dragging && !BA.state.mouseover) {
            var x = (cursor_name == 'BA_CURSOR_X1' ? 'x1' : 'x2');
            if (new_params) {
                $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').show();
                $('#'+cursor_name).addClass('active');
            } else {
                $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').hide();
                $('#'+cursor_name).removeClass('active');
            }
            BA.updateXLinesAndArrows();
        }
    }

    BA.moveX = function(ui) {
        const w = $('#cursors_holder').width() - 2;
        if (ui.helper[0].id == 'cur_x1_arrow') {
            const l = $("#cur_x1_arrow").position().left
            BA.cursorsRelative.x1 = l / w;
            if (BA.cursorsRelative.x1 < 0)
                BA.cursorsRelative.x1 = 0;
            if (BA.cursorsRelative.x1 > 1)
                BA.cursorsRelative.x1 = 1;

        } else if (ui.helper[0].id == 'cur_x2_arrow') {
            const l = $("#cur_x2_arrow").position().left
            BA.cursorsRelative.x2 = l / w;
            if (BA.cursorsRelative.x2 < 0)
                BA.cursorsRelative.x2 = 0;
            if (BA.cursorsRelative.x2 > 1)
                BA.cursorsRelative.x2 = 1;
        }
        BA.updateXLinesAndArrows();
    }

    BA.enableY = function(cursor_name, new_params) {
        if (!BA.state.cursor_dragging && !BA.state.mouseover) {
            var y = (cursor_name == 'BA_CURSOR_Y1' ? 'y1' : 'y2');

            if (new_params) {
                $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').show();
                $('#'+cursor_name).addClass('active');
            } else {
                $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').hide();
                $('#'+cursor_name).removeClass('active');               
            }
            BA.updateYLinesAndArrows();
        }
    }

    BA.moveY = function(ui) {
        const h = $('#cursors_holder').height() - 2;
        if (ui.helper[0].id == 'cur_y1_arrow') {
            const t = $("#cur_y1_arrow").position().top
            BA.cursorsRelative.y1 = t / h;
            if (BA.cursorsRelative.y1 < 0)
                BA.cursorsRelative.y1 = 0;
            if (BA.cursorsRelative.y1 > 1)
                BA.cursorsRelative.y1 = 1;

        } else if (ui.helper[0].id == 'cur_y2_arrow') {
            const t = $("#cur_y2_arrow").position().top
            BA.cursorsRelative.y2 = t / h;
            if (BA.cursorsRelative.y2 < 0)
                BA.cursorsRelative.y2 = 0;
            if (BA.cursorsRelative.y2 > 1)
                BA.cursorsRelative.y2 = 1;
        }
        BA.updateYLinesAndArrows();
    }

    BA.enableZ = function(cursor_name, new_params) {
        if (!BA.state.cursor_dragging && !BA.state.mouseover) {
            var z = (cursor_name == 'BA_CURSOR_Z1' ? 'z1' : 'z2');

            if (new_params) {
                $('#cur_' + z + '_arrow, #cur_' + z + ', #cur_' + z + '_info').show();
                $('#'+cursor_name).addClass('active');                
            } else {
                $('#cur_' + z + '_arrow, #cur_' + z + ', #cur_' + z + '_info').hide();
                $('#'+cursor_name).removeClass('active');                
            }
            BA.updateZLinesAndArrows();
        }
    }

    BA.moveZ = function(ui) {
        const h = $('#cursors_holder').height() - 2;
        if (ui.helper[0].id == 'cur_z1_arrow') {
            const t = $("#cur_z1_arrow").position().top
            BA.cursorsRelative.z1 = t / h;
            if (BA.cursorsRelative.z1 < 0)
                BA.cursorsRelative.z1 = 0;
            if (BA.cursorsRelative.z1 > 1)
                BA.cursorsRelative.z1 = 1;

        } else if (ui.helper[0].id == 'cur_z2_arrow') {
            const t = $("#cur_z2_arrow").position().top
            BA.cursorsRelative.z2 = t / h;
            if (BA.cursorsRelative.z2 < 0)
                BA.cursorsRelative.z2 = 0;
            if (BA.cursorsRelative.z2 > 1)
                BA.cursorsRelative.z2 = 1;
        }
        BA.updateZLinesAndArrows();
    }

    BA.enableCursor = function(x) {
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
            BA.enableX('BA_CURSOR_' + x[0].toUpperCase() + d, true);
            BA.enableX('BA_CURSOR_' + x[0].toUpperCase() + d, true);
        } else if (x[0] == 'y') {
            BA.enableY('BA_CURSOR_' + x[0].toUpperCase() + d, true);
            BA.enableY('BA_CURSOR_' + x[0].toUpperCase() + d, true);
        } else if (x[0] == 'z') {
            BA.enableZ('BA_CURSOR_' + x[0].toUpperCase() + d, true);
            BA.enableZ('BA_CURSOR_' + x[0].toUpperCase() + d, true);
        }

    };

    BA.disableCursor = function(x) {
        var d = (x[1] == '1') ? '1' : '2';

        $('cur_' + x).hide();
        $('cur_' + x + '_info').hide();
        $('cur_' + x + '_arrow').hide();
        $('cur_' + x[0] + '_diff').hide();
        $('cur_' + x[0] + '_diff_info').hide();

        if (x[0] == 'x') {
            BA.enableX('BA_CURSOR_' + x[0].toUpperCase() + d, false);
            BA.enableX('BA_CURSOR_' + x[0].toUpperCase() + d, false);
        } else if (x[0] == 'y') {
            BA.enableY('BA_CURSOR_' + x[0].toUpperCase() + d, false);
            BA.enableY('BA_CURSOR_' + x[0].toUpperCase() + d, false);
        } else if (x[0] == 'z') {
            BA.enableZ('BA_CURSOR_' + x[0].toUpperCase() + d, false);
            BA.enableZ('BA_CURSOR_' + x[0].toUpperCase() + d, false);
        }
    };

    BA.updateXLinesPosition = function() {
        const w = $('#cursors_holder').width();
        const h = $('#cursors_holder').height();
        $('#cur_x1_arrow').css({
            left: w * BA.cursorsRelative.x1,
            top: h - 27
        });

        $('#cur_x2_arrow').css({
            left: w * BA.cursorsRelative.x2,
            top: h - 27
        });

        $('#cur_x1').height(h);
        $('#cur_x2').height(h);
    }

    BA.updateYLinesPosition = function() {
        const w = $('#cursors_holder').width();
        const h = $('#cursors_holder').height();
        $('#cur_y1_arrow').css({
            left: w - 27 ,
            top: h * BA.cursorsRelative.y1
        });

        $('#cur_y2_arrow').css({
            left: w - 27 ,
            top: h * BA.cursorsRelative.y2
        });

        $('#cur_y1').width(w);
        $('#cur_y2').width(w);
    }

     BA.updateZLinesPosition = function() {
        const w = $('#cursors_holder').width();
        const h = $('#cursors_holder').height();
        $('#cur_z1_arrow').css({
            left: 0 ,
            top: h * BA.cursorsRelative.z1
        });

        $('#cur_z2_arrow').css({
            left: 0 ,
            top: h * BA.cursorsRelative.z2
        });

        $('#cur_z1').width(w);
        $('#cur_z2').width(w);
    }

    // X-cursors
    BA.updateXLinesAndArrows = function() {

        const cl = $('#cursors_holder').offset().left;
        const w = $('#cursors_holder').width();

        var axes = BA.graphCache.plot.getAxes();
        var diff_px = 0;
        let unit = ' Hz';
        let min = axes.xaxis.min;
        let max = axes.xaxis.max;
        let new_val = 0;

        const setCursor = (cur, min,max,t) => {

            if (BA.curGraphScale) {
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
        
        setCursor('x1',min,max,BA.cursorsRelative.x1)       
        setCursor('x2',min,max,BA.cursorsRelative.x2)


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
    BA.updateYLinesAndArrows = function() {

        const ct = $('#cursors_holder').offset().top;
        const h = $('#cursors_holder').height();

        var diff_px = 0;
        let unit = ' dB';

        var axes = BA.graphCache.plot.getAxes();
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
        
        setCursor('y1',min,max,BA.cursorsRelative.y1)       
        setCursor('y2',min,max,BA.cursorsRelative.y2)
      

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


    // Z-cursors
    BA.updateZLinesAndArrows = function() {

        const cl = $('#cursors_holder').offset().left;
        const ct = $('#cursors_holder').offset().top;

        const w = $('#cursors_holder').width();
        const h = $('#cursors_holder').height();
                
        var diff_px = 0;
        let unit = ' deg';
        var axes = BA.graphCache.plot.getAxes();
        let min = axes.y2axis.min;
        let max = axes.y2axis.max;
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
            $('#cur_' + cur + '_info').css({left: w -  $('#cur_' + cur + '_info').width() - 40});
            
        };
        
        setCursor('z1',min,max,BA.cursorsRelative.z1)       
        setCursor('z2',min,max,BA.cursorsRelative.z2)

        diff_px = Math.abs($('#cur_z1').offset().top - $('#cur_z2').offset().top) - 6;
        if ($('#cur_z1').is(':visible') && $('#cur_z2').is(':visible') && diff_px > 12) {
            var top = Math.min($('#cur_z1').offset().top, $('#cur_z2').offset().top) + 3;
            var value = $('#cur_z1_info').data('cleanval') - $('#cur_z2_info').data('cleanval');
            $('#cur_z_diff').show()
            $('#cur_z_diff_info').show().html(Math.abs(value).toFixed(2) + unit)

            $('#cur_z_diff')
                .offset({top: top })
                .height(diff_px)
                .css({left: w - 60})
            $('#cur_z_diff_info')
                .offset({ top: top + diff_px / 2  - $('#cur_y_diff_info').height() / 2 })
                .css({left: w -  $('#cur_z_diff_info').width() / 2  - 60 })

        } else {
            $('#cur_z_diff, #cur_z_diff_info').hide();
        }
    }


    BA.updateLinesAndArrows = function() {
        BA.updateXLinesAndArrows();
        BA.updateYLinesAndArrows();
        BA.updateZLinesAndArrows();
    }


    BA.updateCursors = function() {
        if (BA.graphCache === undefined)
            return;
        var plot = BA.graphCache.plot;
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

        BA.updateXLinesPosition()
        BA.updateYLinesPosition()
        BA.updateZLinesPosition()
    };


}(window.BA = window.BA || {}, jQuery));




// Page onload event handler
$(function() {

    // X cursor arrows dragging
    $('#cur_x1_arrow, #cur_x2_arrow').draggable({
        axis: 'x',
        containment: 'parent',
        start: function(ev, ui) {
            BA.state.line_moving = true;
            BA.state.cursor_dragging = true;
        },
        drag: function(ev, ui) {
            BA.moveX(ui);
        },
        stop: function(ev, ui) {
            BA.moveX(ui);
            BA.state.line_moving = false;
            BA.state.cursor_dragging = false;
            CLIENT.parametersCache["BA_CURSOR_X1_POS"] = { value: BA.cursorsRelative.x1 };
            CLIENT.parametersCache["BA_CURSOR_X2_POS"] = { value: BA.cursorsRelative.x2 };
            CLIENT.sendParameters();
        }
    });


    // Y cursor arrows dragging
    $('#cur_y1_arrow, #cur_y2_arrow').draggable({
        axis: 'y',
        containment: 'parent',
        start: function(ev, ui) {
            BA.state.cursor_dragging = true;
        },
        drag: function(ev, ui) {
            BA.moveY(ui);
        },
        stop: function(ev, ui) {
            BA.moveY(ui);
            BA.state.cursor_dragging = false;
            CLIENT.parametersCache["BA_CURSOR_Y1_POS"] = { value: BA.cursorsRelative.y1 };
            CLIENT.parametersCache["BA_CURSOR_Y2_POS"] = { value: BA.cursorsRelative.y2 };
            CLIENT.sendParameters();
        }
    });


    // Z cursor arrows dragging
    $('#cur_z1_arrow, #cur_z2_arrow').draggable({
        axis: 'y',
        containment: 'parent',
        start: function(ev, ui) {
            BA.state.cursor_dragging = true;
        },
        drag: function(ev, ui) {
            BA.moveZ(ui);
        },
        stop: function(ev, ui) {
            BA.moveZ(ui);
            BA.state.cursor_dragging = false;
            CLIENT.parametersCache["BA_CURSOR_Z1_POS"] = { value: BA.cursorsRelative.z1 };
            CLIENT.parametersCache["BA_CURSOR_Z2_POS"] = { value: BA.cursorsRelative.z2 };
            CLIENT.sendParameters();            
        }
    });


    $('#BA_CURSOR_X1').click(function() {
        CLIENT.parametersCache["BA_CURSOR_X1"] = { value: !$(this).hasClass('active') };
        CLIENT.sendParameters();
    });

    $('#BA_CURSOR_X2').click(function() {
        CLIENT.parametersCache["BA_CURSOR_X2"] = { value: !$(this).hasClass('active') };
        CLIENT.sendParameters();
    });

    $('#BA_CURSOR_Y1').click(function() {
        CLIENT.parametersCache["BA_CURSOR_Y1"] = { value: !$(this).hasClass('active') };
        CLIENT.sendParameters();
    });

    $('#BA_CURSOR_Y2').click(function() {
        CLIENT.parametersCache["BA_CURSOR_Y2"] = { value: !$(this).hasClass('active') };
        CLIENT.sendParameters();
    });

    $('#BA_CURSOR_Z1').click(function() {
        CLIENT.parametersCache["BA_CURSOR_Z1"] = { value: !$(this).hasClass('active') };
        CLIENT.sendParameters();
    });

    $('#BA_CURSOR_Z2').click(function() {
        CLIENT.parametersCache["BA_CURSOR_Z2"] = { value: !$(this).hasClass('active') };
        CLIENT.sendParameters();
    });

});