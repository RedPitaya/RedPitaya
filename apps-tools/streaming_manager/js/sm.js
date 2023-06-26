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
    SM.adc_channels = undefined;
    SM.dac_channels = undefined;
    SM.is_acdc = undefined;

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
                SM.requestAllParameters();
                SM.unexpectedClose = true;
                SM.getClients();
                SM.refreshFiles();
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

        SM.ws.send(JSON.stringify({ parameters: SM.parametersCache }));
        console.log(SM.parametersCache)
        SM.parametersCache = {};
        return true;
    };

    SM.requestAllParameters = function() {
        if (!SM.state.socket_opened) {
            console.log('ERROR: Cannot save changes, socket not opened');
            return false;
        }
        SM.parametersCache = {};
        SM.parametersCache["in_command"] = { value: "send_all_params" };
        SM.ws.send(JSON.stringify({ parameters: SM.parametersCache }));
        console.log(SM.parametersCache)
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
                $('#svg-is-runnung').attr("src","./img/red_led.png")
                $('#info_dialog_label').text("Out of free disk space");
                $('#info_dialog').modal('show');
                SM.parametersCache["SS_STATUS"] = { value: 0 };
                SM.sendParameters();
                SM.refreshFiles();
            }

            if (ss_status == 3) {
                $('#svg-is-runnung').attr("src","./img/red_led.png")
                $('#info_dialog_label').text("Data recording completed");
                $('#info_dialog').modal('show');
                SM.parametersCache["SS_STATUS"] = { value: 0 };
                SM.sendParameters();
                SM.refreshFiles();
            }

            if (ss_status == 1) {

                $('#svg-is-runnung').attr("src","./img/green_led.png")
            }

            if (ss_status == 0) {
                $('#svg-is-runnung').attr("src","./img/red_led.png")
                SM.refreshFiles();
            }
        }
        SM.ss_status_last = ss_status;
    }

    SM.change_adc_data_pass = function(new_params) {
        if (new_params['SS_ADC_DATA_PASS'].value == 1) {
            $('#svg-is-data-pass').show();
        }else{
            $('#svg-is-data-pass').hide();
        }
    }

    //Handlers
    var signalsHandler = function() {
        if (SM.signalStack.length > 0) {

            SM.signalStack.splice(0, 1);
        }
        if (SM.signalStack.length > 2)
            SM.signalStack.length = [];
    }

    SM.updateADCMode = function(state){
        // if (new_params[param_name].value){
        //     $(".network").hide();
        //     $(".file").show();
        // }else{
        //     $(".network").show();
        //     $(".file").hide();
        // }
    }

    SM.processParameters = function(new_params) {
        var old_params = $.extend(true, {}, SM.params.orig);
        var send_all_params = Object.keys(new_params).indexOf('send_all_params') != -1;
        SM.updateMaxLimits(new_params['RP_MODEL_STR']);

        if (Object.keys(new_params).length !== 0)

            console.log(new_params)

        for (var param_name in new_params) {

            SM.params.orig[param_name] = new_params[param_name];
            var field = $('#' + param_name);

            if (SM.param_callbacks[param_name] !== undefined)
                SM.param_callbacks[param_name](new_params);

            // Do not change fields from dialogs when user is editing something
            // if ((old_params[param_name] === undefined || old_params[param_name].value !== new_params[param_name].value)) {
            //     if (field.is('select') || field.is('input:text')) {
            //         if (param_name == "SS_RATE"){
            //             SM.ss_rate = new_params[param_name].value;
            //             rateFocusOutValue();
            //         }else{
            //             field.val(new_params[param_name].value)
            //         }
            //     } else if (field.is('button')) {
            //         field[new_params[param_name].value === true ? 'addClass' : 'removeClass']('active');
            //     } else if (field.is('input:radio')) {
            //         if (param_name == "SS_USE_FILE"){
            //             SM.updateADCMode(new_params[param_name].value);
            //         }else{
            //             var radios = $('input[name="' + param_name + '"]');
            //             radios.closest('.btn-group').children('.btn.active').removeClass('active');
            //             radios.eq([+new_params[param_name].value]).prop('checked', true).parent().addClass('active');
            //         }
            //     } else if (field.is('span')) {
            //         field.html(new_params[param_name].value);
            //     }
            // }
        }

    };

    SM.calcRateHz = function(val) {
        if (val <= 1)
            val = 1;
        if (val > SM.ss_max_rate)
            val = SM.ss_max_rate;
        SM.ss_rate = Math.round(SM.ss_full_rate / val);
        if (SM.ss_rate >= 65536){
            SM.ss_rate = 65536;
        }
        if (SM.ss_rate == 3){
            SM.ss_rate = 4;
        }
        if (SM.ss_rate >= 5 && SM.ss_rate <= 7){
            SM.ss_rate = 8;
        }
        if (SM.ss_rate >= 9 && SM.ss_rate <= 15){
            SM.ss_rate = 16;
        }
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
            url: '/get_streaming_ip',
            type: 'GET',
        }).fail(function(msg) {
            $('#SS_IP_ADDR').val(parseAddress(msg.responseText));
            ipAddressChange();
        }).done(function(msg) {
            $('#SS_IP_ADDR').val(parseAddress(msg.responseText));
            ipAddressChange();
        });
    }

    SM.getClients = function() {
        $("#PC_DESK_WIN").removeAttr("href");
        $("#PC_CON_WIN").removeAttr("href");
        $("#PC_DESK_LIN").removeAttr("href");
        $("#PC_CON_LIN").removeAttr("href");
        $("#PC_DESK_WIN > img").attr('src',"./img/DeskWinRed.png")
        $("#PC_CON_WIN > img").attr('src',"./img/ConWinRed.png")
        $("#PC_DESK_LIN > img").attr('src',"./img/DeskLinRed.png")
        $("#PC_CON_LIN > img").attr('src',"./img/ConLinRed.png")
        $.ajax({
            url: '/streaming_manager_get_clients_list',
            type: 'GET',
        }).fail(function(msg) {
            console.log("Error get clients",msg)
        }).done(function(msg) {

            const splitLines = msg => msg.split(/\r?\n/);
            splitLines(msg).forEach(function(item){
                if (item !== ""){
                    console.log(item)
                    var is_win = item.includes("win.zip")
                    var is_desktop = item.includes("rpsa_client-desktop")
                    if (is_desktop && is_win){
                        $("#PC_DESK_WIN > img").attr('src',"./img/DeskWinGreen.png")
                        $("#PC_DESK_WIN").attr('href',"/streaming_manager/clients/"+item)
                    } else if (is_desktop) {
                        $("#PC_DESK_LIN > img").attr('src',"./img/DeskLinGreen.png")
                        $("#PC_DESK_LIN").attr('href',"/streaming_manager/clients/"+item)
                    } else if (is_win) {
                        $("#PC_CON_WIN > img").attr('src',"./img/ConWinGreen.png")
                        $("#PC_CON_WIN").attr('href',"/streaming_manager/clients/"+item)
                    } else {
                        $("#PC_CON_LIN > img").attr('src',"./img/ConLinGreen.png")
                        $("#PC_CON_LIN").attr('href',"/streaming_manager/clients/"+item)
                    }
                }
            });

        });
    }

    SM.calcSize = function(x) {
        if (x  < 1024) {
            return x + " b"
        }

        if (x  < 1024 * 1024) {
            return (x / 1024).toFixed(3)  + " kb"
        }

        return (x / (1024 * 1024)).toFixed(3)  + " Mb"
    }

    SM.refreshFiles = function() {
        $("#files_table").empty();

        $.ajax({
            url: '/streaming_manager_get_files',
            type: 'GET',
        }).fail(function(msg) {
            console.log("Error get stored files",msg)
        }).done(function(msg) {

            const splitLines = msg => msg.split(/\r?\n/);
            var files = {}
            splitLines(msg).forEach(function(item){
                if (item !== "" && item.includes("data_file")){
                    var cols = item.trim().split(' ');
                    cols = cols.filter(item => item.trim().length > 0); 
                    if (cols.length >= 7){
                        var size = parseInt(cols[4]);
                        var name = cols[8].toString();
                        var baseName = name.substr(0,29);
                        if (files[baseName] == undefined){
                            files[baseName] = {}
                        }
                        if (name.endsWith(".log.lost")){
                            files[baseName]["log.lost"] = true
                        }

                        if (name.endsWith(".log")){
                            files[baseName]["log"] = true
                        }
                        
                        if (name.endsWith(".tdms")){
                            files[baseName]["format"] = "tdms"
                            files[baseName]["size"] = size
                        }

                        if (name.endsWith(".bin")){
                            files[baseName]["format"] = "bin"
                            files[baseName]["size"] = size
                        }

                        if (name.endsWith(".wav")){
                            files[baseName]["format"] = "wav"
                            files[baseName]["size"] = size
                        }
                    }
                }
            });
            console.log(files)

            for (const [key, value] of Object.entries(files)) {
                
                var new_row = document.createElement('div');
                new_row.className = "filecell"
                var sub_row = document.createElement('div');
                new_row.append(sub_row)
                var sub_row_name = document.createElement('div');
                sub_row.append(sub_row_name)
                var label_name = document.createElement('label');
                sub_row_name.append(label_name)
                label_name.innerText = key
                var span = document.createElement('span');
                sub_row.append(span)
                var item = document.createElement('item');
                var subitem = document.createElement('subitem');
                item.append(subitem)
                var item2 = document.createElement('item2');
                var item3 = document.createElement('item3');
                var item4 = document.createElement('item4');
                span.append(item)
                span.append(item2)
                span.append(item3)
                span.append(item4)
                label_name = document.createElement('label');
                label_name.innerText = value["format"] !== undefined ? value["format"] : "Error"
                subitem.append(label_name)
                label_name = document.createElement('label');
                label_name.innerText =  SM.calcSize(value["format"] !== undefined ? value["size"] : 0)
                item2.append(label_name)

                if (value["log"] !== undefined && value["log"]){
                    var li = document.createElement('li');
                    li.className = "run_buttons2"
                    item4.append(li)
                    var a = document.createElement('a');
                    a.innerText = "LOG"
                    a.setAttribute('href', "/streaming_manager/upload/" + key + "." + value["format"] + ".log");                    
                    li.append(a)
                }

                if (value["log.lost"] !== undefined && value["log.lost"]){
                    var li = document.createElement('li');
                    li.className = "run_buttons2"
                    item4.append(li)
                    var a = document.createElement('a');
                    a.innerText = "LOST"
                    a.setAttribute('href',"/streaming_manager/upload/" + key + "." + value["format"] + ".log.lost");
                    li.append(a)

                }

                var li = document.createElement('li');
                li.className = "run_buttons2"
                item4.append(li)
                var a = document.createElement('a');
                a.innerText = "🡇"
                a.setAttribute('href', "/streaming_manager/upload/" + key + "." + value["format"]);
                li.append(a)          
                document.getElementById('files_table').appendChild(new_row);

            }
        });
    }
    

    SM.setBoardMode = function(param) {
        var str = ""

        if (param["SS_IS_MASTER"].value === 0) {
            str = "(Unknown mode)"
        }

        if (param["SS_IS_MASTER"].value === 1) {
            str = "(Master mode)"
        }

        if (param["SS_IS_MASTER"].value === 2) {
            str = "(Slave mode)"
        }

        $("#TITLE_ID").text("Stream server application " + str)
    }

    SM.setRate = function(param) {
        var value = param["SS_RATE"].value;
        SM.ss_rate = value;
        rateFocusOutValue();
    }

    SM.setProtocol = function(param) {
        var value = param["SS_PROTOCOL"].value;
        $("#SS_PROTOCOL").prop('checked', value);
    }

    SM.setSaveMode = function(param) {
        var value = param["SS_SAVE_MODE"].value;
        $("#SS_SAVE_MODE").prop('checked', value);
    }

    SM.setCalibration = function(param) {
        var value = param["SS_USE_CALIB"].value;
        $("#SS_USE_CALIB").prop('checked', value);
    }

    SM.setMode = function(param) {
        var value = param["SS_USE_FILE"].value;
        $("#SS_MODE").prop('checked', value);
        if (value){
            $(".net_use").hide();
            $(".file_use").show();
        }else{
            $(".net_use").show();
            $(".file_use").hide();
        }
    }

    SM.setPort = function(param) {
        var value = param["SS_PORT_NUMBER"].value;
        $("#SS_PORT_NUMBER").val(value)
    }

    SM.setSamples = function(param) {
        var value = param["SS_SAMPLES"].value;
        $("#SS_SAMPLES").val(value)
    }

    SM.setFormat = function(param) {
        var value = param["SS_FORMAT"].value;
        $("#SS_FORMAT").val(value)
    }

    SM.setResolution = function(param) {
        var value = param["SS_RESOLUTION"].value;
        $("#SS_RESOLUTION").prop('checked', value);
    }

    SM.setChannel = function(param) {
        var value = param["SS_CHANNEL"].value;
        $("#SS_CH1_ENABLE").prop('checked', value & 0x1);
        $("#SS_CH2_ENABLE").prop('checked', value & 0x2);
        $("#SS_CH3_ENABLE").prop('checked', value & 0x4);
        $("#SS_CH4_ENABLE").prop('checked', value & 0x8);
    }

    SM.setAttenuator = function(param) {
        var value = param["SS_ATTENUATOR"].value;
        $("#SS_CH1_ATTENUATOR").prop('checked', value & 0x1);
        $("#SS_CH2_ATTENUATOR").prop('checked', value & 0x2);
        $("#SS_CH3_ATTENUATOR").prop('checked', value & 0x4);
        $("#SS_CH4_ATTENUATOR").prop('checked', value & 0x8);
    }

    SM.setACDC = function(param) {
        var value = param["SS_AC_DC"].value;
        $("#SS_CH1_AC_DC").prop('checked', value & 0x1);
        $("#SS_CH2_AC_DC").prop('checked', value & 0x2);
        $("#SS_CH3_AC_DC").prop('checked', value & 0x4);
        $("#SS_CH4_AC_DC").prop('checked', value & 0x8);
    }

    SM.setADCChannel = function(param) {
        var value = param["SS_ADC_CHANNELS"].value;
        if (SM.adc_channels == undefined){
            SM.adc_channels = value;
            for(var i = 1 ; i <= 4; i++){
                if (i > value){
                    $(".ch"+i).hide();
                }
            }
        }
    }

    SM.setDACChannel = function(param) {
        var value = param["SS_DAC_CHANNELS"].value;
        if (SM.dac_channels == undefined){
            SM.dac_channels = value;
            for(var i = 1 ; i <= 4; i++){
                if (i > value){
                    $(".dac_ch"+i).hide();
                }
            }
        }
    }

    SM.setISACDC = function(param) {
        var value = param["SS_IS_AC_DC"].value;
        if (SM.is_acdc == undefined){
            SM.is_acdc = value;
            if (!value)
                $(".ac_dc").hide();
        }
    }

    //Set handlers timers
    //    setInterval(signalsHandler, 40);
    setInterval(parametersHandler, 50);

    SM.param_callbacks["SS_STATUS"] = SM.change_status;
    SM.param_callbacks["SS_ADC_DATA_PASS"] = SM.change_adc_data_pass;
    SM.param_callbacks["SS_IS_MASTER"] = SM.setBoardMode;
    SM.param_callbacks["SS_RATE"] = SM.setRate;
    SM.param_callbacks["SS_PROTOCOL"] = SM.setProtocol;
    SM.param_callbacks["SS_SAVE_MODE"] = SM.setSaveMode;
    SM.param_callbacks["SS_USE_CALIB"] = SM.setCalibration;
    SM.param_callbacks["SS_USE_FILE"] = SM.setMode;
    SM.param_callbacks["SS_PORT_NUMBER"] = SM.setPort;
    SM.param_callbacks["SS_SAMPLES"] = SM.setSamples;
    SM.param_callbacks["SS_FORMAT"] = SM.setFormat;
    SM.param_callbacks["SS_CHANNEL"] = SM.setChannel;
    SM.param_callbacks["SS_ATTENUATOR"] = SM.setAttenuator;
    SM.param_callbacks["SS_AC_DC"] = SM.setACDC;
    SM.param_callbacks["SS_ADC_CHANNELS"] = SM.setADCChannel;
    SM.param_callbacks["SS_DAC_CHANNELS"] = SM.setDACChannel;
    SM.param_callbacks["SS_IS_AC_DC"] = SM.setISACDC;
    SM.param_callbacks["SS_RESOLUTION"] = SM.setResolution;
    

    
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

    $('#B_REFRESH').on('click', function(ev) {
        ev.preventDefault();
        SM.refreshFiles();
    });

    
    $('#B_DELETE_ALL').on('click', function(ev) {
        ev.preventDefault();
        $.ajax({
            url: '/stream_manager_delete_files',
            type: 'GET',
        }).fail(function(msg) {
            SM.refreshFiles();
        }).done(function(msg) {
            SM.refreshFiles();
        });
    });



    // Bind to the window resize event to redraw the graph; trigger that event to do the first drawing
    $(window).resize(function() {
        if (SM.ws) {
            SM.sendParameters();
        }
    }).resize();




    // Stop the application when page is unloaded
    $(window).on('beforeunload', function(event) {
        var target = document.activeElement.href
        console.log(document.activeElement.href)
        if (!target.includes("/streaming_manager/")){
            SM.ws.onclose = function() {}; // disable onclose handler first
            SM.ws.close();
            $.get(
                SM.config.stop_app_url
            )
        }
    });



    //Crash buttons
    $('#send_report_btn').on('click', function() { SM.formEmail() });
    $('#restart_app_btn').on('click', function() { location.reload() });

    $('#CLEAR_FILES').click(SM.DeleteFiles);


    // Everything prepared, start application
    SM.startApp();

});