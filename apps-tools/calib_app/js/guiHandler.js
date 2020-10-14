$.fn.fI = function(e) { //Flash Item
    if (!e) { e = {} }
    if (this) { e.e = this }
    switch (e.f) {
        case 0:
            break;
        default:
            switch (e.css) {
                case 0:
                    e.d = 'background-color'
                    break;
                case undefined:
                    e.d = 'border-color'
                    break;
                default:
                    e.d = e.css
                    break;
            }
            if (!e.c1) { e.c1 = '#FF0000' }
            if (!e.c2) { e.c2 = '#A00000' }
            if (!e.p) { e.p = 200 }
            e.e.css(e.d, e.c1)
            setTimeout(function() {
                e.e.css(e.d, e.c2)
                setTimeout(function() {
                    e.e.css(e.d, e.c1)
                    setTimeout(function() {
                        e.e.css(e.d, e.c2)
                        setTimeout(function() {
                            e.e.css(e.d, '')
                        }, e.p)
                    }, e.p)
                }, e.p)
            }, e.p)
            break;
    }
    return this
}

function ValidateIPaddress(ipaddress) {
    if (ipaddress == '')
        return false;
    if (/^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/.test(ipaddress)) {
        return (true);
    }
    return (false);
}

function Validate(port) {
    if (port == '')
        return false;
    if (/^([0-9]{1,2}\.{0,1}[0-9]*)$/.test(port)) {
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
var refVoltChange = function(event) {
    if (Validate($("#SS_REF_VOLT").val()) == false) {
        $("#SS_REF_VOLT").val(1);
        $('#SS_REF_VOLT').fI();
        return;
    }

    if ($("#SS_REF_VOLT").val() > 20){
        $("#SS_REF_VOLT").val(1);
        $('#SS_REF_VOLT').fI();
        return;
    }
    else if ($("#SS_REF_VOLT").val() < 0.001){
        $("#SS_REF_VOLT").val(1);
        $('#SS_REF_VOLT').fI();
        return;
    }
}



//Create callback
var changeCallbacks = {}

changeCallbacks["SS_REF_VOLT"] = refVoltChange;

var clickCallbacks = {}

//Subscribe changes and clicks
$(document).ready(function() {
    for (var k in changeCallbacks) {
        $("#" + k).change(changeCallbacks[k]);
    }
    for (var i in clickCallbacks) {
        $("#" + i).click(clickCallbacks[i]);
    }
})