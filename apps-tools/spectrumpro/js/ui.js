(function(UI, $, undefined) {

    UI.formatVals = function() {
        // DUTY CYCLE FORMATTING
        {
            var SOUR1_DCYC = parseFloat($("SOUR1_DCYC").val());
            $("SOUR1_DCYC").val(SOUR1_DCYC.toFixed(1));
            var SOUR2_DCYC = parseFloat($("SOUR2_DCYC").val());
            $("SOUR2_DCYC").val(SOUR2_DCYC.toFixed(1));

            var SOUR1_PHAS = parseFloat($("SOUR1_PHAS").val());
            $("SOUR1_PHAS").val(SOUR1_PHAS.toFixed(1));
            var SOUR2_PHAS = parseFloat($("SOUR2_PHAS").val());
            $("SOUR2_PHAS").val(SOUR2_PHAS.toFixed(1));
        }
    }

    $.fn.iLightInputNumber = function(options) {
        var inBox = '.input-number-box';
        var newInput = '.input-number';
        var moreVal = '.input-number-more';
        var lessVal = '.input-number-less';

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

        var value;
        var step;
        var interval = null;
        var timeout = null;

        function ToggleValue(input) {
            input.val(parseInt(input.val(), 10) + d);
            // console.log(input);
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

        $("#ext_con_but").click(function(event) {
            $('#ext_connections_dialog').modal("show");
        });

        $('#reset_settings').click(function() {
            SPEC.params.local['RESET_CONFIG_SETTINGS'] = { value: 1 };
            SPEC.sendParams();
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
            var limits = getLimits(input);
            var min = limits.min;

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
            SPEC.exitEditing(true);
        }

        function checkInputAttr(input) {
            value = parseFloat(input.val());

            if (!($.isNumeric(value))) {
                value = 0;
            }

            if (input.attr('step')) {
                step = parseFloat(input.attr('step'));
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
    }

    UI.initHandlers = function() {
        $(".btn").mouseup(function() {
            setTimeout(function() {
                UI.formatVals();
            }, 20);
        });

        $('input[type=text]:not([readonly]):not(.no-arrows)[step]').iLightInputNumber({
            mobile: false
        });
    }

    UI.updateARBFunc = function(list) {
        const splitLines = value => value.split(/\r?\n/);
        splitLines(list).forEach(function(item){
            var id = item.trim();
            if (id !== ""){
                var name = id.slice(1);
                var opt = document.createElement('option')
                var opt2 = document.createElement('option')
                opt.setAttribute('value', id)
                opt.innerText = name
                opt2.setAttribute('value', id)
                opt2.innerText = name
                var r1 = document.getElementById('SOUR1_FUNC');
                if (r1!= null)
                    r1.appendChild(opt);
                var r2 = document.getElementById('SOUR2_FUNC');
                if (r2!= null)
                    r2.appendChild(opt2);
            }
        });
    }

}(window.UI = window.UI || {}, jQuery));