(function(SPI, $, undefined) {
    SPI.ANNOTATIONS = {
        DATA: 0,
    };

    SPI.data_arr = [];

    SPI.colors = {
        DATA: "#c9c25d",
        NOTHING: "#154473",
    }

    SPI.getAnnotation = function(value){
        return SERIES.decoderAnno.SPI[value]
    }

    SPI.getValue = function(item,radix){
        switch (item.c) {
            case SPI.ANNOTATIONS.DATA:
                return COMMON.formatData(item.d, "", "", radix)[0]
            default:
                break;
        }
        return "Error"
    }

    SPI.drawDecoded = function(plot, canvascontext, offset, decoded_data) {
        var dd = decoded_data;
        for (var i = 0; i < dd.length; i++) {
            if (dd[i].c == SPI.ANNOTATIONS.DATA) {
                SERIES.drawHexagon(plot, canvascontext, offset, dd[i].s, dd[i].l,SPI.colors.DATA, COMMON.formatData(dd[i].d, "", "", COMMON.radix))
                SERIES.drawBitsBars(plot,canvascontext,offset, dd[i].s, dd[i].l,SPI.colors.DATA, dd[i].b)
            }
        }
    }

}(window.SPI = window.SPI || {}, jQuery));