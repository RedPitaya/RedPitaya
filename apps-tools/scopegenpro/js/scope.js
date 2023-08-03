/*
 * Red Pitaya Oscilloscope client
 *
 * Author: Dakus <info@eskala.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function() {
    var originalAddClassMethod = jQuery.fn.addClass;
    var originalRemoveClassMethod = jQuery.fn.removeClass;
    $.fn.addClass = function(clss) {
        var result = originalAddClassMethod.apply(this, arguments);
        $(this).trigger('activeChanged', 'add');
        return result;
    };
    $.fn.removeClass = function(clss) {
        var result = originalRemoveClassMethod.apply(this, arguments);
        $(this).trigger('activeChanged', 'remove');
        return result;
    }
})();


(function() {

    if ("performance" in window == false) {
        window.performance = {};
    }

    Date.now = (Date.now || function() { // thanks IE8
        return new Date().getTime();
    });

    if ("now" in window.performance == false) {
        var nowOffset = Date.now();
        if (performance.timing && performance.timing.navigationStart) {
            nowOffset = performance.timing.navigationStart
        }
        window.performance.now = function now() {
            return Date.now() - nowOffset;
        }
    }

})();

(function(OSC, $, undefined) {

    // App configuration

    OSC.startTime = 0;
    OSC.param_callbacks = {};
    OSC.config = {};
    OSC.config.app_id = 'scopegenpro';
    OSC.config.server_ip = ''; // Leave empty on production, it is used for testing only

    OSC.config.start_app_url = window.location.origin + '/bazaar?start=' + OSC.config.app_id;
    OSC.config.stop_app_url = window.location.origin + '/bazaar?stop=' + OSC.config.app_id;
    OSC.config.socket_url = 'ws://' + window.location.host + '/wss';
    OSC.rp_model = "";
    OSC.adc_channes = 2;
    OSC.is_ext_trig_level_present = false;
    OSC.is_webpage_loaded = false;

    OSC.config.graph_colors = {
        'ch1': '#f3ec1a',
        'ch2': '#31b44b',
        'ch3': '#ee3739',
        'ch4': '#3af7f7',
        'output1': '#9595ca',
        'output2': '#ee3739',
        'math': '#ab4d9d',
        'trig': '#75cede'
    };

    // Time scale steps in millisecods
    OSC.time_steps = [
        // Nanoseconds
        5 / 1000000, 10 / 1000000, 50 / 1000000, 100 / 1000000, 200 / 1000000, 500 / 1000000,
        // Microseconds
        1 / 1000, 2 / 1000, 5 / 1000, 10 / 1000, 20 / 1000, 50 / 1000, 100 / 1000, 200 / 1000, 500 / 1000,
        // Millisecods
        1, 2, 5, 10, 20, 50, 100, 200, 500, 1000
    ];

    // Voltage scale steps in volts
    OSC.voltage_steps = [
        // Millivolts
        1 / 1000, 2 / 1000, 5 / 1000, 10 / 1000, 20 / 1000, 50 / 1000, 100 / 1000, 200 / 1000, 500 / 1000,
        // Volts
        1
    ];

    // Voltage scale steps in volts for Math
    OSC.voltage_steps_math = [
        // Millivolts
        1 / 1000, 2 / 1000, 5 / 1000, 10 / 1000, 20 / 1000, 50 / 1000, 100 / 1000, 200 / 1000, 500 / 1000,
        // Volts
        1 , 5 , 25 , 50, 100 , 250 , 500 , 1000, 2000, 5000, 10000, 20000, 50000, 100000 , 200000, 500000, 1000000
    ];

    OSC.bad_connection = [false, false, false, false]; // time in s.

    OSC.compressed_data = 0;
    OSC.decompressed_data = 0;
    OSC.refresh_times = [];
    OSC.counts_offset = 0;

    // Sampling rates
    OSC.sample_rates = {1:'125M', 8:'15.625M', 64: '1.953M', 1024: '122.070k', 8192:'15.258k', 65536:'1.907k'};
    OSC.mouseWheelEventFired = false; // for MAC

    // App state
    OSC.state = {
        socket_opened: false,
        processing: false,
        editing: false,
        time_dragging: false,
        trig_dragging: false,
        cursor_dragging: false,
        cursor_dragging_measure:false,
        simulated_drag:false,
        mouseover: false,
        resized: false,
        sel_sig_name: 'ch1',
        fine: false,
        graph_grid_height: null,
        graph_grid_width: null,
        calib: 0,
        sweep_ch1:false,
        sweep_ch2:false,
        sweep_ch1_time:1,
        sweep_ch2_time:1,
        burst_ch1:false,
        burst_ch2:false
    };

    // Params cache
    OSC.params = {
        orig: {},
        old: {},
        local: {}
    };

    // Other global variables
    OSC.ws = null;
    OSC.graphs = {};
    OSC.touch = {};

    OSC.connect_time;

    // OSC.inGainValue1 = '-';
    // OSC.inGainValue2 = '-';

    OSC.loaderShow = false;
    OSC.running = true;
    OSC.unexpectedClose = false;

    OSC.parameterStack = [];
    OSC.signalStack = [];

    OSC.lastSignals = [];
    OSC.client_id = undefined;

    var g_counter = 0;
    var g_PacketsRecv = 0;
    OSC.g_CpuLoad = 100.0;
    OSC.g_TotalMemory = 256.0;
    OSC.g_FreeMemory = 256.0;

    OSC.time_offset = 0;
    OSC.time_scale = 0;

    OSC.checkStatusTimer = undefined;
    OSC.changeStatusForRestart = false;
    OSC.changeStatusStep = 0;


    $.preloadImages = function() {
        for (var i = 0; i < arguments.length; i++) {
            $('<img />').attr('src', 'img/' + arguments[i]);
        }
    }
    $.preloadImages(
        'edge1_active.png',
        'edge2_active.png',
        'node_up.png',
        'node_left.png',
        'node_right.png',
        'node_down.png',
        'fine_active.png',
        'trig-edge-up.png',
        'trig-edge-down.png',
        'out1-offset-arrow.png',
        'out2-offset-arrow.png',
        'ch1-offset-arrow.png',
        'ch2-offset-arrow.png',
        'ch3-offset-arrow.png',
        'ch4-offset-arrow.png'
    );

    // Starts the oscilloscope application on server
    OSC.startApp = function() {
        $.get(
                OSC.config.start_app_url
            )
            .done(function(dresult) {
                if (dresult.status == 'OK') {
                    try {
                        OSC.connectWebSocket();
                    } catch (e) {
                        OSC.startApp();
                    }
                } else if (dresult.status == 'ERROR') {
                    console.log(dresult.reason ? dresult.reason : 'Could not start the application (ERR1)');
                    OSC.startApp();
                } else {
                    console.log('Could not start the application (ERR2)');
                    OSC.startApp();
                }
            })
            .fail(function() {
                console.log('Could not start the application (ERR3)');
                OSC.startApp();
            });
    };


    Date.prototype.format = function(mask, utc) {
        return dateFormat(this, mask, utc);
    };

    var guiHandler = function() {
        if (OSC.signalStack.length > 0) {
            var p = performance.now();
            if (OSC.is_webpage_loaded){
                OSC.processSignals(OSC.signalStack[0]);
            }
            // console.log(OSC.signalStack.length,OSC.signalStack[0]);
            OSC.signalStack.splice(0, 1);
            OSC.refresh_times.push("tick");
            // console.log("Drawing: " + (performance.now() - p));
        }
        if (OSC.signalStack.length > 2)
            OSC.signalStack.length = [];
    }

    var parametersHandler = function() {
        if (OSC.parameterStack.length > 0) {
            var p = performance.now();
            console.log(OSC.parameterStack.length,OSC.parameterStack[0]);
            while(OSC.parameterStack.length){
                OSC.processParameters(OSC.parameterStack[0]);
                OSC.parameterStack.splice(0, 2);
            }
        }
    }

    var performanceHandler = function() {

        $('#fps_view').text(OSC.refresh_times.length);
        $('#throughput_view').text((OSC.compressed_data / 1024).toFixed(2) + "kB/s");
        $('#throughput_view2').text((OSC.compressed_data / 1024).toFixed(2) + "kB/s");
        $('#cpu_load').text(OSC.g_CpuLoad.toFixed(2) + "%");
        $('#totalmem_view').text((OSC.g_TotalMemory / (1024 * 1024)).toFixed(2) + "Mb");
        $('#freemem_view').text((OSC.g_FreeMemory / (1024 * 1024)).toFixed(2) + "Mb");
        $('#usagemem_view').text(((OSC.g_TotalMemory - OSC.g_FreeMemory) / (1024 * 1024)).toFixed(2) + "Mb");
        if ($('#connection_icon').attr('src') !== '../assets/images/good_net.png')
            $('#connection_icon').attr('src', '../assets/images/good_net.png');
        $('#connection_meter').attr('title', 'It seems like your connection is ok');
        if (g_PacketsRecv < 5 || g_PacketsRecv > 25) {
            if ($('#connection_icon').attr('src') !== '../assets/images/bad_net.pngg')
                $('#connection_icon').attr('src', '../assets/images/bad_net.png');
            $('#connection_meter').attr('title', 'Connection problem');
        }
        g_PacketsRecv = 0;

        OSC.compressed_data = 0;
        OSC.decompressed_data = 0;

        if (OSC.refresh_times.length < 3)
            OSC.bad_connection[g_counter] = true;
        else
            OSC.bad_connection[g_counter] = false;

        g_counter++;
        if (g_counter == 4) g_counter = 0;

        OSC.refresh_times = [];
    }


    OSC.reloadPage = function() {
        $.ajax({
            method: "GET",
            url: "/get_client_id",
            timeout: 2000
        }).done(function(msg) {
            if (msg.trim() === OSC.client_id) {
                location.reload();
            } else {
                $('body').removeClass('user_lost');
                OSC.stopCheckStatus();
            }
        }).fail(function(msg) {
            console.log(msg);
            $('body').removeClass('connection_lost');
        });
    }

    OSC.startCheckStatus = function() {
        if (OSC.checkStatusTimer === undefined) {
            OSC.changeStatusStep = 0;
            OSC.checkStatusTimer = setInterval(OSC.checkStatus, 4000);
        }
    }

    OSC.stopCheckStatus = function() {
        if (OSC.checkStatusTimer !== undefined) {
            clearInterval(OSC.checkStatusTimer);
            OSC.checkStatusTimer = undefined;
        }
    }

    OSC.checkStatus = function() {
        $.ajax({
            method: "GET",
            url: "/check_status",
            timeout: 2000
        }).done(function(msg) {
            switch (OSC.changeStatusStep) {
                case 0:
                    OSC.changeStatusStep = 1;
                    break;
                case 2:
                    OSC.reloadPage();
                    break;
            }
        }).fail(function(msg) {
            // check status. If don't have good state after start. We lock system.
            $('body').removeClass('connection_lost');
            switch (OSC.changeStatusStep) {
                case 0:
                    OSC.changeStatusStep = -1;
                    break;
                case 1:
                    OSC.changeStatusStep = 2;
                    break;
            }

        });
    }

    setInterval(performanceHandler, 1000);
    setInterval(guiHandler, 2);
    setInterval(parametersHandler, 2);

    OSC.setModel = function(_value) {
        if (OSC.rp_model === "") {
            console.log("Model",_value.value)
            $('#BODY').load((_value.value === "Z20_125_4CH" ? "4ch_adc.html" : "2ch_adc.html"), function() {
                OSC.rp_model = _value.value;
                console.log( "Load was performed." );

                const ob = new ResizeObserver(function(entries) {
                    OSC.updateJoystickPosition();
                });

                ob.observe(document.querySelector("#menu-root"));

                OSC.updateInterfaceFor250(OSC.rp_model);
                OSC.updateInterfaceForZ20(OSC.rp_model);
                OSC.initUI();
                OSC.initCursors();
                OSC.initOSCHandlers();
                OSC.initOSCCursors();
                OSC.drawGraphGrid();
                OSC.requestAllParam();
                OSC.resize();
                OSC.is_webpage_loaded = true;
            });
            return false
        }
        return true
    };

    OSC.updateJoystickPosition = function(){
        let height = $("#menu-root").height();
        let g_height = $("#graphs").height() - 150;
        let limit = Math.min(g_height,400)
        height = Math.max(height,g_height);
        height = height > limit ? height : limit;
        $("#joystick").css('top',height + 10);
    };

    // Creates a WebSocket connection with the web server
    OSC.connectWebSocket = function() {

        if (window.WebSocket) {
            OSC.ws = new WebSocket(OSC.config.socket_url);
            OSC.ws.binaryType = "arraybuffer";
        } else if (window.MozWebSocket) {
            OSC.ws = new MozWebSocket(OSC.config.socket_url);
            OSC.ws.binaryType = "arraybuffer";
        } else {
            console.log('Browser does not support WebSocket');
        }

        // Define WebSocket event listeners
        if (OSC.ws) {
            OSC.ws.onopen = function() {
                console.log('Socket opened');
                OSC.state.socket_opened = true;
                OSC.unexpectedClose = true;
                OSC.startTime = performance.now();
                $('body').addClass('loaded');
                $('body').addClass('connection_lost');
                $('body').addClass('user_lost');
                OSC.startCheckStatus();
            };

            OSC.ws.onclose = function() {
                OSC.state.socket_opened = false;
                $('#graphs .plot').hide(); // Hide all graphs
                console.log('Socket closed');
                if (OSC.unexpectedClose == true) {
                    setTimeout(OSC.reloadPage, '1000');
                }
            };

            OSC.ws.onerror = function(ev) {
                if (!OSC.state.socket_opened)
                    OSC.startApp();
                console.log('Websocket error: ', ev);
            };

            var last_time = undefined;
            OSC.ws.onmessage = function(ev) {
                var start_time = +new Date();
                if (OSC.state.processing) {
                    return;
                }
                OSC.state.processing = true;

                try {
                    var data = new Uint8Array(ev.data);
                    OSC.compressed_data += data.length;
                    var inflate = pako.inflate(data);
                    var text = String.fromCharCode.apply(null, new Uint8Array(inflate));

                    OSC.decompressed_data += text.length;
                    var receive = JSON.parse(text);

                    if (receive.parameters) {
                        OSC.parameterStack.push(receive.parameters);
                        if ((Object.keys(OSC.params.orig).length == 0) && (Object.keys(receive.parameters).length == 0)) {
                            OSC.requestAllParam();
                        }
                    }

                    if (receive.signals) {
                        g_PacketsRecv++;
                        OSC.signalStack.push(receive.signals);
                    }
                    OSC.state.processing = false;
                } catch (e) {
                    OSC.state.processing = false;
                    console.log(e);
                } finally {
                    OSC.state.processing = false;
                }
            };
        }
    };

    OSC.processRun = function(new_params) {
        if (new_params['OSC_RUN'].value === true) {
            $('#OSC_RUN').hide();
            $('#OSC_STOP').css('display', 'block');
        } else {
            $('#OSC_STOP').hide();
            $('#OSC_RUN').show();
        }
    }

    OSC.processViewPart = function(new_params) {
        var full_width = $('#buffer').width() - 4;
        var visible_width = full_width * new_params['OSC_VIEV_PART'].value;

        $('#buffer .buf-red-line').width(visible_width).show();
        $('#buffer .buf-red-line-holder').css('left', full_width / 2 - visible_width / 2);
        OSC.timeOffset()
    }


    OSC.mathOffset = function(new_params) {
        // вернуть
        // OSC.chOffset("MATH", new_params);
    }



    OSC.param_callbacks["OSC_RUN"] = OSC.processRun;
    OSC.param_callbacks["OSC_VIEV_PART"] = OSC.processViewPart;
    OSC.param_callbacks["OSC_SAMPL_RATE"] = OSC.processSampleRate;
    OSC.param_callbacks["OSC_TRIG_INFO"] = OSC.processTrigInfo;
    OSC.param_callbacks["OSC_MATH_OFFSET"] = OSC.mathOffset;
    OSC.param_callbacks["OUTPUT1_SHOW_OFF"] = OSC.out1ShowOffset;
    OSC.param_callbacks["OUTPUT2_SHOW_OFF"] = OSC.out2ShowOffset;

    OSC.param_callbacks["OUTPUT1_SHOW"] = OSC.setGenShow1;
    OSC.param_callbacks["OUTPUT2_SHOW"] = OSC.setGenShow2;
    OSC.param_callbacks["OUTPUT1_STATE"] = OSC.setGenState1;
    OSC.param_callbacks["OUTPUT2_STATE"] = OSC.setGenState2;



    OSC.param_callbacks["OSC_TIME_OFFSET"] = OSC.timeOffset;
    OSC.param_callbacks["OSC_TRIG_LEVEL"] = OSC.trigLevel;
    OSC.param_callbacks["OSC_TRIG_HYST"] = OSC.trigHyst;
    OSC.param_callbacks["OSC_TRIG_SOURCE"] = OSC.trigSource;
    OSC.param_callbacks["OSC_TRIG_SLOPE"] = OSC.trigSlope;
    OSC.param_callbacks["OSC_TRIG_SWEEP"] = OSC.trigSweep;
    OSC.param_callbacks["OSC_TRIG_LIMIT"] = OSC.triggerLimit;

    OSC.param_callbacks["OSC_CURSOR_Y1"] = OSC.cursorY1;
    OSC.param_callbacks["OSC_CURSOR_Y2"] = OSC.cursorY2;
    OSC.param_callbacks["OSC_CURSOR_X1"] = OSC.cursorX1;
    OSC.param_callbacks["OSC_CURSOR_X2"] = OSC.cursorX2;
    OSC.param_callbacks["OSC_CURSOR_SRC"] = OSC.setCursorSrc;

    OSC.param_callbacks["OSC_CUR1_T"] = OSC.cursorX1;
    OSC.param_callbacks["OSC_CUR2_T"] = OSC.cursorX2;
    OSC.param_callbacks["OSC_CUR1_V"] = OSC.cursorY1;
    OSC.param_callbacks["OSC_CUR2_V"] = OSC.cursorY2;

    OSC.param_callbacks["SOUR1_VOLT"] = OSC.src1Volt;
    OSC.param_callbacks["SOUR2_VOLT"] = OSC.src2Volt;
    OSC.param_callbacks["SOUR1_VOLT_OFFS"] = OSC.src1VoltOffset;
    OSC.param_callbacks["SOUR2_VOLT_OFFS"] = OSC.src2VoltOffset;


    OSC.param_callbacks["OSC_MEAS_SELN"] = OSC.measSelN;
    OSC.param_callbacks["SOUR1_SWEEP_STATE"] = OSC.sweepResetButton;
    OSC.param_callbacks["SOUR2_SWEEP_STATE"] = OSC.sweepResetButton;
    OSC.param_callbacks["SOUR1_SWEEP_TIME"] = OSC.sweepTime;
    OSC.param_callbacks["SOUR2_SWEEP_TIME"] = OSC.sweepTime;
    OSC.param_callbacks["SOUR1_BURST_STATE"] = OSC.burstResetButton;
    OSC.param_callbacks["SOUR2_BURST_STATE"] = OSC.burstResetButton;
    OSC.param_callbacks["SOUR1_RISE"] = OSC.riseFallTime;
    OSC.param_callbacks["SOUR1_FALL"] = OSC.riseFallTime;
    OSC.param_callbacks["SOUR2_RISE"] = OSC.riseFallTime;
    OSC.param_callbacks["SOUR2_FALL"] = OSC.riseFallTime;

    OSC.param_callbacks["SOUR1_DCYC"] = OSC.setOut1DCyc;
    OSC.param_callbacks["SOUR2_DCYC"] = OSC.setOut2DCyc;
    OSC.param_callbacks["SOUR1_PHAS"] = OSC.setOut1Phase;
    OSC.param_callbacks["SOUR2_PHAS"] = OSC.setOut2Phase;

    OSC.param_callbacks["SOUR1_FREQ_FIX"] = OSC.updateGenFreq;
    OSC.param_callbacks["SOUR2_FREQ_FIX"] = OSC.updateGenFreq;

    OSC.param_callbacks["SOUR1_SWEEP_FREQ_START"] = OSC.updateGenSweepStartFreq;
    OSC.param_callbacks["SOUR2_SWEEP_FREQ_START"] = OSC.updateGenSweepStartFreq;

    OSC.param_callbacks["SOUR1_SWEEP_FREQ_END"] = OSC.updateGenSweepEndFreq;
    OSC.param_callbacks["SOUR2_SWEEP_FREQ_END"] = OSC.updateGenSweepEndFreq;

    OSC.param_callbacks["SOUR1_SWEEP_MODE"] = OSC.updateGenSweepMode;
    OSC.param_callbacks["SOUR2_SWEEP_MODE"] = OSC.updateGenSweepMode;

    OSC.param_callbacks["SOUR1_SWEEP_DIR"] = OSC.updateGenSweepMode;
    OSC.param_callbacks["SOUR2_SWEEP_DIR"] = OSC.updateGenSweepMode;

    OSC.param_callbacks["SOUR1_FUNC"] = OSC.updateGenSweepFunc;
    OSC.param_callbacks["SOUR2_FUNC"] = OSC.updateGenSweepFunc;

    OSC.param_callbacks["SOUR1_TRIG_SOUR"] = OSC.updateGenTrigSource;
    OSC.param_callbacks["SOUR2_TRIG_SOUR"] = OSC.updateGenTrigSource;

    OSC.param_callbacks["SOUR1_BURST_COUNT"] = OSC.updateGenBurstCount;
    OSC.param_callbacks["SOUR2_BURST_COUNT"] = OSC.updateGenBurstCount;

    OSC.param_callbacks["SOUR1_BURST_REP"] = OSC.updateGenBurstRep;
    OSC.param_callbacks["SOUR2_BURST_REP"] = OSC.updateGenBurstRep;

    OSC.param_callbacks["SOUR1_BURST_INF"] = OSC.updateGenBurstInf;
    OSC.param_callbacks["SOUR2_BURST_INF"] = OSC.updateGenBurstInf;

    OSC.param_callbacks["SOUR1_BURST_INF"] = OSC.updateGenBurstInf;
    OSC.param_callbacks["SOUR2_BURST_INF"] = OSC.updateGenBurstInf;

    OSC.param_callbacks["SOUR1_BURST_DELAY"] = OSC.updateGenBurstDelay;
    OSC.param_callbacks["SOUR2_BURST_DELAY"] = OSC.updateGenBurstDelay;

    OSC.param_callbacks["SOUR1_TEMP_RUNTIME"] = OSC.updateOverheatBlockHandler;
    OSC.param_callbacks["SOUR2_TEMP_RUNTIME"] = OSC.updateOverheatBlockHandler;

    OSC.param_callbacks["SOUR1_TEMP_LATCHED"] = OSC.updateOverheatInfoHandler;
    OSC.param_callbacks["SOUR2_TEMP_LATCHED"] = OSC.updateOverheatInfoHandler;

    OSC.param_callbacks["EXT_CLOCK_LOCKED"] = OSC.updateExtClockLocked;

    OSC.param_callbacks["SOUR1_IMPEDANCE"] = OSC.updateMaxLimitOnLoadHandler;
    OSC.param_callbacks["SOUR2_IMPEDANCE"] = OSC.updateMaxLimitOnLoadHandler;

    OSC.param_callbacks["OSC_CH1_OUT_GAIN"] = OSC.processParametersZ250;
    OSC.param_callbacks["OSC_CH2_OUT_GAIN"] = OSC.processParametersZ250;

    OSC.param_callbacks["OSC_OUTPUT1_SCALE"] = OSC.ch1SetGenScale;
    OSC.param_callbacks["OSC_OUTPUT2_SCALE"] = OSC.ch2SetGenScale;

    OSC.param_callbacks["OSC_CH1_OFFSET"] = OSC.ch1Offset;
    OSC.param_callbacks["OSC_CH2_OFFSET"] = OSC.ch2Offset;
    OSC.param_callbacks["OSC_CH3_OFFSET"] = OSC.ch3Offset;
    OSC.param_callbacks["OSC_CH4_OFFSET"] = OSC.ch4Offset;

    OSC.param_callbacks["OSC_CH1_SCALE"] = OSC.ch1SetScale;
    OSC.param_callbacks["OSC_CH2_SCALE"] = OSC.ch2SetScale;
    OSC.param_callbacks["OSC_CH3_SCALE"] = OSC.ch3SetScale;
    OSC.param_callbacks["OSC_CH4_SCALE"] = OSC.ch4SetScale;

    OSC.param_callbacks["OSC_CH1_IN_GAIN"] = OSC.ch1SetGain;
    OSC.param_callbacks["OSC_CH2_IN_GAIN"] = OSC.ch2SetGain;
    OSC.param_callbacks["OSC_CH3_IN_GAIN"] = OSC.ch3SetGain;
    OSC.param_callbacks["OSC_CH4_IN_GAIN"] = OSC.ch4SetGain;

    OSC.param_callbacks["OSC_CH1_IN_AC_DC"] = OSC.ch1SetACDC;
    OSC.param_callbacks["OSC_CH2_IN_AC_DC"] = OSC.ch2SetACDC;
    OSC.param_callbacks["OSC_CH3_IN_AC_DC"] = OSC.ch3SetACDC;
    OSC.param_callbacks["OSC_CH4_IN_AC_DC"] = OSC.ch4SetACDC;

    OSC.param_callbacks["OSC_CH1_PROBE"] = OSC.setOscProbe1;
    OSC.param_callbacks["OSC_CH2_PROBE"] = OSC.setOscProbe2;
    OSC.param_callbacks["OSC_CH3_PROBE"] = OSC.setOscProbe3;
    OSC.param_callbacks["OSC_CH4_PROBE"] = OSC.setOscProbe4;


    OSC.param_callbacks["CH1_SHOW"] = OSC.ch1Show;
    OSC.param_callbacks["CH2_SHOW"] = OSC.ch2Show;
    OSC.param_callbacks["CH3_SHOW"] = OSC.ch3Show;
    OSC.param_callbacks["CH4_SHOW"] = OSC.ch4Show;

    OSC.param_callbacks["CH1_SHOW_INVERTED"] = OSC.updateOscShowInverted;
    OSC.param_callbacks["CH2_SHOW_INVERTED"] = OSC.updateOscShowInverted;
    OSC.param_callbacks["CH3_SHOW_INVERTED"] = OSC.updateOscShowInverted;
    OSC.param_callbacks["CH4_SHOW_INVERTED"] = OSC.updateOscShowInverted;



    OSC.param_callbacks["OSC_MEAS_VAL1"] = OSC.measureHandler1Func;
    OSC.param_callbacks["OSC_MEAS_VAL2"] = OSC.measureHandler2Func;
    OSC.param_callbacks["OSC_MEAS_VAL3"] = OSC.measureHandler3Func;
    OSC.param_callbacks["OSC_MEAS_VAL4"] = OSC.measureHandler4Func;

    OSC.param_callbacks["OSC_TIME_SCALE"] = OSC.setTimeScale;

    OSC.param_callbacks["MATH_SHOW"] = OSC.mathShow;
    OSC.param_callbacks["OSC_MATH_OFFSET"] = OSC.chMathOffset;
    OSC.param_callbacks["OSC_MATH_SCALE"] = OSC.updateMathScale;
    OSC.param_callbacks["OSC_MATH_OP"] = OSC.updateMathOp;
    OSC.param_callbacks["OSC_MATH_SRC1"] = OSC.updateMathSrc1;
    OSC.param_callbacks["OSC_MATH_SRC2"] = OSC.updateMathSrc2;
    OSC.param_callbacks["MATH_SHOW_INVERTED"] = OSC.updateMathShowInverted;


    OSC.param_callbacks["CPU_LOAD"] = OSC.setCPULoad;
    OSC.param_callbacks["TOTAL_RAM"] = OSC.setRamTotal;
    OSC.param_callbacks["FREE_RAM"] = OSC.setFreeRam;



    // Processes newly received values for parameters
    OSC.processParameters = function(new_params) {
        // console.log(new_params);

        if (new_params['ADC_COUNT']){
            OSC.adc_channes = new_params['ADC_COUNT'].value;
        }

        if (new_params['OSC_TRIG_LIMIT_IS_PRESENT']){
            OSC.is_ext_trig_level_present = new_params['OSC_TRIG_LIMIT_IS_PRESENT'].value;
        }

        // Hack for json limitation
        if (new_params['OSC_TIME_SCALE']){
            new_params['OSC_TIME_SCALE'].value = parseFloat(new_params['OSC_TIME_SCALE'].value) / 1000.0;
        }

        if (new_params['RP_MODEL_STR']){
            if (!OSC.setModel(new_params['RP_MODEL_STR']))
                return
        }else{
            if (OSC.rp_model === ""){
                OSC.requestAllParam();
                return;
            }
        }

        OSC.params.old = $.extend(true, {}, OSC.params.orig);
        var old_params = OSC.params.old;


        for (var param_name in new_params) {

            // Save new parameter value
            OSC.params.orig[param_name] = new_params[param_name];

            if (param_name == "OSC_AUTOSCALE") {
                // console.log(OSC.params.orig[param_name].value);
                if (OSC.params.orig["OSC_AUTOSCALE"].value == false)
                    OSC.loaderShow = false;

            }

            if (OSC.param_callbacks[param_name] !== undefined){
                OSC.param_callbacks[param_name](new_params,param_name);
                continue;
            }
            // Find the field having ID equal to current parameter name
            // TODO: Use classes instead of ids, to be able to use a param name in multiple fields and to loop through all fields to set new values
            var field = $('#' + param_name);

            // Do not change fields from dialogs when user is editing something or new parameter value is the same as the old one
            if (field.closest('.menu-content').length == 0 ||
                (!OSC.state.editing && (old_params[param_name] === undefined ||
                    old_params[param_name].value !== new_params[param_name].value))) {

                if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
                        field.val(new_params[param_name].value);
                        console.log("Uhandled parameter",param_name,new_params[param_name])
                } else if (field.is('button')) {
                    field[new_params[param_name].value === true ? 'addClass' : 'removeClass']('active');
                    console.log("Uhandled parameter",param_name,new_params[param_name])
                } else if (field.is('input:radio')) {
                    var radios = $('input[name="' + param_name + '"]');

                    radios.closest('.btn-group').children('.btn.active').removeClass('active');
                    radios.eq([+new_params[param_name].value]).prop('checked', true).parent().addClass('active');
                    console.log("Uhandled parameter",param_name,new_params[param_name])
                } else if (field.is('span')) {
                    if ($.inArray(param_name, ['OSC_CH1_SCALE', 'OSC_CH2_SCALE', 'OSC_CH3_SCALE', 'OSC_CH4_SCALE', 'OSC_MATH_SCALE', 'OSC_OUTPUT1_SCALE', 'OSC_OUTPUT2_SCALE']) > -1) {
                    } else {
                        var exclude = ["OSC_SAMPL_RATE", "OSC_AUTOSCALE", "OSC_VIEV_PART", "OSC_RUN", "OSC_MEAS_UNITS", "OSC_MEAS_VAL1", "OSC_MEAS_VAL2", "OSC_MEAS_VAL3", "OSC_MEAS_VAL4"];
                        if (exclude.indexOf(param_name) == -1){
                            console.log("Uhandled parameter",param_name,new_params[param_name])
                            field.html(new_params[param_name].value);
                        }
                    }
                    console.log("Uhandled parameter",param_name,new_params[param_name])
                }
            } else {
                console.log("Uhandled parameter",param_name,new_params[param_name])
            }
        }
    };

    // Processes newly received data for signals
    OSC.iterCnt = 0;
    OSC.processSignals = function(new_signals) {
        var visible_btns = [];
        var visible_plots = [];
        var visible_info = '';
        var start = +new Date();

        // Do nothing if no parameters received yet
        if ($.isEmptyObject(OSC.params.orig)) {
            return;
        }

        var pointArr = [];
        var colorsArr = [];
        $('#right_menu .menu-btn').not('.not-signal').prop('disabled', true);
        // (Re)Draw every signal
        for (sig_name in new_signals) {

            // Ignore empty signals
            if (new_signals[sig_name].size == 0)
                continue;


            var sig_btn = $('#right_menu .menu-btn.' + sig_name);
            // Ignore disabled signals
            if (OSC.params.orig[sig_name.toUpperCase() + '_SHOW'] && OSC.params.orig[sig_name.toUpperCase() + '_SHOW'].value == false) {
                sig_btn.not('.not-signal').prop('disabled', true);
                continue;
            } else
                sig_btn.not('.not-signal').prop('disabled', false);

            // Ignore math signal if no operator defined
            if (sig_name == 'math' && (!OSC.params.orig['MATH_SHOW'] || OSC.params.orig['MATH_SHOW'].value == false))
                continue;

            var points = [];
            var color = OSC.config.graph_colors[sig_name];

            if (OSC.params.orig['OSC_VIEW_START_POS'] && OSC.params.orig['OSC_VIEW_END_POS']) {
                if ((((sig_name == 'output1') || (sig_name == 'output2')) && OSC.params.orig['OSC_VIEW_END_POS'].value != 0)) {
                    for (var i = 0; i < new_signals[sig_name].size; i++) {
                        points.push([i, new_signals[sig_name].value[i]]);
                    }
                } else {
                    for (var i = OSC.params.orig['OSC_VIEW_START_POS'].value; i < OSC.params.orig['OSC_VIEW_END_POS'].value; i++)
                        points.push([i, new_signals[sig_name].value[i]]);
                }
            } else {
                for (var i = 0; i < new_signals[sig_name].size; i++) {
                    points.push([i, new_signals[sig_name].value[i]]);
                }
            }

            OSC.lastSignals[sig_name] = new_signals[sig_name];

            if (!OSC.loaderShow) {
                $('body').addClass('loaded');
            }

            pointArr.push(points);
            colorsArr.push(color);

            // By default first signal is selected
            if (!OSC.state.sel_sig_name && !$('#right_menu .not-signal').hasClass('active')) {
                $('#right_menu .menu-btn.' + OSC.state.sel_sig_name).addClass('active');
            }
        }

        if (OSC.graphs["ch1"]) {
            OSC.graphs["ch1"].elem.show();
            OSC.graphs["ch1"].plot.setColors(colorsArr);
            OSC.graphs["ch1"].plot.resize();
            OSC.graphs["ch1"].plot.setupGrid();
            OSC.graphs["ch1"].plot.setData(pointArr);
            OSC.graphs["ch1"].plot.draw();
        } else {
            OSC.graphs["ch1"] = {};
            OSC.graphs["ch1"].elem = $('<div class="plot" />').css($('#graph_grid').css(['height', 'width'])).appendTo('#graphs');
            OSC.graphs["ch1"].plot = $.plot(OSC.graphs["ch1"].elem, [pointArr], {
                name: "ch1",
                series: {
                    shadowSize: 0, // Drawing is faster without shadows
                },
                yaxis: {
                    min: -5,
                    max: 5
                },
                xaxis: {
                    min: 0
                },
                grid: {
                    show: false
                },
                colors: [
                    '#FF2A68', '#FF9500', '#FFDB4C', '#87FC70', '#22EDC7', '#1AD6FD', '#C644FC', '#52EDC7', '#EF4DB6'
                ]
            });
            // If page not full loaded
            if (OSC.graphs["ch1"].elem === undefined){
                OSC.graphs = {};
            }
        }

        visible_plots.push(OSC.graphs["ch1"].elem[0]);
        visible_info += (visible_info.length ? ',' : '') + '.' + "ch1";

        var arr_show = ["CH1_SHOW", "CH2_SHOW", "CH3_SHOW", "CH4_SHOW", "MATH_SHOW", "OUTPUT1_SHOW", "OUTPUT2_SHOW"];
        var arr_show2 = ["ch1", "ch2", "ch3", "ch4", "math", "output1", "output2"];
        for (var i = 0; i < 7; i++) {
            if (OSC.params.orig[arr_show[i]] && OSC.params.orig[arr_show[i]].value)
                $('#info').find("." + arr_show2[i]).show();
            else
                $('#info').find("." + arr_show2[i]).hide();
        }

        // Hide plots without signal
        $('#graphs .plot').not(visible_plots).hide();

        // Disable buttons related to inactive signals
        // $('#right_menu .menu-btn').not(visible_btns).not('.not-signal').prop('disabled', true);

        // Show only information about active signals

        // Reset resize flag
        OSC.state.resized = false;

        // Check if selected signal is still visible
        if (OSC.state.sel_sig_name && OSC.graphs[OSC.state.sel_sig_name] && !OSC.graphs[OSC.state.sel_sig_name].elem.is(':visible')) {
            $('#right_menu .menu-btn.active.' + OSC.state.sel_sig_name).removeClass('active');
        }
    };



    // Exits from editing mode
    OSC.exitEditing = function(noclose,event) {
        var id =  "undefined"
        if (event != undefined){
            if (event.id != undefined){
                id = event.id
            }

            if (event.currentTarget != undefined){
                id = event.currentTarget.id
            }
        }

        var id_original = id
        console.log("exitEditing(id:",id,")")
        if ($('#math_dialog').is(':visible')) {
            //for values == abs, dy/dt, ydt (5, 6, 7) deselect and disable signal2 buttons
            var radios = $('input[name="OSC_MATH_SRC2"]');
            var field = $('#OSC_MATH_OP');
            var value = field.val();
            if (value >= 5) {
                radios.closest('.btn-group').children('.btn').addClass('disabled');
            } else {
                radios.closest('.btn-group').children('.btn').removeClass('disabled');
            }
        }

        // Trim keys of radio buttons
        if (id.startsWith("OSC_TRIG_SLOPE")){
            id = "OSC_TRIG_SLOPE"
        }

        if (id.startsWith("OSC_TRIG_SWEEP")){
            id = "OSC_TRIG_SWEEP"
        }

        if (id.startsWith("OSC_MATH_SRC1")){
            id = "OSC_MATH_SRC1"
        }

        if (id.startsWith("OSC_MATH_SRC2")){
            id = "OSC_MATH_SRC2"
        }

        for(var i = 1 ; i <= OSC.adc_channes; i++){
            if (id.startsWith("OSC_CH"+i+"_IN_GAIN")){
                id = "OSC_CH"+i+"_IN_GAIN"
            }
            if (id.startsWith("OSC_CH"+i+"_IN_AC_DC")){
                id = "OSC_CH"+i+"_IN_AC_DC"
            }
        }

        if (id in OSC.params.orig) {
            var key = id;
            var field = $('#' + key);
            var value = undefined;

            if (key == 'OSC_RUN') {
                value = (field.is(':visible') ? 0 : 1);
            } else if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
                value = field.val();
            } else if (field.is('button')) {
                value = (field.hasClass('active') ? 1 : 0);
            } else if (field.is('input:radio')) {
                value = $('input[name="' + key + '"]:checked').val();
            }

            if (key == "OSC_CH1_OFFSET") {
                var units = $('#OSC_CH1_OFFSET_UNIT').html();
                var divider = units == "mV" ? 1000 : 1;
                value /= divider;
            }

            if (key == "OSC_CH2_OFFSET") {
                var units = $('#OSC_CH2_OFFSET_UNIT').html();
                var divider = units == "mV" ? 1000 : 1;
                value /= divider;
            }

            if (OSC.adc_channes > 2)
            if (key == "OSC_CH3_OFFSET") {
                var units = $('#OSC_CH3_OFFSET_UNIT').html();
                var divider = units == "mV" ? 1000 : 1;
                value /= divider;
            }

            if (OSC.adc_channes > 3)
            if (key == "OSC_CH4_OFFSET") {
                var units = $('#OSC_CH4_OFFSET_UNIT').html();
                var divider = units == "mV" ? 1000 : 1;
                value /= divider;
            }

            for(var i = 1 ; i <= OSC.adc_channes; i++){
                if (id == "OSC_CH"+i+"_IN_GAIN"){

                }
            }

            if (key == "SOUR1_VOLT") {
                value = parseFloat(value);
                var maxAmp = parseFloat($("#"+key).attr("max"));
                if (value > maxAmp){
                    value = maxAmp;
                }
            }

            if (key == "SOUR2_VOLT") {
                value = parseFloat(value);
                var maxAmp = parseFloat($("#"+key).attr("max"));
                if (value > maxAmp){
                    value = maxAmp;
                }
            }

            if (key == "SOUR1_SWEEP_TIME") {
                if (!String(value).includes("s")){
                    OSC.state.sweep_ch1_time = value
                }
                value = OSC.state.sweep_ch1_time
            }

            if (key == "SOUR2_SWEEP_TIME") {
                if (!String(value).includes("s")){
                    OSC.state.sweep_ch2_time = value
                }
                value = OSC.state.sweep_ch2_time
            }

            if (key == "SOUR1_VOLT_OFFS") {
                if (OSC.rp_model == "Z20") {
                    value = 0;
                } else {
                    value = parseFloat(value);
                    var maxAmp = parseFloat($("#"+key).attr("max"));
                    var minAmp = parseFloat($("#"+key).attr("min"));
                    if (value > maxAmp){
                        value = maxAmp;
                    }
                    if (value < minAmp){
                        value = minAmp;
                    }
                }
            }

            if (key == "SOUR2_VOLT_OFFS") {
                if (OSC.rp_model == "Z20") {
                    value = 0;
                } else {
                    value = parseFloat(value);
                    var maxAmp = parseFloat($("#"+key).attr("max"));
                    var minAmp = parseFloat($("#"+key).attr("min"));
                    if (value > maxAmp){
                        value = maxAmp;
                    }
                    if (value < minAmp){
                        value = minAmp;
                    }
                }
            }

            if (key == "OSC_MATH_OFFSET") {
                value = OSC.convertMathUnitToValue();
            }

            if (value !== undefined && value != OSC.params.orig[key].value) {
                console.log(key + ' changed from ' + OSC.params.orig[key].value + ' to ' + ($.type(OSC.params.orig[key].value) == 'boolean' ? !!value : value));
                OSC.params.local[key] = { value: ($.type(OSC.params.orig[key].value) == 'boolean' ? !!value : value) };
            }
        }

        // Check changes in measurement list
        try {
            if (id.startsWith("meas_")){
                OSC.params.local['OSC_MEAS_SELN'] = { value: JSON.stringify(OSC.handleMeasureList()) };
            }
        } catch (e) {
            console.log(e);
        }

        // Send params then reset editing state and hide dialog
        OSC.sendParams();
        OSC.state.editing = false;
        if (noclose) return;
        $('.dialog:visible').hide();
        $('#right_menu').show();
    };

    // Sends to server modified parameters
    OSC.sendParams = function(disable_defCur = false) {
        if ($.isEmptyObject(OSC.params.local)) {
            return false;
        }

        if (!OSC.state.socket_opened) {
            console.log('ERROR: Cannot save changes, socket not opened');
            return false;
        }
        //if (!disable_defCur) OSC.setDefCursorVals();

        // Hack for json limitation
        if ('OSC_TIME_SCALE' in OSC.params.local){
            OSC.params.local['OSC_TIME_SCALE'].value = OSC.params.local['OSC_TIME_SCALE'].value * 1000;
        }

        console.log("sendParams",OSC.params.local)
        OSC.ws.send(JSON.stringify({ parameters: OSC.params.local }));
        OSC.params.local = {};
        return true;
    };

    OSC.requestAllParam = function(disable_defCur = false) {
        if (!OSC.state.socket_opened) {
            console.log('ERROR: Cannot save changes, socket not opened');
            return false;
        }
        OSC.params.local['in_command'] = { value: 'send_all_params' };
        OSC.ws.send(JSON.stringify({ parameters: OSC.params.local }));
        OSC.params.local = {};
        console.log("requestAllParam")
        return true;
    };

    // Draws the grid on the lowest canvas layer
    OSC.drawGraphGrid = function() {
        var graph = $('#graphs')
        var graph_grid = $('#graph_grid')
        if (graph.length === 0) return;
        if (graph_grid.length  === 0) return;

        var canvas_width = $('#graphs').width() - 2;
        var canvas_height = window.innerHeight - 300; // Math.round(canvas_width / 2.5);

        var center_x = canvas_width / 2;
        var center_y = canvas_height / 2;

        var ctx = $('#graph_grid')[0].getContext('2d');

        var x_offset = 0;
        var y_offset = 0;

        // Set canvas size
        ctx.canvas.width = canvas_width;
        ctx.canvas.height = canvas_height;

        // Set draw options
        ctx.beginPath();
        ctx.lineWidth = 1;
        ctx.strokeStyle = '#5d5d5c';

        // Draw ticks
        for (var i = 1; i < 50; i++) {
            x_offset = x_offset + (canvas_width / 50);
            y_offset = y_offset + (canvas_height / 50);

            if (i == 25) {
                continue;
            }

            ctx.moveTo(x_offset, canvas_height - 3);
            ctx.lineTo(x_offset, canvas_height);

            ctx.moveTo(0, y_offset);
            ctx.lineTo(3, y_offset);
        }

        // Draw lines
        x_offset = 0;
        y_offset = 0;

        for (var i = 1; i < 10; i++) {
            x_offset = x_offset + (canvas_height / 10);
            y_offset = y_offset + (canvas_width / 10);

            if (i == 5) {
                continue;
            }

            ctx.moveTo(y_offset, 0);
            ctx.lineTo(y_offset, canvas_height);

            ctx.moveTo(0, x_offset);
            ctx.lineTo(canvas_width, x_offset);
        }

        ctx.stroke();

        // Draw central cross
        ctx.beginPath();
        ctx.lineWidth = 1;
        ctx.strokeStyle = '#999';

        ctx.moveTo(center_x, 0);
        ctx.lineTo(center_x, canvas_height);

        ctx.moveTo(0, center_y);
        ctx.lineTo(canvas_width, center_y);

        ctx.stroke();
    };

    // Changes Y zoom/scale for the selected signal
    OSC.changeYZoom = function(direction, curr_scale, send_changes) {

        // Output 1/2 signals do not have zoom
        if ($.inArray(OSC.state.sel_sig_name, ['ch1', 'ch2','ch3', 'ch4', 'math', 'output1', 'output2']) < 0) {
            return;
        }

        var mult = 1;
        var math_range = 10;
        var math_min = 0;
        var scale_list = OSC.voltage_steps;
        var math_sig = false;

        if (OSC.state.sel_sig_name.toUpperCase() === 'MATH') {
            math_min = OSC.params.orig['OSC_MATH_SCALE'] !== undefined ?  OSC.params.orig['OSC_MATH_SCALE'].min  : 0;
            math_range = OSC.params.orig['OSC_MATH_SCALE'] !== undefined ? OSC.params.orig['OSC_MATH_SCALE'].max   : 10;
            mult = math_range;
            math_sig = true;
            scale_list = OSC.voltage_steps_math;
        }

        if (OSC.state.sel_sig_name.toUpperCase() === 'CH1') {
            var probeAttenuation = parseInt($("#OSC_CH1_PROBE option:selected").text());
            var jumperSettings = $("#OSC_CH1_IN_GAIN").parent().hasClass("active") ? 1 : 10;
            mult = probeAttenuation * jumperSettings;
        }

        if (OSC.state.sel_sig_name.toUpperCase() === 'CH2') {
            var probeAttenuation = parseInt($("#OSC_CH2_PROBE option:selected").text());
            var jumperSettings = $("#OSC_CH2_IN_GAIN").parent().hasClass("active") ? 1 : 10;
            mult = probeAttenuation * jumperSettings;
        }

        if (OSC.state.sel_sig_name.toUpperCase() === 'CH3') {
            var probeAttenuation = parseInt($("#OSC_CH3_PROBE option:selected").text());
            var jumperSettings = $("#OSC_CH3_IN_GAIN").parent().hasClass("active") ? 1 : 10;
            mult = probeAttenuation * jumperSettings;
        }

        if (OSC.state.sel_sig_name.toUpperCase() === 'CH4') {
            var probeAttenuation = parseInt($("#OSC_CH4_PROBE option:selected").text());
            var jumperSettings = $("#OSC_CH4_IN_GAIN").parent().hasClass("active") ? 1 : 10;
            mult = probeAttenuation * jumperSettings;
        }

        if (OSC.rp_model === "Z20_250_12" || OSC.rp_model == "Z20_250_12_120") {
            mult = 1;
        }

        var curr_scale = (curr_scale === undefined ? OSC.params.orig['OSC_' + OSC.state.sel_sig_name.toUpperCase() + '_SCALE'].value : curr_scale) / mult;
        curr_scale = curr_scale.toFixed(4)

        var new_scale;

        for (var i = 0; i < scale_list.length - 1; i++) {

            if (OSC.state.fine && (curr_scale == scale_list[i] || (curr_scale > scale_list[i] && curr_scale < scale_list[i + 1]) || (curr_scale == scale_list[i + 1] && direction == '-'))) {
                new_scale = curr_scale + (OSC.scale_list[i + 1] / 100) * (direction == '-' ? -1 : 1);
                // Do not allow values smaller than the lowest possible one
                if (new_scale < scale_list[0]) {
                    new_scale = scale_list[0];
                }
                break;
            }

            if (!OSC.state.fine && curr_scale == scale_list[i]) {
                new_scale = scale_list[direction == '-' ? (i > 0 ? i - 1 : 0) : i + 1];
                break;
            } else if (!OSC.state.fine && ((curr_scale > scale_list[i] && curr_scale < scale_list[i + 1]) || (curr_scale == scale_list[i + 1] && i == scale_list.length - 2))) {
                new_scale = scale_list[direction == '-' ? i : i + 1];
                break;
            }
        }

        // if (OSC.rp_model != "Z20_250_12") {
        if (new_scale === undefined) {
            if (curr_scale < scale_list[0]) {
                new_scale = scale_list[0];
            }
            if (curr_scale > scale_list[scale_list.length - 1]) {
                new_scale = scale_list[scale_list.length - 1];
            }
        }
        // }


        if (new_scale !== undefined && new_scale > 0 && new_scale != curr_scale) {
            new_scale *= mult;

            // Fix float length
            //      new_scale = parseFloat(new_scale.toFixed(OSC.state.fine ? 5 : 3));
            if (send_changes !== false) {
                OSC.params.local['OSC_' + OSC.state.sel_sig_name.toUpperCase() + '_SCALE'] = { value: new_scale };
                if (OSC.params.orig['OSC_' + OSC.state.sel_sig_name.toUpperCase() + '_OFFSET'] != undefined) {
                    var cur_offset = OSC.params.orig['OSC_' + OSC.state.sel_sig_name.toUpperCase() + '_OFFSET'].value;
                    var new_offset = (cur_offset / curr_scale) * (new_scale / mult);
                    if (!math_sig)
                        OSC.params.local['OSC_' + OSC.state.sel_sig_name.toUpperCase() + '_OFFSET'] = { value: new_offset };
                }
                OSC.sendParams();
            }
            return new_scale;
        }

        return null;
    };

    // Changes X zoom/scale for all signals
    OSC.changeXZoom = function(direction, curr_scale, send_changes) {
        var curr_scale = (curr_scale === undefined ? OSC.params.orig['OSC_TIME_SCALE'].value : curr_scale);
        var new_scale;

        for (var i = 0; i < OSC.time_steps.length - 1; i++) {
            if (OSC.state.fine && (curr_scale == OSC.time_steps[i] || (curr_scale > OSC.time_steps[i] && curr_scale < OSC.time_steps[i + 1]) || (curr_scale == OSC.time_steps[i + 1] && direction == '-'))) {
                new_scale = parseFloat(curr_scale) + parseFloat((OSC.time_steps[i + 1] / 100)) * parseFloat((direction == '-' ? -1 : 1));
                // Do not allow values smaller than the lowest possible one
                if (new_scale < OSC.time_steps[0]) {
                    new_scale = OSC.time_steps[0];
                }
                break;
            }

            if (!OSC.state.fine && curr_scale == OSC.time_steps[i]) {
                new_scale = OSC.time_steps[direction == '-' ? (i > 0 ? i - 1 : 0) : i + 1];
                break;
            } else if (!OSC.state.fine && ((curr_scale > OSC.time_steps[i] && curr_scale < OSC.time_steps[i + 1]) || (curr_scale == OSC.time_steps[i + 1] && i == OSC.time_steps.length - 2))) {
                new_scale = OSC.time_steps[direction == '-' ? i : i + 1]
                break;
            }
        }

        if (new_scale !== undefined && new_scale > 0 && new_scale != curr_scale) {

            // Fix float length
            new_scale = parseFloat(new_scale.toFixed(OSC.state.fine ? 8 : 6));
            if (send_changes !== false) {
                var new_offset = OSC.params.orig['OSC_TIME_OFFSET'].value * new_scale / OSC.params.orig['OSC_TIME_SCALE'].value;
                if (new_scale < 500)
                    OSC.params.local['OSC_TIME_OFFSET'] = { value: new_offset };
                OSC.params.local['OSC_TIME_SCALE'] = { value: new_scale + "" };
                OSC.sendParams();
            }
            return new_scale;
        }
        return null;
    };


    // Updates Y offset in the signal config dialog, if opened, or saves new value
    OSC.updateYOffset = function(ui, save) {
        var graph_height = $('#graph_grid').outerHeight();
        var zero_pos = (graph_height + 7) / 2;
        var new_value;

        if (ui.helper[0].id == 'ch1_offset_arrow' || ui.helper[0].id == "ch2_offset_arrow" || ui.helper[0].id == "ch3_offset_arrow" || ui.helper[0].id == "ch4_offset_arrow") {
            var ch_n = ''
            if (ui.helper[0].id == 'ch1_offset_arrow') ch_n = "1"
            if (ui.helper[0].id == 'ch2_offset_arrow') ch_n = "2"
            if (ui.helper[0].id == 'ch3_offset_arrow') ch_n = "3"
            if (ui.helper[0].id == 'ch4_offset_arrow') ch_n = "4"
            // var ch_n = (ui.helper[0].id == 'ch1_offset_arrow') ? "1" : "2";

            var volt_per_px = (OSC.params.orig['OSC_CH' + ch_n + '_SCALE'].value * 10) / graph_height;
            new_value = (zero_pos - ui.position.top + parseInt(ui.helper.css('margin-top')) / 2) * volt_per_px;

            $('#info_box').html('IN' + ch_n + ' zero offset ' + OSC.convertVoltage(new_value));

            if ($('#in' + ch_n + '_dialog').is(':visible')) {
                var units = $('#OSC_CH' + ch_n + '_OFFSET_UNIT').html();
                var multiplier = units == "mV" ? 1000 : 1;

                var probeAttenuation = parseInt($("#OSC_CH" + ch_n + "_PROBE option:selected").text());
                var jumperSettings = $("#OSC_CH" + ch_n + "_IN_GAIN").parent().hasClass("active") ? 1 : 20;
                OSC.setValue($('#OSC_CH' + ch_n + '_OFFSET'), OSC.formatInputValue(new_value * multiplier, probeAttenuation, units == "mV", jumperSettings == 20));
            }
            OSC.params.local['OSC_CH' + ch_n + '_OFFSET'] = { value: new_value };
        } else if (ui.helper[0].id == 'output1_offset_arrow' || ui.helper[0].id == 'output2_offset_arrow') {
            var volt_per_px = 10 / graph_height;
            var ch_n = (ui.helper[0].id == 'output1_offset_arrow') ? "1" : "2";

            new_value = (zero_pos - ui.position.top + parseInt(ui.helper.css('margin-top')) / 2) * volt_per_px;
            $('#info_box').html('OUT' + ch_n + ' zero offset ' + OSC.convertVoltage(new_value));
            if (save) {
                OSC.params.local['OUTPUT' + ch_n + '_SHOW_OFF'] = { value: new_value };
            }
        } else if (ui.helper[0].id == 'math_offset_arrow') {
            var volt_per_px = (OSC.params.orig['OSC_MATH_SCALE'].value * 10) / graph_height;

            new_value = (zero_pos - ui.position.top + parseInt(ui.helper.css('margin-top')) / 2) * volt_per_px;
            $('#info_box').html('MATH zero offset ' + OSC.convertVoltage(new_value));

            if ($('#math_dialog').is(':visible'))
                OSC.convertValueToMathUnit(new_value);

            OSC.params.local['OSC_MATH_OFFSET'] = { value: new_value };
        }

        if (new_value !== undefined && save)
            OSC.sendParams();
    };

    OSC.resize = function() {
        if ($('#global_container').length === 0) return
        if ($('#main').length === 0) return

        var window_width = window.innerWidth;
        var window_height = window.innerHeight;

        // if (window_width > 768 && window_height > 580) {
        //     var global_width = window_width - 30,
        //         global_height = global_width / 1.77885;
        //     if (window_height < global_height) {
        //         global_height = window_height - 70 * 1.77885;
        //         global_width = global_height * 1.77885;
        //     }

        //     $('#global_container').css('width', global_width);
        //     $('#global_container').css('height', global_height);


        //     OSC.drawGraphGrid();
        //     var main_width = $('#main').outerWidth(true);
        //     var main_height = $('#main').outerHeight(true);
        //     $('#global_container').css('width', main_width);
        //     $('#global_container').css('height', main_height);

        //     OSC.drawGraphGrid();
        //     main_width = $('#main').outerWidth(true);
        //     main_height = $('#main').outerHeight(true);
        //     window_width = window.innerWidth;
        //     window_height = window.innerHeight;
        //     console.log("window_width = " + window_width);
        //     console.log("window_height = " + window_height);
        //     if (main_height > (window_height - 80)) {
        //         $('#global_container').css('height', window_height - 80);
        //         $('#global_container').css('width', 1.82 * (window_height - 80));
        //         OSC.drawGraphGrid();
        //         $('#global_container').css('width', $('#main').outerWidth(true) - 2);
        //         $('#global_container').css('height', $('#main').outerHeight(true) - 2);
        //         OSC.drawGraphGrid();
        //     }
        // }

        var global_width = window_width - 30,
            global_height = window_height - 200;

        //     if (window_height < global_height) {
        //         global_height = window_height - 70 * 1.77885;
        //         global_width = global_height * 1.77885;
        //     }
        $('#global_container').css('width', global_width);
        $('#global_container').css('height', global_height);

        $('#main').css('width', global_width - 190);
        $('#main').css('height', global_height);

        OSC.drawGraphGrid();

        $(window).on('focus', function() {
            // console.log("Tab FOCUS");
            OSC.drawGraphGrid();
        });

        $(window).on('blur', function() {
            // console.log("Tab BLUR");
        });


        $('#global_container').offset({ left: (window_width - $('#global_container').width()) / 2 });

        if ($('.plot').length !== 0){
            // Resize the graph holders
            $('.plot').css($('#graph_grid').css(['height', 'width']));

            // Hide all graphs, they will be shown next time signal data is received
            $('#graphs .plot').hide();
        }

        // Hide offset arrows, trigger level line and arrow
        $('.y-offset-arrow, #time_offset_arrow, #buf_time_offset, #trig_level_arrow, #trigger_level').hide();

        OSC.requestAllParam();

        // Reset left position for trigger level arrow, it is added by jQ UI draggable
        $('#trig_level_arrow').css('left', '');
        // Set the resized flag
        OSC.state.resized = true;
        OSC.updateJoystickPosition();

    };




}(window.OSC = window.OSC || {}, jQuery));

// Page onload event handler
$(function() {

    var reloaded = $.cookie("osc_forced_reload");
    if (reloaded == undefined || reloaded == "false") {
        $.cookie("osc_forced_reload", "true");
        window.location.reload(true);
    }


    OSC.client_id = 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
        var r = Math.random() * 16 | 0,
            v = c == 'x' ? r : (r & 0x3 | 0x8);
        return v.toString(16);
    });

    $.ajax({
        url: '/set_client_id', //Server script to process data
        type: 'POST',
        //Ajax events
        //beforeSend: beforeSendHandler,
        success: function(e) { console.log(e); },
        error: function(e) { console.log(e); },
        // Form data
        data: OSC.client_id,
        //Options to tell jQuery not to process data or worry about content-type.
        cache: false,
        contentType: false,
        processData: false
    });

    OSC.checkStatus();


    $('#modal-warning').hide();




    // Touch events
    // $(document).on('touchstart', '.plot', function(ev) {
    //     ev.preventDefault();

    //     // Multi-touch is used for zooming
    //     if (!OSC.touch.start && ev.originalEvent.touches.length > 1) {
    //         OSC.touch.zoom_axis = null;
    //         OSC.touch.start = [
    //             { clientX: ev.originalEvent.touches[0].clientX, clientY: ev.originalEvent.touches[0].clientY },
    //             { clientX: ev.originalEvent.touches[1].clientX, clientY: ev.originalEvent.touches[1].clientY }
    //         ];
    //     }
    //     // Single touch is used for changing offset
    //     else if (!OSC.state.simulated_drag) {
    //         OSC.state.simulated_drag = true;
    //         OSC.touch.offset_axis = null;
    //         OSC.touch.start = [
    //             { clientX: ev.originalEvent.touches[0].clientX, clientY: ev.originalEvent.touches[0].clientY }
    //         ];
    //     }
    // });

    // $(document).on('touchmove', '.plot', function(ev) {
    //     ev.preventDefault();

    //     // Multi-touch is used for zooming
    //     if (ev.originalEvent.touches.length > 1) {

    //         OSC.touch.curr = [
    //             { clientX: ev.originalEvent.touches[0].clientX, clientY: ev.originalEvent.touches[0].clientY },
    //             { clientX: ev.originalEvent.touches[1].clientX, clientY: ev.originalEvent.touches[1].clientY }
    //         ];

    //         // Find zoom axis
    //         if (!OSC.touch.zoom_axis) {
    //             var delta_x = Math.abs(OSC.touch.curr[0].clientX - OSC.touch.curr[1].clientX);
    //             var delta_y = Math.abs(OSC.touch.curr[0].clientY - OSC.touch.curr[1].clientY);

    //             if (Math.abs(delta_x - delta_y) > 10) {
    //                 if (delta_x > delta_y) {
    //                     OSC.touch.zoom_axis = 'x';
    //                 } else if (delta_y > delta_x) {
    //                     OSC.touch.zoom_axis = 'y';
    //                 }
    //             }
    //         }

    //         // Skip first touch event
    //         if (OSC.touch.prev) {
    //             // Time zoom
    //             if (OSC.touch.zoom_axis == 'x') {
    //                 var prev_delta_x = Math.abs(OSC.touch.prev[0].clientX - OSC.touch.prev[1].clientX);
    //                 var curr_delta_x = Math.abs(OSC.touch.curr[0].clientX - OSC.touch.curr[1].clientX);

    //                 if (OSC.state.fine || Math.abs(curr_delta_x - prev_delta_x) > $(this).width() * 0.9 / OSC.time_steps.length) {
    //                     var new_scale = OSC.changeXZoom((curr_delta_x < prev_delta_x ? '+' : '-'), OSC.touch.new_scale_x, true);

    //                     if (new_scale !== null) {
    //                         OSC.touch.new_scale_x = new_scale;
    //                         $('#info_box').html('Time scale ' + OSC.convertTime(new_scale) + '/div');
    //                     }

    //                     OSC.touch.prev = OSC.touch.curr;
    //                 }
    //             }
    //             // Voltage zoom
    //             else if (OSC.touch.zoom_axis == 'y' && OSC.state.sel_sig_name) {
    //                 var prev_delta_y = Math.abs(OSC.touch.prev[0].clientY - OSC.touch.prev[1].clientY);
    //                 var curr_delta_y = Math.abs(OSC.touch.curr[0].clientY - OSC.touch.curr[1].clientY);

    //                 if (OSC.state.fine || Math.abs(curr_delta_y - prev_delta_y) > $(this).height() * 0.9 / OSC.voltage_steps.length) {
    //                     var new_scale = OSC.changeYZoom((curr_delta_y < prev_delta_y ? '+' : '-'), OSC.touch.new_scale_y, true);

    //                     if (new_scale !== null) {
    //                         OSC.touch.new_scale_y = new_scale;
    //                         $('#info_box').html('Vertical scale ' + OSC.convertVoltage(new_scale) + '/div');
    //                     }

    //                     OSC.touch.prev = OSC.touch.curr;
    //                 }
    //             }
    //         } else if (OSC.touch.prev === undefined) {
    //             OSC.touch.prev = OSC.touch.curr;
    //         }
    //     }
    //     // Single touch is used for changing offset
    //     else if (OSC.state.simulated_drag) {

    //         // Find offset axis
    //         if (!OSC.touch.offset_axis) {
    //             var delta_x = Math.abs(OSC.touch.start[0].clientX - ev.originalEvent.touches[0].clientX);
    //             var delta_y = Math.abs(OSC.touch.start[0].clientY - ev.originalEvent.touches[0].clientY);

    //             if (delta_x > 5 || delta_y > 5) {
    //                 if (delta_x > delta_y) {
    //                     OSC.touch.offset_axis = 'x';
    //                 } else if (delta_y > delta_x) {
    //                     OSC.touch.offset_axis = 'y';
    //                 }
    //             }
    //         }

    //         if (OSC.touch.prev) {

    //             // Time offset
    //             if (OSC.touch.offset_axis == 'x') {
    //                 var delta_x = ev.originalEvent.touches[0].clientX - OSC.touch.prev[0].clientX;
    //                 if (delta_x != 0)
    //                     $('#time_offset_arrow').simulate('drag', { dx: delta_x, dy: 0 });
    //             }
    //             // Voltage offset
    //             else if (OSC.touch.offset_axis == 'y' && OSC.state.sel_sig_name) {
    //                 var delta_y = ev.originalEvent.touches[0].clientY - OSC.touch.prev[0].clientY;
    //                 if (delta_y != 0)
    //                     $('#' + OSC.state.sel_sig_name + '_offset_arrow').simulate('drag', { dx: 0, dy: delta_y });
    //             }
    //         }

    //         OSC.touch.prev = [
    //             { clientX: ev.originalEvent.touches[0].clientX, clientY: ev.originalEvent.touches[0].clientY }
    //         ];
    //     }
    // });

    // $(document).on('touchend', '.plot', function(ev) {
    //     ev.preventDefault();

    //     if (OSC.state.simulated_drag) {
    //         OSC.state.simulated_drag = false;

    //         if (OSC.touch.offset_axis == 'x') {
    //             //$('#time_offset_arrow').simulate('drag', { dx: 0, dy: 0 });
    //             $('#buf_time_offset').simulate('drag', { dx: 0, dy: 0 });
    //         } else if (OSC.touch.offset_axis == 'y' && OSC.state.sel_sig_name) {
    //             $('#' + OSC.state.sel_sig_name + '_offset_arrow').simulate('drag', { dx: 0, dy: 0 });
    //         }

    //         delete OSC.touch.start;
    //         delete OSC.touch.prev;
    //     } else {
    //         // Send new scale
    //         if (OSC.touch.new_scale_y !== undefined) {
    //             OSC.params.local['OSC_' + OSC.state.sel_sig_name.toUpperCase() + '_SCALE'] = { value: OSC.touch.new_scale_y };
    //             OSC.sendParams();
    //         } else if (OSC.touch.new_scale_x !== undefined) {
    //             OSC.params.local['OSC_TIME_SCALE'] = { value: OSC.touch.new_scale_x };
    //             OSC.sendParams();
    //         }
    //     }

    //     // Reset touch information
    //     OSC.touch = {};
    //     $('#info_box').empty();
    // });

    // Bind to the window resize event to redraw the graph; trigger that event to do the first drawing



    $(window).resize(
        OSC.resize).resize();



    // Stop the application when page is unloaded
    $(window).on('beforeunload', function() {
        OSC.ws.onclose = function() {}; // disable onclose handler first
        OSC.ws.close();
        $.ajax({
            url: OSC.config.stop_app_url,
            async: false
        });
        OSC.unexpectedClose = false;
    });

    // Init help
    Help.init(helpListScope);
    Help.setState("idle");

    // Everything prepared, start application
    OSC.startApp();

});
