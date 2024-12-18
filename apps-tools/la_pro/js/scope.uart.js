(function(UART, $, undefined) {
    // UART.ANNOTATIONS = {
    //     PARITY_ERR: 2,
    //     START_BIT_ERR: 4,
    //     STOP_BIT_ERR: 8,
    //     NOTHING: 16,
    //     START_BIT: 32,
    //     STOP_BIT: 64,
    //     PARITY_BIT: 128,
    // };

    UART.ANNOTATIONS = {
        NOTHING: 0,
        PARITY_ERR: 1,
        START_BIT_ERR: 2,
        STOP_BIT_ERR: 3,
        DATA: 4,
        START_BIT: 5,
        STOP_BIT: 6,
        PARITY_BIT: 7,
    };

    UART.data_arr = [];

    UART.colors = {
        DATA: "#c9c25d",
        START: "#243cf0",
        STOP: "#243cf0",
        PARITY: "#24baf0",
        WARNING: "#f0e224",
        ERROR: "#ed3f1c",
    };

    UART.drawDecoded = function(plot, canvascontext, offset, decoded_data) {
        var dd = decoded_data;


        COMMON.appendHead("Time[s], Bus, Data");

        // Remained data after offset
        for (var i = 0; i < dd.length; i++) {
            var chan = dd[i].ln.toUpperCase() + ":"
            if (dd[i].c == UART.ANNOTATIONS.START_BIT) {
                COMMON.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,UART.colors.START, "S");
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.START,dd[i].b)
            }

            if (dd[i].c == UART.ANNOTATIONS.START_BIT_ERR) {
                COMMON.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,UART.colors.ERROR, "S!");
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.ERROR,dd[i].b)
            }

            if (dd[i].c == UART.ANNOTATIONS.STOP_BIT) {
                COMMON.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,UART.colors.STOP, "O");
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.STOP,dd[i].b)
            }

            if (dd[i].c == UART.ANNOTATIONS.STOP_BIT_ERR) {
                COMMON.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,UART.colors.ERROR, "O!");
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.ERROR,dd[i].b)
            }

            if (dd[i].c == UART.ANNOTATIONS.PARITY_BIT) {
                COMMON.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,UART.colors.PARITY, "P");
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.PARITY,dd[i].b)
            }

            if (dd[i].c == UART.ANNOTATIONS.PARITY_ERR) {
                COMMON.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,UART.colors.ERROR, "P!");
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.ERROR,dd[i].b)
            }

            if (dd[i].c == UART.ANNOTATIONS.DATA) {
                COMMON.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l, UART.colors.DATA, COMMON.formatData(dd[i].d))
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.DATA,dd[i].b)
            }

            // if (dd[i].control == 0xFF) {
            //     // COMMON.drawHexagon(plot, canvascontext, offset, last_pos, dd[i].length * coef, UART.colors.DATA, "");
            // } else if (!(dd[i].control & UART.ANNOTATIONS.NOTHING)) {
            //     if (dd[i].control & UART.ANNOTATIONS.START_BIT) {
            //         if (dd[i].control & UART.ANNOTATIONS.START_BIT_ERR) {
            //             COMMON.drawCircle(plot, canvascontext, offset, sample, dd[i].length, UART.colors.START, "S!");
            //             if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "UART, " + chan + "Start bit error");
            //         } else {
            //             COMMON.drawCircle(plot, canvascontext, offset, sample, dd[i].length, UART.colors.START, "S");
            //             if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "UART, " + chan + "Start bit");
            //         }
            //     } else if (dd[i].control & UART.ANNOTATIONS.STOP_BIT) {
            //         if (dd[i].control & UART.ANNOTATIONS.STOP_BIT_ERR) {
            //             COMMON.drawCircle(plot, canvascontext, offset, sample, dd[i].length, UART.colors.STOP, "O!");
            //             if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "UART, " + chan + "Stop bit error");
            //         } else {
            //             COMMON.drawCircle(plot, canvascontext, offset, sample, dd[i].length, UART.colors.STOP, "O");
            //             if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "UART, " + chan + "Stop bit");
            //         }
            //     } else if (dd[i].control & UART.ANNOTATIONS.PARITY_BIT) {
            //         if (OSC.buses[bus].parity != 0) {
            //             if (dd[i].control & UART.ANNOTATIONS.PARITY_ERR) {
            //                 COMMON.drawCircle(plot, canvascontext, offset, sample, dd[i].length, UART.colors.PARITY, "P!");
            //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "UART, " + chan + "Parity bit error");
            //             } else {
            //                 COMMON.drawCircle(plot, canvascontext, offset, sample, dd[i].length, UART.colors.PARITY, "P");
            //                 if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "UART, " + chan + "Stop bit error");
            //             }
            //         }
            //     } else {
            //         var val = dd[i].data;
            //        // COMMON.drawHexagon(plot, canvascontext, offset, last_pos, dd[i].length * coef, UART.colors.DATA, COMMON.formatData(val));
            //         if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, COMMON.formatExportData(val, "UART, " + chan));
            //     }
            // }
        }
    }
}(window.UART = window.UART || {}, jQuery));
