(function(OSC, $, undefined) {

    OSC.setInOffsetPlotCh = function(ch){
        var param_name_offset = "GPOS_OFFSET_CH" + ch
        var scale = OSC.params.orig["GPOS_SCALE_CH" + ch] ? OSC.params.orig["GPOS_SCALE_CH" + ch].value : undefined;
        var offset = OSC.getInOffsetPlot(ch);

        var inp_field = $('#' + param_name_offset);
        var inp_field_units = $('#' + param_name_offset+"_UNIT");
        var probeAttenuation = OSC.params.orig['OSC_' + ch + '_PROBE'] !== undefined ? OSC.params.orig['OSC_' + ch + '_PROBE'].value : 1;
        var gain_mode = OSC.params.orig['OSC_' + ch + '_IN_GAIN'] !== undefined ? OSC.params.orig['OSC_' + ch + '_IN_GAIN'].value : 0;

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
            inp_field.val(OSC.formatInputValue(offset * multiplier, probeAttenuation, units == "mV", gain_mode !== 0));
            inp_field.attr("step",OSC.getStepValue(probeAttenuation, units == "mV", gain_mode !== 0));
            OSC.setInOffsetPlotChLimits(ch)
        }
    }

    OSC.setInOffsetPlotChLimits = function(ch){
        var param_name_offset = "GPOS_OFFSET_CH" + ch
        var scale = OSC.params.orig["GPOS_SCALE_CH" + ch] ? OSC.params.orig["GPOS_SCALE_CH" + ch].value : undefined;

        if (scale !== undefined){
            var multiplier = 1
            if (Math.abs(scale) < 1) {
                multiplier = 1000
            }
            var newMin = -1 * 5 * multiplier * scale;
            var newMax = 1 * 5 * multiplier * scale;
            $("#"+param_name_offset).attr("min", newMin).attr("max", newMax);
        }
    }

    OSC.getInOffsetPlot = function(ch){
        var param_name_offset = "GPOS_OFFSET_CH" + ch
        return OSC.params.orig[param_name_offset] ? OSC.params.orig[param_name_offset].value : undefined;
    }

    OSC.modifyForSendInOffsetPlot = function(ch,value){
        var divider = 1;
        var scale = OSC.params.orig["GPOS_SCALE_CH" + ch] ? OSC.params.orig["GPOS_SCALE_CH" + ch].value : undefined;
        if (scale !== undefined) {
            if (Math.abs(scale) < 1) {
                divider = 1000
            }
        }
        value /= divider;
        return value
    }

}(window.OSC = window.OSC || {}, jQuery));