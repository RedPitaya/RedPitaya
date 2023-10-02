/*
 * Red Pitaya LCR meter client
 *
 * Author: Luka Golinar <luka.golinar@gmail.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


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

(function(LCR, $, undefined) {

    //Configure APP
    LCR.startTime = 0;
    LCR.config = {};
    LCR.config.app_id = 'lcr_meter';
    LCR.config.server_ip = '';
    LCR.config.start_app_url = window.location.origin + '/bazaar?start=' + LCR.config.app_id;
    LCR.config.socket_url = 'ws://' + window.location.host + '/wss'; // WebSocket server URI
    LCR.client_id = undefined;
    // App state
    LCR.state = {
        socket_opened: false,
        processing: false,
        editing: false,
        trig_dragging: false,
        cursor_dragging: false,
        resized: false,
        sel_sig_name: null,
        fine: false,
        graph_grid_height: null,
        graph_grid_width: null,
        calib: 0,
        demo_label_visible: false,
        demo_modal_visible: false
    };

    // Params cache
    LCR.params = {
        orig: {},
        local: {}
    };

    // Other global variables
    LCR.ws = null;
    LCR.touch = {};

    LCR.connect_time;

    LCR.displ_params = {
        prim: 'LCR_Z',
        prim_val: 0,
        sec: 'LCR_P',
        sec_val: 0,
        p_base_u: "Ω",
        s_base_u: "deg",
        p_units: "",
        s_units: ""
    };

    LCR.secondary_meas = {
        apply_tolerance: false,
        apply_relative: false,
        ampl_tol: 0
    };

    LCR.data_log = {
        prim_display: 'LCR_Z',
        sec_display: 'LCR_P',
        save_data: false,
        //Web socket params have a very fast interval. Interval ought to be interpredted as
        // something similar to decimation in oscilloscppe.
        interval: 10,
        //How much data to be stored.
        max_store: 1000,
        curr_store: 1
    }

    LCR.selected_meas = 1;
    LCR.unexpectedClose = false;

    LCR.i2ct_checked = false;
    LCR.i2c0x20_res = "";
    LCR.module_disconnected = false;
    LCR.modal_opened = false;

    //LCR.toClearLog = false;

    LCR.startApp = function() {
        $.get(LCR.config.start_app_url)
            .done(function(dresult) {
                if (dresult.status == 'OK') {
                    try {
                        LCR.connectWebSocket();
                        console.log('Socket opened');
                    } catch (e) {
                        LCR.startApp();
                    }
                } else if (dresult.status == 'ERROR') {
                    console.log(dresult.reason ? dresult.reason : 'Could not start the application (ERR1)');
                    LCR.startApp();
                } else {
                    console.log('Could not start application (ERR2)');
                    LCR.startApp();
                }
            })
            .fail(function() {
                console.log('Could not start the application (ERR3)');
                LCR.startApp();
            });
    };


    LCR.checkStatusTimer = undefined;
    LCR.changeStatusForRestart = false;
    LCR.changeStatusStep = 0;

    LCR.reloadPage = function() {
        $.ajax({
            method: "GET",
            url: "/get_client_id",
            timeout: 2000
        }).done(function(msg) {
            if (msg.trim() === LCR.client_id) {
                location.reload();
            } else {
                $('body').removeClass('user_lost');
                LCR.stopCheckStatus();
            }
        }).fail(function(msg) {
            console.log(msg);
            $('body').removeClass('connection_lost');
        });
    }

    LCR.startCheckStatus = function() {
        if (LCR.checkStatusTimer === undefined) {
            LCR.changeStatusStep = 0;
            LCR.checkStatusTimer = setInterval(LCR.checkStatus, 4000);
        }
    }

    LCR.stopCheckStatus = function() {
        if (LCR.checkStatusTimer !== undefined) {
            clearInterval(LCR.checkStatusTimer);
            LCR.checkStatusTimer = undefined;
        }
    }

    LCR.checkStatus = function() {
        $.ajax({
            method: "GET",
            url: "/check_status",
            timeout: 2000
        }).done(function(msg) {
            switch (LCR.changeStatusStep) {
                case 0:
                    LCR.changeStatusStep = 1;
                    break;
                case 2:
                    LCR.reloadPage();
                    break;
            }
        }).fail(function(msg) {
            // check status. If don't have good state after start. We lock system.
            $('body').removeClass('connection_lost');
            switch (LCR.changeStatusStep) {
                case 0:
                    LCR.changeStatusStep = -1;
                    break;
                case 1:
                    LCR.changeStatusStep = 2;
                    break;
            }

        });
    }

    LCR.connectWebSocket = function() {
        if (window.WebSocket) {
            LCR.ws = new WebSocket(LCR.config.socket_url);
            LCR.ws.binaryType = "arraybuffer";
        } else if (window.MozWebSocket) {
            LCR.ws = new MozWebSocket(LCR.config.socket_url);
            LCR.ws.binaryType = "arraybuffer";
        } else {
            console.log('Browser does not support WebSocket');
        }


        //Define WebSocket event listeners
        if (LCR.ws) {
            LCR.ws.onopen = function() {
                LCR.state.socket_opened = true;
                console.log('Socket opened');

                LCR.params.local['in_command'] = { value: 'send_all_params' };
                LCR.ws.send(JSON.stringify({ parameters: LCR.params.local }));
                LCR.params.local = {};
                LCR.unexpectedClose = true;
                LCR.startTime = performance.now();
                $('body').addClass('loaded');
                $('body').addClass('connection_lost');
                $('body').addClass('user_lost');
                LCR.startCheckStatus();
            };

            LCR.ws.onclose = function() {
                LCR.state.socket_opened = false;
                console.log('Socket closed');
                $('#modal_socket_closed').modal('show');
                if (LCR.unexpectedClose == true) {
                    setTimeout(LCR.reloadPage, '1000');
                }
            };

            $('#send_report_btn').on('click', function() {
                //var file = new FileReader();
                var mail = "support@redpitaya.com";
                var subject = "Crash report Red Pitaya OS";
                var body = "%0D%0A%0D%0A------------------------------------%0D%0A" + "DEBUG INFO, DO NOT EDIT!%0D%0A" + "------------------------------------%0D%0A%0D%0A";
                body += "Parameters:" + "%0D%0A" + JSON.stringify({ parameters: LCR.params }) + "%0D%0A";
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

            LCR.ws.onerror = function(ev) {
                if (!LCR.state.socket_opened)
                    LCR.startApp();
                console.log('Websocket error: ', ev);
            };

            LCR.ws.onmessage = function(ev) {
                if (LCR.state.processing) {
                    return;
                }

                var data = new Uint8Array(ev.data);
                var inflate = pako.inflate(data);
                var text = String.fromCharCode.apply(null, new Uint8Array(inflate));
                var receive = JSON.parse(text);

                LCR.state.processing = true;
                if (receive.parameters) {
                    if ((Object.keys(LCR.params.orig).length == 0) && (Object.keys(receive.parameters).length == 0)) {

                        LCR.params.local['in_command'] = { value: 'send_all_params' };
                        LCR.ws.send(JSON.stringify({ parameters: LCR.params.local }));
                        LCR.params.local = {};
                    } else {
                        console.log(receive.parameters)
                        LCR.processParameters(receive.parameters);
                    }
                    if (LCR.params.orig['is_demo']) {
                        setTimeout(function() {
                            if (!LCR.state.demo_modal_visible) {
                                LCR.state.demo_modal_visible = true;
                                $('#get_lic').modal('show');
                            }
                        }, 2500);
                        if (!LCR.state.demo_label_visible) {
                            LCR.state.demo_label_visible = true;
                            //$('#demo_label').show();
                            var htmlText = "<p id='browser_detect_text'>You are working in demo mode.</p>";
                            PopupStack.add(htmlText);
                            $('#demo_label').hide();
                        }
                    } else {
                        if (LCR.state.demo_label_visible) {
                            LCR.state.demo_label_visible = false;
                            $('#demo_label').hide();
                        }
                    }
                }
                LCR.state.processing = false;
            };
        }
    };

    // LCR.checkAll = function() {
    //     var trata = function(msg) {
    //         console.log(msg);
    //         if (msg != "\n") {
    //             var callback = function(msg) {
    //                 LCR.i2c0x20_res = msg + "";
    //                 if (LCR.i2c0x20_res.indexOf("--") != -1 && !LCR.module_disconnected) {
    //                     $('#modal_module_disconnected').modal('show');
    //                     LCR.module_disconnected = true;
    //                     LCR.startApp();
    //                 } else //if (LCR.i2c0x20_res.indexOf("--") == -1 && LCR.module_disconnected)
    //                 {
    //                     LCR.module_disconnected = false;
    //                     LCR.startApp();
    //                 }
    //             };
    //             $.get('/check_0x20').done(callback).fail(callback);
    //             LCR.i2ct_checked = true;
    //         } else {
    //             $("#i2c_install").modal('show');
    //             LCR.i2ct_checked = false;
    //         }
    //     };

    //     $.get('/check_i2c_tools').done(trata).fail(trata);
    // }

    // LCR.checkI2cTools = function() {
    //     $.get('/check_i2c_tools').done(function(msg) {
    //         console.log(msg);
    //         if (msg != "\n")
    //             LCR.i2ct_checked = true;
    //         else
    //             LCR.i2ct_checked = false;
    //     }).fail(function(msg) {
    //         console.log(msg);
    //         if (msg != "\n")
    //             LCR.i2ct_checked = true;
    //         else
    //             LCR.i2ct_checked = false;
    //     });
    // }

    // LCR.installI2cTools = function() {
    //     $.get('/install_i2c_tools').done(function(msg) {
    //         console.log(msg);
    //         if (LCR.unexpectedClose)
    //             return;
    //         var callback = function(msg) {
    //             LCR.i2c0x20_res = msg + "";
    //             if (!LCR.module_disconnected) {
    //                 $('#modal_module_disconnected').modal('show');
    //                 LCR.module_disconnected = true;
    //             } else if (LCR.i2c0x20_res.indexOf("--") == -1 && LCR.module_disconnected) {
    //                 LCR.module_disconnected = false;
    //                 LCR.startApp();
    //             }
    //         };
    //         $.get('/check_0x20').done(callback).fail(callback);
    //         LCR.i2ct_checked = true;
    //     }).fail(function(msg) {
    //         console.log(msg);
    //         if (LCR.unexpectedClose)
    //             return;
    //         // var callback = function(msg) {
    //         //     LCR.i2c0x20_res = msg + "";
    //         //     if (LCR.i2c0x20_res.indexOf("--") != -1 && !LCR.module_disconnected) {
    //         //         $('#modal_module_disconnected').modal('show');
    //         //         LCR.module_disconnected = true;
    //         //     } else if (LCR.i2c0x20_res.indexOf("--") == -1 && LCR.module_disconnected) {
    //         //         LCR.module_disconnected = false;
    //         //         LCR.startApp();
    //         //     }
    //         // };
    //         // $.get('/check_0x20').done(callback).fail(callback);
    //         LCR.module_disconnected = true;
    //         LCR.i2ct_checked = true;
    //     });
    // }

    LCR.processParameters = function(new_params) {

        var old_params = $.extend(true, {}, LCR.params.orig);

        var send_all_params = Object.keys(new_params).indexOf('send_all_params') != -1;
        for (var param_name in new_params) {
            LCR.params.orig[param_name] = new_params[param_name];

            if (param_name == 'LCR_SHUNT') {
                let shunt_text = "";
                switch (new_params['LCR_SHUNT'].value) {
                    case 0:
                        shunt_text = "10 Ohm";
                        break;
                    case 1:
                        shunt_text = "100 Ohm";
                        break;
                    case 2:
                        shunt_text = "1 kOhm";
                        break;
                    case 3:
                        shunt_text = "10 kOhm";
                        break;
                    case 4:
                        shunt_text = "100 kOhm";
                        break;
                    case 5:
                        shunt_text = "1 MOhm";
                        break;
                }
                $('#lb_shunt').empty().append(shunt_text);
            }
            
            if (param_name.includes("LCR_L")){
                new_params[param_name].value /= 100000;
            }

            if (param_name == 'LCR_RUN') {
                if (new_params['LCR_RUN'].value == false && new_params['LCR_TOLERANCE'].value == 0) {
                    $('#lb_prim_displ').empty().append(LCR.displ_params.prim_val);
                    $('#lb_prim_displ_units').empty().append(LCR.displ_params.p_units);

                    $('#lb_sec_displ').empty().append(LCR.displ_params.sec_val);
                    $('#lb_sec_displ_units').empty().append(LCR.displ_params.s_units);
                }
            }

            if (param_name == 'LCR_EXTMODULE_STATUS') {
                if (new_params['LCR_EXTMODULE_STATUS'].value === false) {
                    $('body').removeClass('loaded');
                    $('#text_message').show();
                    $('#cont').hide();
                    LCR.module_disconnected = true;
                } else {
                    if (LCR.module_disconnected == true) {
                        LCR.startApp();
                        LCR.module_disconnected = false;
                        return;
                    }
                    $('#text_message').hide();
                    $('#cont').show();
                }
            }

            //Change primary display value
            if (new_params['LCR_RUN'].value == true &&
                param_name == LCR.displ_params.prim && LCR.secondary_meas.apply_tolerance == false) {

                if (new_params['LCR_RANGE'].value == 0 && new_params['LCR_RELATIVE'].value == 0) {
                    formatRangeAuto(false, 1, new_params['LCR_C_PREC'].value, new_params[param_name].value);
                } else if (new_params['LCR_RELATIVE'].value == 0 && new_params['LCR_RANGE'].value != 0) {
                    formatRangeManual(new_params['LCR_C_PREC'].value, $('#sel_range_f').val(), parseInt($('#sel_range_u').val(), 10), new_params[param_name].value);
                } else {
                    console.log(new_params[param_name].value);
                    if (new_params['LCR_RELATIVE'].value < Math.abs(999)) {
                        formatRangeAuto(false, 1, null, new_params[param_name].value);
                    }
                }

                if (LCR.displ_params.prim_val < 0) {
                    $('#lb_prim_displ').empty().append("OVER R.");
                    $('#lb_prim_displ_units').empty();
                } else {

                    var units = LCR.displ_params.p_units;
                    var data = LCR.displ_params.prim_val;

                    if (data == "OVER R.") {
                        $('#lb_prim_displ').css('font-size', '100%').css('width: 100%');
                    } else {
                        $('#lb_prim_displ').css('font-size', '100%');
                    }

                    $('#lb_prim_displ').empty().append(data);
                    $('#lb_prim_displ_units').empty().append(units);
                }
            }

            //Change secondary display value
            if (new_params['LCR_RUN'].value == true && param_name == LCR.displ_params.sec && LCR.secondary_meas.apply_tolerance == false) {

                formatRangeAuto(0, 2, null, new_params[param_name].value);

                if (param_name == 'LCR_P') {

                    if (new_params[param_name].value < -180 || new_params[param_name].value > 180) {
                        $('#lb_sec_displ').empty().append("OVER R.");
                        $('#lb_sec_displ_units').empty();
                    }
                    $('#lb_sec_displ').empty().append(new_params[param_name].value.toFixed(2));
                    $('#lb_sec_displ_units').empty().append("deg");
                } else {
                    var units = LCR.displ_params.s_units;
                    var data = LCR.displ_params.sec_val;
                    $('#lb_sec_displ_units').empty().append(units);
                    $('#lb_sec_displ').empty().append(data);
                }

            }

            if (param_name == LCR.displ_params.prim && LCR.secondary_meas.apply_tolerance == true && new_params['LCR_RUN'].value == true) {


                var diff = 100 - (new_params[param_name].value / Math.abs(new_params['LCR_TOL_SAVED'].value) * 100);
                //var diff = (( - new_params[param_name].value)) /
                //  ((new_params['LCR_TOL_SAVED'].value + new_params[param_name].value) / 2) * 100);

                var str_diff = diff.toString().substr(0, 5);
                diff = parseInt(diff);

                formatRangeAuto(false, 1, null, new_params[param_name].value);
                var units = LCR.displ_params.p_units;
                var data = LCR.displ_params.prim_val;

                if (Math.abs(diff) > 1 && Math.abs(diff) < 100) {
                    console.log(diff.toFixed(2));
                    $('#lb_sec_displ').empty().append(100 - diff + "%");
                    $('#lb_prim_displ').empty().append(data);
                    $('#lb_prim_displ_units').empty().append(units);
                } else if (Math.abs(diff) > 100) {
                    $('#lb_sec_displ').empty().append("0%");
                    $('#lb_prim_displ').empty().append("0%");
                    $('#lb_prim_displ_units').empty();
                    $('#lb_sec_displ_units').empty();
                } else {
                    $('#lb_sec_displ').empty().append("100%");
                    $('#lb_prim_displ').empty().append(data);
                    $('#lb_prim_displ_units').empty().append(units);
                }
                console.log("selected_meas: " + LCR.selected_meas);
                console.log("Saved val: " + new_params['LCR_TOL_SAVED'].value);
                console.log("CUrrent meas value: " + new_params[param_name].value);
            }
            if (param_name == 'is_demo')
                LCR.params.orig['is_demo'] = new_params['is_demo'].value;

            // console.log(LCR.displ_params.prim);
            var quantity = param_name.substr(0, param_name.length - 4);
            if (param_name == (quantity + "_MIN") && param_name == (LCR.displ_params.prim + "_MIN")) {
                // console.log(quantity);
                var format = formatRangeAuto(true, 1, new_params['LCR_C_PREC'].value, new_params[param_name].value);
                if (format == "OVER R.") $('#meas_min_d').empty().append(0);
                else $('#meas_min_d').empty().append(format);
            }

            if (param_name == (quantity + "_MAX") && param_name == (LCR.displ_params.prim + "_MAX")) {
                var format = formatRangeAuto(true, 1, new_params['LCR_C_PREC'].value, new_params[param_name].value);
                if (format == "OVER R.") $('#meas_max_d').empty().append(0);
                else $('#meas_max_d').empty().append(format);
            }

            if (param_name == (quantity + "_AVG") && param_name == (LCR.displ_params.prim + "_AVG")) {
                var format = formatRangeAuto(true, 1, new_params['LCR_C_PREC'].value, new_params[param_name].value);
                if (format == "OVER R.") $('#meas_avg_d').empty().append(0);
                else $('#meas_avg_d').empty().append(format);
            }
        }

        if (LCR.data_log.save_data &&
            LCR.data_log.curr_store < LCR.data_log.max_store && $('#LCR_HOLD').is(':visible')) {
            $('#m_table tbody').append('<tr><td><input type="checkbox" name="data">' + LCR.data_log.curr_store + '</td><td>' +
                new_params[LCR.data_log.prim_display].value + '</td><td>' +
                new_params[LCR.data_log.sec_display].value + '</td></tr>');
            LCR.data_log.curr_store++;
        }
    };

    // Sends to server modified parameters
    LCR.sendParams = function() {
        if ($.isEmptyObject(LCR.params.local)) {
            return false;
        }

        if (!LCR.state.socket_opened) {
            console.log('ERROR: Cannot save changes, socket not opened');
            return false;
        }

        LCR.params.local['in_command'] = { value: 'send_all_params' };
        LCR.ws.send(JSON.stringify({ parameters: LCR.params.local }));
        LCR.params.local = {};

        return true;
    };


}(window.LCR = window.LCR || {}, jQuery));

$(function() {
    // $('#bt_install_i2c').click(function() {
    //     LCR.installI2cTools();
    // });

    // $('#bt_close_app1, #bt_close_app2').click(function() {
    //     //window.location.assign('/');
    //     $('#modal_module_disconnected').modal('hide');
    // });

    var reloaded = $.cookie("lcr_forced_reload");
    if (reloaded == undefined || reloaded == "false") {
        $.cookie("lcr_forced_reload", "true");
        window.location.reload(true);
    }

    LCR.client_id = 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
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
        data: LCR.client_id,
        //Options to tell jQuery not to process data or worry about content-type.
        cache: false,
        contentType: false,
        processData: false
    });

    LCR.checkStatus();

    //Header options. Prevent aggressive firefox caching
    $("html :checkbox").attr("autocomplete", "off");

    console.log('Processing on site events');

    //LCR set run state
    $('#LCR_START').on('click', function(ev) {
        ev.preventDefault();
        $('#LCR_START').hide();
        $('#LCR_HOLD').css('display', 'block');

        //Get value
        var freq = parseInt($("#LCR_FREQUENCY").val());
        LCR.params.local['LCR_FREQ'] = { value: freq };
        var shunt = parseInt($("#LCR_SHUNT").val());
        LCR.params.local['LCR_SHUNT'] = { value: freq };
        LCR.params.local['LCR_RUN'] = { value: true };
        LCR.sendParams();
    });

    //Freeze LCR data
    $('#LCR_HOLD').on('click', function(ev) {
        ev.preventDefault();
        $('#LCR_HOLD').hide();
        $('#LCR_START').css('display', 'block');
        LCR.params.local['LCR_RUN'] = { value: false };
        LCR.displ_params.prim_val = $('#lb_prim_displ').text();
        LCR.displ_params.sec_val = $('#lb_sec_displ').text();
        LCR.sendParams();
    });

    $('#btn_c_meas').on('click', function(ev) {
        //ev.preventDefault();
        LCR.params.local['LCR_M_RESET'] = { value: true };
        LCR.sendParams();
    });

    /* --------------------- CALIBRATION --------------------- */
    $('#LCR_CALIBRATE').on('click', function(ev) {
        ev.preventDefault();
        $('#LCR_HOLD').hide();
        $('#LCR_START').css('display', 'block');
        LCR.params.local['LCR_RUN'] = { value: false };
        LCR.sendParams();
        $('#modal_calib_start').modal('show');
    });

    $('#bt_calib_start').on('click', function(ev) {
        ev.preventDefault();
        LCR.params.local['LCR_CALIB_MODE'] = { value: 1 };
        LCR.params.local['LCR_CALIBRATION'] = { value: true };
        $('#modal_calib_start').modal('hide');
        $('#modal_calib_open').modal('show');
        LCR.sendParams();
    });

    $('#bt_calib_open').on('click', function(ev) {
        ev.preventDefault();
        LCR.params.local['LCR_CALIB_MODE'] = { value: 2 };
        LCR.params.local['LCR_CALIBRATION'] = { value: true };
        $('#modal_calib_open').modal('hide');
        $('#modal_calib_short').modal('show');
        LCR.sendParams();
        setTimeout(function() {
            return;
        }, 100);
        LCR.params.local['LCR_CALIB_MODE'] = { value: 0 };
        LCR.params.local['LCR_CALIBRATION'] = { value: true };
        LCR.sendParams();
    });

    /* ------------------------------------------------------- */

    //Log data
    $('#LCR_LOG').click(function(ev) {
        /*if (LCR.toClearLog) {
            clearTableAll();
            LCR.toClearLog = false;
        }*/

        ev.preventDefault();
        $('#LCR_LOG').hide();
        $('#LCR_LOG_STOP').css('display', 'block');
        LCR.data_log.save_data = true;
    });

    $('#LCR_LOG_STOP').click(function(ev) {
        ev.preventDefault();
        $('#LCR_LOG_STOP').hide();
        $('#LCR_LOG').css('display', 'block');
        LCR.data_log.save_data = false;
    });

    $('#LCR_FREQUENCY').change(function() {
        $('#meas_freq_d').empty().append($('option:selected', $(this)).text());
        LCR.params.local['LCR_FREQ'] = { value: parseInt(this.value) };
        LCR.sendParams();
    });

    $('#LCR_SHUNT').change(function() {
        LCR.params.local['LCR_SHUNT'] = { value: parseInt(this.value) };
        LCR.sendParams();
    });

    $('#prim_displ_choice :checkbox').click(function() {
        clearTableAll();
        //LCR.toClearLog = true;

        LCR.displ_params.prim = this.id;
        LCR.displ_params.p_base_u = this.value;

        var i;
        var op = document.getElementById("sel_range_u").getElementsByTagName("option");
        for (i = 0; i < 6; i++) {
            op[i].disabled = false;
            if (i == 3) op[i].text = this.value;
            else op[i].text = op[i].text.substr(0, 1) + this.value;
        }

        if (this.id == 'LCR_Z' || this.id == 'LCR_R') {
            for (i = 0; i < 3; i++) {
                op[i].disabled = true;
            }
            op[3].selected = true;
        } else if (this.id == 'LCR_C' || this.id == 'LCR_L') {
            for (i = 3; i < 6; i++) {
                op[i].disabled = true;
            }
            op[0].selected = true;
        }

        $('#meas_p_d').empty().append($(this).next('label').text());

        if (this.id == 'LCR_Z') {
            LCR.selected_meas = 1;
        } else if (this.id == 'LCR_L') {
            LCR.selected_meas = 2;
        } else if (this.id == 'LCR_C') {
            LCR.selected_meas = 3;
        } else if (this.id == 'LCR_R') {
            LCR.selected_meas = 4;
        }

        //Disable manual model, when switching quantities
        LCR.params.local['LCR_RANGE'] = { value: 0 };
        $('#cb_manual').prop("checked", false);
        $('#cb_auto').prop("checked", true);

        if (LCR.params.orig['LCR_TOLERANCE'].value != 0) {
            LCR.params.local['LCR_TOLERANCE'] = { value: LCR.selected_meas };
        }

        if (LCR.params.orig['LCR_RELATIVE'].value != 0) {
            LCR.params.local['LCR_RELATIVE'] = { value: LCR.selected_meas };
        }

        LCR.data_log.prim_display = this.id;
        LCR.sendParams();
        document.getElementById('table_p_header').innerHTML = LCR.data_log.prim_display[4];
    });

    $('#sec_displ_choice :checkbox').click(function() {
        //LCR.toClearLog = true;
        clearTableAll();

        LCR.displ_params.sec = this.id;
        LCR.displ_params.s_base_u = this.value;

        LCR.data_log.sec_display = this.id;

        $('#meas_s_d').empty().append($(this).next('label').text());
        document.getElementById('table_s_header').innerHTML = LCR.data_log.sec_display[4];
    });

    $('#LCR_LOG').on('click', function() {
        document.getElementById('table_p_header').innerHTML = LCR.data_log.prim_display[4];
        document.getElementById('table_s_header').innerHTML = LCR.data_log.sec_display[4];
        LCR.data_log.save_data = true;
    });

    $('#cb_tol').change(function() {

        if (!LCR.secondary_meas.apply_tolerance) {
            LCR.secondary_meas.apply_relative = false;
            LCR.params.local['LCR_RELATIVE'] = { value: 0 };
            $('#cb_rel').prop("checked", false);
            LCR.sendParams();
            LCR.secondary_meas.apply_tolerance = true;
            LCR.params.local['LCR_TOLERANCE'] = { value: LCR.selected_meas };
            $('#lb_prim_displ').empty().append("100%");
            $('#lb_prim_displ_units').empty();
            $('#lb_sec_displ_units').empty();
            $('#rec_image').css('display', 'block');
            $('#rec_lb').css('display', 'block');
        } else {
            LCR.secondary_meas.apply_tolerance = false;
            LCR.params.local['LCR_TOLERANCE'] = { value: 0 };
            $('#rec_image').css('display', 'none');
            $('#rec_lb').css('display', 'none');
            $('#cb_tol').prop("checked", false);
        }
        LCR.sendParams();
    });

    $('#cb_rel').change(function() {
        $('#cb_tol').prop("checked", false);
        LCR.secondary_meas.apply_tolerance = false;
        $('#rec_image').css('display', 'none');
        $('#rec_lb').css('display', 'none');

        if (!LCR.secondary_meas.apply_relative) {
            LCR.secondary_meas.apply_relative = true;
            LCR.params.local['LCR_RELATIVE'] = { value: LCR.selected_meas };
        } else {
            LCR.secondary_meas.apply_relative = false;
            LCR.params.local['LCR_RELATIVE'] = { value: 0 };
            $('#cb_rel').prop("checked", false);
        }
        LCR.sendParams();
    });

    $('#cb_ser').click(function() {
        LCR.params.local['LCR_SERIES'] = { value: true };
        LCR.sendParams();
    });

    $('#cb_paralel').click(function() {
        LCR.params.local['LCR_SERIES'] = { value: false };
        LCR.sendParams();
    });

    $('#cb_manual').click(function() {

        LCR.params.local['LCR_RANGE'] = {
            value: LCR.selected_meas
        }

        LCR.params.local['LCR_RANGE_F'] = {
            value: $('#sel_range_f :selected').val()
        }

        LCR.params.local['LCR_RANGE_U'] = {
            value: $('#sel_range_u :selected').val()
        }

        //LCR.displ_params.p_base_u = $('#sel_range_u :selected').text();
        $('#lb_prim_displ_units').empty().append($('#sel_range_u option:selected').text());
        $('#meas_mode_d').empty().append('Manual');
        LCR.sendParams();
    });

    $('#cb_auto').click(function() {
        LCR.params.local['LCR_RANGE'] = { value: 0 };
        $('#meas_mode_d').empty().append('Auto');
        LCR.sendParams();
    });

    $('#sel_range_u').change(function() {
        if (LCR.params.orig['LCR_RANGE'].value != 0) {
            //LCR.displ_params.p_base_u = $('#sel_range_u :selected').text();
            LCR.params.local['LCR_RANGE_U'] = { value: this.value };
            LCR.sendParams();
        }
    });

    $('#sel_range_f').change(function() {
        LCR.params.local['LCR_RANGE_F'] = { value: this.value };
        LCR.sendParams();
    });


    // Init help
    Help.init(helpListLCR);
    Help.setState("idle");

    // Everything prepared, start application
    LCR.startApp();
    //LCR.checkAll();
});

