(function(OSC, $, undefined) {


    OSC.formatInputValueOut = function(value, scale) {
        var z = value;
        var round =   Math.log10(scale)
        if (round  <= 0){
            z = parseFloat(z.toFixed(-round))
        }
        return z;
    }

    // Out offset handlers

    OSC.setOutOffsetPlotCh = function(ch){
        var param_name_offset = "GPOS_OFFSET_OUTPUT" + ch
        var scale = OSC.params.orig["GPOS_SCALE_OUTPUT" + ch] ? OSC.params.orig["GPOS_SCALE_OUTPUT" + ch].value : undefined;
        var offset = OSC.getOutOffsetPlot(ch);

        var inp_field = $('#' + param_name_offset);
        var inp_field_units = $('#' + param_name_offset+"_UNIT");

        var units;
        if (scale !== undefined) {
            if (Math.abs(scale) >= 1) {
                units = 'V';
            } else {
                units = 'mV';
            }
            inp_field_units.text(units)
        } else {
            units = inp_field_units.html();
        }

        if (offset !== undefined){
            var multiplier = units == "mV" ? 1000 : 1;
            var step = units == "mV" ? 0.1 : 0.001;
            inp_field.val(OSC.formatInputValueOut(offset * multiplier, step));
            inp_field.attr("step",step);
        }
    }

    OSC.setOutOffsetPlotChLimits = function(ch){
        var param_name_offset = "GPOS_OFFSET_OUTPUT" + ch
        var scale = OSC.params.orig["GPOS_SCALE_OUTPUT" + ch] ? OSC.params.orig["GPOS_SCALE_OUTPUT" + ch].value : undefined;
        if (scale !== undefined){
            var multiplier = 1
            if (Math.abs(scale) < 1) {
                multiplier = 1000
            }
            var newMin = -1 * 5 *  multiplier * scale;
            var newMax = 1 * 5 *  multiplier * scale;
            $("#"+param_name_offset).attr("min", newMin).attr("max", newMax);
        }
    }

    OSC.getOutOffsetPlot = function(ch){
        var param_name_offset = "GPOS_OFFSET_OUTPUT" + ch
        return OSC.params.orig[param_name_offset] ? OSC.params.orig[param_name_offset].value : undefined;
    }

    OSC.modifyForSendOutOffsetPlot = function(ch,value){
        var divider = 1;
        var scale = OSC.params.orig["GPOS_SCALE_OUTPUT" + ch] ? OSC.params.orig["GPOS_SCALE_OUTPUT" + ch].value : undefined;
        if (scale){
            if (Math.abs(scale) < 1) {
                divider = 1000
            }
        }
        value /= divider;
        return value
    }

    // End out offset handlers

}(window.OSC = window.OSC || {}, jQuery));