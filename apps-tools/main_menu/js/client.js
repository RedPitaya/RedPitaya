(function(CLIENT, $, undefined) {

    // Params cache
    CLIENT.params = {
        orig: {},
        local: {}
    };

    // App configuration
    CLIENT.config = {};
    CLIENT.config.app_id = 'main_menu';
    CLIENT.config.server_ip = ''; // Leave empty on production, it is used for testing only
    CLIENT.config.search = "?type=run" //location.search
    CLIENT.config.start_app_url = (CLIENT.config.server_ip.length ? 'http://' + CLIENT.config.server_ip : '') + '/bazaar?start=' + CLIENT.config.app_id + '?' + CLIENT.config.search.substr(1);
    CLIENT.config.stop_app_url = (CLIENT.config.server_ip.length ? 'http://' + CLIENT.config.server_ip : '') + '/bazaar?stop=' + CLIENT.config.app_id;
    CLIENT.config.socket_url = 'ws://' + (CLIENT.config.server_ip.length ? CLIENT.config.server_ip : window.location.hostname) + '/wss'; // WebSocket server URI
    CLIENT.client_id = undefined;
    // App state
    CLIENT.state = {
        socket_opened: false,
        processing: false,
    };

    // Parameters cache
    CLIENT.parametersCache = {};

    // Other global variables
    CLIENT.ws = null;
    CLIENT.unexpectedClose = false;

    CLIENT.parameterStack = [];
    CLIENT.signalStack = [];

    CLIENT.compressed_data = 0;
    CLIENT.decompressed_data = 0;

    CLIENT.startApp = function() {
        $.get(
            CLIENT.config.start_app_url
            )
            .done(function(dresult) {
                if (dresult.status == 'OK') {
                    try {
                        CLIENT.connectWebSocket();
                    } catch (e) {
                        CLIENT.startApp();
                    }
                } else if (dresult.status == 'ERROR') {
                    console.log(dresult.reason ? dresult.reason : 'Could not start the application (ERR1)');
                    CLIENT.startApp();
                } else {
                    console.log('Could not start the application (ERR2)');
                    CLIENT.startApp();
                }
            })
            .fail(function() {
                console.log('Could not start the application (ERR3)');
                CLIENT.startApp();
            });
    };

    CLIENT.checkStatusTimer = undefined;
    CLIENT.changeStatusForRestart = false;
    CLIENT.changeStatusStep = 0;

    CLIENT.reloadPage = function() {
        $.ajax({
            method: "GET",
            url: "/get_client_id",
            timeout: 2000
        }).done(function(msg) {
            if (msg.trim() === CLIENT.client_id) {
                location.reload();
            } else {
                $('body').removeClass('user_lost');
                CLIENT.stopCheckStatus();
            }
        }).fail(function(msg) {
            console.log(msg);
            $('body').removeClass('connection_lost');
        });
    }

    CLIENT.startCheckStatus = function() {
        if (CLIENT.checkStatusTimer === undefined) {
            CLIENT.changeStatusStep = 0;
            CLIENT.checkStatusTimer = setInterval(CLIENT.checkStatus, 4000);
        }
    }

    CLIENT.stopCheckStatus = function() {
        if (CLIENT.checkStatusTimer !== undefined) {
            clearInterval(CLIENT.checkStatusTimer);
            CLIENT.checkStatusTimer = undefined;
        }
    }

    CLIENT.checkStatus = function() {
        $.ajax({
            method: "GET",
            url: "/check_status",
            timeout: 2000
        }).done(function(msg) {
            switch (CLIENT.changeStatusStep) {
                case 0:
                    CLIENT.changeStatusStep = 1;
                    break;
                case 2:
                    CLIENT.reloadPage();
                    break;
            }
        }).fail(function(msg) {
            // check status. If don't have good state after start. We lock system.
            $('body').removeClass('connection_lost');
            switch (CLIENT.changeStatusStep) {
                case 0:
                    CLIENT.changeStatusStep = -1;
                    break;
                case 1:
                    CLIENT.changeStatusStep = 2;
                    break;
            }
        });
    }


    // Creates a WebSocket connection with the web server
    CLIENT.connectWebSocket = function() {

        if (window.WebSocket) {
            CLIENT.ws = new WebSocket(CLIENT.config.socket_url);
            CLIENT.ws.binaryType = "arraybuffer";
        } else if (window.MozWebSocket) {
            CLIENT.ws = new MozWebSocket(CLIENT.config.socket_url);
            CLIENT.ws.binaryType = "arraybuffer";
        } else {
            console.log('Browser does not support WebSocket');
        }

        // Define WebSocket event listeners
        if (CLIENT.ws) {
            CLIENT.ws.onopen = function() {
                console.log('Socket opened');

                CLIENT.state.socket_opened = true;
                CLIENT.parametersCache['in_command'] = {
                    value: 'send_all_params'
                };
                CLIENT.sendParameters();

                //setTimeout(showLicenseDialog, 2500);
                CLIENT.unexpectedClose = true;
                $('body').addClass('loaded');
                $('body').addClass('connection_lost');
                $('body').addClass('user_lost');
                CLIENT.startCheckStatus();
            };

            CLIENT.ws.onclose = function() {
                CLIENT.state.socket_opened = false;
                console.log('Socket closed');
                if (CLIENT.unexpectedClose == true) {
                    setTimeout(CLIENT.reloadPage, '1000');
                }
            };

            CLIENT.ws.onerror = function(ev) {
                if (!CLIENT.state.socket_opened)
                    CLIENT.startApp();
                console.log('Websocket error: ', ev);
            };

            CLIENT.ws.onmessage = function(ev) {
                if (CLIENT.state.processing) {
                    return;
                }
                CLIENT.state.processing = true;

                try {
                    var data = new Uint8Array(ev.data);
                    CLIENT.compressed_data += data.length;
                    var inflate = pako.inflate(data);
                    // var text = String.fromCharCode.apply(null, new Uint8Array(inflate));
                    var bytes = new Uint8Array(inflate);
                    var text = '';
                    for(var i = 0; i < Math.ceil(bytes.length / 32768.0); i++) {
                      text += String.fromCharCode.apply(null, bytes.slice(i * 32768, Math.min((i+1) * 32768, bytes.length)))
                    }

                    CLIENT.decompressed_data += text.length;
                    var receive = JSON.parse(text);

                    //Recieving parameters
                    if (receive.parameters) {
                        CLIENT.parameterStack.push(receive.parameters);
                    }

                    //Recieve signals
                    if (receive.signals) {
                        CLIENT.signalStack.push(receive.signals);
                    }

                    CLIENT.state.processing = false;
                } catch (e) {
                    CLIENT.state.processing = false;
                    console.log(e);
                } finally {
                    CLIENT.state.processing = false;
                }
            };
        }
    };



    // Sends to server parameters
    CLIENT.sendParameters = function() {
        if (!CLIENT.state.socket_opened) {
            console.log('ERROR: Cannot save changes, socket not opened');
            return false;
        }
        CLIENT.ws.send(JSON.stringify({ parameters: CLIENT.parametersCache }));
        console.log("SEND: ", CLIENT.parametersCache )
        CLIENT.parametersCache = {};
        return true;
    };


    //Handlers
    var signalsHandler = function() {
        if (CLIENT.signalStack.length > 0) {
            MAIN.processSignals(CLIENT.signalStack[0])
            CLIENT.signalStack.splice(0, 1);
        }
        if (CLIENT.signalStack.length > 2)
        CLIENT.signalStack.length = [];
    }

    CLIENT.processParameters = function(new_params) {
        if (Object.keys(new_params).length > 0) {
            console.log(new_params)
        }

        for (var param_name in new_params) {
            if (MAIN.param_callbacks[param_name] !== undefined)
                MAIN.param_callbacks[param_name](new_params);
            CLIENT.params.orig[param_name] = new_params[param_name];
        }
        // Resize double-headed arrows showing the difference between cursors
    };

    var parametersHandler = function() {
        if (CLIENT.parameterStack.length > 0) {
            CLIENT.processParameters(CLIENT.parameterStack[0]);
            CLIENT.parameterStack.splice(0, 1);
        }
    }


    //Set handlers timers
    setInterval(signalsHandler, 10);
    setInterval(parametersHandler, 10);

}(window.CLIENT = window.CLIENT || {}, jQuery));


// Page onload event handler
$(function() {

    CLIENT.client_id = 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
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
        data: CLIENT.client_id,
        //Options to tell jQuery not to process data or worry about content-type.
        cache: false,
        contentType: false,
        processData: false
    });

    CLIENT.checkStatus();
})