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

    OSC.in1Name = function(new_params) {
        $('#IN1_CHANNEL_NAME_INPUT').val(new_params['IN1_CHANNEL_NAME_INPUT'].value)
        $('.in1_channel_name').html(new_params['IN1_CHANNEL_NAME_INPUT'].value)
        OSC.handleMeasureList();
    }

    OSC.in2Name = function(new_params) {
        $('#IN2_CHANNEL_NAME_INPUT').val(new_params['IN2_CHANNEL_NAME_INPUT'].value)
        $('.in2_channel_name').html(new_params['IN2_CHANNEL_NAME_INPUT'].value)
        OSC.handleMeasureList();
    }

    OSC.in3Name = function(new_params) {
        $('#IN3_CHANNEL_NAME_INPUT').val(new_params['IN3_CHANNEL_NAME_INPUT'].value)
        $('.in3_channel_name').html(new_params['IN3_CHANNEL_NAME_INPUT'].value)
        OSC.handleMeasureList();
    }

    OSC.in4Name = function(new_params) {
        $('#IN4_CHANNEL_NAME_INPUT').val(new_params['IN4_CHANNEL_NAME_INPUT'].value)
        $('.in4_channel_name').html(new_params['IN4_CHANNEL_NAME_INPUT'].value)
        OSC.handleMeasureList();
    }

    OSC.setScale = function(ch,new_params) {
        var param_name = "GPOS_SCALE_"+ch
        var field = $('#' + param_name);

        var inp_units;
        if (Math.abs(new_params[param_name].value) >= 1) {
            inp_units = 'V';
        } else if (Math.abs(new_params[param_name].value) >= 0.001) {
            inp_units = 'mV';
        }
        field.html(OSC.convertVoltage(new_params[param_name].value));
        $("#"+param_name+"_UNIT").html(inp_units)
        if (!OSC.state.trig_dragging)
            OSC.updateTriggerDragHandle()
        OSC.setXYAxisScale()
        OSC.updateTitileXAxisTicksXY()
        OSC.updateTitileYAxisTicksXY()
        OSC.updateTitileYAxisTicks()
    }


    OSC.ch1SetScale = function(new_params) {
        OSC.setScale("CH1",new_params);
        OSC.setGposOffset("CH1")
        OSC.setInOffsetPlotCh("1")
        OSC.setInOffsetZeroPlotCh("1")
    }

    OSC.ch2SetScale = function(new_params) {
        OSC.setScale("CH2",new_params);
        OSC.setGposOffset("CH2")
        OSC.setInOffsetPlotCh("2")
        OSC.setInOffsetZeroPlotCh("2")
    }

    OSC.ch3SetScale = function(new_params) {
        OSC.setScale("CH3",new_params);
        OSC.setGposOffset("CH3")
        OSC.setInOffsetPlotCh("3")
        OSC.setInOffsetZeroPlotCh("3")
    }

    OSC.ch4SetScale = function(new_params) {
        OSC.setScale("CH4",new_params);
        OSC.setGposOffset("CH4")
        OSC.setInOffsetPlotCh("4")
        OSC.setInOffsetZeroPlotCh("4")
    }

    OSC.processSampleRate = function(new_params) {
        var curRate = OSC.adc_max_rate / new_params['OSC_SAMPL_RATE'].value;
        var val = curRate;
        var suf = ""
        if (curRate > 1000000){
            suf = "M"
            val  = (curRate / 1000000).toFixed(3);;

        }else if (curRate > 1000){
            suf = "k";
            val  = (curRate / 1000).toFixed(3);;
        }

        $('#OSC_SAMPL_RATE').html(val + " " + suf+ 'S/s');
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
        OSC.updateTitileXAxisTicks()
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
        OSC.updateOSCOffsetLimits("1");
        OSC.updateOSCOffsetZeroLimits("1")
        OSC.setGposOffset("CH1");
    }

    OSC.ch2SetGain = function(new_params){
        OSC.setGain("OSC_CH2_IN_GAIN")
        OSC.updateOSCOffsetLimits("2");
        OSC.updateOSCOffsetZeroLimits("2")
        OSC.setGposOffset("CH2");
    }

    OSC.ch3SetGain = function(new_params){
        OSC.setGain("OSC_CH3_IN_GAIN")
        OSC.updateOSCOffsetLimits("3");
        OSC.updateOSCOffsetZeroLimits("3")
        OSC.setGposOffset("CH3");
    }

    OSC.ch4SetGain = function(new_params){
        OSC.setGain("OSC_CH4_IN_GAIN")
        OSC.updateOSCOffsetLimits("4");
        OSC.updateOSCOffsetZeroLimits("4")
        OSC.setGposOffset("CH4");
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
        OSC.setInOffsetPlotChLimits(ch)
    }

    OSC.updateOSCOffsetZeroLimits = function(ch){
        OSC.setInOffsetZeroPlotChLimits(ch)
    }

    OSC.setOscProbe = function(param_name){
        var field = $('#' + param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.val(OSC.params.orig[param_name].value);
        }
    }

    OSC.setOscProbe1 = function(new_params){
        OSC.setOscProbe("OSC_CH1_PROBE")
        OSC.updateOSCOffsetLimits("1")
        OSC.updateOSCOffsetZeroLimits("1")
        OSC.setGposOffset("CH1");
        OSC.updateTriggerDragHandle();
    }

    OSC.setOscProbe2 = function(new_params){
        OSC.setOscProbe("OSC_CH2_PROBE")
        OSC.updateOSCOffsetLimits("2")
        OSC.updateOSCOffsetZeroLimits("2")
        OSC.setGposOffset("CH2");
        OSC.updateTriggerDragHandle();
    }

    OSC.setOscProbe3 = function(new_params){
        OSC.setOscProbe("OSC_CH3_PROBE")
        OSC.updateOSCOffsetLimits("3")
        OSC.updateOSCOffsetZeroLimits("3")
        OSC.setGposOffset("CH3");
        OSC.updateTriggerDragHandle();
    }

    OSC.setOscProbe4 = function(new_params){
        OSC.setOscProbe("OSC_CH4_PROBE")
        OSC.updateOSCOffsetLimits("4")
        OSC.updateOSCOffsetZeroLimits("4")
        OSC.setGposOffset("CH4");
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
        var index = ""
        if (param_name.includes("CH1")) index = "1"
        if (param_name.includes("CH2")) index = "2"
        if (param_name.includes("CH3")) index = "3"
        if (param_name.includes("CH4")) index = "4"
        if (index !== ""){
            OSC.updateOSCOffsetLimits(index)
            OSC.updateOSCOffsetZeroLimits(index)
            OSC.setGposOffset("CH"+index);
            OSC.updateTriggerDragHandle();
        }
    }

    OSC.createAxisTicks = function(){
        OSC.createXAxisTicks()
        OSC.createYAxisTicks()
    }

    OSC.createXAxisTicks = function(){
        var graphs = document.getElementById("main");
        for(var i = -5; i <= 5; i++){
            var tick = document.createElement('div');
            tick.id = "xaxis_tick" + (i + 5)
            tick.className = "x_axis_ticks"
            tick.innerText = i;
            graphs.appendChild(tick)
        }
        OSC.moveTitileXAxisTicks()
    }

    OSC.updateTitileXAxisTicks = function(){
        var scale = 0
        if (OSC.params.orig['OSC_TIME_SCALE']){
            scale = OSC.params.orig['OSC_TIME_SCALE'].value * -1
        }
        for(var i = -5; i <= 5; i++){
            var v = OSC.convertTime(i * scale)
            $("#xaxis_tick" + (i + 5)).html(v)
        }
        OSC.moveTitileXAxisTicks()
    }

    OSC.moveTitileXAxisTicks = function(){
        var gh = $('#graphs_holder').height()
        var gw = $('#graphs_holder').width()
        for(var i = -5; i <= 5; i++){
            var ws = $("#xaxis_tick" + (i + 5)).width() / 2
            if (i == -5) ws = 0
            if (i ==  5) ws *= 2
            $("#xaxis_tick" + (i + 5)).css('top',gh + 30).css('left', gw / 2.0 + (gw / 2.0) * i/5.0 + 20 - ws)
        }
    }

    OSC.createYAxisTicks = function(){
        var graphs = document.getElementById("main");
        for(var i = -5; i <= 5; i++){
            var tick = document.createElement('div');
            tick.id = "yaxis_tick" + (i + 5)
            tick.className = "y_axis_ticks rotate"
            tick.innerText = i
            graphs.appendChild(tick)
        }
        OSC.updateTitileYAxisTicks()
    }

    OSC.updateTitileYAxisTicks = function(){
        var itm = OSC.getSettingsActiveChannel()
        if (itm.channel !== ''){
            for(var i = -5; i <= 5; i++){
                var v = OSC.convertVoltageForAxis(i * -itm.scale) + itm.suffix
                $("#yaxis_tick" + (i + 5)).html(v)
            }
        }else{
            for(var i = -5; i <= 5; i++){
                $("#yaxis_tick" + (i + 5)).html('')
            }
        }
        OSC.moveTitileYAxisTicks()
    }

    OSC.moveTitileYAxisTicks = function(){
        var gh = $('#graphs').height() - 8
        for(var i = -5; i <= 5; i++){
            var ws = $("#yaxis_tick" + (i + 5)).height() / 2
            if (i == -5) ws = 0
            if (i ==  5) ws *= 2
            $("#yaxis_tick" + (i + 5)).css('left',2).css('top', gh / 2.0 + (gh / 2.0) * i/5.0 + 33 - ws)
        }
    }

    OSC.setCurrentBuffer = function(new_params){
        if (OSC.params.orig['OSC_BUFFER_CURRENT']){
            var cur = OSC.params.orig['OSC_BUFFER_CURRENT'].value
            $('#buffer_selector_info').html('Current buffer: ' + cur)
            var max = OSC.params.orig['OSC_BUFFER_CURRENT'].max
            var min = OSC.params.orig['OSC_BUFFER_CURRENT'].min
            if (cur == max){
                $('#OSC_NEXT_BUFFER').prop('disabled', true);
            }else{
                $('#OSC_NEXT_BUFFER').prop('disabled', false);
            }

            if (cur == min){
                $('#OSC_PREV_BUFFER').prop('disabled', true);
            }else{
                $('#OSC_PREV_BUFFER').prop('disabled', false);
            }
        }
    }

}(window.OSC = window.OSC || {}, jQuery));