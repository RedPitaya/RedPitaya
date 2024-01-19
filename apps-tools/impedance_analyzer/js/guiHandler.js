
(function($) {

    $.fn.iLightInputNumber = function(options) {

        var inBox = '.input-number-box',
            newInput = '.input-number',
            moreVal = '.input-number-more',
            lessVal = '.input-number-less';

        this.each(function() {

            var el = $(this);
            $('<div class="' + inBox.substr(1) + '"></div>').insertAfter(el);
            var parent = el.find('+ ' + inBox);
            parent.append(el);
            var classes = el.attr('class');

            el.addClass(classes);
            var attrValue;


            parent.append('<div class=' + moreVal.substr(1) + '></div>');
            parent.append('<div class=' + lessVal.substr(1) + '></div>');

        }); //end each

        var value,
            step;

        var interval = null,
            timeout = null;

        function ToggleValue(input) {
            input.val(parseInt(input.val(), 10) + d);
            console.log(input);
        }

        $('body').on('mousedown', moreVal, function() {
            var el = $(this);
            var input = el.siblings(newInput);
            moreValFn(input);
            timeout = setTimeout(function() {
                interval = setInterval(function() {
                    moreValFn(input);
                }, 50);
            }, 200);

        });

        $('body').on('mousedown', lessVal, function() {
            var el = $(this);
            var input = el.siblings(newInput);
            lessValFn(input);
            timeout = setTimeout(function() {
                interval = setInterval(function() {
                    lessValFn(input);
                }, 50);
            }, 200);
        });

        $(moreVal + ', ' + lessVal).on("mouseup mouseout", function() {
            clearTimeout(timeout);
            clearInterval(interval);
        });

        function moreValFn(input) {
            var max;
            var limits = getLimits(input);
            max = limits.max;
            checkInputAttr(input);

            var newValue = value + step;
            var parts = step.toString().split('.');
            var signs = parts.length < 2 ? 0 : parts[1].length;
            newValue = parseFloat(newValue.toFixed(signs));

            if (newValue > max) {
                newValue = max;
            }
            changeInputsVal(input, newValue);
        }

        function getLimits(input) {
            var min = parseFloat(input.attr('min'));
            var max = parseFloat(input.attr('max'));
            return {
                'min': min,
                'max': max
            };
        }

        function lessValFn(input) {

            var min;
            var limits = getLimits(input);
            min = limits.min;

            checkInputAttr(input);

            var newValue = value - step;
            var parts = step.toString().split('.');
            var signs = parts.length < 2 ? 0 : parts[1].length;
            newValue = parseFloat(newValue.toFixed(signs));
            if (newValue < min) {
                newValue = min;
            }
            changeInputsVal(input, newValue);
        }

        function changeInputsVal(input, newValue) {
            input.val(newValue);
            input.change();
        }

        function checkInputAttr(input) {

            value = parseFloat(input.val());


            if (!($.isNumeric(value))) {
                value = 0;
            }
            if (input.attr('step')) {
                step = parseFloat(input.attr('step'));
                if (['OSC_CH1_OFFSET', 'OSC_CH2_OFFSET'].indexOf(input.attr('id')) != -1) {

                    var ch = "";
                    if (input.attr('id') == "OSC_CH1_OFFSET")
                        ch = "CH1";
                    else
                        ch = "CH2";

                    var probeAttenuation = parseInt($("#OSC_" + ch + "_PROBE option:selected").text());
                    var jumperSettings = $("#OSC_" + ch + "_IN_GAIN").parent().hasClass("active") ? 1 : 20;
                    var units = $('#OSC_' + ch + '_OFFSET_UNIT').html();
                    var multiplier = units == "mV" ? 1000 : 1;

                    if (multiplier == 1000) {
                        step = 1;
                        return;
                    }
                    if (jumperSettings == 20) {
                        switch (probeAttenuation) {
                            case 1:
                                step = 0.01;
                                return;
                            case 10:
                                step = 0.1;
                                return;
                            case 100:
                                step = 1;
                                return;
                        }
                    } else {
                        switch (probeAttenuation) {
                            case 1:
                                step = 0.001;
                                return;
                            case 10:
                                step = 0.01;
                                return;
                            case 100:
                                step = 0.1;
                        }
                    }
                }
                if (['OSC_MATH_OFFSET'].indexOf(input.attr('id')) != -1) {

                    var unit_holder = $('#OSC_MATH_OFFSET_UNIT');
                    var unit = unit_holder.html().charAt(0);
                    var scale_val = $("#OSC_MATH_SCALE").text();
                    var math_vdiv = parseFloat(scale_val);
                    step = math_vdiv / 100;
                }

            } else {
                step = 1;
            }
        }

        $(newInput).change(function() {

            var input = $(this);

            checkInputAttr(input);
            var limits = getLimits(input);
            var min = limits.min;
            var max = limits.max;

            var parts = step.toString().split('.');
            var signs = parts.length < 2 ? 0 : parts[1].length;
            value = parseFloat(value.toFixed(signs));

            if (value < min) {
                value = min;
            } else if (value > max) {
                value = max;
            }
            if (!($.isNumeric(value))) {
                value = 0;
            }
            input.val(value);
        });

        $(newInput).keydown(function(e) {
            var input = $(this);
            var k = e.keyCode;
            if (k == 38) {
                moreValFn(input);
            } else if (k == 40) {
                lessValFn(input);
            }
        });
    };
})(jQuery);


