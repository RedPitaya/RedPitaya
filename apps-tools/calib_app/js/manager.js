/*
 * Red Pitaya calib_app
 *
 * Author: Danilyuk Nikolay <n.danilyuk@integrasources.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function(OBJ, $, undefined) {

    OBJ.model = undefined;
    OBJ.maxGenFreq = 62500000;

    OBJ.setMainMenu = function(_visible) {
        if (_visible) {
            $("#main_menu_body").show();
            $("#B_APPLY_CONT").hide();
            $("#B_CLOSE_CONT").hide();
            $("#B_DEFAULT_CONT").hide();
            $("#B_AUTO_CLOSE_CONT").hide();
            $("#B_RESET_CONT").hide();
        } else {
            $("#main_menu_body").hide();
        }
    }

    OBJ.setAutoMode = function(_visible) {
        if (_visible) {
            $("#auto_mode_body").show();
            $("#B_AUTO_CLOSE_CONT").show();
        } else {
            $("#auto_mode_body").hide();
        }
    }

    OBJ.setFAutoMode = function(_visible) {
        if (_visible) {
            $("#fauto_mode_body").show();
            $("#B_AUTO_CLOSE_CONT").show();
        } else {
            $("#fauto_mode_body").hide();
        }
    }

    OBJ.setADCMode = function(_visible) {
        if (OBJ.model !== undefined) {
            if (OBJ.model === "Z10" || OBJ.amModel === "Z20_125") {

            }

            if (OBJ.model === "Z20_250_12") {

            }

            if (_visible) {
                $("#adc_mode_body").show();
                $("#B_APPLY_CONT").show();
                $("#B_CLOSE_CONT").show();
                $("#B_RESET_CONT").show();
            } else {
                $("#adc_mode_body").hide();
            }
        }
    }

    OBJ.setFILTERMode = function(_visible) {
        if (OBJ.model !== undefined) {
            if (OBJ.model === "Z10" || OBJ.amModel === "Z20_125") {

            }

            if (OBJ.model === "Z20_250_12") {

            }

            if (_visible) {
                $("#filter_mode_body").show();
                $("#B_APPLY_CONT").show();
                $("#B_CLOSE_CONT").show();
                $("#B_DEFAULT_CONT").show();
            } else {
                $("#filter_mode_body").hide();
            }
        }
    }

    OBJ.makeid = function(length) {
        var result = '';
        var characters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
        var charactersLength = characters.length;
        for (var i = 0; i < length; i++) {
            result += characters.charAt(Math.floor(Math.random() * charactersLength));
        }
        return result;
    }

    OBJ.setCH1Awg = function(_value) {
        OBJ.amSetCH1Avg(_value.value);
    }

    OBJ.setCH2Awg = function(_value) {
        OBJ.amSetCH2Avg(_value.value);
    }

    OBJ.setCH1Max = function(_value) {
        OBJ.amSetCH1Max(_value.value);
    }

    OBJ.setCH2Max = function(_value) {
        OBJ.amSetCH2Max(_value.value);
    }

    OBJ.showMainMenu = function() {
        OBJ.setADCMode(false);
        OBJ.setAutoMode(false);
        OBJ.setFILTERMode(false);
        OBJ.setFAutoMode(false);
        OBJ.setMainMenu(true);
    }

    OBJ.setModel = function(_value) {
        if (OBJ.model === undefined) {
            OBJ.model = _value.value;
            if (OBJ.model !== "Z20_250_12") {
                $("#manual_x1_x5_mode").remove();
                $("#manual_ac_dc_mode").remove();
            } else {
                $("#b_auto_menu").text("AUTO AC/DC");
                $("#b_manual_menu").text("MANUAL AC/DC");
            }
            if (OBJ.model !== "Z10" && OBJ.amModel !== "Z20_125") {
                $("#filter_calib_button").remove();
                $("#afilter_calib_button").remove();
            }

            if (OBJ.model === "Z10" || OBJ.amModel === "Z20_125") {
                $("#filter_calib_button").show();
                $("#afilter_calib_button").show();
            }

        }
        OBJ.amSetModel(_value);
        OBJ.famSetModel(_value);
    }

    OBJ.closeManualMode = function() {
        SM.param_callbacks["ch1_avg"] = undefined;
        SM.param_callbacks["ch2_avg"] = undefined;
        SM.param_callbacks["ch1_max"] = undefined;
        SM.param_callbacks["ch2_max"] = undefined;
        SM.param_callbacks["ch1_min"] = undefined;
        SM.param_callbacks["ch2_min"] = undefined;
        OBJ.showMainMenu();
    }




}(window.OBJ = window.OBJ || {}, jQuery));


// Page onload event handler
$(function() {

    $('#B_AUTO_MODE').on('click', function(ev) {
        SM.param_callbacks["ch1_avg"] = OBJ.setCH1Awg;
        SM.param_callbacks["ch2_avg"] = OBJ.setCH2Awg;
        SM.param_callbacks["ch1_max"] = OBJ.setCH1Max;
        SM.param_callbacks["ch2_max"] = OBJ.setCH2Max;
        SM.param_callbacks["ch1_min"] = undefined;
        SM.param_callbacks["ch2_min"] = undefined;
        OBJ.setMainMenu(false);
        OBJ.setAutoMode(true);
        OBJ.amClearTable();
        OBJ.amStartCalibration();
    });

    $('#B_ADC_MODE').on('click', function(ev) {
        SM.param_callbacks["ch1_avg"] = OBJ.adcSetCH1Avg;
        SM.param_callbacks["ch2_avg"] = OBJ.adcSetCH2Avg;
        SM.param_callbacks["ch1_max"] = OBJ.adcSetCH1Max;
        SM.param_callbacks["ch2_max"] = OBJ.adcSetCH2Max;
        SM.param_callbacks["ch1_min"] = OBJ.adcSetCH1Min;
        SM.param_callbacks["ch2_min"] = OBJ.adcSetCH2Min;
        OBJ.adcInitData();
        OBJ.adcInitRequest();
        OBJ.adcInitPlotCH1(true);
        OBJ.adcInitPlotCH2(true);
        OBJ.setMainMenu(false);
        OBJ.setADCMode(true);
    });

    $('#B_AUTO_FILTER_MODE').on('click', function(ev) {
        //  OBJ.filterInitRequest();
        OBJ.setMainMenu(false);
        OBJ.setFAutoMode(true);
        OBJ.famClearTable();
        OBJ.famStartCalibration();
    });

    $('#B_FILTER_MODE').on('click', function(ev) {
        OBJ.filterInitRequest();
        OBJ.setMainMenu(false);
        OBJ.setFILTERMode(true);
    });

    $('#B_APPLY').on('click', function(ev) {
        $("#dialog_reset_text").text("Apply new calibration?");
        $("#reset_ok_btn").off('click');
        $('#reset_cancel_btn').off('click');
        $('#reset_ok_btn').on('click', function() {
            SM.parametersCache["calib_sig"] = { value: 5 };
            SM.sendParameters();
            OBJ.adcCalibChange = false;
            OBJ.filterCalibChange = false;
        });
        $('#reset_cancel_btn').on('click', function() {});
        $("#dialog_reset").modal('show');
    });

    $('#B_RESET_DEFAULT').on('click', function(ev) {
        $("#dialog_reset_text").text("Reset to default?");
        $("#reset_ok_btn").off('click');
        $('#reset_cancel_btn').off('click');
        $('#reset_ok_btn').on('click', function() {
            SM.parametersCache["calib_sig"] = { value: 3 };
            SM.sendParameters();
            OBJ.adcCalibChange = false;
        });
        $('#reset_cancel_btn').on('click', function() {});
        $("#dialog_reset").modal('show');
    });

    $('#B_RESET_FACTORY').on('click', function(ev) {
        $("#dialog_reset_text").text("Reset to factory calibration?");
        $("#reset_ok_btn").off('click');
        $('#reset_cancel_btn').off('click');
        $('#reset_ok_btn').on('click', function() {
            SM.parametersCache["calib_sig"] = { value: 4 };
            SM.sendParameters();
            OBJ.adcCalibChange = false;
        });
        $('#reset_cancel_btn').on('click', function() {});
        $("#dialog_reset").modal('show');
    });

    $('#B_CANCEL_CALIB').on('click', function(ev) {
        SM.param_callbacks["ch1_avg"] = undefined;
        SM.param_callbacks["ch2_avg"] = undefined;
        SM.param_callbacks["ch1_max"] = undefined;
        SM.param_callbacks["ch2_max"] = undefined;
        SM.param_callbacks["ch1_min"] = undefined;
        SM.param_callbacks["ch2_min"] = undefined;
        OBJ.showMainMenu();
        SM.parametersCache["SS_NEXTSTEP"] = { value: -2 };
        SM.parametersCache["ref_volt"] = { value: 0 }; // SS_NEXTSTEP work only in pair ref_volt
        SM.sendParameters();
    });

    $('#B_CLOSE_ADC_CALIB').on('click', function(ev) {
        if (OBJ.adcCalibChange || OBJ.filterCalibChange) {
            $("#dialog_reset_text").text("Save new parameters?");
            $("#reset_ok_btn").off('click');
            $('#reset_cancel_btn').off('click');
            $('#reset_ok_btn').on('click', function() {
                SM.parametersCache["calib_sig"] = { value: 5 };
                OBJ.filterCalibChange = false;
                OBJ.adcCalibChange = false;
                SM.sendParameters();
                OBJ.closeManualMode();
            });
            $('#reset_cancel_btn').on('click', function() {
                OBJ.filterCalibChange = false;
                OBJ.adcCalibChange = false;
                OBJ.closeManualMode();
            });

            $("#dialog_reset").modal('show');
        } else {
            OBJ.closeManualMode();
        }
    });

    $('#B_DEFAULT').on('click', function(ev) {

        $("#dialog_reset_text").text("Set default parameters for current channel?");
        $("#reset_ok_btn").off('click');
        $('#reset_cancel_btn').off('click');
        $('#reset_ok_btn').on('click', function() {
            SM.parametersCache["calib_sig"] = { value: 6 };
            SM.sendParameters();
        });

        $('#reset_cancel_btn').on('click', function() {});

        $("#dialog_reset").modal('show');

    });


    SM.param_callbacks["RP_MODEL_STR"] = OBJ.setModel;



});