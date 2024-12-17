(function(UART, $, undefined) {
    UART.ANNOTATIONS = {
        PARITY_ERR: 2,
        START_BIT_ERR: 4,
        STOP_BIT_ERR: 8,
        NOTHING: 16,
        START_BIT: 32,
        STOP_BIT: 64,
        PARITY_BIT: 128,
    };

    UART.data_arr = [];

    UART.colors = {
        DATA: "#154473",//"#BEEB9F",
        START: "#243cf0",//"#FFFF9D",
        STOP: "#243cf0",//"#79BD8F",
        PARITY: "#243cf0",//"#00A388",
    };

    UART.drawDecoded = function(plot, canvascontext, offset, decoded_data) {
        var dd = decoded_data;

        var sample = 0;

        COMMON.appendHead("Time[s], Bus, Data");

        // Remained data after offset
        for (var i = 0; i < dd.length; i++) {
            var chan = dd[i].line_name.toUpperCase() + ":"
            if (dd[i].control == 0xFF) {
                // COMMON.drawHexagon(plot, canvascontext, offset, last_pos, dd[i].length * coef, UART.colors.DATA, "");
            } else if (!(dd[i].control & UART.ANNOTATIONS.NOTHING)) {
                if (dd[i].control & UART.ANNOTATIONS.START_BIT) {
                    if (dd[i].control & UART.ANNOTATIONS.START_BIT_ERR) {
                        COMMON.drawCircle(plot, canvascontext, offset, sample, dd[i].length, UART.colors.START, "S!");
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "UART, " + chan + "Start bit error");
                    } else {
                        COMMON.drawCircle(plot, canvascontext, offset, sample, dd[i].length, UART.colors.START, "S");
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "UART, " + chan + "Start bit");
                    }
                } else if (dd[i].control & UART.ANNOTATIONS.STOP_BIT) {
                    if (dd[i].control & UART.ANNOTATIONS.STOP_BIT_ERR) {
                        COMMON.drawCircle(plot, canvascontext, offset, sample, dd[i].length, UART.colors.STOP, "O!");
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "UART, " + chan + "Stop bit error");
                    } else {
                        COMMON.drawCircle(plot, canvascontext, offset, sample, dd[i].length, UART.colors.STOP, "O");
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "UART, " + chan + "Stop bit");
                    }
                } else if (dd[i].control & UART.ANNOTATIONS.PARITY_BIT) {
                    if (OSC.buses[bus].parity != 0) {
                        if (dd[i].control & UART.ANNOTATIONS.PARITY_ERR) {
                            COMMON.drawCircle(plot, canvascontext, offset, sample, dd[i].length, UART.colors.PARITY, "P!");
                            if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "UART, " + chan + "Parity bit error");
                        } else {
                            COMMON.drawCircle(plot, canvascontext, offset, sample, dd[i].length, UART.colors.PARITY, "P");
                            if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "UART, " + chan + "Stop bit error");
                        }
                    }
                } else {
                    var val = dd[i].data;
                   // COMMON.drawHexagon(plot, canvascontext, offset, last_pos, dd[i].length * coef, UART.colors.DATA, COMMON.formatData(val));
                    if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, COMMON.formatExportData(val, "UART, " + chan));
                }
            }
            sample += dd[i].length;
        }
    }
}(window.UART = window.UART || {}, jQuery));
