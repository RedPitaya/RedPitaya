(function(SPEC, $, undefined) {

    SPEC.initHandlers = function(){
        $('button').bind('activeChanged', function() {
            SPEC.exitEditing(true);
        });

        $('#downl_graph').on('click', function() {
            if (SPEC.isVisibleChannels())
                setTimeout(SPEC.SaveGraphsPNG, 30);
        });

        $('#downl_csv').on('click', function() {
            SPEC.sendParameters({ 'requestFullData': true});
        });

        $('select, input').on('change', function() {
            SPEC.exitEditing(true);
            setTimeout('SPEC.exitEditing(true)', 1000);
        });

        // Initialize FastClick to remove the 300ms delay between a physical tap and the firing of a click event on mobile browsers
        new FastClick(document.body);

        // Process clicks on top menu buttons
        $('#SPEC_RUN').on('click', function() {
            //ev.preventDefault();
            $('#SPEC_RUN').hide();
            $('#SPEC_STOP').css('display', 'block');
            SPEC.params.local['SPEC_RUN'] = { value: true };
            SPEC.sendParams();
        });

        $('#SPEC_STOP').on('click', function() {
            //ev.preventDefault();
            $('#SPEC_STOP').hide();
            $('#SPEC_RUN').show();
            SPEC.params.local['SPEC_RUN'] = { value: false };
            SPEC.sendParams();
        });

        $('#SPEC_SINGLE').on('click', function() {
            ev.preventDefault();
            SPEC.params.local['SPEC_SINGLE'] = { value: true };
            SPEC.sendParams();
        });

        $('#SPEC_AUTSPECALE').on('click', function() {
            ev.preventDefault();
            SPEC.params.local['SPEC_AUTSPECALE'] = { value: true };
            SPEC.sendParams();
        });

        $('#SPEC_BUFFER_SIZE').on('click', function() {
            SPEC.params.local['SPEC_BUFFER_SIZE'] = { value: $("#SPEC_BUFFER_SIZE option:selected").val() };
            SPEC.sendParams();
        });

        $("#SWEEP_RESET").on('click',function() {
            SPEC.params.local['SWEEP_RESET'] = { value: true };
            SPEC.sendParams();
        });

        // Opening a dialog for changing parameters
        $('.edit-mode').on('click', function() {
            SPEC.state.editing = true;
            $('#right_menu').hide();
            $('#' + $(this).attr('id') + '_dialog').show();

            if ($.inArray($(this).data('signal'), ['output1', 'output2']) >= 0) {
                if (SPEC.state.sel_sig_name)
                    $('#right_menu .menu-btn.' + SPEC.state.sel_sig_name).removeClass('active');
                if ($(this).data('signal') == 'output1' || $(this).data('signal') == 'output2') {
                    var out_enabled = $(this).data('signal') == 'output1' ? SPEC.params.orig["OUTPUT1_STATE"].value : SPEC.params.orig["OUTPUT2_STATE"].value;
                    if (out_enabled) {
                        SPEC.state.sel_sig_name = $(this).data('signal');
                        $('#right_menu .menu-btn.' + SPEC.state.sel_sig_name).addClass('active');
                        $('.y-offset-arrow').css('z-index', 10);
                        $('#' + SPEC.state.sel_sig_name + '_offset_arrow').css('z-index', 11);
                    } else {
                        SPEC.state.sel_sig_name = null;
                    }
                } else {
                    SPEC.state.sel_sig_name = $(this).data('signal');

                    $('#right_menu .menu-btn.' + SPEC.state.sel_sig_name).addClass('active');
                    $('.y-offset-arrow').css('z-index', 10);
                    $('#' + SPEC.state.sel_sig_name + '_offset_arrow').css('z-index', 11);
                }
            }
        });

        // Close parameters dialog after Enter key is pressed
        $('input').keyup(function(event) {
            if (event.keyCode == 13) {
                SPEC.exitEditing(true);
            }
        });

        // Close parameters dialog on close button click
        $('.close-dialog').on('click', function() {
            SPEC.exitEditing();
        });

        $('.close-sweep-dialog').on('click', function() {
            $('.dialog:visible').hide();
            $('#'+$(this).attr('id')+"_dialog").show();
        });

        $('.edit-mode-sweep').on('click', function() {
            $('#'+$(this).attr('id')+"_dialog").hide();
            $('#' + $(this).attr('id') + '_sweep_dialog').show();
        });

        // Measurement dialog
        $('#meas_done').on('click', function() {
            var meas_signal = $('#meas_dialog input[name="meas_signal"]:checked');

            if (meas_signal.length) {
                var operator_name = $('#meas_operator option:selected').html();
                var operator_val = parseInt($('#meas_operator').val());
                var signal_name = meas_signal.val();
                var item_id = 'meas_' + operator_name + '_' + signal_name;

                // Check if the item already exists
                if ($('#' + item_id).length > 0) {
                    return;
                }

                // Add new item
                $('<div id="' + item_id + '" class="meas-item">' + operator_name + ' (' + signal_name + ')</div>').data({
                    value: (signal_name == 'CH1' ? operator_val : (signal_name == 'CH2' ? operator_val + 1 : null)), // Temporarily set null for MATH signal, because no param yet defined fot that signal
                    operator: operator_name,
                    signal: signal_name
                }).prependTo('#meas_list');
            }
        });

        $(document).on('click', '.meas-item', function() {
            $(this).remove();
        });

        // Joystick events
        $('#jtk_up').on('mousedown', function() { $('#jtk_btns').attr('src', 'img/node_up.png'); });
        $('#jtk_left').on('mousedown', function() { $('#jtk_btns').attr('src', 'img/node_left.png'); });
        $('#jtk_right').on('mousedown', function() { $('#jtk_btns').attr('src', 'img/node_right.png'); });
        $('#jtk_down').on('mousedown', function() { $('#jtk_btns').attr('src', 'img/node_down.png'); });
        $('#jtk_fine').on('mousedown', function() { $('#jtk_fine').attr('src', 'img/reset_active.png'); });


        $(document).on('mouseup', function() {
            $('#jtk_btns').attr('src', 'img/node_fine.png');
            $('#jtk_fine').attr('src', 'img/reset.png');
        });

        $('#jtk_fine').on('click', function(ev) {
            UI_GRAPH.resetZoom();
        });

        $('#jtk_up, #jtk_down').on('click', function(ev) {
            UI_GRAPH.changeYZoom(ev.target.id == 'jtk_down' ? '-' : '+');
        });

        $('#jtk_left, #jtk_right').on('click', function(ev) {
            UI_GRAPH.changeXZoom(ev.target.id == 'jtk_left' ? '-' : '+');
        });

        // Y cursor arrows dragging
        $('#cur_y1_arrow, #cur_y2_arrow').draggable({
            axis: 'y',
            containment: 'parent',
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
            containment: 'parent',
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



        $('#BDM_DBU_FUNC').change(function() {
            var mode = $("#BDM_DBU_FUNC option:selected").val();
            UI_GRAPH.updateYAxis();
            UI_GRAPH.changeYAxisMode(mode);
            SPEC.updateYCursorDiff();
        });

        $('#DBU_IMP_FUNC').bind("propertychange change click keyup input paste", function() {
            SPEC.config.dbu_imp = $("#DBU_IMP_FUNC").val()
        });

        $("#SOUR1_SWEEP_TIME").focus(function() {
            $(this).val(SPEC.state.sweep_ch1_time);
            $(this).addClass("focus")
        });

        $("#SOUR1_SWEEP_TIME").focusout(function() {
            $(this).removeClass("focus")
            $(this).val(SPEC.convertTimeToText(SPEC.state.sweep_ch1_time));
        });

        $("#SOUR2_SWEEP_TIME").focus(function() {
            $(this).val(SPEC.state.sweep_ch2_time);
            $(this).addClass("focus")
        });

        $("#SOUR2_SWEEP_TIME").focusout(function() {
            $(this).removeClass("focus")
            $(this).val(SPEC.convertTimeToText(SPEC.state.sweep_ch2_time));
        });

        $(window).on('focus', function() {
            SPEC.drawGraphGrid();
        });

        // // Prevent native touch activity like scrolling
        // $('html, body').on('touchstart touchmove', function(ev) {
        //     ev.preventDefault();
        // });

        $('#SPEC_CLEAR').click(function() {
            SPEC.sendParameters({ 'SPEC_RST': true});
            SPEC.clear = true;
        });
    }

}(window.SPEC = window.SPEC || {}, jQuery));