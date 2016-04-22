/*
 * Red Pitaya WIFI wizard client
 *
 * Author: Artem Kokos
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
    WIZARD.config.app_id = 'wifi_wizard';
    WIZARD.config.server_ip = ''; // Leave empty on production, it is used for testing only
    WIZARD.config.start_app_url = (WIZARD.config.server_ip.length ? 'http://' + WIZARD.config.server_ip : '') + '/bazaar?start=' + WIZARD.config.app_id + '?' + location.search.substr(1);
    WIZARD.config.stop_app_url = (WIZARD.config.server_ip.length ? 'http://' + WIZARD.config.server_ip : '') + '/bazaar?stop=' + WIZARD.config.app_id;
    // WIZARD.config.socket_url = 'ws://' + (WIZARD.config.server_ip.length ? WIZARD.config.server_ip : window.location.hostname) + ':9002'; // WebSocket server URI
    WIZARD.config.socket_url = 'ws://' + (WIZARD.config.server_ip.length ? WIZARD.config.server_ip : window.location.hostname) + '/wss'; // WebSocket server URI

    WIZARD.bad_connection = [false, false, false, false]; // time in s.

    WIZARD.compressed_data = 0;
    WIZARD.decompressed_data = 0;
    WIZARD.refresh_times = [];

    // App state
    WIZARD.state = {
        socket_opened: false,
        processing: false,
        editing: false,
        resized: false,
        demo_label_visible: false
    };

    WIZARD.wifiName = "";

    // Params cache
    WIZARD.params = {
        orig: {},
        local: {}
    };

    // Other global variables
    WIZARD.ws = null;

    WIZARD.running = true;
    WIZARD.unexpectedClose = false;

    WIZARD.state.connected = false;

    // Starts the WIFI wizard application on server
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
        for (var param_name in new_params) {
            if (param_name == 'WIFI_LIST') {
                if (new_params[param_name].value != "")
                    WIZARD.updateList(JSON.parse(new_params[param_name].value))
            } else if (param_name == 'WIFI_ERROR') {
                if (new_params[param_name].value == "wt not installed")
                    WIZARD.installWTDialog();
            } else if (param_name == 'WIFI_OK') {
                console.log(new_params[param_name].value);
            } else if (param_name == 'WIFI_CONNECTED') {
                WIZARD.connectionControl(new_params[param_name].value);
            } else if (param_name == 'WIFI_DONGLE_STATE') {
                WIZARD.dongleEnablingDialog(new_params[param_name].value);
                console.log("DONGLE: " + new_params[param_name].value);
            } else if (param_name == 'WIFI_NAME') {
                WIZARD.wifiName = new_params[param_name].value;
            }
        }
        $('.btn-wifi-item').css("color", "#cdcccc");
        if (WIZARD.wifiName != undefined && WIZARD.wifiName != "") {
            $('.btn-wifi-item[key=' + WIZARD.wifiName + ']').css('color', 'red');
        }
    };

    WIZARD.installWTDialog = function() {
        $('#wtools_missing').modal('show');
    }

    WIZARD.dongleEnablingDialog = function(state) {
        if (!state)
            $('#dongle_missing').modal('show');
    }

    WIZARD.connectionControl = function(result) {
        if (result && !WIZARD.state.connected) {
            $('#connect_btn').text("Disconnect");
            WIZARD.state.connected = true;
        } else if (!result && WIZARD.state.connected) {
            $('#connect_btn').text("Connect");
            WIZARD.state.connected = false;
        }
    }

    WIZARD.updateList = function(list) {
        var htmlList = "";

        for (var item in list) {
            var icon = "";
            var lock = (list[item].keyEn == "1") ? "<img src='img/wifi-icons/lock.png' width=15>" : "";

            if (parseInt(list[item].sigLevel) < 20)
                icon = "<div style='width: 40px; float: left;'><img src='img/wifi-icons/connection_0.png' width=25>" + lock + "</div>";
            else if (parseInt(list[item].sigLevel) < 50)
                icon = "<div style='width: 40px; float: left;'><img src='img/wifi-icons/connection_1.png' width=25>" + lock + "</div>";
            else if (parseInt(list[item].sigLevel) < 80)
                icon = "<div style='width: 40px; float: left;'><img src='img/wifi-icons/connection_2.png' width=25>" + lock + "</div>";
            else
                icon = "<div style='width: 40px; float: left;'><img src='img/wifi-icons/connection_3.png' width=25>" + lock + "</div>";

            htmlList += icon + "<div key='" + list[item].essid + "' class='btn-wifi-item btn'>" + list[item].essid + "</div>";
        }
        if ($('#wifi_list').html() != htmlList)
            $('#wifi_list').html(htmlList);

        $('.btn-wifi-item').click(function() {
            $('#essid_enter').val($(this).attr('key'))
        });
    }

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

            // Send report button
            $('#send_report_btn').on('click', function() {
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

            // Restart application button
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

        WIZARD.params.local['in_command'] = { value: 'WIFI_LIST' };
        WIZARD.ws.send(JSON.stringify({ parameters: WIZARD.params.local }));
        WIZARD.params.local = {};

        return true;
    };
}(window.WIZARD = window.WIZARD || {}, jQuery));

// Page onload event handler
$(function() {
    $('#connect_btn').click(function() {
        var ssid = $('#essid_enter').val();
        var pass = $('#passw_enter').val();
        WIZARD.params.local['WIFI_SSID'] = { value: ssid };
        WIZARD.params.local['WIFI_PASSW'] = { value: pass };
        if (!WIZARD.state.connected) {
            WIZARD.params.local['WIFI_CONNECT'] = { value: true };
            WIZARD.sendParams();
        } else {
            WIZARD.params.local['WIFI_CONNECT'] = { value: false };
            WIZARD.sendParams();
        }
        WIZARD.params.local = {};
    });

    $('#get_wtools').on('click', function() {
        if (OnlineChecker.isOnline()) {
            WIZARD.params.local['WIFI_INSTALL'] = { value: true };
            WIZARD.sendParams();
            WIZARD.params.local = {};
        } else
            $('#offline_dialog').modal('show');
    });

    var reloaded = $.cookie("wifi_forced_reload");
    if (reloaded == undefined || reloaded == "false") {
        $.cookie("wifi_forced_reload", "true");
        window.location.reload(true);
    }

    // Stop the application when page is unloaded
    window.onbeforeunload = function() {
        WIZARD.ws.onclose = function() {}; // disable onclose handler first
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
