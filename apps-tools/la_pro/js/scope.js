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
    OSC.param_callbacks = {};
    OSC.startTime = 0;
    OSC.config = {};
    OSC.config.app_id = 'la_pro';
    OSC.config.server_ip = ''; // Leave empty on production, it is used for testing only
    OSC.config.start_app_url = (OSC.config.server_ip.length ? 'http://' + OSC.config.server_ip : '') + '/bazaar?start=' + OSC.config.app_id + '?' + location.search.substr(1);

    OSC.config.start_app_url = window.location.origin + '/bazaar?start=' + OSC.config.app_id;
    OSC.config.stop_app_url = window.location.origin + '/bazaar?stop=' + OSC.config.app_id;
    OSC.config.socket_url = 'ws://' + window.location.host + '/wss';

    OSC.config.graph_colors = {
        'ch1': '#dc1809',
        'ch2': '#e6890c',
        'ch3': '#fed730',
        'ch4': '#00ae73',
        'ch5': '#0760be',
        'ch6': '#846167',
        'ch7': '#6b6f61',
        'ch8': '#ebf1e7',
        'trig': '#EF4DB6'
    };
    OSC.client_id = undefined;
    OSC.redrawTimeout = undefined;
    OSC.latest_signal = {};
    OSC.enabled_channels = [true, true, true, true, true, true, true, true];
    OSC.mouseWheelEventFired = false;

    // Time scale steps in millisecods
    OSC.time_steps = [
        // Nanoseconds
        100 / 1000000, 200 / 1000000, 500 / 1000000,
        // Microseconds
        1 / 1000, 2 / 1000, 5 / 1000, 10 / 1000, 20 / 1000, 50 / 1000, 100 / 1000, 200 / 1000, 500 / 1000,
        // Millisecods
        1, 2, 5, 10, 20, 50, 100, 200, 500,
        // Seconds
        1 * 1000, 2 * 1000, 5 * 1000, 10 * 1000, 20 * 1000, 50 * 1000
    ];

    // Voltage scale steps in minorgrid values
    OSC.voltage_steps = [0.2, 0.4, 0.8, 1.6];
    OSC.voltage_index = 2;
    OSC.voltage_offset = [4, 3, 2, 1, 0, -1, -2, -3];
    OSC.counts_offset = 0;

    OSC.bad_connection = [false, false, false, false]; // time in s.

    OSC.compressed_data = 0;
    OSC.decompressed_data = 0;
    OSC.refresh_times = [
        [],
        []
    ];
    OSC.ch_names = ["DIN0", "DIN1", "DIN2", "DIN3", "DIN4", "DIN5", "DIN6", "DIN7"];
    OSC.log_buses = [false, false, false, false];

    // Sampling rates
    OSC.sample_rates = ['125M', '15.625M', '1.953M', '122.070k', '15.258k', '1.907k'];
    OSC.triggers_list_id = [];
    OSC.triggers_list_action = [];
    OSC.triggers_list_channel = [];
    OSC.triggers_count = 0;
    OSC.current_bus = "bus-1";

    OSC.triggers_list = [-1, -1, -1, -1, -1, -1, -1, -1];

    OSC.loaderShow = false;
    OSC.max_freq = 125e6;
    // App state
    OSC.state = {
        socket_opened: false,
        processing: false,
        editing: false,
        trig_dragging: false,
        cursor_dragging: false,
        resized: false,
        sel_sig_name: 'ch1',
        fine: false,
        graph_grid_height: null,
        graph_grid_width: null,
        calib: 0,
        bus_editing: 0,
        decoder_id: 1,
        radix: 17,
        export_radix: 16,
        acq_speed: OSC.max_freq,
        line_moving: false
    };

    OSC.buses = {};
    OSC.buses.bus1 = {};
    OSC.buses.bus2 = {};
    OSC.buses.bus3 = {};
    OSC.buses.bus4 = {};

    OSC.recv_signals = {};
    OSC.scale_index = 7;
    OSC.scales = [0.01, 0.05, 0.1, 0.3, 0.5, 0.6, 0.8, 1, 3, 5, 7, 8, 10, 15, 20, 25, 50, 100];

    // Params cache
    OSC.params = {
        orig: {},
        local: {}
    };

    //  OSC.laMode = 0;
    OSC.measureMode = 0;

    // Other global variables
    OSC.ws = null;
    OSC.graphs = {};
    OSC.touch = {};
    OSC.rp_model = "";
    OSC.connect_time;
    OSC.offsetForDecoded = 0;

    OSC.was_loaded = false;

    OSC.decoders_array = [];
    OSC.loaded_ind = 0;
    OSC.time_offset_str = "";
    OSC.time_scale = 1;
    OSC.unexpectedClose = true;
    OSC.scrollLogContainer = 0;
    OSC.ch1_size = 0;

    OSC.parameterStack = [];
    OSC.signalStack = [];

    OSC.trigger_position = 0;
    OSC.flagWheelHandled = false;
    OSC.allSignalShown = false;
    OSC.state.decimate = 1;
    // OSC.demoDecodersCreated = false;
    // OSC.demo_mode = false;
    OSC.samples_sum = 512;
    OSC.samples_first = true;


    OSC.scaleWasChanged = false;
    OSC.splitted_signal = {};
    OSC.splittedAvgSignal = {};

    // Starts the oscilloscope application on server
    OSC.startApp = function() {
        // Reset dropdown protocol_selector (this is fix for Firefox)
        $('#protocol_selector').prop('selectedIndex', 0);

        // Delete old datalogic file
        $.get("/lapro_rm_datafile");

        $.get(
                OSC.config.start_app_url
            )
            .done(function(dresult) {
                if (dresult.status == 'OK') {
                    OSC.connectWebSocket();
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

    OSC.guiHandler = function() {
        if (OSC.signalStack.length > 0) {
            OSC.recv_signals = OSC.signalStack[OSC.signalStack.length - 1];
            OSC.signalStack.splice(0, 1);
        } else {
            if (OSC.latest_signal['ch1'] != undefined)
                OSC.recv_signals = OSC.latest_signal;
            else
                return;
        }
        if (OSC.scaleWasChanged && OSC.time_scale < 0.25) {
            var base = 1 / OSC.time_scale;
            OSC.splittedAvgSignal = OSC.repackSignalsAVG(OSC.recv_signals, base);
            OSC.splitted_signal = OSC.splitSignals(OSC.recv_signals);
        } else {
            OSC.splitted_signal = OSC.splitSignals(OSC.recv_signals);
        }

        OSC.processSignals(OSC.splitted_signal);
        OSC.refresh_times.push("tick");
        if (OSC.scaleWasChanged)
            OSC.scrollDataArea();
        OSC.scaleWasChanged = false;
        if (OSC.signalStack.length > 2)
            OSC.signalStack.length = [];

        $('.data_row').unbind('click', clickDivListedData);
        $('.data_row').bind('click', clickDivListedData);
        OSC.flagWheelHandled = false;
    }

    OSC.scrollDataArea = function() {
        // Scroll
        var div_index = 0;
        COMMON.oldResult = [];
        for (var i = 0; i < COMMON.savedResultArr.length; i++) {
            if ((COMMON.savedResultArr[i].abspos) < OSC.counts_offset)
                div_index++;
            else
                break;
        }
        $('#log-container').scrollTop(20 * div_index);
    }



    var clickDivListedData = function() {
        var offset = $(this).attr('offset');
        OSC.counts_offset = parseInt(offset);
        var trigPosInPoints = (OSC.samples_sum * OSC.time_scale - OSC.counts_offset) * $('#graphs').width() / 1024 - $('#time_offset_arrow').width() / 2;
        $('#time_offset_arrow').css('left', trigPosInPoints);
        // I cry when I see this code :'(
        OSC.guiHandler();
        $('#OSC_TIME_OFFSET').text(OSC.convertTime(5 * (($('#time_offset_arrow').position().left + $('#time_offset_arrow').width() / 2) / ($('#graphs').width() / 2) - 1) * 102400 / OSC.time_scale / OSC.state.acq_speed));
    }

    var parametersHandler = function() {
        if (OSC.parameterStack.length > 0) {
            var p = performance.now();
            OSC.processParameters(OSC.parameterStack[0]);
            OSC.parameterStack.splice(0, 2);
        }
    }

    // setInterval(OSC.guiHandler, 50);
    setInterval(parametersHandler, 10);

    var g_count = 0;
    var g_time = 0;
    var g_iter = 10;
    var g_delay = 200;
    var g_counter = 0;
    var g_CpuLoad = 100.0;
    var g_TotalMemory = 256.0;
    var g_FreeMemory = 256.0;

    setInterval(function() {
        if (!OSC.state.socket_opened)
            return;
        var now = new Date();
        var now_str = now.getHours() + ":" + now.getMinutes() + ":" + now.getSeconds() + ":" + now.getMilliseconds();
        var times = "";
        for (var i = 0; i < OSC.refresh_times.length; i++)
            times += OSC.refresh_times[i] + " ";

        $('#fps_view').text(OSC.refresh_times.length);
        $('#throughput_view').text((OSC.compressed_data / 1024).toFixed(2) + "kB/s");
        $('#cpu_load').text(g_CpuLoad.toFixed(2) + "%");
        $('#totalmem_view').text((g_TotalMemory / (1024 * 1024)).toFixed(2) + "Mb");
        $('#freemem_view').text((g_FreeMemory / (1024 * 1024)).toFixed(2) + "Mb");
        $('#usagemem_view').text(((g_TotalMemory - g_FreeMemory) / (1024 * 1024)).toFixed(2) + "Mb");


        if (OSC.refresh_times.length < 3)
            OSC.bad_connection[g_counter] = true;
        else
            OSC.bad_connection[g_counter] = false;
        g_counter++;
        if (g_counter == 4) g_counter = 0;


        if ($('#weak_conn_msg').is(':visible')) {
            if (!OSC.bad_connection[0] && !OSC.bad_connection[1] && !OSC.bad_connection[2] && !OSC.bad_connection[3])
                $('#weak_conn_msg').hide();
        } else {
            if (OSC.bad_connection[0] && OSC.bad_connection[1] && OSC.bad_connection[2] && OSC.bad_connection[3]) {
                if ($('#sys_info_view').is(':visible')) {
                    $('#weak_conn_msg').css('bottom', 45);
                }
            }
        }


        OSC.compressed_data = 0;
        OSC.decompressed_data = 0;
        OSC.refresh_times = [];
    }, 1000);


    OSC.convertUnpacked = function(array) {
        var CHUNK_SIZE = 0x8000; // arbitrary number here, not too small, not too big
        var index = 0;
        var length = array.length;
        var result = '';
        var slice;
        while (index < length) {
            slice = Array.prototype.slice.call(array, index, Math.min(index + CHUNK_SIZE, length));
            //slice = array.slice(index, Math.min(index + CHUNK_SIZE, length)); // `Math.min` is not really necessary here I think
            result += String.fromCharCode.apply(null, slice);
            index += CHUNK_SIZE;
        }
        return result;
    }

    OSC.load_params = function() {
        var pref = 'full_';

        var obj;
        obj = $.cookie(pref + 'la_voltage_index');
        if (obj !== undefined)
            OSC.voltage_index = JSON.parse(obj);
        // obj = $.cookie(pref + 'la_counts_offset')
        // if (obj !== undefined)
        //     OSC.counts_offset = JSON.parse(obj);
        // obj = $.cookie(pref + 'la_time_scale')
        // if (obj !== undefined)
        //     OSC.time_scale = JSON.parse(obj);
        obj = $.cookie(pref + 'la_voltage_offset')
        if (obj !== undefined)
            OSC.voltage_offset = JSON.parse(obj);

        obj = $.cookie(pref + 'la_enabled_channels');
        if (obj !== undefined)
            OSC.enabled_channels = JSON.parse(obj);
        obj = $.cookie(pref + 'la_ch_names')
        if (obj !== undefined)
            OSC.ch_names = JSON.parse(obj);
        obj = $.cookie(pref + 'la_bus');
        if (obj !== undefined)
            OSC.buses = JSON.parse(obj);

        obj = $.cookie(pref + 'la_decoder_id');
        if (obj !== undefined)
            OSC.state.decoder_id = JSON.parse(obj);
        obj = $.cookie(pref + 'la_radix');
        if (obj !== undefined) {
            OSC.state.radix = JSON.parse(obj);

            $('#DISPLAY_RADIX option').removeAttr('selected');
            $('#DISPLAY_RADIX option[value=' + OSC.state.radix + "]").attr('selected', 'selected');
            $('#DISPLAY_RADIX').val(OSC.state.radix);
        }
        obj = $.cookie(pref + 'la_export_radix');
        if (obj !== undefined) {
            OSC.state.export_radix = JSON.parse(obj);
            $('#EXPORT_RADIX option').removeAttr('selected');
            $('#EXPORT_RADIX option[value=' + OSC.state.export_radix + "]").attr('selected', 'selected');
            $('#EXPORT_RADIX').val(OSC.state.export_radix);
        }

        obj = $.cookie(pref + 'la_export_buses');
        if (obj !== undefined)
            OSC.log_buses = JSON.parse(obj);

        obj = $.cookie(pref + 'la_acq_speed_value');
        if (obj !== undefined) {
            obj = (obj == 0) ? 1 : obj;
            OSC.state.acq_speed = OSC.max_freq / obj;
            OSC.state.decimate = obj;
            $('#ACQ_SPEED').prop('selectedIndex', Math.log2(obj));

            // Calculate time per division
            var samplerate = OSC.state.acq_speed;
            var samples = 1024;
            var mul = 1000;
            var scale = OSC.time_scale;
            var dev_num = 10;
            var timePerDevInMs = (((samples / scale) / samplerate) * mul) / dev_num;
            $('#OSC_TIME_SCALE').text(OSC.convertTime(timePerDevInMs));
            OSC.sendACQ();
        }

        obj = $.cookie(pref + 'triggers_list');
        if (obj !== undefined) {
            OSC.triggers_list = JSON.parse(obj);
            for (var i = 0; i < OSC.triggers_list.length; i++) {
                $('select[name="din' + i + '"]').val(OSC.triggers_list[i]);
            }
        }

        OSC.sendTrigInfo();

        for (var i = 0; i < OSC.enabled_channels.length; i++) {
            var ch_val = 1;
            obj = $.cookie(pref + 'la_osc_ch_val');
            if (obj !== undefined)
                ch_val = JSON.parse(obj);
            if (OSC.enabled_channels[i]) {
                $('#CH' + (i + 1) + '_ENABLED').find('img').show();
                $('#CH' + (i + 1) + '_NAME').val(OSC.ch_names[i]);
                $('#ch' + (i + 1) + '_offset_arrow').show();
                OSC.updateChVisibility(i);
            } else {
                if (OSC.ch_names[i] != ("DIN" + i))
                    $('#CH' + (i + 1) + '_NAME').val(OSC.ch_names[i]);
                $('#ch' + (i + 1) + '_offset_arrow').hide();
                $('#CH' + (i + 1) + '_ENABLED').find('img').hide();
            }
        }

        for (var i = 1; i < 5; i++) {
            var decoder_obj = {};
            var bus = "bus" + i;

            if (OSC.buses[bus].name !== undefined) {
                if (OSC.buses[bus].enabled) {
                    if (OSC.buses[bus].name == "UART" || OSC.buses[bus].name == "I2C" || OSC.buses[bus].name == "CAN") {
                        decoder_obj['key'] = OSC.buses[bus].name.toLowerCase();
                        decoder_obj['val'] = OSC.buses[bus].decoder;
                        OSC.decoders_array.push(decoder_obj);
                    } else {
                        if (OSC.buses[bus].miso_decoder !== undefined && OSC.buses[bus].miso_decoder != "") {
                            decoder_obj['key'] = OSC.buses[bus].name.toLowerCase();
                            decoder_obj['val'] = OSC.buses[bus].miso_decoder;
                            OSC.decoders_array.push(decoder_obj);
                        }
                        if (OSC.buses[bus].mosi_decoder !== undefined && OSC.buses[bus].mosi_decoder != "") {
                            decoder_obj['key'] = OSC.buses[bus].name.toLowerCase();
                            decoder_obj['val'] = OSC.buses[bus].mosi_decoder;
                            OSC.decoders_array.push(decoder_obj);
                        }
                    }
                }
                $('#BUS' + i + '_NAME').text(OSC.buses[bus].name);
                $('#DATA_BUS' + (i - 1)).text(OSC.buses[bus].name);
                if (OSC.log_buses[i - 1])
                    $('#DATA_BUS' + (i - 1)).addClass('active');
            }
        }

        OSC.was_loaded = true;
        OSC.guiHandler();

        setInterval(function() {
            if (OSC.loaded_ind >= OSC.decoders_array.length)
                return;

            var params = {};

            params['CREATE_DECODER'] = {
                value: OSC.decoders_array[OSC.loaded_ind]['key']
            }
            params['DECODER_NAME'] = {
                value: OSC.decoders_array[OSC.loaded_ind]['val']
            }

            OSC.ws.send(JSON.stringify({
                parameters: params
            }));

            $('#BUS' + (OSC.loaded_ind + 1) + '_ENABLED').find('img').show();

            OSC.loaded_ind++;
        }, 500);
        OSC.updateChVisibility();
    }

    OSC.save_params = function() {
        var pref = 'full_';
        // if (OSC.params.orig['is_demo'].value)
        //     pref = 'demo_';

        var expiresDate = new Date(2220, 1, 1, 0, 0, 0, 0);

        $.cookie(pref + 'la_voltage_index', JSON.stringify(OSC.voltage_index), { expires: expiresDate });
        $.cookie(pref + 'la_counts_offset', JSON.stringify(OSC.counts_offset), { expires: expiresDate });
        $.cookie(pref + 'la_time_scale', JSON.stringify(OSC.time_scale), { expires: expiresDate });
        $.cookie(pref + 'la_voltage_offset', JSON.stringify(OSC.voltage_offset), { expires: expiresDate });

        $.cookie(pref + 'la_enabled_channels', JSON.stringify(OSC.enabled_channels), { expires: expiresDate });
        $.cookie(pref + 'la_ch_names', JSON.stringify(OSC.ch_names), { expires: expiresDate });
        $.cookie(pref + 'la_bus', JSON.stringify(OSC.buses), { expires: expiresDate });

        $.cookie(pref + 'la_decoder_id', JSON.stringify(OSC.state.decoder_id), { expires: expiresDate });
        $.cookie(pref + 'la_radix', JSON.stringify(OSC.state.radix), { expires: expiresDate });
        $.cookie(pref + 'la_export_radix', JSON.stringify(OSC.state.export_radix), { expires: expiresDate });

        $.cookie(pref + 'la_acq_speed', JSON.stringify(OSC.state.acq_speed), { expires: expiresDate });
        $.cookie(pref + 'la_export_buses', JSON.stringify(OSC.log_buses), { expires: expiresDate });

        $.cookie(pref + 'la_acq_speed_value', OSC.state.decimate, { expires: expiresDate });


        $.cookie(pref + 'triggers_list', JSON.stringify(OSC.triggers_list), { expires: expiresDate });
        // $.cookie(pref + 'triggers_list_id', JSON.stringify(OSC.triggers_list_id), { expires: expiresDate });
        // $.cookie(pref + 'triggers_list_action', JSON.stringify(OSC.triggers_list_action), { expires: expiresDate });
        // $.cookie(pref + 'triggers_list_channel', JSON.stringify(OSC.triggers_list_channel), { expires: expiresDate });
        // $.cookie(pref + 'triggers_count', JSON.stringify(OSC.triggers_count), { expires: expiresDate });
    }

    OSC.checkStatusTimer = undefined;
    OSC.changeStatusForRestart = false;
    OSC.changeStatusStep = 0;

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
                OSC.state.socket_opened = true;
                console.log('Socket opened');

                OSC.params.local['in_command'] = {
                    value: 'send_all_params'
                };
                OSC.sendParams();

                OSC.params.local['OSC_TIME_OFFSET'] = { value: 0 };
                OSC.params.local['OSC_VIEV_PART'] = { value: 0 };
                OSC.time_offset(OSC.params.local);
                OSC.startTime = performance.now();
                OSC.params.local = {};
                $('body').addClass('loaded');
                $('body').addClass('connection_lost');
                $('body').addClass('user_lost');
                OSC.startCheckStatus();
                var obj = $.cookie('measure_mode')
                if (obj === undefined)
                    obj = 1;
                OSC.setMeasureMode(obj);
                setTimeout(OSC.load_params, 1000);
            };

            OSC.ws.onclose = function() {

                OSC.state.socket_opened = false;
                $('#graphs .plot').hide(); // Hide all graphs
                console.log('Socket closed');
                if (OSC.unexpectedClose == true) {
                    setTimeout(OSC.reloadPage, '1000');
                }
            };

            $('#send_report_btn').on('click', function() {
                var mail = "support@redpitaya.com";
                var subject = "Feedback";
                var body = "%0D%0A%0D%0A------------------------------------%0D%0A" + "DEBUG INFO, DO NOT EDIT!%0D%0A" + "------------------------------------%0D%0A%0D%0A";
                body += "Parameters:" + "%0D%0A" + JSON.stringify({ parameters: OSC.params }) + "%0D%0A";
                body += "Browser:" + "%0D%0A" + JSON.stringify({ parameters: $.browser }) + "%0D%0A";

                var url = 'info/info.json';
                $.ajax({
                    method: "GET",
                    url: url
                }).done(function(msg) {
                    console.log(msg.responseText);
                    body += " info.json: " + "%0D%0A" + msg.responseText;
                    document.location.href = "mailto:" + mail + "?subject=" + subject + "&body=" + body;
                }).fail(function(msg) {
                    console.log(msg.responseText);
                    body += " info.json: " + "%0D%0A" + msg.responseText;
                    document.location.href = "mailto:" + mail + "?subject=" + subject + "&body=" + body;
                });
            });

            $('#restart_app_btn').on('click', function() {
                location.reload();
            });

            OSC.ws.onerror = function(ev) {
                console.log('Websocket error: ', ev);
            };

            var last_time = undefined;
            OSC.ws.onmessage = function(ev) {
                var start_time = +new Date();
                var data = new Uint8Array(ev.data);
                OSC.compressed_data += data.length;

                var inflate = new Zlib.Gunzip(data);
                var decompressed = inflate.decompress();
                var arr = new Uint16Array(decompressed)
                var text = OSC.convertUnpacked(arr);
                OSC.decompressed_data += text.length;

                var receive = JSON.parse(text);

                if (receive.parameters) {
                    if ((Object.keys(OSC.params.orig).length == 0) && (Object.keys(receive.parameters).length == 0)) {
                        OSC.params.local['in_command'] = {
                            value: 'send_all_params'
                        };
                        OSC.sendParams();
                    } else {
                        // if ('LA_MODE' in receive.parameters && receive.parameters['LA_MODE'].value != undefined) {
                        //     OSC.laMode = receive.parameters['LA_MODE'].value;
                        //     if (OSC.laMode == 3) {
                        //         if (!$.cookie('measure_mode'))
                        //             $('#modal_module_disconnected').modal('show');
                        //         else {
                        //             OSC.setMeasureMode(+$.cookie('measure_mode'));
                        //         }
                        //         $('#select_mode').parent().show();
                        //     }
                        // }

                        if ('MEASURE_MODE' in receive.parameters && receive.parameters['MEASURE_MODE'].value != undefined) {
                            OSC.measureMode = receive.parameters['MEASURE_MODE'].value;
                            OSC.updateMeasureMode(OSC.measureMode);
                        }

                        if ('CPU_LOAD' in receive.parameters && receive.parameters['CPU_LOAD'].value != undefined)
                            g_CpuLoad = receive.parameters['CPU_LOAD'].value;

                        if ('TOTAL_RAM' in receive.parameters && receive.parameters['TOTAL_RAM'].value != undefined)
                            g_TotalMemory = receive.parameters['TOTAL_RAM'].value;

                        if ('FREE_RAM' in receive.parameters && receive.parameters['FREE_RAM'].value != undefined)
                            g_FreeMemory = receive.parameters['FREE_RAM'].value;
                        OSC.parameterStack.push(receive.parameters);
                    }
                }

                if (receive.signals) {
                    ++g_count;
                    var changed = false;
                    for (var k in receive.signals) {
                        changed = true;
                        OSC.latest_signal[k] = JSON.parse(JSON.stringify(receive.signals[k]));
                    }
                    if ('ch1' in receive.signals) {
                        var split = OSC.repackSignals(OSC.latest_signal);
                        for (var k in split) {
                            OSC.latest_signal[k] = JSON.parse(JSON.stringify(split[k]));
                        }
                    }
                    if (changed) {

                        setTimeout(function() {
                            OSC.scaleWasChanged = true;
                            OSC.guiHandler();
                            OSC.checkAndShowArrows();
                        }, 50);

                    }

                    OSC.signalStack.push(OSC.latest_signal);
                    OSC.ch1_size = 0;
                    for (var i = 0; i < OSC.latest_signal['ch1'].size; i += 2) {
                        OSC.ch1_size += OSC.latest_signal['ch1'].value[i] + 1;
                    }
                }

            };
        }
    };
    OSC.repackSignals = function(signals) {
        var vals = {};
        var res = {};
        var hasData = false;
        for (var i = 1; i < 9; i++) {
            res["ch" + i] = {};
            res["ch" + i]["value"] = [];
            res["ch" + i]["size"] = 0;
        }

        for (var i = 0; i < signals["ch1"].size; i += 2) {
            var length = signals["ch1"].value[i] + 1;
            for (var chn = 0; chn < 8; chn++) {
                var ch = "ch" + (chn + 1);
                var val = (signals["ch1"].value[i + 1] >> chn) & 1;
                if (res[ch].value.length > 0) {
                    if (val == (res[ch].value[res[ch].value.length - 1]))
                        res[ch].value[res[ch].value.length - 2] += length;
                    else {
                        res[ch].value.push(length);
                        res[ch].value.push(val);
                    }
                } else {
                    res[ch].value.push(length);
                    res[ch].value.push(val);
                }
            }
        }

        for (var k in res) {
            res[k]['size'] = res[k].value.length;
        }
        return res;
    }

    OSC.repackSignalsAVG = function(signals, base) {
        BLOCK_SIZE = (base == undefined) ? 32 : base;
        BLOCK_SIZE = (BLOCK_SIZE >= 32) ? BLOCK_SIZE * 2 : BLOCK_SIZE;

        var vals = {};
        var res = {};
        var hasData = false;
        for (var i = 1; i < 9; i++) {
            var ch = "ch" + i;
            var ch_avg = ch; // + "_avg";

            res[ch_avg] = {};
            res[ch_avg]["value"] = [];
            res[ch_avg]["size"] = 0;

            var srcValues = signals[ch].value;
            var lastValue = undefined;
            var sum = 0;
            var counted = 0;
            var overflow = false;

            for (var j = 0; j < srcValues.length; j += 2) {
                if (srcValues[j] + counted >= BLOCK_SIZE) {
                    var diff = BLOCK_SIZE - counted;
                    counted += diff;
                    if (srcValues[j + 1] == 1)
                        sum += diff;
                    var coeff = sum / BLOCK_SIZE;
                    var averagedValue = 0;
                    if (coeff > 0.5)
                        averagedValue = 1;

                    res[ch_avg]["value"].push(BLOCK_SIZE);
                    res[ch_avg]["value"].push(averagedValue);

                    var toCount = srcValues[j] - diff;
                    while (toCount > BLOCK_SIZE) {
                        res[ch_avg]["value"].push(BLOCK_SIZE);
                        if (srcValues[j + 1] == 1)
                            res[ch_avg]["value"].push(1);
                        else
                            res[ch_avg]["value"].push(0);
                        toCount -= BLOCK_SIZE;
                    }
                    counted = toCount;
                    sum = (srcValues[j + 1] == 1) ? toCount : 0;
                } else {
                    counted += srcValues[j];
                    if (srcValues[j + 1] == 1)
                        sum += srcValues[j];
                }
            }
            if (counted > 0) {
                var coeff = sum / BLOCK_SIZE;
                var averagedValue = 0;
                if (coeff > 0.5)
                    averagedValue = 1;

                res[ch_avg]["value"].push(counted);
                res[ch_avg]["value"].push(averagedValue);
            }
        }
        for (var k in res) {
            res[k]['size'] = res[k].value.length;
        }
        return res;
    }

    OSC.splitSignals = function(signals) {
            var vals = {};
            for (var i = 1; i < 9; i++)
                vals["ch" + i] = [];

            var offset = OSC.counts_offset;
            OSC.offsetForDecoded = OSC.counts_offset;

            for (var chn = 0; chn < 8; chn++) {
                if (!OSC.enabled_channels[chn])
                    continue;


                var overflow = false;
                var encoded = 0;
                var skip = 0;
                var first_skip = true;

                var ch_get = "ch" + (chn + 1); // + "_avg";
                var ch_set = "ch" + (chn + 1);

                var amount = 0;

                for (var i = 0; i < signals[ch_get].value.length; i += 2) {
                    var length = signals[ch_get].value[i] * OSC.time_scale;

                    if ((skip + length) >= offset) {
                        if (first_skip) {
                            length = (skip + length) - offset;
                            skip = offset;
                            first_skip = false;
                            encoded = amount = length;

                        } else {
                            if ((encoded + length) >= 1024) {
                                overflow = true;
                                amount = 1024 - encoded;
                            } else
                                amount = length;
                            encoded += amount;
                        }
                    } else {
                        skip += length;
                        continue;
                    }
                    var amp = OSC.voltage_steps[OSC.voltage_index];
                    var offset1 = OSC.voltage_offset[chn];
                    var val = signals[ch_get].value[i + 1] * amp + offset1;
                    vals[ch_set].push(amount);
                    vals[ch_set].push(val);
                    if (overflow)
                        break;
                }

            }
            signals = {};
            for (var channel in vals) {
                signals[channel] = {};
                signals[channel].size = vals[channel].length;
                signals[channel].value = vals[channel];
            }
            return signals;
        }
        // Processes newly received values for parameters

    // Export
    // Firefox
    function fireEvent(obj, evt) {
        var fireOnThis = obj;
        if (document.createEvent) {
            var evObj = document.createEvent('MouseEvents');
            evObj.initEvent(evt, true, false);
            fireOnThis.dispatchEvent(evObj);

        } else if (document.createEventObject) {
            var evObj = document.createEventObject();
            fireOnThis.fireEvent('on' + evt, evObj);
        }
    }

    OSC.SaveGraphs = function() {
        html2canvas($('body'), {
            background: '#343433', // Like background of BODY
            onrendered: function(canvas) {
                var a = document.createElement('a');
                a.href = canvas.toDataURL('image/jpeg').replace('image/jpeg', 'image/octet-stream');
                a.download = 'graphs.jpg';
                fireEvent(a, 'click');
            }
        });
    }

    OSC.downloadDataAsCSV = function(filename) {
        var strings = ['i'];
        var col_delim = ', ';
        var row_delim = '\n';

        // Do nothing if no parameters received yet
        if ($.isEmptyObject(OSC.params.orig))
            return;

        var signal_names = ['ch1', 'ch2', 'ch3', 'ch4', 'ch5', 'ch6', 'ch7', 'ch8'];
        var points_all = [];
        var size_max = 0;

        for (var sig_name in OSC.recv_signals) {
            var index = signal_names.indexOf(sig_name);

            // Ignore empty signals
            if (OSC.recv_signals[sig_name].size == 0)
                continue;

            if (!OSC.enabled_channels[index])
                continue;

            var points = [];
            var start_x = 0;

            for (var u = 0; u < OSC.recv_signals[sig_name].value.length; u += 2) {
                // Start, end, value
                var size_x = OSC.recv_signals[sig_name].value[u];
                points.push([start_x, start_x + size_x, OSC.recv_signals[sig_name].value[u + 1]]);
                start_x += size_x;
            }

            if (start_x > size_max) {
                size_max = start_x;
            }

            strings.push(col_delim, sig_name);
            points_all.push(points);
        }

        strings.push(row_delim);

        for (var i = 0; i < size_max; ++i) {
            strings.push(i);

            for (var sig_i = 0; sig_i < points_all.length; ++sig_i) {
                var points = points_all[sig_i];
                var is_valid_value = false;

                for (var point_i = 0; point_i < points.length; ++point_i) {
                    if ((i >= points[point_i][0]) && (i < points[point_i][1])) {
                        strings.push(col_delim, points[point_i][2]);
                        is_valid_value = true;
                        break;
                    }
                }

                if (!is_valid_value) {
                    strings.push(col_delim, '-1');
                }
            }

            strings.push(row_delim);
        }

        saveAs(new Blob([strings.join('')], { type: "text/plain;charset=utf-8" }), filename);
    };

    OSC.process_run = function(new_params) {
        if (new_params['LA_RUN'].value === true) {
            $('#OSC_RUN').hide();
            $('#OSC_STOP').css('display', 'block');
        } else {
            $('#OSC_STOP').hide();
            $('#OSC_RUN').show();
        }
    }

    OSC.view_part = function(new_params) {
        var full_width = $('#buffer').width() - 4;
        var visible_width = full_width * new_params['OSC_VIEV_PART'].value;

        $('#buffer .buf-red-line').width(visible_width).show();
        $('#buffer .buf-red-line-holder').css('left', full_width / 2 - visible_width / 2);
    }

    OSC.sample_rate = function(new_params) {
        // Calculate time per division
        var samplerate = OSC.state.acq_speed;
        var samples = 1024;
        var mul = 1000;
        var scale = OSC.time_scale;
        var dev_num = 10;
        var timePerDevInMs = (((samples / scale) / samplerate) * mul) / dev_num;
        $('#OSC_TIME_SCALE').text(OSC.convertTime(timePerDevInMs));

        var graph_width = $('#graph_grid').outerWidth();
        ms_per_px = (timePerDevInMs * 10) / graph_width;
        var new_value = OSC.trigger_position * ms_per_px;
        $('#OSC_TIME_OFFSET').text(OSC.convertTime(new_value));

        $('#OSC_SAMPL_RATE').text($('#ACQ_SPEED option:selected').text());
    }

    OSC.auto_set_trig = function() {
        if (OSC.samples_first /* || OSC.demo_mode*/ ) {
            OSC.samples_first = false;
            return;
        }

        var trig_pos = ($('#time_offset_arrow').position().left + $('#time_offset_arrow').width() * 0.5) / $('#graphs').width() * 1024;
        OSC.time_scale = 1;
        OSC.scaleWasChanged = true;
        OSC.counts_offset = OSC.samples_sum - 512;

        while (OSC.samples_sum * OSC.time_scale < trig_pos) {
            $("#jtk_right").click();
        }

        var trigPosInPoints = (OSC.samples_sum * OSC.time_scale - OSC.counts_offset) * $('#graphs').width() / 1024 - $('#time_offset_arrow').width() / 2;
        $('#time_offset_arrow').css('left', trigPosInPoints);

        $('#OSC_SAMPL_RATE').text($('#ACQ_SPEED option:selected').text());

        // Calculate time per division
        var samplerate = OSC.state.acq_speed;
        var samples = 1024;
        var mul = 1000;
        var scale = OSC.time_scale;
        var dev_num = 10;
        var timePerDevInMs = (((samples / scale) / samplerate) * mul) / dev_num;
        $('#OSC_TIME_SCALE').text(OSC.convertTime(timePerDevInMs));

        // I cry when I see this code :'(
        $('#OSC_TIME_OFFSET').text(OSC.convertTime(5 * (($('#time_offset_arrow').position().left + $('#time_offset_arrow').width() / 2) / ($('#graphs').width() / 2) - 1) * 102400 / OSC.time_scale / OSC.state.acq_speed));
        OSC.guiHandler();
    }

    OSC.set_trig = function(new_params) {
        if (OSC.samples_first) {
            OSC.samples_first = false;
            return;
        }
        OSC.samples_sum = new_params['SAMPLES_SUM'].value;


        // OSC.time_scale = 1;
        // OSC.counts_offset = OSC.samples_sum - 512;
        var trig_pos = ($('#time_offset_arrow').position().left + $('#time_offset_arrow').width() * 0.5) / $('#graphs').width() * 1024;
        if (((OSC.samples_sum * OSC.time_scale) - OSC.counts_offset) >= trig_pos) {
            OSC.counts_offset = (OSC.samples_sum * OSC.time_scale) - parseInt(trig_pos);
        } else {
            OSC.time_scale = 1;
            OSC.counts_offset = OSC.samples_sum - 512;
            while (OSC.samples_sum * OSC.time_scale < trig_pos) {
                $("#jtk_right").click();
            }
        }

        // while (OSC.samples_sum * OSC.time_scale < trig_pos) {
        //     $("#jtk_right").click();
        // }
        // OSC.counts_offset = OSC.samples_sum * OSC.time_scale - trig_pos;

        var trigPosInPoints = (OSC.samples_sum * OSC.time_scale - OSC.counts_offset) * $('#graphs').width() / 1024 - $('#time_offset_arrow').width() / 2;
        $('#time_offset_arrow').css('left', trigPosInPoints);

        $('#OSC_SAMPL_RATE').text($('#ACQ_SPEED option:selected').text());

        // Calculate time per division
        var samplerate = OSC.state.acq_speed;
        var samples = 1024;
        var mul = 1000;
        var scale = OSC.time_scale;
        var dev_num = 10;
        var timePerDevInMs = (((samples / scale) / samplerate) * mul) / dev_num;
        $('#OSC_TIME_SCALE').text(OSC.convertTime(timePerDevInMs));
        OSC.checkAndShowArrows();

        // I cry when I see this code :'(
        $('#OSC_TIME_OFFSET').text(OSC.convertTime(5 * (($('#time_offset_arrow').position().left + $('#time_offset_arrow').width() / 2) / ($('#graphs').width() / 2) - 1) * 102400 / OSC.time_scale / OSC.state.acq_speed));
        OSC.guiHandler();
    }

    OSC.time_offset = function(new_params) {
        var graph_width = $('#graph_grid').outerWidth();

        // Calculate time per division
        var samplerate = OSC.state.acq_speed;
        var samples = 1024;
        var mul = 1000;
        var scale = OSC.time_scale;
        var timePerDevInMs = ((samples / scale) / samplerate) * mul;

        var ms_per_px = timePerDevInMs / graph_width;
        var px_offset = -(new_params['OSC_TIME_OFFSET'].value / ms_per_px + $('#time_offset_arrow').width() / 2 + 1);
        var arrow_left = (graph_width + 2) / 2 + px_offset;
        var buf_width = graph_width - 2;
        var ratio = buf_width / (buf_width * new_params['OSC_VIEV_PART'].value);
        OSC.state.graph_grid_width = graph_width;
        $('#time_offset_arrow').css('left', arrow_left).show();
        $('#buf_time_offset').css('left', buf_width / 2 - buf_width * new_params['OSC_VIEV_PART'].value / 2 + arrow_left / ratio - 4).show();
    }

    OSC.cursor_x = function(param_name, new_params) {
        if (!OSC.state.cursor_dragging) {
            var x = (param_name == 'OSC_CURSOR_X1' ? 'x1' : 'x2');

            if (new_params[param_name].value) {
                var new_value = new_params[x == 'x1' ? 'OSC_CUR1_T' : 'OSC_CUR2_T'].value;
                var graph_width = $('#graph_grid').width();

                // Calculate time per division
                var samplerate = OSC.state.acq_speed;
                var samples = 1024;
                var mul = 1000;
                var scale = OSC.time_scale;
                var timePerDevInMs = ((samples / scale) / samplerate) * mul;
                var ms_per_px = timePerDevInMs / graph_width;

                var px_offset = -(parseInt($('#cur_' + x + '_arrow').css('margin-left')) / 2 - 2.5);
                var msg_width = $('#cur_' + x + '_info').outerWidth();
                var left = (graph_width + 2) / 2 + px_offset;

                var overflow = false;
                if (left < 0) {
                    left = 0;
                    overflow = true;
                }
                if (left > graph_width) {
                    left = graph_width;
                    overflow = true;
                }

                $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').css('left', left).show();
                $('#cur_' + x + '_info')
                    .html(OSC.convertTime(-new_value))
                    .data('cleanval', -new_value)
                    .css('margin-left', (left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));

                if (overflow)
                    $('#cur_' + x + '_info').hide();
            } else {
                $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').hide();
            }
        }
    }

    OSC.cursor_x1 = function(new_params) {
        OSC.cursor_x('OSC_CURSOR_X1', new_params);
    }

    OSC.cursor_x2 = function(new_params) {
        OSC.cursor_x('OSC_CURSOR_X2', new_params);
    }

    OSC.getBusByDecoderName = function(decoder_name) {
        for (var i = 1; i < 5; i++) {
            var bus_name = "bus" + i;
            if (OSC.buses[bus_name] != undefined && OSC.buses[bus_name].name != undefined && OSC.buses[bus_name].enabled) {
                switch (OSC.buses[bus_name].name) {
                    case "UART":
                    case "CAN":
                    case "I2C":
                        if (OSC.buses[bus_name].decoder == decoder_name)
                            return bus_name;
                        break;
                    case "SPI":
                        if (OSC.buses[bus_name].miso_decoder == decoder_name)
                            return bus_name;
                        if (OSC.buses[bus_name].mosi_decoder == decoder_name)
                            return bus_name;
                        break;
                }
            }
        }
        return "";
    }

    OSC.decoder_created = function(new_params) {
        var decoder_name = new_params['CREATE_DECODER'].value;
        if (decoder_name == "")
            return;

        var bus = OSC.getBusByDecoderName(decoder_name);
        if (bus != "") {
            var param = {};
            if (OSC.buses[bus].name == "UART" || OSC.buses[bus].name == "I2C" || OSC.buses[bus].name == "CAN") {
                param[decoder_name + "_parameters"] = {
                    value: OSC.buses[bus]
                };
            } else {
                var p = OSC.buses[bus];
                if (OSC.buses[bus].miso_decoder == decoder_name)
                    p['data'] = OSC.buses[bus].miso;
                else if (OSC.buses[bus].mosi_decoder == decoder_name)
                    p['data'] = OSC.buses[bus].mosi;
                else {
                    console.log("Bus is not miso or mosi.");
                    return;
                }
                param[decoder_name + "_parameters"] = {
                    value: p
                };
            }
            OSC.ws.send(JSON.stringify({
                parameters: param
            }));
        }
    }

    OSC.i2c_paramteters = function(new_params) {
        var param = new_params['i2c1_parameters'].value;
        console.log(param);
    }

    OSC.processParameters = function(new_params) {
        var old_params = $.extend(true, {}, OSC.params.orig);

        var send_all_params = Object.keys(new_params).indexOf('send_all_params') != -1;

        OSC.updateInterfaceFor250(new_params['RP_MODEL_STR']);
        OSC.updateInterfaceForZ20(new_params['RP_MODEL_STR']);
        for (var param_name in new_params) {
            OSC.params.orig[param_name] = new_params[param_name];
            if (OSC.param_callbacks[param_name] !== undefined)
                OSC.param_callbacks[param_name](new_params);
        }

        // Resize double-headed arrows showing the difference between cursors
        OSC.updateXCursorDiff();
    };

    OSC.getChByColor = function(color) {
        for (var i = 1; i < 9; i++) {
            var ch = "ch" + i;
            if (OSC.config.graph_colors[ch] == color) {
                return i;
            }
        }
        return -1;
    }

    OSC.getDecoderByChannelNum = function(ch) {
        for (var i = 1; i < 5; i++) {
            var bus = "bus" + i;
            if (OSC.buses[bus] != undefined && OSC.buses[bus].name != undefined) {
                switch (OSC.buses[bus].name) {
                    case "UART":
                        if (OSC.buses[bus].rx == ch)
                            return OSC.buses[bus].decoder;
                        break;
                    case "I2C":
                        if (OSC.buses[bus].sda == ch)
                            return OSC.buses[bus].decoder;
                        break;
                    case "CAN":
                        if (OSC.buses[bus].can_rx == ch)
                            return OSC.buses[bus].decoder;
                        break;
                    case "SPI":
                        if (OSC.buses[bus].miso == ch)
                            return OSC.buses[bus].miso_decoder;
                        if (OSC.buses[bus].mosi == ch)
                            return OSC.buses[bus].mosi_decoder;
                        break;
                }
            }
        }
        return "";
    }

    OSC.getBusByChNum = function(ch) {
        for (var i = 1; i < 5; i++) {
            var bus = "bus" + i;
            if (OSC.buses[bus] != undefined && OSC.buses[bus].name != undefined) {
                switch (OSC.buses[bus].name) {
                    case "UART":
                        if (OSC.buses[bus].rx == ch)
                            return bus;
                        break;
                    case "I2C":
                        if (OSC.buses[bus].sda == ch)
                            return bus;
                        break;
                    case "CAN":
                        if (OSC.buses[bus].can_rx == ch)
                            return bus;
                        break;
                    case "SPI":
                        if (OSC.buses[bus].miso == ch)
                            return bus;
                        if (OSC.buses[bus].mosi == ch)
                            return bus;
                        break;
                }
            }
        }
        return "";
    }

    OSC.drawSeries = function(ch, plot, canvascontext) {
        if (ch == -1)
            return;
        var decoder = OSC.getDecoderByChannelNum(ch + 1);
        if (decoder == "")
            return;
        var signal = decoder + "_signal";

        if (signal in OSC.recv_signals) {
            OSC.current_bus = OSC.getBusByChNum(ch + 1);
            if (decoder.startsWith('can'))
                CAN.drawDecoded(ch, plot, canvascontext, OSC.voltage_offset[ch], OSC.recv_signals[signal]);
            else if (decoder.startsWith('i2c'))
                I2C.drawDecoded(ch, plot, canvascontext, OSC.voltage_offset[ch], OSC.recv_signals[signal]);
            else if (decoder.startsWith('spi'))
                SPI.drawDecoded(ch, plot, canvascontext, OSC.voltage_offset[ch], OSC.recv_signals[signal], OSC.accordingChanName(ch + 1));
            else if (decoder.startsWith('uart'))
                UART.drawDecoded(ch, plot, canvascontext, OSC.voltage_offset[ch], OSC.recv_signals[signal], OSC.current_bus, OSC.accordingChanName(ch + 1));
        }
        OSC.current_bus = "bus-1";
    }

    // Processes newly received data for signals
    OSC.processSignals = function(new_signals) {
        var visible_btns = [];
        var visible_plots = [];
        var visible_info = '';
        var start = +new Date();

        // Do nothing if no parameters received yet
        if ($.isEmptyObject(OSC.params.orig))
            return;
        var arrrrr = ['ch1', 'ch2', 'ch3', 'ch4', 'ch5', 'ch6', 'ch7', 'ch8'];
        var pointArr = [];
        var colorsArr = [];

        for (var sig_name in new_signals) {
            var index = arrrrr.indexOf(sig_name);

            // Ignore empty signals
            if (new_signals[sig_name].size == 0)
                continue;

            if (!OSC.enabled_channels[index])
                continue;

            var points = [];
            var color = OSC.config.graph_colors[sig_name];
            var start_point = 0;

            for (var u = 0; u < new_signals[sig_name].value.length; u += 2) {
                var start_x = start_point;
                var start_y = new_signals[sig_name].value[u + 1];

                points.push([start_x, start_y]);

                start_point += new_signals[sig_name].value[u];
                if (start_point > 1024)
                    start_point = 1024;

                var end_x = start_point;
                var end_y = start_y;
                points.push([end_x, end_y]);
                if (end_x == 1024)
                    break;
            }
            pointArr.push(points);
            colorsArr.push(color);
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
        }

        for (var sig_name in new_signals) {
            var index = arrrrr.indexOf(sig_name);
            if (!OSC.enabled_channels[index])
                continue;
            OSC.drawSeries(index, OSC.graphs["ch1"].plot, OSC.graphs["ch1"].plot.getCanvas().getContext("2d"));
        }

        visible_plots.push(OSC.graphs["ch1"].elem[0]);
        visible_info += (visible_info.length ? ',' : '') + '.' + "ch1";

        if (OSC.scaleWasChanged)
            COMMON.fflushLog();
        // Hide plots without signal
        $('#graphs .plot').not(visible_plots).hide();

        // Show only information about active signals
        $('#info').find(visible_info).show();

        // Reset resize flag
        OSC.state.resized = false;

        // Check if selected signal is still visible
        if (OSC.state.sel_sig_name && OSC.graphs[OSC.state.sel_sig_name] && !OSC.graphs[OSC.state.sel_sig_name].elem.is(':visible')) {
            $('#right_menu .menu-btn.active.' + OSC.state.sel_sig_name).removeClass('active');
            //OSC.state.sel_sig_name = nameull;
        }
    };

    OSC.getRandomArbitary = function(min, max) {
        return Math.random() * (max - min) + min;
    }

    // Exits from editing mode
    OSC.exitEditing = function(noclose) {

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

        for (var key in OSC.params.orig) {
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

            if (value !== undefined && value != OSC.params.orig[key].value) {
                console.log(key + ' changed from ' + OSC.params.orig[key].value + ' to ' + ($.type(OSC.params.orig[key].value) == 'boolean' ? !!value : value));
                OSC.params.local[key] = {
                    value: ($.type(OSC.params.orig[key].value) == 'boolean' ? !!value : value)
                };
            }
        }

        // Send params then reset editing state and hide dialog
        OSC.sendParams();
        OSC.state.editing = false;
        if (noclose) return;
        $('.dialog:visible').hide();
        $('#right_menu').show();
    };

    // Sends to server modified parameters
    OSC.sendParams = function() {
        if ($.isEmptyObject(OSC.params.local)) {
            return false;
        }

        if (!OSC.state.socket_opened) {
            console.log('ERROR: Cannot save changes, socket not opened');
            return false;
        }

        OSC.params.local['in_command'] = {
            value: 'send_all_params'
        };

        OSC.ws.send(JSON.stringify({
            parameters: OSC.params.local
        }));
        OSC.params.local = {};

        return true;
    };

    OSC.checkAndShowArrows = function() {
        var trigPosInPoints = (OSC.samples_sum * OSC.time_scale - OSC.counts_offset) * $('#graphs').width() / 1024 - $('#time_offset_arrow').width() / 2;
        $('#time_offset_arrow').css('left', trigPosInPoints);
        $("#time_offset_arrow").show();
        if ($('#OSC_CURSOR_X1').hasClass('active'))
            $('#cur_x1_arrow').show();
        if ($('#OSC_CURSOR_X2').hasClass('active'))
            $('#cur_x2_arrow').show();
    }

    // Draws the grid on the lowest canvas layer
    OSC.drawGraphGrid = function() {
        var canvas_width = $('#graphs').width() - 2;
        var canvas_height = Math.round(canvas_width / 2);

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
        if (direction == '-') {
            if (OSC.voltage_index == 3)
                return;
            OSC.voltage_index++;
        } else {
            if (OSC.voltage_index == 0)
                return;
            OSC.voltage_index--;
        }
    };

    // Changes X zoom/scale for all signals
    OSC.changeXZoom = function(direction, curr_scale, send_changes) {
        // Calculate time per division
        var samplerate = OSC.state.acq_speed;
        var samples = 1024;
        var mul = 1000;
        var newScaleMul;

        if (direction == '+') {
            newScaleMul = OSC.time_scale * ((OSC.state.fine == false) ? 2 : 1.1);
            OSC.allSignalShown = false; // Reset 'do not change time_scale' flag
        } else if (direction == '-') {
            newScaleMul = OSC.time_scale / ((OSC.state.fine == false) ? 2 : 1.1);
        } else if (direction == '1') {
            OSC.time_scale = 1;
        }

        var new_l = 0;
        if (OSC.latest_signal.ch1 !== undefined) {
            for (var i = 0; i < OSC.latest_signal.ch1.value.length; i += 2) { new_l += OSC.latest_signal.ch1.value[i] + 1; }
            new_l *= newScaleMul;
            if (new_l < 1024)
                return false;

            var timePerDevInMs = (((samples / newScaleMul) / samplerate) * mul);

            // zoom borders
            if (timePerDevInMs <= 250000 && timePerDevInMs >= 0.00001 && !OSC.allSignalShown) {
                var scale_limit = OSC.state.decimate / 1024;
                // if (newScaleMul > scale_limit || direction == '+') {
                OSC.time_scale = newScaleMul;
                return true;
                // }
            }
        }
        return false;
    };

    // Sets default values for cursors, if values not yet defined
    OSC.updateCursorWithNewScale = function() {
        var graph_height = $('#graph_grid').height();
        var graph_width = $('#graph_grid').width();

        // Calculate time per division
        var samplerate = OSC.state.acq_speed;
        var samples = 1024;
        var mul = 1000;
        var scale = OSC.time_scale;
        var timePerDevInMs = (((samples / scale) / samplerate) * mul);
        var ms_per_px = timePerDevInMs / graph_width;

        // Default value for X1 cursor is 1/4 from graph width
        if ($('#cur_x1').is(':visible')) {
            var left = parseInt($('#cur_x1').css('left'));

            var msg_width = $('#cur_x1' + '_info').outerWidth();
            var new_value = (graph_width / 2 - left - (10 - 2) / 2 - parseInt($('#graph_grid').css('margin-left'))) * ms_per_px;
            $('#cur_x1' + '_info')
                .html(OSC.convertTime(-new_value))
                .data('cleanval', -new_value)
                .css('margin-left', (left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));
        }

        // Default value for X2 cursor is 1/3 from graph width
        if ($('#cur_x2').is(':visible')) {
            var left = parseInt($('#cur_x2').css('left'));

            var msg_width = $('#cur_x2' + '_info').outerWidth();
            var new_value = (graph_width / 2 - left - (10 - 2) / 2 - parseInt($('#graph_grid').css('margin-left'))) * ms_per_px;
            $('#cur_x2' + '_info')
                .html(OSC.convertTime(-new_value))
                .data('cleanval', -new_value)
                .css('margin-left', (left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));
        }

        OSC.updateXCursorDiff();
    };

    // Sets default values for cursors, if values not yet defined
    OSC.setDefCursorVals = function() {
        var graph_height = $('#graph_grid').height();
        var graph_width = $('#graph_grid').width();

        // Calculate time per division
        var samplerate = OSC.state.acq_speed;
        var samples = 1024;
        var mul = 1000;
        var scale = OSC.time_scale;
        var timePerDevInMs = (((samples / scale) / samplerate) * mul);
        var ms_per_px = timePerDevInMs / graph_width;

        // Default value for X1 cursor is 1/4 from graph width
        if ($('#cur_x1').is(':visible')) {
            var left = graph_width * 0.25;

            var msg_width = $('#cur_x1' + '_info').outerWidth();
            var new_value = (graph_width / 2 - left - (10 - 2) / 2 - parseInt($('#graph_grid').css('margin-left'))) * ms_per_px;
            $('#cur_x1' + '_arrow, #cur_x1' + ', #cur_x1' + '_info').show();
            $('#cur_x1' + ', #cur_x1' + '_info').css('left', left);
            $('#cur_x1' + '_info')
                .html(OSC.convertTime(-new_value))
                .data('cleanval', -new_value)
                .css('margin-left', (left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));

            $('#cur_x1_arrow, #cur_x1').css('left', left).show();
            $('#cur_x1').data('init', true);
        }

        // Default value for X2 cursor is 1/3 from graph width
        if ($('#cur_x2').is(':visible')) {
            var left = graph_width * 0.33;

            var msg_width = $('#cur_x2' + '_info').outerWidth();
            var new_value = (graph_width / 2 - left - (10 - 2) / 2 - parseInt($('#graph_grid').css('margin-left'))) * ms_per_px;
            $('#cur_x2' + '_arrow, #cur_x2' + ', #cur_x2' + '_info').show();
            $('#cur_x2' + ', #cur_x2' + '_info').css('left', left);
            $('#cur_x2' + '_info')
                .html(OSC.convertTime(-new_value))
                .data('cleanval', -new_value)
                .css('margin-left', (left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));

            $('#cur_x2_arrow, #cur_x2').css('left', left).show();
            $('#cur_x2').data('init', true);
        }

        OSC.updateXCursorDiff();
    };

    // Updates all elements related to a X cursor
    OSC.updateXCursorElems = function(ui, save) {
        var x = (ui.helper[0].id == 'cur_x1_arrow' ? 'x1' : 'x2');
        var graph_width = $('#graph_grid').width();

        // Calculate time per division
        var samplerate = OSC.state.acq_speed;
        var samples = 1024;
        var mul = 1000;
        var scale = OSC.time_scale;
        var timePerDevInMs = (((samples / scale) / samplerate) * mul);

        var ms_per_px = timePerDevInMs / graph_width;
        var msg_width = $('#cur_' + x + '_info').outerWidth();
        var new_value = (graph_width / 2 - ui.position.left - (ui.helper.width() - 2) / 2 - parseInt(ui.helper.css('margin-left'))) * ms_per_px;

        $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').show();
        $('#cur_' + x + ', #cur_' + x + '_info').css('left', ui.position.left);
        $('#cur_' + x + '_info')
            .html(OSC.convertTime(-new_value))
            .data('cleanval', -new_value)
            .css('margin-left', (ui.position.left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));

        OSC.updateXCursorDiff();
    };

    // Resizes double-headed arrow showing the difference between X cursors
    OSC.updateXCursorDiff = function() {
        var x1 = $('#cur_x1_info');
        var x2 = $('#cur_x2_info');
        var x1_left = parseInt(x1.css('left'));
        var x2_left = parseInt(x2.css('left'));
        var diff_px = Math.abs(x1_left - x2_left) - 9;

        if (x1.is(':visible') && x2.is(':visible') && diff_px > 12) {
            var left = Math.min(x1_left, x2_left);
            var value = $('#cur_x1_info').data('cleanval') - $('#cur_x2_info').data('cleanval');

            $('#cur_x_diff')
                .css('left', left + 1)
                .width(diff_px)
                .show();
            $('#cur_x_diff_info')
                .html(OSC.convertTime(Math.abs(value)))
                .show()
                .css('left', left + diff_px / 2 - $('#cur_x_diff_info').width() / 2 + 3);
        } else {
            $('#cur_x_diff, #cur_x_diff_info').hide();
        }
    };

    // Updates Y offset in the signal config dialog, if opened, or saves new value
    OSC.updateYOffset = function(ui, save) {
        var graph_height = $('#graph_grid').outerHeight();
        var zero_pos = (graph_height + 7) / 2;
        var new_value;

        var arrows = ["ch1_offset_arrow", "ch2_offset_arrow", "ch3_offset_arrow", "ch4_offset_arrow",
            "ch5_offset_arrow", "ch6_offset_arrow", "ch7_offset_arrow", "ch8_offset_arrow"
        ];
        var ch = arrows.indexOf(ui.helper[0].id);
        if (ch != -1) {
            var volt_per_px = (1 * 10) / graph_height;
            new_value = (zero_pos - ui.position.top + parseInt(ui.helper.css('margin-top')) / 2) * volt_per_px;

            //$('#info_box').html('CH ' + (ch + 1) + ' zero offset ' + OSC.convertVoltage(new_value));

            OSC.voltage_offset[ch] = new_value;
            OSC.guiHandler();
        }
        if (new_value !== undefined && save) {
            OSC.sendParams();
        }
    };

    // Converts time from milliseconds to a more 'user friendly' time unit; returned value includes units
    OSC.convertTime = function(t) {
        var abs_t = Math.abs(t);
        var unit = 'ms';

        if (abs_t >= 1000) {
            t = t / 1000;
            unit = 's';
        } else if (abs_t >= 1) {
            t = t * 1;
            unit = 'ms';
        } else if (abs_t >= 0.001) {
            t = t * 1000;
            unit = 'us';
        } else if (abs_t >= 0.000001) {
            t = t * 1000000;
            unit = ' ns';
        }

        return +(t.toFixed(2)) + ' ' + unit;
    };

    OSC.getTimePerDiv = function(t) {
        var abs_t = Math.abs(t);
        if (abs_t >= 1000) {
            return t / 1000;
        } else if (abs_t >= 1) {
            return t;
        } else if (abs_t >= 0.001) {
            return t * 1000;
        } else if (abs_t >= 0.000001) {
            return t * 1000000;
        }
    };

    // Converts voltage from volts to a more 'user friendly' unit; returned value includes units
    OSC.convertVoltage = function(v) {
        var abs_v = Math.abs(v);
        var unit = 'V';

        if (abs_v >= 1) {
            v = v * 1;
            unit = 'V';
        } else if (abs_v >= 0.001) {
            v = v * 1000;
            unit = 'mV';
        }

        return +(v.toFixed(2)) + ' ' + unit;
    };

    OSC.setValue = function(input, value) {
        input.val(value);
        //input.change();
    };


    OSC.enableCursor = function(x) {
        var x2 = (x == 'x1') ? 'x2' : 'x1';
        var d = (x == 'x1') ? '1' : '2';
        $('cur_' + x).show();
        $('cur_' + x + '_info').show();
        $('cur_' + x + '_arrow').show();

        if ($('cur_' + x2).is(':visible')) {
            $('cur_x_diff').show();
            $('cur_x_diff_info').show();
        }

        OSC.params.local['OSC_CURSOR_X1'] = { value: 1 };
        OSC.params.local['OSC_CURSOR_X2'] = { value: 1 };

        OSC.params.local['OSC_CUR1_T'] = { value: 1 };
        OSC.params.local['OSC_CUR2_T'] = { value: 1 };

        OSC.cursor_x('OSC_CURSOR_X' + d, OSC.params.local);
        OSC.cursor_x('OSC_CURSOR_X' + d, OSC.params.local);

        OSC.setDefCursorVals();

        OSC.params.local = {};
    };

    OSC.disableCursor = function(x) {
        var d = (x == 'x1') ? '1' : '2';

        $('cur_' + x).hide();
        $('cur_' + x + '_info').hide();
        $('cur_' + x + '_arrow').hide();
        $('cur_x_diff').hide();
        $('cur_x_diff_info').hide();

        OSC.params.local['OSC_CURSOR_X1'] = { value: 0 };
        OSC.params.local['OSC_CURSOR_X2'] = { value: 0 };

        OSC.cursor_x('OSC_CURSOR_X' + d, OSC.params.local);
        OSC.cursor_x('OSC_CURSOR_X' + d, OSC.params.local);

        OSC.params.local = {};
    };

    OSC.updateChVisibility = function(ch) {
        var arrow = $('#ch' + (ch + 1) + '_offset_arrow');
        if (!arrow.is(':visible')) {
            var grid = $('#graph_grid');
            var volt_per_px = grid.outerHeight() / 10;
            var px_offset = -(OSC.voltage_offset[ch] * volt_per_px);
            OSC.state.graph_grid_height = grid.outerHeight();
            arrow.css('top', grid.outerHeight() / 2 + px_offset).show();
        }
        var txt = $('#CH' + (ch + 1) + '_NAME').val();
        if (txt == "") {
            txt = "DIN" + ch;
        }
        $('#CH' + (ch + 1) + '_NAME').val(txt);
        arrow.find('#CH' + (ch + 1) + '_LABEL').text(txt);
        OSC.ch_names[ch] = txt;

        OSC.showInfoArrow(ch);
    }

    OSC.accordingChanName = function(chan_number) {
        for (var i = 1; i < 5; i++) {
            var bus = 'bus' + i;
            if (OSC.buses[bus] !== undefined && OSC.buses[bus].name !== undefined && OSC.buses[bus].enabled) {
                // Check UART
                if (OSC.buses[bus].name == "UART" && OSC.buses[bus].rx == chan_number) {
                    if (OSC.buses[bus].rxtxstr == "RX")
                        return "UART: RX";
                    else if (OSC.buses[bus].rxtxstr == "TX")
                        return "UART: TX";
                }

                if (OSC.buses[bus].name == "CAN" && OSC.buses[bus].can_rx == chan_number) {
                    return "CAN: RX";
                }

                // Check I2C
                if (OSC.buses[bus].name == "I2C") {
                    if (OSC.buses[bus].scl == chan_number)
                        return "I2C: SCL";
                    else if (OSC.buses[bus].sda == chan_number)
                        return "I2C: SDA";
                }

                // Check SPI
                if (OSC.buses[bus].name == "SPI") {
                    if (OSC.buses[bus].clk == chan_number)
                        return "SPI: SCK";
                    else if (OSC.buses[bus].miso == chan_number)
                        return "SPI: MISO";
                    else if (OSC.buses[bus].mosi == chan_number)
                        return "SPI: MOSI";
                    else if (OSC.buses[bus].cs == chan_number)
                        return "SPI: CS";
                }
            }
        }
        return "";
    }

    OSC.showInfoArrow = function(ch) {
        var arrow_info = $('#ch' + (ch + 1) + '_info');
        var arrow_img = $('#img_info_arrow' + (ch + 1));
        var info_text = OSC.accordingChanName(ch + 1);

        if (info_text !== "") {
            arrow_info.text(info_text);
            arrow_img.show();
        }
    }

    OSC.hideInfoArrow = function(ch) {
        var arrow_info = $('#ch' + (ch + 1) + '_info');
        var arrow_img = $('#img_info_arrow' + (ch + 1));

        arrow_info.text("");
        arrow_img.hide();
    }

    OSC.needUpdateDecoder = function(bus, new_bus) {
        if (OSC.buses[bus] == undefined || OSC.buses[bus].name == undefined)
            return false;
        if (OSC.buses[bus].enabled && OSC.buses[bus].name == new_bus)
            return true;
        return false;
    }

    OSC.destroyDecoder = function(bus, new_bus) {
        if (OSC.buses[bus] == undefined || OSC.buses[bus].name == undefined)
            return;
        if (!OSC.buses[bus].enabled)
            return;
        if (OSC.buses[bus].name == new_bus)
            return;

        if (OSC.buses[bus].name === "UART") {
            OSC.hideInfoArrow(parseInt(OSC.buses[bus].rx) - 1);
            OSC.hideInfoArrow(parseInt(OSC.buses[bus].tx) - 1);
        } else if (OSC.buses[bus].name === "CAN") {
            OSC.hideInfoArrow(parseInt(OSC.buses[bus].can_rx) - 1);
        } else if (OSC.buses[bus].name === "I2C") {
            OSC.hideInfoArrow(parseInt(OSC.buses[bus].scl) - 1);
            OSC.hideInfoArrow(parseInt(OSC.buses[bus].sda) - 1);
        } else if (OSC.buses[bus].name === "SPI") {
            OSC.hideInfoArrow(parseInt(OSC.buses[bus].clk) - 1);
            OSC.hideInfoArrow(parseInt(OSC.buses[bus].cs) - 1);
            OSC.hideInfoArrow(parseInt(OSC.buses[bus].mosi) - 1);
            OSC.hideInfoArrow(parseInt(OSC.buses[bus].miso) - 1);
        }

        // Drop decoder signal
        var signalName = '';

        if (OSC.buses[bus].name == 'UART' || OSC.buses[bus].name == 'I2C' || OSC.buses[bus].name == 'CAN')
            signalName = OSC.buses[bus].decoder + '_signal';
        else if (OSC.buses[bus].miso_decoder !== undefined && OSC.buses[bus].miso_decoder !== "")
            signalName = OSC.buses[bus].miso_decoder + '_signal';
        else if (OSC.buses[bus].mosi_decoder !== undefined && OSC.buses[bus].mosi_decoder !== "")
            signalName = OSC.buses[bus].mosi_decoder + '_signal';

        delete OSC.signalStack[signalName];
        delete OSC.latest_signal[signalName];

        switch (OSC.buses[bus].name) {
            case "UART":
            case "CAN":
            case "I2C":
                OSC.params.local['DESTROY_DECODER'] = {
                    value: OSC.buses[bus].decoder
                };
                OSC.sendParams()
                break;
            case "SPI":
                if (OSC.buses[bus].miso_decoder !== undefined && OSC.buses[bus].miso_decoder !== "") {
                    OSC.params.local['DESTROY_DECODER'] = {
                        value: OSC.buses[bus].miso_decoder
                    };
                    OSC.sendParams();
                }
                if (OSC.buses[bus].mosi_decoder !== undefined && OSC.buses[bus].mosi_decoder !== "") {
                    OSC.params.local['DESTROY_DECODER'] = {
                        value: OSC.buses[bus].mosi_decoder
                    };
                    OSC.sendParams();
                }
                break;
        }
    }

    OSC.startEditBus = function(bus) {
        $('#warning-dialog').hide();
        var arr = ["BUS1_SETTINGS", "BUS2_SETTINGS", "BUS3_SETTINGS", "BUS4_SETTINGS"];
        var bn = arr.indexOf(bus) + 1;
        if (bn != 0) {
            OSC.state.bus_editing = bn;
            $('.channels_selector').empty();
            $('.channels_selector').append('<option value="-1">-</option>');
            for (var i = 1; i < 9; i++) {
                $('.channels_selector').append('<option value="' + (i) + '">' + (($('#CH' + i + '_NAME').val() != "") ? $('#CH' + i + '_NAME').val() : $('#CH' + i + '_NAME').attr('placeholder')) + '</option>');
            }

            if ($("BUS" + bn + "_NAME").text() != "BUS" + bn) {
                var bus = 'bus' + bn;
                if (OSC.buses[bus].name !== undefined) {
                    $('.decoder-window').hide();
                    if (OSC.buses[bus].name == "UART") {
                        $('#protocol_selector option').removeAttr('selected');
                        $('#protocol_selector option[value=#uart_decoder]').attr('selected', 'selected');
                        $('#protocol_selector').val("#uart_decoder");
                        $('#uart_decoder').show();

                        $('#uart_data_length option[value=' + OSC.buses[bus].num_data_bits + ']').attr('selected', 'selected');
                        $('#uart_stop_bits option[value=' + OSC.buses[bus].num_stop_bits + ']').attr('selected', 'selected');
                        $('#uart_parity option[value=' + OSC.buses[bus].parity + ']').attr('selected', 'selected');
                        $('#uart_order option[value=' + OSC.buses[bus].bitOrder + ']').attr('selected', 'selected');
                        $('#uart_order option[value=' + OSC.buses[bus].invert_rx + ']').attr('selected', 'selected');

                        $('#uart_serial option[value=' + (OSC.buses[bus].rx) + ']').attr('selected', 'selected');
                        $('#uart_baudrate').val(OSC.buses[bus].baudrate);
                    } else if (OSC.buses[bus].name == "CAN") {
                        $('#protocol_selector option').removeAttr('selected');
                        $('#protocol_selector option[value=#can_decoder]').attr('selected', 'selected');
                        $('#protocol_selector').val("#can_decoder");
                        $('#can_decoder').show();

                        $('#can_rx option[value=' + (parseInt(OSC.buses[bus].can_rx)) + ']').attr('selected', 'selected');
                        $('#can_nom_bitrate').val(OSC.buses[bus].nominal_bitrate);
                        $('#can_fast_bitrate').val(OSC.buses[bus].fast_bitrate);
                        $('#sample_point').val(OSC.buses[bus].sample_point);
                        $('#can_frame_limit').val(OSC.buses[bus].frame_limit);
                        $('#can_invert option[value=' + OSC.buses[bus].invert_bit + ']').attr('selected', 'selected');

                    } else if (OSC.buses[bus].name == "I2C") {
                        $('#protocol_selector option').removeAttr('selected');
                        $('#protocol_selector option[value=#i2c_decoder]').attr('selected', 'selected');
                        $('#protocol_selector').val("#i2c_decoder");
                        $('#i2c_decoder').show();

                        $('#i2c_sda option[value=' + (parseInt(OSC.buses[bus].sda)) + ']').attr('selected', 'selected');
                        $('#i2c_scl option[value=' + (parseInt(OSC.buses[bus].scl)) + ']').attr('selected', 'selected');

                        $('#i2c_addr option[value=' + OSC.buses[bus].address_format + ']').attr('selected', 'selected');
                        $('#i2c_invert option[value=' + OSC.buses[bus].invert_bit + ']').attr('selected', 'selected');

                    } else if (OSC.buses[bus].name == "SPI") {
                        $('#protocol_selector option').removeAttr('selected');
                        $('#protocol_selector option[value=#spi_decoder]').attr('selected', 'selected');
                        $('#protocol_selector').val("#spi_decoder");
                        $('#spi_decoder').show();

                        $('#spi_clk option[value=' + (OSC.buses[bus].clk) + ']').attr('selected', 'selected');
                        $('#spi_mosi option[value=' + (OSC.buses[bus].mosi) + ']').attr('selected', 'selected');
                        $('#spi_miso option[value=' + (OSC.buses[bus].miso) + ']').attr('selected', 'selected');
                        $('#spi_cs option[value=' + (OSC.buses[bus].cs) + ']').attr('selected', 'selected');

                        $('#spi_order option[value=' + OSC.buses[bus].word_size + ']').attr('selected', 'selected');
                        $('#spi_length option[value=' + OSC.buses[bus].data_length + ']').attr('selected', 'selected');
                        $('#spi_cpol option[value=' + OSC.buses[bus].cpol + ']').attr('selected', 'selected');
                        $('#spi_cpha option[value=' + OSC.buses[bus].cpha + ']').attr('selected', 'selected');
                        $('#spi_state option[value=' + OSC.buses[bus].cs_polarity + ']').attr('selected', 'selected');
                        $('#spi_invert option[value=' + OSC.buses[bus].invert_bit + ']').attr('selected', 'selected');
                    }
                }
            }
            $('#decoder_dialog').modal('show');

            // Check enabled decoders, check existing logic data file and show help link
            $.get("/lapro_copy_datafile");
            $.get("/check_datafile_exists").done(function(res) {
                if (res == "OK\n")
                    $('#porblemsLink').show();
                else
                    $('#porblemsLink').hide();
            }).fail(function(res) {});
        }
    }

    OSC.show_step = function(new_params) {
        step = new_params["MEASURE_STATE"].value;
        if (step === 1){
            $("#STATUS_MSG").text("CLICK RUN TO START").css('color', '#f00');
        }

        if (step === 2){
            $("#STATUS_MSG").text("WAITING").css('color', '#ff0');;
        }

        if (step === 3){
            $("#STATUS_MSG").text("DONE").css('color', '#0f0');;
        }

        if (step === 4){
            $("#STATUS_MSG").text("TRIGGER TIMEOUT").css('color', '#f00');;
        }
    }

    OSC.sendTrigInfo = function() {

        var data = '';
        // for (var i in OSC.triggers_list_channel) {
        //     var str = OSC.triggers_list_action[i];
        //     var val = str;

        //     if (val != -1) {
        //         data += '' + OSC.triggers_list_channel[i][3] + val; // (dinN,trigType)
        //     }
        // }

        for (var i = 0; i < OSC.triggers_list.length; i++) {
            if (OSC.triggers_list[i] != -1) {
                data += '' + i + OSC.triggers_list[i]; // (dinN,trigType)
            }
        }

        OSC.params.local['DINS'] = { value: data };
        OSC.sendParams();
    }

    OSC.sendACQ = function() {
        OSC.params.local['SAMPLE_RATE'] = { value: OSC.state.acq_speed };
        OSC.params.local['DECIMATE'] = { value: OSC.state.decimate };

        var val = $('#pre-sample-buf-val').val();
        val = (val == "") ? 100 : parseInt(val);
        val /= 1000; // Convert milliseconds to seconds
        val *= OSC.state.acq_speed; // Convert to samples number

        OSC.params.local['PRE_SAMPLE_BUFFER'] = {
            value: val
        };

        OSC.sendParams();
    }


    OSC.setMeasureMode = function(param) {
        OSC.params.local["MEASURE_SELECT"] = { value: param };
        OSC.sendParams();
    }


    OSC.updateMeasureMode = function(param) {
        if (param == 1) {
            $('#stem').text('');
            $('#select_mode').html('Ext. MODULE');
        } else {
            $('#stem').text('Ext. module used');
            $('#select_mode').html('&check; Ext. MODULE');
        }
    }


    OSC.param_callbacks["LA_RUN"] = OSC.process_run;
    OSC.param_callbacks["OSC_VIEV_PART"] = OSC.view_part;
    OSC.param_callbacks["SAMPLE_RATE"] = OSC.sample_rate;
    OSC.param_callbacks["SAMPLES_SUM"] = OSC.set_trig;
    OSC.param_callbacks["OSC_TIME_OFFSET"] = OSC.time_offset;
    OSC.param_callbacks["OSC_CURSOR_X1"] = OSC.cursor_x1;
    OSC.param_callbacks["OSC_CURSOR_X2"] = OSC.cursor_x2;
    OSC.param_callbacks["CREATE_DECODER"] = OSC.decoder_created;
    OSC.param_callbacks["MEASURE_STATE"] = OSC.show_step;

}(window.OSC = window.OSC || {}, jQuery));

