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

//Input data threshold changed
var inputThresholdChange = function(event){
    if ($("#BA_INPUT_THRESHOLD").val() > 1)
        $("#BA_INPUT_THRESHOLD").val(1);
    else if ($("#BA_INPUT_THRESHOLD").val() < 0)
        $("#BA_INPUT_THRESHOLD").val(0);
	BA.input_threshold = parseFloat($("#BA_INPUT_THRESHOLD").val());
	BA.parametersCache["BA_INPUT_THRESHOLD"] = { value: $("#BA_INPUT_THRESHOLD").val() };
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
	BA.parametersCache["BA_GAIN_MIN"] = { value: $("#BA_SIGNAL_1_MIN").val() };
	BA.graphCache.plot.getAxes().yaxis.options.min = parseInt($("#BA_SIGNAL_1_MIN").val() ) * 1;
    BA.graphCache.plot.setupGrid();
    BA.graphCache.plot.draw();
	BA.sendParameters();
}


//Max gain changed
var gainMaxChange = function(event){
	BA.parametersCache["BA_GAIN_MAX"] = { value: $("#BA_SIGNAL_1_MAX").val() };
	BA.graphCache.plot.getAxes().yaxis.options.max = parseInt($("#BA_SIGNAL_1_MAX").val() ) * 1;
	BA.graphCache.plot.setupGrid();
    BA.graphCache.plot.draw();
	BA.sendParameters();
}


//Min phase changed
var phaseMinChange = function(event){
	BA.parametersCache["BA_PHASE_MIN"] = { value: $("#BA_SIGNAL_2_MIN").val() };
	BA.graphCache.plot.getAxes().y2axis.options.min = parseInt($("#BA_SIGNAL_2_MIN").val() ) * 1;
	BA.graphCache.plot.setupGrid();
    BA.graphCache.plot.draw();
	BA.sendParameters();
}


//Max phase changed
var phaseMaxChange = function(event){
	BA.parametersCache["BA_PHASE_MAX"] = { value: $("#BA_SIGNAL_2_MAX").val() };
	BA.graphCache.plot.getAxes().y2axis.options.max = parseInt($("#BA_SIGNAL_2_MAX").val() ) * 1;
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


// Calibration start click
var calibrateClick = function(event){
	if (BA.running)
		return;
    BA.parametersCache["BA_CALIBRATE_START"] = { value: 1 };
    $('#calibration_dialog').modal('hide');
    $('#calibration').show();
    $("#BA_START_FREQ").val(100);
    $("#BA_END_FREQ").val(62500000);
    $("#BA_STEPS").val(500);
    $("#BA_PERIODS_NUMBER").val(8);
    $("#BA_AVERAGING").val(1);
    $('body').removeClass('loaded');
    BA.curGraphScale = BA.scale;
    BA.calibrating = true;
	BA.sendParameters();
}


// Calibration reset click
var calibrateResetClick = function(event){
	if (BA.running)
		return;
    BA.parametersCache["BA_CALIBRATE_RESET"] = { value: 1 };
    $('#calibration_dialog').modal('hide');
	BA.sendParameters();
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
changeCallbacks["BA_SIGNAL_1_MIN"] = gainMinChange;
changeCallbacks["BA_SIGNAL_1_MAX"] = gainMaxChange;
changeCallbacks["BA_SIGNAL_2_MIN"] = phaseMinChange;
changeCallbacks["BA_SIGNAL_2_MAX"] = phaseMaxChange;


var clickCallbacks={}

clickCallbacks["BA_SCALE0"] = scale0Click;
clickCallbacks["BA_SCALE1"] = scale1Click;
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
})
