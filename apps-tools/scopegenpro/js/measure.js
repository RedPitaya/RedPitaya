(function(OSC, $, undefined) {

    OSC.measureHandlerFunc = function(param_name,new_params){
        if (param_name.indexOf('OSC_MEAS_VAL') == 0) {
            var orig_units = $("#" + param_name).parent().children("#OSC_MEAS_ORIG_UNITS").text();
            var orig_function = $("#" + param_name).parent().children("#OSC_MEAS_ORIG_FOO").text();
            var orig_source = $("#" + param_name).parent().children("#OSC_MEAS_ORIG_SIGNAME").text();
            var y = new_params[param_name].value;
            var o_val = new_params[param_name].value;
            var z = parseFloat(y) * 1;
            var factor = '';

            if (orig_function == "PERIOD") {
                y /= 1000; // Now in seconds and not ms
                z = y;
                orig_units = 's';
                if (y < 0.000000010)
                    new_params[param_name].value = 'OVER RANGE';
                else if (y >= 0.000000010 && y <= 0.00000099990) {
                    z *= 1e9;
                    factor = 'n';
                    new_params[param_name].value = z.toFixed(0);
                } else if (y > 0.00000099990 && y <= 0.00099990) {
                    z *= 1e6;
                    factor = 'µ';
                    new_params[param_name].value = z.toFixed(1);
                } else if (y > 0.00099990 && y <= 0.99990) {
                    z *= 1e3;
                    factor = 'm';
                    new_params[param_name].value = z.toFixed(2);
                } else if (y > 0.99990 && y <= 8.5901) {
                    new_params[param_name].value = z.toFixed(3);
                } else
                    new_params[param_name].value = 'NO EDGES';

            } else if (orig_function == "FREQ") {
                if (y < 0.12)
                    new_params[param_name].value = 'NO EDGES';
                else if (y >= 0.12 && y <= 0.99990) {
                    z *= 1e3;
                    factor = 'm';
                    new_params[param_name].value = z.toFixed(0);
                } else if (y > 0.99990 && y <= 999.990) {
                    new_params[param_name].value = z.toFixed(2);
                } else if (y > 999.990 && y <= 999900.0) {
                    z /= 1e3;
                    factor = 'k';
                    new_params[param_name].value = z.toFixed(2);
                } else if (y > 999900.0 && y <= 9999900.0) {
                    z /= 1e6;
                    factor = 'M';
                    new_params[param_name].value = z.toFixed(3);
                } else if (y > 9999900.0 && y <= 50000000.0) {
                    z /= 1e6;
                    factor = 'M';
                    new_params[param_name].value = z.toFixed(2);
                } else
                    new_params[param_name].value = 'OVER RANGE';
            } else if (orig_function == "DUTY CYCLE") {
                if (y < 0 || y > 100)
                    new_params[param_name].value = 'ERROR';
                else
                    new_params[param_name].value = z.toFixed(1);
            } else // P2P, MEAN, MAX, MIN, RMS
            {
                y = Math.abs(y);
                if (orig_source == "MATH") {
                    if (y < 0.00000000000010)
                        new_params[param_name].value = 'No signal';
                    else if (y > 0.00000000000010 && y <= 0.000000999990) {
                        z *= 1e9;
                        factor = 'n';
                        if (y > 0.00000000000010 && y <= 0.00000000999990)
                            new_params[param_name].value = z.toFixed(4);
                        else if (y > 0.00000000999990 && y <= 0.0000000999990)
                            new_params[param_name].value = z.toFixed(3);
                        else if (y > 0.0000000999990 && y <= 0.000000999990)
                            new_params[param_name].value = z.toFixed(2);
                    } else if (y > 0.000000999990 && y <= 0.000999990) {
                        z *= 1e6;
                        factor = 'u';
                        if (y > 0.000000999990 && y <= 0.00000999990)
                            new_params[param_name].value = z.toFixed(4);
                        else if (y > 0.00000999990 && y <= 0.0000999990)
                            new_params[param_name].value = z.toFixed(3);
                        else if (y > 0.0000999990 && y <= 0.000999990)
                            new_params[param_name].value = z.toFixed(2);
                    } else if (y > 0.000999990 && y <= 0.999990) {
                        z *= 1e3;
                        factor = 'm';
                        if (y > 0.000999990 && y <= 0.00999990)
                            new_params[param_name].value = z.toFixed(4);
                        else if (y > 0.00999990 && y <= 0.0999990)
                            new_params[param_name].value = z.toFixed(3);
                        else if (y > 0.0999990 && y <= 0.999990)
                            new_params[param_name].value = z.toFixed(2);
                    } else if (y > 0.999990 && y <= 9999.90) {
                        if (y > 0.999990 && y <= 9.99990)
                            new_params[param_name].value = z.toFixed(4);
                        else if (y > 9.99990 && y <= 99.9990)
                            new_params[param_name].value = z.toFixed(3);
                        else if (y > 99.9990 && y <= 999.990)
                            new_params[param_name].value = z.toFixed(2);
                        else if (y > 999.990 && y <= 9999.90)
                            new_params[param_name].value = z.toFixed(1);
                    } else if (y > 9999.90 && y <= 999990.0) {
                        z /= 1e3;
                        factor = 'k';
                        if (y > 9999.90 && y <= 99999.0)
                            new_params[param_name].value = z.toFixed(3);
                        else if (y > 99999.0 && y <= 999990.0)
                            new_params[param_name].value = z.toFixed(2);
                    } else if (y > 999990.0 && y <= 999990000.0) {
                        z /= 1e6;
                        factor = 'M';
                        if (y > 999990.0 && y <= 9999900.0)
                            new_params[param_name].value = z.toFixed(4);
                        else if (y > 9999900.0 && y <= 99999000.0)
                            new_params[param_name].value = z.toFixed(3);
                        else if (y > 99999000.0 && y <= 999990000.0)
                            new_params[param_name].value = z.toFixed(2);
                    }
                } else { // CH1 or CH2
                    if (y < 0.00010)
                        new_params[param_name].value = 'LOW SIGNAL';
                    else if (y >= 0.00010 && y <= 0.99990) {
                        z *= 1e3;
                        factor = 'm';
                        new_params[param_name].value = z.toFixed(1);
                    } else if (y > 0.99990 && y <= 9.9990) {
                        new_params[param_name].value = z.toFixed(3);
                    } else if (y > 9.9990 && y <= 99.990) {
                        new_params[param_name].value = z.toFixed(2);
                    } else if (y > 99.990 && y <= 999.90) {
                        new_params[param_name].value = z.toFixed(1);
                    } else if (y > 999.90 && y <= 4000.0) {
                        z /= 1e3;
                        factor = 'k';
                        new_params[param_name].value = z.toFixed(1);
                    } else {
                        new_params[param_name].value = "OVER RANGE";
                    }
                }
            }
            $("#" + param_name).parent().children("#OSC_MEAS_UNITS").text(factor + orig_units);
            $("#" + param_name).html(new_params[param_name].value);
            new_params[param_name].value = o_val;
        }
    }

    OSC.measSelN = function(new_params) {
        try {
            if (new_params['OSC_MEAS_SELN'].value !== ""){
                OSC.updateMeasureList(JSON.parse(new_params['OSC_MEAS_SELN'].value));
                OSC.handleMeasureList();
            }
        } catch (e) {
            console.log(e);
        }
    }

    OSC.updateMeasureList = function(measureList) {
        signalNameArray = ['CH1', 'CH2','CH3', 'CH4', 'MATH'];
        signalTextArray = ['IN1', 'IN2','IN3', 'IN4', 'MATH'];

        for (var i = 0; i < measureList.length; ++i) {
            var signal_index = measureList[i] % 10;

            var options = $('#meas_operator')[0].options;
            var operator_name = null;

            for (var option_i = 0; option_i < options.length; ++option_i) {
                if (options[option_i].value == ((measureList[i] - signal_index)/10)) {
                    operator_name = options[option_i].text;
                    break;
                }
            }

            if (operator_name === null) {
                // Operator not found
                continue;
            }

            var item_id = 'meas_' + operator_name.replace(/\s/g, '_') + '_' + signalNameArray[signal_index];

            // Check if the item already exists
            if ($('#' + item_id).length > 0) {
                continue;
            }

            // Add new item
            $('<div id="' + item_id + '" class="meas-item">' + operator_name + ' (' + signalTextArray[signal_index] + ')</div>').data({
                value: measureList[i],
                operator: operator_name,
                signal: signalNameArray[signal_index]
            }).appendTo('#meas_list');
        }
    };

    OSC.handleMeasureList = function() {
        var mi_count = 0;
        var measureList = [];
        $('#info-meas').empty();
        $('#meas_list .meas-item').each(function(index, elem) {
            var $elem = $(elem);
            var item_val = $elem.data('value');

            if (item_val !== null) {
                ++mi_count;
                var units = { 'P2P': 'V', 'MEAN': 'V', 'MAX': 'V', 'MIN': 'V', 'RMS': 'V', 'DUTY CYCLE': '%', 'PERIOD': 'ms', 'FREQ': 'Hz' };
                OSC.params.local['OSC_MEAS_SEL' + mi_count] = { value: item_val };
                var sig_name = 'MATH';
                if ($elem.data('signal')[2] == '1')
                    sig_name = 'IN1';
                else if ($elem.data('signal')[2] == '2')
                    sig_name = 'IN2';
                else if ($elem.data('signal')[2] == '3')
                    sig_name = 'IN3';
                else if ($elem.data('signal')[2] == '4')
                    sig_name = 'IN4';

                // V/s or Vs unit for dy/dt and ydt
                if (sig_name == 'MATH') {
                    if ($('#OSC_MATH_OP').find(":selected").text() == 'dy/dt') {
                        units['P2P'] = 'V/s';
                        units['MEAN'] = 'V/s';
                        units['MAX'] = 'V/s';
                        units['MIN'] = 'V/s';
                        units['RMS'] = 'V/s';
                    } else if ($('#OSC_MATH_OP').find(":selected").text() == 'ydt') {
                        units['P2P'] = 'Vs';
                        units['MEAN'] = 'Vs';
                        units['MAX'] = 'Vs';
                        units['MIN'] = 'Vs';
                        units['RMS'] = 'Vs';
                    } else if ($('#OSC_MATH_OP').find(":selected").text() == '*') {
                        units['P2P'] = 'V^2';
                        units['MEAN'] = 'V^2';
                        units['MAX'] = 'V^2';
                        units['MIN'] = 'V^2';
                        units['RMS'] = 'V^2';
                    }
                }

                var u = '';
                if (OSC.params.orig['OSC_MEAS_VAL' + mi_count])
                    u = OSC.params.orig['OSC_MEAS_VAL' + mi_count].value == 'No signal' ? '' : units[$elem.data('operator')];
                $('#info-meas').append(
                    '<div>' + $elem.data('operator') + '(<span style="color:' + OSC.config.graph_colors[$elem.data('signal').toLowerCase()] + '">' + sig_name + '</span>) <span id="OSC_MEAS_VAL' + mi_count + '">-</span>&nbsp;<span id="OSC_MEAS_UNITS">' + u + '</span><span id="OSC_MEAS_ORIG_UNITS" style="display:none;">' + u + '</span><span id="OSC_MEAS_ORIG_FOO" style="display:none;">' + $elem.data('operator') + '</span><span id="OSC_MEAS_ORIG_SIGNAME" style="display:none;">' + sig_name + '</span></div>'
                );

                if (measureList.indexOf(item_val) === -1) {
                    measureList.push(item_val);
                }
            }
        });

        return measureList;
    };

    OSC.measureHandler1Func = function(new_params){
        OSC.measureHandlerFunc("OSC_MEAS_VAL1",new_params);
    }

    OSC.measureHandler2Func = function(new_params){
        OSC.measureHandlerFunc("OSC_MEAS_VAL2",new_params);
    }

    OSC.measureHandler3Func = function(new_params){
        OSC.measureHandlerFunc("OSC_MEAS_VAL3",new_params);
    }

    OSC.measureHandler4Func = function(new_params){
        OSC.measureHandlerFunc("OSC_MEAS_VAL4",new_params);
    }

}(window.OSC = window.OSC || {}, jQuery));