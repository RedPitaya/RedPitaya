//Callbacks
//Start frequency changed
var startFreqChange = function(event){
    if (parseInt($("#BA_START_FREQ").val()) >= parseInt($("#BA_END_FREQ").val()))
        $("#BA_START_FREQ").val(parseInt($("#BA_END_FREQ").val()) - 1);
    else if (parseInt($("#BA_START_FREQ").val()) < 1)
        $("#BA_START_FREQ").val(1);

    CLIENT.parametersCache["BA_START_FREQ"] = { value: $("#BA_START_FREQ").val() };
	CLIENT.sendParameters();
}


//Stop frequency changed
var endFreqChange = function(event){
    if (parseInt($("#BA_END_FREQ").val()) <= parseInt($("#BA_START_FREQ").val()))
        $("#BA_END_FREQ").val(parseInt($("#BA_START_FREQ").val()) + 1);
    else if (parseInt($("#BA_END_FREQ").val()) > 125e6)
        $("#BA_END_FREQ").val(125e6);

	CLIENT.parametersCache["BA_END_FREQ"] = { value: $("#BA_END_FREQ").val() };
	CLIENT.sendParameters();
}


//Steps changed
var stepsChange = function(event){
    if (parseInt($("#BA_STEPS").val()) < 2)
        $("#BA_STEPS").val(2);
    else if (parseInt($("#BA_STEPS").val()) > (parseInt($("#BA_END_FREQ").val()) - parseInt($("#BA_START_FREQ").val())))
        $("#BA_STEPS").val((parseInt($("#BA_END_FREQ").val()) - parseInt($("#BA_START_FREQ").val())));

	CLIENT.parametersCache["BA_STEPS"] = { value: $("#BA_STEPS").val() };
	CLIENT.sendParameters();
}

//Periods number changed
var periodsNumberChange = function(event){
    if ($("#BA_PERIODS_NUMBER").val() > 24)
        $("#BA_PERIODS_NUMBER").val(24);
    else if ($("#BA_PERIODS_NUMBER").val() < 1)
        $("#BA_PERIODS_NUMBER").val(1);
	CLIENT.parametersCache["BA_PERIODS_NUMBER"] = { value: $("#BA_PERIODS_NUMBER").val() };
	CLIENT.sendParameters();
}

//Input data threshold changed
var inputThresholdChange = function(event){
    if ($("#BA_INPUT_THRESHOLD").val() > 1)
        $("#BA_INPUT_THRESHOLD").val(1);
    else if ($("#BA_INPUT_THRESHOLD").val() < 0)
        $("#BA_INPUT_THRESHOLD").val(0);
	BA.input_threshold = parseFloat($("#BA_INPUT_THRESHOLD").val());
	CLIENT.parametersCache["BA_INPUT_THRESHOLD"] = { value: $("#BA_INPUT_THRESHOLD").val() };
	CLIENT.sendParameters();
}

//Averaging changed
var averChange = function(event){
    if ($("#BA_AVERAGING").val() > 10)
        $("#BA_AVERAGING").val(10);
    else if ($("#BA_AVERAGING").val() < 1)
        $("#BA_AVERAGING").val(1);

	CLIENT.parametersCache["BA_AVERAGING"] = { value: $("#BA_AVERAGING").val() };
	CLIENT.sendParameters();
}


//Amplitude changed
var amplChange = function(event){
	var bias = parseFloat($("#BA_DC_BIAS").val());
    if (parseFloat($("#BA_AMPLITUDE").val()) + Math.abs(bias) > 1.0)
        $("#BA_AMPLITUDE").val(1.0 - Math.abs(bias));
    else if (parseFloat($("#BA_AMPLITUDE").val()) < 0.001)
        $("#BA_AMPLITUDE").val(0);


	if ($("#BA_AMPLITUDE").val().length > 4)
		$("#BA_AMPLITUDE").val(parseFloat($("#BA_AMPLITUDE").val()).toFixed(2));

	CLIENT.parametersCache["BA_AMPLITUDE"] = { value: $("#BA_AMPLITUDE").val() };
	CLIENT.sendParameters();
}


//DC bias changed
var biasDCChange = function(event){
    var bias = parseFloat($("#BA_DC_BIAS").val());
	if (parseFloat($("#BA_AMPLITUDE").val()) + Math.abs(bias) > 1.0){
		if (bias > 0)
			$("#BA_DC_BIAS").val(1.0 - parseFloat($("#BA_AMPLITUDE").val()));
		if (bias < 0)
			$("#BA_DC_BIAS").val(-1.0 + parseFloat($("#BA_AMPLITUDE").val()));
	}

	if ($("#BA_DC_BIAS").val().length > 5)
		$("#BA_DC_BIAS").val(parseFloat($("#BA_DC_BIAS").val()).toFixed(2));

	CLIENT.parametersCache["BA_DC_BIAS"] = { value: $("#BA_DC_BIAS").val() };
	CLIENT.sendParameters();
}

var probeChange = function(event){
	CLIENT.parametersCache["BA_PROBE"] = { value: $("#BA_PROBE").val() };
	CLIENT.sendParameters();
}

