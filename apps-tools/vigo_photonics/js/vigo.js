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

    var TREND_SECONDS = 300;

    OSC.ptcc = {
        trend: [],
        gains: [],
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

        if (field.is('select')) {
            field.val(String(value));
        } else if (field.is('input')) {
            field.val(value);
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

        var cutoff = now - TREND_SECONDS * 1000;
        while (OSC.ptcc.trend.length > 0 && OSC.ptcc.trend[0].t < cutoff) {
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
        if (max - min < 1e-6) {
            // A value that does not move still needs a box to be drawn in.
            min -= 0.5;
            max += 0.5;
        }
        var margin = (max - min) * 0.1;
        return { min: min - margin, max: max + margin };
    }

    function drawSeries(context, key, range, color, width, height, first, last) {
        if (range === null || last === first) {
            return;
        }

        context.strokeStyle = color;
        context.lineWidth = 1.5;
        context.beginPath();

        var started = false;
        OSC.ptcc.trend.forEach(function(point) {
            if (point[key] === null) {
                return;
            }
            var x = ((point.t - first) / (last - first)) * width;
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

        var first = OSC.ptcc.trend[0].t;
        var last = OSC.ptcc.trend[OSC.ptcc.trend.length - 1].t;

        drawSeries(context, 'tdet', trendRange('tdet'), '#f3ec1a', canvas.width, height, first, last);
        drawSeries(context, 'uout', trendRange('uout'), '#3bb0ff', canvas.width, height, first, last);

        $('#ptcc_trend_span').text(Math.round((last - first) / 1000) + ' s');
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