//TODO: This solution is ugly as fuck. Make it better!
function formatRangeAuto(meas_param, display, precision, meas_data) {


    var data = 0;
    var sub_idx = 1;
    var units = "";
    var base = "";
    var inverse = 1;

    if (meas_data < 0) {
        meas_data = (Math.abs(meas_data));
        inverse = -1;
    } else {
        meas_data = (Math.abs(meas_data));
    }

    if (display == 1 && meas_data != 0) {
        base = LCR.displ_params.p_base_u;
    } else if (display == 2 && meas_data != 0) {
        base = LCR.displ_params.s_base_u;
    }

    if (precision != null && LCR.displ_params.prim == "LCR_C") {
        meas_data = meas_data * Math.pow(10, -precision);
        if (meas_data > 0.1) {
            meas_data = "OVER R.";
        }
    }


    if (meas_data < 0.00000000000010) {
        data = "OVER R.";
    } else if (meas_data > 0.00000000000010 && meas_data <= 0.00000000999990) {

        data = (meas_data * Math.pow(10, 9)).toFixed(4);
        units = "n" + base;

    } else if (meas_data > 0.00000000999990 && meas_data <= 0.00000009999900) {

        data = (meas_data * Math.pow(10, 9)).toFixed(3);
        units = "n" + base;

    } else if (meas_data > 0.00000009999900 && meas_data <= 0.00000099999000) {

        data = (meas_data * Math.pow(10, 9)).toFixed(2);
        units = "n" + base;

    } else if (meas_data > 0.000000999990 && meas_data <= 0.00000999990) {

        data = (meas_data * Math.pow(10, 6)).toFixed(4);
        units = "u" + base;

    } else if (meas_data > 0.000009999900 && meas_data <= 0.00009999900) {

        data = (meas_data * Math.pow(10, 6)).toFixed(3);
        units = "u" + base;

    } else if (meas_data > 0.000099999000 && meas_data <= 0.00099999000) {

        data = (meas_data * Math.pow(10, 6)).toFixed(2);
        units = "u" + base;

    } else if (meas_data > 0.000999990 && meas_data <= 0.00999990) {

        data = (meas_data * Math.pow(10, 3)).toFixed(4);
        units = "m" + base;

    } else if (meas_data > 0.009999900 && meas_data <= 0.09999900) {

        data = (meas_data * Math.pow(10, 3)).toFixed(3);
        units = "m" + base;
    } else if (meas_data > 0.099999000 && meas_data <= 0.99999000) {

        data = (meas_data * Math.pow(10, 3)).toFixed(2);
        units = "m" + base;

    } else if (meas_data > 0.999990 && meas_data <= 9.99990) {

        data = meas_data.toFixed(4);
        units = base;

    } else if (meas_data > 9.99990 && meas_data <= 99.9990) {

        data = meas_data.toFixed(3);
        units = base;

    } else if (meas_data > 99.9990 && meas_data <= 999.990) {

        data = meas_data.toFixed(2);
        units = base;

    } else if (meas_data > 999.990 && meas_data <= 9999.90) {

        data = meas_data.toFixed(1);
        units = base;

    } else if (meas_data <= 99999.0 && meas_data > 9999.90) {

        data = (meas_data / Math.pow(10, 3)).toFixed(3);
        units = "k" + base;

    } else if (meas_data <= 999990.0 && meas_data > 99999.0) {

        data = (meas_data / Math.pow(10, 3)).toFixed(2);
        units = "k" + base;

    } else if (meas_data <= 9999900.0 && 999990.0) {

        data = (meas_data / Math.pow(10, 6)).toFixed(1);
        units = "M" + base;

    } else if (meas_data > 9999900.0) {

        data = "OVER R.";
        units = "";
    }

    //If we are formatting log data, we do not want to change the main measurment
    if (meas_param == true) {

        if (data == "OVER R.") {
            return data;
        }

        return (data * inverse + units);
    }

    switch (display) {
        case 1:
            if (data == "OVER R.") {
                LCR.displ_params.prim_val = data;
                LCR.displ_params.p_units = "";
                break;
            }
            LCR.displ_params.prim_val = data * inverse;
            LCR.displ_params.p_units = units;
            break;
        case 2:
            if (data == "OVER R.") {
                LCR.displ_params.sec_val = data;
                LCR.displ_params.s_units = "";
                break;
            }

            LCR.displ_params.sec_val = data * inverse;
            LCR.displ_params.s_units = units;
            break;
    }

    return 0;
}