//Callbacks
//Start frequency changed
var startFreqChange = function(event){
    if (parseInt($("#BA_START_FREQ").val()) >= parseInt($("#BA_END_FREQ").val()))
        $("#BA_START_FREQ").val(parseInt($("#BA_END_FREQ").val()) - 1);
    else if (parseInt($("#BA_START_FREQ").val()) < 1)
        $("#BA_START_FREQ").val(1);

    BA.parametersCache["BA_START_FREQ"] = { value: $("#BA_START_FREQ").val() };
	BA.sendParameters();
}


//Stop frequency changed
var endFreqChange = function(event){
    if (parseInt($("#BA_END_FREQ").val()) <= parseInt($("#BA_START_FREQ").val()))
        $("#BA_END_FREQ").val(parseInt($("#BA_START_FREQ").val()) + 1);
    else if (parseInt($("#BA_END_FREQ").val()) > 125e6)
        $("#BA_END_FREQ").val(125e6);

	BA.parametersCache["BA_END_FREQ"] = { value: $("#BA_END_FREQ").val() };
	BA.sendParameters();
}


//Steps changed
var stepsChange = function(event){
    if (parseInt($("#BA_STEPS").val()) < 2)
        $("#BA_STEPS").val(2);
    else if (parseInt($("#BA_STEPS").val()) > (parseInt($("#BA_END_FREQ").val()) - parseInt($("#BA_START_FREQ").val())))
        $("#BA_STEPS").val((parseInt($("#BA_END_FREQ").val()) - parseInt($("#BA_START_FREQ").val())));

	BA.parametersCache["BA_STEPS"] = { value: $("#BA_STEPS").val() };
	BA.sendParameters();
}

//Periods number changed
var periodsNumberChange = function(event){
    if ($("#BA_PERIODS_NUMBER").val() > 24)
        $("#BA_PERIODS_NUMBER").val(24);
    else if ($("#BA_PERIODS_NUMBER").val() < 1)
        $("#BA_PERIODS_NUMBER").val(1);
	BA.parametersCache["BA_PERIODS_NUMBER"] = { value: $("#BA_PERIODS_NUMBER").val() };
	BA.sendParameters();
}

