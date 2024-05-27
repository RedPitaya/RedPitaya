/*
 * Red Pitaya stream service manager
 *
 * Author: Danilyuk Nikolay <n.danilyuk@integrasources.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

function ValidateIPaddress(ipaddress) {
    if (ipaddress == '')
        return false;
    if (/^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/.test(ipaddress)) {
        return (true);
    }
    return (false);
}

function ValidatePort(port) {
    if (port == '')
        return false;
    if (/^([0-9]{1,5}?)$/.test(port)) {
        return (true);
    }
    return (false);
}

function ValidateSamples(port) {
    if (port == '')
        return false;
    if (/^([0-9]{1,15}?)$/.test(port)) {
        return (true);
    }
    return (false);
}

//Port number changed
var portNumberChange = function(event) {
    if (ValidatePort($("#SS_PORT_NUMBER").val()) == false) {
        $("#SS_PORT_NUMBER").val(8900);
        $('#SS_PORT_NUMBER').fI();
        return;
    }

    if ($("#SS_PORT_NUMBER").val() > 65535)
        $("#SS_PORT_NUMBER").val(8900);
    else if ($("#SS_PORT_NUMBER").val() < 1)
        $("#SS_PORT_NUMBER").val(8900);

    CLIENT.parametersCache["SS_PORT_NUMBER"] = { value: $("#SS_PORT_NUMBER").val() };
    CLIENT.sendParameters();
}


var samplesNumberChange = function(event) {
    var samples = $("#SS_SAMPLES").val()
    var max_val = 2000000000;
    if (samples != "") {
        if (ValidateSamples(samples) == false) {
            $("#SS_SAMPLES").val(max_val);
            $('#SS_SAMPLES').fI();
            samples = max_val;
        }
        if (samples > max_val) {
            samples = max_val;
            $("#SS_SAMPLES").val(max_val);
        }
        CLIENT.parametersCache["SS_SAMPLES"] = { value: samples };
    } else {
        $("#SS_SAMPLES").val("ALL");
        CLIENT.parametersCache["SS_SAMPLES"] = { value: -1 };
    }
    CLIENT.sendParameters();
}


//IP address changed
var ipAddressChange = function(event) {
    if (ValidateIPaddress($("#SS_IP_ADDR").val()) == false) {
        SM.GetIP();
        $('#SS_IP_ADDR').fI();
        return;
    }

    CLIENT.parametersCache["SS_IP_ADDR"] = { value: $("#SS_IP_ADDR").val() };
    CLIENT.sendParameters();
}


var sendByNetChange = function(event) {
    CLIENT.parametersCache["SS_USE_FILE"] = { value: false };
    CLIENT.sendParameters();
    SM.updateLimits();
}

var sendToFileChange = function(event) {
    CLIENT.parametersCache["SS_USE_FILE"] = { value: true };
    CLIENT.sendParameters();
    SM.updateLimits();
}

var attenuatorChange = function(event) {
    CLIENT.parametersCache["SS_ATTENUATOR"] = { value: $("#SS_ATTENUATOR option:selected").val() };
    CLIENT.sendParameters();
    SM.updateLimits();
}

var acdcChange = function(event) {
    CLIENT.parametersCache["SS_AC_DC"] = { value: $("#SS_AC_DC option:selected").val() };
    CLIENT.sendParameters();
    SM.updateLimits();
}

var formatСhange = function(event) {
    CLIENT.parametersCache["SS_FORMAT"] = { value: $("#SS_FORMAT option:selected").val() };
    CLIENT.sendParameters();
}

var rateFocusOut = function(event) {
    SM.calcRateHz($("#SS_RATE").val());
    rateFocusOutValue();
}

var rateFocusOutValue = function() {

    CLIENT.parametersCache["SS_RATE"] = { value: SM.ss_rate };
    CLIENT.sendParameters();
    console.log("SEND RATE " + SM.ss_rate);

    text = "";
    rate_hz = SM.calcRateDecToHz();
    if (rate_hz > 1000000)
        text = Math.round(rate_hz / 1000) / 1000 + " MHz";
    else if (rate_hz > 1000)
        text = rate_hz / 1000 + " kHz";
    else
        text = rate_hz + " Hz";

    $("#SS_RATE").val(text);
}

var setMode = function(_mode, _state) {
    if (_mode == "SS_MODE") {
        if (_state){
            sendToFileChange();
        }else{
            sendByNetChange();
        }
    }

    if (_mode == "SS_PROTOCOL"){
        CLIENT.parametersCache["SS_PROTOCOL"] = { value: _state ? 1 : 0};
        CLIENT.sendParameters();
    }

    if (_mode == "SS_USE_CALIB"){
        CLIENT.parametersCache["SS_USE_CALIB"] = { value: _state ? 1 : 0};
        CLIENT.sendParameters();
    }

    if (_mode == "SS_RESOLUTION"){
        CLIENT.parametersCache["SS_RESOLUTION"] = { value: _state ? 1 : 0};
        CLIENT.sendParameters();
    }

    if (_mode == "SS_SAVE_MODE"){
        CLIENT.parametersCache["SS_SAVE_MODE"] = { value: _state ? 1 : 0};
        CLIENT.sendParameters();
    }

    if (_mode == "SS_CH1_ENABLE"){
        var curValue = CLIENT.params.orig["SS_CHANNEL"] != undefined ? CLIENT.params.orig["SS_CHANNEL"].value : 0;
        CLIENT.parametersCache["SS_CHANNEL"] = { value: (curValue & 0xE) | (_state ? 0x1 : 0x0) };
        CLIENT.sendParameters();
    }

    if (_mode == "SS_CH2_ENABLE"){
        var curValue = CLIENT.params.orig["SS_CHANNEL"] != undefined ? CLIENT.params.orig["SS_CHANNEL"].value : 0;
        CLIENT.parametersCache["SS_CHANNEL"] = { value: (curValue & 0xD) | (_state ? 0x2 : 0x0) };
        CLIENT.sendParameters();
    }

    if (_mode == "SS_CH3_ENABLE"){
        var curValue = CLIENT.params.orig["SS_CHANNEL"] != undefined ? CLIENT.params.orig["SS_CHANNEL"].value : 0;
        CLIENT.parametersCache["SS_CHANNEL"] = { value: (curValue & 0xB) | (_state ? 0x4 : 0x0) };
        CLIENT.sendParameters();
    }

    if (_mode == "SS_CH4_ENABLE"){
        var curValue = CLIENT.params.orig["SS_CHANNEL"] != undefined ? CLIENT.params.orig["SS_CHANNEL"].value : 0;
        CLIENT.parametersCache["SS_CHANNEL"] = { value: (curValue & 0x7) | (_state ? 0x8 : 0x0) };
        CLIENT.sendParameters();
    }

    if (_mode == "SS_CH1_ATTENUATOR"){
        var curValue = CLIENT.params.orig["SS_ATTENUATOR"] != undefined ? CLIENT.params.orig["SS_ATTENUATOR"].value : 0;
        CLIENT.parametersCache["SS_ATTENUATOR"] = { value: (curValue & 0xE) | (_state ? 0x1 : 0x0) };
        CLIENT.sendParameters();
    }

    if (_mode == "SS_CH2_ATTENUATOR"){
        var curValue = CLIENT.params.orig["SS_ATTENUATOR"] != undefined ? CLIENT.params.orig["SS_ATTENUATOR"].value : 0;
        CLIENT.parametersCache["SS_ATTENUATOR"] = { value: (curValue & 0xD) | (_state ? 0x2 : 0x0) };
        CLIENT.sendParameters();
    }

    if (_mode == "SS_CH3_ATTENUATOR"){
        var curValue = CLIENT.params.orig["SS_ATTENUATOR"] != undefined ? CLIENT.params.orig["SS_ATTENUATOR"].value : 0;
        CLIENT.parametersCache["SS_ATTENUATOR"] = { value: (curValue & 0xB) | (_state ? 0x4 : 0x0) };
        CLIENT.sendParameters();
    }

    if (_mode == "SS_CH4_ATTENUATOR"){
        var curValue = CLIENT.params.orig["SS_ATTENUATOR"] != undefined ? CLIENT.params.orig["SS_ATTENUATOR"].value : 0;
        CLIENT.parametersCache["SS_ATTENUATOR"] = { value: (curValue & 0x7) | (_state ? 0x8 : 0x0) };
        CLIENT.sendParameters();
    }


    if (_mode == "SS_CH1_AC_DC"){
        var curValue = CLIENT.params.orig["SS_AC_DC"] != undefined ? CLIENT.params.orig["SS_AC_DC"].value : 0;
        CLIENT.parametersCache["SS_AC_DC"] = { value: (curValue & 0xE) | (_state ? 0x1 : 0x0) };
        CLIENT.sendParameters();
    }

    if (_mode == "SS_CH2_AC_DC"){
        var curValue = CLIENT.params.orig["SS_AC_DC"] != undefined ? CLIENT.params.orig["SS_AC_DC"].value : 0;
        CLIENT.parametersCache["SS_AC_DC"] = { value: (curValue & 0xD) | (_state ? 0x2 : 0x0) };
        CLIENT.sendParameters();
    }

    if (_mode == "SS_CH3_AC_DC"){
        var curValue = CLIENT.params.orig["SS_AC_DC"] != undefined ? CLIENT.params.orig["SS_AC_DC"].value : 0;
        CLIENT.parametersCache["SS_AC_DC"] = { value: (curValue & 0xB) | (_state ? 0x4 : 0x0) };
        CLIENT.sendParameters();
    }

    if (_mode == "SS_CH4_AC_DC"){
        var curValue = CLIENT.params.orig["SS_AC_DC"] != undefined ? CLIENT.params.orig["SS_AC_DC"].value : 0;
        CLIENT.parametersCache["SS_AC_DC"] = { value: (curValue & 0x7) | (_state ? 0x8 : 0x0) };
        CLIENT.sendParameters();
    }

}



//Create callback
var changeCallbacks = {}

changeCallbacks["SS_PORT_NUMBER"] = portNumberChange;
changeCallbacks["SS_IP_ADDR"] = ipAddressChange;
// changeCallbacks["SS_USE_NET"] = sendByNetChange;
// changeCallbacks["SS_USE_FILE"] = sendToFileChange;
// changeCallbacks["SS_CHANNEL"] = channelChange;
changeCallbacks["SS_FORMAT"] = formatСhange;
changeCallbacks["SS_SAMPLES"] = samplesNumberChange;
// changeCallbacks["SS_ATTENUATOR"] = attenuatorChange;
// changeCallbacks["SS_AC_DC"] = acdcChange;

var clickCallbacks = {}

//Subscribe changes and clicks
$(document).ready(function() {
    for (var k in changeCallbacks) {
        $("#" + k).change(changeCallbacks[k]);
    }
    for (var i in clickCallbacks) {
        $("#" + i).click(clickCallbacks[i]);
    }

    $("#SS_RATE").focus(function() {
        $("#SS_RATE").val(SM.calcRateDecToHz());
    });

    $("#SS_RATE").focusout(rateFocusOut);

    $('.man_flipswitch').change(function() {
        $(this).next().text($(this).is(':checked') ? ':checked' : ':not(:checked)');
        setMode($(this).attr('id'), $(this).is(':checked'));

    }).trigger('change');
})