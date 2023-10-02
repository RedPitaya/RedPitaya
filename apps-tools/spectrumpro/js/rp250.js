(function(SPEC, $, undefined) {
    SPEC.updateInterfaceFor250 = function(model) {
        if (model !== undefined) {
                if (model.value != "Z20_250_12" && model.value != "Z20_250_12_120") {
                    var nodes = document.getElementsByClassName("250_12_block");
                    [...nodes].forEach((element, index, array) => {
                        element.parentNode.removeChild(element);
                    });
                } else {
                    SPEC.rp_model = model;
                    SPEC.config.xmax = SPEC.rp_model === "Z20_250_12_120" ? 120 : 60;
                    $("#SOUR1_FREQ_FIX").attr("max", SPEC.rp_model === "Z20_250_12_120" ? 120e6 : 60e6);
                    $("#SOUR2_FREQ_FIX").attr("max", SPEC.rp_model === "Z20_250_12_120" ? 120e6 : 60e6);
                    $("#SOUR1_VOLT").attr("max", 10);
                    $("#SOUR2_VOLT").attr("max", 10);
                    $("#SOUR1_VOLT_OFFS").attr("max", 10);
                    $("#SOUR2_VOLT_OFFS").attr("max", 10);
                    $("#SOUR1_VOLT_OFFS").attr("min", -10);
                    $("#SOUR2_VOLT_OFFS").attr("min", -10);
                    $("#IN_GAIN_L").text("1:1");
                    $("#IN_GAIN_L2").text("1:20");
                    var nodes = document.getElementsByName("AMPLITUDE_NODE");
                    [...nodes].forEach((element, index, array) => {
                        element.classList.remove('col-xs-12');
                        element.classList.add('col-xs-6');
                    });
                }
            }
    };

    SPEC.processParametersZ250 = function(parameter, value) {
        switch (parameter) {
            case 'CH1_OUT_GAIN':
                $("#CH1_OUT_GAIN_L").text(value === 0 ? "x1" : "x5");
                break;
            case 'CH2_OUT_GAIN':
                $("#CH2_OUT_GAIN_L").text(value === 0 ? "x1" : "x5");
                break;
        }
    }

    SPEC.updateMaxLimitOnLoad = function(ch, value) {
        if (SPEC.rp_model == "Z20_250_12" || SPEC.rp_model == "Z20_250_12_120") {
            if (ch == "CH1") {
                if (value == 0) {
                    $("#SOUR1_VOLT").attr("max", 5);
                    $("#SOUR1_VOLT_OFFS").attr("max", 5);
                    $("#SOUR1_VOLT_OFFS").attr("min", -5);
                } else {
                    $("#SOUR1_VOLT").attr("max", 2.5);
                    $("#SOUR1_VOLT_OFFS").attr("max", 2.5);
                    $("#SOUR1_VOLT_OFFS").attr("min", -2.5);
                }
            }

            if (ch == "CH2") {
                if (value == 0) {
                    $("#SOUR2_VOLT").attr("max", 5);
                    $("#SOUR2_VOLT_OFFS").attr("max", 5);
                    $("#SOUR2_VOLT_OFFS").attr("min", -5);
                } else {
                    $("#SOUR2_VOLT").attr("max", 2.5);
                    $("#SOUR2_VOLT_OFFS").attr("max", 2.5);
                    $("#SOUR2_VOLT_OFFS").attr("min", -2.5);
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

    var tmrOut1 = null;
    var tmrOut2 = null;
    SPEC.updateOverheatInfo = function(channel, state) {
        if (SPEC.rp_model == "Z20_250_12" || SPEC.rp_model == "Z20_250_12_120") {
            if (channel == 1) {
                if (state == 1) {
                    if ($("#OUTPUT1_STATE_ON").attr("src") !== "img/overheating.png"){
                        $("#OUTPUT1_STATE_ON").attr("src", "img/overheating.png");
                        $("#OUTPUT1_STATE_ON2").attr("src", "img/overheating.png");
                    }
                    $("#OUTPUT1_BUTTON").attr("style", "opacity: 1");
                    if (tmrOut1 == null)
                        tmrOut1 = setInterval(blink_fire1, 1000);
                } else {
                    if (tmrOut1 != null) {
                        clearInterval(tmrOut1);
                        tmrOut1 = null;
                    }
                    if ($("#OUTPUT1_STATE_ON").attr("src") !== "img/green_led.png"){
                        $("#OUTPUT1_STATE_ON").attr("src", "img/green_led.png");
                    }
                    $("#OUTPUT1_STATE_ON2").attr("src", "");
                    $("#OUTPUT1_BUTTON").attr("style", "");
                }
            }

            if (channel == 2) {
                if (state == 1) {
                    if ($("#OUTPUT2_STATE_ON").attr("src") !== "img/overheating.png"){
                        $("#OUTPUT2_STATE_ON").attr("src", "img/overheating.png");
                        $("#OUTPUT2_STATE_ON2").attr("src", "img/overheating.png");
                    }
                    $("#OUTPUT2_BUTTON").attr("style", "opacity: 1");
                    if (tmrOut2 == null)
                        tmrOut2 = setInterval(blink_fire2, 1000);
                } else {
                    if (tmrOut2 != null) {
                        clearInterval(tmrOut2);
                        tmrOut2 = null;
                    }
                    if ($("#OUTPUT2_STATE_ON").attr("src") !== "img/green_led.png"){
                        $("#OUTPUT2_STATE_ON").attr("src", "img/green_led.png");
                    }
                    $("#OUTPUT2_STATE_ON2").attr("src", "");
                    $("#OUTPUT2_BUTTON").attr("style", "");
                }
            }
        }
    };

    SPEC.updateOverheatBlock = function(channel, state) {
        if (SPEC.rp_model == "Z20_250_12" || SPEC.rp_model == "Z20_250_12_120") {
            if (channel == 1) {
                if (state == 1) {
                    $("#OUTPUT1_STATE").attr("class", "btn");
                    $("#OUTPUT1_STATE").attr("aria-pressed", false);
                    SPEC.exitEditing(true);
                    $("#OUTPUT1_STATE").attr("disabled", true);
                } else {
                    $("#OUTPUT1_STATE").attr("disabled", false);
                }
            }

            if (channel == 2) {
                if (state == 1) {
                    $("#OUTPUT2_STATE").attr("class", "btn");
                    $("#OUTPUT2_STATE").attr("aria-pressed", false);
                    SPEC.exitEditing(true);
                    $("#OUTPUT2_STATE").attr("disabled", true);
                } else {
                    $("#OUTPUT2_STATE").attr("disabled", false);
                }
            }
        }
    };

    SPEC.updateExtClockLocked = function(state) {
        if (state == 1) {
            if ($("#EXT_CLOCK").attr("src") !== "img/green_led.png")
                $("#EXT_CLOCK").attr("src", "img/green_led.png");
        } else {
            if ($("#EXT_CLOCK").attr("src") !== "img/red_led.png")
                $("#EXT_CLOCK").attr("src", "img/red_led.png");
        }
    };

}(window.SPEC = window.SPEC || {}, jQuery));