//Averaging changed
var averChange = function(event){
    if ($("#BA_AVERAGING").val() > 10)
        $("#BA_AVERAGING").val(10);
    else if ($("#BA_AVERAGING").val() < 1)
        $("#BA_AVERAGING").val(1);

	BA.parametersCache["BA_AVERAGING"] = { value: $("#BA_AVERAGING").val() };
	BA.sendParameters();
}


//Amplitude changed
var amplChange = function(event){
    if (parseFloat($("#BA_AMPLITUDE").val()) + parseFloat($("#BA_DC_BIAS").val()) > 1.0)
        $("#BA_AMPLITUDE").val(1.0 - parseFloat($("#BA_DC_BIAS").val()));
    else if (parseFloat($("#BA_AMPLITUDE").val()) < 0.001)
        $("#BA_AMPLITUDE").val(0);


	if ($("#BA_AMPLITUDE").val().length > 4)
		$("#BA_AMPLITUDE").val(parseFloat($("#BA_AMPLITUDE").val()).toFixed(2));

	BA.parametersCache["BA_AMPLITUDE"] = { value: $("#BA_AMPLITUDE").val() };
	BA.sendParameters();
}


//DC bias changed
var biasDCChange = function(event){
    if (parseFloat($("#BA_AMPLITUDE").val()) + Math.abs(parseFloat($("#BA_DC_BIAS").val())) > 1.0)
        $("#BA_DC_BIAS").val(1.0 - parseFloat($("#BA_AMPLITUDE").val()));
    else if (parseFloat($("#BA_DC_BIAS").val()) < -1)
        $("#BA_DC_BIAS").val(-1);

	if ($("#BA_DC_BIAS").val().length > 5)
		$("#BA_DC_BIAS").val(parseFloat($("#BA_DC_BIAS").val()).toFixed(2));

	BA.parametersCache["BA_DC_BIAS"] = { value: $("#BA_DC_BIAS").val() };
	BA.sendParameters();
}


//Min gain changed
var gainMinChange = function(event){
	BA.parametersCache["BA_GAIN_MIN"] = { value: $("#BA_GAIN_MIN").val() };
	BA.graphCache.plot.getAxes().yaxis.options.min = parseInt($("#BA_GAIN_MIN").val() ) * 1;
    BA.graphCache.plot.setupGrid();
    BA.graphCache.plot.draw();
	BA.sendParameters();
}


//Max gain changed
var gainMaxChange = function(event){
	BA.parametersCache["BA_GAIN_MAX"] = { value: $("#BA_GAIN_MAX").val() };
	BA.graphCache.plot.getAxes().yaxis.options.max = parseInt($("#BA_GAIN_MAX").val() ) * 1;
	BA.graphCache.plot.setupGrid();
    BA.graphCache.plot.draw();
	BA.sendParameters();
}


//Min phase changed
var phaseMinChange = function(event){
	BA.parametersCache["BA_PHASE_MIN"] = { value: $("#BA_PHASE_MIN").val() };
	BA.graphCache.plot.getAxes().y2axis.options.min = parseInt($("#BA_PHASE_MIN").val() ) * 1;
	BA.graphCache.plot.setupGrid();
    BA.graphCache.plot.draw();
	BA.sendParameters();
}


//Max phase changed
var phaseMaxChange = function(event){
	BA.parametersCache["BA_PHASE_MAX"] = { value: $("#BA_PHASE_MAX").val() };
	BA.graphCache.plot.getAxes().y2axis.options.max = parseInt($("#BA_PHASE_MAX").val() ) * 1;
	BA.graphCache.plot.setupGrid();
    BA.graphCache.plot.draw();
	BA.sendParameters();
}


//Scale button 0 set
var scale0Click = function(event){
	BA.parametersCache["BA_SCALE"] = { value: false };
	BA.sendParameters();
	BA.scale = false;
}


//Scale button 1 set
var scale1Click = function(event){
	BA.parametersCache["BA_SCALE"] = { value: true };
	BA.sendParameters();
	BA.scale = true;
}

