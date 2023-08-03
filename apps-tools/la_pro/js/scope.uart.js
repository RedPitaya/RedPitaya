(function(UART, $, undefined) {
    UART.ANNOTATIONS = {
        BIT9: 1,
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

    UART.drawDecoded = function(index, plot, canvascontext, series, decoded_data, bus, channel) {
        var dd = decoded_data.value;
        var last_pos = -OSC.counts_offset; //-OSC.offsetForDecoded;
        var coef = OSC.time_scale; //OSC.scales[OSC.scale_index];

        var chan = "";
        if (channel == "UART: RX")
            chan = "RX: ";
        else if (channel == "UART: TX")
            chan = "TX: ";

        COMMON.appendHead("Time[s], Bus, Data");

        // Remained data after offset
        for (var i = 0; i < dd.length; i++) {
            if (dd[i].control == 0xFF) {
                COMMON.drawHexagon(plot, canvascontext, series, last_pos, dd[i].length * coef, UART.colors.DATA, "");
            } else if (!(dd[i].control & UART.ANNOTATIONS.NOTHING)) {
                if (dd[i].control & UART.ANNOTATIONS.START_BIT) {
                    if (dd[i].control & UART.ANNOTATIONS.START_BIT_ERR) {
                        COMMON.drawCircle(index, plot, canvascontext, series, last_pos, dd[i].length * coef, UART.colors.START, "S!");
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "UART, " + chan + "Start bit error");
                    } else {
                        COMMON.drawCircle(index, plot, canvascontext, series, last_pos, dd[i].length * coef, UART.colors.START, "S");
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "UART, " + chan + "Start bit");
                    }
                } else if (dd[i].control & UART.ANNOTATIONS.STOP_BIT) {
                    if (dd[i].control & UART.ANNOTATIONS.STOP_BIT_ERR) {
                        COMMON.drawCircle(index, plot, canvascontext, series, last_pos, dd[i].length * coef, UART.colors.STOP, "O!");
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "UART, " + chan + "Stop bit error");
                    } else {
                        COMMON.drawCircle(index, plot, canvascontext, series, last_pos, dd[i].length * coef, UART.colors.STOP, "O");
                        if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "UART, " + chan + "Stop bit");
                    }
                } else if (dd[i].control & UART.ANNOTATIONS.PARITY_BIT) {
                    if (OSC.buses[bus].parity != 0) {
                        if (dd[i].control & UART.ANNOTATIONS.PARITY_ERR) {
                            COMMON.drawCircle(index, plot, canvascontext, series, last_pos, dd[i].length * coef, UART.colors.PARITY, "P!");
                            if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "UART, " + chan + "Parity bit error");
                        } else {
                            COMMON.drawCircle(index, plot, canvascontext, series, last_pos, dd[i].length * coef, UART.colors.PARITY, "P");
                            if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, "UART, " + chan + "Stop bit error");
                        }
                    }
                } else {
                    var val = 0;
                    if (OSC.buses[bus].num_data_bits == 9) {
                        val = dd[i].control & UART.ANNOTATIONS.BIT9;
                        val = val << 8;
                    }
                    val += dd[i].data;
                    COMMON.drawHexagon(index, plot, canvascontext, series, last_pos, dd[i].length * coef, UART.colors.DATA, COMMON.formatData(val));
                    if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, COMMON.formatExportData(val, "UART, " + chan));
                }
            }
            last_pos += (dd[i].length + 1) * coef;
        }
    }
}(window.UART = window.UART || {}, jQuery));
