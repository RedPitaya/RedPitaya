/*
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


(function(COMMON, $, undefined) {

    // Sampling rates
    COMMON.max_freq = undefined;


    $.fn.iLightInputNumber = function(options) {
        var inBox = '.input-number-box';
        var newInput = '.input-number';
        var moreVal = '.input-number-more';
        var lessVal = '.input-number-less';

        this.each(function() {
            var el = $(this);
            $('<div class="' + inBox.substr(1) + '"></div>').insertAfter(el);
            var parent = el.find('+ ' + inBox);
            parent.append(el);
            var classes = el.attr('class');

            el.addClass(classes);
            var attrValue;

            parent.append('<div class=' + moreVal.substr(1) + '></div>');
            parent.append('<div class=' + lessVal.substr(1) + '></div>');
        }); //end each

        var value;
        var step;
        var interval = null;
        var timeout = null;

        function ToggleValue(input) {
            input.val(parseInt(input.val(), 10) + d);
            // console.log(input);
        }

        $('body').on('mousedown', moreVal, function() {
            var el = $(this);
            var input = el.siblings(newInput);
            moreValFn(input);
            timeout = setTimeout(function() {
                interval = setInterval(function() {
                    moreValFn(input);
                }, 50);
            }, 200);

        });

        $('body').on('mousedown', lessVal, function() {
            var el = $(this);
            var input = el.siblings(newInput);
            lessValFn(input);
            timeout = setTimeout(function() {
                interval = setInterval(function() {
                    lessValFn(input);
                }, 50);
            }, 200);
        });

        $(moreVal + ', ' + lessVal).on("mouseup mouseout", function() {
            clearTimeout(timeout);
            clearInterval(interval);
        });

        function moreValFn(input) {
            var max;
            var limits = getLimits(input);
            max = limits.max;
            checkInputAttr(input);

            var newValue = value + step;
            var parts = step.toString().split('.');
            var signs = parts.length < 2 ? 0 : parts[1].length;
            newValue = parseFloat(newValue.toFixed(signs));

            if (newValue > max) {
                newValue = max;
            }
            changeInputsVal(input, newValue);
        }

        function getLimits(input) {
            var min = parseFloat(input.attr('min'));
            var max = parseFloat(input.attr('max'));
            return {
                'min': min,
                'max': max
            };
        }

        function lessValFn(input) {
            var limits = getLimits(input);
            var min = limits.min;

            checkInputAttr(input);

            var newValue = value - step;
            var parts = step.toString().split('.');
            var signs = parts.length < 2 ? 0 : parts[1].length;
            newValue = parseFloat(newValue.toFixed(signs));
            if (newValue < min) {
                newValue = min;
            }
            changeInputsVal(input, newValue);
        }

        function changeInputsVal(input, newValue) {
            input.val(newValue);
            SPEC.exitEditing(true);
        }

        function checkInputAttr(input) {
            value = parseFloat(input.val());

            if (!($.isNumeric(value))) {
                value = 0;
            }

            if (input.attr('step')) {
                step = parseFloat(input.attr('step'));
            } else {
                step = 1;
            }
        }

        $(newInput).change(function() {
            var input = $(this);

            checkInputAttr(input);
            var limits = getLimits(input);
            var min = limits.min;
            var max = limits.max;

            var parts = step.toString().split('.');
            var signs = parts.length < 2 ? 0 : parts[1].length;
            value = parseFloat(value.toFixed(signs));

            if (value < min) {
                value = min;
            } else if (value > max) {
                value = max;
            }

            if (!($.isNumeric(value))) {
                value = 0;
            }

            input.val(value);
        });

        $(newInput).keydown(function(e) {
            var input = $(this);
            var k = e.keyCode;
            if (k == 38) {
                moreValFn(input);
            } else if (k == 40) {
                lessValFn(input);
            }
        });
    }

    Date.prototype.format = function(mask, utc) {
        return dateFormat(this, mask, utc);
    };

    COMMON.updateMaxFreq = function(value) {
        if (COMMON.max_freq == undefined) {
            COMMON.max_freq = value;
            const round = (n, dp) => {
                const h = +('1'.padEnd(dp + 1, '0')) // 10 or 100 or 1000 or etc
                return Math.round(n * h) / h
            }
            var nodes = document.getElementsByClassName("speed_val");
            [...nodes].forEach((element, index, array) => {
                dec = parseInt(element.attributes.getNamedItem("value").value);
                val = COMMON.max_freq / dec
                item = val
                if (item >= 1e6){
                    suff = "M"
                    val = round(val / 1e6,3)
                } else if (item >= 1e3){
                    suff = "k"
                    val = round(val / 1e3,3)
                }
                element.textContent = val + " " + suff + "S/s"
            });
        }
    };

    COMMON.repackSignals = function(signals) {
        var vals = {};
        var res = {};
        var hasData = false;
        for (var i = 1; i < 9; i++) {
            res["ch" + i] = {};
            res["ch" + i]["value"] = [];
            res["ch" + i]["size"] = 0;
        }

        for (var i = 0; i < signals["data_rle"].size; i += 2) {
            var length = signals["data_rle"].value[i] + 1;
            for (var chn = 0; chn < 8; chn++) {
                var ch = "ch" + (chn + 1);
                var val = (signals["data_rle"].value[i + 1] >> chn) & 1;
                if (res[ch].value.length > 0) {
                    if (val == (res[ch].value[res[ch].value.length - 1]))
                        res[ch].value[res[ch].value.length - 2] += length;
                    else {
                        res[ch].value.push(length);
                        res[ch].value.push(val);
                    }
                } else {
                    res[ch].value.push(length);
                    res[ch].value.push(val);
                }
            }
        }

        for (var k in res) {
            res[k]['size'] = res[k].value.length;
        }
        return res;
    }

    COMMON.saveGraphs = function() {
        const fireEvent = function (obj, evt) {
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

        html2canvas(document.querySelector("body"), {backgroundColor: '#343433'}).then(canvas => {
            var a = document.createElement('a');
            // toDataURL defaults to png, so we need to request a jpeg, then convert for file download.
            a.href = canvas.toDataURL("image/jpeg").replace("image/jpeg", "image/octet-stream");
            a.download = 'graphs.jpg';
            // a.click(); // Not working with firefox
            fireEvent(a, 'click');
        });
    }

    COMMON.downloadDataAsCSV = function(filename) {
        var strings = ['i'];
        var col_delim = ', ';
        var row_delim = '\n';

        // Do nothing if no parameters received yet
        if ($.isEmptyObject(OSC.params.orig))
            return;

        var signal_names = ['ch1', 'ch2', 'ch3', 'ch4', 'ch5', 'ch6', 'ch7', 'ch8'];
        var points_all = [];
        var size_max = 0;

        for (var sig_name in OSC.recv_signals) {
            var index = signal_names.indexOf(sig_name);

            // Ignore empty signals
            if (OSC.recv_signals[sig_name].size == 0)
                continue;

            if (!OSC.enabled_channels[index])
                continue;

            var points = [];
            var start_x = 0;

            for (var u = 0; u < OSC.recv_signals[sig_name].value.length; u += 2) {
                // Start, end, value
                var size_x = OSC.recv_signals[sig_name].value[u];
                points.push([start_x, start_x + size_x, OSC.recv_signals[sig_name].value[u + 1]]);
                start_x += size_x;
            }

            if (start_x > size_max) {
                size_max = start_x;
            }

            strings.push(col_delim, sig_name);
            points_all.push(points);
        }

        strings.push(row_delim);

        for (var i = 0; i < size_max; ++i) {
            strings.push(i);

            for (var sig_i = 0; sig_i < points_all.length; ++sig_i) {
                var points = points_all[sig_i];
                var is_valid_value = false;

                for (var point_i = 0; point_i < points.length; ++point_i) {
                    if ((i >= points[point_i][0]) && (i < points[point_i][1])) {
                        strings.push(col_delim, points[point_i][2]);
                        is_valid_value = true;
                        break;
                    }
                }

                if (!is_valid_value) {
                    strings.push(col_delim, '-1');
                }
            }

            strings.push(row_delim);
        }

        saveAs(new Blob([strings.join('')], { type: "text/plain;charset=utf-8" }), filename);
    };

    // Converts time from milliseconds to a more 'user friendly' time unit; returned value includes units
    COMMON.convertTime = function(t) {
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
            unit = 'us';
        } else if (abs_t >= 0.000001) {
            t = t * 1000000;
            unit = ' ns';
        }

        return +(t.toFixed(2)) + ' ' + unit;
    };

    COMMON.convertTimeFromSec = function(t) {
        var abs_t = Math.abs(t);
        var unit = 's';
        if (abs_t >= 1) {
            t = t;
            unit = 's';
        } else if (abs_t >= 0.001) {
            t = t * 1000;
            unit = 'ms';
        } else if (abs_t >= 0.000001) {
            t = t * 1000000;
            unit = 'us';
        } else if (abs_t >= 0.000000001) {
            t = t * 1000000000;
            unit = ' ns';
        }

        return +(t.toFixed(2)) + ' ' + unit;
    };


    COMMON.getTimePerDiv = function(t) {
        var abs_t = Math.abs(t);
        if (abs_t >= 1000) {
            return t / 1000;
        } else if (abs_t >= 1) {
            return t;
        } else if (abs_t >= 0.001) {
            return t * 1000;
        } else if (abs_t >= 0.000001) {
            return t * 1000000;
        }
    };

    // Converts voltage from volts to a more 'user friendly' unit; returned value includes units
    COMMON.convertVoltage = function(v) {
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

    COMMON.accordingChanName = function(chan_number) {
        for (var i = 1; i < 5; i++) {
            var bus = 'bus' + i;
            var enable = CLIENT.getValue('DECODER_ENABLED_'+i)
            if (LA.buses[bus] !== undefined && LA.buses[bus].name !== undefined && enable) {
                // Check UART
                if (LA.buses[bus].name == "UART"){
                    if (LA.buses[bus].config.rx == chan_number)
                        return "UART: RX";
                    if (LA.buses[bus].config.tx == chan_number)
                        return "UART: TX";
                }

                if (LA.buses[bus].name == "CAN" && LA.buses[bus].config.rx == chan_number) {
                    return "CAN: RX";
                }

                // Check I2C
                if (LA.buses[bus].name == "I2C") {
                    if (LA.buses[bus].config.scl == chan_number)
                        return "I2C: SCL";
                    else if (LA.buses[bus].config.sda == chan_number)
                        return "I2C: SDA";
                }

                // Check SPI
                if (LA.buses[bus].name == "SPI") {
                    if (LA.buses[bus].config.clk == chan_number)
                        return "SPI: CLK";
                    else if (LA.buses[bus].config.miso == chan_number)
                        return "SPI: MISO";
                    else if (LA.buses[bus].config.mosi == chan_number)
                        return "SPI: MOSI";
                    else if (LA.buses[bus].config.cs == chan_number)
                        return "SPI: CS";
                }
            }
        }
        return "";
    }

    COMMON.formatData = function(data, prefix, postfix, radix) {

        const appendPrefixPostfix = function(value, prefix, postfix) {
            var res = "";
            if (prefix != undefined) res += prefix;
            res += value;
            if (postfix != undefined)
                res += postfix;
            return res;
        }

        var ch = "";
        var ch1 = '\'' + String.fromCharCode(data) + '\'';
        var hex = ConvertBase.dec2hex(data);
        if (hex.length < 2) hex = '0' + hex;
        var ch17 = '\'' + String.fromCharCode(data) + '\'' + "(" + "0x" + hex + ")";
        var ch10 = data;
        ch = ConvertBase.dec2bin(data);
        if (ch.length < 8) {
            var howMany = 8 - ch.length;
            var appString = "";
            for (var i = 0; i < howMany; i++)
                appString += '0';
            appString += ch;
            ch = appString;
        }
        var ch2 = "0b" + ch;
        ch = ConvertBase.dec2hex(data);
        if (ch.length < 2)
            ch = '0' + ch;
        var ch16 = "0x" + ch;
        var res = [];
        switch (parseInt(radix)) {
            case 1: //ASCII
                res.push(appendPrefixPostfix(ch1, prefix, postfix));
                res.push(appendPrefixPostfix(ch1, "", ""));
                break;
            case 17: // ASCII & HEX
                res.push(appendPrefixPostfix(ch17, prefix, postfix));
                res.push(appendPrefixPostfix(ch16, prefix, postfix));
                res.push(appendPrefixPostfix(ch16, "", ""));
                break;
            case 10: //DEC
                res.push(appendPrefixPostfix(ch10, prefix, postfix));
                res.push(appendPrefixPostfix(ch16, prefix, postfix));
                res.push(appendPrefixPostfix(ch16, "", ""));
                break;
            case 2: //BIN
                res.push(appendPrefixPostfix(ch2, prefix, postfix));
                res.push(appendPrefixPostfix(ch16, prefix, postfix));
                res.push(appendPrefixPostfix(ch16, "", ""));
                break;
            case 16: //HEX
                res.push(appendPrefixPostfix(ch16, prefix, postfix));
                res.push(appendPrefixPostfix(ch16, "", ""));
                break;
        }
        if (res.length === 0) res = [""];
        return res;
    }

    // COMMON.formatExportData = function(data, prefix, postfix) {
    //     var ch = "";
    //     switch (parseInt(OSC.state.export_radix)) {
    //         case 1: //ASCII
    //             ch = '\'' + String.fromCharCode(data) + '\'';
    //             break;
    //         case 17: // ASCII & HEX
    //             var hex = ConvertBase.dec2hex(data);
    //             if (hex.length < 2)
    //                 hex = '0' + hex;
    //             ch = '\'' + String.fromCharCode(data) + '\'' + "(" + "0x" + hex + ")";
    //             break;
    //         case 10: //DEC
    //             ch = "" + data;
    //             break;
    //         case 2: //BIN
    //             ch = ConvertBase.dec2bin(data);
    //             if (ch.length < 8) {
    //                 var howMany = 8 - ch.length;
    //                 var appString = "";
    //                 for (var i = 0; i < howMany; i++)
    //                     appString += '0';
    //                 appString += ch;
    //                 ch = appString;
    //             }
    //             ch = "0b" + ch;
    //             break;
    //         case 16: //HEX
    //             ch = ConvertBase.dec2hex(data);
    //             if (ch.length < 2)
    //                 ch = '0' + ch;
    //             ch = "0x" + ch;
    //             break;
    //     }
    //     var res = "";
    //     if (prefix != undefined)
    //         res += prefix;
    //     res += ch;
    //     if (postfix != undefined)
    //         res += postfix;
    //     return res;
    // }

}(window.COMMON = window.COMMON || {}, jQuery));
