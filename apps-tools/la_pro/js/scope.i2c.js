(function(I2C, $, undefined) {
    I2C.ANNOTATIONS = {
        START:          0,
        REPEAT_START:   1,
        STOP:           2,
        ACK:            3,
        NACK:           4,
        READ_ADDRESS:   5,
        WRITE_ADDRESS:  6,
        DATA_READ:      7,
        DATA_WRITE:     8,
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

    I2C.colors = {
        WRITE_ADDRESS: "#FFFF9D",
        READ_ADDRESS: "#FFE11A",
        DATA_READ: "#FFE11A",
        DATA_WRITE: "#FFFF9D",
        ACK: "#BEDB39",
        NACK:"#FD5E00",
        START: "#1DBB6A",
        REPEAT_START: "#0f5e34",
        STOP: "#243cf0"
    };

    I2C.drawDecoded = function( plot, canvascontext, offset, decoded_data) {
        var dd = decoded_data;
        COMMON.appendHead("Time[s], Bus, R/W, Address/Data, Data, ACK/NAK");
        // Remained data after offset
        for (var i = 0; i < dd.length; i++) {

            switch (dd[i].c) {

                case I2C.ANNOTATIONS.START:
                    COMMON.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,I2C.colors.START, "S");
                    COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,I2C.colors.START,dd[i].b)
                    // if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "I2C, Start");
                    break;
                case I2C.ANNOTATIONS.REPEAT_START:
                    COMMON.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,I2C.colors.REPEAT_START, "Sr");
                    COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,I2C.colors.REPEAT_START,dd[i].b)
                    // if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "I2C, Repeat start");
                    break;
                case I2C.ANNOTATIONS.STOP:
                    COMMON.drawCircleTop(plot, canvascontext, offset, dd[i].s, dd[i].l,I2C.colors.STOP, "P");
                    // if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "I2C, Stop");
                    break;
                case I2C.ANNOTATIONS.ACK:
                    COMMON.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,I2C.colors.ACK, "A");
                    COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,I2C.colors.ACK,dd[i].b)
                    // if (OSC.scaleWasChanged) COMMON.appendLog(tmpLastPos, tmpAckNack + ", Ack");
                    break;
                case I2C.ANNOTATIONS.NACK:
                    COMMON.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,I2C.colors.NACK, "!A");
                    COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,I2C.colors.NACK,dd[i].b)
                    // if (OSC.scaleWasChanged) COMMON.appendLog(tmpLastPos, tmpAckNack + ", Nack");
                    break;
                case I2C.ANNOTATIONS.READ_ADDRESS:
                    COMMON.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l, UART.colors.DATA, COMMON.formatData(dd[i].d, "RA: "))
                    COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.DATA,dd[i].b)

                    // tmpAckNack = COMMON.formatExportData(dd[i].data, "I2C, R, Address, ");
                    break;
                case I2C.ANNOTATIONS.WRITE_ADDRESS:
                    COMMON.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l, UART.colors.DATA, COMMON.formatData(dd[i].d, "WA: "))
                    COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.DATA,dd[i].b)
                    // tmpAckNack = COMMON.formatExportData(dd[i].data, "I2C, W, Address, ");
                    break;
                case I2C.ANNOTATIONS.DATA_READ:
                    COMMON.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l, UART.colors.DATA, COMMON.formatData(dd[i].d, "RA: "))
                    COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.DATA,dd[i].b)
                    // tmpAckNack = COMMON.formatExportData(dd[i].data, "I2C, R, Data, ");
                    break;
                case I2C.ANNOTATIONS.DATA_WRITE:
                    COMMON.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l, UART.colors.DATA, COMMON.formatData(dd[i].d, "WD: "))
                    COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.DATA,dd[i].b)
                    // tmpAckNack = COMMON.formatExportData(dd[i].data, "I2C, W, Data, ");
                    break;
                default:
                    break;
            }
        }
    }

}(window.I2C = window.I2C || {}, jQuery));