// Page onload event handler
$(function() {

    var sendLogicData = function() {
        var mail = "support@redpitaya.com";
        var subject = "Decoding Help";
        var body = "DON'T FORGET TO ATTACH FILE!%0D%0A I need help for decoding this data%0D%0A";
        body += "%0D%0A%0D%0A------------------------------------%0D%0A" + "DEBUG INFO, DO NOT EDIT!%0D%0A" + "------------------------------------%0D%0A%0D%0A";
        body += "Parameters:" + "%0D%0A" + JSON.stringify({ parameters: OSC.params }) + "%0D%0A";
        body += "Browser:" + "%0D%0A" + JSON.stringify({ parameters: $.browser }) + "%0D%0A";

        var url = 'info/info.json';
        $.ajax({
            method: "GET",
            url: url
        }).done(function(msg) {
            console.log(msg.responseText);
            body += " info.json: " + "%0D%0A" + msg.responseText;
            document.location.href = "mailto:" + mail + "?subject=" + subject + "&body=" + body;
        }).fail(function(msg) {
            console.log(msg.responseText);
            body += " info.json: " + "%0D%0A" + msg.responseText;
            document.location.href = "mailto:" + mail + "?subject=" + subject + "&body=" + body;
        });
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

    $('#porblemsLink').click(function() {
        $('#decodehelp_dialog').modal('show');
    });

    $('#download_logicdata').click(function() {
        $('#hidden_link_logicdata').get(0).click();
    });

    $('#generate_help_email').click(function() {
        sendLogicData();
    });

    $('#calib-input').hide();
    $('#calib-input-text').hide();
    $('#modal-warning').hide();

    $('button').bind('activeChanged', function() {
        OSC.exitEditing(true);
    });
    $('select, input').on('change', function() {
        OSC.exitEditing(true);
    });

    // Initialize FastClick to remove the 300ms delay between a physical tap and the firing of a click event on mobile browsers
    //new FastClick(document.body);

    $(".dbl").on('dblclick', function() {
        var cls = $(this).attr('class');
        if (cls.indexOf('ch1') != -1)
            $('#OSC_CH1_OFFSET').val(0);
        if (cls.indexOf('ch2') != -1)
            $('#OSC_CH2_OFFSET').val(0);
        if (cls.indexOf('math') != -1)
            $('#OSC_MATH_OFFSET').val(0);
        if (cls.indexOf('trig') != -1)
            $('#OSC_TRIG_LEVEL').val(0);
        OSC.exitEditing(true);
    });

    // Process clicks on top menu buttons
    $('#OSC_RUN').on('click', function(ev) {
        ev.preventDefault();
        OSC.params.local['LA_RUN'] = {
            value: true
        };

        for (var i = 1; i < 5; i++) {
            var bus = "bus" + i;
            if (OSC.buses[bus].name !== undefined && OSC.buses[bus].name == "UART" && OSC.buses[bus].enabled) {
                OSC.buses[bus].samplerate = OSC.state.acq_speed;
                OSC.params.local[OSC.buses[bus].decoder + "_parameters"] = {
                    value: OSC.buses[bus]
                };
            }
            if (OSC.buses[bus].name !== undefined && OSC.buses[bus].name == "CAN" && OSC.buses[bus].enabled) {
                OSC.buses[bus].acq_speed = OSC.state.acq_speed;
                OSC.params.local[OSC.buses[bus].decoder + "_parameters"] = {
                    value: OSC.buses[bus]
                };
            }
        }

        OSC.ws.send(JSON.stringify({
            parameters: OSC.params.local
        }));
        var val = $('#pre-sample-buf-val').val();
        val = (val == "") ? 100 : parseFloat(val);
        val /= 1000; // Convert milliseconds to seconds
        val *= OSC.state.acq_speed; // Convert to samples number

        OSC.params.local['PRE_SAMPLE_BUFFER'] = {
            value: val
        };

        OSC.sendParams();
    });

    $('#OSC_STOP').on('click', function(ev) {
        ev.preventDefault();
        OSC.params.local['LA_RUN'] = {
            value: false
        };
        OSC.sendParams();
    });

    //  $('#OSC_SINGLE').on('click touchstart', function(ev) {
    $('#OSC_SINGLE').on('click', function(ev) {
        ev.preventDefault();
        OSC.params.local['OSC_SINGLE'] = {
            value: true
        };
        OSC.sendParams();
    });

    //  $('#OSC_AUTOSCALE').on('click touchstart', function(ev) {
    $('#OSC_AUTOSCALE').on('click', function(ev) {
        ev.preventDefault();
        OSC.params.local['OSC_AUTOSCALE'] = {
            value: true
        };
        OSC.sendParams();
    });

    // Export
    $('#downl_graph').on('click', function() {
        setTimeout(OSC.SaveGraphs, 30);
    });

    $('#downl_csv').on('click', function() {
        OSC.downloadDataAsCSV("laData.csv");
    });

    // Selecting active signal
    //  $('.menu-btn').on('click touchstart', function() {
    $('.menu-btn').on('click', function() {
        $('#right_menu .menu-btn').not(this).removeClass('active');
        if (!$(this).hasClass('active'))
            OSC.state.sel_sig_name = $(this).data('signal');
        else
            OSC.state.sel_sig_name = null;
        $('.y-offset-arrow').css('z-index', 10);
        $('#' + OSC.state.sel_sig_name + '_offset_arrow').css('z-index', 11);
    });

    // Opening a dialog for changing parameters
    //  $('.edit-mode').on('click touchstart', function() {
    $('.edit-mode').on('click', function() {
        OSC.state.editing = true;
        $('#right_menu').hide();
        $('#' + $(this).attr('id') + '_dialog').show();

        if ($.inArray($(this).data('signal'), ['ch1', 'ch2', 'math', 'output1', 'output2']) >= 0) {
            if (OSC.state.sel_sig_name)
                $('#right_menu .menu-btn.' + OSC.state.sel_sig_name).removeClass('active');
            if ($(this).data('signal') == 'output1' || $(this).data('signal') == 'output2' || $(this).data('signal') == 'math') {
                var out_enabled = $(this).data('signal') == 'output1' ? OSC.params.orig["OUTPUT1_STATE"].value : $(this).data('signal') == 'output2' ? OSC.params.orig["OUTPUT2_STATE"].value : OSC.params.orig["MATH_SHOW"].value;
                if (out_enabled) {
                    OSC.state.sel_sig_name = $(this).data('signal');
                    $('#right_menu .menu-btn.' + OSC.state.sel_sig_name).addClass('active');
                    $('.y-offset-arrow').css('z-index', 10);
                    $('#' + OSC.state.sel_sig_name + '_offset_arrow').css('z-index', 11);
                } else
                    OSC.state.sel_sig_name = null;
            } else {
                OSC.state.sel_sig_name = $(this).data('signal');

                //$('#right_menu .menu-btn.' + OSC.state.sel_sig_name).addClass('active');
                $('.y-offset-arrow').css('z-index', 10);
                $('#' + OSC.state.sel_sig_name + '_offset_arrow').css('z-index', 11);
            }
        }
    });

    // Close parameters dialog after Enter key is pressed
    $('input').keyup(function(event) {
        if (event.keyCode == 13) {
            OSC.exitEditing(true);
        }
    });

    // Close parameters dialog on close button click
    //  $('.close-dialog').on('click touchstart', function() {
    $('.close-dialog').on('click', function() {
        OSC.exitEditing();
    });

    $('#AUTO_BUTTON').on('click', function() {
        OSC.auto_set_trig();
    });

    // Joystick events
    $('#jtk_up').on('mousedown touchstart', function() {
        $('#jtk_btns').attr('src', 'img/node_up.png');
    });
    $('#jtk_left').on('mousedown touchstart', function() {
        $('#jtk_btns').attr('src', 'img/node_left.png');
    });
    $('#jtk_right').on('mousedown touchstart', function() {
        $('#jtk_btns').attr('src', 'img/node_right.png');
    });
    $('#jtk_down').on('mousedown touchstart', function() {
        $('#jtk_btns').attr('src', 'img/node_down.png');
    });

    //  $('#jtk_fine').on('click touchstart', function(ev){
    $('#jtk_fine').on('click', function(ev) {
        var img = $('#jtk_fine');

        if (img.attr('src') == 'img/fine.png') {
            img.attr('src', 'img/fine_active.png');
            OSC.state.fine = true;
        } else {
            img.attr('src', 'img/fine.png');
            OSC.state.fine = false;
        }

        ev.preventDefault();
        ev.stopPropagation();
    });

    $(document).on('mouseup touchend', function() {
        $('#jtk_btns').attr('src', 'img/node_fine.png');
    });

    //  $('#jtk_up, #jtk_down').on('click touchstart', function(ev) {
    $('#jtk_up, #jtk_down').on('click', function(ev) {
        ev.preventDefault();
        ev.stopPropagation();
        OSC.changeYZoom(ev.target.id == 'jtk_down' ? '+' : '-');
        OSC.guiHandler();
    });

    //  $('#jtk_left, #jtk_right').on('click touchstart', function(ev) {
    var time_zoom = function(ev, offsetPx, byMouseWheel) {
        for (var i = 1; i < 5; i++) {
            var bus = "bus" + i;
            if (OSC.buses[bus].name !== undefined && OSC.buses[bus].name == "UART" && OSC.buses[bus].enabled) {
                OSC.buses[bus].samplerate = OSC.state.acq_speed;
                OSC.params.local[OSC.buses[bus].decoder + "_parameters"] = {
                    value: OSC.buses[bus]
                };
            }
            if (OSC.buses[bus].name !== undefined && OSC.buses[bus].name == "CAN" && OSC.buses[bus].enabled) {
                OSC.buses[bus].acq_speed = OSC.state.acq_speed;
                OSC.params.local[OSC.buses[bus].decoder + "_parameters"] = {
                    value: OSC.buses[bus]
                };
            }
        }

        OSC.params.local = {};

        OSC.state.resized = true;

        OSC.scaleWasChanged = OSC.changeXZoom(ev);
        OSC.updateCursorWithNewScale();
        var graphWidth = $('#graphs').width();
        var pxPerPoint = graphWidth / 1024;
        // For no moving signals to right
        if (OSC.scaleWasChanged) {
            var cursorPoint = 512; // For scaling from center (just initialize and +/- buttons);
            if (byMouseWheel) {
                var pointNum = offsetPx / pxPerPoint; // ?!
                cursorPoint = (!OSC.state.fine) ? pointNum : pointNum / 10;
            } else
                cursorPoint = (!OSC.state.fine) ? cursorPoint : cursorPoint / 10;
            var scale = (!OSC.state.fine) ? 2 : 1.1;

            if (ev == '+') {
                if ((OSC.ch1_size * OSC.time_scale - 1024))
                    OSC.counts_offset *= scale;
                OSC.counts_offset += cursorPoint;
            } else if (ev == '-') {
                OSC.counts_offset -= cursorPoint;
                OSC.counts_offset /= scale;
            }
            if (OSC.counts_offset > (OSC.ch1_size * OSC.time_scale - 1024))
                OSC.counts_offset = OSC.ch1_size * OSC.time_scale - 1024;
            if (OSC.counts_offset < 0)
                OSC.counts_offset = 0;

            // Set 'do not change time_scale' flag
            if (OSC.ch1_size * OSC.time_scale <= 1024 && OSC.counts_offset == 0)
                OSC.allSignalShown = true;
        }

        var trigPosInPoints = (OSC.samples_sum * OSC.time_scale - OSC.counts_offset) * $('#graphs').width() / 1024 - $('#time_offset_arrow').width() / 2;
        $('#time_offset_arrow').css('left', trigPosInPoints);


        // Calculate time per division
        var samplerate = OSC.state.acq_speed;
        var samples = 1024;
        var mul = 1000;
        var scale = OSC.time_scale;
        var dev_num = 10;
        var timePerDevInMs = (((samples / scale) / samplerate) * mul) / dev_num;
        $('#OSC_TIME_SCALE').text(OSC.convertTime(timePerDevInMs));

        var graph_width = $('#graph_grid').outerWidth();
        ms_per_px = (timePerDevInMs * 10) / graph_width;
        var new_value = OSC.trigger_position * ms_per_px;
        // I cry when I see this code :'(
        $('#OSC_TIME_OFFSET').text(OSC.convertTime(5 * (($('#time_offset_arrow').position().left + $('#time_offset_arrow').width() / 2) / ($('#graphs').width() / 2) - 1) * 102400 / OSC.time_scale / OSC.state.acq_speed));
        OSC.guiHandler();
    }

    $('#jtk_left, #jtk_right').on('click', function(ev) {
        ev.preventDefault();
        ev.stopPropagation();
        time_zoom(ev.target.id == 'jtk_left' ? '-' : '+', 0, false)
    });

    $('.y-offset-arrow').on('mousedown', function() {
        OSC.state.line_moving = true;
    });

    $('.y-offset-arrow').on('mouseup', function() {
        OSC.state.line_moving = false;
    });

    // Voltage offset arrow dragging
    $('.y-offset-arrow').draggable({
        axis: 'y',
        containment: 'parent',
        start: function(ev, ui) {
            OSC.state.line_moving = true;
        },
        drag: function(ev, ui) {
            OSC.state.line_moving = true;
            var margin_top = Math.abs(parseInt(ui.helper.css('marginTop')));
            var min_top = ((ui.helper.height() / 2) + margin_top) * -1;
            var max_top = $('#graphs').height() - margin_top;

            if (ui.position.top < min_top) {
                ui.position.top = min_top;
            } else if (ui.position.top > max_top) {
                ui.position.top = max_top;
            }

            OSC.updateYOffset(ui, false);

        },
        stop: function(ev, ui) {
            OSC.state.line_moving = false;
            if (!OSC.state.simulated_drag) {
                OSC.updateYOffset(ui, true);
                $('#info_box').empty();
            }
        }
    });

    // Time offset arrow dragging
    $('#time_offset_arrow').draggable({
        axis: 'x',
        containment: 'parent',
        drag: function(ev, ui) {
            OSC.state.line_moving = true;
            var graph_width = $('#graph_grid').outerWidth();
            var zero_pos = (graph_width + 2) / 2;

            // Dragging signals
            if (!$.isEmptyObject(OSC.graphs)) {
                var graph_width = $('#graph_grid').width();
                var pxPerPoint = graph_width / 1024;
                var diff = event.pageX - curXPos;

                if (diff < 0 && (OSC.ch1_size <= ((OSC.counts_offset + 1024) / OSC.time_scale)))
                    return;

                var signalDiff = diff / pxPerPoint;
                OSC.counts_offset -= signalDiff;

                curXPos = event.pageX;
                if (OSC.counts_offset <= 0) {
                    OSC.counts_offset = 0;
                    ev.preventDefault();
                }


                OSC.offsetForDecoded = OSC.counts_offset;

                console.log(OSC.counts_offset);

                // Scroll
                OSC.scrollDataArea();
                var trigPosInPoints = (OSC.samples_sum * OSC.time_scale - OSC.counts_offset) * $('#graphs').width() / 1024 - $('#time_offset_arrow').width() / 2;
                $('#time_offset_arrow').css('left', trigPosInPoints);
                // I cry when I see this code :'(
                $('#OSC_TIME_OFFSET').text(OSC.convertTime(5 * (($('#time_offset_arrow').position().left + $('#time_offset_arrow').width() / 2) / ($('#graphs').width() / 2) - 1) * 102400 / OSC.time_scale / OSC.state.acq_speed));

            }

            // Calculate time per division
            var samplerate = OSC.state.acq_speed;
            var samples = 1024;
            var mul = 1000;
            var scale = OSC.time_scale;
            var dev_num = 10;
            var timeInMs = (((samples / scale) / samplerate) * mul);

            ms_per_px = timeInMs / graph_width;
            OSC.trigger_position = (zero_pos - ui.position.left - ui.helper.width() / 2 - 1);
            var new_value = +(((zero_pos - ui.position.left - ui.helper.width() / 2 - 1) * ms_per_px).toFixed(6));
            var buf_width = graph_width - 2;

            $('#info_box').html('Time offset ' + OSC.convertTime(new_value));
            $('#OSC_TIME_OFFSET').text(OSC.convertTime(new_value));
            OSC.time_offset_str = OSC.convertTime(new_value);
            OSC.guiHandler();
        },
        stop: function(ev, ui) {
            OSC.state.line_moving = false;
            if (!OSC.state.simulated_drag) {
                var graph_width = $('#graph_grid').outerWidth();
                var zero_pos = (graph_width + 2) / 2;

                // Calculate time per division
                var samplerate = OSC.state.acq_speed;
                var samples = 1024;
                var mul = 1000;
                var scale = OSC.time_scale;
                var dev_num = 10;
                var timeInMs = (((samples / scale) / samplerate) * mul);

                ms_per_px = timeInMs / graph_width;

                OSC.params.local['OSC_TIME_OFFSET'] = {
                    value: (zero_pos - ui.position.left - ui.helper.width() / 2 - 1) * ms_per_px
                };
                OSC.sendParams();
                $('#info_box').empty();
                OSC.guiHandler();
            }
        }
    });

    // Time offset rectangle dragging
    $('#buf_time_offset').draggable({
        axis: 'x',
        containment: 'parent',
        drag: function(ev, ui) {
            OSC.state.line_moving = true;
            var buf_width = $('#buffer').width();
            var zero_pos = (buf_width + 2) / 2;

            // Calculate time per division
            var samplerate = OSC.state.acq_speed;
            var samples = 1024;
            var mul = 1000;
            var scale = OSC.time_scale;
            var dev_num = 10;
            var timeInMs = (((samples / scale) / samplerate) * mul);

            var ms_per_px = timeInMs / buf_width;
            var ratio = buf_width / (buf_width * OSC.params.orig['OSC_VIEV_PART'].value);
            var new_value = +(((zero_pos - ui.position.left - ui.helper.width() / 2 - 1) * ms_per_px * ratio).toFixed(2));
            var px_offset = -(new_value / ms_per_px + $('#time_offset_arrow').width() / 2 + 1);

            $('#info_box').html('Time offset ' + OSC.convertTime(new_value));
            $('#OSC_TIME_OFFSET').text(OSC.convertTime(new_value));
            OSC.time_offset_str = OSC.convertTime(new_value);
            $('#time_offset_arrow').css('left', (buf_width + 2) / 2 + px_offset);

            // I cry when I see this code :'(
            $('#OSC_TIME_OFFSET').text(OSC.convertTime(5 * (($('#time_offset_arrow').position().left + $('#time_offset_arrow').width() / 2) / ($('#graphs').width() / 2) - 1) * 102400 / OSC.time_scale / OSC.state.acq_speed));
            OSC.guiHandler();
        },
        stop: function(ev, ui) {
            if (!OSC.state.simulated_drag) {
                var buf_width = $('#buffer').width();
                var zero_pos = (buf_width + 2) / 2;
                OSC.state.line_moving = false;
                // Calculate time per division
                var samplerate = OSC.state.acq_speed;
                var samples = 1024;
                var mul = 1000;
                var scale = OSC.time_scale;
                var dev_num = 10;
                var timeInMs = (((samples / scale) / samplerate) * mul);

                var ms_per_px = timeInMs / buf_width;
                $('#info_box').empty();
                OSC.guiHandler();
            }
        }
    });

    // X cursor arrows dragging
    $('#cur_x1_arrow, #cur_x2_arrow').draggable({
        axis: 'x',
        containment: 'parent',
        start: function(ev, ui) {
            OSC.state.line_moving = true;
            OSC.state.cursor_dragging = true;
        },
        drag: function(ev, ui) {
            OSC.updateXCursorElems(ui, false);
        },
        stop: function(ev, ui) {
            OSC.state.line_moving = false;
            OSC.updateXCursorElems(ui, true);
            OSC.state.cursor_dragging = false;
        }
    });

    // Touch events
    $(document).on('touchstart', '.plot', function(ev) {
        ev.preventDefault();

        // Multi-touch is used for zooming
        if (!OSC.touch.start && ev.originalEvent.touches.length > 1) {
            OSC.touch.zoom_axis = null;
            OSC.touch.start = [{
                clientX: ev.originalEvent.touches[0].clientX,
                clientY: ev.originalEvent.touches[0].clientY
            }, {
                clientX: ev.originalEvent.touches[1].clientX,
                clientY: ev.originalEvent.touches[1].clientY
            }];
        }
        // Single touch is used for changing offset
        else if (!OSC.state.simulated_drag) {
            OSC.state.simulated_drag = true;
            OSC.touch.offset_axis = null;
            OSC.touch.start = [{
                clientX: ev.originalEvent.touches[0].clientX,
                clientY: ev.originalEvent.touches[0].clientY
            }];
        }
    });

    $(document).on('touchmove', '.plot', function(ev) {
        ev.preventDefault();

        // Multi-touch is used for zooming
        if (ev.originalEvent.touches.length > 1) {

            OSC.touch.curr = [{
                clientX: ev.originalEvent.touches[0].clientX,
                clientY: ev.originalEvent.touches[0].clientY
            }, {
                clientX: ev.originalEvent.touches[1].clientX,
                clientY: ev.originalEvent.touches[1].clientY
            }];

            // Find zoom axis
            if (!OSC.touch.zoom_axis) {
                var delta_x = Math.abs(OSC.touch.curr[0].clientX - OSC.touch.curr[1].clientX);
                var delta_y = Math.abs(OSC.touch.curr[0].clientY - OSC.touch.curr[1].clientY);

                if (Math.abs(delta_x - delta_y) > 10) {
                    if (delta_x > delta_y) {
                        OSC.touch.zoom_axis = 'x';
                    } else if (delta_y > delta_x) {
                        OSC.touch.zoom_axis = 'y';
                    }
                }
            }

            // Skip first touch event
            if (OSC.touch.prev) {

                // Time zoom
                if (OSC.touch.zoom_axis == 'x') {
                    var prev_delta_x = Math.abs(OSC.touch.prev[0].clientX - OSC.touch.prev[1].clientX);
                    var curr_delta_x = Math.abs(OSC.touch.curr[0].clientX - OSC.touch.curr[1].clientX);

                    if (OSC.state.fine || Math.abs(curr_delta_x - prev_delta_x) > $(this).width() * 0.9 / OSC.time_steps.length) {
                        var new_scale = OSC.changeXZoom((curr_delta_x < prev_delta_x ? '+' : '-'), OSC.touch.new_scale_x, true);

                        if (new_scale !== null) {
                            OSC.touch.new_scale_x = new_scale;
                            $('#info_box').html('Time scale ' + OSC.convertTime(new_scale) + '/div');
                        }

                        OSC.touch.prev = OSC.touch.curr;
                    }
                }
                // Voltage zoom
                else if (OSC.touch.zoom_axis == 'y' && OSC.state.sel_sig_name) {
                    var prev_delta_y = Math.abs(OSC.touch.prev[0].clientY - OSC.touch.prev[1].clientY);
                    var curr_delta_y = Math.abs(OSC.touch.curr[0].clientY - OSC.touch.curr[1].clientY);

                    if (OSC.state.fine || Math.abs(curr_delta_y - prev_delta_y) > $(this).height() * 0.9 / OSC.voltage_steps.length) {
                        var new_scale = OSC.changeYZoom((curr_delta_y < prev_delta_y ? '+' : '-'), OSC.touch.new_scale_y, true);

                        if (new_scale !== null) {
                            OSC.touch.new_scale_y = new_scale;
                            $('#info_box').html('Vertical scale ' + OSC.convertVoltage(new_scale) + '/div');
                        }

                        OSC.touch.prev = OSC.touch.curr;
                    }
                }
            } else if (OSC.touch.prev === undefined) {
                OSC.touch.prev = OSC.touch.curr;
            }
        }
        // Single touch is used for changing offset
        else if (OSC.state.simulated_drag) {

            // Find offset axis
            if (!OSC.touch.offset_axis) {
                var delta_x = Math.abs(OSC.touch.start[0].clientX - ev.originalEvent.touches[0].clientX);
                var delta_y = Math.abs(OSC.touch.start[0].clientY - ev.originalEvent.touches[0].clientY);

                if (delta_x > 5 || delta_y > 5) {
                    if (delta_x > delta_y) {
                        OSC.touch.offset_axis = 'x';
                    } else if (delta_y > delta_x) {
                        OSC.touch.offset_axis = 'y';
                    }
                }
            }

            if (OSC.touch.prev) {

                // Time offset
                if (OSC.touch.offset_axis == 'x') {
                    var delta_x = ev.originalEvent.touches[0].clientX - OSC.touch.prev[0].clientX;

                    if (delta_x != 0) {
                        //$('#time_offset_arrow').simulate('drag', { dx: delta_x, dy: 0 });
                        $('#time_offset_arrow').simulate('drag', {
                            dx: delta_x,
                            dy: 0
                        });
                    }
                }
                // Voltage offset
                else if (OSC.touch.offset_axis == 'y' && OSC.state.sel_sig_name) {
                    var delta_y = ev.originalEvent.touches[0].clientY - OSC.touch.prev[0].clientY;

                    if (delta_y != 0) {
                        $('#' + OSC.state.sel_sig_name + '_offset_arrow').simulate('drag', {
                            dx: 0,
                            dy: delta_y
                        });
                    }
                }
            }

            OSC.touch.prev = [{
                clientX: ev.originalEvent.touches[0].clientX,
                clientY: ev.originalEvent.touches[0].clientY
            }];
        }
    });

    $(document).on('touchend', '.plot', function(ev) {
        ev.preventDefault();

        if (OSC.state.simulated_drag) {
            OSC.state.simulated_drag = false;

            if (OSC.touch.offset_axis == 'x') {
                //$('#time_offset_arrow').simulate('drag', { dx: 0, dy: 0 });
                $('#buf_time_offset').simulate('drag', {
                    dx: 0,
                    dy: 0
                });
            } else if (OSC.touch.offset_axis == 'y' && OSC.state.sel_sig_name) {
                $('#' + OSC.state.sel_sig_name + '_offset_arrow').simulate('drag', {
                    dx: 0,
                    dy: 0
                });
            }

            delete OSC.touch.start;
            delete OSC.touch.prev;
        } else {
            // Send new scale
            if (OSC.touch.new_scale_y !== undefined) {
                OSC.params.local['OSC_' + OSC.state.sel_sig_name.toUpperCase() + '_SCALE'] = {
                    value: OSC.touch.new_scale_y
                };
                OSC.sendParams();
            } else if (OSC.touch.new_scale_x !== undefined) {
                OSC.params.local['OSC_TIME_SCALE'] = {
                    value: OSC.touch.new_scale_x
                };
                OSC.sendParams();
            }
        }

        // Reset touch information
        OSC.touch = {};
        $('#info_box').empty();
    });

    // Preload images which are not visible at the beginning
    $.preloadImages = function() {
        for (var i = 0; i < arguments.length; i++) {
            $('<img />').attr('src', 'img/' + arguments[i]);
        }
    }
    $.preloadImages(
        'node_up.png',
        'node_left.png',
        'node_right.png',
        'node_down.png',
        'fine_active.png'
    );
    OSC.drawGraphGrid();
    // Bind to the window resize event to redraw the graph; trigger that event to do the first drawing
    $(window).resize(function() {

        var window_width = window.innerWidth;
        var window_height = window.innerHeight;
        if (window_width > 768 && window_height > 580) {
            var global_width = window_width - 30,
                global_height = global_width / 1.77885;
            if (window_height < global_height) {
                global_height = window_height - 70 * 1.77885;
                global_width = global_height * 1.77885;
            }

            $('#global_container').css('width', global_width);
            $('#global_container').css('height', global_height);


            OSC.drawGraphGrid();
            var main_width = $('#main').outerWidth(true);
            var main_height = $('#main').outerHeight(true);
            $('#global_container').css('width', main_width);
            $('#global_container').css('height', main_height);

            OSC.drawGraphGrid();
            main_width = $('#main').outerWidth(true);
            main_height = $('#main').outerHeight(true);
            window_width = window.innerWidth;
            window_height = window.innerHeight;
            console.log("window_width = " + window_width);
            console.log("window_height = " + window_height);
            if (main_height > (window_height - 80)) {

                $('#global_container').css('height', window_height - 80);
                $('#global_container').css('width', 1.82 * (window_height - 80));
                OSC.drawGraphGrid();
                $('#global_container').css('width', $('#main').outerWidth(true) - 2);
                $('#global_container').css('height', $('#main').outerHeight(true) - 2);
                OSC.drawGraphGrid();
            }
        }
        OSC.checkAndShowArrows();
        setTimeout(function() { OSC.checkAndShowArrows(); }, 500);

        $('#global_container').offset({
            left: (window_width - $('#global_container').width()) / 2
        });

        // Resize the graph holders
        $('.plot').css($('#graph_grid').css(['height', 'width']));

        // Hide all graphs, they will be shown next time signal data is received
        $('#graphs .plot').hide();

        // Hide offset arrows, trigger level line and arrow
        $('.y-offset-arrow, #time_offset_arrow, #buf_time_offset, #trig_level_arrow, #trigger_level').hide();

        if (OSC.ws) {
            OSC.params.local['in_command'] = {
                value: 'send_all_params'
            };
            OSC.sendParams();
        }

        // Reset left position for trigger level arrow, it is added by jQ UI draggable
        $('#trig_level_arrow').css('left', '');
        //$('#graphs').height($('#graph_grid').height() - 5);
        // Set the resized flag
        for (var i = 0; i < 8; i++) {
            if (OSC.enabled_channels[i])
                OSC.updateChVisibility(i);
        }
        OSC.state.resized = true;

    }).resize();

    var unload = function() {
        OSC.save_params();
        OSC.ws.onclose = function() {}; // disable onclose handler first
        OSC.ws.close();
        $.ajax({
            url: OSC.config.stop_app_url,
            async: false
        });
    }

    // Stop the application when page is unloaded
    $(window).on('beforeunload', function() {
        unload();
    });

    $(document).bind('keydown keyup', function(e) {
        if (e.which === 116)
            unload();
        if (e.which === 82 && e.ctrlKey)
            unload();
    });

    // Init help
    Help.init(helpListLA);
    Help.setState("idle");

    // Everything prepared, start application
    OSC.startApp();

    $('#protocol_selector').change(function() {
        $('.decoder-window').hide();
        $($(this).val()).show();
    });
    $('.btn-less').click(function() {
        var inp = $(this).find('input');
        $('.decoder-tab').hide();
        $(inp.attr('data-attr')).show();
    });

    var enableChannel = function() {
        var element = $(this);
        var img = $(this).find('img');
        var arr = ["CH1_ENABLED", "CH2_ENABLED", "CH3_ENABLED", "CH4_ENABLED", "CH5_ENABLED", "CH6_ENABLED", "CH7_ENABLED", "CH8_ENABLED"];
        var arr2 = ["BUS1_ENABLED", "BUS2_ENABLED", "BUS3_ENABLED", "BUS4_ENABLED"];
        var ch = arr.indexOf(element.attr('id'));
        var bn = arr2.indexOf(element.attr('id'));
        var inp = element.parent().parent().find('#CH' + (ch + 1) + "_NAME")

        if (img.is(":visible")) {
            img.hide();
            // TODO ON CHECK CHANGED
            if (ch != -1) {
                OSC.enabled_channels[ch] = false;
                $('#ch' + (ch + 1) + '_offset_arrow').hide();
            }
            if (bn != -1) {
                if (OSC.buses["bus" + (bn + 1)].enabled !== undefined) {
                    OSC.destroyDecoder("bus" + (bn + 1), "-");
                    OSC.buses["bus" + (bn + 1)].enabled = false;
                }
            }
        } else {
            if (ch != -1) {
                if (inp.val() == undefined || inp.val() == "")
                    inp.val("DIN" + ch);
                OSC.enabled_channels[ch] = true;
                OSC.updateChVisibility(ch)
                img.show();
            }
            if (bn != -1) {
                if ($('#BUS' + (bn + 1) + '_NAME').text() == ('BUS' + bn)) {
                    OSC.startEditBus('BUS' + (bn + 1) + '_SETTINGS');
                } else {
                    var bus = "bus" + (bn + 1);
                    if (OSC.buses[bus] != undefined && OSC.buses[bus].name != undefined) {
                        switch (OSC.buses[bus].name) {
                            case "UART":
                                if (OSC.buses[bus].serial != -1 && !isChannelInUse(OSC.buses[bus].serial, bus)) {
                                    OSC.params.local['CREATE_DECODER'] = {
                                        value: 'uart'
                                    };

                                    OSC.buses[bus].decoder = 'uart' + OSC.state.decoder_id;
                                    OSC.state.decoder_id++;
                                    OSC.params.local['DECODER_NAME'] = {
                                        value: OSC.buses[bus].decoder
                                    };

                                    OSC.sendParams();
                                } else
                                    OSC.startEditBus('BUS' + (bn + 1) + '_SETTINGS');
                                break;
                            case "CAN":
                                if (OSC.buses[bus].serial != -1 && !isChannelInUse(OSC.buses[bus].serial, bus)) {
                                    OSC.params.local['CREATE_DECODER'] = {
                                        value: 'can'
                                    };

                                    OSC.buses[bus].decoder = 'can' + OSC.state.decoder_id;
                                    OSC.state.decoder_id++;
                                    OSC.params.local['DECODER_NAME'] = {
                                        value: OSC.buses[bus].decoder
                                    };

                                    OSC.sendParams();
                                } else
                                    OSC.startEditBus('BUS' + (bn + 1) + '_SETTINGS');
                                break;
                            case "I2C":
                                if (OSC.buses[bus].sda != -1 && !isChannelInUse(OSC.buses[bus].sda, bus)) {
                                    if (OSC.buses[bus].scl != -1 && !isChannelInUse(OSC.buses[bus].scl, bus)) {
                                        OSC.params.local['CREATE_DECODER'] = {
                                            value: 'i2c'
                                        };
                                        OSC.buses[bus].decoder = 'i2c' + OSC.state.decoder_id;
                                        OSC.state.decoder_id++;
                                        OSC.params.local['DECODER_NAME'] = {
                                            value: OSC.buses[bus].decoder
                                        };

                                        OSC.sendParams();
                                    } else
                                        OSC.startEditBus('BUS' + (bn + 1) + '_SETTINGS');
                                } else
                                    OSC.startEditBus('BUS' + (bn + 1) + '_SETTINGS');

                                break;
                            case "SPI":
                                if (OSC.buses[bus].clk != -1 && !isChannelInUse(OSC.buses[bus].clk, bus)) {
                                    var miso_ok = false,
                                        mosi_ok = false,
                                        cs_ok;

                                    if (OSC.buses[bus].miso != -1 && !isChannelInUse(OSC.buses[bus].miso, bus)) {
                                        miso_ok = true;
                                    } else if (OSC.buses[bus].miso == -1)
                                        miso_ok = true;

                                    if (OSC.buses[bus].mosi != -1 && !isChannelInUse(OSC.buses[bus].mosi, bus)) {
                                        mosi_ok = true;
                                    } else if (OSC.buses[bus].mosi == -1)
                                        mosi_ok = true;

                                    if (OSC.buses[bus].cs != -1 && !isChannelInUse(OSC.buses[bus].cs, bus)) {
                                        cs_ok = true;
                                    } else if (OSC.buses[bus].cs == -1)
                                        cs_ok = true;

                                    if (miso_ok && mosi_ok && cs_ok) {
                                        if (OSC.buses[bus].mosi != -1) {
                                            OSC.buses[bus].mosi_decoder = 'spi' + OSC.state.decoder_id;
                                            OSC.params.local['CREATE_DECODER'] = {
                                                value: 'spi'
                                            };
                                            OSC.params.local['DECODER_NAME'] = {
                                                value: 'spi' + OSC.state.decoder_id
                                            };
                                            OSC.state.decoder_id++;
                                        }


                                        if (OSC.buses[bus].miso != -1) {
                                            OSC.buses[bus].miso_decoder = 'spi' + OSC.state.decoder_id;
                                            OSC.params.local['CREATE_DECODER'] = {
                                                value: 'spi'
                                            };
                                            OSC.params.local['DECODER_NAME'] = {
                                                value: 'spi' + OSC.state.decoder_id
                                            };
                                            OSC.state.decoder_id++;
                                        }
                                        OSC.sendParams();

                                    } else
                                        OSC.startEditBus('BUS' + (bn + 1) + '_SETTINGS');
                                } else
                                    OSC.startEditBus('BUS' + (bn + 1) + '_SETTINGS');
                                break;
                        }
                    }
                    img.show();
                }
            }
        }
        OSC.guiHandler();
    }

    $('.enable-ch').click(enableChannel);

    $('.ch-name-inp').change(function() {
        var arr = ["CH1_NAME", "CH2_NAME", "CH3_NAME", "CH4_NAME", "CH5_NAME", "CH6_NAME", "CH7_NAME", "CH8_NAME"];
        var element = $(this);
        var ch = arr.indexOf(element.attr('id'));
        if (ch != -1) {
            OSC.updateChVisibility(ch);
        }
    })

    $('#OSC_CURSOR_X1').click(function() {
        var btn = $(this);
        if (!btn.hasClass('active'))
            OSC.enableCursor('x1');
        else
            OSC.disableCursor('x1');
    });

    $('#OSC_CURSOR_X2').click(function() {
        var btn = $(this);
        if (!btn.hasClass('active'))
            OSC.enableCursor('x2');
        else
            OSC.disableCursor('x2');
    });

    $('.bus-settings-btn').click(function() {
        OSC.startEditBus($(this).attr('id'));
    });

    var isChannelInUse = function(ch, ex_bus) {
        for (var i = 1; i < 5; i++) {
            if (ex_bus !== undefined && ex_bus == i)
                continue;
            var bus = 'bus' + i;
            if (OSC.buses[bus] !== undefined && OSC.buses[bus].name !== undefined && OSC.buses[bus].enabled) {
                if (OSC.buses[bus].name == "UART" && (OSC.buses[bus].rx == ch || OSC.buses[bus].rx == ch))
                    return i;

                if (OSC.buses[bus].name == "SPI" &&
                    (OSC.buses[bus].scl == ch || OSC.buses[bus].miso == ch || OSC.buses[bus].mosi == ch || OSC.buses[bus].cs == ch))
                    return i;

                if (OSC.buses[bus].name == "I2C" && (OSC.buses[bus].scl == ch || OSC.buses[bus].sda == ch))
                    return i;
                if (OSC.buses[bus].name == "CAN" && (OSC.buses[bus].can_rx == ch))
                    return i;
            }
        }
        return -1;
    }

    $('#trig').click(function() {
        $('#TRIGGER_CHANNEL').empty();
        for (var i = 1; i < 9; i++) {
            $('#TRIGGER_CHANNEL').append('<option value="' + (($('#CH' + i + '_NAME').val() != "") ? $('#CH' + i + '_NAME').val() : $('#CH' + i + '_NAME').attr('placeholder')) + '">' + (($('#CH' + i + '_NAME').val() != "") ? $('#CH' + i + '_NAME').val() : $('#CH' + i + '_NAME').attr('placeholder')) + '</option>');
        }
    });

    var applyDecoder = function() {
        var protocol = $('#protocol_selector option:selected').text();
        $('#warning-dialog').hide();
        // Saving local data
        var bus = 'bus' + OSC.state.bus_editing;
        if (protocol == "UART") {
            var baudrate = $('#uart_baudrate').val();
            var serial = $('#uart_serial').val();
            var rxtx = $('#uart_rxtx').val();

            if (baudrate <= 0) {
                $('#warn-message').text("Baudrate value is incorrect");
                $('#warning-dialog').show();
                return;
            }
            if (serial < 0) {
                $('#warn-message').text("Please specify channel for decoding");
                $('#warning-dialog').show();
                return;
            }
            if (isChannelInUse(serial, OSC.state.bus_editing) != -1) {
                $('#warn-message').text($('#uart_serial option:selected').text() + " is already in use");
                $('#warning-dialog').show();
                return;
            }

            OSC.destroyDecoder(bus, "UART");
            var upd = OSC.needUpdateDecoder(bus, "UART");

            if (!upd)
                OSC.buses[bus] = {};
            else
                OSC.hideInfoArrow(parseInt(OSC.buses[bus].rx) - 1);

            OSC.buses[bus].name = "UART";
            OSC.buses[bus].enabled = true;

            OSC.buses[bus].rx = serial;
            OSC.buses[bus].rxtxstr = rxtx;

            OSC.buses[bus].baudrate = baudrate;
            OSC.buses[bus].num_data_bits = $('#uart_data_length').val();
            OSC.buses[bus].num_stop_bits = $('#uart_stop_bits').val();
            OSC.buses[bus].parity = $('#uart_parity').val();
            OSC.buses[bus].bitOrder = $('#uart_order').val();
            OSC.buses[bus].invert_rx = $('#uart_invert').val();

            OSC.buses[bus].samplerate = OSC.state.acq_speed;

            OSC.showInfoArrow(parseInt(OSC.buses[bus].rx) - 1);

            if (!upd) {
                OSC.buses[bus].decoder = 'uart' + OSC.state.decoder_id;
                OSC.state.decoder_id++;
            }

            if (!upd) {
                OSC.params.local['CREATE_DECODER'] = {
                    value: 'uart'
                };
                OSC.params.local['DECODER_NAME'] = {
                    value: OSC.buses[bus].decoder
                };
            } else {
                OSC.params.local[OSC.buses[bus].decoder + "_parameters"] = {
                    value: OSC.buses[bus]
                };
            }

            OSC.sendParams();
        } else if (protocol == "SPI") {

            var miso = $('#spi_miso').val();
            var mosi = $('#spi_mosi').val();
            var clk = $('#spi_clk').val();
            var cs = $('#spi_cs').val();
            var invert_bit = $('#spi_invert').val();

            if (isChannelInUse(miso, OSC.state.bus_editing) != -1) {
                $('#warn-message').text($('#spi_miso option:selected').text() + " is already in use");
                $('#warning-dialog').show();
                return;
            }
            if (isChannelInUse(mosi, OSC.state.bus_editing) != -1) {
                $('#warn-message').text($('#spi_mosi option:selected').text() + " is already in use");
                $('#warning-dialog').show();
                return;
            }
            if (isChannelInUse(clk, OSC.state.bus_editing) != -1) {
                $('#warn-message').text($('#spi_clk option:selected').text() + " is already in use");
                $('#warning-dialog').show();
                return;
            }
            if (isChannelInUse(cs, OSC.state.bus_editing) != -1) {
                $('#warn-message').text($('#spi_cs option:selected').text() + " is already in use");
                $('#warning-dialog').show();
                return;
            }
            if (miso == -1 && mosi == -1) {
                $('#warn-message').text("You have to specify MISO or MOSI or both lines for decoding");
                $('#warning-dialog').show();
                return;
            }
            if (clk == -1) {
                $('#warn-message').text("You have to specify CLK line");
                $('#warning-dialog').show();
                return;
            }
            if (miso == mosi || miso == clk || miso == cs ||
                mosi == clk || mosi == cs || clk == cs) {
                $('#warn-message').text("You can't specify same input lines");
                $('#warning-dialog').show();
                return;
            }

            OSC.destroyDecoder(bus, "SPI");
            var upd = OSC.needUpdateDecoder(bus, "SPI");

            if (!upd)
                OSC.buses[bus] = {};
            else {
                if (OSC.buses[bus].clk !== clk)
                    OSC.hideInfoArrow(parseInt(OSC.buses[bus].clk) - 1);
                if (OSC.buses[bus].miso !== miso)
                    OSC.hideInfoArrow(parseInt(OSC.buses[bus].miso) - 1);
                if (OSC.buses[bus].mosi !== mosi)
                    OSC.hideInfoArrow(parseInt(OSC.buses[bus].mosi) - 1);
                if (OSC.buses[bus].cs !== cs)
                    OSC.hideInfoArrow(parseInt(OSC.buses[bus].cs) - 1);
            }

            OSC.buses[bus].name = "SPI";
            OSC.buses[bus].enabled = true;

            OSC.buses[bus].clk = clk;
            OSC.buses[bus].miso = miso;
            OSC.buses[bus].mosi = mosi;
            OSC.buses[bus].cs = cs;
            OSC.buses[bus].invert_bit = invert_bit;

            OSC.showInfoArrow(parseInt(OSC.buses[bus].clk) - 1);
            OSC.showInfoArrow(parseInt(OSC.buses[bus].miso) - 1);
            OSC.showInfoArrow(parseInt(OSC.buses[bus].mosi) - 1);
            OSC.showInfoArrow(parseInt(OSC.buses[bus].cs) - 1);

            OSC.buses[bus].bit_order = $('#spi_order').val();
            OSC.buses[bus].word_size = $('#spi_length').val();
            OSC.buses[bus].cpol = $('#spi_cpol').val();
            OSC.buses[bus].cpha = $('#spi_cpha').val();
            OSC.buses[bus].cs_polarity = $('#spi_state').val();
            OSC.buses[bus].acq_speed = 0;

            if (miso != -1) {
                if (OSC.buses[bus].miso_decoder == undefined || OSC.buses[bus].miso_decoder == "") {
                    OSC.buses[bus].miso_decoder = 'spi' + OSC.state.decoder_id;
                    OSC.params.local['CREATE_DECODER'] = {
                        value: 'spi'
                    };
                    OSC.params.local['DECODER_NAME'] = {
                        value: 'spi' + OSC.state.decoder_id
                    };
                    OSC.state.decoder_id++;
                } else {
                    var p = OSC.buses[bus];
                    p['data'] = OSC.buses[bus].miso;
                    OSC.params.local[OSC.buses[bus].miso_decoder + "_parameters"] = {
                        value: p
                    };
                }
                OSC.sendParams();
            } else {
                if (OSC.buses[bus].miso_decoder != undefined && OSC.buses[bus].miso_decoder != "") {
                    var p = {};
                    p['DESTROY_DECODER'] = {
                        value: OSC.buses[bus].miso_decoder
                    };
                    OSC.ws.send(JSON.stringify({
                        parameters: p
                    }));
                    OSC.buses[bus].miso_decoder = "";
                }
            }
            if (mosi != -1) {
                if (OSC.buses[bus].mosi_decoder == undefined || OSC.buses[bus].mosi_decoder == "") {
                    OSC.buses[bus].mosi_decoder = 'spi' + OSC.state.decoder_id;
                    OSC.params.local['CREATE_DECODER'] = {
                        value: 'spi'
                    };
                    OSC.params.local['DECODER_NAME'] = {
                        value: 'spi' + OSC.state.decoder_id
                    };
                    OSC.state.decoder_id++;
                } else {
                    var p = OSC.buses[bus];
                    p['data'] = OSC.buses[bus].mosi;
                    OSC.params.local[OSC.buses[bus].mosi_decoder + "_parameters"] = {
                        value: p
                    };
                }
                OSC.sendParams();
            } else {
                if (OSC.buses[bus].mosi_decoder != undefined && OSC.buses[bus].mosi_decoder != "") {
                    var p = {};
                    p['DESTROY_DECODER'] = {
                        value: OSC.buses[bus].miso_decoder
                    };
                    OSC.ws.send(JSON.stringify({
                        parameters: p
                    }));
                    OSC.buses[bus].mosi_decoder = "";
                }
            }

        } else if (protocol == "I2C") {
            var scl = $('#i2c_scl').val();
            var sda = $('#i2c_sda').val();
            var invert_bit = $('#i2c_invert').val();

            if (isChannelInUse(scl, OSC.state.bus_editing) != -1) {
                $('#warn-message').text($('#i2c_scl option:selected').text() + " is already in use");
                $('#warning-dialog').show();
                return;
            }
            if (isChannelInUse(sda, OSC.state.bus_editing) != -1) {
                $('#warn-message').text($('#i2c_sda option:selected').text() + " is already in use");
                $('#warning-dialog').show();
                return;
            }
            if (scl == -1 || sda == -1) {
                $('#warn-message').text("You have to specify both lines for decoding");
                $('#warning-dialog').show();
                return;
            }
            if (scl == sda) {
                $('#warn-message').text("You can't specify same lines");
                $('#warning-dialog').show();
                return;
            }
            OSC.destroyDecoder(bus, "I2C");
            var upd = OSC.needUpdateDecoder(bus, "I2C");

            if (!upd)
                OSC.buses[bus] = {};
            else {
                if (OSC.buses[bus].scl !== scl)
                    OSC.hideInfoArrow(parseInt(OSC.buses[bus].scl) - 1);
                if (OSC.buses[bus].sda !== sda)
                    OSC.hideInfoArrow(parseInt(OSC.buses[bus].sda) - 1);
            }

            OSC.buses[bus].name = "I2C";
            OSC.buses[bus].enabled = true;
            OSC.buses[bus].scl = scl;
            OSC.buses[bus].sda = sda;
            OSC.buses[bus].address_format = $('#i2c_addr').val();
            OSC.buses[bus].acq_speed = 0;
            OSC.buses[bus].invert_bit = invert_bit;

            OSC.showInfoArrow(parseInt(OSC.buses[bus].scl) - 1);
            OSC.showInfoArrow(parseInt(OSC.buses[bus].sda) - 1);

            if (!upd) {
                OSC.buses[bus].decoder = 'i2c' + OSC.state.decoder_id;
                OSC.state.decoder_id++;
            }

            if (!upd) {
                OSC.params.local['CREATE_DECODER'] = {
                    value: 'i2c'
                };
                OSC.params.local['DECODER_NAME'] = {
                    value: OSC.buses[bus].decoder
                };
            } else {
                OSC.params.local[OSC.buses[bus].decoder + "_parameters"] = {
                    value: OSC.buses[bus]
                };
            }
            OSC.sendParams();

        } else if (protocol == "CAN") {
            var nom_bitrate = $('#can_nom_bitrate').val();
            var fast_bitrate = $('#can_fast_bitrate').val();
            var sample_point = $('#sample_point').val();
            var frame_limit = $('#can_frame_limit').val();
            var can_rx = $('#can_rx').val();
            var invert_bit = $('#can_invert').val();


            if (nom_bitrate <= 0) {
                $('#warn-message').text("Nominal bitrate value is incorrect");
                $('#warning-dialog').show();
                return;
            }
            if (fast_bitrate <= 0) {
                $('#warn-message').text("Fast bitrate value is incorrect");
                $('#warning-dialog').show();
                return;
            }

            if (parseInt(fast_bitrate) < parseInt(nom_bitrate)) {
                $('#warn-message').text("Fast bitrate value is incorrect. Fast bitrate must be more than nominal.");
                $('#warning-dialog').show();
                return;
            }

            if (sample_point < 0 || sample_point > 99.99 || sample_point == "") {
                $('#warn-message').text("Sample point value is incorrect. Must in (0-99.99)%");
                $('#warning-dialog').show();
                return;
            }

            if (frame_limit < 10 || frame_limit > 500 || frame_limit == "") {
                $('#warn-message').text("Meximum detected frames must in (10-500)");
                $('#warning-dialog').show();
                return;
            }

            if (isChannelInUse(can_rx, OSC.state.bus_editing) != -1) {
                $('#warn-message').text($('#can_rx option:selected').text() + " is already in use");
                $('#warning-dialog').show();
                return;
            }

            if (can_rx == -1) {
                $('#warn-message').text("You have to specify line for decoding");
                $('#warning-dialog').show();
                return;
            }

            OSC.destroyDecoder(bus, "CAN");
            var upd = OSC.needUpdateDecoder(bus, "CAN");

            if (!upd)
                OSC.buses[bus] = {};
            else
                OSC.hideInfoArrow(parseInt(OSC.buses[bus].can_rx) - 1);

            OSC.buses[bus].name = "CAN";
            OSC.buses[bus].enabled = true;
            OSC.buses[bus].can_rx = can_rx;
            OSC.buses[bus].nominal_bitrate = nom_bitrate;
            OSC.buses[bus].fast_bitrate = fast_bitrate;
            OSC.buses[bus].sample_point = sample_point;
            OSC.buses[bus].acq_speed = OSC.state.acq_speed;
            OSC.buses[bus].invert_bit = invert_bit;
            OSC.buses[bus].frame_limit = frame_limit;

            OSC.showInfoArrow(parseInt(OSC.buses[bus].can_rx) - 1);

            if (!upd) {
                OSC.buses[bus].decoder = 'can' + OSC.state.decoder_id;
                OSC.state.decoder_id++;
            }

            if (!upd) {
                OSC.params.local['CREATE_DECODER'] = {
                    value: 'can'
                };
                OSC.params.local['DECODER_NAME'] = {
                    value: OSC.buses[bus].decoder
                };
            } else {
                OSC.params.local[OSC.buses[bus].decoder + "_parameters"] = {
                    value: OSC.buses[bus]
                };
            }

            OSC.sendParams();
        }

        $("#BUS" + OSC.state.bus_editing + "_ENABLED").find('img').show();
        $("#BUS" + OSC.state.bus_editing + "_NAME").text(protocol);
        $("#DATA_BUS" + (OSC.state.bus_editing - 1)).text(protocol);

        $('#decoder_dialog').modal('hide');
        OSC.state.bus_editing = 0;
    }

    var laAxesMoving = false;
    var curXPos = 0;

    $('#apply_decoder').click(applyDecoder);

    $(window).on('focus', function() {
        OSC.checkAndShowArrows();
        setTimeout(function() { OSC.checkAndShowArrows(); }, 750);
        OSC.drawGraphGrid();
    });

    $(window).on('blur', function() {
        OSC.checkAndShowArrows();
        setTimeout(function() { OSC.checkAndShowArrows(); }, 750);
    });

    $('#DISPLAY_RADIX').change(function() {
        OSC.state.radix = $(this).val();
        OSC.guiHandler();
    });

    $('#EXPORT_RADIX').change(function() {
        OSC.state.export_radix = $(this).val();
        OSC.guiHandler();
    });

    $("#graphs").mousewheel(function(event) {
        if (OSC.mouseWheelEventFired)
            return;
        // console.log(event);
        if (!laAxesMoving) {
            if (Math.abs(event.deltaY) >= 40)
                event.deltaY /= 40;
            if (Math.abs(event.deltaX) >= 40)
                event.deltaX /= 40;
            time_zoom(event.deltaY > 0 ? '+' : '-', event.pageX - parseInt($('#graphs').offset().left), true);
            OSC.flagWheelHandled = true;
            OSC.mouseWheelEventFired = true;
            // I cry when I see this code :'(
            $('#OSC_TIME_OFFSET').text(OSC.convertTime(5 * (($('#time_offset_arrow').position().left + $('#time_offset_arrow').width() / 2) / ($('#graphs').width() / 2) - 1) * 102400 / OSC.time_scale / OSC.state.acq_speed));
            setTimeout(function() { OSC.mouseWheelEventFired = false; }, 300);
            OSC.guiHandler();
        }
    });

    $("#graphs").dblclick(function(event) {
        time_zoom('+', event.pageX - parseInt($('#graphs').offset().left), true);
        OSC.flagWheelHandled = true;
        $('#OSC_TIME_OFFSET').text(OSC.convertTime(5 * (($('#time_offset_arrow').position().left + $('#time_offset_arrow').width() / 2) / ($('#graphs').width() / 2) - 1) * 102400 / OSC.time_scale / OSC.state.acq_speed));
        OSC.checkAndShowArrows();
    });

    $("#graphs").mousedown(function(event) {
        laAxesMoving = true;
        curXPos = event.pageX;
    });

    $("#graphs").mouseup(function(event) {
        laAxesMoving = false;
    });

    $("#graphs").mouseout(function(event) {
        laAxesMoving = false;
    });

    var afterScroll = function() {
        var trigPosInPoints = (OSC.samples_sum * OSC.time_scale - OSC.counts_offset) * $('#graphs').width() / 1024 - $('#time_offset_arrow').width() / 2;
        $('#time_offset_arrow').css('left', trigPosInPoints);
        $('#OSC_TIME_OFFSET').text(OSC.convertTime(5 * (($('#time_offset_arrow').position().left + $('#time_offset_arrow').width() / 2) / ($('#graphs').width() / 2) - 1) * 102400 / OSC.time_scale / OSC.state.acq_speed));
        OSC.guiHandler();
        OSC.scrollDataArea();
    }

    $("#graphs").mousemove(function(event) {
        if (OSC.state.line_moving) return;
        if (laAxesMoving) {
            if (!$.isEmptyObject(OSC.graphs)) {
                var graph_width = $('#graph_grid').width();
                var pxPerPoint = graph_width / 1024;
                var diff = event.pageX - curXPos;
                if (diff < 0 && (OSC.ch1_size <= ((OSC.counts_offset + 1024 - diff) / OSC.time_scale)))
                    diff = -1 * (OSC.ch1_size * OSC.time_scale - OSC.counts_offset - 1024);

                var signalDiff = diff / pxPerPoint;
                OSC.counts_offset -= signalDiff;

                curXPos = event.pageX;

                if (OSC.counts_offset <= 0) {
                    diff = (signalDiff + OSC.counts_offset) * pxPerPoint;
                    OSC.counts_offset = 0;
                }
                OSC.offsetForDecoded = OSC.counts_offset;
                OSC.redrawTimeout = setTimeout(afterScroll, 1);
            }
        }
    });

    var applyAcqSpeed = function() {
        OSC.state.acq_speed = (OSC.state.acq_speed / $(this).val()) ;
        OSC.state.decimate = $(this).val();

        var val = $('#pre-sample-buf-val').val();
        val /= 1000;
        val *= OSC.state.acq_speed;
        if (val > 750000) {
            var newVal = ((750000 / OSC.state.acq_speed) * 1000).toFixed(0);
            $('#pre-sample-buf-val').val(newVal);
            setTimeout(function() { $('#pre-sample-buf-val').change(); }, 100);

        }

        OSC.sendACQ();

        for (var i = 1; i < 5; i++) {
            var bus = "bus" + i;
            if (OSC.buses[bus].name !== undefined && OSC.buses[bus].name == "UART" && OSC.buses[bus].enabled) {
                OSC.buses[bus].samplerate = OSC.state.acq_speed;
                OSC.params.local[OSC.buses[bus].decoder + "_parameters"] = {
                    value: OSC.buses[bus]
                };
            }
        }

        OSC.sendParams();

    }

    $('#ACQ_SPEED').change(applyAcqSpeed);

    $(".data-bus").click(function() {
        var arr = ["DATA_BUS0", "DATA_BUS1", "DATA_BUS2", "DATA_BUS3"];
        var bus = arr.indexOf($(this).attr('id'));
        OSC.log_buses[bus] = !$(this).hasClass('active');
        OSC.guiHandler();
        if (OSC.log_buses.indexOf(true) === -1)
            $('#log-container').html('');
        OSC.scaleWasChanged = true;
    });

    // $('.y-offset-arrow').mouseenter(function() {
    //     var idStr = $(this).attr('id');
    //     for (var i = 1; i < 9; i++) {
    //         var tmp = 'ch' + i + '_offset_arrow';
    //         if (tmp == idStr) {
    //             var tooltip = 'ch' + i + '_tooltip';
    //             text = OSC.accordingChanName(i);
    //             if (text != "") {
    //                 $("#" + tooltip).text(text);
    //                 $("#" + tooltip).css('visibility', 'visible');
    //             }
    //         }
    //     }
    // });

    // $('.y-offset-arrow').mouseleave(function() {
    //     var idStr = $(this).attr('id');
    //     for (var i = 1; i < 9; i++) {
    //         var tmp = 'ch' + i + '_offset_arrow';
    //         if (tmp == idStr) {
    //             var tooltip = 'ch' + i + '_tooltip';
    //             $("#" + tooltip).text("");
    //             $("#" + tooltip).css('visibility', 'hidden');
    //         }
    //     }
    // });

    var resetAllParams = function() {
        time_zoom('1', 0, false);
        // Reset trigger position to center
        OSC.params.local['OSC_TIME_OFFSET'] = { value: 0 };
        OSC.params.local['OSC_VIEV_PART'] = { value: 0 };
        OSC.time_offset(OSC.params.local);
        OSC.params.local = {};

        // Disable cursors
        OSC.disableCursor('x1');
        OSC.disableCursor('x2');

        // Remove triggers
        $('.trig-item').remove();
        for (var i = 0; i < OSC.triggers_list_id.length; i++)
            delete_trigger(i);

        for (var i = 0; i < 8; i++) {
            // Reset channels names
            var txt = "DIN" + i;
            $('#CH' + (i + 1) + '_NAME').val(txt);

            if (OSC.enabled_channels[i])
                OSC.updateChVisibility(i);
        }

        // Disable all enabled channels
        // for (var i = 0; i < 8; i++) {
        //     $('#ch' + (i + 1) + "_offset_arrow").hide();
        //     OSC.enabled_channels[i] = false;
        //     $('#CH' + (i + 1) + "_ENABLED").find('img').hide();
        // }

        // Disable all enabled decoders
        for (var i = 0; i < 4; i++) {
            if (OSC.buses["bus" + (i + 1)].enabled !== undefined) {
                OSC.destroyDecoder("bus" + (i + 1), "-");
                OSC.buses["bus" + (i + 1)].enabled = false;

                $("#BUS" + (i + 1) + "_ENABLED").find('img').hide();
                $("#BUS" + (i + 1) + "_NAME").text("BUS" + i);
            }
        }

        var arr = ["DATA_BUS0", "DATA_BUS1", "DATA_BUS2", "DATA_BUS3"];

        for (var i = 0; i < 4; i++) {
            $('#' + arr[i]).removeClass('active');
            $('#' + arr[i]).text('BUS' + i);
        }

        // Reset SPI setting in dialog
        $('#spi_order').val("1");
        $('#spi_length').val("8");
        $('#spi_cpol').val("0");
        $('#spi_cpha').val("0");
        $('#spi_state').val("0");
        $('#spi_invert').val("0");
        $('#spi_decoder').hide();

        // Reset UART setting in dialog
        $('#uart_baudrate').val("9600");
        $('#uart_rxtx').val("RX");
        $('#uart_data_length').val("8");
        $('#uart_stop_bits').val("1");
        $('#uart_parity').val("0");
        $('#uart_invert').val("0");
        $('#uart_decoder').hide();

        // Reset CAN setting in dialog
        $('#can_rx').val("0");
        $('#can_nom_bitrate').val("500000");
        $('#can_fast_bitrate').val("1000000");
        $('#sample_point').val("87.5");
        $('#can_frame_limit').val("50");
        $('#can_invert').val("0");
        $('#can_decoder').hide();

        // Reset I2C setting in dialog
        $('#i2c_addr').val("0");
        $('#i2c_invert').val("0");
        $('#i2c_decoder').show();

        // Reset protocol selector
        $('#protocol_selector').val("#i2c_decoder");

        OSC.voltage_index = 2;
        OSC.counts_offset = 0;
        OSC.ch_names = ["DIN0", "DIN1", "DIN2", "DIN3", "DIN4", "DIN5", "DIN6", "DIN7"];
        OSC.state = {
            socket_opened: false,
            processing: false,
            editing: false,
            trig_dragging: false,
            cursor_dragging: false,
            resized: false,
            sel_sig_name: 'ch1',
            fine: false,
            graph_grid_height: null,
            graph_grid_width: null,
            calib: 0,
            bus_editing: 0,
            decoder_id: 1,
            radix: 17,
            export_radix: 16,
            acq_speed: OSC.max_freq,
            line_moving: false
        };
        OSC.enabled_channels = [true, true, true, true, true, true, true, true];
        OSC.buses = {};
        OSC.buses.bus1 = {};
        OSC.buses.bus2 = {};
        OSC.buses.bus3 = {};
        OSC.buses.bus4 = {};
        OSC.bad_connection = [false, false, false, false]; // time in s.
        OSC.log_buses = [false, false, false, false];
        OSC.voltage_offset = [4, 3, 2, 1, 0, -1, -2, -3];
        OSC.sample_rate();
        OSC.state.decimate = 1;
        OSC.state.acq_speed = OSC.max_freq;
        OSC.scaleWasChanged = true;
        $('#pre-sample-buf-val').val(1);
        setTimeout(function() { $('#pre-sample-buf-val').change(); }, 100);
        OSC.guiHandler();
    }

    $('#reset_to_default').click(function() {
        resetAllParams();
        OSC.save_params();
        OSC.load_params();
    });

    $('#select_mode').click(function() {
        var x = OSC.measureMode === 1 ? 2 : 1
        OSC.setMeasureMode(x);
        $.cookie('measure_mode', x, new Date(2220, 1, 1, 0, 0, 0, 0));
    });

    var applyPreSampleBufVal = function(val) {
        OSC.params.local['PRE_SAMPLE_BUFFER'] = {
            value: val
        };

        OSC.sendParams();
    }

    $('#pre-sample-buf-val').change(function(e) {
        var val = $('#pre-sample-buf-val').val();
        if (val == "") {
            val = 1;
            $('#pre-sample-buf-val').val("1");
        } else
            val = parseInt(val);

        if (val < 1) {
            val = 1;
            $('#pre-sample-buf-val').val(val);
        }

        val /= 1000; // Convert milliseconds to seconds
        val *= OSC.state.acq_speed; // Convert to samples number

        if (val > 750000) {
            var newVal = ((750000 / OSC.state.acq_speed) * 1000).toFixed(0);
            $('#pre-sample-buf-val').val(newVal);
            newVal /= 1000;
            newVal *= OSC.state.acq_speed;
            val = newVal;
        }

        applyPreSampleBufVal(val);
    });

    // $('#stem10_btn').click(function() {
    //     OSC.setMeasureMode(1);
    //     $.cookie('measure_mode', 1, new Date(2220, 1, 1, 0, 0, 0, 0));
    // });

    // $('#stem14_btn').click(function() {

    // });

    $('.trigger_type').change(function(e) {
        var chan_num = parseInt($(this).attr('name')[3]);
        var trig_type = parseInt($(this).val());


        var I = -1;
        var L = 0;
        var H = 1;
        var R = 2;
        var F = 3;
        var E = 4;

        // Set trigger
        OSC.triggers_list[chan_num] = trig_type;

        // If trigger type is R/F/E delete other same triggers
        if (trig_type == R || trig_type == F || trig_type == E) {
            for (var i = 0; i < OSC.triggers_list.length; i++) {
                if (i != chan_num && (OSC.triggers_list[i] == R || OSC.triggers_list[i] == F || OSC.triggers_list[i] == E)) {
                    OSC.triggers_list[i] = I;
                    $('.trigger_type[name=\'din' + i + '\']').val(I);
                }
            }
        }

        OSC.sendTrigInfo();
    });
    if ($.cookie('full_triggers_list') === undefined) {
        resetAllParams();
        OSC.save_params();
        OSC.load_params();
    }

});