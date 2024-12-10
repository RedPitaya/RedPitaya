/*
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


(function(OSC, $, undefined) {


    OSC.setDecimation = function(param) {
        var param_name = "LA_DECIMATE"
        var field = $('#' + param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.val(param[param_name].value);
        }
    }

    OSC.setPresample = function(param) {
        var param_name = "LA_PRE_TRIGGER_BUFFER_MS"
        var field = $('#' + param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.attr('max',param[param_name].max);
            field.val(param[param_name].value);
        }
    }

    OSC.setPostsample = function(param) {
        var param_name = "LA_POST_TRIGGER_BUFFER_MS"
        var field = $('#' + param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.attr('max',param[param_name].max);
            field.val(param[param_name].value);
        }
    }


    OSC.setDINEnabled = function(param,idx,ch) {
        var param_name = "LA_DIN_"+idx
        var el = "CH"+idx+"_ENABLED"
        var img = $("#"+el).find('img');
        if (param[param_name].value){
            img.show()
            LA.updateChVisibility(ch)
        }else{
            img.hide()
            $('#ch' + (ch + 1) + '_offset_arrow').hide();
        }
        LA.setupDataToGraph()
    }

    OSC.setDINName= function(param,idx) {
        var param_name = "LA_DIN_NAME_"+idx
        var el = "CH"+idx+"_NAME"
        var field = $('#' + el);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.val(param[param_name].value);
            var arrow = $('#ch' + idx + '_offset_arrow');
            arrow.find('#CH' + idx + '_LABEL').text(param[param_name].value);
        }
    }

    OSC.setDIN1Enabled = function(param) {
        OSC.setDINEnabled(param,"1",0)
    }

    OSC.setDIN2Enabled = function(param) {
        OSC.setDINEnabled(param,"2",1)
    }

    OSC.setDIN3Enabled = function(param) {
        OSC.setDINEnabled(param,"3",2)
    }

    OSC.setDIN4Enabled = function(param) {
        OSC.setDINEnabled(param,"4",3)
    }

    OSC.setDIN5Enabled = function(param) {
        OSC.setDINEnabled(param,"5",4)
    }

    OSC.setDIN6Enabled = function(param) {
        OSC.setDINEnabled(param,"6",5)
    }

    OSC.setDIN7Enabled = function(param) {
        OSC.setDINEnabled(param,"7",6)
    }

    OSC.setDIN8Enabled = function(param) {
        OSC.setDINEnabled(param,"8",7)
    }

    OSC.setDIN1Name = function(param) {
        OSC.setDINName(param,"1")
    }

    OSC.setDIN2Name = function(param) {
        OSC.setDINName(param,"2")
    }

    OSC.setDIN3Name = function(param) {
        OSC.setDINName(param,"3")
    }

    OSC.setDIN4Name = function(param) {
        OSC.setDINName(param,"4")
    }

    OSC.setDIN5Name = function(param) {
        OSC.setDINName(param,"5")
    }

    OSC.setDIN6Name = function(param) {
        OSC.setDINName(param,"6")
    }

    OSC.setDIN7Name = function(param) {
        OSC.setDINName(param,"7")
    }

    OSC.setDIN8Name = function(param) {
        OSC.setDINName(param,"8")
    }

    OSC.setDIN1Trigger = function(param){
        OSC.setDINTrigger(param,1)
    }

    OSC.setDIN2Trigger = function(param){
        OSC.setDINTrigger(param,2)
    }

    OSC.setDIN3Trigger = function(param){
        OSC.setDINTrigger(param,3)
    }

    OSC.setDIN4Trigger = function(param){
        OSC.setDINTrigger(param,4)
    }

    OSC.setDIN5Trigger = function(param){
        OSC.setDINTrigger(param,5)
    }

    OSC.setDIN6Trigger = function(param){
        OSC.setDINTrigger(param,6)
    }

    OSC.setDIN7Trigger = function(param){
        OSC.setDINTrigger(param,7)
    }

    OSC.setDIN8Trigger = function(param){
        OSC.setDINTrigger(param,8)
    }

    OSC.setDIN1Pos = function(param){
        LA.updateChVisibility(0)
        LA.setupDataToGraph()
    }

    OSC.setDIN2Pos = function(param){
        LA.updateChVisibility(1)
        LA.setupDataToGraph()
    }

    OSC.setDIN3Pos = function(param){
        LA.updateChVisibility(2)
        LA.setupDataToGraph()
    }

    OSC.setDIN4Pos = function(param){
        LA.updateChVisibility(3)
        LA.setupDataToGraph()
    }

    OSC.setDIN5Pos = function(param){
        LA.updateChVisibility(4)
        LA.setupDataToGraph()
    }

    OSC.setDIN6Pos = function(param){
        LA.updateChVisibility(5)
        LA.setupDataToGraph()
    }

    OSC.setDIN7Pos = function(param){
        LA.updateChVisibility(6)
        LA.setupDataToGraph()
    }

    OSC.setDIN8Pos = function(param){
        LA.updateChVisibility(7)
        LA.setupDataToGraph()
    }

    OSC.enableChannel = function() {
        var element = $(this);
        var img = $(this).find('img');
        var arr = ["CH1_ENABLED", "CH2_ENABLED", "CH3_ENABLED", "CH4_ENABLED", "CH5_ENABLED", "CH6_ENABLED", "CH7_ENABLED", "CH8_ENABLED"];
        // var arr2 = ["BUS1_ENABLED", "BUS2_ENABLED", "BUS3_ENABLED", "BUS4_ENABLED"];
        var ch = arr.indexOf(element.attr('id'));
        // var bn = arr2.indexOf(element.attr('id'));
        // var inp = element.parent().parent().find('#CH' + (ch + 1) + "_NAME")

        if (img.is(":visible")) {
            // img.hide();
            if (ch != -1) {
                CLIENT.parametersCache["LA_DIN_"+(ch+1)] = { value: false };
            }

            // // TODO ON CHECK CHANGED
            // if (ch != -1) {
            //     OSC.enabled_channels[ch] = false;
            //     $('#ch' + (ch + 1) + '_offset_arrow').hide();
            // }
            // if (bn != -1) {
            //     if (OSC.buses["bus" + (bn + 1)].enabled !== undefined) {
            //         OSC.destroyDecoder("bus" + (bn + 1), "-");
            //         OSC.buses["bus" + (bn + 1)].enabled = false;
            //     }
            // }
        } else {
            // img.show();
            if (ch != -1) {
                CLIENT.parametersCache["LA_DIN_"+(ch+1)] = { value: true };
            }
            // if (ch != -1) {
            //     if (inp.val() == undefined || inp.val() == "")
            //         inp.val("DIN" + ch);
            //     OSC.enabled_channels[ch] = true;
            //     OSC.updateChVisibility(ch)
            //     img.show();
            // }
            // if (bn != -1) {
            //     if ($('#BUS' + (bn + 1) + '_NAME').text() == ('BUS' + bn)) {
            //         OSC.startEditBus('BUS' + (bn + 1) + '_SETTINGS');
            //     } else {
            //         var bus = "bus" + (bn + 1);
            //         if (OSC.buses[bus] != undefined && OSC.buses[bus].name != undefined) {
            //             switch (OSC.buses[bus].name) {
            //                 case "UART":
            //                     if (OSC.buses[bus].serial != -1 && !isChannelInUse(OSC.buses[bus].serial, bus)) {
            //                         OSC.params.local['CREATE_DECODER'] = {
            //                             value: 'uart'
            //                         };

            //                         OSC.buses[bus].decoder = 'uart' + OSC.state.decoder_id;
            //                         OSC.state.decoder_id++;
            //                         OSC.params.local['DECODER_NAME'] = {
            //                             value: OSC.buses[bus].decoder
            //                         };

            //                         OSC.sendParams();
            //                     } else
            //                         OSC.startEditBus('BUS' + (bn + 1) + '_SETTINGS');
            //                     break;
            //                 case "CAN":
            //                     if (OSC.buses[bus].serial != -1 && !isChannelInUse(OSC.buses[bus].serial, bus)) {
            //                         OSC.params.local['CREATE_DECODER'] = {
            //                             value: 'can'
            //                         };

            //                         OSC.buses[bus].decoder = 'can' + OSC.state.decoder_id;
            //                         OSC.state.decoder_id++;
            //                         OSC.params.local['DECODER_NAME'] = {
            //                             value: OSC.buses[bus].decoder
            //                         };

            //                         OSC.sendParams();
            //                     } else
            //                         OSC.startEditBus('BUS' + (bn + 1) + '_SETTINGS');
            //                     break;
            //                 case "I2C":
            //                     if (OSC.buses[bus].sda != -1 && !isChannelInUse(OSC.buses[bus].sda, bus)) {
            //                         if (OSC.buses[bus].scl != -1 && !isChannelInUse(OSC.buses[bus].scl, bus)) {
            //                             OSC.params.local['CREATE_DECODER'] = {
            //                                 value: 'i2c'
            //                             };
            //                             OSC.buses[bus].decoder = 'i2c' + OSC.state.decoder_id;
            //                             OSC.state.decoder_id++;
            //                             OSC.params.local['DECODER_NAME'] = {
            //                                 value: OSC.buses[bus].decoder
            //                             };

            //                             OSC.sendParams();
            //                         } else
            //                             OSC.startEditBus('BUS' + (bn + 1) + '_SETTINGS');
            //                     } else
            //                         OSC.startEditBus('BUS' + (bn + 1) + '_SETTINGS');

            //                     break;
            //                 case "SPI":
            //                     if (OSC.buses[bus].clk != -1 && !isChannelInUse(OSC.buses[bus].clk, bus)) {
            //                         var miso_ok = false,
            //                             mosi_ok = false,
            //                             cs_ok;

            //                         if (OSC.buses[bus].miso != -1 && !isChannelInUse(OSC.buses[bus].miso, bus)) {
            //                             miso_ok = true;
            //                         } else if (OSC.buses[bus].miso == -1)
            //                             miso_ok = true;

            //                         if (OSC.buses[bus].mosi != -1 && !isChannelInUse(OSC.buses[bus].mosi, bus)) {
            //                             mosi_ok = true;
            //                         } else if (OSC.buses[bus].mosi == -1)
            //                             mosi_ok = true;

            //                         if (OSC.buses[bus].cs != -1 && !isChannelInUse(OSC.buses[bus].cs, bus)) {
            //                             cs_ok = true;
            //                         } else if (OSC.buses[bus].cs == -1)
            //                             cs_ok = true;

            //                         if (miso_ok && mosi_ok && cs_ok) {
            //                             if (OSC.buses[bus].mosi != -1) {
            //                                 OSC.buses[bus].mosi_decoder = 'spi' + OSC.state.decoder_id;
            //                                 OSC.params.local['CREATE_DECODER'] = {
            //                                     value: 'spi'
            //                                 };
            //                                 OSC.params.local['DECODER_NAME'] = {
            //                                     value: 'spi' + OSC.state.decoder_id
            //                                 };
            //                                 OSC.state.decoder_id++;
            //                             }


            //                             if (OSC.buses[bus].miso != -1) {
            //                                 OSC.buses[bus].miso_decoder = 'spi' + OSC.state.decoder_id;
            //                                 OSC.params.local['CREATE_DECODER'] = {
            //                                     value: 'spi'
            //                                 };
            //                                 OSC.params.local['DECODER_NAME'] = {
            //                                     value: 'spi' + OSC.state.decoder_id
            //                                 };
            //                                 OSC.state.decoder_id++;
            //                             }
            //                             OSC.sendParams();

            //                         } else
            //                             OSC.startEditBus('BUS' + (bn + 1) + '_SETTINGS');
            //                     } else
            //                         OSC.startEditBus('BUS' + (bn + 1) + '_SETTINGS');
            //                     break;
            //             }
            //         }
            //         img.show();
            //     }
            // }
        }
        CLIENT.sendParameters();

        // OSC.guiHandler();
    }

    OSC.sendAllTrig = function() {

        for (var i = 0; i < OSC.triggers_list.length; i++) {
            CLIENT.parametersCache["LA_DIN_"+(i+1)+"_TRIGGER"] = { value: OSC.triggers_list[i] }
        }
        CLIENT.sendParameters();
    }

    OSC.setDINTrigger= function(param,idx) {
        var param_name = "LA_DIN_" + idx + "_TRIGGER"
        $('.trigger_type[name=\'din' + (idx - 1) + '\']').val(param[param_name].value);
    }



    OSC.sendTriggerFromUI = function(element) {
        var chan_num = parseInt($(element).attr('name')[3]);
        var trig_type = parseInt($(element).val());

        var I = 0;
        var L = 1;
        var H = 2;
        var R = 3;
        var F = 4;
        var E = 5;

        // Set trigger
        OSC.triggers_list[chan_num] = trig_type;

        // If trigger type is R/F/E delete other same triggers
        if (trig_type == R || trig_type == F || trig_type == E) {
            for (var i = 0; i < OSC.triggers_list.length; i++) {
                if (i != chan_num && (OSC.triggers_list[i] == R || OSC.triggers_list[i] == F || OSC.triggers_list[i] == E)) {
                    OSC.triggers_list[i] = I;
                    $('.trigger_type[name=\'din' + i + '\']').val(I);
                }
            }
        }

        OSC.sendAllTrig();
    }

    OSC.updateMeasureMode = function(params) {
        if (params["LA_MEASURE_MODE"].value == 1) {
            $('#stem').text('');
            $('#select_mode').html('Ext. MODULE');
        } else {
            $('#stem').text('Ext. module used');
            $('#select_mode').html('&check; Ext. MODULE');
        }
    }

    OSC.setCurrentFreq = function(new_params) {
        // Calculate time per division
        var samplerate = CLIENT.getValue("LA_CUR_FREQ")
        var scale = CLIENT.getValue("LA_SCALE")
        if (samplerate !== undefined && scale !== undefined){
            OSC.state.acq_speed = samplerate
            var graph_width = $('#graph_grid').outerWidth();
            var mul = 1000;
            var dev_num = 10;
            var timePerDevInMs = (((graph_width / scale) / samplerate) * mul) / dev_num;

            ms_per_px = (timePerDevInMs * 10) / graph_width;
            var new_value = OSC.trigger_position * ms_per_px;

            $('#LA_TIME_SCALE').text(OSC.convertTime(timePerDevInMs));

            const round = (n, dp) => {
                const h = +('1'.padEnd(dp + 1, '0')) // 10 or 100 or 1000 or etc
                return Math.round(n * h) / h
            }
            var suff = ''
            var orig_samplerate = samplerate
            if (orig_samplerate >= 1e6){
                suff = "M"
                samplerate = round(samplerate / 1e6,3)
            } else if (orig_samplerate >= 1e3){
                suff = "k"
                samplerate = round(samplerate / 1e3,3)
            }

            $('#LA_SAMPLE_RATE').text(samplerate + " " + suff + "S/s");
        }
        LA.updatePositionBufferViewport()
    }

    OSC.updateTimeScale = function(){

    }

    OSC.setTimeScale = function(param){
        OSC.updateTimeScale()
        OSC.setCurrentFreq(param)
        LA.updateXInfo()
        LA.updatePositionBufferViewport()
    }

    // Changes Y zoom/scale for the selected signal
    OSC.changeYZoom = function(direction, curr_scale, send_changes) {
        if (direction == '-') {
            if (OSC.voltage_index == 3)
                return;
            OSC.voltage_index++;
        } else {
            if (OSC.voltage_index == 0)
                return;
            OSC.voltage_index--;
        }
    };

    OSC.processRun = function(new_params) {
        var run = CLIENT.getValue("LA_RUN")
        if (run !== undefined){
            if (run === true) {
                $('#LA_RUN').hide();
                $('#LA_STOP').css('display', 'block');
            } else {
                $('#LA_STOP').hide();
                $('#LA_RUN').show();
            }
        }
    }

    OSC.setPreTriggerCountCaptured = function(){
        LA.setupDataToBufferGraph()
        LA.setupDataToGraph()
    }

    OSC.setPostTriggerCountCaptured = function(){

    }

    OSC.setSampleCountCaptured = function(param){
        LA.resizeAxisGraphBufferFromCount(param['LA_TOTAL_SAMPLES'].value)
        LA.updatePositionBufferViewport()
    }

    OSC.setViewPortPos = function(param) {
        LA.updatePositionBufferViewport()
    }

    OSC.setControlConfig = function(new_params) {
        if (new_params['CONTROL_CONFIG_SETTINGS'].value === 2) {  // RESET_DONE
            location.reload();
        }

        if (new_params['CONTROL_CONFIG_SETTINGS'].value === 7) {  // LOAD_DONE
            location.reload();
        }
    }

    OSC.setCpu = function(new_params){
        OSC.g_CpuLoad = new_params['RP_SYSTEM_CPU_LOAD'].value
    }

    OSC.setCpuTemp = function(new_params){
        OSC.g_CpuTemp = new_params['RP_SYSTEM_TEMPERATURE'].value
    }

    OSC.setFreeRAM = function(new_params){
        OSC.g_FreeMemory = new_params['RP_SYSTEM_FREE_RAM'].value
    }

    OSC.setTotalRAM = function(new_params){
        OSC.g_TotalMemory = new_params['RP_SYSTEM_TOTAL_RAM'].value
    }

    OSC.listSettings = function(new_params){
        var list = new_params['LIST_FILE_SATTINGS'].value
        const splitLines = value => value.split(/\r?\n/);
        $('#settings_dropdown').find('.saved_settings').remove();
        splitLines(list).forEach(function(item){
            var id = item.trim();
            if (id !== ""){
                var li = document.createElement('li')
                var a = document.createElement('a')
                var img = document.createElement('img')
                a.innerHTML = id
                li.appendChild(a)
                a.appendChild(img)
                li.classList.add("saved_settings");
                a.style.paddingLeft = "10px"
                a.style.paddingRight = "10px"
                img.src = "img/delete.png"
                a.setAttribute("file_name",id)
                a.onclick = function() {
                   CLIENT.parametersCache['FILE_SATTINGS'] = { value: $(this).attr('file_name') };
                   CLIENT.parametersCache['CONTROL_CONFIG_SETTINGS'] = { value: 6 }; // LOAD
                   CLIENT.sendParameters();
                };
                img.onclick = function(event) {
                    event.stopPropagation();
                    CLIENT.parametersCache['FILE_SATTINGS'] = { value: $(this).parent().attr('file_name') };
                    CLIENT.parametersCache['CONTROL_CONFIG_SETTINGS'] = { value: 5 }; // DELETE
                    CLIENT.sendParameters();
                };
                var r1 = document.getElementById('settings_dropdown');
                if (r1!= null)
                    r1.appendChild(li);

            }
        })
    }

}(window.OSC = window.OSC || {}, jQuery));
