(function(OSC, $, undefined) {

    OSC.formatInputValueMath = function(value, scale) {
        var z = value;
        var round =   Math.log10(scale)
        if (round  <= 0){
            z = parseFloat(z.toFixed(-round))
        }
        return z;
    }

    OSC.convertValueToMathUnit = function(v) {
        var value = v;
        var unit = 'V';
        var precision = 2;
        var munit = $('#GPOS_OFFSET_MATH_UNIT').html().charAt(0);
        var scale_val = OSC.params.orig['GPOS_SCALE_MATH'] ? OSC.params.orig['GPOS_SCALE_MATH'].value : undefined;
        console.log('TEST',OSC.params.orig['GPOS_SCALE_MATH'] )
        if (OSC.params.orig['OSC_MATH_OP']) {
            if (munit == 'm') {
                value *= 1000;
                unit = 'mV';
                precision = 1;
            } else if (munit == 'M') {
                value /= 1000000;
                unit = 'MV';
            } else if (munit == 'k') {
                value /= 1000;
                unit = 'kV';
            }
            // if (scale_val < 1)
            //     precision = 3;

            var units = ['', unit, unit, unit + '^2', '', unit, unit + '/s', unit + 's'];
            $('#GPOS_OFFSET_MATH_UNIT').html(units[OSC.params.orig['OSC_MATH_OP'].value]);
        }
        if (scale_val !== undefined){
            var value_holder = $('#GPOS_OFFSET_MATH');
            value_holder.val(value.toFixed(precision));
            value_holder.attr("step",1.0 / Math.pow(10,precision))
        }
    };

    OSC.setMathOffsetPlotChLimits = function(){
        var param_name_offset = "GPOS_OFFSET_MATH"
        var scale = OSC.params.orig["GPOS_SCALE_MATH"] ? OSC.params.orig["GPOS_SCALE_MATH"].value : undefined;
        if (scale !== undefined){
            if (Math.abs(scale) >= 1000000) {
                scale /= 1000000
            } else if (Math.abs(scale) >= 1000) {
                scale /= 1000
            } else if (Math.abs(scale) < 1) {
                scale *= 1000
            }
            var newMin = -1 * 5 *  scale;
            var newMax = 1 * 5 *   scale;
            $("#"+param_name_offset).attr("min", newMin).attr("max", newMax);
        }
    }

    OSC.setMathOffsetPlotCh = function(){
        var offset = OSC.getMathOffsetPlot()
        if (offset !== undefined){
            OSC.convertValueToMathUnit(offset)
        }
    }

    OSC.getMathOffsetPlot = function(){
        var param_name_offset = "GPOS_OFFSET_MATH"
        return OSC.params.orig[param_name_offset] ? OSC.params.orig[param_name_offset].value : undefined;
    }

}(window.OSC = window.OSC || {}, jQuery));