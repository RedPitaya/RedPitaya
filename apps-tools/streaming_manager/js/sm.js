/*
 * Red Pitaya Bode analyzer client
 *
 * Author: Dakus <info@eskala.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */
/*
 * Red Pitaya Bode analyzer client
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


(function(SM, $, undefined) {

    // Params cache
    SM.params = {
        orig: {},
        local: {}
    };
    SM.ss_status_last = -1;
    SM.ss_rate = -1;
    SM.ss_max_rate = -1;
    SM.ss_max_rate_devider = -1;
    SM.param_callbacks = {};
    SM.parameterStack = [];
    SM.signalStack = [];
    // Parameters cache
    SM.parametersCache = {};

    // App configuration
    SM.config = {};
    SM.config.app_id = 'streaming_manager';
    SM.config.server_ip = ''; // Leave empty on production, it is used for testing only

    SM.config.start_app_url = window.location.origin + '/bazaar?start=' + SM.config.app_id;
    SM.config.stop_app_url = window.location.origin + '/bazaar?stop=' + SM.config.app_id;
    SM.config.socket_url = 'ws://' + window.location.host + '/wss';

    SM.rp_model = "";

    // App state
    SM.state = {
        socket_opened: false,
        processing: false,
        editing: false,
        trig_dragging: false,
        cursor_dragging: false,
        mouseover: false,
        resized: false,
        graph_grid_height: null,
        graph_grid_width: null,
        demo_label_visible: false,
        cursor_dragging: false
    };


    SM.startApp = function() {
        $.get(
                SM.config.start_app_url
            )
            .done(function(dresult) {
                if (dresult.status == 'OK') {
                    try {
                        SM.connectWebSocket();
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




    //Show license dialog
    var showLicenseDialog = function() {
        if (SM.state.demo_label_visible)
            $('#get_lic').modal('show');
    }


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
                SM.GetIP();
                var element = document.getElementById("loader-wrapper");
                element.parentNode.removeChild(element);
                $('#main').removeAttr("style");
                SM.state.socket_opened = true;
                SM.sendParameters();
                SM.unexpectedClose = true;

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
                        SM.parameterStack.push(receive.parameters);
                        if (SM.ss_rate == -1 && SM.params.orig["SS_ACD_MAX"] != null) {
                            $("#SS_RATE").val(SM.params.orig["SS_ACD_MAX"].value);
                            rateFocusOut();
                        }
                        parametersHandler();
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

    SM.DeleteFiles = function() {
        $.ajax({
                url: '/stream_manager_delete_files',
                type: 'GET',
                timeout: 3000

            })
            .fail(function(msg) {
                if (msg.responseText) {} else {}
            })
            .done(function(msg) {
                if (msg.responseText) {
                    $('#info_dialog_label').text("All files removed: " + msg);
                    $('#info_dialog').modal('show');
                } else {
                    $('#info_dialog_label').text("All files removed");
                    $('#info_dialog').modal('show');
                }

            })
    }


    SM.change_status = function(new_params) {
        ss_status = new_params['SS_STATUS'].value;
        if (SM.ss_status_last != ss_status) {
            if (ss_status == 2) {
                $('#svg-is-runnung').hide();
                $('#info_dialog_label').text("Out of free disk space");
                $('#info_dialog').modal('show');
                SM.parametersCache["SS_STATUS"] = { value: 0 };
                SM.sendParameters();
            }

            if (ss_status == 3) {
                $('#svg-is-runnung').hide();
                $('#info_dialog_label').text("Data recording completed");
                $('#info_dialog').modal('show');
                SM.parametersCache["SS_STATUS"] = { value: 0 };
                SM.sendParameters();
            }

            if (ss_status == 1) {
                $('#svg-is-runnung').show();
            }

            if (ss_status == 0) {
                $('#svg-is-runnung').hide();
            }
        }
        SM.ss_status_last = ss_status;

    }

    //Handlers
    var signalsHandler = function() {
        if (SM.signalStack.length > 0) {

            SM.signalStack.splice(0, 1);
        }
        if (SM.signalStack.length > 2)
            SM.signalStack.length = [];
    }

    SM.processParameters = function(new_params) {
        var old_params = $.extend(true, {}, SM.params.orig);
        var send_all_params = Object.keys(new_params).indexOf('send_all_params') != -1;
        SM.updateMaxLimits(new_params['RP_MODEL_STR']);
        for (var param_name in new_params) {
            SM.params.orig[param_name] = new_params[param_name];
            if (SM.param_callbacks[param_name] !== undefined)
                SM.param_callbacks[param_name](new_params);
        }
    };

    SM.calcRateHz = function(val) {
        if (val <= 1)
            val = 1;
        if (val > SM.ss_max_rate)
            val = SM.ss_max_rate;
        SM.ss_rate = Math.round(SM.ss_full_rate / val);
        return SM.ss_rate;
    }

    SM.calcRateDecToHz = function() {
        return Math.round(SM.ss_full_rate / SM.ss_rate);
    }

    var parametersHandler = function() {
        if (SM.parameterStack.length > 0) {
            SM.processParameters(SM.parameterStack[0]);
            SM.parameterStack.splice(0, 1);
        }
    }

    SM.GetIP = function() {

        var getFirstAddress = function(obj) {
            var address = null;

            for (var i = 0; i < obj.length; ++i) {
                address = obj[i].split(" ")[1].split("/")[0];

                // Link-local address checking.
                // Do not use it if it is not the only one.
                if (!address.startsWith("169.254.")) {
                    // Return the first address.
                    break;
                }
            }

            return address;
        }

        var parseAddress = function(text) {
            var res = text.split(";");
            var addressRegexp = /inet\s+\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\/\b/g;
            var ethIP = res[0].match(addressRegexp);
            var wlanIP = res[1].match(addressRegexp);
            var ip = null;

            if (ethIP != null) {
                ip = getFirstAddress(ethIP);
            } else if (wlanIP != null) {
                ip = getFirstAddress(wlanIP);
            }

            if (ip === null) {
                ip = "None";
            }

            return ip;
        }

        $.ajax({
            url: '/get_ip',
            type: 'GET',
        }).fail(function(msg) {
            $('#SS_IP_ADDR').val(parseAddress(msg.responseText));
            ipAddressChange();
        }).done(function(msg) {
            $('#SS_IP_ADDR').val(parseAddress(msg.responseText));
            ipAddressChange();
        });
    }

    //Set handlers timers
    //    setInterval(signalsHandler, 40);
    //    setInterval(parametersHandler, 50);

    SM.param_callbacks["SS_STATUS"] = SM.change_status;

}(window.SM = window.SM || {}, jQuery));




// Page onload event handler
$(function() {

    var reloaded = $.cookie("SM_forced_reload");
    if (reloaded == undefined || reloaded == "false") {
        $.cookie("SM_forced_reload", "true");
        window.location.reload(true);
    }



    //Run button
    $('#SM_RUN').on('click', function(ev) {
        SM.parametersCache["SS_START"] = { value: true };
        SM.sendParameters();
        SM.ss_status_last = 0;
    });

    //Stop button
    $('#SM_STOP').on('click', function(ev) {
        ev.preventDefault();
        SM.parametersCache["SS_START"] = { value: false };
        SM.sendParameters();
    });




    // Bind to the window resize event to redraw the graph; trigger that event to do the first drawing
    $(window).resize(function() {
        if (SM.ws) {
            SM.sendParameters();
        }
    }).resize();




    // Stop the application when page is unloaded
    $(window).on('beforeunload', function() {
        SM.parametersCache["SS_START"] = { value: false };
        SM.sendParameters();
        SM.ws.onclose = function() {}; // disable onclose handler first
        SM.ws.close();
        $.ajax({
            url: SM.config.stop_app_url,
            async: false
        });
    });




    //Crash buttons
    $('#send_report_btn').on('click', function() { SM.formEmail() });
    $('#restart_app_btn').on('click', function() { location.reload() });

    $('#CLEAR_FILES').click(SM.DeleteFiles);


    // Everything prepared, start application
    SM.startApp();

});