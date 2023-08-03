(function(OSC, $, undefined) {

    OSC.convertTimeToText = function(value){
        var x = ""
        if (value > 1000000)
            x = Math.round(value / 1000) / 1000 + " s";
        else if (value > 1000)
            x = value / 1000 + " ms";
        else
            x = value + " µs";
        return x
    }

    OSC.formEmail = function() {
        //var file = new FileReader();
        var mail = "support@redpitaya.com";
        var subject = "Crash report Red Pitaya OS";
        var body = "%0D%0A%0D%0A------------------------------------%0D%0A" + "DEBUG INFO, DO NOT EDIT!%0D%0A" + "------------------------------------%0D%0A%0D%0A";
        body += "Parameters:" + "%0D%0A" + JSON.stringify({ parameters: OSC.params }) + "%0D%0A";
        body += "Browser:" + "%0D%0A" + JSON.stringify({ parameters: $.browser }) + "%0D%0A";

        var url = 'info/info.json';
        $.ajax({
            method: "GET",
            url: url
        }).done(function(msg) {
            body += " info.json: " + "%0D%0A" + msg.responseText;
        }).fail(function(msg) {
            var info_json = msg.responseText
            var ver = '';
            try {
                var obj = JSON.parse(msg.responseText);
                ver = " " + obj['version'];
            } catch (e) {};

            body += " info.json: " + "%0D%0A" + msg.responseText;
            document.location.href = "mailto:" + mail + "?subject=" + subject + ver + "&body=" + body;
        });
    }

    // For firefox

    function fireEvent(obj, evt) {
        var fireOnThis = obj;
        if (document.createEvent) {
            var evObj = document.createEvent('MouseEvents');
            evObj.initEvent(evt, true, false);
            fireOnThis.dispatchEvent(evObj);

        } else if (document.createEventObject) {
            var evObj = document.createEventObject();
            fireOnThis.fireEvent('on' + evt, evObj);
        }
    }

    OSC.SaveGraphsPNG = function() {
        html2canvas($('body'), {
            background: '#343433', // Like background of BODY
            onrendered: function(canvas) {
                var a = document.createElement('a');
                // toDataURL defaults to png, so we need to request a jpeg, then convert for file download.
                a.href = canvas.toDataURL("image/jpeg").replace("image/jpeg", "image/octet-stream");
                a.download = 'graphs.jpg';
                // a.click(); // Not working with firefox
                fireEvent(a, 'click');
            }
        });
    }

     // Converts time from milliseconds to a more 'user friendly' time unit; returned value includes units
     OSC.convertTime = function(t, precision, print_units) {
        if (precision == undefined)
            precision = 2;

        if (print_units == undefined)
            print_units = true;

        var abs_t = Math.abs(t);
        var unit = 'ms';

        if (abs_t >= 1000) {
            t = t / 1000;
            unit = 's';
        } else if (abs_t >= 1) {
            t = t * 1;
            unit = 'ms';
        } else if (abs_t >= 0.001) {
            t = t * 1000;
            unit = 'μs';
        } else if (abs_t >= 0.000001) {
            t = t * 1000000;
            unit = ' ns';
        }

        if (print_units)
            return +(Number.parseFloat(t).toFixed(precision)) + ' ' + unit;
        else
            return +(Number.parseFloat(t).toFixed(precision));
    };

    // Converts time from milliseconds to a more 'user friendly' time unit.
    // Returns object: {str, multiplier}.
    // str: the time unit string value ('ns' / 'μs' / 'ms' / 's').
    // multiplier: the value to convert ms to target unit.
    OSC.getTimeUnits = function(t) {
        var abs_t = Math.abs(t);
        var str = 'ms';
        var multiplier = 1;
        if (abs_t >= 1000) {
            str = 's';
            multiplier = 0.001;
        } else if (abs_t >= 1) {
            str = 'ms';
            multiplier = 1;
        } else if (abs_t >= 0.001) {
            str = 'μs';
            multiplier = 1000;
        } else if (abs_t >= 0.000001) {
            str = 'ns';
            multiplier = 1000000;
        }

        return {
            str: str,
            multiplier: multiplier
        };
    };

    // Converts voltage from volts to a more 'user friendly' unit; returned value includes units
    OSC.convertVoltage = function(v) {
        var abs_v = Math.abs(v);
        var unit = 'V';

        if (abs_v >= 1) {
            v = v * 1;
            unit = 'V';
        } else if (abs_v >= 0.001) {
            v = v * 1000;
            unit = 'mV';
        }

        return +(v.toFixed(2)) + ' ' + unit;
    };

    OSC.formatInputValue = function(oldValue, attenuation, is_milis, is_hv) {
        var z = oldValue;
        if (is_milis)
            return z.toFixed(0);
        if (is_hv) {
            switch (attenuation) {
                case 1:
                    return z.toFixed(2);
                case 10:
                    return z.toFixed(1);
                case 100:
                    return z.toFixed(0);
            }
        } else {
            switch (attenuation) {
                case 1:
                    return z.toFixed(3);
                case 10:
                    return z.toFixed(2);
                case 100:
                    return z.toFixed(1);
            }
        }
        return z;
    }

    OSC.getStepValue = function(attenuation, is_milis, is_hv) {
        if (is_milis)
            return 1;
        if (is_hv) {
            switch (attenuation) {
                case 1:
                    return 0.01;
                case 10:
                    return 0.1;
                case 100:
                    return 1;
            }
        } else {
            switch (attenuation) {
                case 1:
                    return 0.001;
                case 10:
                    return 0.01;
                case 100:
                    return 0.1;
            }
        }
        return 0.001;
    }

    OSC.setValue = function(input, value) {
        input.val(value);
    };




    OSC.downloadDataAsCSV = function(filename) {
        var strings = "";


        var time_scale = OSC.params.orig["OSC_TIME_SCALE"].value; // ms
        var right_x = time_scale * 5;
        var time_step = right_x / 512;

        var in1 = OSC.params.orig["CH1_SHOW"] ? OSC.params.orig["CH1_SHOW"].value : false;
        var in2 = OSC.params.orig["CH2_SHOW"] ? OSC.params.orig["CH2_SHOW"].value : false;
        var in3 = OSC.params.orig["CH3_SHOW"] ? OSC.params.orig["CH3_SHOW"].value : false;
        var in4 = OSC.params.orig["CH4_SHOW"] ? OSC.params.orig["CH4_SHOW"].value : false;
        var out1 = OSC.params.orig["OUTPUT1_SHOW"] && OSC.params.orig["OUTPUT1_SHOW"].value && OSC.params.orig["OUTPUT1_STATE"].value;
        var out2 = OSC.params.orig["OUTPUT2_SHOW"] && OSC.params.orig["OUTPUT2_SHOW"].value && OSC.params.orig["OUTPUT2_STATE"].value;
        var math = OSC.params.orig["MATH_SHOW"] && OSC.params.orig["MATH_SHOW"].value;

        var col_delim = ", ";
        var row_delim = "\n";

        var time_unit = OSC.getTimeUnits(right_x);
        strings += "TIME " + time_unit.str

        if (in1)
            strings += col_delim + "IN1";
        if (in2)
            strings += col_delim + "IN2";
        if (in3)
            strings += col_delim + "IN3";
        if (in4)
            strings += col_delim + "IN4";
        if (out1)
            strings += col_delim + "OUT1";
        if (out2)
            strings += col_delim + "OUT2";
        if (math)
            strings += col_delim + "MATH";

        strings += row_delim;

        for (var i = 0; i < 1024; i++) {
            strings += +((time_unit.multiplier * (right_x - time_step * i)).toFixed(3));

            if (in1)
                strings += col_delim + OSC.lastSignals["ch1"].value[i];
            if (in2)
                strings += col_delim + OSC.lastSignals["ch2"].value[i];
            if (in3)
                strings += col_delim + OSC.lastSignals["ch3"].value[i];
            if (in4)
                strings += col_delim + OSC.lastSignals["ch4"].value[i];
            if (out1)
                strings += col_delim + OSC.lastSignals["output1"].value[i];
            if (out2)
                strings += col_delim + OSC.lastSignals["output2"].value[i];
            if (math)
                strings += col_delim + OSC.lastSignals["math"].value[i];

            strings += row_delim;
        };
        return 'data:' + 'text/html' + ';charset=utf-8,' + encodeURIComponent(strings);
    }

    OSC.setCPULoad = function(new_params){
        OSC.g_CpuLoad = new_params['CPU_LOAD'].value;
    }

    OSC.setRamTotal = function(new_params){
        OSC.g_TotalMemory = new_params['TOTAL_RAM'].value;
    }

    OSC.setFreeRam = function(new_params){
        OSC.g_FreeMemory = new_params['FREE_RAM'].value;
    }

}(window.OSC = window.OSC || {}, jQuery));