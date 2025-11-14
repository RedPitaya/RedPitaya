(function(CLIENT, $, undefined) {

    // Params cache
    CLIENT.params = {
        orig: {},
        local: {}
    };

    // App configuration
    CLIENT.config = {};
    CLIENT.config.app_id = 'ba_pro';
    CLIENT.config.server_ip = ''; // Leave empty on production, it is used for testing only
    CLIENT.config.search = "?type=run" //location.search
    CLIENT.config.start_app_url = (CLIENT.config.server_ip.length ? 'http://' + CLIENT.config.server_ip : '') + '/bazaar?start=' + CLIENT.config.app_id + '?' + CLIENT.config.search.substr(1);
    CLIENT.config.stop_app_url = (CLIENT.config.server_ip.length ? 'http://' + CLIENT.config.server_ip : '') + '/bazaar?stop=' + CLIENT.config.app_id;
    CLIENT.config.socket_url = 'ws://' + (CLIENT.config.server_ip.length ? CLIENT.config.server_ip : window.location.hostname) + '/wss'; // WebSocket server URI
    CLIENT.config.debug = false

    // App state
    CLIENT.state = {
        socket_opened: false
    };

    // Parameters cache
    CLIENT.parametersCache = {};

    // Other global variables
    CLIENT.ws = null;
    CLIENT.unexpectedClose = false;

    CLIENT.parameterStack = [];
    CLIENT.signalStack = [];

    CLIENT.client_log = function(...args) {
        if (CLIENT.config.debug){
            const d = new Date();
            console.log("LOG:CLIENT.js",d.getHours() + ":" + d.getMinutes() + ":"+ d.getSeconds() + ":" + d.getMilliseconds() ,...args);
        }
    }

    CLIENT.startApp = function() {
        $.ajax({
            url: CLIENT.config.start_app_url,
            type: 'GET',
            timeout: 5000
        }).done(function(res) {
            if (res.status == 'OK') {
                try {
                    CLIENT.connectWebSocket();
                    RP_CLIENT.connectWebSocket();
                } catch (e) {
                    setTimeout(CLIENT.startApp, 2000);
                }
            } else if (res.status == 'ERROR') {
                console.log(res.reason ? res.reason : 'Could not start the application (ERR1)');
                setTimeout(CLIENT.startApp, 2000);
            } else {
                console.log('Could not start the application (ERR2)');
                setTimeout(CLIENT.startApp, 2000);
            }
        }).fail(function(msg) {
            console.log('Could not start the application (ERR3)');
            setTimeout(CLIENT.startApp, 2000);
        });
    };

    // Creates a WebSocket connection with the web server
    CLIENT.connectWebSocket = function() {
        let binParser = new BinarySignalParser();
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
                CLIENT.client_log('Socket opened');

                CLIENT.state.socket_opened = true;
                CLIENT.parametersCache['in_command'] = {
                    value: 'send_all_params'
                };
                CLIENT.sendParameters();
                CLIENT.unexpectedClose = true;
            };

            CLIENT.ws.onclose = function() {
                CLIENT.state.socket_opened = false;
                CLIENT.client_log('Socket closed');
                setTimeout(RP_CLIENT.reloadPage, 2000);
            };

            CLIENT.ws.onerror = function(ev) {
                CLIENT.client_log('Websocket error: ', ev);
                setTimeout(RP_CLIENT.reloadPage, 2000);
            };

            CLIENT.ws.onmessage = function(ev) {
                try {

                    var receive = binParser.convert(ev.data);

                    //Recieving parameters
                    if (receive.parameters) {
                        CLIENT.parameterStack.push(receive.parameters);
                    }

                    //Recieve signals
                    if (receive.signals) {
                        CLIENT.signalStack.push(receive.signals);
                    }
                } catch (e) {
                    console.log(e);
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
        CLIENT.client_log("SEND: ", CLIENT.parametersCache )
        CLIENT.parametersCache = {};
        return true;
    };


    //Handlers
    var signalsHandler = function() {
        if (CLIENT.signalStack.length > 0) {
            BA.drawSignals();
            CLIENT.signalStack.splice(0, 1);
        }
        if (CLIENT.signalStack.length > 2)
        CLIENT.signalStack.length = [];
    }

    CLIENT.processParameters = function(new_params) {

        if (Object.keys(new_params).length > 0) {
            CLIENT.client_log(new_params)
        }

        for (var param_name in new_params) {
            CLIENT.params.orig[param_name] = new_params[param_name];
            if (BA.param_callbacks[param_name] !== undefined)
                BA.param_callbacks[param_name](new_params, param_name);
        }
        // Resize double-headed arrows showing the difference between cursors
    };

    CLIENT.getValue = function(name){
        return CLIENT.params.orig[name] ? CLIENT.params.orig[name].value : undefined
    }

    var parametersHandler = function() {
        if (CLIENT.parameterStack.length > 0) {
            var params = [...CLIENT.parameterStack]
            var pack_params = []
            for( var i = 0 ; i < params.length; i++){
                for (var param_name in params[i]) {
                    pack_params[param_name] = params[i][param_name]
                }
            }
            CLIENT.processParameters(pack_params);
            CLIENT.parameterStack = []
        }
    }


    //Set handlers timers
    setInterval(signalsHandler, 30);
    setInterval(parametersHandler, 30);

}(window.CLIENT = window.CLIENT || {}, jQuery));

$(function() {
    // Stop the application when page is unloaded
    $(window).on('beforeunload', function() {
        CLIENT.ws.onclose = function() {}; // disable onclose handler first
        CLIENT.ws.close();
        $.ajax({
            url: CLIENT.config.stop_app_url,
            async: false
        });
    });

    setTimeout(CLIENT.startApp,2000)
})
