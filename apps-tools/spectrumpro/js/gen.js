/*
 * Red Pitaya Spectrum Analizator client
 *
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function(GEN, $, undefined) {


    GEN.sour1Imp = function(new_params) {
        var param_name = "SOUR1_IMPEDANCE"
        var field = $('#' + param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.val(new_params[param_name].value);
        }
    }

    GEN.sour2Imp = function(new_params) {
        var param_name = "SOUR2_IMPEDANCE"
        var field = $('#' + param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.val(new_params[param_name].value);
        }
    }

    GEN.sweepResetButton = function(new_params) {
        if ('SOUR1_SWEEP_STATE' in new_params){
            SPEC.state.sweep_ch1 = new_params['SOUR1_SWEEP_STATE'].value
        }
        if ('SOUR2_SWEEP_STATE' in new_params){
            SPEC.state.sweep_ch2 = new_params['SOUR2_SWEEP_STATE'].value
        }
        if (SPEC.state.sweep_ch1 || SPEC.state.sweep_ch2){
            $(".sweep_button").show();
        }else{
            $(".sweep_button").hide();
        }
    }

    GEN.srcVoltOffset = function(new_params,param_name) {
        var value = $('#' + param_name).val();
        if (value !== new_params[param_name].value) {
            $('#' + param_name).val(new_params[param_name].value);
        }
    }

    GEN.srcVolt = function(new_params,param_name) {
        var value = $('#' + param_name).val();
        if (value !== new_params[param_name].value) {
            $('#' + param_name).val(new_params[param_name].value);
        }
    }

    GEN.srcFreq = function(new_params,param_name) {
        var value = $('#' + param_name).val();
        if (value !== new_params[param_name].value) {
            $('#' + param_name).val(new_params[param_name].value);
        }
    }

    GEN.srcPhase = function(new_params,param_name) {
        var value = $('#' + param_name).val();
        if (value !== new_params[param_name].value) {
            $('#' + param_name).val(new_params[param_name].value);
        }
    }

    GEN.srcDCyc = function(new_params,param_name) {
        var value = $('#' + param_name).val();
        if (value !== new_params[param_name].value) {
            $('#' + param_name).val(new_params[param_name].value);
        }
    }

    GEN.srcFunc = function(new_params,param_name) {
        var value = $('#' + param_name).val();
        if (value !== new_params[param_name].value) {
            $('#' + param_name).val(new_params[param_name].value);
        }
    }

    GEN.sweepFreq = function(new_params,param_name) {
        var value = $('#' + param_name).val();
        if (value !== new_params[param_name].value) {
            $('#' + param_name).val(new_params[param_name].value);
        }
    }

    GEN.sweepMode = function(new_params,param_name) {
        var value = $('#' + param_name).val();
        if (value !== new_params[param_name].value) {
            $('#' + param_name).val(new_params[param_name].value);
        }
    }

    GEN.sweepRep = function(new_params,param_name) {
        var value = $('#' + param_name).val();
        if (value !== new_params[param_name].value) {
            $('#' + param_name).val(new_params[param_name].value);
        }
    }

    GEN.sweepDir = function(new_params,param_name) {
        var value = $('#' + param_name).val();
        if (value !== new_params[param_name].value) {
            $('#' + param_name).val(new_params[param_name].value);
        }
    }

    GEN.sweepTime = function(new_params) {
        if ('SOUR1_SWEEP_TIME' in new_params){
            SPEC.state.sweep_ch1_time = new_params['SOUR1_SWEEP_TIME'].value
            if (!$("#SOUR1_SWEEP_TIME").hasClass("focus")){
                $("#SOUR1_SWEEP_TIME").val(SPEC.convertTimeToText(SPEC.state.sweep_ch1_time));
            }
        }
        if ('SOUR2_SWEEP_TIME' in new_params){
            SPEC.state.sweep_ch2_time = new_params['SOUR2_SWEEP_TIME'].value
            if (!$("#SOUR2_SWEEP_TIME").hasClass("focus")){
                $("#SOUR2_SWEEP_TIME").val(SPEC.convertTimeToText(SPEC.state.sweep_ch2_time));
            }
        }
    }

    GEN.sweepInf = function(values,name){
        if (values[name].value == true) {
            $('#'+name)['addClass']('active');
        } else {
            $('#'+name)['removeClass']('active');
        }
    }

    GEN.riseFallTime = function(new_params) {
        if('SOUR1_RISE' in new_params){
            $("#SOUR1_RISE").val(new_params['SOUR1_RISE'].value);
            $("#SOUR1_RISE").attr("min", new_params['SOUR1_RISE'].min);
            $("#SOUR1_RISE").attr("max", new_params['SOUR1_RISE'].max);
            $("#SOUR1_RISE").attr("step", new_params['SOUR1_RISE'].min);
        }
        if('SOUR1_FALL' in new_params){
            $("#SOUR1_FALL").val(new_params['SOUR1_FALL'].value);
            $("#SOUR1_FALL").attr("min", new_params['SOUR1_FALL'].min);
            $("#SOUR1_FALL").attr("max", new_params['SOUR1_FALL'].max);
            $("#SOUR1_FALL").attr("step", new_params['SOUR1_FALL'].min);
        }
        if('SOUR2_RISE' in new_params){
            $("#SOUR2_RISE").val(new_params['SOUR2_RISE'].value);
            $("#SOUR2_RISE").attr("min", new_params['SOUR2_RISE'].min);
            $("#SOUR2_RISE").attr("max", new_params['SOUR2_RISE'].max);
            $("#SOUR2_RISE").attr("step", new_params['SOUR2_RISE'].min);
        }
        if('SOUR2_FALL' in new_params){
            $("#SOUR2_FALL").val(new_params['SOUR2_FALL'].value);
            $("#SOUR2_FALL").attr("min", new_params['SOUR2_FALL'].min);
            $("#SOUR2_FALL").attr("max", new_params['SOUR2_FALL'].max);
            $("#SOUR2_FALL").attr("step", new_params['SOUR2_FALL'].min);
        }
    }

}(window.GEN = window.GEN || {}, jQuery));