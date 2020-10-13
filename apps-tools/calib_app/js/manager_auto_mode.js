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
        0    : {name: "Reset to default", span: true}, 
        1    : {name: "ADC offset (1:1)",img: "./img/125/RP_125_GND.png", hint:"Please set LV mode and connect IN1 and IN2 to GND."}, 
        2    : {name: "ADC gain   (1:1)"}, 
        3    : {name: "ADC offset (1:20)"}, 
        4    : {name: "ADC gain   (1:20)"}, 
        5    : {name: "DAC offset"}, 
        6    : {name: "DAC gain"},
        7    : {name: "Calibration complete", span: true}   
    };

    OBJ.STATES_250_12 = {
        0    : {name: "Reset to default", span: true},
        1    : {name: "ADC offset DC (1:1)"}, 
        2    : {name: "ADC gain DC   (1:1)"}, 
        3    : {name: "ADC offset DC (1:20)"}, 
        4    : {name: "ADC gain DC   (1:20)"}, 
        5    : {name: "DAC offset"}, 
        6    : {name: "DAC gain"},
        7    : {name: "DAC offset"}, 
        8    : {name: "DAC gain"},
        9    : {name: "ADC offset AC (1:1)"}, 
        10   : {name: "ADC gain AC   (1:1)"}, 
        11   : {name: "ADC offset AC (1:20)"}, 
        12   : {name: "ADC gain AC   (1:20)"},
        13   : {name: "Calibration complete", span: true}       
    };

    OBJ.amModel = undefined;
    OBJ.amStates = undefined;
    OBJ.amCurrentTest = undefined;
    OBJ.amCurrentSuccesTest = undefined;
    OBJ.amCurrentRowID = undefined;
    
    OBJ.amSetModel = function(_model) {
        if (OBJ.amModel === undefined){
            OBJ.amModel = _model.value;
            if (OBJ.amModel === "Z10")        OBJ.amStates = OBJ.STATES_125_14;
            if (OBJ.amModel === "Z20_250_12") OBJ.amStates = OBJ.STATES_250_12;           
        }
    }

    OBJ.amClearTable = function() {
        $("#auto_calib_table").empty();
    }

    OBJ.amAddNewRow = function() {
        var table = document.getElementById("auto_calib_table");
        var row = table.insertRow(-1);
        var id = OBJ.makeid(8);
        row.setAttribute("id", id);
        let newCell = row.insertCell(-1);
        newCell.setAttribute("id",id+"_mode");
        newCell = row.insertCell(-1);
        newCell.setAttribute("id",id+"_ch1_befor");
        newCell = row.insertCell(-1);
        newCell.setAttribute("id",id+"_ch1_after");
        newCell = row.insertCell(-1);
        newCell.setAttribute("id",id+"_ch2_befor");
        newCell = row.insertCell(-1);
        newCell.setAttribute("id",id+"_ch2_after");
        newCell = row.insertCell(-1);
        newCell.setAttribute("id",id+"_value_ch1");
        newCell = row.insertCell(-1);
        newCell.setAttribute("id",id+"_value_ch2");
        newCell = row.insertCell(-1);
        newCell.setAttribute("id",id+"_state");
        return id;
    }

    OBJ.amAddNewRowSpan = function() {
        var table = document.getElementById("auto_calib_table");
        var row = table.insertRow(-1);
        var id = OBJ.makeid(8);
        row.setAttribute("id", id);
        let newCell = row.insertCell(-1);
        newCell.setAttribute("id",id+"_mode");
        newCell.setAttribute("colspan","7");
        newCell = row.insertCell(-1);
        newCell.setAttribute("id",id+"_state");
        return id;
    }

    OBJ.amCheckEmptyVariables = function() {
        return (OBJ.amCurrentTest != undefined && OBJ.amCurrentRowID != undefined && OBJ.amStates != undefined);
    }
    
    OBJ.amSetRowName = function(_id,_name){
        $("#"+_id+"_mode").text(_name);
    }

    OBJ.amSetName = function(){
        if (OBJ.amCheckEmptyVariables()){
            OBJ.amSetRowName(OBJ.amCurrentRowID,OBJ.amStates[OBJ.amCurrentTest].name);
        }
    }

    OBJ.amSetStartButton = function(){
        if (OBJ.amCheckEmptyVariables()){
            var element = document.getElementById(OBJ.amCurrentRowID + "_state");
            var d = document.createElement("div");
            element.appendChild(d);
            d.setAttribute("class","am_start_buttons");
            var l = document.createElement("li");
            d.appendChild(l);
            var a = document.createElement("a");
            l.appendChild(a);
            a.innerHTML = "START";
            a.setAttribute("href","#");
            l.onclick = OBJ.amStartButtonPress;
        }
    }

    OBJ.amRemoveStateObject = function(){
        if (OBJ.amCheckEmptyVariables()){
            OBJ.amRemoveStateObjectId(OBJ.amCurrentRowID);
        }
    }

    OBJ.amRemoveStateObjectId = function(_id){
        var element = document.getElementById(_id + "_state");
        element.innerHTML = '';
    }

    OBJ.amSetWait = function(){
        if (OBJ.amCheckEmptyVariables()){
            var element = document.getElementById(OBJ.amCurrentRowID + "_state");
            var d2 = document.createElement("div");
            element.appendChild(d2);
            d2.setAttribute("class","lds-ellipsis");
            d2.appendChild(document.createElement("div"));
            d2.appendChild(document.createElement("div"));
            d2.appendChild(document.createElement("div"));
            d2.appendChild(document.createElement("div"));
        }
    }

    OBJ.amSetOkState = function(){
        if (OBJ.amCheckEmptyVariables()){
            OBJ.amSetOkStateId(OBJ.amCurrentRowID);
        }
    }

    OBJ.amSetOkStateId = function(_id){
        var element = document.getElementById(_id + "_state");
        var elem = document.createElement("img");
        elem.setAttribute("src", "./img/ok.png");
        elem.setAttribute("height", "16");
        elem.setAttribute("width", "16");
        element.appendChild(elem);
    }

    OBJ.amStartButtonPress = function(){
        if (OBJ.amStates[OBJ.amCurrentTest] !== undefined){
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("span")) {
                OBJ.amRemoveStateObject();
                OBJ.amSetWait();
                OBJ.amSendState(OBJ.amCurrentTest);
            }else{
                OBJ.amShowDloag();
            }
        }
    }

    OBJ.amAddNextStepRow = function(){
        if (OBJ.amStates[OBJ.amCurrentTest] !== undefined){
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("span")) {
                OBJ.amCurrentRowID = OBJ.amAddNewRowSpan();
            }else{
                OBJ.amCurrentRowID = OBJ.amAddNewRow();
            }
            OBJ.amSetName();
            OBJ.amSetStartButton();
        }
    }

    OBJ.amGetStatus = function(_state) {
        if (_state.value === -1) return;
        if (OBJ.amCurrentSuccesTest != _state.value){
            console.log(_state);
            OBJ.amCurrentSuccesTest = _state.value;
            setTimeout(function(){
                OBJ.amRemoveStateObject();
                OBJ.amSetOkState();
                OBJ.amContinueCalibration();
            }, 1000);
        }
    }

    OBJ.amShowDloag = function(){
        if (OBJ.amCheckEmptyVariables()){
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("img")) {
                $("#am_dialog_img").attr("src",OBJ.amStates[OBJ.amCurrentTest].img);
            }else{
                $("#am_dialog_img").attr("src","");
            }
            if (OBJ.amStates[OBJ.amCurrentTest].hasOwnProperty("hint")) {
                $("#am_dialog_text").text(OBJ.amStates[OBJ.amCurrentTest].hint);
            }else{
                $("#am_dialog_text").text("");
            }
        }
        $("#am_dialog_calib").modal('show');
    }

    OBJ.amClickOkDialog = function(){
        OBJ.amRemoveStateObject();
        OBJ.amSetWait();
        OBJ.amSendState(OBJ.amCurrentTest);
    }

    OBJ.amStartCalibration = function(){
        OBJ.amCurrentTest = 0;
        OBJ.amCurrentRowID = OBJ.amAddNewRowSpan();
        OBJ.amSetName();
        OBJ.amSetStartButton();
        // OBJ.amCurrentTest = 1;
        // OBJ.amCurrentRowID = OBJ.amAddNewRow();
        // OBJ.amSetName();
        // OBJ.amSetStartButton();
    }

    OBJ.amContinueCalibration = function(){
        OBJ.amCurrentTest++;
        OBJ.amAddNextStepRow();               
    }

    OBJ.amSendState = function(_state){
        SM.parametersCache["SS_NEXTSTEP"] = { value: _state };
        SM.sendParameters();
    }

    OBJ.amSetCH1Avg = function(_value){
        if (OBJ.amCheckEmptyVariables()){
            var element_b = document.getElementById(OBJ.amCurrentRowID + "_ch1_befor");
            var element_a = document.getElementById(OBJ.amCurrentRowID + "_ch1_after");
            if (OBJ.amCurrentTest !== OBJ.amCurrentSuccesTest){
                if (element_b != undefined) element_b.innerText = _value + " V";
            }else{
                if (element_a != undefined) element_a.innerText = _value + " V";
            }
        }
    }

    OBJ.amSetCH2Avg = function(_value){
        if (OBJ.amCheckEmptyVariables()){
            var element_b = document.getElementById(OBJ.amCurrentRowID + "_ch2_befor");
            var element_a = document.getElementById(OBJ.amCurrentRowID + "_ch2_after");
            if (OBJ.amCurrentTest !== OBJ.amCurrentSuccesTest){
                if (element_b != undefined) element_b.innerText = _value + " V";
            }else{
                if (element_a != undefined) element_a.innerText = _value + " V";
            }
        }
    }

}(window.OBJ = window.OBJ || {}, jQuery));


// Page onload event handler
$(function() {
    SM.param_callbacks["RP_MODEL_STR"] = OBJ.amSetModel;
    SM.param_callbacks["SS_STATE"] = OBJ.amGetStatus;     
    $('#am_ok_btn').on('click', function() { OBJ.amClickOkDialog() });
});