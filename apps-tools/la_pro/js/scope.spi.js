(function(SPI, $, undefined) {
    SPI.ANNOTATIONS = {
        DATA: 0,
        NOTHING: 10,
    };

    SPI.data_arr = [];

    SPI.colors = [
        "#154473", //"#AEEE00", // DATA
        "#154473", //"#000", // START
    ];

    SPI.drawDecoded = function(index, plot, canvascontext, series, decoded_data, channel) {
        var dd = decoded_data.value;
        var last_pos = -OSC.offsetForDecoded;
        var coef = OSC.time_scale; //OSC.scales[OSC.scale_index];
        var chan = "";

        if (channel == "SPI: MOSI")
            chan = "MOSI, ";
        else if (channel == "SPI: MISO")
            chan = "MISO, ";

        COMMON.appendHead("Time[s], Bus, MISO/MOSI, Data");

        // Remained data after offset
        for (var i = 0; i < dd.length; i++) {
            if (dd[i].control != SPI.ANNOTATIONS.NOTHING) {
                var color = SPI.colors[dd[i].control];
                var txt = (dd[i].control == 0xFF) ? [""] : COMMON.formatData(dd[i].data);
                COMMON.drawHexagon(index, plot, canvascontext, series, last_pos, (dd[i].length + 1) * coef, color, txt);
                if (OSC.scaleWasChanged) COMMON.appendLog(last_pos, COMMON.formatExportData(dd[i].data, "SPI, " + chan));
            }
            last_pos += dd[i].length * coef;
        }
    }

}(window.SPI = window.SPI || {}, jQuery));