(function(OSC, $, undefined) {
    OSC.updateInterfaceFor250 = function(model) {
        if (model !== undefined) {
            if (model != "Z20_250_12" && model != "Z20_250_12_120") {
                var nodes = document.getElementsByClassName("250_12_block");
                [...nodes].forEach((element, index, array) => {
                                        element.parentNode.removeChild(element);
                                    });
            } else {
                OSC.rp_model = model;
                OSC.sample_rates = {1:'250M', 8:'31.25M', 64: '3.906M', 1024: '244.141k', 8192:'30.518k', 65536:'3.815k'};

                OSC.voltage_steps = [
                    // Millivolts
                    1 / 1000, 2 / 1000, 5 / 1000, 10 / 1000, 20 / 1000, 50 / 1000, 100 / 1000, 200 / 1000, 500 / 1000,
                    // Volts
                    1, 2, 5, 10, 20, 50
                ];
                $("#SOUR1_FREQ_FIX").attr("max", 250e6/2);
                $("#SOUR2_FREQ_FIX").attr("max", 250e6/2);

                $("#OSC_CH1_IN_GAIN_L").text("1:1");
                $("#OSC_CH1_IN_GAIN2_L").text("1:20");
                $("#OSC_CH2_IN_GAIN_L").text("1:1");
                $("#OSC_CH2_IN_GAIN2_L").text("1:20");
                OSC.trigger_limit = 10.0;
                var nodes = document.getElementsByName("AMPLITUDE_NODE");
                [...nodes].forEach((element, index, array) => {
                    element.classList.remove('col-xs-12');
                    element.classList.add('col-xs-6');
                });
            }
        }
    };

    OSC.processParametersZ250 = function(new_params,param_name) {
        var value = new_params[param_name].value
        switch(param_name){
            case 'OSC_CH1_OUT_GAIN':
                    $("#OSC_CH1_OUT_GAIN_L").text(value === 0 ? "x1" : "x5");
                break;
            case 'OSC_CH2_OUT_GAIN':
                    $("#OSC_CH2_OUT_GAIN_L").text(value === 0 ? "x1" : "x5");
                break;
        }
    }

    OSC.updateMaxLimitOnLoadHandler = function(new_params , param_name) {
        if (param_name === "SOUR1_IMPEDANCE"){
            OSC.updateMaxLimitOnLoad("CH1",new_params['SOUR1_IMPEDANCE'].value);
        }

        if (param_name === "SOUR2_IMPEDANCE"){
            OSC.updateMaxLimitOnLoad("CH2",new_params['SOUR2_IMPEDANCE'].value);
        }
        var field = $('#' + param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.val(new_params[param_name].value);
        }
    }

    OSC.updateMaxLimitOnLoad = function(ch,value) {
        if (OSC.rp_model == "Z20_250_12" || OSC.rp_model == "Z20_250_12_120") {
            if (ch == "CH1") {
                if (value == 0) {
                    // Hi-Z mode
                    $("#SOUR1_VOLT").attr("max", 10);
                    $("#SOUR1_VOLT_OFFS").attr("max", 10);
                    $("#SOUR1_VOLT_OFFS").attr("min", -10);
                }else{
                    // 50 omh mode
                    $("#SOUR1_VOLT").attr("max", 5);
                    $("#SOUR1_VOLT_OFFS").attr("max", 5);
                    $("#SOUR1_VOLT_OFFS").attr("min", -5);
                }
            }

            if (ch == "CH2") {
                if (value == 0) {
                   // Hi-Z mode
                   $("#SOUR2_VOLT").attr("max", 10);
                    $("#SOUR2_VOLT_OFFS").attr("max", 10);
                    $("#SOUR2_VOLT_OFFS").attr("min", -10);
                }else{
                    // 50 omh mode
                    $("#SOUR2_VOLT").attr("max", 5);
                    $("#SOUR2_VOLT_OFFS").attr("max", 5);
                    $("#SOUR2_VOLT_OFFS").attr("min", -5);
                }

            }
        }
    }



    function blink_fire1() {
        $("#OUTPUT1_STATE_ON").fadeOut(500);
        $("#OUTPUT1_STATE_ON2").fadeOut(500);
        $("#OUTPUT1_STATE_ON").fadeIn(500);
        $("#OUTPUT1_STATE_ON2").fadeIn(500);
    };

    function blink_fire2() {
        $("#OUTPUT2_STATE_ON").fadeOut(500);
        $("#OUTPUT2_STATE_ON2").fadeOut(500);
        $("#OUTPUT2_STATE_ON").fadeIn(500);
        $("#OUTPUT2_STATE_ON2").fadeIn(500);
    };

    OSC.updateOverheatInfoHandler = function(new_params , param_name) {
        if (param_name === "SOUR1_TEMP_LATCHED"){
            OSC.updateOverheatInfo(1,new_params['SOUR1_TEMP_LATCHED'].value);
        }

        if (param_name === "SOUR2_TEMP_LATCHED"){
            OSC.updateOverheatInfo(2,new_params['SOUR2_TEMP_LATCHED'].value);
        }
    }

    var tmrOut1 = null;
    var tmrOut2 = null;
    OSC.updateOverheatInfo = function(channel , state) {
        if (OSC.rp_model == "Z20_250_12" || OSC.rp_model == "Z20_250_12_120") {
            if (channel == 1){
                if (state == 1) {
                    $("#OUTPUT1_STATE_ON").attr("src","img/overheating.png");
                    $("#OUTPUT1_STATE_ON2").attr("src","img/overheating.png");
                    $("#OUTPUT1_BUTTON").attr("style","opacity: 1");
                    if(tmrOut1 == null)
                        tmrOut1 = setInterval(blink_fire1, 1000);
                } else {
                    if(tmrOut1 != null){
                        clearInterval(tmrOut1);
                        tmrOut1 = null;
                    }
                    $("#OUTPUT1_STATE_ON").attr("src","img/green_led.png");
                    $("#OUTPUT1_STATE_ON2").attr("src","");
                    $("#OUTPUT1_BUTTON").attr("style","");
                }
            }

            if (channel == 2){
                if (state == 1) {
                    $("#OUTPUT2_STATE_ON").attr("src","img/overheating.png");
                    $("#OUTPUT2_STATE_ON2").attr("src","img/overheating.png");
                    $("#OUTPUT2_BUTTON").attr("style","opacity: 1");
                    if(tmrOut2 == null)
                        tmrOut2 = setInterval(blink_fire2, 1000);
                } else {
                    if(tmrOut2 != null){
                        clearInterval(tmrOut2);
                        tmrOut2 = null;
                    }
                    $("#OUTPUT2_STATE_ON").attr("src","img/green_led.png");
                    $("#OUTPUT2_STATE_ON2").attr("src","");
                    $("#OUTPUT2_BUTTON").attr("style","");
                }
            }
        }
    };

    OSC.updateOverheatBlockHandler = function(new_params,param_name) {
        if (param_name === "SOUR1_TEMP_RUNTIME"){
            OSC.updateOverheatBlock(1,new_params['SOUR1_TEMP_RUNTIME'].value);
        }

        if (param_name === "SOUR2_TEMP_RUNTIME"){
            OSC.updateOverheatBlock(2,new_params['SOUR2_TEMP_RUNTIME'].value);
        }
    }

    OSC.updateOverheatBlock = function(channel , state) {
        if (OSC.rp_model == "Z20_250_12" || OSC.rp_model == "Z20_250_12_120") {
            if (channel == 1){
                if (state == 1) {
                    $("#OUTPUT1_STATE").attr("class", "btn");
                    $("#OUTPUT1_STATE").attr("aria-pressed", false);
                    OSC.exitEditing(true);
                    $("#OUTPUT1_STATE").attr("disabled", true);
                } else {
                    $("#OUTPUT1_STATE").attr("disabled", false);
                }
            }

            if (channel == 2){
                if (state == 1) {
                    $("#OUTPUT2_STATE").attr("class", "btn");
                    $("#OUTPUT2_STATE").attr("aria-pressed", false);
                    OSC.exitEditing(true);
                    $("#OUTPUT2_STATE").attr("disabled", true);
                } else {
                    $("#OUTPUT2_STATE").attr("disabled", false);
                }
            }
        }
    };

    OSC.updateExtClockLocked = function(new_params,param_name) {
        var state = new_params[param_name].value;
        if (state == 1) {
            if ($("#EXT_CLOCK").attr("src")!== "img/green_led.png")
                $("#EXT_CLOCK").attr("src","img/green_led.png");
        } else {
            if ($("#EXT_CLOCK").attr("src")!== "img/red_led.png")
                $("#EXT_CLOCK").attr("src","img/red_led.png");
        }
    };

    OSC.updateExtClockEnable = function(state) {
        if (state == 0) {
            $('#ext_clock_enable').text('EXT. CLOCK');
            $('#ext_clock_enable_view').hide();
        }
    }

}(window.OSC = window.OSC || {}, jQuery));
