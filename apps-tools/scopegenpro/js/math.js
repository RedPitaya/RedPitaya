(function(OSC, $, undefined) {

    OSC.chMathOffset = function(new_params) {
        OSC.setGposOffset("MATH")
        OSC.setMathOffsetPlotCh()
        // var params = $.extend(true, {}, OSC.orig.old);
        // var ch_name = "MATH"
        // var param_name = "OSC_"+ ch_name+"_OFFSET";
        // var scale = OSC.params.orig["OSC_"+ch_name+"_SCALE"] ? OSC.params.orig["OSC_"+ch_name+"_SCALE"].value : undefined;
        // var offset = OSC.params.orig['OSC_' + ch_name + '_OFFSET'].value;
        // var ch_name_l = ch_name.toLowerCase();

        // //if ($('#' + ch_name_l + '_offset_arrow').is(':visible') || (OSC.state.graph_grid_height && OSC.state.graph_grid_height !== $('#graph_grid').outerHeight())) {
        //     if (scale !== undefined && offset !== undefined && OSC.state.cursor_dragging === false){
        //         var volt_per_px = (scale * 10) / $('#graph_grid').outerHeight();
        //         var px_offset = -(offset / volt_per_px - parseInt($('#' + ch_name_l + '_offset_arrow').css('margin-top')) / 2);
        //         OSC.state.graph_grid_height = $('#graph_grid').outerHeight();
        //         $('#' + ch_name_l + '_offset_arrow').css('top', ($('#graph_grid').outerHeight() + 7) / 2 + px_offset);
        //     }
        // //}

        // var field = $('#' + param_name);
        // field.val(OSC.formatMathValue(OSC.params.orig[param_name].value));

        // OSC.cursorY();
    }

    OSC.showMathArrow = function(state) {
        if (state){
            $('#math_offset_arrow').show();
        }else{
            $('#math_offset_arrow').hide();
        }
    }

    OSC.mathShow = function(new_params) {
        var param_name = "MATH_SHOW";
        var state = new_params[param_name].value;
        var field = $('#' + param_name);
        if (field.is('button')) {
            field[state === true? 'addClass' : 'removeClass']('active');
        }

        var sig_name = "math";
        if (new_params[param_name].value === true) {
            if (OSC.state.sel_sig_name)
                $('#right_menu .menu-btn.' + OSC.state.sel_sig_name).removeClass('active');
            OSC.state.sel_sig_name = sig_name;

            $('#right_menu .menu-btn.' + OSC.state.sel_sig_name).addClass('active');
            $('.y-offset-arrow').css('z-index', 10);
            $('#' + OSC.state.sel_sig_name + '_offset_arrow').css('z-index', 11);
        } else {
            if (OSC.state.sel_sig_name == sig_name) {
                $('#right_menu .menu-btn.' + OSC.state.sel_sig_name).removeClass('active');
                OSC.state.sel_sig_name = null;
            }
        }

        OSC.showMathArrow(state);
    }

    OSC.mathName = function(new_params) {
        $('#MATH_CHANNEL_NAME_INPUT').val(new_params['MATH_CHANNEL_NAME_INPUT'].value)
        $('.math_channel_name').html(new_params['MATH_CHANNEL_NAME_INPUT'].value)
        OSC.handleMeasureList();
    }


    OSC.formatMathValue = function(oldValue) {
        var z = oldValue;
        var precision = 2;
        var munit = $('#munit').html().charAt(0);
        var scale_val = $("#GPOS_SCALE_MATH").text();
        var math_vdiv = parseFloat(scale_val);
        if (munit == 'm')
            precision = 0;
        if (math_vdiv < 1)
            precision = 3;

        return z.toFixed(precision);
    }

    OSC.convertMathUnitToValue = function(value) {
        var unit = $('#GPOS_OFFSET_MATH_UNIT').html().charAt(0);
        // var precision = 3;
        if (unit === 'm')
            value /= 1000;
        else if (unit === 'M')
            value *= 1000000;
        else if (unit === 'k')
            value *= 1000;
        return value;
    };

    OSC.setMathScale = function() {
        var value = OSC.params.orig["GPOS_SCALE_MATH"] ? OSC.params.orig["GPOS_SCALE_MATH"].value : undefined;
        var value_op = OSC.params.orig["OSC_MATH_OP"] ? OSC.params.orig["OSC_MATH_OP"].value : undefined;

        var unit = 'V';
        OSC.div = 1;
        if (Math.abs(value) <= 0.1) {
            value *= 1000;
            OSC.div = 0.001;
            unit = 'mV';
        } else if (Math.abs(value) >= 1000000) {
            value /= 1000000;
            OSC.div = 1000000;
            unit = 'MV';
        } else if (Math.abs(value) >= 1000) {
            value /= 1000;
            OSC.div = 1000;
            unit = 'kV';
        }
        var field = $('#GPOS_SCALE_MATH');
        if (field != undefined)
            field.html(value);

        if (value_op !== undefined){
            var units = ['', unit, unit, unit + '^2', '', unit, unit + '/s', unit + 's'];
            $('#munit').html(units[value_op] + '/div');
            $('#GPOS_OFFSET_MATH_UNIT').html(units[value_op]);
        }
        OSC.setXYAxisScale()
        OSC.updateTitileXAxisTicksXY()
        OSC.updateTitileYAxisTicksXY()
        OSC.setMathOffsetPlotChLimits()
    }


    OSC.updateMathScale = function(new_params) {
        OSC.setMathScale()
        OSC.chMathOffset()
        OSC.setMathOffsetPlotCh()
        OSC.updateTitileYAxisTicks()
    }

    OSC.updateMathOp = function(new_params,param_name) {
        var field = $('#'+param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.val(new_params[param_name].value);
        }
        OSC.setMathScale()
        OSC.chMathOffset()
        OSC.setMathOffsetPlotCh()
        OSC.handleMeasureList()
    }

    OSC.updateMathSrc1 = function(new_params,param_name) {
        var field = $('#'+param_name);
        if (field.is('input:radio')) {
            var radios = $('input[name="' + param_name + '"]');
            radios.closest('.btn-group').children('.btn.active').removeClass('active');
            radios.eq([+new_params[param_name].value]).prop('checked', true).parent().addClass('active');
        }
    }

    OSC.updateMathSrc2 = function(new_params,param_name) {
        var field = $('#'+param_name);
        if (field.is('input:radio')) {
            var radios = $('input[name="' + param_name + '"]');
            radios.closest('.btn-group').children('.btn.active').removeClass('active');
            radios.eq([+new_params[param_name].value]).prop('checked', true).parent().addClass('active');
        }
    }

    OSC.updateMathShowInverted = function(new_params,param_name){
        var state = OSC.params.orig[param_name].value;
        var field = $('#' + param_name);
        if (field.is('button')) {
            field[state === true? 'addClass' : 'removeClass']('active');
        }
    }

}(window.OSC = window.OSC || {}, jQuery));