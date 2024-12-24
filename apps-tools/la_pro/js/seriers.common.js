(function(SERIES, $, undefined) {
    SERIES.decoderAnno = {};
    SERIES.radix = 16;

    SERIES.addAlpha = function(color, opacity) {
        // coerce values so it is between 0 and 1.
        var _opacity = Math.round(Math.min(Math.max(opacity ?? 1, 0), 1) * 255);
        return color + _opacity.toString(16).toUpperCase();
    }

    SERIES.drawHexagon = function(plot, canvascontext, offset, begin, length, fillcolor, data) {

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

        canvascontext.fillStyle = SERIES.addAlpha(fillcolor,0.3);
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

    SERIES.drawTextTop = function(plot, canvascontext, offset, begin, length, fillcolor, data) {

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

    SERIES.drawCircle = function(plot, canvascontext, offset, begin, length, fillcolor, textVal) {

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

    SERIES.drawCircleTop = function(plot, canvascontext, offset, begin, length, fillcolor, textVal) {


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


    SERIES.drawBitsBars = function(plot, canvascontext, offset, begin, length, fillcolor, bits) {


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

        canvascontext.fillStyle = SERIES.addAlpha(fillcolor,1);
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
                var text = COMMON.convertTimeFromSec(time)
                canvascontext.font = "10px Arial";
                canvascontext.textAlign = "center";
                canvascontext.fillStyle = "#fff";
                if (canvascontext.measureText(text).width * 1.1 < stop.left - start.left){
                    canvascontext.fillText(text, start.left + (stop.left - start.left )/ 2, stop.top + 10);
                }
            }
        }
    }

    SERIES.drawBitsBarsTop = function(plot, canvascontext, offset, begin, length, fillcolor, bits, show_time) {

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

        canvascontext.fillStyle = SERIES.addAlpha(fillcolor,1);
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
                var text = COMMON.convertTimeFromSec(time)
                canvascontext.font = "10px Arial";
                canvascontext.textAlign = "center";
                canvascontext.fillStyle = "#fff";
                if (canvascontext.measureText(text).width * 1.1 < stop.left - start.left){
                    canvascontext.fillText(text, start.left + (stop.left - start.left )/ 2, stop.top - 10);
                }
            }
        }
    }

    SERIES.setAnnoSetUART = function(new_params){
        SERIES.decoderAnno['UART'] = JSON.parse(new_params['DECODER_ANNOTATION_UART'].value)
    }

    SERIES.setAnnoSetCAN = function(new_params){
        SERIES.decoderAnno['CAN'] = JSON.parse(new_params['DECODER_ANNOTATION_CAN'].value)
    }

    SERIES.setAnnoSetSPI = function(new_params){
        SERIES.decoderAnno['SPI'] = JSON.parse(new_params['DECODER_ANNOTATION_SPI'].value)
    }

    SERIES.setAnnoSetI2C = function(new_params){
        SERIES.decoderAnno['I2C'] = JSON.parse(new_params['DECODER_ANNOTATION_I2C'].value)
    }

}(window.SERIES = window.SERIES || {}, jQuery));