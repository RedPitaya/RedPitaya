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
        0: { name: "Reset to default", span: true, command:"RESET_DEFAULT" },
        1: { name: "ADC set in HV", span: true, command:"INIT_ADC_HV" },
        2: { name: "ADC offset (1:20)", img: "./img/125/RP_125_GND_HV.png", hint: "Please set HV mode and connect IN1 and IN2 to GND." , command:"CALIB_ADC_HV_OFFSET", show_adc : true},
        3: { name: "ADC gain   (1:20)", img: "./img/125/RP_125_REF_HV.png", hint: "Please set HV mode and connect IN1 and IN2 to reference DC source.", input: 5 , command:"CALIB_ADC_HV_GAIN", show_adc : true, use_float: true},
        4: { name: "ADC set in LV", span: true, command:"INIT_ADC_LV" },
        5: { name: "ADC offset (1:1)", img: "./img/125/RP_125_GND.png", hint: "Please set LV mode and connect IN1 and IN2 to GND." , command:"CALIB_ADC_LV_OFFSET", show_adc : true},
        6: { name: "ADC gain   (1:1)", img: "./img/125/RP_125_REF.png", hint: "Please set LV mode and connect IN1 and IN2 to reference DC source.", input: 0.5, command:"CALIB_ADC_LV_GAIN" , show_adc : true, use_float: true},
        7: { name: "Enable DAC", span: true , command: "GEN_LV_0_5"},
        8: { name: "DAC Gain First Stage", img: "./img/125/RP_125_GEN.png", hint: "Please set LV mode and connect OUT1 to IN1 and OUT2 to IN2." , command:"GEN_LV_CALIB_GAIN_F_STAGE", show_adc : true, use_float: true},
        9: { name: "Disable DAC", span: true , command: "GEN_DISABLE"},
        10: { name: "DAC Offset First Stage",img: "./img/125/RP_125_GEN.png", hint: "Please set LV mode and connect OUT1 to IN1 and OUT2 to IN2." , command:"GEN_LV_CALIB_OFFSET_F_STAGE", show_adc : true},
        11: { name: "Enable DAC", span: true , command: "GEN_LV_0_5"},
        12: { name: "DAC Gain Second Stage", img: "./img/125/RP_125_GEN.png", hint: "Please set LV mode and connect OUT1 to IN1 and OUT2 to IN2." , command:"GEN_LV_CALIB_GAIN_S_STAGE", show_adc : true, use_float: true},
        13: { name: "Disable DAC", span: true , command: "GEN_DISABLE"},
        14: { name: "DAC Offset Second Stage",img: "./img/125/RP_125_GEN.png", hint: "Please set LV mode and connect OUT1 to IN1 and OUT2 to IN2." , command:"GEN_LV_CALIB_OFFSET_S_STAGE", show_adc : true},
        15: { name: "Enable DAC", span: true , command: "GEN_LV_0_5"},
        16: { name: "DAC Gain Third Stage", img: "./img/125/RP_125_GEN.png", hint: "Please set LV mode and connect OUT1 to IN1 and OUT2 to IN2." , command:"GEN_LV_CALIB_GAIN_S_STAGE", show_adc : true, use_float: true},
        17: { name: "Calibration complete", span: true, end: true , command:""}
    };

    OBJ.STATES_125_14_4CH = {
        0: { name: "Reset to default", span: true, command:"RESET_DEFAULT" },
        1: { name: "ADC set in HV", span: true, command:"INIT_ADC_HV" },
        2: { name: "ADC offset (1:20)", img: "./img/125_4CH/RP_125_GND_HV.png", hint: "Please set HV mode and connect IN1, IN2, IN3 and IN4 to GND." , command:"CALIB_ADC_HV_OFFSET", show_adc : true},
        3: { name: "ADC gain   (1:20)", img: "./img/125_4CH/RP_125_REF_HV.png", hint: "Please set HV mode and connect IN1, IN2, IN3 and IN4 to reference DC source.", input: 5, command:"CALIB_ADC_HV_GAIN", show_adc : true, use_float: true },
        4: { name: "ADC set in LV", span: true, command:"INIT_ADC_LV" },
        5: { name: "ADC offset (1:1)", img: "./img/125_4CH/RP_125_GND.png", hint: "Please set LV mode and connect IN1, IN2, IN3 and IN4 to GND.", command:"CALIB_ADC_LV_OFFSET", show_adc : true },
        6: { name: "ADC gain   (1:1)", img: "./img/125_4CH/RP_125_REF.png", hint: "Please set LV mode and connect IN1, IN2, IN3 and IN4 to reference DC source.", input: 0.5, command:"CALIB_ADC_LV_GAIN" , show_adc : true, use_float: true },
        7: { name: "Calibration complete", span: true, end: true }
    };

    OBJ.STATES_250_12 = {
        0: { name: "Reset to default", span: true, command:"RESET_DEFAULT" },
        1: { name: "Set 1:1 DC mode", span: true , command:"INIT_ADC_LV_AC"},
        2: { name: "ADC offset DC (1:1)", img: "./img/250/RP_250_GND.png", hint: "Please connect IN1 and IN2 to GND." , command:"CALIB_ADC_LV_DC_OFFSET", show_adc : true },
        3: { name: "ADC gain DC   (1:1)", img: "./img/250/RP_250_REF.png", hint: "Please connect IN1 and IN2 to reference DC source.", input: 0.5, command:"CALIB_ADC_LV_DC_GAIN" , show_adc : true, use_float: true },
        4: { name: "Set 1:20 DC mode", span: true , span: true , command:"INIT_ADC_HV_DC"},
        5: { name: "ADC offset DC (1:20)", img: "./img/250/RP_250_GND.png", hint: "Please connect IN1 and IN2 to GND." , command:"CALIB_ADC_HV_DC_OFFSET", show_adc : true},
        6: { name: "ADC gain DC   (1:20)", img: "./img/250/RP_250_REF.png", hint: "Please connect IN1 and IN2 to reference DC source.", input: 1, command:"CALIB_ADC_HV_DC_GAIN" , show_adc : true, use_float: true  },
        
        7: { name: "Enable DAC (x1)", span: true , command: "GEN_LV_0_5_X1"},
        8: { name: "DAC Gain First Stage (x1)", img: "./img/250/RP_250_GEN.png", hint: "Please connect OUT1 to IN1 and OUT2 to IN2." , command:"GEN_LV_CALIB_GAIN_F_STAGE_X1", show_adc : true, use_float: true},
        9: { name: "Disable DAC (x1)", span: true , command: "GEN_DISABLE_X1"},
        10: { name: "DAC Offset First Stage (x1)",img: "./img/250/RP_250_GEN.png", hint: "Please connect OUT1 to IN1 and OUT2 to IN2." , command:"GEN_LV_CALIB_OFFSET_F_STAGE_X1", show_adc : true},
        11: { name: "Enable DAC (x1)", span: true , command: "GEN_LV_0_5_X1"},
        12: { name: "DAC Gain Second Stage (x1)", img: "./img/250/RP_250_GEN.png", hint: "Please connect OUT1 to IN1 and OUT2 to IN2." , command:"GEN_LV_CALIB_GAIN_S_STAGE_X1", show_adc : true, use_float: true},
        13: { name: "Disable DAC (x1)", span: true , command: "GEN_DISABLE_X1"},
        14: { name: "DAC Offset Second Stage (x1)",img: "./img/250/RP_250_GEN.png", hint: "Please connect OUT1 to IN1 and OUT2 to IN2." , command:"GEN_LV_CALIB_OFFSET_S_STAGE_X1", show_adc : true},
        15: { name: "Enable DAC (x1)", span: true , command: "GEN_LV_0_5_X1"},
        16: { name: "DAC Gain Third Stage (x1)", img: "./img/250/RP_250_GEN.png", hint: "Please connect OUT1 to IN1 and OUT2 to IN2.." , command:"GEN_LV_CALIB_GAIN_S_STAGE_X1", show_adc : true, use_float: true},

        17: { name: "Enable DAC (x5)", span: true , command: "GEN_HV_0_15_X5"},
        18: { name: "DAC Gain First Stage (x5)", img: "./img/250/RP_250_GEN.png", hint: "Please connect OUT1 to IN1 and OUT2 to IN2." , command:"GEN_CALIB_GAIN_F_STAGE_X5", show_adc : true, use_float: true},
        19: { name: "Disable DAC (x5)", span: true , command: "GEN_DISABLE_X5"},
        20: { name: "DAC Offset First Stage (x5)",img: "./img/250/RP_250_GEN.png", hint: "Please connect OUT1 to IN1 and OUT2 to IN2." , command:"GEN_CALIB_OFFSET_F_STAGE_X5", show_adc : true},
        21: { name: "Enable DAC (x5)", span: true , command: "GEN_HV_0_15_X5"},
        22: { name: "DAC Gain Second Stage (x5)", img: "./img/250/RP_250_GEN.png", hint: "Please connect OUT1 to IN1 and OUT2 to IN2." , command:"GEN_CALIB_GAIN_S_STAGE_X5", show_adc : true, use_float: true},
        23: { name: "Disable DAC (x5)", span: true , command: "GEN_DISABLE_X5"},
        24: { name: "DAC Offset Second Stage (x5)",img: "./img/250/RP_250_GEN.png", hint: "Please connect OUT1 to IN1 and OUT2 to IN2." , command:"GEN_CALIB_OFFSET_S_STAGE_X5", show_adc : true},
        25: { name: "Enable DAC (x5)", span: true , command: "GEN_HV_0_15_X5"},
        26: { name: "DAC Gain Third Stage (x5)", img: "./img/250/RP_250_GEN.png", hint: "Please connect OUT1 to IN1 and OUT2 to IN2." , command:"GEN_CALIB_GAIN_S_STAGE_X5", show_adc : true, use_float: true},

        27: { name: "ADC set in LV/AC", span: true, command:"INIT_ADC_LV_AC" },
        28: { name: "ADC offset (1:1)/AC", img: "./img/250/RP_250_GND.png", hint: "Please connect IN1 and IN2 to GND.",  command:"CALIB_ADC_LV_AC_OFFSET", show_adc : true},
        29: { name: "Enable DAC", span: true, command:"GEN_LV_0_5_SINE_X1"  },
        30: { name: "ADC gain   (1:1)/AC", img: "./img/250/RP_250_GEN.png", hint: "Please connect OUT1 to IN1 and OUT2 to IN2.", use_max: true, command:"CALIB_ADC_LV_AC_GAIN" , show_adc : true, use_float: true  },

        31: { name: "ADC set in HV/AC", span: true, command:"INIT_ADC_HV_AC" },
        32: { name: "ADC offset (1:20)/AC", img: "./img/250/RP_250_GND.png", hint: "Please connect IN1 and IN2 to GND.", command:"CALIB_ADC_HV_AC_OFFSET", show_adc : true},
        33: { name: "Enable DAC", span: true, command:"GEN_LV_0_9_SINE_X1"  },
        34: { name: "ADC gain   (1:20)/AC", img: "./img/250/RP_250_GEN.png", hint: "Please connect OUT1 to IN1 and OUT2 to IN2.", use_max: true, command:"CALIB_ADC_HV_AC_GAIN" , show_adc : true, use_float: true  },
        35: { name: "Calibration complete", span: true, end: true }


    };

    OBJ.amModel = undefined;
    OBJ.amStates = undefined;
    OBJ.amCurrentTest = undefined;
    OBJ.amCurrentWaitTest = undefined;
    OBJ.amCurrentRowID = undefined;
    OBJ.amFirstColumnUpdate = undefined;
    OBJ.amSecondColumnUpdate = undefined;
    OBJ.amLastAVGCH1 = 0;
    OBJ.amLastAVGCH2 = 0;
    OBJ.amLastAVGCH3 = 0;
    OBJ.amLastAVGCH4 = 0;
    OBJ.amLastState = false;

    OBJ.amSetModel = function(_model) {
        if (OBJ.amModel === undefined) {
            OBJ.amModel = _model.value;
            if (OBJ.amModel === "Z10") OBJ.amStates = OBJ.STATES_125_14;
            if (OBJ.amModel === "Z20_125") OBJ.amStates = OBJ.STATES_125_14;
            if (OBJ.amModel === "Z20_125_LL") OBJ.amStates = OBJ.STATES_125_14;
            if (OBJ.amModel === "Z20_65_LL") OBJ.amStates = OBJ.STATES_125_14;
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
                OBJ.amFirstColumnUpdate = false;
                OBJ.amRemoveStateObject();
                OBJ.amSetWait();
                OBJ.amSendState(OBJ.amStates[OBJ.amCurrentTest].command, 0);
            } else {
                OBJ.amShowDloag();
            }
        }
    }

    OBJ.amAddNextStepRow = function() {
        if (OBJ.amStates[OBJ.amCurrentTest] !== undefined) {
            OBJ.amFirstColumnUpdate = false;
            OBJ.amSecondColumnUpdate = false;
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("span")) {
                OBJ.amCurrentRowID = OBJ.amAddNewRowSpan();
            } else {
                OBJ.amCurrentRowID = OBJ.amAddNewRow();
            }
            OBJ.amSetName();
            OBJ.amSetStartButton();
            OBJ.amFirstColumnUpdate = true;
            OBJ.amSecondColumnUpdate = false;
        }
    }

    OBJ.amGetStatus = function(_state) {
        if (_state.value === "") return;
        if (OBJ.amCurrentWaitTest == _state.value) {
            OBJ.amCurrentWaitTest = "";
            OBJ.amFirstColumnUpdate = false;
            OBJ.amSecondColumnUpdate = true;
            console.log(_state);
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
        OBJ.amFirstColumnUpdate = false;
        OBJ.amRemoveStateObject();
        OBJ.amSetWait();
        var ref = 0;
        if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("input")) {
            ref = $("#SS_REF_VOLT").val();
        }
        OBJ.amSendState(OBJ.amStates[OBJ.amCurrentTest].command, ref);
        $("#am_dialog_calib").modal('hide');
    }

    OBJ.amStartCalibration = function() {
        OBJ.amCurrentSuccesTest = "-1"
        OBJ.amCurrentTest = 0;
        OBJ.amFirstColumnUpdate = undefined;
        OBJ.amSecondColumnUpdate = undefined;
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
        OBJ.amCurrentWaitTest = _state;
        CLIENT.parametersCache["SS_NEXTSTEP"] = { value: _state };
        CLIENT.parametersCache["ref_volt"] = { value: _ref_volt };
        CLIENT.sendParameters();
    }

    OBJ.amCheckInputRef = function() {
        var ref = parseFloat($("#SS_REF_VOLT").val());
        var state = true;
        if (isNaN(ref) === false) {
            if ((ref * 0.4 > OBJ.amLastAVGCH1) || (OBJ.amLastAVGCH1 > ref * 1.6)) {
                state = false;
            }
            if ((ref * 0.4 > OBJ.amLastAVGCH2) || (OBJ.amLastAVGCH2 > ref * 1.6)) {
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
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("show_adc") && !OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_max")) {
                _value = _value.toFixed(4);
                var element_b = document.getElementById(OBJ.amCurrentRowID + "_ch1_befor");
                var element_a = document.getElementById(OBJ.amCurrentRowID + "_ch1_after");
                if (element_b != undefined && OBJ.amFirstColumnUpdate == true) 
                    element_b.innerText = _value + " V";
                if (element_a != undefined && OBJ.amSecondColumnUpdate == true) 
                    element_a.innerText = _value + " V";
            }
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("input")) {
                OBJ.amCheckInputRef();
            }
        }
    }

    OBJ.amSetCH2Avg = function(_value) {
        if (OBJ.amCheckEmptyVariables()) {
            OBJ.amLastAVGCH2 = _value;
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("show_adc") && !OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_max")) {
                _value = _value.toFixed(4);
                var element_b = document.getElementById(OBJ.amCurrentRowID + "_ch2_befor");
                var element_a = document.getElementById(OBJ.amCurrentRowID + "_ch2_after");
                if (element_b != undefined && OBJ.amFirstColumnUpdate == true) 
                    element_b.innerText = _value + " V";
                if (element_a != undefined && OBJ.amSecondColumnUpdate == true) 
                    element_a.innerText = _value + " V";
            }
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("input")) {
                OBJ.amCheckInputRef();
            }
        }
    }

    OBJ.amSetCH3Avg = function(_value) {
        if (OBJ.amCheckEmptyVariables()) {
            OBJ.amLastAVGCH3 = _value;
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("show_adc") && !OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_max")) {
                _value = _value.toFixed(4);
                var element_b = document.getElementById(OBJ.amCurrentRowID + "_ch3_befor");
                var element_a = document.getElementById(OBJ.amCurrentRowID + "_ch3_after");
                if (element_b != undefined && OBJ.amFirstColumnUpdate == true) 
                    element_b.innerText = _value + " V";
                if (element_a != undefined && OBJ.amSecondColumnUpdate == true) 
                    element_a.innerText = _value + " V";
            }
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("input")) {
                OBJ.amCheckInputRef();
            }
        }
    }

    OBJ.amSetCH4Avg = function(_value) {
        if (OBJ.amCheckEmptyVariables()) {
            OBJ.amLastAVGCH4 = _value;
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("show_adc") && !OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_max")) {
                _value = _value.toFixed(4);
                var element_b = document.getElementById(OBJ.amCurrentRowID + "_ch4_befor");
                var element_a = document.getElementById(OBJ.amCurrentRowID + "_ch4_after");
                if (element_b != undefined && OBJ.amFirstColumnUpdate == true) 
                    element_b.innerText = _value + " V";
                if (element_a != undefined && OBJ.amSecondColumnUpdate == true) 
                    element_a.innerText = _value + " V";
            }
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("input")) {
                OBJ.amCheckInputRef();
            }
        }
    }

    OBJ.amSetCH1Max = function(_value) {
        if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("show_adc") && OBJ.amCheckEmptyVariables()) {
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_max")) {
                _value = _value.toFixed(4);
                var element_b = document.getElementById(OBJ.amCurrentRowID + "_ch1_befor");
                var element_a = document.getElementById(OBJ.amCurrentRowID + "_ch1_after");
                if (element_b != undefined && OBJ.amFirstColumnUpdate == true) 
                    element_b.innerText = _value + " V";
                if (element_a != undefined && OBJ.amSecondColumnUpdate == true) 
                    element_a.innerText = _value + " V";
            }
        }
    }

    OBJ.amSetCH2Max = function(_value) {
        if (OBJ.amCheckEmptyVariables()) {
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("show_adc") && OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_max")) {
                _value = _value.toFixed(4);
                var element_b = document.getElementById(OBJ.amCurrentRowID + "_ch2_befor");
                var element_a = document.getElementById(OBJ.amCurrentRowID + "_ch2_after");
                if (element_b != undefined && OBJ.amFirstColumnUpdate == true) 
                    element_b.innerText = _value + " V";
                if (element_a != undefined && OBJ.amSecondColumnUpdate == true) 
                    element_a.innerText = _value + " V";
            }
        }
    }

    OBJ.amSetCH3Max = function(_value) {
        if (OBJ.amCheckEmptyVariables()) {
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("show_adc") && OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_max")) {
                _value = _value.toFixed(4);
                var element_b = document.getElementById(OBJ.amCurrentRowID + "_ch3_befor");
                var element_a = document.getElementById(OBJ.amCurrentRowID + "_ch3_after");
                if (element_b != undefined && OBJ.amFirstColumnUpdate == true) 
                    element_b.innerText = _value + " V";
                if (element_a != undefined && OBJ.amSecondColumnUpdate == true) 
                    element_a.innerText = _value + " V";
            }
        }
    }

    OBJ.amSetCH4Max = function(_value) {
        if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("show_adc") && OBJ.amCheckEmptyVariables()) {
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_max")) {
                _value = _value.toFixed(4);
                var element_b = document.getElementById(OBJ.amCurrentRowID + "_ch4_befor");
                var element_a = document.getElementById(OBJ.amCurrentRowID + "_ch4_after");
                if (element_b != undefined && OBJ.amFirstColumnUpdate == true) 
                    element_b.innerText = _value + " V";
                if (element_a != undefined && OBJ.amSecondColumnUpdate == true) 
                    element_a.innerText = _value + " V";
            }
        }
    }

    OBJ.amSetCalibValueCh1 = function(_value) {
        if (OBJ.amCheckEmptyVariables() && !OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_float")) {
            var element_b = document.getElementById(OBJ.amCurrentRowID + "_value_ch1");
            if (element_b != undefined) element_b.innerText = _value.value;
        }
    }

    OBJ.amSetCalibValueCh2 = function(_value) {
        if (OBJ.amCheckEmptyVariables() && !OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_float")) {
            var element_b = document.getElementById(OBJ.amCurrentRowID + "_value_ch2");
            if (element_b != undefined) element_b.innerText = _value.value;
        }
    }

    OBJ.amSetCalibValueCh3 = function(_value) {
        if (OBJ.amCheckEmptyVariables() && !OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_float")) {
            var element_b = document.getElementById(OBJ.amCurrentRowID + "_value_ch3");
            if (element_b != undefined) element_b.innerText = _value.value;
        }
    }

    OBJ.amSetCalibValueCh4 = function(_value) {
        if (OBJ.amCheckEmptyVariables() && !OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_float")) {
            var element_b = document.getElementById(OBJ.amCurrentRowID + "_value_ch4");
            if (element_b != undefined) element_b.innerText = _value.value;
        }
    }

    OBJ.amSetCalibValueFCh1 = function(_value) {
        if (OBJ.amCheckEmptyVariables() && OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_float")) {
            var element_b = document.getElementById(OBJ.amCurrentRowID + "_value_ch1");
            if (element_b != undefined) element_b.innerText = _value.value.toFixed(4);
        }
    }

    OBJ.amSetCalibValueFCh2 = function(_value) {
        if (OBJ.amCheckEmptyVariables() && OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_float")) {
            var element_b = document.getElementById(OBJ.amCurrentRowID + "_value_ch2");
            if (element_b != undefined) element_b.innerText = _value.value.toFixed(4);
        }
    }

    OBJ.amSetCalibValueFCh3 = function(_value) {
        if (OBJ.amCheckEmptyVariables() && OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_float")) {
            var element_b = document.getElementById(OBJ.amCurrentRowID + "_value_ch3");
            if (element_b != undefined) element_b.innerText = _value.value.toFixed(4);
        }
    }

    OBJ.amSetCalibValueFCh4 = function(_value) {
        if (OBJ.amCheckEmptyVariables() && OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("use_float")) {
            var element_b = document.getElementById(OBJ.amCurrentRowID + "_value_ch4");
            if (element_b != undefined) element_b.innerText = _value.value.toFixed(4);
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
    SM.param_callbacks["ch1_calib_pass_f"] = OBJ.amSetCalibValueFCh1;
    SM.param_callbacks["ch2_calib_pass_f"] = OBJ.amSetCalibValueFCh2;
    SM.param_callbacks["ch3_calib_pass_f"] = OBJ.amSetCalibValueFCh3;
    SM.param_callbacks["ch4_calib_pass_f"] = OBJ.amSetCalibValueFCh4;

});