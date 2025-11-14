/*
 * Red Pitaya Spectrum Analizator client
 *
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function(SPEC, $, undefined) {

    SPEC.initHandlers = function(){
        $('button').bind('activeChanged', function(e) {
            SPEC.exitEditing(true, e.target.id);
        });

        $('#downl_graph').on('click', function() {
            if (SPEC.isVisibleChannels())
                setTimeout(SPEC.SaveGraphsPNG, 30);
        });

        $('#downl_csv').on('click', function() {
            SPEC.downloadDataAsCSV("spectrumSignal.csv",SPEC.latest_signal);
        });

        $('select, input').on('change', function(e) {
            SPEC.exitEditing(true, e.target.id);
        });

        // Process clicks on top menu buttons
        $('#SPEC_RUN').on('click', function() {
            //ev.preventDefault();
            $('#SPEC_RUN').hide();
            $('#SPEC_STOP').css('display', 'block');
            CLIENT.sendParametersEx({'SPEC_RUN': {value: true}});
        });

        $('#SPEC_STOP').on('click', function() {
            //ev.preventDefault();
            $('#SPEC_STOP').hide();
            $('#SPEC_RUN').show();
            CLIENT.sendParametersEx({'SPEC_RUN': {value: false}});
        });

        $('#SPEC_BUFFER_SIZE').on('click', function() {
            CLIENT.sendParametersEx({'SPEC_BUFFER_SIZE': {value:$("#SPEC_BUFFER_SIZE option:selected").val()}});
        });

        $("#SWEEP_RESET").on('click',function() {
            CLIENT.sendParametersEx({'SWEEP_RESET': {value: true}});
        });

        // Opening a dialog for changing parameters
        $('.edit-mode').on('click', function() {
            $('#right_menu').hide();
            $('#' + $(this).attr('id') + '_dialog').show();

            if ($.inArray($(this).data('signal'), ['output1', 'output2']) >= 0) {
                if (SPEC.state.sel_sig_name)
                    $('#right_menu .menu-btn.' + SPEC.state.sel_sig_name).removeClass('active');
                if ($(this).data('signal') == 'output1' || $(this).data('signal') == 'output2') {
                    var out_enabled = $(this).data('signal') == 'output1' ? CLIENT.params.orig["OUTPUT1_STATE"].value : CLIENT.params.orig["OUTPUT2_STATE"].value;
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
                SPEC.exitEditing(true, event.target.id);
            }
        });

        // Close parameters dialog on close button click

        $('.close-sweep-dialog').on('click', function() {
            $('.dialog:visible').hide();
            $('#'+$(this).attr('id')+"_dialog").show();
        });

        $('.edit-mode-sweep').on('click', function() {
            $('#'+$(this).attr('id')+"_dialog").hide();
            $('#' + $(this).attr('id') + '_sweep_dialog').show();
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

        $('#BDM_DBU_FUNC').change(function() {
            var mode = $("#BDM_DBU_FUNC option:selected").val();
            UI_GRAPH.updateYAxis();
            UI_GRAPH.changeYAxisMode(mode);
            SPEC.updateYCursorDiff();
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

        $('#SPEC_CLEAR').click(function() {
            SPEC.minMaxController.resetAll()
            SPEC.latest_signal = {'ch_xaxis' : SPEC.latest_signal['ch_xaxis']}
            SPEC.processSignals(SPEC.latest_signal)
        });
    }

}(window.SPEC = window.SPEC || {}, jQuery));