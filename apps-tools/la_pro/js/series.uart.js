(function(UART, $, undefined) {

    UART.ANNOTATIONS = {
        PARITY_ERR: 0,
        START_BIT_ERR: 1,
        STOP_BIT_ERR: 2,
        DATA: 3,
        START_BIT: 4,
        STOP_BIT: 5,
        PARITY_BIT: 6,
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

    UART.getAnnotation = function(value){
        return SERIES.decoderAnno.UART[value]
    }

    UART.getValue = function(item,radix){
        switch (item.c) {
            case UART.ANNOTATIONS.START_BIT:
                return "Start"
            case UART.ANNOTATIONS.START_BIT_ERR:
                return "Start bit error"
            case UART.ANNOTATIONS.STOP_BIT:
                return "Stop bit"
            case UART.ANNOTATIONS.STOP_BIT_ERR:
                return "Stop bit error"
            case UART.ANNOTATIONS.PARITY_BIT:
                return "Parity bit"
            case UART.ANNOTATIONS.PARITY_ERR:
                return "Parity bit error"
            case UART.ANNOTATIONS.DATA:
                return COMMON.formatData(item.d, "", "", radix)[0]
            default:
                break;
        }
        return "Error"
    }

    UART.drawDecoded = function(plot, canvascontext, offset, decoded_data) {
        var axes = plot.getAxes();
        var min = axes.xaxis.options.min
        var max = axes.xaxis.options.max
        var dd = decoded_data;
        // Remained data after offset
        for (var i = 0; i < dd.length; i++) {
            if (dd[i].s + dd[i].l < min) continue;
            if (dd[i].s > max) continue;
            if (dd[i].c == UART.ANNOTATIONS.START_BIT) {
                SERIES.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,UART.colors.START, "S");
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.START,dd[i].b)
            }

            if (dd[i].c == UART.ANNOTATIONS.START_BIT_ERR) {
                SERIES.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,UART.colors.ERROR, "S!");
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.ERROR,dd[i].b)
            }

            if (dd[i].c == UART.ANNOTATIONS.STOP_BIT) {
                SERIES.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,UART.colors.STOP, "O");
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.STOP,dd[i].b)
            }

            if (dd[i].c == UART.ANNOTATIONS.STOP_BIT_ERR) {
                SERIES.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,UART.colors.ERROR, "O!");
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.ERROR,dd[i].b)
            }

            if (dd[i].c == UART.ANNOTATIONS.PARITY_BIT) {
                SERIES.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,UART.colors.PARITY, "P");
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.PARITY,dd[i].b)
            }

            if (dd[i].c == UART.ANNOTATIONS.PARITY_ERR) {
                SERIES.drawCircle(plot, canvascontext, offset, dd[i].s, dd[i].l,UART.colors.ERROR, "P!");
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.ERROR,dd[i].b)
            }

            if (dd[i].c == UART.ANNOTATIONS.DATA) {
                SERIES.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l, UART.colors.DATA, COMMON.formatData(dd[i].d, "", "", COMMON.radix))
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,UART.colors.DATA,dd[i].b)
            }
        }
    }
}(window.UART = window.UART || {}, jQuery));
