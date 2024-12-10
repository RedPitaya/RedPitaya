/*
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


(function(LA, $, undefined) {

    LA.initHandlers = function() {

        $('.trigger_type').change(function(e) {
            OSC.sendTriggerFromUI(this)
        });

        $('input[type=text]:not([readonly]):not(.no-arrows)[step]').iLightInputNumber({
            mobile: false
        });

        $('#save_settings').click(function() {
            $('#save_settings_dialog').modal("show");
        });

        $('#reset_settings').click(function() {
            CLIENT.parametersCache["CONTROL_CONFIG_SETTINGS"] = { value: 1 };
            CLIENT.sendParameters()
        });

        $('#LA_REQ_SAVE_SETTINGS').on('click', function() {
            var name = $("#SETTINGS_NEW_NAME").val().trim()
            if (name !== ""){
                CLIENT.parametersCache['FILE_SATTINGS'] = { value: name };
                CLIENT.parametersCache['CONTROL_CONFIG_SETTINGS'] = { value: 4 }; // SAVE
                CLIENT.sendParameters()
            }
        });

        $('.enable-ch').click(OSC.enableChannel);

        $('.ch-name-inp').change(function() {
            var arr = ["CH1_NAME", "CH2_NAME", "CH3_NAME", "CH4_NAME", "CH5_NAME", "CH6_NAME", "CH7_NAME", "CH8_NAME"];
            var element = $(this);
            var ch = arr.indexOf(element.attr('id'));
            if (ch != -1) {
                CLIENT.parametersCache["LA_DIN_NAME_"+(ch+1)] = { value: element.val() };
                CLIENT.sendParameters()
            }
        })

        $('#LA_CURSOR_X1').click(function() {
            var btn = $(this);
            if (!btn.hasClass('active'))
                LA.enableCursor('1');
            else
            LA.disableCursor('1');
        });

        $('#LA_CURSOR_X2').click(function() {
            var btn = $(this);
            if (!btn.hasClass('active'))
                LA.enableCursor('2');
            else
            LA.disableCursor('2');
        });

        $('#select_mode').click(function() {
            var curVal = CLIENT.getValue("LA_MEASURE_MODE")
            if (curVal !== undefined){
                var x = curVal === 1 ? 2 : 1
                CLIENT.parametersCache["LA_MEASURE_MODE"] = { value: x };
                CLIENT.sendParameters()
            }
        });

        $('#cur_x1_arrow, #cur_x2_arrow').draggable({
            axis: 'x',
            containment: 'parent',
            start: function(ev, ui) {
                LA.state.line_moving = true;
                LA.state.cursor_dragging = true;
            },
            drag: function(ev, ui) {
                LA.updateXCursorElems(ui, false);
            },
            stop: function(ev, ui) {
                LA.state.line_moving = false;
                LA.updateXCursorElems(ui, true);
                LA.state.cursor_dragging = false;
            }
        });

        $('.y-offset-arrow').on('mousedown', function() {
            OSC.state.line_moving = true;
        });

        $('.y-offset-arrow').on('mouseup', function() {
            OSC.state.line_moving = false;
        });

        $('.y-offset-arrow').dblclick(function(event) {
            event.stopPropagation();
            var index = event.currentTarget.getAttribute("index")
            CLIENT.parametersCache["LA_DIN_" + index + "_POS"] = { value: index };
            CLIENT.sendParameters()
        });

        // Voltage offset arrow dragging
        $('.y-offset-arrow').draggable({
            axis: 'y',
            containment: 'parent',
            start: function(ev, ui) {
                OSC.state.line_moving = true;
            },
            drag: function(ev, ui) {
                OSC.state.line_moving = true;
                var margin_top = Math.abs(parseInt(ui.helper.css('marginTop')));
                var min_top = ((ui.helper.height() / 2) + margin_top) * -1;
                var max_top = $('#graphs').height() - margin_top;

                if (ui.position.top < min_top) {
                    ui.position.top = min_top;
                } else if (ui.position.top > max_top) {
                    ui.position.top = max_top;
                }

                OSC.updateYOffset(ui, false);

            },
            stop: function(ev, ui) {
                OSC.state.line_moving = false;
                if (!OSC.state.simulated_drag) {
                    OSC.updateYOffset(ui, true);
                    $('#info_box').empty();
                }
            }
        });

        $('#buffer_time_region').draggable({
            axis: 'x',
            containment: 'parent',
            start: function(ev, ui) {
                LA.region_view_moving = true
               // OSC.state.line_moving = true;
            },
            drag: function(ev, ui) {
                LA.region_view_moving = true
                var w =$('#buffer_time_region').width()
                var fw = $('#graphs_buffer').width()
                var newPos = (ui.position.left + w/2.0) / fw
                CLIENT.params.orig['LA_VIEW_PORT_POS'] = {value: newPos}
                LA.updatePositionBufferViewport()
            },
            stop: function(ev, ui) {
                LA.region_view_moving = false
                CLIENT.parametersCache['LA_VIEW_PORT_POS'] = {value: CLIENT.getValue('LA_VIEW_PORT_POS')}
                CLIENT.sendParameters()
            }
        });

        $('#buffer').on('click', function(ev) {
            var fw = $('#graphs_buffer').width()
            var left = ev.currentTarget.getBoundingClientRect().left
            CLIENT.parametersCache['LA_VIEW_PORT_POS'] = {value: (ev.clientX - left) / fw}
            CLIENT.sendParameters()
            ev.preventDefault();
            ev.stopPropagation();
        });

            // Joystick events
        $('#jtk_up').on('mousedown touchstart', function() {
            $('#jtk_btns').attr('src', 'img/navigation_up.png');
        });
        $('#jtk_left').on('mousedown touchstart', function() {
            $('#jtk_btns').attr('src', 'img/navigation_left.png');
        });
        $('#jtk_right').on('mousedown touchstart', function() {
            $('#jtk_btns').attr('src', 'img/navigation_right.png');
        });
        $('#jtk_down').on('mousedown touchstart', function() {
            $('#jtk_btns').attr('src', 'img/navigation_down.png');
        });

        //  $('#jtk_fine').on('click touchstart', function(ev){
        $('#jtk_fine').on('click', function(ev) {
            var img = $('#jtk_fine');

            if (img.attr('src') == 'img/fine.png') {
                img.attr('src', 'img/fine_active.png');
                OSC.state.fine = true;
            } else {
                img.attr('src', 'img/fine.png');
                OSC.state.fine = false;
            }

            ev.preventDefault();
            ev.stopPropagation();
        });

        $(document).on('mouseup touchend', function() {
            $('#jtk_btns').attr('src', 'img/navigation.png');
        });

        $('#jtk_up, #jtk_down').on('click', function(ev) {
            ev.preventDefault();
            ev.stopPropagation();
            LA.time_zoom(ev.target.id == 'jtk_down' ? '-' : '+', 0, false)
            OSC.guiHandler();
        });

        $('#jtk_left, #jtk_right').on('click', function(ev) {
            ev.preventDefault();
            ev.stopPropagation();
            LA.moveViewPort(ev.target.id == 'jtk_left' ? '-' : '+');
        });

            // Process clicks on top menu buttons
        $('#LA_RUN').on('click', function(ev) {
            ev.preventDefault();
            CLIENT.parametersCache['LA_RUN'] = { value: true };
            CLIENT.sendParameters();
        });

        $('#LA_STOP').on('click', function(ev) {
            ev.preventDefault();
            CLIENT.parametersCache['LA_RUN'] = { value: false };
            CLIENT.sendParameters();
        });

        $(window).resize(function() {
            var window_height = window.innerHeight;
            $('#main_block').css('height', window_height - 200);

            LA.initCursors()
            LA.resizePlots()
            LA.drawGraphGrid()
            LA.updateCursors()
            LA.updateChannels()
            OSC.setCurrentFreq()

        }).resize();

        $('#graphs').dblclick(function(event) {
            event.stopPropagation();
            var s = CLIENT.getValue('LA_TOTAL_SAMPLES')
            var pre = CLIENT.getValue('LA_PRE_TRIGGER_SAMPLES')
            if (s !== undefined && pre !== undefined){
                CLIENT.parametersCache['LA_VIEW_PORT_POS'] = {value: pre / s}
                CLIENT.sendParameters()
            }
        });

        $('#sys_info').click(function() {
            var elem = $(this);
            if (elem.text() == 'SYS INFO') {
                elem.html('&check; SYS INFO');
                $('#sys_info_view').show();
            } else {
                elem.text('SYS INFO');
                $('#sys_info_view').hide();
            }
        });

    }

}(window.LA = window.LA || {}, jQuery));
