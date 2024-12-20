(function(SPI, $, undefined) {
    SPI.ANNOTATIONS = {
        DATA: 0,
    };

    SPI.data_arr = [];

    SPI.colors = {
        DATA: "#c9c25d", //"#AEEE00", // DATA
        NOTHING: "#154473", //"#000", // START
    }

    SPI.drawDecoded = function(plot, canvascontext, offset, decoded_data) {
        var dd = decoded_data;


        // if (channel == "SPI: MOSI")
        //     chan = "MOSI, ";
        // else if (channel == "SPI: MISO")
        //     chan = "MISO, ";

        // COMMON.appendHead("Time[s], Bus, MISO/MOSI, Data");

        for (var i = 0; i < dd.length; i++) {
            var chan = dd[i].ln.toUpperCase() + ":"
            if (dd[i].c == SPI.ANNOTATIONS.DATA) {
                COMMON.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l,SPI.colors.DATA, COMMON.formatData(dd[i].d))
                COMMON.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,SPI.colors.DATA, dd[i].b)
            }
        }

        // // Remained data after offset
        // for (var i = 0; i < dd.length; i++) {
        //     if (dd[i].control != SPI.ANNOTATIONS.NOTHING) {
        //         var color = SPI.colors[dd[i].control];
        //         var txt = (dd[i].control == 0xFF) ? [""] : COMMON.formatData(dd[i].data);
        //         COMMON.drawHexagon(index, plot, canvascontext, series, last_pos, (dd[i].length + 1) * coef, color, txt);
        //         if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, COMMON.formatExportData(dd[i].data, "SPI, " + chan));
        //     }
        //     last_pos += dd[i].length * coef;
        // }
    }

}(window.SPI = window.SPI || {}, jQuery));