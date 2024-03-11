(function(I2C, $, undefined) {
    I2C.ANNOTATIONS = {
        DATA: 0,
        START: 1,
        REPEAT_START: 2,
        STOP: 3,
        ACK: 4,
        NACK: 5,
        READ_ADDRESS: 6,
        WRITE_ADDRESS: 7,
        DATA_READ: 8,
        DATA_WRITE: 9,
        NOTHING: 10,
        NOT_FIT: 0xFF,
    };

    I2C.data_arr = [];

    I2C.colors = [
        "#243cf0", //"#000", // DATA
        "#243cf0", //"#1DBB6A", // START
        "#243cf0", //"#1DBB6A", // REPEAT_START
        "#243cf0", //"#FD5E00", // STOP
        "#243cf0", //"#BEDB39", // ACK
        "#243cf0", //"#FD5E00", // NACK
        "#154473", //"#FFE11A", // RA
        "#154473", //"#FFFF9D", // WA
        "#154473", //"#FFE11A", // DR
        "#154473", //"#FFFF9D", // DW
        "#243cf0", //"#000"
    ];

    I2C.drawDecoded = function(index, plot, canvascontext, series, decoded_data, format) {
        var dd = decoded_data.value;
        var last_pos = -OSC.offsetForDecoded;
        var coef = OSC.time_scale;
        COMMON.appendHead("Time[s], Bus, R/W, Address/Data, Data, ACK/NAK");
        var tmpAckNack = "";
        var tmpLastPos = 0;

        for (var i = 0; i < dd.length; i++) {
            if (dd[i].control != I2C.ANNOTATIONS.NOTHING && dd[i].control != I2C.ANNOTATIONS.DATA) {
                var color = I2C.colors[dd[i].control];
                var textVal = "";
                switch (dd[i].control) {

                    case I2C.ANNOTATIONS.START:
                        textVal = "S";
                        COMMON.drawCircle(index, plot, canvascontext, series, last_pos, (dd[i].length+1) * coef, color, textVal);
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "I2C, Start");
                        break;
                    case I2C.ANNOTATIONS.REPEAT_START:
                        textVal = "Rs";
                        COMMON.drawCircle(index, plot, canvascontext, series, last_pos, (dd[i].length+1) * coef, color, textVal);
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "I2C, Repeat start");
                        break;
                    case I2C.ANNOTATIONS.STOP:
                        textVal = "P";
                        COMMON.drawCircle(index, plot, canvascontext, series, last_pos, (dd[i].length+1) * coef, color, textVal);
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "I2C, Stop");
                        break;
                    case I2C.ANNOTATIONS.ACK:
                        textVal = "A";
                        COMMON.drawCircle(index, plot, canvascontext, series, last_pos, (dd[i].length+1) * coef, color, textVal);
                        if (OSC.scaleWasChanged) COMMON.appendLog(tmpLastPos, tmpAckNack + ", Ack");
                        break;
                    case I2C.ANNOTATIONS.NACK:
                        textVal = "N";
                        COMMON.drawCircle(index, plot, canvascontext, series, last_pos, (dd[i].length+1), color, textVal);
                        if (OSC.scaleWasChanged) COMMON.appendLog(tmpLastPos, tmpAckNack + ", Nack");
                        break;
                    case I2C.ANNOTATIONS.READ_ADDRESS:
                        COMMON.drawHexagon(index, plot, canvascontext, series, last_pos, dd[i].length * coef, color, COMMON.formatData(dd[i].data, "RA: "));
                        tmpAckNack = COMMON.formatExportData(dd[i].data, "I2C, R, Address, ");
                        tmpLastPos = last_pos;
                        break;
                    case I2C.ANNOTATIONS.WRITE_ADDRESS:
                        textVal = "WA: " + dd[i].data;
                        COMMON.drawHexagon(index, plot, canvascontext, series, last_pos, dd[i].length * coef, color, COMMON.formatData(dd[i].data, "WA: "));
                        tmpAckNack = COMMON.formatExportData(dd[i].data, "I2C, W, Address, ");
                        tmpLastPos = last_pos;
                        break;
                    case I2C.ANNOTATIONS.DATA_READ:
                        textVal = "RD: " + dd[i].data;
                        COMMON.drawHexagon(index, plot, canvascontext, series, last_pos, dd[i].length * coef, color, COMMON.formatData(dd[i].data, "RD: "));
                        tmpAckNack = COMMON.formatExportData(dd[i].data, "I2C, R, Data, ");
                        tmpLastPos = last_pos;
                        break;
                    case I2C.ANNOTATIONS.DATA_WRITE:
                        textVal = "WD: " + dd[i].data;
                        COMMON.drawHexagon(index, plot, canvascontext, series, last_pos, dd[i].length * coef, color, COMMON.formatData(dd[i].data, "WD: "));
                        tmpAckNack = COMMON.formatExportData(dd[i].data, "I2C, W, Data, ");
                        tmpLastPos = last_pos;
                        break;
                    case I2C.ANNOTATIONS.NOT_FIT:
                        COMMON.drawHexagon(index, plot, canvascontext, series, last_pos, dd[i].length * coef, color, "");
                        break;
                    default:
                        break;
                }
            }
            last_pos += dd[i].length * coef;
        }
    }

}(window.I2C = window.I2C || {}, jQuery));
