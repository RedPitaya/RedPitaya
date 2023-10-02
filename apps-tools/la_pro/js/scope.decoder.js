(function(COMMON, $, undefined) {
    COMMON.log_data = "";
    COMMON.hidden_log_data = "";
    COMMON.export_log = "";
    COMMON.dataArr = [];
    COMMON.hiddenDataArr = [];
    COMMON.oldResult = "";
    COMMON.savedResultArr = [];

    COMMON.drawHexagon = function(index, plot, canvascontext, series, begin, length, fillcolor, textVal) {
        if (!OSC.enabled_channels[index])
            return;

        if (length < 10)
            return;

        if (begin > 1024) return;

        canvascontext.font = "15px Arial";
        canvascontext.textAlign = "center";
        canvascontext.fillStyle = fillcolor;

        var o = plot.pointOffset({
            x: begin,
            y: series + OSC.voltage_steps[OSC.voltage_index] / 2
        });
        var q = plot.pointOffset({
            x: length,
            y: series + OSC.voltage_steps[OSC.voltage_index] / 2
        });
        o.top -= 8;
        q.top -= 8;

        canvascontext.fillRect(o.left, o.top, q.left, 16);
        canvascontext.fillStyle = fillcolor;

        canvascontext.fillStyle = "#fff";

        for (var t in textVal) {
            // Calculate width of text
            txtWidth = canvascontext.measureText(textVal[t]).width;

            // Draw text if can
            if (q.left >= txtWidth) {
                canvascontext.fillText(textVal[t], o.left + q.left / 2, o.top + 12);
                break;
            }
        }
    }

    COMMON.drawCircle = function(index, plot, canvascontext, series, begin, length, fillcolor, textVal) {
        if (!OSC.enabled_channels[index])
            return;

        if (length < 0.125 / 4.0)
            return;

        if (begin > 1024) return;

        canvascontext.font = "15px Arial";
        canvascontext.textAlign = "center";

        var o = plot.pointOffset({
            x: begin,
            y: series + OSC.voltage_steps[OSC.voltage_index] / 2

        });
        o.top -= 8;

        canvascontext.beginPath();
        canvascontext.arc(o.left + 5, o.top + 8, 8, 0, 2 * Math.PI, false);
        canvascontext.fillStyle = fillcolor;
        canvascontext.fill();

        canvascontext.fillStyle = "#fff";
        canvascontext.fillText(textVal, o.left + 5, o.top + 12);
    }

    COMMON.drawTopCircle = function(index, plot, canvascontext, series, begin, length, fillcolor, textVal) {
        if (!OSC.enabled_channels[index])
            return;

        if (length < 0.125 / 4.0)
            return;

        if (begin > 1024) return;

        canvascontext.font = "15px Arial";
        canvascontext.textAlign = "center";

        var o = plot.pointOffset({
            x: begin,
            y: series + OSC.voltage_steps[OSC.voltage_index] / 2

        });
        o.top += 8;

        canvascontext.beginPath();
        canvascontext.arc(o.left + 5, o.top + 8, 8, 0, 2 * Math.PI, false);
        canvascontext.fillStyle = fillcolor;
        canvascontext.fill();

        canvascontext.fillStyle = "#fff";
        canvascontext.fillText(textVal, o.left + 5, o.top + 12);
    }

    COMMON.appendPrefixPostfix = function(value, prefix, postfix) {
        var res = "";
        if (prefix != undefined) res += prefix;
        res += value;
        if (postfix != undefined)
            res += postfix;
        return res;
    }

    COMMON.formatData = function(data, prefix, postfix) {
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
        switch (parseInt(OSC.state.radix)) {
            case 1: //ASCII
                res.push(COMMON.appendPrefixPostfix(ch1, prefix, postfix));
                res.push(COMMON.appendPrefixPostfix(ch1, "", ""));
                break;
            case 17: // ASCII & HEX
                res.push(COMMON.appendPrefixPostfix(ch17, prefix, postfix));
                res.push(COMMON.appendPrefixPostfix(ch16, prefix, postfix));
                res.push(COMMON.appendPrefixPostfix(ch16, "", ""));
                break;
            case 10: //DEC
                res.push(COMMON.appendPrefixPostfix(ch10, prefix, postfix));
                res.push(COMMON.appendPrefixPostfix(ch16, prefix, postfix));
                res.push(COMMON.appendPrefixPostfix(ch16, "", ""));
                break;
            case 2: //BIN
                res.push(COMMON.appendPrefixPostfix(ch2, prefix, postfix));
                res.push(COMMON.appendPrefixPostfix(ch16, prefix, postfix));
                res.push(COMMON.appendPrefixPostfix(ch16, "", ""));
                break;
            case 16: //HEX
                res.push(COMMON.appendPrefixPostfix(ch16, prefix, postfix));
                res.push(COMMON.appendPrefixPostfix(ch16, "", ""));
                break;
        }
        if (res.length === 0) res = [""];
        return res;
    }

    COMMON.formatExportData = function(data, prefix, postfix) {
        var ch = "";
        switch (parseInt(OSC.state.export_radix)) {
            case 1: //ASCII
                ch = '\'' + String.fromCharCode(data) + '\'';
                break;
            case 17: // ASCII & HEX
                var hex = ConvertBase.dec2hex(data);
                if (hex.length < 2)
                    hex = '0' + hex;
                ch = '\'' + String.fromCharCode(data) + '\'' + "(" + "0x" + hex + ")";
                break;
            case 10: //DEC
                ch = "" + data;
                break;
            case 2: //BIN
                ch = ConvertBase.dec2bin(data);
                if (ch.length < 8) {
                    var howMany = 8 - ch.length;
                    var appString = "";
                    for (var i = 0; i < howMany; i++)
                        appString += '0';
                    appString += ch;
                    ch = appString;
                }
                ch = "0b" + ch;
                break;
            case 16: //HEX
                ch = ConvertBase.dec2hex(data);
                if (ch.length < 2)
                    ch = '0' + ch;
                ch = "0x" + ch;
                break;
        }
        var res = "";
        if (prefix != undefined)
            res += prefix;
        res += ch;
        if (postfix != undefined)
            res += postfix;
        return res;
    }

    COMMON.appendHead = function(value) {
        var bus = parseInt(OSC.current_bus.substr(3)) * 1;
        if (bus != -1 && OSC.log_buses[bus - 1])
            COMMON.hidden_log_data += value + "\n";
    }

    COMMON.appendLog = function(start_pos, value) {
        var bus = parseInt(OSC.current_bus.substr(3)) * 1;
        if (bus != -1 && OSC.log_buses[bus - 1]) {
            var samplerate = OSC.state.acq_speed * OSC.scales[OSC.scale_index];
            var s = start_pos / samplerate;

            var dataObj = {
                time: s,
                value: "<div class='data_row' offset='" + (start_pos + OSC.counts_offset) + "'>" + value + "</div>"
            };

            var dataHiddenObj = {
                time: s,
                value: "s;" + value + '\n',
                startpos: start_pos,
                abspos: start_pos + OSC.counts_offset
            };

            COMMON.dataArr.push(dataObj);
            COMMON.hiddenDataArr.push(dataHiddenObj);
        }
    }

    COMMON.compare = function(a, b) {
        if (a.time > b.time)
            return 1;
        if (a.time < b.time)
            return -1;
        return 0;
    }

    COMMON.fflushLog = function() {
        var bus = parseInt(OSC.current_bus.substr(3)) * 1;

        var sortedDataArr = COMMON.dataArr.sort(COMMON.compare);
        var hiddenSortedDataArr = COMMON.hiddenDataArr.sort(COMMON.compare);
        var result = "";
        var hiddenResult = "";

        for (var item in sortedDataArr) {
            result += sortedDataArr[item].value;
        }

        for (var item in hiddenSortedDataArr) {
            hiddenResult += hiddenSortedDataArr[item].time.toFixed(6) + hiddenSortedDataArr[item].value;
        }

        if (hiddenResult != COMMON.oldResult) {
            COMMON.clearLog();
            $('#log-container').html(result);
            $('#hidden-log-container').text(hiddenResult);
            COMMON.oldResult = hiddenResult;
            COMMON.savedResultArr = COMMON.hiddenDataArr;
        }

        COMMON.log_data = "";
        COMMON.hidden_log_data = "";

        result = "";
        COMMON.dataArr = [];
        hiddenResult = "";
        COMMON.hiddenDataArr = [];
    }

    COMMON.clearLog = function() {
        $('.data_row').unbind('click');
        $('#log-container').empty();
        $('#hidden-log-container').empty();
    }

}(window.COMMON = window.COMMON || {}, jQuery));