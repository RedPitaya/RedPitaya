(function(OSC, $, undefined) {

    $.fn.fI = function(e) { //Flash Item
        if (!e) { e = {} }
        if (this) { e.e = this }
        switch (e.f) {
            case 0:
                break;
            default:
                switch (e.css) {
                    case 0:
                        e.d = 'background-color'
                        break;
                    case undefined:
                        e.d = 'border-color'
                        break;
                    default:
                        e.d = e.css
                        break;
                }
                if (!e.c1) { e.c1 = '#FF0000' }
                if (!e.c2) { e.c2 = '#A00000' }
                if (!e.p) { e.p = 200 }
                e.e.css(e.d, e.c1)
                setTimeout(function() {
                    e.e.css(e.d, e.c2)
                    setTimeout(function() {
                        e.e.css(e.d, e.c1)
                        setTimeout(function() {
                            e.e.css(e.d, e.c2)
                            setTimeout(function() {
                                e.e.css(e.d, '')
                            }, e.p)
                        }, e.p)
                    }, e.p)
                }, e.p)
                break;
        }
        return this
    }

    $.fn.iLightInputNumber = function(options) {

        var inBox = '.input-number-box',
            newInput = '.input-number',
            moreVal = '.input-number-more',
            lessVal = '.input-number-less';

        this.each(function() {

            var el = $(this);
            $('<div class="' + inBox.substr(1) + '"></div>').insertAfter(el);
            var parent = el.find('+ ' + inBox);
            parent.append(el);
            var classes = el.attr('class');

            el.addClass(classes);
            var attrValue;


            parent.append('<div class=' + moreVal.substr(1) + '></div>');
            parent.append('<div class=' + lessVal.substr(1) + '></div>');

        }); //end each

        var value,
            step;

        var interval = null,
            timeout = null;

        // function ToggleValue(input) {
        //     input.val(parseInt(input.val(), 10) + d);
        //     console.log(input);
        // }

        $('body').on('mousedown', moreVal, function() {
            var el = $(this);
            var input = el.siblings(newInput);
            moreValFn(input);
            timeout = setTimeout(function() {
                interval = setInterval(function() {
                    moreValFn(input);
                }, 50);
            }, 200);

        });

        $('body').on('mousedown', lessVal, function() {
            var el = $(this);
            var input = el.siblings(newInput);
            lessValFn(input);
            timeout = setTimeout(function() {
                interval = setInterval(function() {
                    lessValFn(input);
                }, 50);
            }, 200);
        });

        $("#ext_con_but").click(function(event) {
            $('#ext_connections_dialog').modal("show");
        });

        $(moreVal + ', ' + lessVal).on("mouseup mouseout", function() {
            clearTimeout(timeout);
            clearInterval(interval);
        });

        function moreValFn(input) {
            var max;
            var limits = getLimits(input);
            max = limits.max;
            checkInputAttr(input);

            var newValue = value + step;
            var parts = step.toString().split('.');
            var signs = parts.length < 2 ? 0 : parts[1].length;
            newValue = parseFloat(newValue.toFixed(signs));
            if (newValue > max) {
                newValue = max;
            }
            changeInputsVal(input, newValue);
        }

        function getLimits(input) {
            var min = parseFloat(input.attr('min'));
            var max = parseFloat(input.attr('max'));
            return {
                'min': min,
                'max': max
            };
        }

        function lessValFn(input) {

            var min;
            var limits = getLimits(input);
            min = limits.min;

            checkInputAttr(input);

            var newValue = value - step;
            var parts = step.toString().split('.');
            var signs = parts.length < 2 ? 0 : parts[1].length;
            newValue = parseFloat(newValue.toFixed(signs));
            if (newValue < min) {
                newValue = min;
            }
            changeInputsVal(input, newValue);
        }

        function changeInputsVal(input, newValue) {
            input.val(newValue);
            OSC.exitEditing(true, '0' in input ? input['0'] : undefined);
        }

        function checkInputAttr(input) {

            value = parseFloat(input.val());
            if (!($.isNumeric(value))) {
                value = 0;
            }

            if (input.attr('step')) {
                step = parseFloat(input.attr('step'));
                var signs = Math.log10(step)
                value = parseFloat(value.toFixed(-signs));
            } else {
                step = 1;
            }
        }

        $(newInput).change(function() {
            var input = $(this);

            checkInputAttr(input);
            var limits = getLimits(input);
            var min = limits.min;
            var max = limits.max;

            if (value < min) {
                value = min;
            } else if (value > max) {
                value = max;
            }
            if (!($.isNumeric(value))) {
                value = 0;
            }
            input.val(value);
            OSC.exitEditing(true,this)
        });

        $(newInput).keydown(function(e) {
            var input = $(this);
            var k = e.keyCode;
            if (k == 38) {
                moreValFn(input);
            } else if (k == 40) {
                lessValFn(input);
            }
        });
    };

    OSC.uintToColor = function(uint) {
        const red = (uint >> 16) & 255;
        const green = (uint >> 8) & 255;
        const blue = uint & 255;

        // Convert RGB components to hex format
        const rHex = red.toString(16).padStart(2, '0');
        const gHex = green.toString(16).padStart(2, '0');
        const bHex = blue.toString(16).padStart(2, '0');

        // Create the hex color string
        const hexColor = "#" + rHex + gHex + bHex;

        return hexColor;
    }

    OSC.updateARBFunc = function(list) {
        const splitLines = value => value.split(/\r?\n/);
        splitLines(list).forEach(function(item){
            var id = item.trim();
            if (id !== ""){
                var name = id.slice(1);
                var cols = name.trim().split('\t');
                id = id.trim().split('\t');
                var opt = document.createElement('option')
                var opt2 = document.createElement('option')
                opt.setAttribute('value', id[0])
                opt.innerText = cols[0]
                opt2.setAttribute('value', id[0])
                opt2.innerText = cols[0]
                var color = OSC.uintToColor(parseInt(cols[1]))
                opt.style.color = color
                opt2.style.color = color
                var r1 = document.getElementById('SOUR1_FUNC');
                if (r1!= null)
                    r1.appendChild(opt);
                var r2 = document.getElementById('SOUR2_FUNC');
                if (r2!= null)
                    r2.appendChild(opt2);
            }
        });
    }

    OSC.updateLimits = function () {
        // { // OSC_CH1_OFFSET limits
        //     var probeAttenuation = parseInt($("#OSC_CH1_PROBE option:selected").text());
        //     var jumperSettings = $("#OSC_CH1_IN_GAIN").parent().hasClass("active") ? 1 : 20;
        //     var units = $('#OSC_CH1_OFFSET_UNIT').html();
        //     var multiplier = units == "mV" ? 1000 : 1;
        //     var newMin = -1 * 10 * jumperSettings * probeAttenuation * multiplier;
        //     var newMax = 1 * 10 * jumperSettings * probeAttenuation * multiplier;
        //     $("#OSC_CH1_OFFSET").attr("min", newMin);
        //     $("#OSC_CH1_OFFSET").attr("max", newMax);
        // }

        // { // OSC_CH2_OFFSET limits
        //     var probeAtt1 = parseInt($("#OSC_CH2_PROBE option:selected").text());
        //     var jumperSettings1 = $("#OSC_CH2_IN_GAIN").parent().hasClass("active") ? 1 : 20;
        //     var units = $('#OSC_CH2_OFFSET_UNIT').html();
        //     var multiplier = units == "mV" ? 1000 : 1;
        //     var newMin = -1 * 10 * jumperSettings1 * probeAtt1 * multiplier;
        //     var newMax = 1 * 10 * jumperSettings1 * probeAtt1 * multiplier;
        //     $("#OSC_CH2_OFFSET").attr("min", newMin);
        //     $("#OSC_CH2_OFFSET").attr("max", newMax);
        // }

        // { // OSC_MATH_OFFSET limits
        //     var scale_val = $("#OSC_MATH_SCALE").text();
        //     var math_vdiv = parseFloat(scale_val);
        //     var newMin = -1 * 5 * math_vdiv;
        //     var newMax = 1 * 5 * math_vdiv;
        //     // $("#OSC_MATH_OFFSET").attr("min", newMin);
        //     // $("#OSC_MATH_OFFSET").attr("max", newMax);
        // }

    }

    OSC.formatVals =function () {
        // { // OSC_CH1_OFFSET
        //     var probeAttenuation = parseInt($("#OSC_CH1_PROBE option:selected").text());
        //     var jumperSettings = $("#OSC_CH1_IN_GAIN").parent().hasClass("active") ? 1 : 20;
        //     var units = $('#OSC_CH1_OFFSET_UNIT').html();
        //     var multiplier = units == "mV" ? 1000 : 1;
        //     var in1_value = parseFloat($("#OSC_CH1_OFFSET").val());
        //     $("#OSC_CH1_OFFSET").val(OSC.formatInputValue(in1_value, probeAttenuation, units == "mV", jumperSettings == 20));
        // }

        // { // OSC_CH2_OFFSET
        //     var probeAtt1 = parseInt($("#OSC_CH2_PROBE option:selected").text());
        //     var jumperSettings1 = $("#OSC_CH2_IN_GAIN").parent().hasClass("active") ? 1 : 20;
        //     var units = $('#OSC_CH2_OFFSET_UNIT').html();
        //     var multiplier = units == "mV" ? 1000 : 1;
        //     var in2_value = parseFloat($("#OSC_CH2_OFFSET").val());
        //     $("#OSC_CH2_OFFSET").val(OSC.formatInputValue(in2_value, probeAtt1, units == "mV", jumperSettings1 == 20));
        // }

        { // OSC_MATH_OFFSET
            // var scale_val = $("#OSC_MATH_SCALE").text();
            // var units = $("#munit").text();
            // var math_vdiv = parseFloat(scale_val);

            // var munit = $('#munit').html().charAt(0);
            // var precision = 2;
            // if (munit == 'm')
            //     precision = 0;
            // if (math_vdiv < 1)
            //     precision = 3;
            // var math_value = parseFloat($("#OSC_MATH_OFFSET").val());
            // $("#OSC_MATH_OFFSET").val(math_value.toFixed(precision));
        }

        // { // OSC_TRIG_LEVEL_OFFSET
        //     var probeAttenuation = 1;
        //     var jumperSettings = 1;
        //     var ch = "";
        //     if ($("#OSC_TRIG_SOURCE").parent().hasClass("active"))
        //         ch = "CH1";
        //     else if ($("OSC_TRIG_SOURCE2").parent().hasClass("active"))
        //         ch = "CH2";
        //     else {
        //         probeAttenuation = 1;
        //     }

        //     if (ch == "CH1" || ch == "CH2") {
        //         probeAttenuation = parseInt($("#OSC_" + ch + "_PROBE option:selected").text());
        //         jumperSettings = $("#OSC_" + ch + "_IN_GAIN").parent().hasClass("active") ? 1 : 20;
        //     }

        //     var trig_value = parseFloat($("#OSC_TRIG_LEVEL").val());
        //     $("#OSC_TRIG_LEVEL").val(OSC.formatInputValue(trig_value, probeAttenuation, false, jumperSettings == 20));
        // }


        // { // DUTY CYCLE FORMATTING
        //     var SOUR1_DCYC = parseFloat($("SOUR1_DCYC").val());
        //     $("SOUR1_DCYC").val(SOUR1_DCYC.toFixed(1));
        //     var SOUR2_DCYC = parseFloat($("SOUR2_DCYC").val());
        //     $("SOUR2_DCYC").val(SOUR2_DCYC.toFixed(1));

        //     var SOUR1_PHAS = parseFloat($("SOUR1_PHAS").val());
        //     $("SOUR1_PHAS").val(SOUR1_PHAS.toFixed(1));
        //     var SOUR2_PHAS = parseFloat($("SOUR2_PHAS").val());
        //     $("SOUR2_PHAS").val(SOUR2_PHAS.toFixed(1));
        // }
    }


    OSC.initUI = function(){


        $('.btn.menu-btn').onClassChange(function(el, newClass) {
            OSC.getCurrentActiveChannel()
            OSC.updateTitileYAxisTicks()
            OSC.cursorY()
        } );

        // Joystick events
        $('#jtk_up').on('mousedown touchstart', function() { $('#jtk_btns').attr('src', 'img/node_up.png'); });
        $('#jtk_left').on('mousedown touchstart', function() { $('#jtk_btns').attr('src', 'img/node_left.png'); });
        $('#jtk_right').on('mousedown touchstart', function() { $('#jtk_btns').attr('src', 'img/node_right.png'); });
        $('#jtk_down').on('mousedown touchstart', function() { $('#jtk_btns').attr('src', 'img/node_down.png'); });

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
            $('#jtk_btns').attr('src', 'img/node_fine.png');
        });

        $('#jtk_up, #jtk_down').on('click', function(ev) {
            ev.preventDefault();
            ev.stopPropagation();
            OSC.changeYZoom(ev.target.id == 'jtk_down' ? '+' : '-');
        });

        $('#jtk_left, #jtk_right').on('click', function(ev) {
            ev.preventDefault();
            ev.stopPropagation();
            OSC.changeXZoom(ev.target.id == 'jtk_left' ? '+' : '-');
        });

        $(".sweep_reset").on('click',function() {
            OSC.params.local['SWEEP_RESET'] = { value: true };
            OSC.sendParams();
        });

        $(".gen_reset").on('click',function() {
            OSC.params.local['SYNC_GEN'] = { value: true };
            OSC.sendParams();
        });


        // Time offset arrow dragging
        $('#time_offset_arrow').draggable({
            axis: 'x',
            containment: 'parent',
            drag: function(ev, ui) {
                OSC.moveTimeOffset(ui.position.left);
            },
            stop: function(ev, ui) {
                OSC.endTimeMove(ui.position.left);
            }
        });

        // Time offset rectangle dragging
        $('#buf_time_offset').draggable({
            axis: 'x',
            containment: 'parent',
            drag: function(ev, ui) {
                var buf_width = $('#buffer').width();
                var zero_pos = (buf_width + 2) / 2;
                var ms_per_px = (OSC.params.orig['OSC_TIME_SCALE'].value * 10) / buf_width;
                var ratio = buf_width / (buf_width * OSC.params.orig['OSC_VIEV_PART'].value);
                var new_value = +(((zero_pos - ui.position.left - ui.helper.width() / 2 - 1) * ms_per_px * ratio).toFixed(2));
                var px_offset = -(new_value / ms_per_px + $('#time_offset_arrow').width() / 2 + 1);

                $('#info_box').html('Time offset ' + OSC.convertTime(new_value));
                $('#time_offset_arrow').css('left', (buf_width + 2) / 2 + px_offset);
            },
            stop: function(ev, ui) {
                if (!OSC.state.simulated_drag) {
                    var buf_width = $('#buffer').width();
                    var zero_pos = (buf_width + 2) / 2;
                    var ms_per_px = (OSC.params.orig['OSC_TIME_SCALE'].value * 10) / buf_width;
                    var ratio = buf_width / (buf_width * OSC.params.orig['OSC_VIEV_PART'].value);

                    OSC.params.local['OSC_TIME_OFFSET'] = { value: (zero_pos - ui.position.left - ui.helper.width() / 2 - 1) * ms_per_px * ratio };
                    OSC.sendParams();
                    $('#info_box').empty();
                }
            }
        });


        $(".dbl").on('dblclick', function(event) {
            var cls = $(this).attr('id');
            if (cls.indexOf('ch1') != -1)
                OSC.params.local['GPOS_OFFSET_CH1'] = { value: 0 }
            if (cls.indexOf('ch2') != -1)
                OSC.params.local['GPOS_OFFSET_CH2'] = { value: 0 }
            if (cls.indexOf('ch3') != -1)
                OSC.params.local['GPOS_OFFSET_CH3'] = { value: 0 }
            if (cls.indexOf('ch4') != -1)
                OSC.params.local['GPOS_OFFSET_CH4'] = { value: 0 }
            if (cls.indexOf('math') != -1)
                OSC.params.local['GPOS_OFFSET_MATH'] = { value: 0 }
            if (cls.indexOf('trig') != -1)
                OSC.params.local['OSC_TRIG_LEVEL'] = { value: 0 }
            if (cls.indexOf('output1') != -1)
                OSC.params.local['GPOS_OFFSET_OUTPUT1'] = { value: 0 }
            if (cls.indexOf('output2') != -1)
                OSC.params.local['GPOS_OFFSET_OUTPUT2'] = { value: 0 }
            OSC.sendParams()
        });

        // Process clicks on top menu buttons
        $('#OSC_RUN').on('click', function(ev) {
            ev.preventDefault();
            $('#OSC_RUN').hide();
            $('#OSC_STOP').css('display', 'block');
            OSC.params.local['OSC_RUN'] = { value: true };
            OSC.sendParams();
            OSC.running = true;
        });

        $('#OSC_STOP').on('click', function(ev) {
            ev.preventDefault();
            $('#OSC_STOP').hide();
            $('#OSC_RUN').show();
            OSC.params.local['OSC_RUN'] = { value: false };
            OSC.sendParams();
            OSC.running = false;
        });

        $('#OSC_SAVEGRAPH').on('click', function() {
            setTimeout(OSC.SaveGraphsPNG, 30);
        });

        $('#normalize_chbox').click(function(event){
            var chkBox = document.getElementById('normalize_chbox')
            var state = chkBox.getAttribute('data-checked') === "true"
            OSC.params.local['REQUEST_NORMALIZE'] = { value: !state }
            OSC.sendParams();
        });

        $('#view_chbox').click(function(event){
            var chkBox = document.getElementById('view_chbox')
            var state = chkBox.getAttribute('data-checked') === "true"
            OSC.params.local['REQUEST_VIEW'] = { value: !state }
            OSC.sendParams();
        });

        $('#OSC_REQ_EXPORT_FILE').on('click', function() {
            OSC.params.local['REQUEST_DATA'] = { value: true };
            OSC.sendParams();
        });

        $('#OSC_REQ_SAVE_SETTINGS').on('click', function() {
            var name = $("#SETTINGS_NEW_NAME").val().trim()
            if (name !== ""){
                OSC.params.local['FILE_SATTINGS'] = { value: name };
                OSC.params.local['CONTROL_CONFIG_SETTINGS'] = { value: 4 }; // SAVE
                OSC.sendParams();
            }
        });

        $('#OSC_PREV_BUFFER').on('click', function(ev) {
            ev.preventDefault();
            OSC.params.local['OSC_BUFFER_REQUEST'] = { value: -1 };
            OSC.sendParams();
        });

        $('#OSC_NEXT_BUFFER').on('click', function(ev) {
            ev.preventDefault();
            OSC.params.local['OSC_BUFFER_REQUEST'] = { value: 1 };
            OSC.sendParams();
        });

        $('#OSC_SINGLE').on('click', function(ev) {
            ev.preventDefault();
            OSC.params.local['OSC_SINGLE'] = { value: true };
            OSC.sendParams();
        });

        $('#OSC_AUTOSCALE').on('click', function(ev) {
            if (OSC.running == false)
                return;
            if ((OSC.params.orig["CH1_SHOW"] && OSC.params.orig["CH1_SHOW"].value == false) &&
                (OSC.params.orig["CH2_SHOW"] && OSC.params.orig["CH2_SHOW"].value == false) &&
                (OSC.params.orig["CH3_SHOW"] && OSC.params.orig["CH3_SHOW"].value == false) &&
                (OSC.params.orig["CH4_SHOW"] && OSC.params.orig["CH4_SHOW"].value == false) &&
                ((OSC.params.orig["OUTPUT1_SHOW"].value == false) || (OSC.params.orig["OUTPUT1_STATE"].value == false)) &&
                ((OSC.params.orig["OUTPUT2_SHOW"].value == false) || (OSC.params.orig["OUTPUT2_STATE"].value == false)) &&
                (OSC.params.orig["MATH_SHOW"].value == false))
                return;
            ev.preventDefault();
            OSC.params.local['OSC_AUTOSCALE'] = { value: true };
            // OSC.params.orig['OSC_AUTOSCALE'].value = true;
            OSC.sendParams();
            $('body').removeClass('loaded');
            OSC.loaderShow = true;
            OSC.signalStack = [];
        });

        $('#OSC_EXPORT').on('click', function(ev) {
            $('#export_dialog').modal("show");
        });

        // Selecting active signal
        $('.menu-btn').on('click', function() {
            if (!$(this).hasClass('not-signal')){
                $('#right_menu .menu-btn').not(this).removeClass('active');
                if (!$(this).hasClass('active'))
                    OSC.state.sel_sig_name = $(this).data('signal');
                else
                    OSC.state.sel_sig_name = null;
                $('.y-offset-arrow').css('z-index', 10);
                $('#' + OSC.state.sel_sig_name + '_offset_arrow').css('z-index', 11);
            }
            OSC.updateTitileYAxisTicks()
        });

        // Opening a dialog for changing parameters
        $('.edit-mode').on('click', function() {
            OSC.state.editing = true;
            $('#right_menu').hide();
            $('#' + $(this).attr('id') + '_dialog').show();

            if ($.inArray($(this).data('signal'), ['ch1', 'ch2', 'ch3', 'ch4', 'math', 'output1', 'output2']) >= 0) {
                if (OSC.state.sel_sig_name)
                    $('#right_menu .menu-btn.' + OSC.state.sel_sig_name).removeClass('active');
                if ($(this).data('signal') == 'output1' || $(this).data('signal') == 'output2' || $(this).data('signal') == 'math') {
                    var out_enabled = $(this).data('signal') == 'output1' ? OSC.params.orig["OUTPUT1_STATE"].value : $(this).data('signal') == 'output2' ? OSC.params.orig["OUTPUT2_STATE"].value : OSC.params.orig["MATH_SHOW"].value;
                    if (out_enabled) {
                        OSC.state.sel_sig_name = $(this).data('signal');
                        $('#right_menu .menu-btn.' + OSC.state.sel_sig_name).addClass('active');
                        $('.y-offset-arrow').css('z-index', 10);
                        $('#' + OSC.state.sel_sig_name + '_offset_arrow').css('z-index', 11);
                    } else
                        OSC.state.sel_sig_name = null;
                } else {
                    OSC.state.sel_sig_name = $(this).data('signal');

                    $('#right_menu .menu-btn.' + OSC.state.sel_sig_name).addClass('active');
                    $('.y-offset-arrow').css('z-index', 10);
                    $('#' + OSC.state.sel_sig_name + '_offset_arrow').css('z-index', 11);
                }
            }
        });


        $('.edit-mode-sweep').on('click', function() {
            $('#'+$(this).attr('id')+"_dialog").hide();
            $('#' + $(this).attr('id') + '_sweep_dialog').show();
        });

        $('.edit-mode-burst').on('click', function() {
            $('#'+$(this).attr('id')+"_dialog").hide();
            $('#' + $(this).attr('id') + '_burst_dialog').show();
        });

        $('.edit-mode-trig-input').on('click', function() {
            $('#trig_dialog').hide();
            $('#trig_dialog_input').show();
        });

        $('.edit-mode-trig-output').on('click', function() {
            $('#trig_dialog').hide();
            $('#trig_dialog_output').show();
        });

        $('.close-trig-dialog').on('click', function() {
            $('.dialog:visible').hide();
            $('#'+$(this).attr('id')+"_dialog").show();
        });

        // Close parameters dialog after Enter key is pressed
        $('input').keyup(function(event) {
            if (event.keyCode == 13)
                OSC.exitEditing(true,event);
        });

        // Close parameters dialog on close button click
        $('.close-dialog').on('click', function() {
            OSC.exitEditing();
        });

        $('.close-sweep-dialog').on('click', function() {
            $('.dialog:visible').hide();
            $('#'+$(this).attr('id')+"_dialog").show();
        });

        $('.close-burst-dialog').on('click', function() {
            $('.dialog:visible').hide();
            $('#'+$(this).attr('id')+"_dialog").show();
        });

        // Measurement dialog
        $('#meas_done').on('click', function(event) {
            var meas_signal = $('#meas_signal option:selected');

            if (meas_signal.length) {
                var operator_name = $('#meas_operator option:selected').html();
                var operator_val = parseInt($('#meas_operator').val()) * 10;
                var signal_name = meas_signal.val();
                var item_id = 'meas_' + operator_name.replace(/\s/g, '_') + '_' + signal_name;

                // Check if the item already exists
                if ($('#' + item_id).length > 0)
                    return;

                var sig_text = '';
                var ch_index = 0;
                if (signal_name == 'CH1'){
                    sig_text = 'IN1';
                    ch_index = 0;
                }
                else if (signal_name == 'CH2'){
                    sig_text = 'IN2';
                    ch_index = 1;
                }
                else if (signal_name == 'CH3'){
                    sig_text = 'IN3';
                    ch_index = 2;
                }
                else if (signal_name == 'CH4'){
                    sig_text = 'IN4';
                    ch_index = 3;
                }
                else if (signal_name == 'MATH'){
                    sig_text = 'MATH';
                    ch_index = 4;
                }

                // Add new item
                $('<div id="' + item_id + '" class="meas-item">' + operator_name + ' (' + sig_text + ')</div>').data({
                    value: operator_val + ch_index,
                    operator: operator_name,
                    signal: signal_name
                }).appendTo('#meas_list');
            }
            OSC.exitEditing(true,event);
        });

        $(document).on('click', '.meas-item', function(event) {
            $(this).remove();
            OSC.exitEditing(true,event);
        });

        $('button').bind('activeChanged', function(event) { OSC.exitEditing(true,event); });
        $('select').on('change', function(event) { OSC.exitEditing(true,event); });
        $("input[type='radio']").on('change', function(event) { OSC.exitEditing(true,event); });


        $('.input_name').focus( function() {
            $(this).attr('old_value', $(this).val())
        });

        $('.input_name').blur( function() {
            if ($(this).val().trim() === ''){
                $(this).val($(this).attr('old_value'))
            }
        });


        $("#graphs").mousewheel(function(event) {
            if (OSC.mouseWheelEventFired)
                return;
            OSC.changeXZoom(event.deltaY > 0 ? '+' : '-');
            OSC.mouseWheelEventFired = true;
            setTimeout(function() { OSC.mouseWheelEventFired = false; }, 300);
        });

        $('#send_report_btn').on('click', function() { OSC.formEmail() });
        $('#restart_app_btn').on('click', function() { location.reload() });

        var laAxesMoving = false;
        var curXPos = 0;
        $("#graphs").mousedown(function(event) {
            if (OSC.state.trig_dragging || OSC.state.cursor_dragging || OSC.state.cursor_dragging_measure) {
                laAxesMoving = false;
                return;
            }
            laAxesMoving = true;
            curXPos = event.pageX;
        });

        $(".full-content").mouseup(function(event) {
            if (OSC.state.trig_dragging || OSC.state.cursor_dragging || OSC.state.cursor_dragging_measure) {
                laAxesMoving = false;
                return;
            }
            laAxesMoving = false;
            OSC.endTimeMove($('#time_offset_arrow').position().left);
        });
        $(".full-content").mouseout(function(event) {
            if (OSC.state.trig_dragging || OSC.state.cursor_dragging || OSC.state.cursor_dragging_measure) {
                OSC.endTimeMove($('#time_offset_arrow').position().left);
                laAxesMoving = false;
                return;
            }
            //laAxesMoving = false;
        });
        $(".full-content").mousemove(function(event) {
            if (OSC.state.trig_dragging || OSC.state.cursor_dragging || OSC.state.trig_in || OSC.state.cursor_dragging_measure) {
                laAxesMoving = false;
                return;
            }
            if (OSC.state.line_moving) return;
            if (laAxesMoving && !OSC.state.cursor_dragging && !OSC.state.trig_dragging &&!OSC.state.cursor_dragging_measure && !OSC.state.mouseover) {
                if (!$.isEmptyObject(OSC.graphs)) {
                    var diff = event.pageX - curXPos;
                    curXPos = event.pageX;
                    var graphs_width = $('#graphs').width();
                    var graphs_offset = $('#graphs').offset().left;

                    var arrow_left = $('#time_offset_arrow').offset().left;
                    var arrow_width = $('#time_offset_arrow').width();

                    // For limits left and right moving of signals
                    var buffer_width = $('#buffer').width();
                    var buffer_offset = $('#buffer').offset().left;
                    var buf_arrow_width = $('#buf_time_offset').width();
                    var buf_arrow_offset = $('#buf_time_offset').offset().left;

                    if ((buf_arrow_offset + diff) <= buffer_offset && diff < 0) {
                        return;
                    }

                    if ((buf_arrow_offset + diff) >= (buffer_offset + buffer_width - buf_arrow_width) && diff > 0) {
                        return;
                    }

                    $('#time_offset_arrow').offset({ left: ($('#time_offset_arrow').offset().left + diff) })
                    OSC.moveTimeOffset($('#time_offset_arrow').position().left);
                }
            }
        });

        $('#time_offset_arrow, #time_offset_static_img').dblclick(function() {
            OSC.params.local['OSC_TIME_OFFSET'] = { value: 0 };
            OSC.sendParams();
        });



        $('#ext_clock_enable').click(function() {
            var elem = $(this);
            if (elem.text() == 'EXT. CLOCK') {
                elem.html('&check; EXT. CLOCK');
                $('#ext_clock_enable_view').show();
                OSC.params.local['EXT_CLOCK_ENABLE'] = { value: 1 };
            } else {
                elem.text('EXT. CLOCK');
                $('#ext_clock_enable_view').hide();
                OSC.params.local['EXT_CLOCK_ENABLE'] = { value: 0 };
            }
            OSC.sendParams();
        });

        $("#SOUR1_SWEEP_TIME").focus(function() {
            $(this).val(OSC.state.sweep_ch1_time);
            $(this).addClass("focus")
        });

        $("#SOUR1_SWEEP_TIME").focusout(function() {
            $(this).removeClass("focus")
        });

        $("#SOUR2_SWEEP_TIME").focus(function() {
            $(this).val(OSC.state.sweep_ch2_time);
            $(this).addClass("focus")
        });

        $("#SOUR2_SWEEP_TIME").focusout(function() {
            $(this).removeClass("focus")
        });

        $('input[type=text]:not(.no-arrows)').iLightInputNumber({
            mobile: false
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

        $('#slow_adc_info').click(function() {
            var elem = $(this);
            if (elem.text() == 'IN/E2') {
                elem.html('&check; IN/E2');
                $('#slow_adc_info_view').show();
            } else {
                elem.text('IN/E2');
                $('#slow_adc_info_view').hide();
            }
        });

        $('#save_settings').click(function() {
            $('#save_settings_dialog').modal("show");

            // OSC.params.local['CONTROL_CONFIG_SETTINGS'] = { value: 4 }; // SAVE
            // OSC.sendParams();
        });

        $('#reset_settings').click(function() {
            OSC.params.local['CONTROL_CONFIG_SETTINGS'] = { value: 1 }; // REQUEST_RESET
            OSC.sendParams();
        });

        // $(".btn").mouseup(function() {
        //     setTimeout(function() {
        //         OSC.updateLimits();
        //         OSC.formatVals();
        //     }, 20);
        // });

        // TODO need fix
        // $('#OSC_CH1_OFFSET_UNIT').bind("DOMSubtreeModified", function() {
        //     // OSC.updateLimits();
        //     OSC.formatVals();
        // });

        // // TODO need fix
        // $('#OSC_CH2_OFFSET_UNIT').bind("DOMSubtreeModified", function() {
        //     // OSC.updateLimits();
        //     OSC.formatVals();
        // });

        // // TODO need fix
        // $("#OSC_MATH_SCALE").bind("DOMSubtreeModified", function() {
        //     // OSC.updateLimits();
        //     OSC.formatVals();
        // });

        // // TODO need fix
        // $("#OSC_CH1_PROBE").change(function() {
        //     OSC.updateLimits();
        // });

        // // TODO need fix
        // $("#OSC_CH2_PROBE").change(function() {
        //     OSC.updateLimits();
        // });

        // OSC.updateLimits();
        OSC.formatVals();


    }


}(window.OSC = window.OSC || {}, jQuery));