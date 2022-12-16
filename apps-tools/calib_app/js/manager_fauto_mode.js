/*
 * Red Pitaya calib_app
 *
 * Author: Danilyuk Nikolay <n.danilyuk@integrasources.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


(function(OBJ, $, undefined) {


    OBJ.F_STATES_125_14 = {
        0: { name: "Prepare", span: true },
        1: { name: "LV mode", input: 0.9, low: true },
        2: { name: "Prepare", span: true },
        3: { name: "HV mode", input: 9 },
        4: { name: "Save calibration values", span: true },
        5: { name: "Calibration complete", span: true, end: true }
    };


    OBJ.famModel = undefined;
    OBJ.famStates = undefined;
    OBJ.famCurrentTest = undefined;
    OBJ.famCurrentSuccesTest = undefined;
    OBJ.famCurrentRowID = undefined;
    OBJ.famLastAVGCH1 = 0;
    OBJ.famLastAVGCH2 = 0;
    OBJ.famLastState = false;

    OBJ.famSetModel = function(_model) {
        if (OBJ.famModel === undefined) {
            OBJ.famModel = _model.value;
            if (OBJ.famModel === "Z10" || OBJ.famModel === "Z20_125" || OBJ.famModel === "Z20_125_4CH") OBJ.famStates = OBJ.F_STATES_125_14;

            $('#am_a_filt_external_btn').on('click', function() { OBJ.famClickOkDialog() });

            $('.a_filter_flipswitch').change(function() {
                $(this).next().text($(this).is(':checked') ? ':checked' : ':not(:checked)');
                OBJ.aFilterSetMode($(this).attr('id'), $(this).is(':checked'));

            }).trigger('change');
        }
    }

    OBJ.famClearTable = function() {
        $("#fauto_calib_table").empty();
    }

    OBJ.famAddNewRow = function(row_id,ch) {
        var table = document.getElementById("fauto_calib_table");
        var row = table.insertRow(-1);

        row.setAttribute("id", row_id);
        let newCell = row.insertCell(-1);
        newCell.setAttribute("class", row_id + "_mode");
        newCell = row.insertCell(-1);
        newCell.setAttribute("class", row_id + "_ch" + ch + "_name");
        newCell.innerHTML = "CH "+ ch;
        newCell = row.insertCell(-1);
        newCell.setAttribute("id", row_id + "_ch" + ch + "_befor");
        newCell = row.insertCell(-1);
        newCell.setAttribute("id", row_id + "_ch" + ch + "_after");
        newCell = row.insertCell(-1);
        newCell.setAttribute("id", row_id + "_aa_ch" + ch);
        newCell = row.insertCell(-1);
        newCell.setAttribute("id", row_id + "_bb_ch" + ch );
        newCell = row.insertCell(-1);
        newCell.setAttribute("id", row_id + "_pp_ch" + ch);
        newCell = row.insertCell(-1);
        newCell.setAttribute("id", row_id + "_kk_ch" + ch);
        newCell = row.insertCell(-1);
        newCell.setAttribute("class", row_id + "_state");
        return row_id;
    }

    OBJ.famAddNewRowSpan = function() {
        var table = document.getElementById("fauto_calib_table");
        var row = table.insertRow(-1);
        var id = OBJ.makeid(8);
        row.setAttribute("class", id);
        let newCell = row.insertCell(-1);
        newCell.setAttribute("class", id + "_mode");
        newCell.setAttribute("colspan", "8");
        newCell = row.insertCell(-1);
        newCell.setAttribute("class", id + "_state");
        return id;
    }

    OBJ.famCheckEmptyVariables = function() {
        return (OBJ.famCurrentTest != undefined && OBJ.famCurrentRowID != undefined && OBJ.famStates != undefined);
    }

    OBJ.famSetRowName = function(_id, _name) {
        $("." + _id + "_mode").text(_name);
    }


    OBJ.famSetName = function() {
        if (OBJ.famCheckEmptyVariables()) {
            OBJ.famSetRowName(OBJ.famCurrentRowID, OBJ.famStates[OBJ.famCurrentTest].name);
        }
    }

    OBJ.famSetNameEx = function(channels) {
        if (OBJ.famCheckEmptyVariables()) {
            for(var i = 1; i<= channels ;i++)
                OBJ.famSetRowName(OBJ.famCurrentRowID, OBJ.famStates[OBJ.famCurrentTest].name);
        }
    }

    OBJ.famSetStartButton = function() {
        if (OBJ.famCheckEmptyVariables()) {
            var arr = document.getElementsByClassName(OBJ.famCurrentRowID + "_state");
            var element = arr[arr.length -1];
            var d = document.createElement("div");
            element.appendChild(d);
            d.setAttribute("class", "am_start_buttons");
            var l = document.createElement("li");
            d.appendChild(l);
            var a = document.createElement("a");
            l.appendChild(a);
            a.setAttribute("href", "#");
            if (!OBJ.famStates[OBJ.famCurrentTest].hasOwnProperty("end")) {
                a.innerHTML = "START";
                l.onclick = OBJ.famStartButtonPress;
            } else {
                a.innerHTML = "DONE";
                l.onclick = OBJ.showMainMenu;
            }
        }
    }


    OBJ.famRemoveStateObject = function() {
        if (OBJ.famCheckEmptyVariables()) {
            OBJ.famRemoveStateObjectId(OBJ.famCurrentRowID);
        }
    }

    OBJ.famRemoveStateObjectId = function(_id) {
        var arr = document.getElementsByClassName(_id + "_state");
        Array.from(arr).forEach(element => {
            element.innerHTML = '';
        });
    }

    OBJ.famSetWait = function() {
        if (OBJ.famCheckEmptyVariables()) {
            var arr = document.getElementsByClassName(OBJ.famCurrentRowID + "_state")
            Array.from(arr).forEach(element => {
                var d2 = document.createElement("div");
                element.appendChild(d2);
                d2.setAttribute("class", "lds-ellipsis");
                d2.appendChild(document.createElement("div"));
                d2.appendChild(document.createElement("div"));
                d2.appendChild(document.createElement("div"));
                d2.appendChild(document.createElement("div"));
            });
        }
    }

    OBJ.famSetOkState = function() {
        if (OBJ.famCheckEmptyVariables()) {
            OBJ.famSetOkStateId(OBJ.famCurrentRowID);
        }
    }

    OBJ.famSetOkStateId = function(_id) {
        var arr = document.getElementsByClassName(_id + "_state");
        Array.from(arr).forEach(element => {
            var elem = document.createElement("img");
            elem.setAttribute("src", "./img/ok.png");
            elem.setAttribute("height", "16");
            elem.setAttribute("width", "16");
            element.appendChild(elem);
        });
    }

    OBJ.famSetProgress = function() {
        if (OBJ.famCheckEmptyVariables()) {
            if ($(".FAUTO_PROGRESS").length === 0) {
                OBJ.famRemoveStateObject();
                var arr = document.getElementsByClassName(OBJ.famCurrentRowID + "_state");
                Array.from(arr).forEach(element => {
                    var elem = document.createElement("progress");
                    elem.setAttribute("class", "FAUTO_PROGRESS");
                    elem.setAttribute("max", "120");
                    elem.setAttribute("value", "0");
                    elem.setAttribute("style", "width:80%;");
                    element.appendChild(elem);
                });
            }
        }
    }

    OBJ.famStartButtonPress = function() {
        if (OBJ.famStates[OBJ.famCurrentTest] !== undefined) {
            if (OBJ.famStates[OBJ.famCurrentTest].hasOwnProperty("span")) {
                OBJ.famRemoveStateObject();
                OBJ.famSetWait();
                OBJ.famSendState(OBJ.famCurrentTest);
            } else {
                OBJ.famShowDloag();
            }
        }
    }

    OBJ.famAddNextStepRow = function() {
        if (OBJ.famStates[OBJ.famCurrentTest] !== undefined) {
            if (OBJ.famStates[OBJ.famCurrentTest].hasOwnProperty("span")) {
                OBJ.famCurrentRowID = OBJ.famAddNewRowSpan();
                OBJ.famSetName();
                OBJ.famSetStartButton("");
            } else {
                var maxChannels = 2
                if (OBJ.famModel === "Z20_125_4CH") maxChannels = 4
                var id = OBJ.makeid(8);
                for(var i = 0; i< maxChannels; i++)
                    OBJ.famCurrentRowID = OBJ.famAddNewRow(id,i+1);
                OBJ.famSetNameEx(maxChannels);
                OBJ.famSetStartButton(maxChannels);
            }

        }
    }

    OBJ.famGetStatus = function(_state) {
        if (_state.value === -1) return;
        if (OBJ.famCurrentSuccesTest != _state.value) {
            console.log(_state);
            OBJ.famCurrentSuccesTest = _state.value;
            setTimeout(function() {
                OBJ.famRemoveStateObject();
                OBJ.famSetOkState();
                OBJ.famContinueCalibration();
            }, 2000);
        }
    }

    OBJ.famShowDloag = function() {
        OBJ.famLastState = true;
        if (OBJ.famCheckEmptyVariables()) {
            if (OBJ.famStates[OBJ.famCurrentTest].hasOwnProperty("low")) {
                if ($("#am_a_filt_switch").is(':checked')) {
                    $("#am_a_filt_dialog_img").attr("src", "./img/125/RP_125_GEN_AUTO_MODE.png");
                    $("#am_a_filt_dialog_text").text("Set jumpers to LV position and connect OUT1 to IN1 and IN2.");
                    $("#am_a_filt_dialog_input").hide();
                } else {
                    $("#am_a_filt_dialog_img").attr("src", "./img/125/RP_125_REF_FILTER.png");
                    $("#am_a_filt_dialog_text").text("Set jumpers to LV position and connect IN1 and IN2 to external signal generator 1kHz square signal.");
                    if (OBJ.famStates[OBJ.famCurrentTest].hasOwnProperty("input")) {
                        $("#SS_A_FILT_REF_VOLT").val(OBJ.famStates[OBJ.famCurrentTest].input);
                        SM.parametersCache["f_ref_volt"] = { value: OBJ.famStates[OBJ.famCurrentTest].input };
                        SM.sendParameters2("f_ref_volt");
                    }
                    $("#am_a_filt_dialog_input").show();
                }
            } else {
                if ($("#am_a_filt_switch").is(':checked')) {
                    $("#am_a_filt_dialog_img").attr("src", "./img/125/RP_125_GEN_HV_AUTO_MODE.png");
                    $("#am_a_filt_dialog_text").text("Set jumpers to HV position and connect OUT1 to IN1 and IN2.");
                    $("#am_a_filt_dialog_input").hide();
                } else {
                    $("#am_a_filt_dialog_img").attr("src", "./img/125/RP_125_REF_HV_FILTER.png");
                    $("#am_a_filt_dialog_text").text("Set jumpers to HV position and connect IN1 and IN2 to external signal generator 1kHz square signal.");
                    if (OBJ.famStates[OBJ.famCurrentTest].hasOwnProperty("input")) {
                        $("#SS_A_FILT_REF_VOLT").val(OBJ.famStates[OBJ.famCurrentTest].input);
                        SM.parametersCache["f_ref_volt"] = { value: OBJ.famStates[OBJ.famCurrentTest].input };
                        SM.sendParameters2("f_ref_volt");
                    }
                    $("#am_a_filt_dialog_input").show();
                }
            }


            $('#am_a_filt_cancel_btn').off('click');
            $('#am_a_filt_cancel_btn').on('click', function() {});
            $("#am_dialog_a_filter_calib").modal('show');
        }
    }

    OBJ.famClickOkDialog = function() {
        if (OBJ.famLastState === false) return;
        OBJ.famRemoveStateObject();
        OBJ.famSetWait();
        OBJ.famSendState(OBJ.famCurrentTest);
        $("#am_dialog_a_filter_calib").modal('hide');
    }

    OBJ.famStartCalibration = function() {
        OBJ.famCurrentTest = 0;
        OBJ.famAddNextStepRow();
    }

    OBJ.famContinueCalibration = function() {
        OBJ.famCurrentTest++;
        OBJ.famAddNextStepRow();
    }

    OBJ.famSendState = function(_state) {
        SM.parametersCache["F_SS_NEXTSTEP"] = { value: _state };
        SM.parametersCache["F_SS_STATE"] = { value: _state - 1 };
        OBJ.famCurrentSuccesTest = _state - 1;
        SM.sendParameters();
    }

    OBJ.famSetCalibAACh1 = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            if (_value.value === 0) return;
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_aa_ch1");
            if (element_b != undefined) element_b.innerText = "0x" + _value.value.toString(16);
        }
    }

    OBJ.famSetCalibAACh2 = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            if (_value.value === 0) return;
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_aa_ch2");
            if (element_b != undefined) element_b.innerText = "0x" + _value.value.toString(16);
        }
    }

    OBJ.famSetCalibAACh3 = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            if (_value.value === 0) return;
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_aa_ch3");
            if (element_b != undefined) element_b.innerText = "0x" + _value.value.toString(16);
        }
    }

    OBJ.famSetCalibAACh4 = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            if (_value.value === 0) return;
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_aa_ch4");
            if (element_b != undefined) element_b.innerText = "0x" + _value.value.toString(16);
        }
    }

    OBJ.famSetCalibBBCh1 = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            if (_value.value === 0) return;
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_bb_ch1");
            if (element_b != undefined) element_b.innerText = "0x" + _value.value.toString(16);
        }
    }

    OBJ.famSetCalibBBCh2 = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            if (_value.value === 0) return;
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_bb_ch2");
            if (element_b != undefined) element_b.innerText = "0x" + _value.value.toString(16);
        }
    }

    OBJ.famSetCalibBBCh3 = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            if (_value.value === 0) return;
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_bb_ch3");
            if (element_b != undefined) element_b.innerText = "0x" + _value.value.toString(16);
        }
    }

    OBJ.famSetCalibBBCh4 = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            if (_value.value === 0) return;
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_bb_ch4");
            if (element_b != undefined) element_b.innerText = "0x" + _value.value.toString(16);
        }
    }

    OBJ.famSetCalibPPCh1 = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            if (_value.value === 0) return;
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_pp_ch1");
            if (element_b != undefined) element_b.innerText = "0x" + _value.value.toString(16);
        }
    }

    OBJ.famSetCalibPPCh2 = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            if (_value.value === 0) return;
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_pp_ch2");
            if (element_b != undefined) element_b.innerText = "0x" + _value.value.toString(16);
        }
    }

    OBJ.famSetCalibPPCh3 = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            if (_value.value === 0) return;
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_pp_ch3");
            if (element_b != undefined) element_b.innerText = "0x" + _value.value.toString(16);
        }
    }

    OBJ.famSetCalibPPCh4 = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            if (_value.value === 0) return;
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_pp_ch4");
            if (element_b != undefined) element_b.innerText = "0x" + _value.value.toString(16);
        }
    }

    OBJ.famSetCalibKKCh1 = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            if (_value.value === 0) return;
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_kk_ch1");
            if (element_b != undefined) element_b.innerText = "0x" + _value.value.toString(16);
        }
    }

    OBJ.famSetCalibKKCh2 = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            if (_value.value === 0) return;
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_kk_ch2");
            if (element_b != undefined) element_b.innerText = "0x" + _value.value.toString(16);
        }
    }

    OBJ.famSetCalibKKCh3 = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            if (_value.value === 0) return;
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_kk_ch3");
            if (element_b != undefined) element_b.innerText = "0x" + _value.value.toString(16);
        }
    }

    OBJ.famSetCalibKKCh4 = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            if (_value.value === 0) return;
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_kk_ch4");
            if (element_b != undefined) element_b.innerText = "0x" + _value.value.toString(16);
        }
    }



    OBJ.famSetValueCh1Before = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_ch1_befor");
            if (element_b != undefined) element_b.innerText = _value.value;
        }
    }

    OBJ.famSetValueCh2Before = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_ch2_befor");
            if (element_b != undefined) element_b.innerText = _value.value;
        }
    }

    OBJ.famSetValueCh3Before = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_ch3_befor");
            if (element_b != undefined) element_b.innerText = _value.value;
        }
    }

    OBJ.famSetValueCh4Before = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_ch4_befor");
            if (element_b != undefined) element_b.innerText = _value.value;
        }
    }

    OBJ.famSetValueCh1After = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_ch1_after");
            if (element_b != undefined) element_b.innerText = _value.value;
        }
    }

    OBJ.famSetValueCh2After = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_ch2_after");
            if (element_b != undefined) element_b.innerText = _value.value;
        }
    }

    OBJ.famSetValueCh3After = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_ch3_after");
            if (element_b != undefined) element_b.innerText = _value.value;
        }
    }

    OBJ.famSetValueCh4After = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            var element_b = document.getElementById(OBJ.famCurrentRowID + "_ch4_after");
            if (element_b != undefined) element_b.innerText = _value.value;
        }
    }

    OBJ.famSetProgressValue = function(_value) {
        if (OBJ.famCheckEmptyVariables()) {
            if (_value.value !== 0) {
                OBJ.famSetProgress();
                $('.FAUTO_PROGRESS').attr('value', _value.value);
            }
        }
    }


    OBJ.aFilterSetMode = function(_mode, _state) {
        if (_mode == "am_a_filt_switch") {
            if (OBJ.famCheckEmptyVariables()) {
                if (OBJ.famStates[OBJ.famCurrentTest].hasOwnProperty("low")) {
                    if (_state) {
                        $("#am_a_filt_dialog_img").attr("src", "./img/125/RP_125_GEN_AUTO_MODE.png");
                        $("#am_a_filt_dialog_text").text("Set jumpers to LV position and connect OUT1 to IN1 and IN2.");
                        $("#am_a_filt_dialog_input").hide();
                    } else {
                        $("#am_a_filt_dialog_img").attr("src", "./img/125/RP_125_REF_FILTER.png");
                        $("#am_a_filt_dialog_text").text("Set jumpers to LV position and connect IN1 and IN2 to external signal generator 1kHz square signal.");
                        if (OBJ.famStates[OBJ.famCurrentTest].hasOwnProperty("input")) {
                            $("#SS_A_FILT_REF_VOLT").val(OBJ.famStates[OBJ.famCurrentTest].input);
                        }
                        $("#am_a_filt_dialog_input").show();
                    }
                } else {
                    if (_state) {
                        $("#am_a_filt_dialog_img").attr("src", "./img/125/RP_125_GEN_HV_AUTO_MODE.png");
                        $("#am_a_filt_dialog_text").text("Set jumpers to HV position and connect OUT1 to IN1 and IN2.");
                        $("#am_a_filt_dialog_input").hide();
                    } else {
                        $("#am_a_filt_dialog_img").attr("src", "./img/125/RP_125_REF_HV_FILTER.png");
                        $("#am_a_filt_dialog_text").text("Set jumpers to HV position and connect IN1 and IN2 to external signal generator 1kHz square signal.");
                        if (OBJ.famStates[OBJ.famCurrentTest].hasOwnProperty("input")) {
                            $("#SS_A_FILT_REF_VOLT").val(OBJ.famStates[OBJ.famCurrentTest].input);
                        }
                        $("#am_a_filt_dialog_input").show();
                    }
                }
            }
            SM.parametersCache["f_external_gen"] = { value: !_state };
            SM.sendParameters2("f_external_gen");
        }
    }


}(window.OBJ = window.OBJ || {}, jQuery));



// Page onload event handler
$(function() {

    SM.param_callbacks["F_SS_STATE"] = OBJ.famGetStatus;

    SM.param_callbacks["fauto_value_ch1_before"] = OBJ.famSetValueCh1Before;
    SM.param_callbacks["fauto_value_ch2_before"] = OBJ.famSetValueCh2Before;
    SM.param_callbacks["fauto_value_ch3_before"] = OBJ.famSetValueCh3Before;
    SM.param_callbacks["fauto_value_ch4_before"] = OBJ.famSetValueCh4Before;
    SM.param_callbacks["fauto_value_ch1_after"] = OBJ.famSetValueCh1After;
    SM.param_callbacks["fauto_value_ch2_after"] = OBJ.famSetValueCh2After;
    SM.param_callbacks["fauto_value_ch3_after"] = OBJ.famSetValueCh3After;
    SM.param_callbacks["fauto_value_ch4_after"] = OBJ.famSetValueCh4After;
    SM.param_callbacks["fauto_calib_progress"] = OBJ.famSetProgressValue;


    SM.param_callbacks["fauto_aa_Ch1"] = OBJ.famSetCalibAACh1;
    SM.param_callbacks["fauto_aa_Ch2"] = OBJ.famSetCalibAACh2;
    SM.param_callbacks["fauto_aa_Ch3"] = OBJ.famSetCalibAACh3;
    SM.param_callbacks["fauto_aa_Ch4"] = OBJ.famSetCalibAACh4;
    SM.param_callbacks["fauto_bb_Ch1"] = OBJ.famSetCalibBBCh1;
    SM.param_callbacks["fauto_bb_Ch2"] = OBJ.famSetCalibBBCh2;
    SM.param_callbacks["fauto_bb_Ch3"] = OBJ.famSetCalibBBCh3;
    SM.param_callbacks["fauto_bb_Ch4"] = OBJ.famSetCalibBBCh4;
    SM.param_callbacks["fauto_pp_Ch1"] = OBJ.famSetCalibPPCh1;
    SM.param_callbacks["fauto_pp_Ch2"] = OBJ.famSetCalibPPCh2;
    SM.param_callbacks["fauto_pp_Ch3"] = OBJ.famSetCalibPPCh3;
    SM.param_callbacks["fauto_pp_Ch4"] = OBJ.famSetCalibPPCh4;
    SM.param_callbacks["fauto_kk_Ch1"] = OBJ.famSetCalibKKCh1;
    SM.param_callbacks["fauto_kk_Ch2"] = OBJ.famSetCalibKKCh2;
    SM.param_callbacks["fauto_kk_Ch3"] = OBJ.famSetCalibKKCh3;
    SM.param_callbacks["fauto_kk_Ch4"] = OBJ.famSetCalibKKCh4;


});