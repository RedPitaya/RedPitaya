(function(RP_CLIENT, $, undefined) {

    // App configuration
    RP_CLIENT.config = {};
    RP_CLIENT.config.server_ip = ''; // Leave empty on production, it is used for testing only
    RP_CLIENT.config.socket_url = 'ws://' + (RP_CLIENT.config.server_ip.length ? RP_CLIENT.config.server_ip : window.location.hostname) + ':9091';
    RP_CLIENT.config.debug = false

    RP_CLIENT.client_id = undefined;
    RP_CLIENT.ping = undefined;
    RP_CLIENT.ping_from_server = undefined;
    RP_CLIENT.nginx_live = false;
    RP_CLIENT.nginx_live_timer = undefined;
    RP_CLIENT.socket_opened = false

    // Other global variables
    RP_CLIENT.ws = null;

    RP_CLIENT.parameterStack = [];

    RP_CLIENT.checkStatusTimer = undefined;
    RP_CLIENT.changeStatusForRestart = false;
    RP_CLIENT.changeStatusStep = 0;

    RP_CLIENT.client_log = function(...args) {
        if (RP_CLIENT.config.debug){
            const d = new Date();
            console.log("LOG:RP_CLIENT.js",d.getHours() + ":" + d.getMinutes() + ":"+ d.getSeconds() + ":" + d.getMilliseconds() ,...args);
        }
    }

    RP_CLIENT.setNormalMode = function() {
        RP_CLIENT.client_log("Set normal mode")
        $('body').addClass('loaded');
        $('body').addClass('connection_lost');
        $('body').addClass('user_lost');
        $('#check-wrapper').hide()
    }


    RP_CLIENT.setUserLost = function() {
        RP_CLIENT.client_log("Set user lost")
        $('body').addClass('loaded');
        $('body').addClass('connection_lost');
        $('body').removeClass('user_lost');
        $('#check-wrapper').hide()
    }

    RP_CLIENT.setConnectionLost = function() {
        RP_CLIENT.client_log("Set connection lost")
        $('body').addClass('loaded');
        $('body').addClass('user_lost');
        $('body').removeClass('connection_lost');
        $('#check-wrapper').hide()
    }

    RP_CLIENT.setLoading = function() {
        RP_CLIENT.client_log("Set loading")
        $('body').addClass('connection_lost');
        $('body').addClass('user_lost');
        $('body').removeClass('loaded');
        $('#check-wrapper').hide()
    }

    RP_CLIENT.reloadPage = function() {
        $.ajax({
            method: "GET",
            url: "/get_client_id",
            timeout: 5000
        }).done(function(msg) {
            if (msg.trim() === RP_CLIENT.client_id) {
                RP_CLIENT.client_log("Need reload")
                location.reload();
            } else {
                RP_CLIENT.setUserLost()
                RP_CLIENT.stopCheckStatus()
            }
        }).fail(function(msg) {
            RP_CLIENT.client_log(msg);
            RP_CLIENT.setConnectionLost()
        });
    }

    RP_CLIENT.startCheckStatus = function() {
        if (RP_CLIENT.checkStatusTimer === undefined) {
            RP_CLIENT.changeStatusStep = 0;
            RP_CLIENT.client_log("Set status step 0 (Need check status)")
            RP_CLIENT.checkStatusTimer = setInterval(RP_CLIENT.checkStatus, 5000);
        }
    }

    RP_CLIENT.stopCheckStatus = function() {
        if (RP_CLIENT.checkStatusTimer !== undefined) {
            clearInterval(RP_CLIENT.checkStatusTimer);
            RP_CLIENT.checkStatusTimer = undefined;
        }
    }

    RP_CLIENT.startCheckNginxStatus = function() {
        if (RP_CLIENT.nginx_live_timer === undefined) {
            RP_CLIENT.nginx_live_timer = setInterval(function(){
                $.ajax({
                    method: "GET",
                    url: "/check_nginx_live",
                    timeout: 2000
                }).done(function(msg) {
                    RP_CLIENT.nginx_live = true
                }).fail(function(msg) {
                    RP_CLIENT.nginx_live = false
                });
            }, 2000);
        }
    }

    RP_CLIENT.stopCheckNginxStatus = function() {
        if (RP_CLIENT.nginx_live_timer !== undefined) {
            clearInterval(RP_CLIENT.nginx_live_timer);
            RP_CLIENT.nginx_live_timer = undefined;
        }
    }

    RP_CLIENT.checkStatus = function() {
        if (RP_CLIENT.ping_from_server!= undefined){
            if (RP_CLIENT.ping != RP_CLIENT.ping_from_server){
                RP_CLIENT.stopCheckNginxStatus()
                switch (RP_CLIENT.changeStatusStep) {
                    case 0:
                        RP_CLIENT.changeStatusStep = 1;
                        RP_CLIENT.client_log("Set status step 1 (Client connected)")
                        break;
                }
            }else{
                RP_CLIENT.startCheckNginxStatus()
                RP_CLIENT.setConnectionLost()
                switch (RP_CLIENT.changeStatusStep) {
                    case 0: // Do nothing, since after the timer started the data did not arrive.
                        RP_CLIENT.changeStatusStep = -1;
                        RP_CLIENT.client_log("Set status step -1 (Do nothing, since after the timer started the data did not arrive.)")
                        break;
                    case 1: // Go to the connection restoration check state.
                        RP_CLIENT.changeStatusStep = 2;
                        RP_CLIENT.client_log("Set status step 2 (Go to the connection restoration check state.)")
                        break;
                }
            }
            RP_CLIENT.ping = RP_CLIENT.ping_from_server
        }
        if (RP_CLIENT.changeStatusStep == 2 && RP_CLIENT.nginx_live){
            RP_CLIENT.reloadPage();
        }
        RP_CLIENT.client_log("checkStatus")
    }


    // Creates a WebSocket connection with the web server
    RP_CLIENT.connectWebSocket = function() {

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
                RP_CLIENT.setNormalMode()
                RP_CLIENT.startCheckStatus();
            };

            RP_CLIENT.ws.onclose = function() {
                RP_CLIENT.socket_opened = false;
                RP_CLIENT.client_log('Socket closed');
                setTimeout(RP_CLIENT.reloadPage, 2000);
            };

            RP_CLIENT.ws.onerror = function(ev) {
                if (!RP_CLIENT.socket_opened)
                    setTimeout(RP_CLIENT.connectWebSocket, 2000);
                RP_CLIENT.client_log('Websocket error: ', ev);
            };

            RP_CLIENT.ws.onmessage = function(ev) {
                try {
                    var bytes = new Uint8Array(ev.data);
                    var text = '';
                    for(var i = 0; i < Math.ceil(bytes.length / 32768.0); i++) {
                      text += String.fromCharCode.apply(null, bytes.slice(i * 32768, Math.min((i+1) * 32768, bytes.length)))
                    }
                    var receive = JSON.parse(text);
                    RP_CLIENT.processParameters(receive);

                } catch (e) {
                    console.log(e);
                }
            };
        }
    };

    RP_CLIENT.processParameters = function(new_params) {
        if (Object.keys(new_params).length > 0) {
            RP_CLIENT.client_log(new_params)
        }
        if (new_params.ping){
            RP_CLIENT.ping_from_server = new_params.ping.value;
        }
    };
}(window.RP_CLIENT = window.RP_CLIENT || {}, jQuery));


// Page onload event handler
$(function() {
    RP_CLIENT.client_id = 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
        var r = Math.random() * 16 | 0,
            v = c == 'x' ? r : (r & 0x3 | 0x8);
        return v.toString(16);
    });

    $.ajax({
            url: '/set_client_id', //Server script to process data
            timeout: 5000,
            type: 'POST',
            success: function(e) { console.log(e); },
            error: function(e) { console.log(e); },
            data: RP_CLIENT.client_id,
            cache: false,
            contentType: false,
            processData: false
    });

    $(window).on('beforeunload', function() {
        RP_CLIENT.ws.onclose = function() {}; // disable onclose handler first
        RP_CLIENT.ws.close();
    });
})
