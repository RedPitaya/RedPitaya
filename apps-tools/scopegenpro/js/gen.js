(function(OSC, $, undefined) {

OSC.showOutArrow = function(ch,state) {
    if (state){
       $('#output' + ch + '_offset_arrow').show();
    }else{
        $('#output' + ch + '_offset_arrow').hide();
    }
}

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

OSC.out1Name = function(new_params) {
    $('#OUT1_CHANNEL_NAME_INPUT').val(new_params['OUT1_CHANNEL_NAME_INPUT'].value)
    $('.out1_channel_name').html(new_params['OUT1_CHANNEL_NAME_INPUT'].value)
}

OSC.out2Name = function(new_params) {
    $('#OUT2_CHANNEL_NAME_INPUT').val(new_params['OUT2_CHANNEL_NAME_INPUT'].value)
    $('.out2_channel_name').html(new_params['OUT2_CHANNEL_NAME_INPUT'].value)
}

OSC.setSourVolt = function(ch){
    var param_name = "SOUR" + ch + "_VOLT"
    var param_name_scale = "GPOS_SCALE_OUTPUT" + ch
    var volt = OSC.params.orig[param_name] !== undefined ? OSC.params.orig[param_name].value : undefined
    var scale = OSC.params.orig[param_name_scale] !== undefined ? OSC.params.orig[param_name_scale].value : undefined

    if (volt != undefined){
        var field = $('#' + param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.val(volt);
        }
    }
    if (scale !== undefined){
        $('#GPOS_SCALE_OUTPUT'+ch).html(OSC.convertVoltage(scale));
    }
    OSC.updateTitileYAxisTicks()

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
    OSC.out1ShowOffset(new_params)
    OSC.setOutOffsetPlotChLimits("1")
    OSC.updateTitileYAxisTicks()
}

OSC.ch2SetGenScale = function(new_params){
    OSC.setSourVolt("2")
    OSC.out2ShowOffset(new_params)
    OSC.setOutOffsetPlotChLimits("2")
    OSC.updateTitileYAxisTicks()
}

OSC.riseFallTime = function(new_params) {

    function update(param) {
        if(param in OSC.params.orig){
            $("#"+param).val(OSC.params.orig[param].value);
            $("#"+param).attr("min", OSC.params.orig[param].min);
            $("#"+param).attr("max", OSC.params.orig[param].max);
            $("#"+param).attr("step", OSC.params.orig[param].min);
        }
    }

    update("SOUR1_RISE")
    update("SOUR1_FALL")
    update("SOUR2_RISE")
    update("SOUR2_FALL")
}

OSC.outExtTrigDeb = function(new_params) {

    function update(param) {
        if(param in OSC.params.orig){
            $("#"+param).val(OSC.params.orig[param].value);
            $("#"+param).attr("min", OSC.params.orig[param].min);
            $("#"+param).attr("max", OSC.params.orig[param].max);
            $("#"+param).attr("step", OSC.params.orig[param].min);
        }
    }

    update("SOUR_DEB")
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

OSC.updateGenFunc= function(new_params,param_name){
    var state = OSC.params.orig[param_name].value;
    var field = $('#' + param_name);
    if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
        field.val(new_params[param_name].value);
    }
}

// OSC.updateGenSweepFunc= function(new_params,param_name){
//     var state = OSC.params.orig[param_name].value;
//     var field = $('#' + param_name);
//     if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
//         field.val(new_params[param_name].value);
//     }
// }

// OSC.updateGenSweepFunc= function(new_params,param_name){
//     var state = OSC.params.orig[param_name].value;
//     var field = $('#' + param_name);
//     if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
//         field.val(new_params[param_name].value);
//     }
// }

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

OSC.updateMaxLimitOnLoadHandler = function(new_params , param_name) {
    if (param_name === "SOUR1_IMPEDANCE"){
        OSC.updateMaxLimitOnLoad("CH1",new_params['SOUR1_IMPEDANCE'].value);
    }

    if (param_name === "SOUR2_IMPEDANCE"){
        OSC.updateMaxLimitOnLoad("CH2",new_params['SOUR2_IMPEDANCE'].value);
    }
    var field = $('#' + param_name);
    if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
        field.val(new_params[param_name].value);
    }
}

OSC.updateMaxLimitOnLoad = function(ch, value) {
    if (OSC.high_z_mode == true) {
        var max_amp = OSC.gen_max_amp;
        if (ch == "CH1") {
            if (value == 0) {
                // Hi-Z mode
                $("#SOUR1_VOLT").attr("max", max_amp);
                $("#SOUR1_VOLT_OFFS").attr("max", max_amp);
                $("#SOUR1_VOLT_OFFS").attr("min", -1 * max_amp);
            }else{
                // 50 omh mode
                $("#SOUR1_VOLT").attr("max", max_amp/ 2.0);
                $("#SOUR1_VOLT_OFFS").attr("max", max_amp / 2.0);
                $("#SOUR1_VOLT_OFFS").attr("min", -1 * max_amp / 2.0);
            }
        }

        if (ch == "CH2") {
            if (value == 0) {
               // Hi-Z mode
               $("#SOUR2_VOLT").attr("max", max_amp);
                $("#SOUR2_VOLT_OFFS").attr("max", max_amp);
                $("#SOUR2_VOLT_OFFS").attr("min", -1 * max_amp);
            }else{
                // 50 omh mode
                $("#SOUR2_VOLT").attr("max", max_amp / 2.0);
                $("#SOUR2_VOLT_OFFS").attr("max", max_amp / 2.0);
                $("#SOUR2_VOLT_OFFS").attr("min", -1 * max_amp / 2.0);
            }

        }
    }else{
        var max_amp = SPEC.gen_max_amp;
        $("#SOUR1_VOLT").attr("max", max_amp);
        $("#SOUR1_VOLT_OFFS").attr("max", max_amp);
        $("#SOUR1_VOLT_OFFS").attr("min", -1 * max_amp);
        $("#SOUR2_VOLT").attr("max", max_amp);
        $("#SOUR2_VOLT_OFFS").attr("max", max_amp);
        $("#SOUR2_VOLT_OFFS").attr("min", -1 * max_amp);
    }
}

}(window.OSC = window.OSC || {}, jQuery));