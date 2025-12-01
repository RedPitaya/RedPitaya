/*
 * Red Pitaya Spectrum Analizator client
 *
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

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

    SPEC.convertFreqToText = function(value){
        var x = ""
        if (value > 1000000)
            x = Math.round(value / 1000) / 1000 + " MHz";
        else if (value > 1000)
            x = Math.round(value) / 1000 + " kHz";
        else
            x = Math.round(value) + " Hz";
        return x
    }

    SPEC.isVisibleSignal = function(signame) {

        if (signame == 'ch1_view') {
            return CLIENT.getValue('CH1_SHOW') == true
        } else if (signame == 'ch2_view') {
            return CLIENT.getValue('CH2_SHOW') == true
        } else if (signame == 'ch3_view') {
            return CLIENT.getValue('CH3_SHOW') == true
        } else if (signame == 'ch4_view') {
            return CLIENT.getValue('CH4_SHOW') == true
        } else if (signame == 'ch1_view_min') {
            return CLIENT.getValue('CH1_SHOW_MIN') == true && CLIENT.getValue('CH1_SHOW') == true
        } else if (signame == 'ch2_view_min') {
            return CLIENT.getValue('CH2_SHOW_MIN') == true && CLIENT.getValue('CH2_SHOW') == true
        } else if (signame == 'ch3_view_min') {
            return CLIENT.getValue('CH3_SHOW_MIN') == true && CLIENT.getValue('CH3_SHOW') == true
        } else if (signame == 'ch4_view_min') {
            return CLIENT.getValue('CH4_SHOW_MIN') == true && CLIENT.getValue('CH4_SHOW') == true
        } else if (signame == 'ch1_view_max') {
            return CLIENT.getValue('CH1_SHOW_MAX') == true && CLIENT.getValue('CH1_SHOW') == true
        } else if (signame == 'ch2_view_max') {
            return CLIENT.getValue('CH2_SHOW_MAX') == true && CLIENT.getValue('CH2_SHOW') == true
        } else if (signame == 'ch3_view_max') {
            return CLIENT.getValue('CH3_SHOW_MAX') == true && CLIENT.getValue('CH3_SHOW') == true
        } else if (signame == 'ch4_view_max') {
            return CLIENT.getValue('CH4_SHOW_MAX') == true && CLIENT.getValue('CH4_SHOW') == true
        }
    }

    SPEC.isVisibleChannel = function(ch) {
        let ch_name = 'CH'+ch

        return (ch_name + '_SHOW' in CLIENT.params.orig && CLIENT.params.orig[ch_name + '_SHOW'].value) ||
                (ch_name + '_SHOW_MIN' in CLIENT.params.orig && CLIENT.params.orig[ch_name + '_SHOW_MIN'].value) ||
                (ch_name + '_SHOW_MAX' in CLIENT.params.orig && CLIENT.params.orig[ch_name + '_SHOW_MAX'].value)
    }

    SPEC.visibleCount = function() {
        var count = 0;
        if (CLIENT.getValue('CH1_SHOW') == true && SPEC.channelsCount >= 1){
            count += 1;
        }

        if (CLIENT.getValue('CH2_SHOW') == true && SPEC.channelsCount >= 2){
            count += 1;
        }

        if (CLIENT.getValue('CH3_SHOW') == true && SPEC.channelsCount >= 3){
            count += 1;
        }

        if (CLIENT.getValue('CH4_SHOW') == true && SPEC.channelsCount >= 4){
            count += 1;
        }
        return count;
    }

    SPEC.y_axis_label = function() {
        if (SPEC.config.y_axis_mode == "dbuV") return "dBµV"
        if (SPEC.config.y_axis_mode == "dbV") return "dBV"
        if (SPEC.config.y_axis_mode == "dbm") return "dBm"
        if (SPEC.config.y_axis_mode == "dbu") return "dBu"
        if (SPEC.config.y_axis_mode == "v")   return "V"
        if (SPEC.config.y_axis_mode == "mW")   return "mW"
        if (SPEC.config.y_axis_mode == "dBW")   return "dBW"
        return ""
    }


    SPEC.y_axis_label_by_mode = function(mode) {
        if (mode == 0) return "dBm"
        if (mode == 1) return "V"
        if (mode == 2) return "dBu"
        if (mode == 3) return "dBV"
        if (mode == 4) return "dBµV"
        if (mode == 5) return "mW"
        if (mode == 6) return "dBW"
        return ""
    }

    SPEC.y_axis_diff_label = function() {
        if (SPEC.config.y_axis_mode == "dbuV") return "dB"
        if (SPEC.config.y_axis_mode == "dbV") return "dB"
        if (SPEC.config.y_axis_mode == "dbm") return "dB"
        if (SPEC.config.y_axis_mode == "dbu") return "dB"
        if (SPEC.config.y_axis_mode == "v")   return "V"
        if (SPEC.config.y_axis_mode == "mW")   return "mW"
        if (SPEC.config.y_axis_mode == "dBW")   return "dB"
        return ""
    }

    SPEC.y_axis_mode = function() {
        if (SPEC.config.y_axis_mode == "dbm") return 0
        if (SPEC.config.y_axis_mode == "v")   return 1
        if (SPEC.config.y_axis_mode == "dbu") return 2
        if (SPEC.config.y_axis_mode == "dbV") return 3
        if (SPEC.config.y_axis_mode == "dbuV") return 4
        if (SPEC.config.y_axis_mode == "mW")   return 5
        if (SPEC.config.y_axis_mode == "dBW")  return 6
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

    SPEC.getBaseLog = function(x, y) {
        return Math.log(y) / Math.log(x);
    }


    SPEC.downloadDataAsCSV = function(filename,signals) {
        var strings = "";
        var ch1_show = 'ch1_view' in signals && signals['ch1_view'].size > 0 && CLIENT.getValue('CH1_SHOW');
        var ch2_show = 'ch2_view' in signals && signals['ch2_view'].size > 0 && CLIENT.getValue('CH2_SHOW');
        var ch3_show = 'ch3_view' in signals && signals['ch3_view'].size > 0 && CLIENT.getValue('CH3_SHOW');
        var ch4_show = 'ch4_view' in signals && signals['ch4_view'].size > 0 && CLIENT.getValue('CH4_SHOW');

        var ch1_min_show = 'ch1_view_min' in signals && signals['ch1_view_min'].size >0 && ch1_show && CLIENT.getValue('CH1_SHOW_MIN');
        var ch2_min_show = 'ch2_view_min' in signals && signals['ch2_view_min'].size >0 && ch2_show && CLIENT.getValue('CH2_SHOW_MIN');
        var ch3_min_show = 'ch3_view_min' in signals && signals['ch3_view_min'].size >0 && ch3_show && CLIENT.getValue('CH3_SHOW_MIN');
        var ch4_min_show = 'ch4_view_min' in signals && signals['ch4_view_min'].size >0 && ch4_show && CLIENT.getValue('CH4_SHOW_MIN');

        var ch1_max_show = 'ch1_view_max' in signals && signals['ch1_view_max'].size >0 && ch1_show && CLIENT.getValue('CH1_SHOW_MAX');
        var ch2_max_show = 'ch2_view_max' in signals && signals['ch2_view_max'].size >0 && ch2_show && CLIENT.getValue('CH2_SHOW_MAX');
        var ch3_max_show = 'ch3_view_max' in signals && signals['ch3_view_max'].size >0 && ch3_show && CLIENT.getValue('CH3_SHOW_MAX');
        var ch4_max_show = 'ch4_view_max' in signals && signals['ch4_view_max'].size >0 && ch4_show && CLIENT.getValue('CH4_SHOW_MAX');

        if (!ch1_show && !ch2_show && !ch3_show && !ch4_show) return;

        var lens    = signals['ch_xaxis'].size;
        var x_axis  = signals['ch_xaxis'].value;
        var ch1     = ch1_show ? signals['ch1_view'].value : undefined;
        var ch2     = ch2_show ? signals['ch2_view'].value : undefined;
        var ch3     = ch3_show ? signals['ch3_view'].value : undefined;
        var ch4     = ch4_show ? signals['ch4_view'].value : undefined;

        var ch1_min = ch1_min_show ? signals['ch1_view_min'].value : undefined;
        var ch2_min = ch2_min_show ? signals['ch2_view_min'].value : undefined;
        var ch3_min = ch3_min_show ? signals['ch3_view_min'].value : undefined;
        var ch4_min = ch4_min_show ? signals['ch4_view_min'].value : undefined;

        var ch1_max = ch1_max_show ? signals['ch1_view_max'].value : undefined;
        var ch2_max = ch2_max_show ? signals['ch2_view_max'].value : undefined;
        var ch3_max = ch3_max_show ? signals['ch3_view_max'].value : undefined;
        var ch4_max = ch4_max_show ? signals['ch4_view_max'].value : undefined;

        var col_delim = ", ";
        var row_delim = "\n";

        strings += "Frequency";

        if (ch1_show)
            strings += ", IN1";
        if (ch1_min_show)
            strings += ", IN1 min";
        if (ch1_max_show)
            strings += ", IN1 max";
        if (ch2_show)
            strings += ", IN2";
        if (ch2_min_show)
            strings += ", IN2 min";
        if (ch2_max_show)
            strings += ", IN2 max";
        if (ch3_show)
            strings += ", IN3";
        if (ch3_min_show)
            strings += ", IN3 min";
        if (ch3_max_show)
            strings += ", IN3 max";
        if (ch4_show)
            strings += ", IN4";
        if (ch4_min_show)
            strings += ", IN4 min";
        if (ch4_max_show)
            strings += ", IN4 max";
        strings += row_delim;


        let xmin = CLIENT.getValue('xmin')
        let xmax = CLIENT.getValue('xmax')
        for (var i = 0; i < lens; i++) {
            if (x_axis[i] < xmin || x_axis[i] > xmax) continue;
            strings += x_axis[i];

            if (ch1_show)
                strings += col_delim + ch1[i];
            if (ch1_min_show)
                strings += col_delim + ch1_min[i];
            if (ch1_max_show)
                strings += col_delim + ch1_max[i];
            if (ch2_show)
                strings += col_delim + ch2[i];
            if (ch2_min_show)
                strings += col_delim + ch2_min[i];
            if (ch2_max_show)
                strings += col_delim + ch2_max[i];
            if (ch3_show)
                strings += col_delim + ch3[i];
            if (ch3_min_show)
                strings += col_delim + ch3_min[i];
            if (ch3_max_show)
                strings += col_delim + ch3_max[i];
            if (ch4_show)
                strings += col_delim + ch4[i];
            if (ch4_min_show)
                strings += col_delim + ch4_min[i];
            if (ch4_max_show)
                strings += col_delim + ch4_max[i];

            strings += row_delim;
        };

        var link = document.createElement('a');
        link.setAttribute('download', filename);
        link.setAttribute('href', 'data:' + 'text/html' + ';charset=utf-8,' + encodeURIComponent(strings));
        SPEC.fireEvent(link, 'click');
    }



}(window.SPEC = window.SPEC || {}, jQuery));