//Logic button 0 set
var logic0Click = function(event){
	BA.parametersCache["BA_LOGIC_MODE"] = { value: 0 };
	BA.sendParameters();
}


//Logic button 1 set
var logic1Click = function(event){
	BA.parametersCache["BA_LOGIC_MODE"] = { value: 1 };
	BA.sendParameters();
}


// Calibration start click
var calibrateClick = function(event){
	if (BA.running)
		return;
    $('#calibration_dialog').modal('hide');
    $('#calibration').show();
    $('body').removeClass('loaded');
    BA.curGraphScale = BA.scale;
    BA.calibrating = true;
    BA.parametersCache["BA_STATUS"] = { value: 2 };
    BA.sendParameters();
}


// Calibration reset click
var calibrateResetClick = function(event){
	if (BA.running)
		return;
    BA.parametersCache["BA_STATUS"] = { value: 3 };
    $('#calibration_dialog').modal('hide');
	BA.sendParameters();
}


//Create callback
var changeCallbacks={}

changeCallbacks["BA_START_FREQ"] = startFreqChange;
changeCallbacks["BA_END_FREQ"] = endFreqChange;
changeCallbacks["BA_STEPS"] = stepsChange;
changeCallbacks["BA_PERIODS_NUMBER"] = periodsNumberChange;
changeCallbacks["BA_AVERAGING"] = averChange;
changeCallbacks["BA_AMPLITUDE"] = amplChange;
changeCallbacks["BA_DC_BIAS"] = biasDCChange;
changeCallbacks["BA_GAIN_MIN"] = gainMinChange;
changeCallbacks["BA_GAIN_MAX"] = gainMaxChange;
changeCallbacks["BA_PHASE_MIN"] = phaseMinChange;
changeCallbacks["BA_PHASE_MAX"] = phaseMaxChange;


var clickCallbacks={}

clickCallbacks["BA_SCALE0"] = scale0Click;
clickCallbacks["BA_SCALE1"] = scale1Click;
clickCallbacks["BA_LOGIC_MODE0"] = logic0Click;
clickCallbacks["BA_LOGIC_MODE1"] = logic1Click;
clickCallbacks["calib_btn"] = calibrateClick;
clickCallbacks["calib_reset_btn"] = calibrateResetClick;




//Subscribe changes and clicks
$(document).ready(function(){
	for (var k in changeCallbacks)
	{
		$("#"+k).change(changeCallbacks[k]);
	}
	for (var i in clickCallbacks)
	{
		$("#"+i).click(clickCallbacks[i]);
	}

    $(".limits").change(function() {
        if (['SOUR1_PHAS', 'SOUR1_DCYC', 'SOUR2_PHAS', 'SOUR2_DCYC'].indexOf($(this).attr('id')) != -1) {
            var min = 0;
            var max = $(this).attr('id').indexOf('DCYC') > 0 ? 100 : 180;

            if (isNaN($(this).val()) || $(this).val() < min)
                $(this).val(min);
            else if ($(this).val() > max)
                $(this).val(max);
        } else {
            var min = $(this).attr('id').indexOf('OFFS') > 0 ? -1 : 0;
            var max = 1;
            if (isNaN($(this).val()) || $(this).val() < min)
                $(this).val(min == -1 ? 0 : 1);
            else if (isNaN($(this).val()) || $(this).val() > max)
                $(this).val(min == -1 ? 0 : 1);
        }
    }).change();

    $('#calib-input').change(function() {
        if (isNaN($(this).val()))
            $(this).val($(this).attr('min'));
        else if ($(this).val() > +$(this).attr('max'))
            $(this).val($(this).attr('max'));
        else if ($(this).val() <= 0)
            $(this).val($(this).attr('min'));
    }).change();

    $('input[type=text]').iLightInputNumber({
        mobile: false
    });
})
