(function(RP_WS_CLIENT, $, undefined) {

    // App configuration
    RP_WS_CLIENT.config = {};
    RP_WS_CLIENT.config.server_ip = ''; // Leave empty on production, it is used for testing only
    RP_WS_CLIENT.config.socket_url = 'ws://' + (RP_WS_CLIENT.config.server_ip.length ? RP_WS_CLIENT.config.server_ip : window.location.hostname) + ':9099';
    RP_WS_CLIENT.config.debug = true

    RP_WS_CLIENT.ws = null;


    RP_WS_CLIENT.client_log = function(...args) {
        if (RP_WS_CLIENT.config.debug){
            const d = new Date();
            console.log("LOG:RP_WS_CLIENT.js",d.getHours() + ":" + d.getMinutes() + ":"+ d.getSeconds() + ":" + d.getMilliseconds() ,...args);
        }
    }

    // Creates a WebSocket connection with the web server
    RP_WS_CLIENT.connectWebSocket = function(onOpen,onClose,onError,onMessage) {
        if (window.WebSocket) {
            RP_WS_CLIENT.ws = new WebSocket(RP_WS_CLIENT.config.socket_url);
            RP_WS_CLIENT.ws.binaryType = "arraybuffer";
        } else if (window.MozWebSocket) {
            RP_WS_CLIENT.ws = new MozWebSocket(RP_WS_CLIENT.config.socket_url);
            RP_WS_CLIENT.ws.binaryType = "arraybuffer";
        } else {
            console.log('Browser does not support WebSocket');
        }

        // Define WebSocket event listeners
        if (RP_WS_CLIENT.ws) {
            RP_WS_CLIENT.ws.onopen = function() {
                RP_WS_CLIENT.client_log('Socket opened');
                if (onOpen != undefined)
                    onOpen()
                // RP_WS_CLIENT.ws.send(JSON.stringify({ "request": {"type":"int", value:1} }));

            };

            RP_WS_CLIENT.ws.onclose = function() {
                RP_WS_CLIENT.client_log('Socket closed');
                if (onClose != undefined)
                    onClose()
            };

            RP_WS_CLIENT.ws.onerror = function(ev) {
                RP_WS_CLIENT.client_log('Websocket error: ', ev);
                if (onError != undefined)
                    onError()
            };

            RP_WS_CLIENT.ws.onmessage = function(ev) {
                try {
                    var bytes = new Uint8Array(ev.data);
                    var text = '';
                    for(var i = 0; i < Math.ceil(bytes.length / 32768.0); i++) {
                      text += String.fromCharCode.apply(null, bytes.slice(i * 32768, Math.min((i+1) * 32768, bytes.length)))
                    }
                    var receive = JSON.parse(text);
                    RP_WS_CLIENT.client_log(receive)
                    if (onMessage != undefined)
                        onMessage(receive)
                } catch (e) {
                    console.log(e);
                }
            };
        }
    };

    RP_WS_CLIENT.closeWS = function() {
        if (RP_WS_CLIENT.ws) {
            RP_WS_CLIENT.ws.close()
        }
    };

}(window.RP_WS_CLIENT = window.RP_WS_CLIENT || {}, jQuery));


// Page onload event handler
$(function() {
    $(window).on('beforeunload', function() {
        RP_WS_CLIENT.closeWS()
    });
})
