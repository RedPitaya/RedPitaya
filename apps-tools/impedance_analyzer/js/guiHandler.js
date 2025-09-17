
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
    if (parseInt($("#IA_START_FREQ").val()) >= parseInt($("#IA_END_FREQ").val()))
        $("#IA_START_FREQ").val(parseInt($("#IA_END_FREQ").val()) - 1);
    else if (parseInt($("#IA_START_FREQ").val()) < 1)
        $("#IA_START_FREQ").val(1);
    stepsChange()
    CLIENT.parametersCache["IA_START_FREQ"] = { value: $("#IA_START_FREQ").val() };
	CLIENT.sendParameters();
}


//Stop frequency changed
var endFreqChange = function(event){
    if (parseInt($("#IA_END_FREQ").val()) <= parseInt($("#IA_START_FREQ").val()))
        $("#IA_END_FREQ").val(parseInt($("#IA_START_FREQ").val()) + 1);
    else if (parseInt($("#BA_END_FREQ").val()) > MAIN.max_adc_rate)
        $("#IA_END_FREQ").val(MAIN.max_adc_rate);
    stepsChange()
    CLIENT.parametersCache["IA_END_FREQ"] = { value: $("#IA_END_FREQ").val() };
    CLIENT.sendParameters();
}


//Steps changed
var stepsChange = function(event){
    if (parseInt($("#IA_STEPS").val()) < 2)
        $("#IA_STEPS").val(2);
    if (parseInt($("#IA_STEPS").val()) > 3000)
        $("#IA_STEPS").val(3000);
    else if (parseInt($("#IA_STEPS").val()) > (parseInt($("#IA_END_FREQ").val()) - parseInt($("#IA_START_FREQ").val())))
        $("#IA_STEPS").val((parseInt($("#IA_END_FREQ").val()) - parseInt($("#IA_START_FREQ").val())));

    CLIENT.parametersCache["IA_STEPS"] = { value: $("#IA_STEPS").val() };
    CLIENT.sendParameters();
}

//Averaging changed
var averChange = function(event){
    if ($("#IA_AVERAGING").val() > 10)
        $("#IA_AVERAGING").val(10);
    else if ($("#IA_AVERAGING").val() < 1)
        $("#IA_AVERAGING").val(1);

    CLIENT.parametersCache["IA_AVERAGING"] = { value: $("#IA_AVERAGING").val() };
    CLIENT.sendParameters();
}


//Amplitude changed
var amplChange = function(event){
    if (parseFloat($("#IA_AMPLITUDE").val()) + parseFloat($("#IA_DC_BIAS").val()) > 1.0)
        $("#IA_AMPLITUDE").val(1.0 - parseFloat($("#IA_DC_BIAS").val()));
    else if (parseFloat($("#IA_AMPLITUDE").val()) < 0.001)
        $("#IA_AMPLITUDE").val(0);


	if ($("#IA_AMPLITUDE").val().length > 4)
		$("#IA_AMPLITUDE").val(parseFloat($("#IA_AMPLITUDE").val()).toFixed(2));

    CLIENT.parametersCache["IA_AMPLITUDE"] = { value: $("#IA_AMPLITUDE").val() };
    CLIENT.sendParameters();
}


//DC bias changed
var biasDCChange = function(event){
    if (parseFloat($("#IA_AMPLITUDE").val()) + Math.abs(parseFloat($("#IA_DC_BIAS").val())) > 1.0)
        $("#IA_DC_BIAS").val(1.0 - parseFloat($("#IA_AMPLITUDE").val()));
    else if (parseFloat($("#IA_DC_BIAS").val()) < -1)
        $("#IA_DC_BIAS").val(-1);

	if ($("#IA_DC_BIAS").val().length > 5)
		$("#IA_DC_BIAS").val(parseFloat($("#IA_DC_BIAS").val()).toFixed(2));

    CLIENT.parametersCache["IA_DC_BIAS"] = { value: $("#IA_DC_BIAS").val() };
    CLIENT.sendParameters();
}


//Scale button 0 set
var scale0Click = function(event){
	CLIENT.parametersCache["IA_SCALE"] = { value: false };
	CLIENT.sendParameters();
}


//Scale button 1 set
var scale1Click = function(event){
	CLIENT.parametersCache["IA_SCALE"] = { value: true };
	CLIENT.sendParameters();
}

var lcrShuntChange = function(event){
    CLIENT.parametersCache["IA_LCR_SHUNT"] = { value: $("#IA_LCR_SHUNT").val() };
    CLIENT.sendParameters();
}

var shuntChange = function(event){
    CLIENT.parametersCache["IA_SHUNT"] = { value: $("#IA_SHUNT").val() };
    CLIENT.sendParameters();
}

var yAxisChange = function(event){
    CLIENT.parametersCache["IA_Y_AXIS"] = { value: $("#IA_Y_AXIS").val() };
    CLIENT.sendParameters();
}

var xAxisChange = function(event){
	CLIENT.parametersCache["IA_X_SCALE"] = { value: $("#IA_X_SCALE").val() };
	CLIENT.sendParameters();
}

//Create callback
var changeCallbacks={}

