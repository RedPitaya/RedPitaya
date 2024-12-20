(function(COMMON, $, undefined) {
    COMMON.log_data = "";
    COMMON.hidden_log_data = "";
    COMMON.export_log = "";
    COMMON.dataArr = [];
    COMMON.hiddenDataArr = [];
    COMMON.oldResult = "";
    COMMON.savedResultArr = [];
    COMMON.decoderAnno = {};

    COMMON.addAlpha = function(color, opacity) {
        // coerce values so it is between 0 and 1.
        var _opacity = Math.round(Math.min(Math.max(opacity ?? 1, 0), 1) * 255);
        return color + _opacity.toString(16).toUpperCase();
    }

    COMMON.drawHexagon = function(plot, canvascontext, offset, begin, length, fillcolor, data) {

        var len_in_pix = LA.calculateSamplesToPixels(length)
        if (len_in_pix < 24)
            return;




        var start = plot.pointOffset({
            x: begin,
            y: offset + 0.1

        });

        var stop = plot.pointOffset({
            x: begin + length,
            y: offset + 0.4

        });

        canvascontext.fillStyle = COMMON.addAlpha(fillcolor,0.3);
        canvascontext.fillRect(start.left + 10, start.top, stop.left - start.left - 20, stop.top - start.top);
        var tw = (stop.left - start.left - 20) * 0.9
        var th = (start.top - stop.top) -4
        canvascontext.fillStyle = "#fff"
        canvascontext.textAlign = "center";
        for (var t in data) {
            for(var z = 15; z > 0; z--){
                canvascontext.font = z+"px Arial";
                var mes = canvascontext.measureText(data[t])
                var txtH = mes.actualBoundingBoxAscent + mes.actualBoundingBoxDescent;
                var txtHA = mes.actualBoundingBoxAscent
                var txtW = mes.width;
                if (tw >= txtW && th >= txtH) {
                    canvascontext.fillText(data[t], start.left + (stop.left - start.left) / 2, stop.top + (start.top - stop.top) / 2 + txtHA / 2);
                    return;
                }
            }
        }
    }


    COMMON.drawTextTop = function(plot, canvascontext, offset, begin, length, fillcolor, data) {

        var len_in_pix = LA.calculateSamplesToPixels(length)
        if (len_in_pix < 24)
            return;

        var start = plot.pointOffset({
            x: begin,
            y: offset + 0.2 + 0.5

        });

        var stop = plot.pointOffset({
            x: begin + length,
            y: offset + 0.4 + 0.5

        });

        var tw = (stop.left - start.left - 20) * 0.9
        var th = (start.top - stop.top) -4
        canvascontext.fillStyle = "#fff"
        canvascontext.textAlign = "center";
        for (var t in data) {
            for(var z = 13; z > 0; z--){
                canvascontext.font = z+"px Arial";
                var mes = canvascontext.measureText(data[t])
                var txtH = mes.actualBoundingBoxAscent + mes.actualBoundingBoxDescent;
                var txtHA = mes.actualBoundingBoxAscent
                var txtW = mes.width;
                if (tw >= txtW && th >= txtH) {
                    canvascontext.fillText(data[t], start.left + (stop.left - start.left) / 2, stop.top + (start.top - stop.top) / 2 + txtHA / 2);
                    return;
                }
            }
        }
    }

    COMMON.drawCircle = function(plot, canvascontext, offset, begin, length, fillcolor, textVal) {

        var len_in_pix = LA.calculateSamplesToPixels(length)
        if (len_in_pix < 24)
            return;


        canvascontext.font = "15px Arial";
        canvascontext.textAlign = "center";

        var o = plot.pointOffset({
            x: begin + length / 2.0,
            y: offset + 0.25

        });

        canvascontext.beginPath();
        canvascontext.arc(o.left - 1, o.top - 4, 8, 0, 2 * Math.PI, false);
        canvascontext.fillStyle = fillcolor;
        canvascontext.fill();

        canvascontext.fillStyle = "#fff";
        canvascontext.fillText(textVal, o.left - 1, o.top + 1);
    }

    COMMON.drawCircleTop = function(plot, canvascontext, offset, begin, length, fillcolor, textVal) {


        canvascontext.font = "15px Arial";
        canvascontext.textAlign = "center";

        var o = plot.pointOffset({
            x: begin,
            y: offset + 0.65

        });

        canvascontext.beginPath();
        canvascontext.arc(o.left - 1, o.top - 4, 8, 0, 2 * Math.PI, false);
        canvascontext.fillStyle = fillcolor;
        canvascontext.fill();

        canvascontext.fillStyle = "#fff";
        canvascontext.fillText(textVal, o.left - 1, o.top + 1);
    }


    COMMON.drawBitsBars = function(plot, canvascontext, offset, begin, length, fillcolor, bits) {


        var start = plot.pointOffset({
            x: begin ,
            y: offset - 0.03

        });

        var stop_point = plot.pointOffset({
            x: begin + length,
            y: offset - 0.03

        });

        var stop = plot.pointOffset({
            x: begin + length,
            y: offset - 0.05

        });

        canvascontext.fillStyle = COMMON.addAlpha(fillcolor,1);
        canvascontext.fillRect(start.left, start.top, stop.left - start.left, stop.top - start.top);

        canvascontext.strokeStyle = "#ccc"
        canvascontext.beginPath();
        canvascontext.moveTo(start.left, start.top+3);
        canvascontext.lineTo(start.left, start.top-3);
        canvascontext.stroke();

        canvascontext.beginPath();
        canvascontext.moveTo(stop_point.left, stop_point.top+3);
        canvascontext.lineTo(stop_point.left, stop_point.top-3);
        canvascontext.stroke();

        if (bits !== 0){
            var step = length / bits
            for(var pos = 0; pos < length; pos += step){
                var p = plot.pointOffset({
                    x: begin + pos,
                    y: offset - 0.03
                });

                canvascontext.beginPath();
                canvascontext.moveTo(p.left, p.top);
                canvascontext.lineTo(p.left, p.top-3);
                canvascontext.stroke();
            }
        }
        var rate = CLIENT.getValue('LA_CUR_FREQ')
        if (rate !== undefined){
            var time = 1 / rate * length
            if (time !== 0){
                var text = OSC.convertTimeFromSec(time)
                canvascontext.font = "10px Arial";
                canvascontext.textAlign = "center";
                canvascontext.fillStyle = "#fff";
                if (canvascontext.measureText(text).width * 1.1 < stop.left - start.left){
                    canvascontext.fillText(text, start.left + (stop.left - start.left )/ 2, stop.top + 10);
                }
            }
        }
    }


    COMMON.drawBitsBarsTop = function(plot, canvascontext, offset, begin, length, fillcolor, bits, show_time) {


        var start = plot.pointOffset({
            x: begin ,
            y: offset + 0.03 + 0.5

        });

        var stop_point = plot.pointOffset({
            x: begin + length,
            y: offset + 0.03 + 0.5

        });

        var stop = plot.pointOffset({
            x: begin + length,
            y: offset + 0.05 + 0.5

        });

        canvascontext.fillStyle = COMMON.addAlpha(fillcolor,1);
        canvascontext.fillRect(start.left, start.top, stop.left - start.left, stop.top - start.top);

        canvascontext.strokeStyle = "#ccc"
        canvascontext.beginPath();
        canvascontext.moveTo(start.left, start.top+3);
        canvascontext.lineTo(start.left, start.top-3);
        canvascontext.stroke();

        canvascontext.beginPath();
        canvascontext.moveTo(stop_point.left, stop_point.top+3);
        canvascontext.lineTo(stop_point.left, stop_point.top-3);
        canvascontext.stroke();

        if (bits !== 0){
            var step = length / bits
            for(var pos = 0; pos < length; pos += step){
                var p = plot.pointOffset({
                    x: begin + pos,
                    y: offset + 0.03
                });

                canvascontext.beginPath();
                canvascontext.moveTo(p.left, p.top);
                canvascontext.lineTo(p.left, p.top-3);
                canvascontext.stroke();
            }
        }
        var rate = CLIENT.getValue('LA_CUR_FREQ')
        if (rate !== undefined && show_time){
            var time = 1 / rate * length
            if (time !== 0){
                var text = OSC.convertTimeFromSec(time)
                canvascontext.font = "10px Arial";
                canvascontext.textAlign = "center";
                canvascontext.fillStyle = "#fff";
                if (canvascontext.measureText(text).width * 1.1 < stop.left - start.left){
                    canvascontext.fillText(text, start.left + (stop.left - start.left )/ 2, stop.top - 10);
                }
            }
        }
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

    COMMON.setAnnoSetUART = function(new_params){
        COMMON.decoderAnno['UART'] = JSON.parse(new_params['DECODER_ANNOTATION_UART'].value)
    }

    COMMON.setAnnoSetCAN = function(new_params){
        COMMON.decoderAnno['CAN'] = JSON.parse(new_params['DECODER_ANNOTATION_CAN'].value)
    }

    COMMON.setAnnoSetSPI = function(new_params){
        COMMON.decoderAnno['SPI'] = JSON.parse(new_params['DECODER_ANNOTATION_SPI'].value)
    }

    COMMON.setAnnoSetI2C = function(new_params){
        COMMON.decoderAnno['I2C'] = JSON.parse(new_params['DECODER_ANNOTATION_I2C'].value)
    }

}(window.COMMON = window.COMMON || {}, jQuery));