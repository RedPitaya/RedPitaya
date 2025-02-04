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

}(window.GEN = window.GEN || {}, jQuery));