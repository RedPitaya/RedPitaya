/*
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


(function(LA, $, undefined) {


    LA.setDecimation = function(param) {
        var param_name = "LA_DECIMATE"
        var field = $('#' + param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.val(param[param_name].value);
        }
    }

    LA.setPresample = function(param) {
        var param_name = "LA_PRE_TRIGGER_BUFFER_MS"
        var field = $('#' + param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.attr('max',param[param_name].max);
            field.val(param[param_name].value);
        }
    }

    LA.setPostsample = function(param) {
        var param_name = "LA_POST_TRIGGER_BUFFER_MS"
        var field = $('#' + param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.attr('max',param[param_name].max);
            field.val(param[param_name].value);
        }
    }


    LA.setDINEnabled = function(param,idx,ch) {
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
        LA.drawAllSeries()
    }

    LA.setBUSEnabled = function(param,idx,ch) {
        var param_name = "DECODER_ENABLED_"+idx
        var el = "BUS"+idx+"_ENABLED"
        var img = $("#"+el).find('img');
        if (param[param_name].value){
            img.show()
        }else{
            img.hide()
        }
        LA.updateUIFromConfig()
        LA.drawAllSeries()
    }

    LA.setDINName= function(param,idx) {
        var param_name = "LA_DIN_NAME_"+idx
        var el = "CH"+idx+"_NAME"
        var field = $('#' + el);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.val(param[param_name].value);
            var arrow = $('#ch' + idx + '_offset_arrow');
            arrow.find('#CH' + idx + '_LABEL').text(param[param_name].value);
        }
    }

    LA.setDIN1Enabled = function(param) {
        LA.setDINEnabled(param,"1",0)
    }

    LA.setDIN2Enabled = function(param) {
        LA.setDINEnabled(param,"2",1)
    }

    LA.setDIN3Enabled = function(param) {
        LA.setDINEnabled(param,"3",2)
    }

    LA.setDIN4Enabled = function(param) {
        LA.setDINEnabled(param,"4",3)
    }

    LA.setDIN5Enabled = function(param) {
        LA.setDINEnabled(param,"5",4)
    }

    LA.setDIN6Enabled = function(param) {
        LA.setDINEnabled(param,"6",5)
    }

    LA.setDIN7Enabled = function(param) {
        LA.setDINEnabled(param,"7",6)
    }

    LA.setDIN8Enabled = function(param) {
        LA.setDINEnabled(param,"8",7)
    }

    LA.setBUS1Enabled = function(param) {
        LA.setBUSEnabled(param,"1",0)
    }

    LA.setBUS2Enabled = function(param) {
        LA.setBUSEnabled(param,"2",1)
    }

    LA.setBUS3Enabled = function(param) {
        LA.setBUSEnabled(param,"3",2)
    }

    LA.setBUS4Enabled = function(param) {
        LA.setBUSEnabled(param,"4",3)
    }

    LA.setDIN1Name = function(param) {
        LA.setDINName(param,"1")
    }

    LA.setDIN2Name = function(param) {
        LA.setDINName(param,"2")
    }

    LA.setDIN3Name = function(param) {
        LA.setDINName(param,"3")
    }

    LA.setDIN4Name = function(param) {
        LA.setDINName(param,"4")
    }

    LA.setDIN5Name = function(param) {
        LA.setDINName(param,"5")
    }

    LA.setDIN6Name = function(param) {
        LA.setDINName(param,"6")
    }

    LA.setDIN7Name = function(param) {
        LA.setDINName(param,"7")
    }

    LA.setDIN8Name = function(param) {
        LA.setDINName(param,"8")
    }

    LA.setDIN1Trigger = function(param){
        LA.setDINTrigger(param,1)
    }

    LA.setDIN2Trigger = function(param){
        LA.setDINTrigger(param,2)
    }

    LA.setDIN3Trigger = function(param){
        LA.setDINTrigger(param,3)
    }

    LA.setDIN4Trigger = function(param){
        LA.setDINTrigger(param,4)
    }

    LA.setDIN5Trigger = function(param){
        LA.setDINTrigger(param,5)
    }

    LA.setDIN6Trigger = function(param){
        LA.setDINTrigger(param,6)
    }

    LA.setDIN7Trigger = function(param){
        LA.setDINTrigger(param,7)
    }

    LA.setDIN8Trigger = function(param){
        LA.setDINTrigger(param,8)
    }

    LA.setDIN1Pos = function(param){
        LA.updateChVisibility(0)
        LA.setupDataToGraph()
        LA.drawAllSeries()
    }

    LA.setDIN2Pos = function(param){
        LA.updateChVisibility(1)
        LA.setupDataToGraph()
        LA.drawAllSeries()
    }

    LA.setDIN3Pos = function(param){
        LA.updateChVisibility(2)
        LA.setupDataToGraph()
        LA.drawAllSeries()
    }

    LA.setDIN4Pos = function(param){
        LA.updateChVisibility(3)
        LA.setupDataToGraph()
        LA.drawAllSeries()
    }

    LA.setDIN5Pos = function(param){
        LA.updateChVisibility(4)
        LA.setupDataToGraph()
        LA.drawAllSeries()
    }

    LA.setDIN6Pos = function(param){
        LA.updateChVisibility(5)
        LA.setupDataToGraph()
        LA.drawAllSeries()
    }

    LA.setDIN7Pos = function(param){
        LA.updateChVisibility(6)
        LA.setupDataToGraph()
        LA.drawAllSeries()
    }

    LA.setDIN8Pos = function(param){
        LA.updateChVisibility(7)
        LA.setupDataToGraph()
        LA.drawAllSeries()
    }

    LA.enableChannelDIN = function() {
        var element = $(this);
        var img = $(this).find('img');
        var arr = ["CH1_ENABLED", "CH2_ENABLED", "CH3_ENABLED", "CH4_ENABLED", "CH5_ENABLED", "CH6_ENABLED", "CH7_ENABLED", "CH8_ENABLED"];
        var ch = arr.indexOf(element.attr('id'));

        if (img.is(":visible")) {
            if (ch != -1) {
                CLIENT.parametersCache["LA_DIN_"+(ch+1)] = { value: false };
            }
        } else {
            if (ch != -1) {
                CLIENT.parametersCache["LA_DIN_"+(ch+1)] = { value: true };
            }
        }
        CLIENT.sendParameters();
    }

    LA.enableChannelBUS = function() {
        var element = $(this);
        var img = $(this).find('img');
        var bus_array = ["BUS1_ENABLED", "BUS2_ENABLED", "BUS3_ENABLED", "BUS4_ENABLED"];
        var ch = bus_array.indexOf(element.attr('id'));

        if (img.is(":visible")) {
            if (ch != -1) {
                CLIENT.parametersCache["DECODER_ENABLED_"+(ch+1)] = { value: false };
            }
        } else {
            if (ch != -1) {
                CLIENT.parametersCache["DECODER_ENABLED_"+(ch+1)] = { value: true };
            }
        }
        CLIENT.sendParameters();
    }

    LA.sendAllTrig = function() {

        for (var i = 0; i < LA.triggers_list.length; i++) {
            CLIENT.parametersCache["LA_DIN_"+(i+1)+"_TRIGGER"] = { value: LA.triggers_list[i] }
        }
        CLIENT.sendParameters();
    }

    LA.setDINTrigger= function(param,idx) {
        var param_name = "LA_DIN_" + idx + "_TRIGGER"
        $('.trigger_type[name=\'din' + (idx - 1) + '\']').val(param[param_name].value);
        LA.updateChVisibility(idx - 1)
    }



    LA.sendTriggerFromUI = function(element) {
        var chan_num = parseInt($(element).attr('name')[3]);
        var trig_type = parseInt($(element).val());

        var I = 0;
        var L = 1;
        var H = 2;
        var R = 3;
        var F = 4;
        var E = 5;

        // Set trigger
        LA.triggers_list[chan_num] = trig_type;

        // If trigger type is R/F/E delete other same triggers
        if (trig_type == R || trig_type == F || trig_type == E) {
            for (var i = 0; i < LA.triggers_list.length; i++) {
                if (i != chan_num && (LA.triggers_list[i] == R || LA.triggers_list[i] == F || LA.triggers_list[i] == E)) {
                    LA.triggers_list[i] = I;
                    $('.trigger_type[name=\'din' + i + '\']').val(I);
                }
            }
        }

        LA.sendAllTrig();
    }

    LA.updateMeasureMode = function(params) {
        if (params["LA_MEASURE_MODE"].value == 1) {
            $('#stem').text('');
            $('#select_mode').html('Ext. MODULE');
        } else {
            $('#stem').text('Ext. module used');
            $('#select_mode').html('&check; Ext. MODULE');
        }
    }

    LA.setCurrentFreq = function(new_params) {
        // Calculate time per division
        var samplerate = CLIENT.getValue("LA_CUR_FREQ")
        var scale = CLIENT.getValue("LA_SCALE")
        if (samplerate !== undefined && scale !== undefined){
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
        LA.updateTimeScale()
    }

    LA.updateTimeScale = function(){
        var samplerate = CLIENT.getValue("LA_CUR_FREQ")
        var plot = LA.getPlot()
        if (plot !== undefined && samplerate !== undefined){
            var axes = plot.getAxes();
            var mul = 1000;
            var dev_num = 10;
            var plot_samples = axes.xaxis.max - axes.xaxis.min
            var timePerWInMs = ((plot_samples  / samplerate) * mul) / dev_num;
            $('#LA_TIME_SCALE').text(COMMON.convertTime(timePerWInMs));
        }
    }

    LA.updateDisplayRadix = function(new_params){
        $('#DISPLAY_RADIX').val(new_params['LA_DISPLAY_RADIX'].value)
        COMMON.radix = new_params['LA_DISPLAY_RADIX'].value
        LA.drawAllSeries()
    }

    LA.setTimeScale = function(param){
        LA.updateTimeScale()
        LA.setCurrentFreq(param)
        LA.updateXInfo()
        LA.updatePositionBufferViewport()
    }

    LA.processRun = function(new_params) {
        var run = CLIENT.getValue("LA_RUN")
        if (run !== undefined){
            if (run === 1) {
                $('#LA_RUN').hide();
                $('#LA_STOP').css('display', 'block');
            }
            if (run === 0) {
                $('#LA_STOP').hide();
                $('#LA_RUN').show();
            }
        }
    }

    LA.setPreTriggerCountCaptured = function(){
        LA.setupDataToBufferGraph()
        LA.setupDataToGraph()
        LOGGER.loadDecoderValues()
    }

    LA.setPostTriggerCountCaptured = function(){ }

    LA.setSampleCountCaptured = function(param){
        LA.resizeAxisGraphBufferFromCount(param['LA_TOTAL_SAMPLES'].value)
        LA.setupDataToBufferGraph()
        LA.setupDataToGraph()
        LOGGER.loadDecoderValues()
        LA.updatePositionBufferViewport()
    }

    LA.setViewPortPos = function(param) {
        CLIENT.params.orig['LA_VIEW_PORT_POS'].value = parseFloat(CLIENT.params.orig['LA_VIEW_PORT_POS'].value)
        LA.setupDataToBufferGraph()
        LA.setupDataToGraph()
        LOGGER.loadDecoderValues()
        LA.updatePositionBufferViewport()
    }

    LA.setControlConfig = function(new_params) {
        if (new_params['CONTROL_CONFIG_SETTINGS'].value === 2) {  // RESET_DONE
            location.reload();
        }

        if (new_params['CONTROL_CONFIG_SETTINGS'].value === 7) {  // LOAD_DONE
            location.reload();
        }
    }

    LA.setCpu = function(new_params){
        LA.g_CpuLoad = new_params['RP_SYSTEM_CPU_LOAD'].value
    }

    LA.setCpuTemp = function(new_params){
        LA.g_CpuTemp = new_params['RP_SYSTEM_TEMPERATURE'].value
    }

    LA.setFreeRAM = function(new_params){
        LA.g_FreeMemory = new_params['RP_SYSTEM_FREE_RAM'].value
    }

    LA.setTotalRAM = function(new_params){
        LA.g_TotalMemory = new_params['RP_SYSTEM_TOTAL_RAM'].value
    }

    LA.listSettings = function(new_params){
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

}(window.LA = window.LA || {}, jQuery));
