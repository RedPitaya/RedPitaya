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

    SM.parametersCache["SS_PORT_NUMBER"] = { value: $("#SS_PORT_NUMBER").val() };
    SM.sendParameters();
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
        SM.parametersCache["SS_SAMPLES"] = { value: samples };
    } else {
        $("#SS_SAMPLES").val("ALL");
        SM.parametersCache["SS_SAMPLES"] = { value: -1 };
    }
    SM.sendParameters();
}


//IP address changed
var ipAddressChange = function(event) {
    if (ValidateIPaddress($("#SS_IP_ADDR").val()) == false) {
        SM.GetIP();
        $('#SS_IP_ADDR').fI();
        return;
    }

    SM.parametersCache["SS_IP_ADDR"] = { value: $("#SS_IP_ADDR").val() };
    SM.sendParameters();
}


var sendByNetChange = function(event) {
    SM.parametersCache["SS_USE_FILE"] = { value: false };
    SM.sendParameters();
    SM.updateLimits();
}

var sendToFileChange = function(event) {
    SM.parametersCache["SS_USE_FILE"] = { value: true };
    SM.sendParameters();
    SM.updateLimits();
}

var attenuatorChange = function(event) {
    SM.parametersCache["SS_ATTENUATOR"] = { value: $("#SS_ATTENUATOR option:selected").val() };
    SM.sendParameters();
    SM.updateLimits();
}

var acdcChange = function(event) {
    SM.parametersCache["SS_AC_DC"] = { value: $("#SS_AC_DC option:selected").val() };
    SM.sendParameters();
    SM.updateLimits();
}

var formatСhange = function(event) {
    SM.parametersCache["SS_FORMAT"] = { value: $("#SS_FORMAT option:selected").val() };
    SM.sendParameters();
}

var rateFocusOut = function(event) {
    SM.calcRateHz($("#SS_RATE").val());
    rateFocusOutValue();
}

var rateFocusOutValue = function() {

    SM.parametersCache["SS_RATE"] = { value: SM.ss_rate };
    SM.sendParameters();
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
        SM.parametersCache["SS_PROTOCOL"] = { value: _state ? 1 : 0};
        SM.sendParameters();
    }

    if (_mode == "SS_USE_CALIB"){
        SM.parametersCache["SS_USE_CALIB"] = { value: _state ? 1 : 0};
        SM.sendParameters();
    }

    if (_mode == "SS_RESOLUTION"){
        SM.parametersCache["SS_RESOLUTION"] = { value: _state ? 1 : 0};
        SM.sendParameters();
    }

    if (_mode == "SS_SAVE_MODE"){
        SM.parametersCache["SS_SAVE_MODE"] = { value: _state ? 1 : 0};
        SM.sendParameters();
    }
    
    if (_mode == "SS_CH1_ENABLE"){
        var curValue = SM.params.orig["SS_CHANNEL"] != undefined ? SM.params.orig["SS_CHANNEL"].value : 0;
        SM.parametersCache["SS_CHANNEL"] = { value: (curValue & 0xE) | (_state ? 0x1 : 0x0) };
        SM.sendParameters();
    }

    if (_mode == "SS_CH2_ENABLE"){
        var curValue = SM.params.orig["SS_CHANNEL"] != undefined ? SM.params.orig["SS_CHANNEL"].value : 0;
        SM.parametersCache["SS_CHANNEL"] = { value: (curValue & 0xD) | (_state ? 0x2 : 0x0) };
        SM.sendParameters();
    }

    if (_mode == "SS_CH3_ENABLE"){
        var curValue = SM.params.orig["SS_CHANNEL"] != undefined ? SM.params.orig["SS_CHANNEL"].value : 0;
        SM.parametersCache["SS_CHANNEL"] = { value: (curValue & 0xB) | (_state ? 0x4 : 0x0) };
        SM.sendParameters();
    }

    if (_mode == "SS_CH4_ENABLE"){
        var curValue = SM.params.orig["SS_CHANNEL"] != undefined ? SM.params.orig["SS_CHANNEL"].value : 0;
        SM.parametersCache["SS_CHANNEL"] = { value: (curValue & 0x7) | (_state ? 0x8 : 0x0) };
        SM.sendParameters();
    }

    if (_mode == "SS_CH1_ATTENUATOR"){
        var curValue = SM.params.orig["SS_ATTENUATOR"] != undefined ? SM.params.orig["SS_ATTENUATOR"].value : 0;
        SM.parametersCache["SS_ATTENUATOR"] = { value: (curValue & 0xE) | (_state ? 0x1 : 0x0) };
        SM.sendParameters();
    }

    if (_mode == "SS_CH2_ATTENUATOR"){
        var curValue = SM.params.orig["SS_ATTENUATOR"] != undefined ? SM.params.orig["SS_ATTENUATOR"].value : 0;
        SM.parametersCache["SS_ATTENUATOR"] = { value: (curValue & 0xD) | (_state ? 0x2 : 0x0) };
        SM.sendParameters();
    }

    if (_mode == "SS_CH3_ATTENUATOR"){
        var curValue = SM.params.orig["SS_ATTENUATOR"] != undefined ? SM.params.orig["SS_ATTENUATOR"].value : 0;
        SM.parametersCache["SS_ATTENUATOR"] = { value: (curValue & 0xB) | (_state ? 0x4 : 0x0) };
        SM.sendParameters();
    }

    if (_mode == "SS_CH4_ATTENUATOR"){
        var curValue = SM.params.orig["SS_ATTENUATOR"] != undefined ? SM.params.orig["SS_ATTENUATOR"].value : 0;
        SM.parametersCache["SS_ATTENUATOR"] = { value: (curValue & 0x7) | (_state ? 0x8 : 0x0) };
        SM.sendParameters();
    }

    
    if (_mode == "SS_CH1_AC_DC"){
        var curValue = SM.params.orig["SS_AC_DC"] != undefined ? SM.params.orig["SS_AC_DC"].value : 0;
        SM.parametersCache["SS_AC_DC"] = { value: (curValue & 0xE) | (_state ? 0x1 : 0x0) };
        SM.sendParameters();
    }

    if (_mode == "SS_CH2_AC_DC"){
        var curValue = SM.params.orig["SS_AC_DC"] != undefined ? SM.params.orig["SS_AC_DC"].value : 0;
        SM.parametersCache["SS_AC_DC"] = { value: (curValue & 0xD) | (_state ? 0x2 : 0x0) };
        SM.sendParameters();
    }

    if (_mode == "SS_CH3_AC_DC"){
        var curValue = SM.params.orig["SS_AC_DC"] != undefined ? SM.params.orig["SS_AC_DC"].value : 0;
        SM.parametersCache["SS_AC_DC"] = { value: (curValue & 0xB) | (_state ? 0x4 : 0x0) };
        SM.sendParameters();
    }

    if (_mode == "SS_CH4_AC_DC"){
        var curValue = SM.params.orig["SS_AC_DC"] != undefined ? SM.params.orig["SS_AC_DC"].value : 0;
        SM.parametersCache["SS_AC_DC"] = { value: (curValue & 0x7) | (_state ? 0x8 : 0x0) };
        SM.sendParameters();
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