function formatRangeManual(precision, format, power, data) {

    var suffixes = ['n', 'u', 'm', '', 'k', 'M'];
    var formats = [1.0000, 10.000, 100.00, 1000.0];
    //Direct formats mirror table
    var limits = [9.999, 99.999, 999.99, 9999.9];
    var format_size = 5;

    if (precision != null && LCR.displ_params.prim == "LCR_C") {
        data = data * Math.pow(10, -precision);
    }

    var i;
    var c = 0;
    for (i = 9; i > -7; i -= 3) {
        if (c == power) {
            data = (data * Math.pow(10, i));
            break;
        }
        c++;
    }

    var limit = limits[parseInt(format)];
    if (data < 0 || (data > limit)) {
        LCR.displ_params.prim_val = "OVER R.";
        LCR.displ_params.p_units = "";
        return;
    }

    var string_data = data.toString();
    var formatted_data = "";
    var flt_idx = parseInt(format) + 1;
    var string_flt;

    //Divide whole and decimal part.
    for (var j = 0; j < string_data.length; j++) {
        if (string_data[j] == '.') {
            string_flt = j;
            break;
        }
    }

    var w = string_data.substr(0, j);
    console.log("W: " + w);
    var d = string_data.substr(j + 1, string_data.length - 1);
    console.log("D: " + d);
    var indicies = [0, 0, 0, 0, 0, 0];

    for (var k = 0; k < w.length; k++) {
        indicies[flt_idx - 1 - k] = w[w.length - 1 - k];
    }

    for (var n = 0; n < (4 - parseInt(format)); n++) {
        indicies[n + flt_idx + 1] = d[n];
    }

    indicies[flt_idx] = '.';

    console.log(indicies);
    console.log(data);

    for (var m = 0; m < 6; m++) {
        if (m == 0) {
            formatted_data += indicies[m];
            continue;
        }

        //if(m < flt_idx && indicies[m] == 0){
        //  continue;
        //}
        if (indicies[m] == undefined) formatted_data += '0';
        else formatted_data += indicies[m];
    }

    if (parseFloat(formatted_data) == 0) {
        LCR.displ_params.prim_val = "OVER R.";
        LCR.displ_params.p_units = "";
        return;
    }

    LCR.displ_params.prim_val = formatted_data;
    LCR.displ_params.p_units = suffixes[c] + LCR.displ_params.p_base_u;

    return data;
}

function clearRowTable() {
    $('#m_table tr').has('input[name="data"]:checked').remove();
}

function clearTableAll() {
    $('#m_table td').remove();
    LCR.data_log.curr_store = 1;
}

function export_table() {
    //var csv = $("#m_table").table2CSV({delivery:'value'});
    //window.location.href = 'data:application/csv;charset=UTF-8,' + encodeURIComponent(csv);
    $('#m_table').table2CSV();
}