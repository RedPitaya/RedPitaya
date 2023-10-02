(function(OSC, $, undefined) {

OSC.getSetState = function(param_name,ch){
    var state = OSC.params.orig[param_name].value;
    var field = $('#' + param_name);
    if (field.is('button')) {
        field[state === true? 'addClass' : 'removeClass']('active');
    }

    // switch green light for output signals
    if (param_name == "OUTPUT1_STATE" || param_name == "OUTPUT2_STATE") {
        var sig_name = param_name == "OUTPUT1_STATE" ? 'output1' : 'output2';
        if (OSC.params.orig[param_name].value === true) {
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

        var value = OSC.params.orig[param_name].value === true ? 1 : 0;
        if (value == 1) {
            $('#' + param_name + '_ON').show();
            $('#' + param_name + '_ON').closest('.menu-btn').addClass('state-on');
        } else {
            $('#' + param_name + '_ON').hide();
            $('#' + param_name + '_ON').closest('.menu-btn').removeClass('state-on');
        }
    }
    if (param_name === "OUTPUT1_SHOW" || param_name === "OUTPUT2_SHOW")
        OSC.showOutArrow(ch,state)
}

OSC.setGenShow1 = function(new_params){
    var param_name = "OUTPUT1_SHOW"
    OSC.getSetState(param_name,"1")
}

OSC.setGenShow2 = function(new_params){
    var param_name = "OUTPUT2_SHOW"
    OSC.getSetState(param_name,"2")
}

OSC.setGenState1 = function(new_params){
    var param_name = "OUTPUT1_STATE"
    OSC.getSetState(param_name,"1")
}

OSC.setGenState2 = function(new_params){
    var param_name = "OUTPUT2_STATE"
    OSC.getSetState(param_name,"2")
}

OSC.setSourVolt = function(ch){
    var param_name = "SOUR" + ch + "_VOLT"
    var param_name_scale = "OSC_OUTPUT" + ch + "_SCALE"
    var volt = OSC.params.orig[param_name] !== undefined ? OSC.params.orig[param_name].value : undefined
    var scale = OSC.params.orig[param_name_scale] !== undefined ? OSC.params.orig[param_name_scale].value : undefined

    if (volt != undefined){
        var field = $('#' + param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.val(volt);
        }
    }
    if (scale !== undefined){
        $('#SOUR'+ch+'_VOLT_info').html(OSC.convertVoltage(scale));
    }
}

OSC.src1Volt = function(new_params) {
    OSC.setSourVolt("1")
}

OSC.src2Volt = function(new_params) {
    OSC.setSourVolt("2")
}



OSC.sweepResetButton = function(new_params) {
    if ('SOUR1_SWEEP_STATE' in OSC.params.orig){
        OSC.state.sweep_ch1 = OSC.params.orig['SOUR1_SWEEP_STATE'].value
    }
    if ('SOUR2_SWEEP_STATE' in OSC.params.orig){
        OSC.state.sweep_ch2 = OSC.params.orig['SOUR2_SWEEP_STATE'].value
    }
    if (OSC.state.sweep_ch1 || OSC.state.sweep_ch2){
        $(".sweep_button").show();
    }else{
        $(".sweep_button").hide();
    }

    var field = $('#SOUR1_SWEEP_STATE');
    if (field.is('button')) {
        field[OSC.state.sweep_ch1 === true? 'addClass' : 'removeClass']('active');
    }

    field = $('#SOUR2_SWEEP_STATE');
    if (field.is('button')) {
        field[OSC.state.sweep_ch2 === true? 'addClass' : 'removeClass']('active');
    }
}

OSC.burstResetButton = function(new_params) {
    if ('SOUR1_BURST_STATE' in OSC.params.orig){
        OSC.state.burst_ch1 = OSC.params.orig['SOUR1_BURST_STATE'].value
    }
    if ('SOUR2_BURST_STATE' in OSC.params.orig){
        OSC.state.burst_ch2 = OSC.params.orig['SOUR2_BURST_STATE'].value
    }

    var field = $('#SOUR1_BURST_STATE');
    if (field.is('button')) {
        field[OSC.state.burst_ch1 === true? 'addClass' : 'removeClass']('active');
    }

    field = $('#SOUR2_BURST_STATE');
    if (field.is('button')) {
        field[OSC.state.burst_ch2 === true? 'addClass' : 'removeClass']('active');
    }
}

OSC.sweepTime = function(new_params) {
    if ('SOUR1_SWEEP_TIME' in OSC.params.orig){
        OSC.state.sweep_ch1_time = OSC.params.orig['SOUR1_SWEEP_TIME'].value
        if (!$("#SOUR1_SWEEP_TIME").hasClass("focus")){
            $("#SOUR1_SWEEP_TIME").val(OSC.convertTimeToText(OSC.state.sweep_ch1_time));
        }
    }
    if ('SOUR2_SWEEP_TIME' in OSC.params.orig){
        OSC.state.sweep_ch2_time = OSC.params.orig['SOUR2_SWEEP_TIME'].value
        if (!$("#SOUR2_SWEEP_TIME").hasClass("focus")){
            $("#SOUR2_SWEEP_TIME").val(OSC.convertTimeToText(OSC.state.sweep_ch2_time));
        }
    }
}

OSC.ch1SetGenScale = function(new_params){
    OSC.setSourVolt("1")
}

OSC.ch2SetGenScale = function(new_params){
    OSC.setSourVolt("2")
}

OSC.riseFallTime = function(new_params) {
    if('SOUR1_RISE' in OSC.params.orig){
        $("#SOUR1_RISE").val(OSC.params.orig['SOUR1_RISE'].value);
        $("#SOUR1_RISE").attr("min", OSC.params.orig['SOUR1_RISE'].min);
        $("#SOUR1_RISE").attr("max", OSC.params.orig['SOUR1_RISE'].max);
        $("#SOUR1_RISE").attr("step", OSC.params.orig['SOUR1_RISE'].min);
    }
    if('SOUR1_FALL' in OSC.params.orig){
        $("#SOUR1_FALL").val(OSC.params.orig['SOUR1_FALL'].value);
        $("#SOUR1_FALL").attr("min", OSC.params.orig['SOUR1_FALL'].min);
        $("#SOUR1_FALL").attr("max", OSC.params.orig['SOUR1_FALL'].max);
        $("#SOUR1_FALL").attr("step", OSC.params.orig['SOUR1_FALL'].min);
    }
    if('SOUR2_RISE' in OSC.params.orig){
        $("#SOUR2_RISE").val(OSC.params.orig['SOUR2_RISE'].value);
        $("#SOUR2_RISE").attr("min", OSC.params.orig['SOUR2_RISE'].min);
        $("#SOUR2_RISE").attr("max", OSC.params.orig['SOUR2_RISE'].max);
        $("#SOUR2_RISE").attr("step", OSC.params.orig['SOUR2_RISE'].min);
    }
    if('SOUR2_FALL' in OSC.params.orig){
        $("#SOUR2_FALL").val(OSC.params.orig['SOUR2_FALL'].value);
        $("#SOUR2_FALL").attr("min", OSC.params.orig['SOUR2_FALL'].min);
        $("#SOUR2_FALL").attr("max", OSC.params.orig['SOUR2_FALL'].max);
        $("#SOUR2_FALL").attr("step", OSC.params.orig['SOUR2_FALL'].min);
    }
}

OSC.setOutDCyc = function(param_name){
    var field = $('#' + param_name);
    if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
        OSC.params.orig[param_name].value = parseFloat(OSC.params.orig[param_name].value).toFixed(1)
        field.val(OSC.params.orig[param_name].value);
    }
}

OSC.setOutPhase = function(param_name){
    var field = $('#' + param_name);
    if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
        OSC.params.orig[param_name].value = parseFloat(OSC.params.orig[param_name].value).toFixed(0)
        field.val(OSC.params.orig[param_name].value);
    }
}

OSC.setOut1DCyc = function(new_params){
    OSC.setOutDCyc("SOUR1_DCYC")
}

OSC.setOut2DCyc = function(new_params){
    OSC.setOutDCyc("SOUR2_DCYC")
}

OSC.setOut1Phase = function(new_params){
    OSC.setOutPhase("SOUR1_PHAS")
}

OSC.setOut2Phase = function(new_params){
    OSC.setOutPhase("SOUR2_PHAS")
}

OSC.updateGenFreq = function(new_params,param_name){
    var state = OSC.params.orig[param_name].value;
    var field = $('#' + param_name);
    if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
        field.val(new_params[param_name].value);
    }
}

