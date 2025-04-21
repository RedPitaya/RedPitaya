(function(RP_CLIENT, $, undefined) {

    // App configuration
    RP_CLIENT.config = {};
    RP_CLIENT.config.server_ip = ''; // Leave empty on production, it is used for testing only
    RP_CLIENT.config.socket_url = 'ws://' + (RP_CLIENT.config.server_ip.length ? RP_CLIENT.config.server_ip : window.location.hostname) + ':9092';
    RP_CLIENT.config.debug = true

    RP_CLIENT.ws = null;


    RP_CLIENT.client_log = function(...args) {
        if (RP_CLIENT.config.debug){
            const d = new Date();
            console.log("LOG:RP_CLIENT.js",d.getHours() + ":" + d.getMinutes() + ":"+ d.getSeconds() + ":" + d.getMilliseconds() ,...args);
        }
    }

    // Creates a WebSocket connection with the web server
    RP_CLIENT.connectWebSocket = function(onOpen,onClose,onError,onMessage) {
        if (window.WebSocket) {
            RP_CLIENT.ws = new WebSocket(RP_CLIENT.config.socket_url);
            RP_CLIENT.ws.binaryType = "arraybuffer";
        } else if (window.MozWebSocket) {
            RP_CLIENT.ws = new MozWebSocket(RP_CLIENT.config.socket_url);
            RP_CLIENT.ws.binaryType = "arraybuffer";
        } else {
            console.log('Browser does not support WebSocket');
        }

        // Define WebSocket event listeners
        if (RP_CLIENT.ws) {
            RP_CLIENT.ws.onopen = function() {
                RP_CLIENT.client_log('Socket opened');
                if (onOpen != undefined)
                    onOpen()
                // RP_CLIENT.ws.send(JSON.stringify({ "request": {"type":"int", value:1} }));

            };

            RP_CLIENT.ws.onclose = function() {
                RP_CLIENT.client_log('Socket closed');
                if (onClose != undefined)
                    onClose()
            };

            RP_CLIENT.ws.onerror = function(ev) {
                RP_CLIENT.client_log('Websocket error: ', ev);
                if (onError != undefined)
                    onError()
            };

            RP_CLIENT.ws.onmessage = function(ev) {
                try {
                    var bytes = new Uint8Array(ev.data);
                    var text = '';
                    for(var i = 0; i < Math.ceil(bytes.length / 32768.0); i++) {
                      text += String.fromCharCode.apply(null, bytes.slice(i * 32768, Math.min((i+1) * 32768, bytes.length)))
                    }
                    var receive = JSON.parse(text);
                    RP_CLIENT.client_log(receive)
                    if (onMessage != undefined)
                        onMessage(receive)
                    // if (receive.total_files){
                    //     RP_CLIENT.total = receive.total_files.value;
                    //     $('#percent_copy').text("0%");
                    //     $('#percent_copy').show();
                    // }

                    // if (receive.copy_index){
                    //     $('#percent_copy').text("0%");
                    //     var percent = 0
                    //     if (RP_CLIENT.total) percent = ((receive.copy_index.value / RP_CLIENT.total) * 100).toFixed(2);
                    //     $('#percent_copy').text(percent + "%");

                    // }

                    // if (receive.reboot){
                    //     $('#percent_copy').hide()
                    //     UPD.showReboot();
                    // }


                } catch (e) {
                    console.log(e);
                }
            };
        }
    };

    RP_CLIENT.closeWS = function() {
        if (RP_CLIENT.ws) {
            RP_CLIENT.ws.close()
        }
    };

}(window.RP_CLIENT = window.RP_CLIENT || {}, jQuery));


// Page onload event handler
$(function() {
    $(window).on('beforeunload', function() {
        RP_CLIENT.closeWS()
    });
})
