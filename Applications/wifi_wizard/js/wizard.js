/*
 * Red Pitaya WIZARDillWIZARDope client
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

(function(WIZARD, $, undefined) {

    // App configuration
    WIZARD.config = {};
    WIZARD.config.app_id = 'scpi_server';
    WIZARD.config.server_ip = ''; // Leave empty on production, it is used for testing only
    WIZARD.config.start_app_url = (WIZARD.config.server_ip.length ? 'http://' + WIZARD.config.server_ip : '') + '/bazaar?start=' + WIZARD.config.app_id + '?' + location.search.substr(1);
    WIZARD.config.stop_app_url = (WIZARD.config.server_ip.length ? 'http://' + WIZARD.config.server_ip : '') + '/bazaar?stop=' + WIZARD.config.app_id;
    WIZARD.config.socket_url = 'ws://' + (WIZARD.config.server_ip.length ? WIZARD.config.server_ip : window.location.hostname) + ':9002'; // WebSocket server URI
    WIZARD.config.graph_colors = {
        'ch1': '#f3ec1a',
        'ch2': '#31b44b',
        'output1': '#9595ca',
        'output2': '#ee3739',
        'math': '#ab4d9d',
        'trig': '#75cede'
    };

    // Time scale steps in millisecods
    WIZARD.time_steps = [
        // Nanoseconds
        100 / 1000000, 200 / 1000000, 500 / 1000000,
        // Microseconds
        1 / 1000, 2 / 1000, 5 / 1000, 10 / 1000, 20 / 1000, 50 / 1000, 100 / 1000, 200 / 1000, 500 / 1000,
        // Millisecods
        1, 2, 5, 10, 20, 50, 100, 200, 500,
        // Seconds
        1 * 1000, 2 * 1000, 5 * 1000, 10 * 1000, 20 * 1000, 50 * 1000
    ];

    // Voltage scale steps in volts
    WIZARD.voltage_steps = [
        // Millivolts
        1 / 1000, 2 / 1000, 5 / 1000, 10 / 1000, 20 / 1000, 50 / 1000, 100 / 1000, 200 / 1000, 500 / 1000,
        // Volts
        1, 2, 5
    ];

    WIZARD.bad_connection = [false, false, false, false]; // time in s.

    WIZARD.compressed_data = 0;
    WIZARD.decompressed_data = 0;
    WIZARD.refresh_times = [];

    WIZARD.counts_offset = 0;

    // Sampling rates
    WIZARD.sample_rates = ['125M', '15.625M', '1.953M', '122.070k', '15.258k', '1.907k'];

    // App state
    WIZARD.state = {
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
        demo_label_visible: false
    };

    // Params cache
    WIZARD.params = {
        orig: {},
        local: {}
    };

    // Other global variables
    WIZARD.ws = null;
    WIZARD.graphs = {};
    WIZARD.touch = {};

    WIZARD.connect_time;

    WIZARD.inGainValue1 = '-';
    WIZARD.inGainValue2 = '-';
    WIZARD.loaderShow = false;
    WIZARD.running = true;
    WIZARD.unexpectedClose = false;

    // Starts the WIZARDillWIZARDope application on server
    WIZARD.startApp = function() {
        $.get(
                WIZARD.config.start_app_url
            )
            .done(function(dresult) {
                if (dresult.status == 'OK') {
                    try {
                        WIZARD.connectWebSocket();
                    } catch (e) {
                        WIZARD.startApp();
                    }
                } else if (dresult.status == 'ERROR') {
                    console.log(dresult.reason ? dresult.reason : 'Could not start the application (ERR1)');
                    WIZARD.startApp();
                } else {
                    console.log('Could not start the application (ERR2)');
                    WIZARD.startApp();
                }
            })
            .fail(function() {
                console.log('Could not start the application (ERR3)');
                WIZARD.startApp();
            });
    };

    Date.prototype.format = function(mask, utc) {
        return dateFormat(this, mask, utc);
    };

    var g_count = 0;
    var g_time = 0;
    var g_iter = 10;
    var g_delay = 200;
    var g_counter = 0;
    var g_CpuLoad = 100.0;
    var g_TotalMemory = 256.0;
    var g_FreeMemory = 256.0;

    setInterval(function() {
        if (!WIZARD.state.socket_opened)
            return;
        var now = new Date();
        var now_str = now.getHours() + ":" + now.getMinutes() + ":" + now.getSeconds() + ":" + now.getMilliseconds();
        var times = "";
        for (var i = 0; i < WIZARD.refresh_times.length; i++)
            times += WIZARD.refresh_times[i] + " ";

        if (WIZARD.refresh_times.length < 3)
            WIZARD.bad_connection[g_counter] = true;
        else
            WIZARD.bad_connection[g_counter] = false;

        g_counter++;
        if (g_counter == 4) g_counter = 0;


        if ($('#weak_conn_msg').is(':visible')) {
            if (!WIZARD.bad_connection[0] && !WIZARD.bad_connection[1] && !WIZARD.bad_connection[2] && !WIZARD.bad_connection[3])
                $('#weak_conn_msg').hide();
        } else {
            if (WIZARD.bad_connection[0] && WIZARD.bad_connection[1] && WIZARD.bad_connection[2] && WIZARD.bad_connection[3])
                $('#weak_conn_msg').show();
        }

        WIZARD.compressed_data = 0;
        WIZARD.decompressed_data = 0;
        WIZARD.refresh_times = [];
    }, 1000);


    WIZARD.convertUnpacked = function(array) {
        var CHUNK_SIZE = 0x8000; // arbitrary number here, not too small, not too big
        var index = 0;
        var length = array.length;
        var result = '';
        var slice;
        while (index < length) {
            slice = array.slice(index, Math.min(index + CHUNK_SIZE, length)); // `Math.min` is not really necessary here I think
            result += String.fromCharCode.apply(null, slice);
            index += CHUNK_SIZE;
        }
        return result;
    }

    // Processes newly received values for parameters
    WIZARD.processParameters = function(new_params) {
        // Run/Stop button
        for (var param_name in new_params) {
            if (param_name == 'WIZARD_RUNNING') {
                if (new_params[param_name].value === true) {
                    // console.log("Running");
                    $('#WIZARD_RUN').hide();
                    $('#WIZARD_STOP').css('display', 'block');
                    WIZARD.running = true;
                    $('#label-is-runnung').hide();
                    $('#label-is-not-runnung').show();
                } else {
                    // console.log("Stopped");
                    $('#WIZARD_STOP').hide();
                    $('#WIZARD_RUN').show();
                    WIZARD.running = false;
                    $('#label-is-not-runnung').hide();
                    $('#label-is-runnung').show();
                }
            }
        }
    };

    // Creates a WebSocket connection with the web server
    WIZARD.connectWebSocket = function() {

        if (window.WebSocket) {
            WIZARD.ws = new WebSocket(WIZARD.config.socket_url);
            WIZARD.ws.binaryType = "arraybuffer";
        } else if (window.MozWebSocket) {
            WIZARD.ws = new MozWebSocket(WIZARD.config.socket_url);
            WIZARD.ws.binaryType = "arraybuffer";
        } else {
            console.log('Browser does not support WebSocket');
        }

        // Define WebSocket event listeners
        if (WIZARD.ws) {

            WIZARD.ws.onopen = function() {
                WIZARD.state.socket_opened = true;
                console.log('Socket opened');

                setTimeout(function() {
                    WIZARD.ws.send(JSON.stringify({ parameters: WIZARD.params.local }));
                    WIZARD.params.local = {};
                }, 2000);
                WIZARD.unexpectedClose = true;
            };

            WIZARD.ws.onclose = function() {
                WIZARD.state.socket_opened = false;
                console.log('Socket closed');
                if (WIZARD.unexpectedClose == true) {
                    $('#feedback_error').modal('show');
                }
            };

            $('#send_report_btn').on('click', function() {
                //var file = new FileReader();
                var mail = "support@redpitaya.com";
                var subject = "Crash report Red Pitaya OS";
                var body = "%0D%0A%0D%0A------------------------------------%0D%0A" + "DEBUG INFO, DO NOT EDIT!%0D%0A" + "------------------------------------%0D%0A%0D%0A";
                body += "Parameters:" + "%0D%0A" + JSON.stringify({ parameters: WIZARD.params }) + "%0D%0A";
                body += "Browser:" + "%0D%0A" + JSON.stringify({ parameters: $.browser }) + "%0D%0A";

                var url = 'info/info.json';
                $.ajax({
                    method: "GET",
                    url: url
                }).done(function(msg) {
                    body += " info.json: " + "%0D%0A" + msg.responseText;
                }).fail(function(msg) {
                    var info_json = msg.responseText
                    var ver = '';
                    try {
                        var obj = JSON.parse(msg.responseText);
                        ver = " " + obj['version'];
                    } catch (e) {};

                    body += " info.json: " + "%0D%0A" + msg.responseText;
                    document.location.href = "mailto:" + mail + "?subject=" + subject + ver + "&body=" + body;
                });
            });

            $('#restart_app_btn').on('click', function() {
                location.reload();
            });

            WIZARD.ws.onerror = function(ev) {
                if (!WIZARD.state.socket_opened)
                  WIZARD.startApp();
                console.log('Websocket error: ', ev);
            };

            var last_time = undefined;

            WIZARD.ws.onmessage = function(ev) {
                var start_time = +new Date();
                if (WIZARD.state.processing) {
                    return;
                }

                try {
                    var data = new Uint8Array(ev.data);
                    WIZARD.compressed_data += data.length;

                    var inflate = new Zlib.Gunzip(data);
                    var decompressed = inflate.decompress();
                    var arr = new Uint16Array(decompressed)
                    var text = WIZARD.convertUnpacked(arr);
                    WIZARD.decompressed_data += text.length;

                    var receive = JSON.parse(text);

                    WIZARD.processParameters(receive.parameters);
                } catch (e) {
                    WIZARD.state.processing = false;
                    console.log(e);
                } finally {
                    WIZARD.state.processing = false;
                }
            };
        }
    };

    // Sends to server modified parameters
    WIZARD.sendParams = function() {
        if ($.isEmptyObject(WIZARD.params.local)) {
            return false;
        }

        if (!WIZARD.state.socket_opened) {
            console.log('ERROR: Cannot save changes, socket not opened');
            return false;
        }

        WIZARD.params.local['in_command'] = { value: 'WIZARD_RUN' };
        WIZARD.ws.send(JSON.stringify({ parameters: WIZARD.params.local }));
        WIZARD.params.local = {};

        return true;
    };
}(window.WIZARD = window.WIZARD || {}, jQuery));

// Page onload event handler
$(function() {

    var reloaded = $.cookie("scpi_forced_reload");
    if (reloaded == undefined || reloaded == "false") {
        $.cookie("scpi_forced_reload", "true");
        window.location.reload(true);
    }
    // Process clicks on top menu buttons
    $('#WIZARD_RUN').on('click', function(ev) {
        ev.preventDefault();
        //$('#WIZARD_RUN').hide();
        //$('#WIZARD_STOP').css('display','block');
        WIZARD.params.local['WIZARD_RUN'] = { value: true };
        WIZARD.sendParams();
        //WIZARD.running = true;
    });

    $('#WIZARD_STOP').on('click', function(ev) {
        ev.preventDefault();
        //$('#WIZARD_STOP').hide();
        //$('#WIZARD_RUN').show();
        WIZARD.params.local['WIZARD_RUN'] = { value: false };
        WIZARD.sendParams();
        //WIZARD.running = false;
    });

    // Stop the application when page is unloaded
    window.onbeforeunload = function() {
        WIZARD.ws.onclose = function() {}; // disable onclose handler first
        // Stop SCPI server and close socket
        WIZARD.params.local['WIZARD_RUN'] = { value: false };
        WIZARD.sendParams();
        WIZARD.running = false;
        WIZARD.ws.close();
        $.ajax({
            url: WIZARD.config.stop_app_url,
            async: false
        });
        WIZARD.unexpectedClose = false;
    };

    // Everything prepared, start application
    WIZARD.startApp();
});
