/*
 * Red Pitaya calib_app
 *
 * Author: Danilyuk Nikolay <n.danilyuk@integrasources.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


(function(OBJ, $, undefined) {


    OBJ.STATES_125_14 = {
        0: { name: "Reset to default", span: true },
        1: { name: "ADC offset (1:20)", img: "./img/125/RP_125_GND_HV.png", hint: "Please set HV mode and connect IN1 and IN2 to GND." },
        2: { name: "ADC gain   (1:20)", img: "./img/125/RP_125_REF_HV.png", hint: "Please set HV mode and connect IN1 and IN2 to reference DC source.", input: 5 },
        3: { name: "ADC offset (1:1)", img: "./img/125/RP_125_GND.png", hint: "Please set LV mode and connect IN1 and IN2 to GND." },
        4: { name: "ADC gain   (1:1)", img: "./img/125/RP_125_REF.png", hint: "Please set LV mode and connect IN1 and IN2 to reference DC source.", input: 0.5 },
        5: { name: "Disable DAC", span: true },
        6: { name: "DAC offset", img: "./img/125/RP_125_GEN.png", hint: "Please set LV mode and connect OUT1 to IN1 and OUT2 to IN2." },
        7: { name: "Enable DAC", span: true },
        8: { name: "DAC gain", img: "./img/125/RP_125_GEN.png", hint: "Please set LV mode and connect OUT1 to IN1 and OUT2 to IN2." },
        9: { name: "Calibration complete", span: true, end: true }
    };

    OBJ.STATES_125_14_4CH = {
        0: { name: "Reset to default", span: true },
        1: { name: "ADC offset (1:20)", img: "./img/125_4CH/RP_125_GND_HV.png", hint: "Please set HV mode and connect IN1, IN2, IN3 and IN4 to GND." },
        2: { name: "ADC gain   (1:20)", img: "./img/125_4CH/RP_125_REF_HV.png", hint: "Please set HV mode and connect IN1, IN2, IN3 and IN4 to reference DC source.", input: 5 },
        3: { name: "ADC offset (1:1)", img: "./img/125_4CH/RP_125_GND.png", hint: "Please set LV mode and connect IN1, IN2, IN3 and IN4 to GND." },
        4: { name: "ADC gain   (1:1)", img: "./img/125_4CH/RP_125_REF.png", hint: "Please set LV mode and connect IN1, IN2, IN3 and IN4 to reference DC source.", input: 0.5 },
        5: { name: "Calibration complete", span: true, end: true }
    };

    OBJ.STATES_250_12 = {
        0: { name: "Reset to default", span: true },
        1: { name: "Set 1:1 DC mode", span: true },
        2: { name: "ADC offset DC (1:1)", img: "./img/250/RP_250_GND.png", hint: "Please connect IN1 and IN2 to GND." },
        3: { name: "ADC gain DC   (1:1)", img: "./img/250/RP_250_REF.png", hint: "Please connect IN1 and IN2 to reference DC source.", input: 0.5 },
        4: { name: "Set 1:20 DC mode", span: true },
        5: { name: "ADC offset DC (1:20)", img: "./img/250/RP_250_GND.png", hint: "Please connect IN1 and IN2 to GND." },
        6: { name: "ADC gain DC   (1:20)", img: "./img/250/RP_250_REF.png", hint: "Please connect IN1 and IN2 to reference DC source.", input: 1 },
        7: { name: "Disable DAC", span: true },
        8: { name: "DAC offset x1", img: "./img/250/RP_250_GEN.png", hint: "Please connect OUT1 to IN1 and OUT2 to IN2." },
        9: { name: "Enable DAC", span: true },
        10: { name: "DAC gain x1", img: "./img/250/RP_250_GEN.png", hint: "Please connect OUT1 to IN1 and OUT2 to IN2." },
        11: { name: "Disable DAC", span: true },
        12: { name: "DAC offset x5", img: "./img/250/RP_250_GEN.png", hint: "Please connect OUT1 to IN1 and OUT2 to IN2." },
        13: { name: "Enable DAC", span: true },
        14: { name: "DAC gain x5", img: "./img/250/RP_250_GEN.png", hint: "Please connect OUT1 to IN1 and OUT2 to IN2." },
        15: { name: "Set 1:1 AC mode and disable dac", span: true },
        16: { name: "ADC offset AC (1:1)", img: "./img/250/RP_250_GND.png", hint: "Please connect IN1 and IN2 to GND.", use_max: true },
        17: { name: "Enable DAC", span: true },
        18: { name: "ADC gain AC   (1:1)", img: "./img/250/RP_250_GEN.png", hint: "Please connect OUT1 to IN1 and OUT2 to IN2.", use_max: true },
        19: { name: "Set 1:20 AC mode and disable dac", span: true },
        20: { name: "ADC offset AC (1:20)", img: "./img/250/RP_250_GND.png", hint: "Please connect IN1 and IN2 to GND.", use_max: true },
        21: { name: "Enable DAC", span: true },
        22: { name: "ADC gain AC   (1:20)", img: "./img/250/RP_250_GEN.png", hint: "Please connect OUT1 to IN1 and OUT2 to IN2.", use_max: true },
        23: { name: "Calibration complete", span: true, end: true }
    };

    OBJ.amModel = undefined;
    OBJ.amStates = undefined;
    OBJ.amCurrentTest = undefined;
    OBJ.amCurrentSuccesTest = undefined;
    OBJ.amCurrentRowID = undefined;
    OBJ.amLastAVGCH1 = 0;
    OBJ.amLastAVGCH2 = 0;
    OBJ.amLastAVGCH3 = 0;
    OBJ.amLastAVGCH4 = 0;
    OBJ.amLastState = false;

    OBJ.amSetModel = function(_model) {
        if (OBJ.amModel === undefined) {
            OBJ.amModel = _model.value;
            if (OBJ.amModel === "Z10" || OBJ.amModel === "Z20_125") OBJ.amStates = OBJ.STATES_125_14;
            if (OBJ.amModel === "Z20_125_4CH") OBJ.amStates = OBJ.STATES_125_14_4CH;
            if (OBJ.amModel === "Z20_250_12") OBJ.amStates = OBJ.STATES_250_12;
            if (OBJ.amModel === "Z20_250_12_120") OBJ.amStates = OBJ.STATES_250_12;

            $('#am_ok_btn').on('click', function() { OBJ.amClickOkDialog() });
        }
    }

    OBJ.amClearTable = function() {
        $("#auto_calib_table").empty();
    }

    OBJ.amSetLedState = function(_state) {
        if (_state) {
            $("#SS_REF_VLOT_STATE").attr("src", "img/green_led.png");
        } else {
            $("#SS_REF_VLOT_STATE").attr("src", "img/red_led.png");
        }
        OBJ.amLastState = _state;
    }

    OBJ.amAddNewRow = function() {
        var table = document.getElementById("auto_calib_table");
        var row = table.insertRow(-1);
        var id = OBJ.makeid(8);
        row.setAttribute("id", id);
        let newCell = row.insertCell(-1);
        newCell.setAttribute("id", id + "_mode");
        newCell = row.insertCell(-1);
        newCell.setAttribute("id", id + "_ch1_befor");
        newCell = row.insertCell(-1);
        newCell.setAttribute("id", id + "_ch1_after");
        newCell = row.insertCell(-1);
        newCell.setAttribute("id", id + "_ch2_befor");
        newCell = row.insertCell(-1);
        newCell.setAttribute("id", id + "_ch2_after");
        if (OBJ.amModel === "Z20_125_4CH"){
            newCell = row.insertCell(-1);
            newCell.setAttribute("id", id + "_ch3_befor");
            newCell = row.insertCell(-1);
            newCell.setAttribute("id", id + "_ch3_after");
            newCell = row.insertCell(-1);
            newCell.setAttribute("id", id + "_ch4_befor");
            newCell = row.insertCell(-1);
            newCell.setAttribute("id", id + "_ch4_after");
        }
        newCell = row.insertCell(-1);
        newCell.setAttribute("id", id + "_value_ch1");
        newCell = row.insertCell(-1);
        newCell.setAttribute("id", id + "_value_ch2");
        if (OBJ.amModel === "Z20_125_4CH"){
            newCell = row.insertCell(-1);
            newCell.setAttribute("id", id + "_value_ch3");
            newCell = row.insertCell(-1);
            newCell.setAttribute("id", id + "_value_ch4");
        }
        newCell = row.insertCell(-1);
        newCell.setAttribute("id", id + "_state");
        return id;
    }

    OBJ.amAddNewRowSpan = function() {
        var table = document.getElementById("auto_calib_table");
        var row = table.insertRow(-1);
        var id = OBJ.makeid(8);
        row.setAttribute("id", id);
        let newCell = row.insertCell(-1);
        newCell.setAttribute("id", id + "_mode");
        if (OBJ.amModel === "Z20_125_4CH"){
            newCell.setAttribute("colspan", "13");
        }else{
            newCell.setAttribute("colspan", "7");
        }
        newCell = row.insertCell(-1);
        newCell.setAttribute("id", id + "_state");
        return id;
    }

    OBJ.amCheckEmptyVariables = function() {
        return (OBJ.amCurrentTest != undefined && OBJ.amCurrentRowID != undefined && OBJ.amStates != undefined);
    }

    OBJ.amSetRowName = function(_id, _name) {
        $("#" + _id + "_mode").text(_name);
    }

    OBJ.amSetName = function() {
        if (OBJ.amCheckEmptyVariables()) {
            OBJ.amSetRowName(OBJ.amCurrentRowID, OBJ.amStates[OBJ.amCurrentTest].name);
        }
    }

    OBJ.amSetStartButton = function() {
        if (OBJ.amCheckEmptyVariables()) {
            var element = document.getElementById(OBJ.amCurrentRowID + "_state");
            var d = document.createElement("div");
            element.appendChild(d);
            d.setAttribute("class", "am_start_buttons");
            var l = document.createElement("li");
            d.appendChild(l);
            var a = document.createElement("a");
            l.appendChild(a);
            a.setAttribute("href", "#");
            if (!OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("end")) {
                a.innerHTML = "START";
                l.onclick = OBJ.amStartButtonPress;
            } else {
                a.innerHTML = "DONE";
                l.onclick = OBJ.showMainMenu;
            }
        }
    }

    OBJ.amRemoveStateObject = function() {
        if (OBJ.amCheckEmptyVariables()) {
            OBJ.amRemoveStateObjectId(OBJ.amCurrentRowID);
        }
    }

    OBJ.amRemoveStateObjectId = function(_id) {
        var element = document.getElementById(_id + "_state");
        element.innerHTML = '';
    }

    OBJ.amSetWait = function() {
        if (OBJ.amCheckEmptyVariables()) {
            var element = document.getElementById(OBJ.amCurrentRowID + "_state");
            var d2 = document.createElement("div");
            element.appendChild(d2);
            d2.setAttribute("class", "lds-ellipsis");
            d2.appendChild(document.createElement("div"));
            d2.appendChild(document.createElement("div"));
            d2.appendChild(document.createElement("div"));
            d2.appendChild(document.createElement("div"));
        }
    }

    OBJ.amSetOkState = function() {
        if (OBJ.amCheckEmptyVariables()) {
            OBJ.amSetOkStateId(OBJ.amCurrentRowID);
        }
    }

    OBJ.amSetOkStateId = function(_id) {
        var element = document.getElementById(_id + "_state");
        var elem = document.createElement("img");
        elem.setAttribute("src", "./img/ok.png");
        elem.setAttribute("height", "16");
        elem.setAttribute("width", "16");
        element.appendChild(elem);
    }

    OBJ.amStartButtonPress = function() {
        if (OBJ.amStates[OBJ.amCurrentTest] !== undefined) {
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("span")) {
                OBJ.amRemoveStateObject();
                OBJ.amSetWait();
                OBJ.amSendState(OBJ.amCurrentTest, 0);
            } else {
                OBJ.amShowDloag();
            }
        }
    }

    OBJ.amAddNextStepRow = function() {
        if (OBJ.amStates[OBJ.amCurrentTest] !== undefined) {
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("span")) {
                OBJ.amCurrentRowID = OBJ.amAddNewRowSpan();
            } else {
                OBJ.amCurrentRowID = OBJ.amAddNewRow();
            }
            OBJ.amSetName();
            OBJ.amSetStartButton();
        }
    }

    OBJ.amGetStatus = function(_state) {
        if (_state.value === -1) return;
        if (OBJ.amCurrentSuccesTest != _state.value) {
            console.log(_state);
            OBJ.amCurrentSuccesTest = _state.value;
            setTimeout(function() {
                OBJ.amRemoveStateObject();
                OBJ.amSetOkState();
                OBJ.amContinueCalibration();
            }, 2000);
        }
    }

    OBJ.amShowDloag = function() {
        OBJ.amLastState = true;
        if (OBJ.amCheckEmptyVariables()) {
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("img")) {
                $("#am_dialog_img").attr("src", OBJ.amStates[OBJ.amCurrentTest].img);
            } else {
                $("#am_dialog_img").attr("src", "");
            }
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("hint")) {
                $("#am_dialog_text").text(OBJ.amStates[OBJ.amCurrentTest].hint);
            } else {
                $("#am_dialog_text").text("");
            }
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("input")) {
                $("#SS_REF_VOLT").val(OBJ.amStates[OBJ.amCurrentTest].input);
                $("#am_dialog_input").show();
            } else {
                $("#am_dialog_input").hide();
            }
        }
        $("#am_dialog_calib").modal('show');
    }

    OBJ.amClickOkDialog = function() {
        if (OBJ.amLastState === false) return;
        OBJ.amRemoveStateObject();
        OBJ.amSetWait();
        var ref = 0;
        if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("input")) {
            ref = $("#SS_REF_VOLT").val();
        }
        OBJ.amSendState(OBJ.amCurrentTest, ref);
        $("#am_dialog_calib").modal('hide');
    }

    OBJ.amStartCalibration = function() {
        OBJ.amCurrentTest = 0;
        OBJ.amAddNextStepRow();
        // OBJ.amCurrentTest = 1;
        // OBJ.amCurrentRowID = OBJ.amAddNewRow();
        // OBJ.amSetName();
        // OBJ.amSetStartButton();
    }

    OBJ.amContinueCalibration = function() {
        OBJ.amCurrentTest++;
        OBJ.amAddNextStepRow();
    }

    OBJ.amSendState = function(_state, _ref_volt) {
        SM.parametersCache["SS_NEXTSTEP"] = { value: _state };
        SM.parametersCache["SS_STATE"] = { value: _state - 1 };
        OBJ.amCurrentSuccesTest = _state - 1;
        SM.parametersCache["ref_volt"] = { value: _ref_volt };
        SM.sendParameters();
    }

    OBJ.amCheckInputRef = function() {
        var ref = parseFloat($("#SS_REF_VOLT").val());
        var state = true;
        if (isNaN(ref) === false) {
            if ((ref * 0.75 > OBJ.amLastAVGCH1) || (OBJ.amLastAVGCH1 > ref * 1.25)) {
                state = false;
            }
            if ((ref * 0.75 > OBJ.amLastAVGCH2) || (OBJ.amLastAVGCH2 > ref * 1.25)) {
                state = false;
            }
        } else {
            state = false;
        }
        OBJ.amSetLedState(state);
    }

    OBJ.amSetCH1Avg = function(_value) {
        if (OBJ.amCheckEmptyVariables()) {
            OBJ.amLastAVGCH1 = _value;
            if (!OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_max")) {
                _value = _value.toFixed(4);
                var element_b = document.getElementById(OBJ.amCurrentRowID + "_ch1_befor");
                var element_a = document.getElementById(OBJ.amCurrentRowID + "_ch1_after");
                if (OBJ.amCurrentTest !== OBJ.amCurrentSuccesTest) {
                    if (element_b != undefined) element_b.innerText = _value + " V";
                } else {
                    if (element_a != undefined) element_a.innerText = _value + " V";
                }
            }
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("input")) {
                OBJ.amCheckInputRef();
            }
        }
    }

    OBJ.amSetCH2Avg = function(_value) {
        if (OBJ.amCheckEmptyVariables()) {
            OBJ.amLastAVGCH2 = _value;
            if (!OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_max")) {
                _value = _value.toFixed(4);
                var element_b = document.getElementById(OBJ.amCurrentRowID + "_ch2_befor");
                var element_a = document.getElementById(OBJ.amCurrentRowID + "_ch2_after");
                if (OBJ.amCurrentTest !== OBJ.amCurrentSuccesTest) {
                    if (element_b != undefined) element_b.innerText = _value + " V";
                } else {
                    if (element_a != undefined) element_a.innerText = _value + " V";
                }
            }
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("input")) {
                OBJ.amCheckInputRef();
            }
        }
    }

    OBJ.amSetCH3Avg = function(_value) {
        if (OBJ.amCheckEmptyVariables()) {
            OBJ.amLastAVGCH3 = _value;
            if (!OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_max")) {
                _value = _value.toFixed(4);
                var element_b = document.getElementById(OBJ.amCurrentRowID + "_ch3_befor");
                var element_a = document.getElementById(OBJ.amCurrentRowID + "_ch3_after");
                if (OBJ.amCurrentTest !== OBJ.amCurrentSuccesTest) {
                    if (element_b != undefined) element_b.innerText = _value + " V";
                } else {
                    if (element_a != undefined) element_a.innerText = _value + " V";
                }
            }
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("input")) {
                OBJ.amCheckInputRef();
            }
        }
    }

    OBJ.amSetCH4Avg = function(_value) {
        if (OBJ.amCheckEmptyVariables()) {
            OBJ.amLastAVGCH4 = _value;
            if (!OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_max")) {
                _value = _value.toFixed(4);
                var element_b = document.getElementById(OBJ.amCurrentRowID + "_ch4_befor");
                var element_a = document.getElementById(OBJ.amCurrentRowID + "_ch4_after");
                if (OBJ.amCurrentTest !== OBJ.amCurrentSuccesTest) {
                    if (element_b != undefined) element_b.innerText = _value + " V";
                } else {
                    if (element_a != undefined) element_a.innerText = _value + " V";
                }
            }
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("input")) {
                OBJ.amCheckInputRef();
            }
        }
    }

    OBJ.amSetCH1Max = function(_value) {
        if (OBJ.amCheckEmptyVariables()) {
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_max")) {
                _value = _value.toFixed(4);
                var element_b = document.getElementById(OBJ.amCurrentRowID + "_ch1_befor");
                var element_a = document.getElementById(OBJ.amCurrentRowID + "_ch1_after");
                if (OBJ.amCurrentTest !== OBJ.amCurrentSuccesTest) {
                    if (element_b != undefined) element_b.innerText = _value + " V";
                } else {
                    if (element_a != undefined) element_a.innerText = _value + " V";
                }
            }
        }
    }

    OBJ.amSetCH2Max = function(_value) {
        if (OBJ.amCheckEmptyVariables()) {
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_max")) {
                _value = _value.toFixed(4);
                var element_b = document.getElementById(OBJ.amCurrentRowID + "_ch2_befor");
                var element_a = document.getElementById(OBJ.amCurrentRowID + "_ch2_after");
                if (OBJ.amCurrentTest !== OBJ.amCurrentSuccesTest) {
                    if (element_b != undefined) element_b.innerText = _value + " V";
                } else {
                    if (element_a != undefined) element_a.innerText = _value + " V";
                }
            }
        }
    }

    OBJ.amSetCH3Max = function(_value) {
        if (OBJ.amCheckEmptyVariables()) {
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_max")) {
                _value = _value.toFixed(4);
                var element_b = document.getElementById(OBJ.amCurrentRowID + "_ch3_befor");
                var element_a = document.getElementById(OBJ.amCurrentRowID + "_ch3_after");
                if (OBJ.amCurrentTest !== OBJ.amCurrentSuccesTest) {
                    if (element_b != undefined) element_b.innerText = _value + " V";
                } else {
                    if (element_a != undefined) element_a.innerText = _value + " V";
                }
            }
        }
    }

    OBJ.amSetCH4Max = function(_value) {
        if (OBJ.amCheckEmptyVariables()) {
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_max")) {
                _value = _value.toFixed(4);
                var element_b = document.getElementById(OBJ.amCurrentRowID + "_ch4_befor");
                var element_a = document.getElementById(OBJ.amCurrentRowID + "_ch4_after");
                if (OBJ.amCurrentTest !== OBJ.amCurrentSuccesTest) {
                    if (element_b != undefined) element_b.innerText = _value + " V";
                } else {
                    if (element_a != undefined) element_a.innerText = _value + " V";
                }
            }
        }
    }

    OBJ.amSetCalibValueCh1 = function(_value) {
        if (OBJ.amCheckEmptyVariables()) {
            var element_b = document.getElementById(OBJ.amCurrentRowID + "_value_ch1");
            if (element_b != undefined) element_b.innerText = _value.value;
        }
    }

    OBJ.amSetCalibValueCh2 = function(_value) {
        if (OBJ.amCheckEmptyVariables()) {
            var element_b = document.getElementById(OBJ.amCurrentRowID + "_value_ch2");
            if (element_b != undefined) element_b.innerText = _value.value;
        }
    }

    OBJ.amSetCalibValueCh3 = function(_value) {
        if (OBJ.amCheckEmptyVariables()) {
            var element_b = document.getElementById(OBJ.amCurrentRowID + "_value_ch3");
            if (element_b != undefined) element_b.innerText = _value.value;
        }
    }

    OBJ.amSetCalibValueCh4 = function(_value) {
        if (OBJ.amCheckEmptyVariables()) {
            var element_b = document.getElementById(OBJ.amCurrentRowID + "_value_ch4");
            if (element_b != undefined) element_b.innerText = _value.value;
        }
    }


}(window.OBJ = window.OBJ || {}, jQuery));


// Page onload event handler
$(function() {

    SM.param_callbacks["SS_STATE"] = OBJ.amGetStatus;
    SM.param_callbacks["ch1_calib_pass"] = OBJ.amSetCalibValueCh1;
    SM.param_callbacks["ch2_calib_pass"] = OBJ.amSetCalibValueCh2;
    SM.param_callbacks["ch3_calib_pass"] = OBJ.amSetCalibValueCh3;
    SM.param_callbacks["ch4_calib_pass"] = OBJ.amSetCalibValueCh4;

});