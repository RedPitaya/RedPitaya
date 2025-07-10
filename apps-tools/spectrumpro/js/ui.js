(function(UI, $, undefined) {

    UI.formatVals = function() {
        // DUTY CYCLE FORMATTING
        {
            var SOUR1_DCYC = parseFloat($("SOUR1_DCYC").val());
            $("SOUR1_DCYC").val(SOUR1_DCYC.toFixed(1));
            var SOUR2_DCYC = parseFloat($("SOUR2_DCYC").val());
            $("SOUR2_DCYC").val(SOUR2_DCYC.toFixed(1));

            var SOUR1_PHAS = parseFloat($("SOUR1_PHAS").val());
            $("SOUR1_PHAS").val(SOUR1_PHAS.toFixed(1));
            var SOUR2_PHAS = parseFloat($("SOUR2_PHAS").val());
            $("SOUR2_PHAS").val(SOUR2_PHAS.toFixed(1));
        }
    }

    $.fn.iLightInputNumber = function(options) {
        var inBox = '.input-number-box';
        var newInput = '.input-number';
        var moreVal = '.input-number-more';
        var lessVal = '.input-number-less';

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

        var value;
        var step;
        var interval = null;
        var timeout = null;

        function ToggleValue(input) {
            input.val(parseInt(input.val(), 10) + d);
            // console.log(input);
        }

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
            var limits = getLimits(input);
            var min = limits.min;

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
            SPEC.exitEditing(true);
        }

        function checkInputAttr(input) {
            value = parseFloat(input.val());

            if (!($.isNumeric(value))) {
                value = 0;
            }

            if (input.attr('step')) {
                step = parseFloat(input.attr('step'));
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

            var parts = step.toString().split('.');
            var signs = parts.length < 2 ? 0 : parts[1].length;
            value = parseFloat(value.toFixed(signs));

            if (value < min) {
                value = min;
            } else if (value > max) {
                value = max;
            }

            if (!($.isNumeric(value))) {
                value = 0;
            }

            input.val(value);
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
    }

    UI.initHandlers = function() {
        $(".btn").mouseup(function() {
            setTimeout(function() {
                UI.formatVals();
            }, 20);
        });

        $('input[type=text]:not([readonly]):not(.no-arrows)[step]').iLightInputNumber({
            mobile: false
        });

        $("#ext_con_but").click(function(event) {
            $('#ext_connections_dialog').modal("show");
        });

        $('#save_settings').click(function() {
            $('#save_settings_dialog').modal("show");
        });

        $('#reset_settings').click(function() {
            CLIENT.params.local['CONTROL_CONFIG_SETTINGS'] = { value: 1 }; // REQUEST_RESET
            SPEC.sendParams();
        });

        $('#OSC_REQ_SAVE_SETTINGS').on('click', function() {
            var name = $("#SETTINGS_NEW_NAME").val().trim()
            if (name !== ""){
                CLIENT.params.local['FILE_SATTINGS'] = { value: name };
                CLIENT.params.local['CONTROL_CONFIG_SETTINGS'] = { value: 4 }; // SAVE
                SPEC.sendParams();
            }
        });
    }

    UI.uintToColor = function(uint) {
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

    UI.updateARBFunc = function(list) {
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
                var color = UI.uintToColor(parseInt(cols[1]))
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

    UI.initUIItems = function(params) {
        if (params['SOUR_IMPEDANCE_Z_MODE'] !== undefined){
            if (params['SOUR_IMPEDANCE_Z_MODE'].value == false){
                var nodes = document.getElementsByClassName("hi-z-mode");
                [...nodes].forEach((element, index, array) => {
                                        element.parentNode.removeChild(element);
                                    });

            }
        }

        if (params['SPEC_IS_FILTER'] !== undefined){
            if (params['SPEC_IS_FILTER'].value == false){
                var nodes = document.getElementsByClassName("filter_block");
                [...nodes].forEach((element, index, array) => {
                                        element.parentNode.removeChild(element);
                                    });

            }
        }

        if (params['SPEC_IS_AC_DC'] !== undefined){
            if (params['SPEC_IS_AC_DC'].value == false){
                var nodes = document.getElementsByClassName("ac_dc_block");
                [...nodes].forEach((element, index, array) => {
                                        element.parentNode.removeChild(element);
                                    });

            }
        }

        if (params['SPEC_IS_HV_LV'] !== undefined){
            if (params['SPEC_IS_HV_LV'].value == false){
                var nodes = document.getElementsByClassName("hv_lv_block");
                [...nodes].forEach((element, index, array) => {
                                        element.parentNode.removeChild(element);
                                    });

            }
        }

        if (params['SOUR_X5_GAIN'] !== undefined){
            if (params['SOUR_X5_GAIN'].value == false){
                var nodes = document.getElementsByClassName("x5_block");
                [...nodes].forEach((element, index, array) => {
                                        element.parentNode.removeChild(element);
                                    });

            }
        }

        if (params['SOUR1_FREQ_FIX'] !== undefined){
            $("#SOUR1_FREQ_FIX").attr("max", params['SOUR1_FREQ_FIX'].max).attr("min", params['SOUR1_FREQ_FIX'].min);
        }

        if (params['SOUR2_FREQ_FIX'] !== undefined){
            $("#SOUR2_FREQ_FIX").attr("max", params['SOUR2_FREQ_FIX'].max).attr("min", params['SOUR2_FREQ_FIX'].min);
        }
    };

    UI.updateMaxLimitOnLoad = function(ch, value) {
        if (SPEC.hi_z_mode == true) {
            var max_amp = SPEC.gen_max_amp;
            if (ch == "CH1") {
                if (value == 0) {
                    // Hi-Z mode
                    $("#SOUR1_VOLT").attr("max", max_amp);
                    $("#SOUR1_VOLT_OFFS").attr("max", max_amp);
                    $("#SOUR1_VOLT_OFFS").attr("min", -1 * max_amp);
                }else{
                    // 50 omh mode
                    $("#SOUR1_VOLT").attr("max", max_amp/ 2.0);
                    $("#SOUR1_VOLT_OFFS").attr("max", max_amp / 2.0);
                    $("#SOUR1_VOLT_OFFS").attr("min", -1 * max_amp / 2.0);
                }
            }
    
            if (ch == "CH2") {
                if (value == 0) {
                   // Hi-Z mode
                    $("#SOUR2_VOLT").attr("max", max_amp);
                    $("#SOUR2_VOLT_OFFS").attr("max", max_amp);
                    $("#SOUR2_VOLT_OFFS").attr("min", -1 * max_amp);
                }else{
                    // 50 omh mode
                    $("#SOUR2_VOLT").attr("max", max_amp / 2.0);
                    $("#SOUR2_VOLT_OFFS").attr("max", max_amp / 2.0);
                    $("#SOUR2_VOLT_OFFS").attr("min", -1 * max_amp / 2.0);
                }
    
            }
        }else{
            var max_amp = SPEC.gen_max_amp;
            $("#SOUR1_VOLT").attr("max", max_amp);
            $("#SOUR1_VOLT_OFFS").attr("max", max_amp);
            $("#SOUR1_VOLT_OFFS").attr("min", -1 * max_amp);
            $("#SOUR2_VOLT").attr("max", max_amp);
            $("#SOUR2_VOLT_OFFS").attr("max", max_amp);
            $("#SOUR2_VOLT_OFFS").attr("min", -1 * max_amp);
        }
    }

}(window.UI = window.UI || {}, jQuery));
