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


(function(SM, $, undefined) {

    // Params cache
    SM.params = {
        orig: {},
        local: {}
    };

    SM.param_callbacks = {};
    SM.parameterStack = [];
    SM.signalStack = [];
    // Parameters cache
    SM.parametersCache = {};

    // App configuration
    SM.config = {};
    SM.config.app_id = 'calib_app';
    SM.config.server_ip = ''; // Leave empty on production, it is used for testing only
    SM.config.start_app_url = (SM.config.server_ip.length ? 'http://' + SM.config.server_ip : '') + '/bazaar?start=' + SM.config.app_id;
    SM.config.stop_app_url = (SM.config.server_ip.length ? 'http://' + SM.config.server_ip : '') + '/bazaar?stop=' + SM.config.app_id;
    SM.config.socket_url = 'ws://' + (SM.config.server_ip.length ? SM.config.server_ip : window.location.hostname) + '/wss'; // WebSocket server URI
    SM.rp_model = "";


    // App state
    SM.state = {
        socket_opened: false,
        processing: false,
        cursor_dragging: false,
        mouseover: false
    };


    SM.startApp = function() {
        $.get(
                SM.config.start_app_url
            )
            .done(function(dresult) {
                if (dresult.status == 'OK') {
                    try {
                        setTimeout(function() {
                            SM.connectWebSocket();
                        }, 1000);
                        //  var element = document.getElementById("loader-wrapper");
                        //  element.parentNode.removeChild(element);
                        // $('body').addClass('loaded');
                        // $('#main').removeAttr("style");
                        console.log("Load manager");
                    } catch (e) {
                        SM.startApp();
                    }
                } else if (dresult.status == 'ERROR') {
                    console.log(dresult.reason ? dresult.reason : 'Could not start the application (ERR1)');
                    SM.startApp();
                } else {
                    console.log('Could not start the application (ERR2)');
                    SM.startApp();
                }
            })
            .fail(function() {
                console.log('Could not start the application (ERR3)');
                SM.startApp();
            });
    };


    //Write email
    SM.formEmail = function() {
        //var file = new FileReader();
        var mail = "support@redpitaya.com";
        var subject = "Crash report Red Pitaya OS";
        var body = "%0D%0A%0D%0A------------------------------------%0D%0A" + "DEBUG INFO, DO NOT EDIT!%0D%0A" + "------------------------------------%0D%0A%0D%0A";
        body += "Parameters:" + "%0D%0A" + JSON.stringify({ parameters: SM.parametersCache }) + "%0D%0A";
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
    }


    // Creates a WebSocket connection with the web server
    SM.connectWebSocket = function() {

        if (window.WebSocket) {
            SM.ws = new WebSocket(SM.config.socket_url);
            SM.ws.binaryType = "arraybuffer";
        } else if (window.MozWebSocket) {
            SM.ws = new MozWebSocket(SM.config.socket_url);
            SM.ws.binaryType = "arraybuffer";
        } else {
            console.log('Browser does not support WebSocket');
        }

        // Define WebSocket event listeners
        if (SM.ws) {
            SM.ws.onopen = function() {
                console.log('Socket opened');

                SM.state.socket_opened = true;
                SM.sendParameters();
                SM.unexpectedClose = true;
                $('body').addClass('loaded');
                $('#main').removeAttr("style");

            };

            SM.ws.onclose = function() {
                SM.state.socket_opened = false;
                console.log('Socket closed');
                if (SM.unexpectedClose == true)
                    $('#feedback_error').modal('show');
            };

            SM.ws.onerror = function(ev) {
                if (!SM.state.socket_opened)
                    SM.startApp();
                console.log('Websocket error: ', ev);
            };

            SM.ws.onmessage = function(ev) {
                try {
                    var data = new Uint8Array(ev.data);
                    //   BA.compressed_data += data.length;
                    var inflate = pako.inflate(data);
                    var text = String.fromCharCode.apply(null, new Uint8Array(inflate));

                    // BA.decompressed_data += text.length;
                    var receive = JSON.parse(text);

                    //Recieving parameters
                    if (receive.parameters) {
                        console.log(receive.parameters);
                        SM.parameterStack.push(receive.parameters);
                        parametersHandler();
                    }

                    if (receive.signals) {
                        if (receive.signals.wave != undefined && receive.signals.wave.size > 0) {
                            SM.signalStack.push(receive.signals);
                            signalsHandler();
                        }
                    }

                } catch (e) {
                    //BA.state.processing = false;
                    console.log(e);
                } finally {
                    //BA.state.processing = false;
                }
            };
        }
    };

    // For Firefox
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


    // Sends to server parameters
    SM.sendParameters = function() {
        if (!SM.state.socket_opened) {
            console.log('ERROR: Cannot save changes, socket not opened');
            return false;
        }

        SM.parametersCache["in_command"] = { value: "send_all_params" };
        SM.ws.send(JSON.stringify({ parameters: SM.parametersCache }));
        SM.parametersCache = {};
        return true;
    };

    SM.sendParameters2 = function(_key) {
        if (!SM.state.socket_opened) {
            console.log('ERROR: Cannot save changes, socket not opened');
            return false;
        }
        SM.parametersCache["in_command"] = { value: _key };
        SM.ws.send(JSON.stringify({ parameters: SM.parametersCache }));
        // console.log(SM.parametersCache)
        SM.parametersCache = {};
        return true;
    };



    //Handlers
    var signalsHandler = function() {
        if (SM.signalStack.length > 0) {
            //console.log(SM.signalStack[0].wave);
            OBJ.filterSignal = SM.signalStack[0].wave;
            SM.signalStack.splice(0, 1);
        }
        if (SM.signalStack.length > 2)
            SM.signalStack.length = [];
    }

    SM.processParameters = function(new_params) {
        var old_params = $.extend(true, {}, SM.params.orig);
        var send_all_params = Object.keys(new_params).indexOf('send_all_params') != -1;

        if (!new_params['RP_MODEL_STR']){
            if (SM.rp_model === ""){
                SM.sendParameters();
                return;
            }
        }


        for (var param_name in new_params) {
            SM.params.orig[param_name] = new_params[param_name];
            if (SM.param_callbacks[param_name] !== undefined)
                SM.param_callbacks[param_name](new_params[param_name]);
        }
    };

    var parametersHandler = function() {
        if (SM.parameterStack.length > 0) {
            SM.processParameters(SM.parameterStack[0]);
            SM.parameterStack.splice(0, 1);
        }
    }




}(window.SM = window.SM || {}, jQuery));




// Page onload event handler
$(function() {

    var reloaded = $.cookie("SM_forced_reload");
    if (reloaded == undefined || reloaded == "false") {
        $.cookie("SM_forced_reload", "true");
        window.location.reload(true);
    }


    // Stop the application when page is unloaded
    $(window).on('beforeunload', function() {
        SM.ws.onclose = function() {}; // disable onclose handler first
        SM.ws.close();
        $.ajax({
            url: SM.config.stop_app_url,
            async: false
        });
    });

    $(window).resize(function() {
        OBJ.cursorResize();
    });


    //Crash buttons
    $('#send_report_btn').on('click', function() { SM.formEmail() });
    $('#restart_app_btn').on('click', function() { location.reload() });



    // Everything prepared, start application
    SM.startApp();

});