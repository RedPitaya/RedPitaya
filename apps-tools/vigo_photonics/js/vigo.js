/*
 * VIGO photonics detector read-out
 *
 * Controller panels: everything the ptcc_control service publishes arrives as
 * an ordinary parameter, so settings are bound by id the way the oscilloscope
 * binds its own. This file only formats the readings, keeps the trend of the
 * two values worth watching over time, and hides what the attached module
 * cannot do.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

(function(OSC, $, undefined) {
    'use strict';

    // Digits are chosen per quantity: a setpoint is meaningful to 1 mK, a
    // supply current is not worth more than a milliamp.
    var READOUTS = {
        PTCC_T_DET: 3,
        PTCC_T_INT: 1,
        PTCC_I_TEC: 3,
        PTCC_U_TEC: 3,
        PTCC_U_SUP_P: 2,
        PTCC_U_SUP_N: 2,
        PTCC_I_SUP_P: 3,
        PTCC_I_SUP_N: 3,
        PTCC_I_FAN: 3,
        PTCC_TH_RES: 0,
        PTCC_PWM: 0,
        PTCC_LABM_U_DET: 3,
        PTCC_LABM_U_1ST: 3,
        PTCC_LABM_U_OUT: 3,
        PTCC_LABM_TEMP: 1,
        PTCC_LABM_U_SUP_P: 2,
        PTCC_LABM_U_SUP_N: 2,
        PTCC_LABM_I_TEC_P: 3,
        PTCC_LABM_I_TEC_N: 3,
        PTCC_LABM_U_FAN: 2,
        PTCC_LABM_TH1: 3,
        PTCC_LABM_TH2: 3
    };

    // Free text shown as it arrives.
    var TEXTS = ['PTCC_STATUS_TEXT', 'PTCC_LAST_ERROR', 'PTCC_MOD_NAME', 'PTCC_DET_NAME'];

    // Input fields bounded by what the module reports it accepts.
    var LIMITS = {
        PTCC_SETPOINT: ['PTCC_SETPOINT_MIN', 'PTCC_SETPOINT_MAX'],
        PTCC_I_TEC_MAX: ['PTCC_I_TEC_MAX_MIN', 'PTCC_I_TEC_MAX_MAX'],
        PTCC_SUP_U_P: ['PTCC_SUP_U_P_MIN', 'PTCC_SUP_U_P_MAX'],
        PTCC_SUP_U_N: ['PTCC_SUP_U_N_MIN', 'PTCC_SUP_U_N_MAX'],
        PTCC_LABM_BIAS_U: ['PTCC_LABM_BIAS_U_MIN', 'PTCC_LABM_BIAS_U_MAX'],
        PTCC_LABM_BIAS_I: ['PTCC_LABM_BIAS_I_MIN', 'PTCC_LABM_BIAS_I_MAX'],
        PTCC_LABM_OFFSET: ['PTCC_LABM_OFFSET_MIN', 'PTCC_LABM_OFFSET_MAX'],
        PTCC_LABM_VARACTOR: ['PTCC_LABM_VARACTOR_MIN', 'PTCC_LABM_VARACTOR_MAX'],
        PTCC_PWM_SET: ['PTCC_PWM_MIN', 'PTCC_PWM_MAX']
    };

    // What a field may be set to. A value the module stores as one of 256
    // codes moves in steps of its own range, which the module reports; the
    // others carry the resolution the protocol gives them.
    var FIELDS = {
        PTCC_SETPOINT: { step: 0.001 },
        PTCC_I_TEC_MAX: { step: 0.01 },
        PTCC_SUP_U_P: { step: 0.1 },
        PTCC_SUP_U_N: { step: 0.1 },
        PTCC_PWM_SET: { step: 1 },
        PTCC_LABM_VARACTOR: { step: 1 },
        PTCC_LABM_BIAS_U: { coded: true },
        PTCC_LABM_BIAS_I: { coded: true },
        PTCC_LABM_OFFSET: { coded: true }
    };

    // Nothing is shown or stepped finer than this: a millivolt, a milliamp,
    // a millikelvin. The module codes its values in 256 steps of its range,
    // which is coarser than that for every quantity it has.
    var FINEST_STEP = 0.001;

    /** Decimals a step itself is written with, so a reading is shown on the
     *  grid it moves on: a step of 0.039 needs three, not the two its
     *  magnitude alone would suggest. */
    function digitsFor(step) {
        var digits = 0;
        var value = step;
        while (digits < 3 && Math.abs(value - Math.round(value)) > 1e-9) {
            value *= 10;
            digits++;
        }
        return digits;
    }

    // A strip chart, the way the calibration application draws its own: a
    // fixed number of slots, a sample per slot, the newest at the right edge.
    // The trace slides left as samples arrive instead of being stretched over
    // whatever time it happens to cover. At the pace a controller with a
    // detection module manages, these slots hold about ten minutes.
    var TREND_SLOTS = 550;

    OSC.ptcc = {
        trend: [],
        gains: [],
        steps: {},
        known: false,
        connected: false,
        module: false
    };

    /** Without a controller there is nothing to show, so the readings and the
     *  values in the status line are taken off the page rather than left at
     *  zero. The detection blocks need a programmable module as well. */
    function updateVisibility() {
        // Until the service has said something about the controller there is
        // nothing to claim: only the state of the service itself is shown.
        var connected = OSC.ptcc.known && OSC.ptcc.connected;
        $('#ptcc_status').toggleClass('ptcc-known', OSC.ptcc.known)
            .toggleClass('ptcc-live', connected);
        $('#ptcc_tec_group').prop('hidden', !connected);
        $('#ptcc_det_group').prop('hidden', !(connected && OSC.ptcc.module));
        $('#ptcc_det_controls').prop('hidden', !(connected && OSC.ptcc.module));
        $('#ptcc_det_absent').prop('hidden', !(connected && !OSC.ptcc.module));
    }

    /** Height of the plot: the region it lives in, less the buffer bar above
     *  it and the time axis labels below, which are drawn over the region
     *  rather than in its flow. The region itself is sized by the layout. */
    OSC.ptccCanvasHeight = function() {
        var area = $('#plots_area');
        var height = area.length ? Math.round(area.height()) : 0;
        if (height <= 0) {
            height = Math.max(200, window.innerHeight - 330);
        }
        return Math.max(160, height - 105);
    };

    /** Lines the controller blocks up with the plot grids above them: the left
     *  edge with the first grid, the right edge with the last one, which in
     *  X-Y mode is the second plot. The grids carry paddings of their own, so
     *  the edges are measured rather than assumed. */
    OSC.ptccAlignUnder = function() {
        var under = $('#ptcc_under');
        var first = $('#graph_grid');
        if (under.length === 0 || first.length === 0 || first.width() === 0) {
            return;
        }

        // In X-Y the right hand edge belongs to the frame around the plot,
        // which is one pixel wider than the canvas inside it.
        var xy = $('#xy_graphs_holder');
        var last = (xy.length && xy.is(':visible') && xy.width() > 0) ? xy : first;

        var box = under.offset().left;
        var width = under.outerWidth();
        var left = Math.max(0, Math.round(first.offset().left - box));
        var right = Math.max(0, Math.round(box + width - (last.offset().left + last.outerWidth())));

        under.css({'padding-left': left, 'padding-right': right});
    };

    function showValue(name, text) {
        $('[data-ptcc="' + name + '"]').each(function() {
            var unit = $(this).data('unit');
            $(this).text(unit ? text + ' ' + unit : text);
        });
    }

    function onReading(new_params, name) {
        var value = new_params[name].value;
        showValue(name, Number(value).toFixed(READOUTS[name]));

        if (name === 'PTCC_T_DET' || name === 'PTCC_LABM_U_OUT') {
            addTrendSample(name, Number(value));
        }
    }

    function onText(new_params, name) {
        $('[data-ptcc-text="' + name + '"]').text(new_params[name].value);
    }

    function onSetting(new_params, name) {
        // Written by the person and echoed back by the device, which may have
        // rounded or clamped it. The echo is ignored while the panel is open,
        // otherwise the field would change under the hands typing in it.
        if (OSC.state.editing) {
            return;
        }

        var field = $('#' + name);
        var value = new_params[name].value;

        // A choice is a group of buttons, one of which carries the value.
        var radios = $('input[name="' + name + '"]');
        if (radios.length > 0) {
            radios.closest('.btn-group').children('.btn.active').removeClass('active');
            radios.eq(Number(value)).prop('checked', true).parent().addClass('active');
            return;
        }

        if (field.is('select')) {
            field.val(String(value));
        } else if (field.is('input')) {
            var entry = FIELDS[name];
            var step = entry ? (entry.step || OSC.ptcc.steps[name]) : undefined;
            field.val(step ? Number(value).toFixed(digitsFor(step)) : value);
        }

        if (name === 'PTCC_SETPOINT') {
            showValue(name, Number(value).toFixed(3));
        }
    }

    function onLimit(new_params, name) {
        for (var target in LIMITS) {
            var bounds = LIMITS[target];
            var field = $('#' + target);
            if (bounds[0] === name) {
                field.attr('min', new_params[name].value);
            } else if (bounds[1] === name) {
                field.attr('max', new_params[name].value);
            }
            applyStep(target, field);
        }
    }

    /** The step of a field, and with it the number of decimals it is shown
     *  with. A coded value moves by a 256th of the range the module reports,
     *  so its step is only known once both limits have arrived. */
    function applyStep(target, field) {
        var entry = FIELDS[target];
        if (entry === undefined || field.length === 0) {
            return;
        }

        var step = entry.step;
        if (entry.coded) {
            var low = OSC.params.orig[LIMITS[target][0]];
            var high = OSC.params.orig[LIMITS[target][1]];
            if (low === undefined || high === undefined) {
                return;
            }
            // The module codes its range in 256 steps, which lands on a
            // binary fraction like 0.00390625. The field is shown to three
            // decimals, so the step is put on that same grid: otherwise an
            // arrow moves the value by less than the field can show and the
            // reading jumps back and forth as the device echoes it.
            step = Math.max(FINEST_STEP,
                            Math.round((high.value - low.value) / 256 * 1000) / 1000);
            OSC.ptcc.steps[target] = step;
        }

        if (step > 0) {
            field.attr('step', step);
        }
    }

    // The module reports the gains it accepts as a comma separated list, and
    // refuses anything else, so the field is a list rather than a number.
    function onGains(new_params, name) {
        var list = String(new_params[name].value);
        if (list === '' || list === OSC.ptcc.gains.join(',')) {
            return;
        }

        OSC.ptcc.gains = list.split(',').filter(function(item) {
            return item !== '';
        });

        var select = $('#PTCC_LABM_GAIN');
        var current = select.val();
        select.empty();
        OSC.ptcc.gains.forEach(function(gain) {
            var value = Number(gain);
            select.append($('<option>').attr('value', String(value)).text(value.toFixed(1)));
        });
        if (current !== null) {
            select.val(current);
        }
    }

    function onGainValue(new_params, name) {
        var value = Number(new_params[name].value);
        if (OSC.state.editing) {
            return;
        }

        var select = $('#PTCC_LABM_GAIN');
        // The device answers with its own rounding of the gain, so the entry
        // closest to what it reports is the one to show.
        var best = null;
        OSC.ptcc.gains.forEach(function(gain) {
            var candidate = Number(gain);
            if (best === null || Math.abs(candidate - value) < Math.abs(best - value)) {
                best = candidate;
            }
        });
        if (best !== null) {
            select.val(String(best));
        }
    }

    function onModule(new_params, name) {
        OSC.ptcc.module = new_params[name].value === true;
        updateVisibility();
    }

    function onStatus(new_params, name) {
        var error = new_params[name].value === true;
        // The lamp stays grey while there is no controller to report on.
        $('#ptcc_status_led').toggleClass('ptcc-led-error', error && OSC.ptcc.connected);
        $('#ptcc_status_led').toggleClass('ptcc-led-ok', !error && OSC.ptcc.connected);
    }

    function onConnected(new_params, name) {
        OSC.ptcc.known = true;
        OSC.ptcc.connected = new_params[name].value === true;
        if (!OSC.ptcc.connected) {
            $('#ptcc_status_led').removeClass('ptcc-led-ok ptcc-led-error');
            $('[data-ptcc-text="PTCC_STATUS_TEXT"]').text('no controller');
        }
        updateVisibility();
    }

    // STOPPED, STARTING, CONNECTED, ALIVE, as the application publishes them.
    var SERVICE_TEXT = ['service stopped', 'service starting', 'service connected', 'service running'];

    function onServiceState(new_params, name) {
        var state = new_params[name].value;
        $('#ptcc_service_text').text(SERVICE_TEXT[state] || 'service');
        $('#ptcc_service_led').toggleClass('ptcc-led-ok', state === 3);
        $('#ptcc_service_led').toggleClass('ptcc-led-error', state === 0);
    }

    function addTrendSample(name, value) {
        var now = Date.now();
        var last = OSC.ptcc.trend[OSC.ptcc.trend.length - 1];

        // Both values come from the same service poll, so they share a point
        // as long as they arrive within the same update.
        if (last === undefined || now - last.t > 100) {
            last = { t: now, tdet: null, uout: null };
            OSC.ptcc.trend.push(last);
        }
        last[name === 'PTCC_T_DET' ? 'tdet' : 'uout'] = value;

        while (OSC.ptcc.trend.length > TREND_SLOTS) {
            OSC.ptcc.trend.shift();
        }

        if (!$('#ptcc_trend').prop('hidden')) {
            drawTrend();
        }
    }

    function trendRange(key) {
        var min = null;
        var max = null;
        OSC.ptcc.trend.forEach(function(point) {
            if (point[key] === null) {
                return;
            }
            min = min === null ? point[key] : Math.min(min, point[key]);
            max = max === null ? point[key] : Math.max(max, point[key]);
        });
        if (min === null) {
            return null;
        }

        // The axis is at least a tenth of the value wide, so a reading that
        // only trembles is drawn as a flat line instead of being blown up to
        // the full height of the block. A span that is wider than that keeps
        // its own scale, with a tenth of it as headroom.
        var centre = (min + max) / 2;
        var half = Math.max(Math.abs(centre) * 0.1, (max - min) / 2 * 1.1, 1e-6);
        return { min: centre - half, max: centre + half };
    }

    function drawSeries(context, key, range, color, width, height) {
        if (range === null) {
            return;
        }

        // Every sample owns a slot of its own, and the slots left empty are
        // the ones in front: a fresh plot draws at the right and walks left.
        var slot = width / (TREND_SLOTS - 1);
        var offset = TREND_SLOTS - OSC.ptcc.trend.length;

        context.strokeStyle = color;
        context.lineWidth = 1.5;
        context.beginPath();

        var started = false;
        OSC.ptcc.trend.forEach(function(point, index) {
            if (point[key] === null) {
                started = false;
                return;
            }
            var x = (offset + index) * slot;
            var y = height - ((point[key] - range.min) / (range.max - range.min)) * height;
            if (started) {
                context.lineTo(x, y);
            } else {
                context.moveTo(x, y);
                started = true;
            }
        });
        context.stroke();
    }

    function drawTrend() {
        var canvas = $('#ptcc_trend_canvas')[0];
        if (!canvas) {
            return;
        }

        // The canvas is laid out by the block; here it is only given the
        // matching number of pixels to draw on. Measuring the element itself
        // rather than its block keeps the two from chasing each other, and it
        // is done before anything else, so an empty plot is still the right
        // size instead of the 300 by 150 a canvas defaults to.
        var width = canvas.clientWidth;
        var height = canvas.clientHeight;
        if (width <= 0 || height <= 0) {
            return;
        }

        if (canvas.width !== width) {
            canvas.width = width;
        }
        if (canvas.height !== height) {
            canvas.height = height;
        }

        var context = canvas.getContext('2d');
        context.clearRect(0, 0, width, height);

        if (OSC.ptcc.trend.length < 2) {
            $('#ptcc_trend_span').text('');
            return;
        }

        drawSeries(context, 'tdet', trendRange('tdet'), '#f3ec1a', width, height);
        drawSeries(context, 'uout', trendRange('uout'), '#3bb0ff', width, height);

        // How much time the whole strip holds, from the pace samples arrive at.
        var first = OSC.ptcc.trend[0].t;
        var last = OSC.ptcc.trend[OSC.ptcc.trend.length - 1].t;
        var pace = (last - first) / (OSC.ptcc.trend.length - 1);
        var span = Math.round(pace * TREND_SLOTS / 1000);
        $('#ptcc_trend_span').text(span < 60 ? span + ' s'
                                             : Math.floor(span / 60) + ':' +
                                               ('0' + (span % 60)).slice(-2));
    }

    OSC.ptccInit = function() {
        for (var name in READOUTS) {
            OSC.param_callbacks[name] = onReading;
        }
        TEXTS.forEach(function(name) {
            OSC.param_callbacks[name] = onText;
        });
        for (var target in LIMITS) {
            OSC.param_callbacks[target] = onSetting;
            LIMITS[target].forEach(function(bound) {
                OSC.param_callbacks[bound] = onLimit;
            });
        }
        ['PTCC_TEC_CTRL', 'PTCC_SUP_CTRL', 'PTCC_FAN_CTRL', 'PTCC_LABM_TRANS', 'PTCC_LABM_COUPLING',
            'PTCC_LABM_BW'
        ].forEach(function(name) {
            OSC.param_callbacks[name] = onSetting;
        });

        OSC.param_callbacks['PTCC_LABM_GAINS'] = onGains;
        OSC.param_callbacks['PTCC_LABM_GAIN'] = onGainValue;
        OSC.param_callbacks['PTCC_LABM_PRESENT'] = onModule;
        OSC.param_callbacks['PTCC_STATUS_IS_ERROR'] = onStatus;
        OSC.param_callbacks['PTCC_CONNECTED'] = onConnected;
        OSC.param_callbacks['PTCC_SERVICE_STATE'] = onServiceState;

        updateVisibility();

        // The trend is switched on from the menu in the header, and the entry
        // carries a tick while it is on, the way SYS INFO does.
        $('#ptcc_monitor_plot').on('click', function(event) {
            event.preventDefault();
            var trend = $('#ptcc_trend');
            var shown = trend.prop('hidden');
            trend.prop('hidden', !shown);
            $(this).html(shown ? '&check; U/T PLOT' : 'U/T PLOT');
            // The plot gives up the height the trend takes, and the other way
            // round when it is folded away again.
            OSC.resize();
            drawTrend();
        });

        // The arrows of a field move it by one device code, and the input
        // widget then writes the value out with as many decimals as that step
        // has - eight for a 256th of a volt. The text is put back on the grid
        // the panel shows; the device is sent what was typed and snaps it to
        // its own code anyway.
        $('#PTCC_LABM_BIAS_U, #PTCC_LABM_BIAS_I, #PTCC_LABM_OFFSET').on('change', function() {
            var step = OSC.ptcc.steps[this.id];
            var value = Number($(this).val());
            if (step && !isNaN(value)) {
                $(this).val(value.toFixed(digitsFor(step)));
            }
        });

        // Starts the strip over, for when the interesting part begins now.
        $('#ptcc_trend_reset').on('click', function() {
            OSC.ptcc.trend = [];
            drawTrend();
        });

        $(window).on('resize', drawTrend);

        // The plot is sized from the height of the blocks, which is only
        // known once the page has been laid out and the images have loaded.
        $(window).on('load', function() {
            OSC.resize();
        });

        // The blocks grow and shrink on their own: a channel is switched on, a
        // measurement is added, a detection module appears. The plot has to
        // give up exactly that much height, so the layout is redone whenever
        // their height actually changes.
        var last_height = 0;
        var observer = new ResizeObserver(function() {
            var height = Math.round($('#ptcc_under').outerHeight(true));
            if (height !== last_height) {
                last_height = height;
                OSC.resize();
                drawTrend();
            }
            OSC.ptccAlignUnder();
        });
        observer.observe(document.querySelector('#ptcc_under'));

        // A canvas keeps the number of pixels it was given, so the bitmap is
        // resized by hand whenever the block it lives in changes size.
        var trend_observer = new ResizeObserver(function() {
            drawTrend();
        });
        trend_observer.observe(document.querySelector('#ptcc_trend'));
    };

}(window.OSC = window.OSC || {}, jQuery));
