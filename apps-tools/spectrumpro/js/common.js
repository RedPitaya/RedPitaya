(function(SPEC, $, undefined) {

    SPEC.convertTimeToText = function(value){
        var x = ""
        if (value > 1000000)
            x = Math.round(value / 1000) / 1000 + " s";
        else if (value > 1000)
            x = value / 1000 + " ms";
        else
            x = value + " µs";
        return x
    }

    SPEC.isVisibleSignal = function(signame) {
        if (signame == 'ch1_view') {
            return 'CH1_SHOW' in SPEC.params.orig && SPEC.params.orig['CH1_SHOW'].value;
        } else if (signame == 'ch2_view') {
            return 'CH2_SHOW' in SPEC.params.orig && SPEC.params.orig['CH2_SHOW'].value;
        } else if (signame == 'ch3_view') {
            return 'CH3_SHOW' in SPEC.params.orig && SPEC.params.orig['CH3_SHOW'].value;
        } else if (signame == 'ch4_view') {
            return 'CH4_SHOW' in SPEC.params.orig && SPEC.params.orig['CH4_SHOW'].value;
        } else if (signame == 'ch1_view_min') {
            return 'CH1_SHOW_MIN' in SPEC.params.orig && SPEC.params.orig['CH1_SHOW_MIN'].value;
        } else if (signame == 'ch2_view_min') {
            return 'CH2_SHOW_MIN' in SPEC.params.orig && SPEC.params.orig['CH2_SHOW_MIN'].value;
        } else if (signame == 'ch3_view_min') {
            return 'CH3_SHOW_MIN' in SPEC.params.orig && SPEC.params.orig['CH3_SHOW_MIN'].value;
        } else if (signame == 'ch4_view_min') {
            return 'CH4_SHOW_MIN' in SPEC.params.orig && SPEC.params.orig['CH4_SHOW_MIN'].value;
        } else if (signame == 'ch1_view_max') {
            return 'CH1_SHOW_MAX' in SPEC.params.orig && SPEC.params.orig['CH1_SHOW_MAX'].value;
        } else if (signame == 'ch2_view_max') {
            return 'CH2_SHOW_MAX' in SPEC.params.orig && SPEC.params.orig['CH2_SHOW_MAX'].value;
        } else if (signame == 'ch3_view_max') {
            return 'CH3_SHOW_MAX' in SPEC.params.orig && SPEC.params.orig['CH3_SHOW_MAX'].value;
        } else if (signame == 'ch4_view_max') {
            return 'CH4_SHOW_MAX' in SPEC.params.orig && SPEC.params.orig['CH4_SHOW_MAX'].value;
        }
    }

    SPEC.visibleCount = function() {
        var count = 0;
        if ('CH1_SHOW' in SPEC.params.orig && SPEC.params.orig['CH1_SHOW'].value){
            count += 1;
        }

        if ('CH2_SHOW' in SPEC.params.orig && SPEC.params.orig['CH2_SHOW'].value){
            count += 1;
        }

        if ('CH3_SHOW' in SPEC.params.orig && SPEC.params.orig['CH3_SHOW'].value){
            count += 1;
        }

        if ('CH4_SHOW' in SPEC.params.orig && SPEC.params.orig['CH4_SHOW'].value){
            count += 1;
        }
        return count;
    }

    SPEC.y_axis_label = function() {
        if (SPEC.config.y_axis_mode == "dbm") return "dBm"
        if (SPEC.config.y_axis_mode == "dbu") return "dBu"
        if (SPEC.config.y_axis_mode == "v")   return "V"
        return ""
    }

    SPEC.y_axis_diff_label = function() {
        if (SPEC.config.y_axis_mode == "dbm") return "dB"
        if (SPEC.config.y_axis_mode == "dbu") return "dB"
        if (SPEC.config.y_axis_mode == "v")   return "V"
        return ""
    }

    SPEC.y_axis_mode = function() {
        if (SPEC.config.y_axis_mode == "dbm") return 0
        if (SPEC.config.y_axis_mode == "v")   return 1
        if (SPEC.config.y_axis_mode == "dbu") return 2
        return ""
    }

    SPEC.processRun = function(new_params) {
        if (new_params['SPEC_RUN'].value === true) {
            $('#SPEC_RUN').hide();
            $('#SPEC_STOP').css('display', 'block');
        } else {
            $('#SPEC_STOP').hide();
            $('#SPEC_RUN').show();
        }
    }

}(window.SPEC = window.SPEC || {}, jQuery));