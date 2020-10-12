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
        0    : {name: "ADC offset (1:1)"}, 
        1    : {name: "ADC gain   (1:1)"}, 
        2    : {name: "ADC offset (1:20)"}, 
        3    : {name: "ADC gain   (1:20)"}, 
        4    : {name: "DAC offset"}, 
        5    : {name: "DAC gain"}   
    };

    OBJ.STATES_250_12 = {
        0    : {name: "ADC offset DC (1:1)"}, 
        1    : {name: "ADC gain DC   (1:1)"}, 
        2    : {name: "ADC offset DC (1:20)"}, 
        3    : {name: "ADC gain DC   (1:20)"}, 
        4    : {name: "DAC offset"}, 
        5    : {name: "DAC gain"},
        6    : {name: "DAC offset"}, 
        7    : {name: "DAC gain"},
        8    : {name: "ADC offset AC (1:1)"}, 
        9    : {name: "ADC gain AC   (1:1)"}, 
        10   : {name: "ADC offset AC (1:20)"}, 
        11   : {name: "ADC gain AC   (1:20)"}    
    };

    OBJ.amModel = undefined;
    OBJ.amStates = undefined;
    OBJ.amCurrentTest = undefined;
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
        newCell.setAttribute("id",id+"_value");
        newCell = row.insertCell(-1);
        newCell.setAttribute("id",id+"_state");
        return id;
    }

    OBJ.amCheckEmptyVariables = function() {
        return (OBJ.amCurrentTest != undefined && OBJ.amCurrentRowID != undefined && OBJ.amStates != undefined);
    }

    OBJ.amSetName = function(){
        if (OBJ.amCheckEmptyVariables()){
            $("#"+OBJ.amCurrentRowID+"_mode").text(OBJ.amStates[OBJ.amCurrentTest].name);
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

    OBJ.amStartButtonPress = function(){
        OBJ.amShowDloag();
    }

    OBJ.amShowDloag = function(){
        $("#am_dialog_calib").modal('show');
    }

    OBJ.amStartCalibration = function(){
        OBJ.amCurrentTest = 0;
        OBJ.amCurrentRowID = OBJ.amAddNewRow();
        OBJ.amSetName();
        OBJ.amSetStartButton();

        OBJ.amCurrentTest = 1;
        OBJ.amCurrentRowID = OBJ.amAddNewRow();
        OBJ.amSetName();
        OBJ.amSetStartButton();

        OBJ.amCurrentTest = 2;
        OBJ.amCurrentRowID = OBJ.amAddNewRow();
        OBJ.amSetName();
        OBJ.amSetStartButton();

        OBJ.amCurrentTest = 3;
        OBJ.amCurrentRowID = OBJ.amAddNewRow();
        OBJ.amSetName();
        OBJ.amSetStartButton();
    }


}(window.OBJ = window.OBJ || {}, jQuery));


// Page onload event handler
$(function() {
    SM.param_callbacks["RP_MODEL_STR"] = OBJ.amSetModel;
    
});