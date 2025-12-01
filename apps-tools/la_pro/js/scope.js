/*
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

(function(LA, $, undefined) {
    LA.param_callbacks = {};


    LA.triggers_list = [0, 0, 0, 0, 0, 0, 0, 0];

    // App state
    LA.state = {
        editing: false,
        fine: false
    };

    LA.guiHandler = function() {
        if (CLIENT.signalStack.length > 0) {
            let signals = Object.assign({}, CLIENT.signalStack[0]);
            for (const property in signals) {
                if (signals[property]['type']){
                    if (signals[property]['type'] == 'h'){
                        signals[property]['value'] = CLIENT.base64ToByteArray(signals[property]['value'])
                    }
                }
            }
            CLIENT.client_log(signals)
            var needRedraw = false
            if (signals['data_rle']){
                LA.lastData = signals
                LA.lastDataRepacked = COMMON.repackSignals(LA.lastData)
                needRedraw = true
            }
            for(var ch = 1; ch <= 4; ch++){
                if (signals['DECODER_SIGNAL_' + ch]){
                    var name = LA.getDecoderName(ch)
                    LA.decodedData[ch] = {name : name, values: LA.repackDecodedData(signals['DECODER_SIGNAL_' + ch],ch)}
                    needRedraw = true
                }
            }

            CLIENT.signalStack.splice(0, 1);
            LA.setupDataToBufferGraph()
            LA.setupDataToGraph()
            LOGGER.setupDataToLogger()
            if (needRedraw)
                LA.drawAllSeries()
        }
    }


    LA.processParameters = function(new_params) {
        var requestRedecode = false;
        // First init after reset
        if (CLIENT.getValue("LA_MAX_FREQ") == undefined){
            LA.initCursors()
            requestRedecode = true
        }

        if (new_params['LA_MAX_FREQ'])
            COMMON.updateMaxFreq(new_params['LA_MAX_FREQ'].value)

        if (new_params['LA_CUR_FREQ']) // Need set before other parameters
            LA.setCurrentFreq(new_params['LA_CUR_FREQ'].value)

        for (var param_name in new_params) {
            CLIENT.params.orig[param_name] = new_params[param_name];
            if (LA.param_callbacks[param_name] !== undefined)
                LA.param_callbacks[param_name](new_params);
        }

        // Need resend decoded data after first init UI
        if (requestRedecode){
            LA.requestRedecode()
        }
    };

    // Exits from editing mode
    LA.exitEditing = function(noclose) {

        var key_for_send = ['LA_DECIMATE','LA_PRE_TRIGGER_BUFFER_MS','LA_POST_TRIGGER_BUFFER_MS','LA_WIN_SHOW','LA_CUR_FREQ']

        for (var key in CLIENT.params.orig) {
            if (!(key_for_send.includes(key)))
                continue
            var field = $('#' + key);
            var value = undefined;

            if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
                value = field.val();
            } else if (field.is('button')) {
                value = (field.hasClass('active') ? 1 : 0);
            } else if (field.is('input:radio')) {
                value = $('input[name="' + key + '"]:checked').val();
            }

            if (value !== undefined && value != CLIENT.params.orig[key].value) {
                CLIENT.client_log(key + ' changed from ' + CLIENT.params.orig[key].value + ' to ' + ($.type(CLIENT.params.orig[key].value) == 'boolean' ? !!value : value));
                CLIENT.parametersCache[key] = {
                    value: ($.type(CLIENT.params.orig[key].value) == 'boolean' ? !!value : value)
                };
                CLIENT.params.orig[key] = CLIENT.parametersCache[key]
                CLIENT.sendParameters();
            }
        }

        // Send params then reset editing state and hide dialog

        LA.state.editing = false;
        if (noclose) return;
        $('.dialog:visible').hide();
        $('#right_menu').show();
    };

    LA.show_step = function(new_params) {
        step = new_params["LA_MEASURE_STATE"].value;
        if (step === 1){
            $("#STATUS_MSG").text("CLICK RUN TO START").css('color', '#f00');
        }

        if (step === 2){
            $("#STATUS_MSG").text("WAITING").css('color', '#ff0');
        }

        if (step === 3){
            $("#STATUS_MSG").text("DONE").css('color', '#0f0');
        }

        if (step === 4){
            $("#STATUS_MSG").text("TRIGGER TIMEOUT").css('color', '#f00');
        }
    }


    LA.param_callbacks["LA_RUN"] = LA.processRun;

    LA.param_callbacks["LA_CUR_FREQ"] = LA.setCurrentFreq;
    LA.param_callbacks["LA_DECIMATE"] = LA.setDecimation;
    LA.param_callbacks["LA_SCALE"] = LA.setTimeScale;
    LA.param_callbacks["LA_VIEW_PORT_POS"] = LA.setViewPortPos;

    LA.param_callbacks["LA_PRE_TRIGGER_BUFFER_MS"] = LA.setPresample;
    LA.param_callbacks["LA_POST_TRIGGER_BUFFER_MS"] = LA.setPostsample;
    LA.param_callbacks["CONTROL_CONFIG_SETTINGS"] = LA.setControlConfig;
    LA.param_callbacks["LIST_FILE_SATTINGS"] = LA.listSettings;

    LA.param_callbacks["LA_PRE_TRIGGER_SAMPLES"] = LA.setPreTriggerCountCaptured;
    LA.param_callbacks["LA_POST_TRIGGER_SAMPLES"] = LA.setPostTriggerCountCaptured;
    LA.param_callbacks["LA_TOTAL_SAMPLES"] = LA.setSampleCountCaptured;

    LA.param_callbacks["LA_MEASURE_MODE"] = LA.updateMeasureMode;
    LA.param_callbacks["LA_DISPLAY_RADIX"] = LA.updateDisplayRadix;

    LA.param_callbacks["LA_DIN_1"] = LA.setDIN1Enabled;
    LA.param_callbacks["LA_DIN_2"] = LA.setDIN2Enabled;
    LA.param_callbacks["LA_DIN_3"] = LA.setDIN3Enabled;
    LA.param_callbacks["LA_DIN_4"] = LA.setDIN4Enabled;
    LA.param_callbacks["LA_DIN_5"] = LA.setDIN5Enabled;
    LA.param_callbacks["LA_DIN_6"] = LA.setDIN6Enabled;
    LA.param_callbacks["LA_DIN_7"] = LA.setDIN7Enabled;
    LA.param_callbacks["LA_DIN_8"] = LA.setDIN8Enabled;


    LA.param_callbacks["LA_DIN_NAME_1"] = LA.setDIN1Name;
    LA.param_callbacks["LA_DIN_NAME_2"] = LA.setDIN2Name;
    LA.param_callbacks["LA_DIN_NAME_3"] = LA.setDIN3Name;
    LA.param_callbacks["LA_DIN_NAME_4"] = LA.setDIN4Name;
    LA.param_callbacks["LA_DIN_NAME_5"] = LA.setDIN5Name;
    LA.param_callbacks["LA_DIN_NAME_6"] = LA.setDIN6Name;
    LA.param_callbacks["LA_DIN_NAME_7"] = LA.setDIN7Name;
    LA.param_callbacks["LA_DIN_NAME_8"] = LA.setDIN8Name;

    LA.param_callbacks["LA_DIN_1_TRIGGER"] = LA.setDIN1Trigger;
    LA.param_callbacks["LA_DIN_2_TRIGGER"] = LA.setDIN2Trigger;
    LA.param_callbacks["LA_DIN_3_TRIGGER"] = LA.setDIN3Trigger;
    LA.param_callbacks["LA_DIN_4_TRIGGER"] = LA.setDIN4Trigger;
    LA.param_callbacks["LA_DIN_5_TRIGGER"] = LA.setDIN5Trigger;
    LA.param_callbacks["LA_DIN_6_TRIGGER"] = LA.setDIN6Trigger;
    LA.param_callbacks["LA_DIN_7_TRIGGER"] = LA.setDIN7Trigger;
    LA.param_callbacks["LA_DIN_8_TRIGGER"] = LA.setDIN8Trigger;

    LA.param_callbacks["LA_DIN_1_POS"] = LA.setDIN1Pos;
    LA.param_callbacks["LA_DIN_2_POS"] = LA.setDIN2Pos;
    LA.param_callbacks["LA_DIN_3_POS"] = LA.setDIN3Pos;
    LA.param_callbacks["LA_DIN_4_POS"] = LA.setDIN4Pos;
    LA.param_callbacks["LA_DIN_5_POS"] = LA.setDIN5Pos;
    LA.param_callbacks["LA_DIN_6_POS"] = LA.setDIN6Pos;
    LA.param_callbacks["LA_DIN_7_POS"] = LA.setDIN7Pos;
    LA.param_callbacks["LA_DIN_8_POS"] = LA.setDIN8Pos;

    LA.param_callbacks["DECODER_1"] = LA.setBUS1Settings;
    LA.param_callbacks["DECODER_2"] = LA.setBUS2Settings;
    LA.param_callbacks["DECODER_3"] = LA.setBUS3Settings;
    LA.param_callbacks["DECODER_4"] = LA.setBUS4Settings;

    LA.param_callbacks["DECODER_ENABLED_1"] = LA.setBUS1Enabled;
    LA.param_callbacks["DECODER_ENABLED_2"] = LA.setBUS2Enabled;
    LA.param_callbacks["DECODER_ENABLED_3"] = LA.setBUS3Enabled;
    LA.param_callbacks["DECODER_ENABLED_4"] = LA.setBUS4Enabled;

    LA.param_callbacks["DECODER_DEF_UART"] = LA.setDefSetUART;
    LA.param_callbacks["DECODER_DEF_CAN"] =  LA.setDefSetCAN;
    LA.param_callbacks["DECODER_DEF_SPI"] =  LA.setDefSetSPI;
    LA.param_callbacks["DECODER_DEF_I2C"] =  LA.setDefSetI2C;

    LA.param_callbacks["DECODER_ANNOTATION_UART"] = SERIES.setAnnoSetUART;
    LA.param_callbacks["DECODER_ANNOTATION_CAN"] =  SERIES.setAnnoSetCAN;
    LA.param_callbacks["DECODER_ANNOTATION_SPI"] =  SERIES.setAnnoSetSPI;
    LA.param_callbacks["DECODER_ANNOTATION_I2C"] =  SERIES.setAnnoSetI2C;

    LA.param_callbacks["LA_CURSOR_X1"] = LA.cursorX;
    LA.param_callbacks["LA_CURSOR_X2"] = LA.cursorX;
    LA.param_callbacks["LA_CURSOR_X1_POS"] = LA.cursorX;
    LA.param_callbacks["LA_CURSOR_X2_POS"] = LA.cursorX;

    LA.param_callbacks["LA_MEASURE_STATE"] = LA.show_step;

    LA.param_callbacks["RP_SYSTEM_CPU_LOAD"] = LA.setCpu;
    LA.param_callbacks["RP_SYSTEM_TEMPERATURE"] = LA.setCpuTemp;
    LA.param_callbacks["RP_SYSTEM_FREE_RAM"] = LA.setFreeRAM;
    LA.param_callbacks["RP_SYSTEM_TOTAL_RAM"] = LA.setTotalRAM;


    LA.param_callbacks["LA_WIN_SHOW"] = LA.setWinShow;
    LA.param_callbacks["LA_WIN_X"] = LA.setWinX;
    LA.param_callbacks["LA_WIN_Y"] = LA.setWinY;
    LA.param_callbacks["LA_WIN_W"] = LA.setWinW;
    LA.param_callbacks["LA_WIN_H"] = LA.setWinH;

    LA.param_callbacks["LA_LOGGER_BUS_1"] = LOGGER.setLoggerBUS1;
    LA.param_callbacks["LA_LOGGER_BUS_2"] = LOGGER.setLoggerBUS2;
    LA.param_callbacks["LA_LOGGER_BUS_3"] = LOGGER.setLoggerBUS3;
    LA.param_callbacks["LA_LOGGER_BUS_4"] = LOGGER.setLoggerBUS4;
    LA.param_callbacks["LA_LOGGER_RADIX"] = LOGGER.setLoggerRadix;


    LA.g_CpuLoad = 100.0;
    LA.g_CpuTemp = 100.0;
    LA.g_TotalMemory = 256.0;
    LA.g_FreeMemory = 256.0;

    setInterval(function() {
        $('#cpu_load').text(LA.g_CpuLoad.toFixed(2) + "%");
        $('#cpu_temp').text(LA.g_CpuTemp.toFixed(0));
        $('#totalmem_view').text((LA.g_TotalMemory / (1024 * 1024)).toFixed(2) + "MB");
        $('#freemem_view').text((LA.g_FreeMemory / (1024 * 1024)).toFixed(2) + "MB");
        $('#usagemem_view').text(((LA.g_TotalMemory - LA.g_FreeMemory) / (1024 * 1024)).toFixed(2) + "MB");
    }, 1000);


}(window.LA = window.LA || {}, jQuery));

// Page onload event handler
$(function() {

    LA.initGraph()
    LA.initHandlers()
    LA.drawGraphGrid();
    LA.initGraphBuffer();
    LA.initSubWindow()
});