changeCallbacks["IA_START_FREQ"] = startFreqChange;
changeCallbacks["IA_END_FREQ"] = endFreqChange;
changeCallbacks["IA_STEPS"] = stepsChange;
changeCallbacks["IA_AVERAGING"] = averChange;
changeCallbacks["IA_AMPLITUDE"] = amplChange;
changeCallbacks["IA_DC_BIAS"] = biasDCChange;
changeCallbacks["IA_LCR_SHUNT"] = lcrShuntChange;
changeCallbacks["IA_SHUNT"] = shuntChange;
changeCallbacks["IA_Y_AXIS"] = yAxisChange;
changeCallbacks["IA_X_SCALE"] = xAxisChange;

var clickCallbacks={}

clickCallbacks["IA_SCALE0"] = scale0Click;
clickCallbacks["IA_SCALE1"] = scale1Click;




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

    $('input[type=text]').iLightInputNumber({
        mobile: false
    });

    $('#jtk_up').on('mousedown', function() { $('#jtk_btns').attr('src', 'img/node_up.png'); });
    $('#jtk_left').on('mousedown', function() { $('#jtk_btns').attr('src', 'img/node_left.png'); });
    $('#jtk_right').on('mousedown', function() { $('#jtk_btns').attr('src', 'img/node_right.png'); });
    $('#jtk_down').on('mousedown', function() { $('#jtk_btns').attr('src', 'img/node_down.png'); });
    $('#jtk_fine').on('mousedown', function() { $('#jtk_fine').attr('src', 'img/reset_active.png'); });


    $(document).on('mouseup', function() {
        $('#jtk_btns').attr('src', 'img/node_fine.png');
        $('#jtk_fine').attr('src', 'img/reset.png');
    });

    $('#jtk_fine').on('click', function(ev) {
        if (MAIN.running == true) return;
        MAIN.resetZoom();
    });

    $('#jtk_up, #jtk_down').on('click', function(ev) {
        MAIN.changeYZoom(ev.target.id == 'jtk_down' ? '-' : '+');
    });

    $('#jtk_left, #jtk_right').on('click', function(ev) {
        MAIN.changeXZoom(ev.target.id == 'jtk_left' ? '-' : '+');
    });

     $(document).on('mousedown', '.plot', function(ev) {
        ev.preventDefault();
        if (MAIN.running == true) return;
        if (!MAIN.zoom_used_x && !MAIN.zoom_used_y){
            var rect = MAIN.getPoltRect()
            var newPos = MAIN.boundCursor(rect,{ x: ev.clientX, y: ev.clientY })
            console.log(newPos)
            MAIN.rect_mode = newPos;
            MAIN.rect_mode_last  = newPos;
            return;
        }

        if (!MAIN.move_mode) {
            var rect = MAIN.getPoltRect()
            var newPos = MAIN.boundCursor(rect,{ x: ev.clientX, y: ev.clientY })
            MAIN.move_mode = newPos;
        }
    });


     $(document).on('mousemove', '.plot', function(ev) {
        ev.preventDefault();
        if (MAIN.running == true) return;
        if (!MAIN.move_mode) return;

        var rect = MAIN.getPoltRect()
        var newPos = MAIN.boundCursor(rect,{ x: ev.clientX, y: ev.clientY })

        var x = MAIN.move_mode.x - newPos.x;
        var y = MAIN.move_mode.y - newPos.y;
        var plot = MAIN.getPlot();
        if (plot === undefined) return
        var options = plot.getOptions();
        var range_x   = options.xaxes[0].max - options.xaxes[0].min;
        var range_y   = options.yaxes[0].max - options.yaxes[0].min;
        MAIN.move_mode  = newPos;
        MAIN.changeX(x * range_x / $(this).width(),x, $(this).width());
        MAIN.changeY(y * range_y / $(this).height());
    });

    $(document).on('mousemove',  function(ev) {
        ev.preventDefault();

        if (MAIN.running == true) return;

        var rect = MAIN.getPoltRect()
        var newPos = MAIN.boundCursor(rect,{ x: ev.clientX, y: ev.clientY })

        if (!MAIN.zoom_used_x && !MAIN.zoom_used_y && MAIN.rect_mode != undefined){

            var x = Math.min(MAIN.rect_mode.x,newPos.x);
            var y = Math.min(MAIN.rect_mode.y,newPos.y);
            var w = Math.max(MAIN.rect_mode.x,newPos.x) - x;
            var h = Math.max(MAIN.rect_mode.y,newPos.y) - y;

            MAIN.rect_mode_last  = newPos;

            $("#cur_rectangle").show();
            $("#cur_rectangle").css("left",x)
            $("#cur_rectangle").css("top",y)
            $("#cur_rectangle").css("width",w)
            $("#cur_rectangle").css("height",h)
            return;
        }
    });

    $(document).on('mouseup', '.plot', function(ev) {
        ev.preventDefault();

        if (MAIN.running == true) return;

        MAIN.move_mode = undefined;
        if (MAIN.rect_mode && MAIN.rect_mode_last){
            var rect = MAIN.getPoltRect()
            var p1 =  {x:MAIN.rect_mode.x - rect.l,y:MAIN.rect_mode.y - rect.t}
            var p2 =  {x:MAIN.rect_mode_last.x - rect.l,y:MAIN.rect_mode_last.y - rect.t}
            MAIN.setMouseZoom(p1,p2,rect)
        }
        MAIN.rect_mode_last = undefined;
        MAIN.rect_mode = undefined;
        $("#cur_rectangle").hide();
    });

    $(document).on('mouseup', function(ev) {
        console.log("mouseup")
        ev.preventDefault();
        MAIN.rect_mode_last = undefined;
        MAIN.rect_mode = undefined;
        $("#cur_rectangle").hide();
    });
})
