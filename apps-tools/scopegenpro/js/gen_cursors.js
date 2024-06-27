(function(OSC, $, undefined) {

    // Common function for output signal offset
    OSC.srcVoltOffset = function(ch, new_params) {
        var param_name = 'SOUR' + ch + '_VOLT_OFFS';
        if (OSC.rp_model != "Z20") {
            if (!OSC.state.editing) {
                var value = $('#' + param_name).val();
                if (value !== new_params[param_name].value) {
                    OSC.setValue($('#' + param_name), new_params[param_name].value);
                }
            }
        }
    }

    OSC.src1VoltOffset = function(new_params) {
        OSC.srcVoltOffset("1", new_params);
    }

    OSC.src2VoltOffset = function(new_params) {
        OSC.srcVoltOffset("2", new_params);
    }

}(window.OSC = window.OSC || {}, jQuery));