OSC.updateGenSweepStartFreq = function(new_params,param_name){
    var state = OSC.params.orig[param_name].value;
    var field = $('#' + param_name);
    if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
        field.val(new_params[param_name].value);
    }
}

OSC.updateGenSweepEndFreq = function(new_params,param_name){
    var state = OSC.params.orig[param_name].value;
    var field = $('#' + param_name);
    if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
        field.val(new_params[param_name].value);
    }
}

OSC.updateGenSweepMode= function(new_params,param_name){
    var state = OSC.params.orig[param_name].value;
    var field = $('#' + param_name);
    if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
        field.val(new_params[param_name].value);
    }
}

OSC.updateGenSweepFunc= function(new_params,param_name){
    var state = OSC.params.orig[param_name].value;
    var field = $('#' + param_name);
    if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
        field.val(new_params[param_name].value);
    }
}

OSC.updateGenSweepFunc= function(new_params,param_name){
    var state = OSC.params.orig[param_name].value;
    var field = $('#' + param_name);
    if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
        field.val(new_params[param_name].value);
    }
}

OSC.updateGenSweepFunc= function(new_params,param_name){
    var state = OSC.params.orig[param_name].value;
    var field = $('#' + param_name);
    if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
        field.val(new_params[param_name].value);
    }
}

OSC.updateGenTrigSource= function(new_params,param_name){
    var state = OSC.params.orig[param_name].value;
    var field = $('#' + param_name);
    if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
        field.val(new_params[param_name].value);
    }
}

OSC.updateGenBurstCount= function(new_params,param_name){
    var state = OSC.params.orig[param_name].value;
    var field = $('#' + param_name);
    if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
        field.val(new_params[param_name].value);
    }
}

OSC.updateGenBurstRep= function(new_params,param_name){
    var state = OSC.params.orig[param_name].value;
    var field = $('#' + param_name);
    if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
        field.val(new_params[param_name].value);
    }
}

OSC.updateGenBurstDelay= function(new_params,param_name){
    var state = OSC.params.orig[param_name].value;
    var field = $('#' + param_name);
    if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
        field.val(new_params[param_name].value);
    }
}

OSC.updateGenBurstInf = function(new_params,param_name){
    var state = OSC.params.orig[param_name].value;
    var field = $('#' + param_name);
    if (field.is('button')) {
        field[state === true? 'addClass' : 'removeClass']('active');
    }
}

}(window.OSC = window.OSC || {}, jQuery));