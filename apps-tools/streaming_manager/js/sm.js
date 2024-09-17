/*
 * Red Pitaya stream service manager
 *
 * Author: Danilyuk Nikolay <n.danilyuk@integrasources.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

function promptFile(contentType, multiple) {
    var input = document.createElement("input");
    input.type = "file";
    input.accept = '.wav,.tdms';
    return new Promise(function(resolve) {
      document.activeElement.onfocus = function() {
        document.activeElement.onfocus = null;
        setTimeout(resolve, 500);
      };
      input.onchange = function() {
        var files = Array.from(input.files);
        resolve(files[0]);
      };
      input.click();
    });
}

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

    SM.ss_status_last = -1;
    SM.ss_dac_status_last = -1;
    SM.ss_gpio_status_last = -1;
    SM.ss_full_rate = undefined;
    SM.ss_rate = -1;
    SM.ss_max_rate = -1;
    SM.ss_max_rate_devider = -1;
    SM.param_callbacks = {};


    SM.adc_channels = undefined;
    SM.dac_channels = undefined;
    SM.is_acdc = undefined;
    SM.is_dac_gain = undefined;


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


    //Write email
    SM.formEmail = function() {
        //var file = new FileReader();
        var mail = "support@redpitaya.com";
        var subject = "Crash report Red Pitaya OS";
        var body = "%0D%0A%0D%0A------------------------------------%0D%0A" + "DEBUG INFO, DO NOT EDIT!%0D%0A" + "------------------------------------%0D%0A%0D%0A";
        body += "Parameters:" + "%0D%0A" + JSON.stringify({ parameters: CLIENT.parametersCache }) + "%0D%0A";
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
            if (ss_status == 0) {
                $('#ADC_STATUS_LED').attr("src","./img/red_led.png")
                $('#ADC_STATUS').html("Status: Stopped").css("color","#cdcccc")
            }

            if (ss_status == 1) {
                $('#ADC_STATUS_LED').attr("src","./img/green_led.png")
                $('#ADC_STATUS').html("Status: Runned").css("color","#cdcccc")
            }

            if (ss_status == 2) {
                $('#ADC_STATUS_LED').attr("src","./img/red_led.png")
                $('#ADC_STATUS').html("Status: Out of free disk space").css("color","#FF0000")
                // $('#info_dialog_label').text("Out of free disk space");
                // $('#info_dialog').modal('show');
                CLIENT.parametersCache["SS_STATUS"] = { value: 0 };
                CLIENT.sendParameters();
            }

            if (ss_status == 3) {
                $('#ADC_STATUS_LED').attr("src","./img/red_led.png")
                $('#ADC_STATUS').html("Status: Data recording completed").css("color","#cdcccc")

                // $('#info_dialog_label').text("Data recording completed");
                // $('#info_dialog').modal('show');
                CLIENT.parametersCache["SS_STATUS"] = { value: 0 };
                CLIENT.sendParameters();
            }

            if (ss_status == 4) {
                $('#ADC_STATUS_LED').attr("src","./img/red_led.png")
                $('#ADC_STATUS').html("Status: Not enough memory").css("color","#FF0000")
                // $('#info_dialog_label').text("Out of free disk space");
                // $('#info_dialog').modal('show');
                CLIENT.parametersCache["SS_STATUS"] = { value: 0 };
                CLIENT.sendParameters();
            }

            if (ss_status == 5) { // Mem modify status
                $('#ADC_STATUS_LED').attr("src","./img/red_led.png")
                $('#ADC_STATUS').html("Status: Stopped").css("color","#cdcccc")
                CLIENT.parametersCache["SS_STATUS"] = { value: 0 };
                CLIENT.sendParameters();
            }

        }
        SM.ss_status_last = ss_status;
    }

    SM.change_dac_status = function(new_params) {
        var status = new_params['SS_DAC_STATUS'].value;
        if (SM.ss_dac_status_last != status) {
            if (status == 0) {
                $('#DAC_STATUS_LED').attr("src","./img/red_led.png")
                $('#DAC_STATUS').html("Status: Stopped").css("color","#cdcccc")
            }

            if (status == 1) {
                $('#DAC_STATUS_LED').attr("src","./img/green_led.png")
                $('#DAC_STATUS').html("Status: Runned").css("color","#cdcccc")
            }

            if (status == 2) {
                $('#DAC_STATUS_LED').attr("src","./img/red_led.png")
                $('#DAC_STATUS').html("Status: Done").css("color","#008000")
                CLIENT.parametersCache["SS_DAC_STATUS"] = { value: 0 };
                CLIENT.sendParameters();
            }

            if (status == 3) {
                $('#DAC_STATUS_LED').attr("src","./img/red_led.png")
                $('#DAC_STATUS').html("Status: File is empty").css("color","#FF0000")
                CLIENT.parametersCache["SS_DAC_STATUS"] = { value: 0 };
                CLIENT.sendParameters();
            }

            if (status == 4) {
                $('#DAC_STATUS_LED').attr("src","./img/red_led.png")
                $('#DAC_STATUS').html("Status: File is broken").css("color","#FF0000")
                CLIENT.parametersCache["SS_DAC_STATUS"] = { value: 0 };
                CLIENT.sendParameters();
            }

            if (status == 5) {
                $('#DAC_STATUS_LED').attr("src","./img/red_led.png")
                $('#DAC_STATUS').html("Status: File is missing").css("color","#FF0000")
                CLIENT.parametersCache["SS_DAC_STATUS"] = { value: 0 };
                CLIENT.sendParameters();
            }

            if (status == 6) {
                $('#DAC_STATUS_LED').attr("src","./img/red_led.png")
                $('#DAC_STATUS').html("Status: Not enough memory").css("color","#FF0000")
                CLIENT.parametersCache["SS_DAC_STATUS"] = { value: 0 };
                CLIENT.sendParameters();
            }

            if (status == 7) { // Mem modify status
                $('#DAC_STATUS_LED').attr("src","./img/red_led.png")
                $('#DAC_STATUS').html("Status: Stopped").css("color","#cdcccc")
                CLIENT.parametersCache["SS_DAC_STATUS"] = { value: 0 };
                CLIENT.sendParameters();
            }
        }
        SM.ss_dac_status_last = status;
    }

    SM.change_adc_data_pass = function(new_params) {
        if (new_params['SS_ADC_DATA_PASS'].value == 1) {
            $('#ADC_DATA_PASS').show();
        }else{
            $('#ADC_DATA_PASS').hide();
            SM.refreshFiles();
        }
    }

    SM.change_dac_data_pass = function(new_params) {
        if (new_params['SS_DAC_DATA_PASS'].value == 1) {
            $('#DAC_DATA_PASS').show();
        }else{
            $('#DAC_DATA_PASS').hide();
        }
    }

    SM.updateADCMode = function(state){
    }

    SM.updateMaxLimits = function(adc_rate) {
        if (adc_rate !== undefined) {
            if (SM.ss_full_rate === undefined) {
                SM.ss_full_rate = adc_rate.value;
                SM.ss_max_rate = adc_rate.value;
                SM.ss_max_rate_devider = 1;
                SM.updateLimits();
            }
        }
    };

    SM.updateLimits = function() {
        if (SM.ss_rate != -1) {
            SM.calcRateHz(SM.ss_full_rate / SM.ss_rate);
        }
        rateFocusOutValue();
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
        $("#RP_CON_LIN > img").attr('src',"./img/ConRPRed.png")

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
                    var is_rp = item.includes("rp.zip")
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
                    } else if (is_rp) {
                        $("#RP_CON_LIN > img").attr('src',"./img/ConRPGreen.png")
                        $("#RP_CON_LIN").attr('href',"/streaming_manager/clients/"+item)
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
                        if (name.endsWith(".log.lost.txt")){
                            files[baseName]["log.lost"] = true
                        }

                        if (name.endsWith(".log.txt")){
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
                    a.setAttribute('href', "/streaming_manager/upload/adc/" + key + "." + value["format"] + ".log.txt");
                    a.setAttribute("download", "");
                    li.append(a)
                }

                if (value["log.lost"] !== undefined && value["log.lost"]){
                    var li = document.createElement('li');
                    li.className = "run_buttons2"
                    item4.append(li)
                    var a = document.createElement('a');
                    a.innerText = "LOST"
                    a.setAttribute('href',"/streaming_manager/upload/adc/" + key + "." + value["format"] + ".log.lost.txt");
                    a.setAttribute("download", "");
                    li.append(a)

                }

                var li = document.createElement('li');
                li.className = "run_buttons2"
                item4.append(li)
                var a = document.createElement('a');
                a.innerText = "🡇"
                a.setAttribute('href', "/streaming_manager/upload/adc/" + key + "." + value["format"]);
                a.setAttribute("download", "");
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
            $("#SS_PASS_SAMPLES").show();
            $("#SS_WRITED_SIZE").show();
        }else{
            $(".net_use").show();
            $(".file_use").hide();
            $("#SS_PASS_SAMPLES").hide();
            $("#SS_WRITED_SIZE").hide();
        }
    }

    SM.setSamples = function(param) {
        var value = param["SS_SAMPLES"].value;
        if (value == 0){
            $("#SS_SAMPLES").val("ALL");
        }else{
            $("#SS_SAMPLES").val(value)
        }
    }


    SM.setFormat = function(param) {
        var value = param["SS_FORMAT"].value;
        $("#SS_FORMAT").val(value)
    }

    SM.setResolution = function(param) {
        var value = param["SS_RESOLUTION"].value;
        $("#SS_RESOLUTION").prop('checked', value);
    }

    SM.setSamplesCount = function(param) {
        var value = param["SS_PASS_SAMPLES"].value;
        $("#SS_PASS_SAMPLES").html("Samples pass: " + value)
    }

    SM.setWritedSize = function(param) {
        var value = param["SS_WRITED_SIZE"].value;
        if (value < 1024){
            value = value + " B"
        }else if (value < 1024 * 1024){
            value = (value/1024).toFixed(0) +" kB"
        }else {
            value = ((value / (1024 * 1024))).toFixed(0) +" MB"
        }
        $("#SS_WRITED_SIZE").html("Recorded to SD: " + value)
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

    SM.setBlockSize = function(param) {
        var value = param["MM_BLOCK_SIZE"].value;
        var max = param["MM_BLOCK_SIZE"].max;
        $('#MM_BLOCK_SIZE option').each(function() {
            var v = $(this).val()
            if ( v > max ) {
                $(this).remove();
            }
        });
        $("#MM_BLOCK_SIZE").val(value)
    }

    SM.setADCSize = function(param) {
        var value = param["MM_ADC_SIZE"].value;
        var max = param["MM_ADC_SIZE"].max;

        $("#ADC_SIZE_TITLE").html("ADC: " + SM.calcSize(value))
        $("#MM_ADC_SIZE").prop('max',max)
        $("#MM_ADC_SIZE").prop('value',value)
    }

    SM.setADCValid = function(param) {
        var value = param["MM_ADC_VALID"].value;
        if (value == false)
            $("#ADC_SIZE_TITLE").addClass('red')
        else
            $("#ADC_SIZE_TITLE").removeClass('red')
    }


    SM.setDACSize = function(param) {
        var value = param["MM_DAC_SIZE"].value;
        var max = param["MM_DAC_SIZE"].max;

        $("#DAC_SIZE_TITLE").html("DAC: " + SM.calcSize(value))
        $("#MM_DAC_SIZE").prop('max',max)
        $("#MM_DAC_SIZE").prop('value',value)
    }

    SM.setDACMode = function(param) {
        var value = param["SS_DAC_MODE"].value;
        $("#SS_DAC_MODE").prop('checked', value);
        if (value){
            $(".dac_file_use").show();
        //     $("#SS_PASS_SAMPLES").show();
        //     $("#SS_WRITED_SIZE").show();
        }else{
            $(".dac_file_use").hide();
        //     $("#SS_PASS_SAMPLES").hide();
        //     $("#SS_WRITED_SIZE").hide();
        }
    }

    SM.setDACSize = function(param) {
        var value = param["MM_DAC_SIZE"].value;
        var max = param["MM_DAC_SIZE"].max;

        $("#DAC_SIZE_TITLE").html("DAC: " + SM.calcSize(value))
        $("#MM_DAC_SIZE").prop('max',max)
        $("#MM_DAC_SIZE").prop('value',value)
    }

    SM.setDACFile = function(param) {
        var value = param["SS_DAC_FILE"].value;
        var itm = value.split("*")
        $("#SS_DAC_FILE").empty();
        if (itm.length > 1){
            var list = itm[1].trim().split("\n")
            list.forEach(element => {
                var new_row = document.createElement('option');
                new_row.setAttribute("value",element)
                new_row.innerText = element
                document.getElementById('SS_DAC_FILE').appendChild(new_row);
            });
            if (itm[0] == "" && list.length > 0){
                if (list[0] !== ""){
                    CLIENT.parametersCache["SS_DAC_FILE"] = { value: list[0] };
                    CLIENT.sendParameters();
                    return;
                }
            }

            $("#SS_DAC_FILE").val(itm[0])
        }
        else{
            $("#SS_DAC_FILE").val("")
        }
    }

    SM.setDACFileType = function(param) {
        var value = param["SS_DAC_FILE_TYPE"].value;
        $("#SS_DAC_FILE_TYPE").val(value)
    }

    SM.setDACRepeatMode = function(param) {
        var value = param["SS_DAC_REPEAT"].value;
        $("#SS_DAC_REPEAT").val(value)
    }

    SM.setDACRepeatCount = function(param) {
        var value = param["SS_DAC_REPEAT_COUNT"].value;
        $("#SS_DAC_REPEAT_COUNT").val(value)
    }

    SM.setDACHz = function(param) {
        var value = param["SS_DAC_HZ"].value;
        var min = param["SS_DAC_HZ"].min;
        var max = param["SS_DAC_HZ"].max;
        $("#SS_DAC_HZ").prop('min',min)
        $("#SS_DAC_HZ").prop('max',max)
        $("#SS_DAC_HZ").prop('value',value)
    }

    SM.setISDACGain = function(param) {
        var value = param["SS_IS_DAC_GAIN"].value;
        if (SM.is_dac_gain == undefined){
            SM.is_dac_gain = value;
            if (!value)
                $(".dac_gain").hide();
        }
    }

    SM.setDACGain = function(param) {
        var value = param["SS_DAC_GAIN"].value;
        $("#SS_CH1_DAC_GAIN").prop('checked', value & 0x1);
        $("#SS_CH2_DAC_GAIN").prop('checked', value & 0x2);
    }

    SM.setDACValid = function(param) {
        var value = param["MM_DAC_VALID"].value;
        if (value == false)
            $("#DAC_SIZE_TITLE").addClass('red')
        else
            $("#DAC_SIZE_TITLE").removeClass('red')
    }

    SM.setGPIOSize = function(param) {
        var value = param["MM_GPIO_SIZE"].value;
        var max = param["MM_GPIO_SIZE"].max;

        $("#GPIO_SIZE_TITLE").html("GPIO: " + SM.calcSize(value))
        $("#MM_GPIO_SIZE").prop('max',max)
        $("#MM_GPIO_SIZE").prop('value',value)
    }

    SM.setGPIOValid = function(param) {
        var value = param["MM_GPIO_VALID"].value;
        if (value == false)
            $("#GPIO_SIZE_TITLE").addClass('red')
        else
            $("#GPIO_SIZE_TITLE").removeClass('red')
    }

    SM.param_callbacks["SS_STATUS"] = SM.change_status;
    SM.param_callbacks["SS_DAC_STATUS"] = SM.change_dac_status;
    SM.param_callbacks["SS_GPIO_STATUS"] = SM.change_gpio_status;
    SM.param_callbacks["SS_ADC_DATA_PASS"] = SM.change_adc_data_pass;
    SM.param_callbacks["SS_DAC_DATA_PASS"] = SM.change_dac_data_pass;
    SM.param_callbacks["SS_IS_MASTER"] = SM.setBoardMode;
    SM.param_callbacks["SS_RATE"] = SM.setRate;
    SM.param_callbacks["SS_SAVE_MODE"] = SM.setSaveMode;
    SM.param_callbacks["SS_USE_CALIB"] = SM.setCalibration;
    SM.param_callbacks["SS_USE_FILE"] = SM.setMode;
    SM.param_callbacks["SS_SAMPLES"] = SM.setSamples;
    SM.param_callbacks["SS_FORMAT"] = SM.setFormat;
    SM.param_callbacks["SS_CHANNEL"] = SM.setChannel;
    SM.param_callbacks["SS_ATTENUATOR"] = SM.setAttenuator;
    SM.param_callbacks["SS_AC_DC"] = SM.setACDC;
    SM.param_callbacks["SS_ADC_CHANNELS"] = SM.setADCChannel;
    SM.param_callbacks["SS_DAC_CHANNELS"] = SM.setDACChannel;
    SM.param_callbacks["SS_IS_AC_DC"] = SM.setISACDC;
    SM.param_callbacks["SS_RESOLUTION"] = SM.setResolution;
    SM.param_callbacks["SS_PASS_SAMPLES"] = SM.setSamplesCount;
    SM.param_callbacks["SS_WRITED_SIZE"] = SM.setWritedSize;

    SM.param_callbacks["SS_DAC_MODE"] = SM.setDACMode;
    SM.param_callbacks["SS_DAC_FILE_TYPE"] = SM.setDACFileType;
    SM.param_callbacks["SS_DAC_FILE"] = SM.setDACFile;
    SM.param_callbacks["SS_DAC_GAIN"] = SM.setDACGain;
    SM.param_callbacks["SS_DAC_REPEAT"] = SM.setDACRepeatMode;
    SM.param_callbacks["SS_DAC_REPEAT_COUNT"] = SM.setDACRepeatCount;


    SM.param_callbacks["SS_DAC_HZ"] = SM.setDACHz;
    SM.param_callbacks["SS_IS_DAC_GAIN"] = SM.setISDACGain;


    SM.param_callbacks["MM_BLOCK_SIZE"] = SM.setBlockSize;
    SM.param_callbacks["MM_ADC_SIZE"] = SM.setADCSize;
    SM.param_callbacks["MM_ADC_VALID"] = SM.setADCValid;
    SM.param_callbacks["MM_DAC_SIZE"] = SM.setDACSize;
    SM.param_callbacks["MM_DAC_VALID"] = SM.setDACValid;
    SM.param_callbacks["MM_GPIO_SIZE"] = SM.setGPIOSize;
    SM.param_callbacks["MM_GPIO_VALID"] = SM.setGPIOValid;



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
        CLIENT.parametersCache["SS_START"] = { value: true };
        CLIENT.sendParameters();
        SM.ss_status_last = 0;
    });

    //Stop button
    $('#SM_STOP').on('click', function(ev) {
        ev.preventDefault();
        CLIENT.parametersCache["SS_START"] = { value: false };
        CLIENT.sendParameters();
    });

    //Run button
    $('#SM_DAC_RUN').on('click', function(ev) {
        CLIENT.parametersCache["SS_DAC_START"] = { value: true };
        CLIENT.sendParameters();
        SM.ss_dac_status_last = 0;
    });

    //Stop button
    $('#SM_DAC_STOP').on('click', function(ev) {
        ev.preventDefault();
        CLIENT.parametersCache["SS_DAC_START"] = { value: false };
        CLIENT.sendParameters();
    });

    //Run button
    $('#SM_GPIO_RUN').on('click', function(ev) {
        CLIENT.parametersCache["SS_GPIO_START"] = { value: true };
        CLIENT.sendParameters();
        SM.ss_gpio_status_last = 0;
    });

    //Stop button
    $('#SM_GPIO_STOP').on('click', function(ev) {
        ev.preventDefault();
        CLIENT.parametersCache["SS_GPIO_START"] = { value: false };
        CLIENT.sendParameters();
    });

    $('#SM_DAC_LOAD').click(function() {
        promptFile().then(function(file) {
            if(file){
                const fileReader = new FileReader(); // initialize the object
                fileReader.readAsArrayBuffer(file); // read file as array buffer
                fileReader.onload = (event) => {
                    console.log('Complete File read successfully!')
                    $.ajax({
                        url: '/upload_dac_file' + '?file=' + file.name, //Server script to process data
                        type: 'POST',
                        //Ajax events
                        //beforeSend: beforeSendHandler,
                        success: function(e) {
                            console.log("Upload done " + file.name);
                            setTimeout(() => {
                                CLIENT.parametersCache["SS_DAC_FILE_CTR"] = { value: 1 };
                                CLIENT.sendParameters();
                            }, 1000);
                        },
                        error: function(e) { console.log(e); },
                        // Form data
                        data: event.target.result,
                        //Options to tell jQuery not to process data or worry about content-type.
                        cache: false,
                        contentType: false,
                        processData: false
                    });
                }
            }
            else
                console.log("no file selected")
        });
    });

    $('#SM_DAC_DELETE').click(function() {
        CLIENT.parametersCache["SS_DAC_FILE_CTR"] = { value: 2 };
        CLIENT.sendParameters();
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
        CLIENT.sendParameters();
    }).resize();

    //Crash buttons
    $('#send_report_btn').on('click', function() { SM.formEmail() });
    $('#restart_app_btn').on('click', function() { location.reload() });

    $('#CLEAR_FILES').click(SM.DeleteFiles);


});