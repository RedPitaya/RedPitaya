(function(OSC, $, undefined) {



    OSC.chShow = function(ch,new_params) {
        var param_name = ch+"_SHOW";
        var state = new_params[param_name].value;
        var field = $('#' + param_name);
        if (field.is('button')) {
            field[state === true? 'addClass' : 'removeClass']('active');
        }
        OSC.showInArrow(ch,state);
        OSC.cursorY();
        OSC.triggerParam();
    }

    OSC.ch1Show = function(new_params) {
        OSC.chShow("CH1", new_params);
    }

    OSC.ch2Show = function(new_params) {
        OSC.chShow("CH2", new_params);
    }

    OSC.ch3Show = function(new_params) {
        OSC.chShow("CH3", new_params);
    }

    OSC.ch4Show = function(new_params) {
        OSC.chShow("CH4", new_params);
    }

    OSC.setScale = function(ch,new_params) {
        var ch_name_l = ch.toLowerCase();
        var param_name = "OSC_"+ch+"_SCALE"
        var field = $('#' + param_name);

        var inp_units;
        if (Math.abs(new_params[param_name].value) >= 1) {
            inp_units = 'V';
        } else if (Math.abs(new_params[param_name].value) >= 0.001) {
            inp_units = 'mV';
        }
        field.html(OSC.convertVoltage(new_params[param_name].value));
        $("#OSC_"+ch+"_OFFSET_UNIT").html(inp_units)
        if (!OSC.state.trig_dragging)
            OSC.updateTriggerDragHandle()
    }


    OSC.ch1SetScale = function(new_params) {
        OSC.setScale("CH1",new_params);
        OSC.chOffset("CH1")
    }

    OSC.ch2SetScale = function(new_params) {
        OSC.setScale("CH2",new_params);
        OSC.chOffset("CH2")
    }

    OSC.ch3SetScale = function(new_params) {
        OSC.setScale("CH3",new_params);
        OSC.chOffset("CH3")
    }

    OSC.ch4SetScale = function(new_params) {
        OSC.setScale("CH4",new_params);
        OSC.chOffset("CH4")
    }

    OSC.processSampleRate = function(new_params) {
        $('#OSC_SAMPL_RATE').html(OSC.sample_rates[new_params['OSC_SAMPL_RATE'].value] + 'S/s');
    }

    OSC.processTrigInfo = function(new_params) {
        var idx = new_params['OSC_TRIG_INFO'].value;
        var states = ['STOPPED', 'AUTO', 'TRIG\'D', 'WAITING'];
        var colors = ['red', 'green', 'green', 'yellow'];

        $('#triginfo').html(states[idx]);
        $('#triginfo').css('color', colors[idx]);
        $('#triginfo').css('display', '');
    }

    OSC.trigSweep = function(new_params) {
        var radios = $('input[name="OSC_TRIG_SWEEP"]');
        radios.closest('.btn-group').children('.btn.active').removeClass('active');

        if (OSC.params.orig['OSC_TRIG_SWEEP'].value == 0) {
            $('#OSC_TRIG_SWEEP').prop('checked', true)
            $('#OSC_TRIG_SWEEP').parent().addClass('active')
        }

        if (OSC.params.orig['OSC_TRIG_SWEEP'].value == 1) {
            $('#OSC_TRIG_SWEEP1').prop('checked', true)
            $('#OSC_TRIG_SWEEP1').parent().addClass('active')
        }

        if (OSC.params.orig['OSC_TRIG_SWEEP'].value == 2) {
            $('#OSC_TRIG_SWEEP2').prop('checked', true)
            $('#OSC_TRIG_SWEEP2').parent().addClass('active')
        }
    }

    OSC.initOSCHandlers = function(){
        $('#reset_time_offset_button').click(function() {
            OSC.params.local['OSC_TIME_OFFSET'] = { value: 0 };
            OSC.sendParams();
        });

        $('#time_offset_additional_label').change(function() {
            var val = parseFloat($('#time_offset_additional_label').val());
            OSC.params.local['OSC_TIME_OFFSET'] = { value: val };
            OSC.sendParams();
        });
    }

    OSC.setTimeScaleOffset = function(param_name){

        if (param_name === 'OSC_TIME_SCALE'){
            var scale = OSC.params.orig['OSC_TIME_SCALE'].value;
            var field = $('#' + param_name);
            if (field.is('span')) {
                field.html(OSC.convertTime(scale));
            }
        }

        if (param_name === 'OSC_TIME_OFFSET'){
            var scale = OSC.params.orig['OSC_TIME_OFFSET'].value;
            var field = $('#' + param_name);
            if (field.is('span')) {
                field.html(OSC.convertTime(scale));
            }
        }

        if (param_name == 'OSC_TIME_OFFSET' || param_name == 'OSC_TIME_SCALE') {
            if (param_name == 'OSC_TIME_OFFSET')
                OSC.time_offset = OSC.params.orig[param_name].value;
            else
                OSC.time_scale = OSC.params.orig[param_name].value;
            $('#time_offset_additional_label').val(parseFloat(OSC.time_offset));
            $('#trig_out_right').remove();
            $('#trig_out_left').remove();

            if (OSC.time_offset > OSC.time_scale * 5) {
                if ($('#trig_out_left').length == 0)
                    $('.plot').append('<div id="trig_out_left" style="margin-top: 15px; float: left;"><img src="img/trig_out_left.png" /></div>');
                $('#trig_out_right').remove();
            } else if (OSC.time_offset < OSC.time_scale * -5) {
                if ($('#trig_out_right').length == 0)
                    $('.plot').append('<div id="trig_out_right" style="margin-top: 15px; float: right;"><img src="img/trig_out_right.png" /></div>');
                $('#trig_out_left').remove();
            } else {
                $('#trig_out_right').remove();
                $('#trig_out_left').remove();
            }
        }

        OSC.cursorX()
    }

    OSC.setTimeScale = function(new_params){
        OSC.setTimeScaleOffset("OSC_TIME_SCALE")
    }

    OSC.trigSlope = function(new_params) {
        var radios = $('input[name="OSC_TRIG_SLOPE"]');
        radios.closest('.btn-group').children('.btn.active').removeClass('active');

        if (OSC.params.orig['OSC_TRIG_SLOPE'].value == 0) {
            $('#edge1').find('img').attr('src', 'img/edge1.png');
            $('#edge2').addClass('active').find('img').attr('src', 'img/edge2_active.png').end().find('#OSC_TRIG_SLOPE1').prop('checked', true);
        } else {
            $('#edge1').addClass('active').find('img').attr('src', 'img/edge1_active.png').end().find('#OSC_TRIG_SLOPE').prop('checked', true);
            $('#edge2').find('img').attr('src', 'img/edge2.png');
        }

        $('#osc_trig_edge_img').attr('src', (OSC.params.orig['OSC_TRIG_SLOPE'].value == 1 ? 'img/trig-edge-up.png' : 'img/trig-edge-down.png'));
    }

    OSC.setGain = function(param_name){
        var radios = $('input[name="' + param_name + '"]');
        radios.closest('.btn-group').children('.btn.active').removeClass('active');
        radios.eq([+OSC.params.orig[param_name].value]).prop('checked', true).parent().addClass('active');
    }

    OSC.ch1SetGain = function(new_params){
        OSC.setGain("OSC_CH1_IN_GAIN")
        OSC.updateOSCOffsetLimits("CH1");
        OSC.chOffset("CH1");
    }

    OSC.ch2SetGain = function(new_params){
        OSC.setGain("OSC_CH2_IN_GAIN")
        OSC.updateOSCOffsetLimits("CH2");
        OSC.chOffset("CH2");
    }

    OSC.ch3SetGain = function(new_params){
        OSC.setGain("OSC_CH3_IN_GAIN")
        OSC.updateOSCOffsetLimits("CH3");
        OSC.chOffset("CH3");
    }

    OSC.ch4SetGain = function(new_params){
        OSC.setGain("OSC_CH4_IN_GAIN")
        OSC.updateOSCOffsetLimits("CH4");
        OSC.chOffset("CH4");
    }

    OSC.setACDC = function(param_name){
        var radios = $('input[name="' + param_name + '"]');
        radios.closest('.btn-group').children('.btn.active').removeClass('active');
        radios.eq([+OSC.params.orig[param_name].value]).prop('checked', true).parent().addClass('active');
    }

    OSC.ch1SetACDC = function(new_params){
        OSC.setACDC("OSC_CH1_IN_AC_DC")
    }

    OSC.ch2SetACDC = function(new_params){
        OSC.setACDC("OSC_CH2_IN_AC_DC")
    }

    OSC.ch3SetACDC = function(new_params){
        OSC.setACDC("OSC_CH3_IN_AC_DC")
    }

    OSC.ch4SetACDC = function(new_params){
        OSC.setACDC("OSC_CH4_IN_AC_DC")
    }

    OSC.updateOSCOffsetLimits = function(ch){
        var probeAttenuation = parseInt($("#OSC_"+ch+"_PROBE option:selected").text());
        var jumperSettings = $("#OSC_"+ch+"_IN_GAIN").parent().hasClass("active") ? 1 : 20;
        var units = $("#OSC_"+ch+"_OFFSET_UNIT").html();
        var multiplier = units == "mV" ? 1000 : 1;
        var newMin = -1 * 10 * jumperSettings * probeAttenuation * multiplier;
        var newMax = 1 * 10 * jumperSettings * probeAttenuation * multiplier;
        $("#OSC_"+ch+"_OFFSET").attr("min", newMin);
        $("#OSC_"+ch+"_OFFSET").attr("max", newMax);
    }

    OSC.setOscProbe = function(param_name){
        var field = $('#' + param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.val(OSC.params.orig[param_name].value);
        }
    }

    OSC.setOscProbe1 = function(new_params){
        OSC.setOscProbe("OSC_CH1_PROBE")
        OSC.updateOSCOffsetLimits("CH1")
        OSC.chOffset("CH1");
        OSC.updateTriggerDragHandle();
    }

    OSC.setOscProbe2 = function(new_params){
        OSC.setOscProbe("OSC_CH2_PROBE")
        OSC.updateOSCOffsetLimits("CH2")
        OSC.chOffset("CH2");
        OSC.updateTriggerDragHandle();
    }

    OSC.setOscProbe3 = function(new_params){
        OSC.setOscProbe("OSC_CH3_PROBE")
        OSC.updateOSCOffsetLimits("CH3")
        OSC.chOffset("CH3");
        OSC.updateTriggerDragHandle();
    }

    OSC.setOscProbe4 = function(new_params){
        OSC.setOscProbe("OSC_CH4_PROBE")
        OSC.updateOSCOffsetLimits("CH4")
        OSC.chOffset("CH4");
        OSC.updateTriggerDragHandle();
    }

    OSC.trigHyst = function(new_params,param_name){
        var field = $('#' + param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.val(new_params[param_name].value);
        }
    }

    OSC.updateOscShowInverted = function(new_params,param_name){
        var state = OSC.params.orig[param_name].value;
        var field = $('#' + param_name);
        if (field.is('button')) {
            field[state === true? 'addClass' : 'removeClass']('active');
        }
    }

}(window.OSC = window.OSC || {}, jQuery));