//Min gain changed
var gainMinChange = function(event){
	CLIENT.parametersCache["BA_GAIN_MIN"] = { value: $("#BA_GAIN_MIN").val() };
	BA.graphCache.plot.getAxes().yaxis.options.min = parseInt($("#BA_GAIN_MIN").val() ) * 1;
    BA.graphCache.plot.setupGrid();
    BA.graphCache.plot.draw();
	CLIENT.sendParameters();
}


//Max gain changed
var gainMaxChange = function(event){
	CLIENT.parametersCache["BA_GAIN_MAX"] = { value: $("#BA_GAIN_MAX").val() };
	BA.graphCache.plot.getAxes().yaxis.options.max = parseInt($("#BA_GAIN_MAX").val() ) * 1;
	BA.graphCache.plot.setupGrid();
    BA.graphCache.plot.draw();
	CLIENT.sendParameters();
}


//Min phase changed
var phaseMinChange = function(event){
	CLIENT.parametersCache["BA_PHASE_MIN"] = { value: $("#BA_PHASE_MIN").val() };
	BA.graphCache.plot.getAxes().y2axis.options.min = parseInt($("#BA_PHASE_MIN").val() ) * 1;
	BA.graphCache.plot.setupGrid();
    BA.graphCache.plot.draw();
	CLIENT.sendParameters();
}


//Max phase changed
var phaseMaxChange = function(event){
	CLIENT.parametersCache["BA_PHASE_MAX"] = { value: $("#BA_PHASE_MAX").val() };
	BA.graphCache.plot.getAxes().y2axis.options.max = parseInt($("#BA_PHASE_MAX").val() ) * 1;
	BA.graphCache.plot.setupGrid();
    BA.graphCache.plot.draw();
	CLIENT.sendParameters();
}


//Scale button 0 set
var scale0Click = function(event){
	CLIENT.parametersCache["BA_SCALE"] = { value: false };
	CLIENT.sendParameters();
	BA.scale = false;
}


//Scale button 1 set
var scale1Click = function(event){
	CLIENT.parametersCache["BA_SCALE"] = { value: true };
	CLIENT.sendParameters();
	BA.scale = true;
}

//Logic button 0 set
var logic0Click = function(event){
	CLIENT.parametersCache["BA_LOGIC_MODE"] = { value: 0 };
	CLIENT.sendParameters();
}


//Logic button 1 set
var logic1Click = function(event){
	CLIENT.parametersCache["BA_LOGIC_MODE"] = { value: 1 };
	CLIENT.sendParameters();
}


//Gain button 0 set
var gain0Click = function(event){
	CLIENT.parametersCache["BA_IN_GAIN"] = { value: 0 };
	CLIENT.sendParameters();
}


//Gain button 1 set
var gain1Click = function(event){
	CLIENT.parametersCache["BA_IN_GAIN"] = { value: 1 };
	CLIENT.sendParameters();
}

//ACDC button 0 set
var acdc0Click = function(event){
	CLIENT.parametersCache["BA_IN_AC_DC"] = { value: 0 };
	CLIENT.sendParameters();
}


//ACDC button 1 set
var acdc1Click = function(event){
	CLIENT.parametersCache["BA_IN_AC_DC"] = { value: 1 };
	CLIENT.sendParameters();
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
    CLIENT.parametersCache["BA_STATUS"] = { value: 2 };
    CLIENT.sendParameters();
}


// Calibration reset click
var calibrateResetClick = function(event){
	if (BA.running)
		return;
    CLIENT.parametersCache["BA_STATUS"] = { value: 3 };
    $('#calibration_dialog').modal('hide');
	CLIENT.sendParameters();
}


//Create callback
var changeCallbacks={}

changeCallbacks["BA_START_FREQ"] = startFreqChange;
changeCallbacks["BA_END_FREQ"] = endFreqChange;
changeCallbacks["BA_STEPS"] = stepsChange;
changeCallbacks["BA_PERIODS_NUMBER"] = periodsNumberChange;
changeCallbacks["BA_INPUT_THRESHOLD"] = inputThresholdChange;
changeCallbacks["BA_AVERAGING"] = averChange;
changeCallbacks["BA_AMPLITUDE"] = amplChange;
changeCallbacks["BA_DC_BIAS"] = biasDCChange;
changeCallbacks["BA_GAIN_MIN"] = gainMinChange;
changeCallbacks["BA_GAIN_MAX"] = gainMaxChange;
changeCallbacks["BA_PHASE_MIN"] = phaseMinChange;
changeCallbacks["BA_PHASE_MAX"] = phaseMaxChange;
changeCallbacks["BA_PROBE"] = probeChange;


var clickCallbacks={}

clickCallbacks["BA_SCALE0"] = scale0Click;
clickCallbacks["BA_SCALE1"] = scale1Click;
clickCallbacks["BA_LOGIC_MODE0"] = logic0Click;
clickCallbacks["BA_LOGIC_MODE1"] = logic1Click;
clickCallbacks["calib_btn"] = calibrateClick;
clickCallbacks["calib_reset_btn"] = calibrateResetClick;
clickCallbacks["BA_IN_GAIN"] = gain0Click;
clickCallbacks["BA_IN_GAIN1"] = gain1Click;
clickCallbacks["BA_IN_AC_DC"] = acdc0Click;
clickCallbacks["BA_IN_AC_DC1"] = acdc1Click;




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
})
