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

(function(OSC, $, undefined) {
    OSC.param_callbacks = {};
    OSC.startTime = 0;
    OSC.config = {};


    OSC.redrawTimeout = undefined;
    OSC.latest_signal = {};

    // Time scale steps in millisecods
    OSC.time_steps = [
        // Nanoseconds
        100 / 1000000, 200 / 1000000, 500 / 1000000,
        // Microseconds
        1 / 1000, 2 / 1000, 5 / 1000, 10 / 1000, 20 / 1000, 50 / 1000, 100 / 1000, 200 / 1000, 500 / 1000,
        // Millisecods
        1, 2, 5, 10, 20, 50, 100, 200, 500,
        // Seconds
        1 * 1000, 2 * 1000, 5 * 1000, 10 * 1000, 20 * 1000, 50 * 1000
    ];

    // Voltage scale steps in minorgrid values
    OSC.voltage_steps = [0.2, 0.4, 0.8, 1.6];
    OSC.voltage_index = 2;
    OSC.counts_offset = 0;


    // OSC.ch_names = ["DIN0", "DIN1", "DIN2", "DIN3", "DIN4", "DIN5", "DIN6", "DIN7"];
    OSC.log_buses = [false, false, false, false];



    // OSC.triggers_list_id = [];
    // OSC.triggers_list_action = [];
    // OSC.triggers_list_channel = [];
    // OSC.triggers_count = 0;
    OSC.current_bus = "bus-1";

    OSC.triggers_list = [0, 0, 0, 0, 0, 0, 0, 0];

    OSC.loaderShow = false;
    // App state
    OSC.state = {
        socket_opened: false,
        processing: false,
        editing: false,
        trig_dragging: false,
        cursor_dragging: false,
        resized: false,
        sel_sig_name: 'ch1',
        fine: false,
        graph_grid_height: null,
        graph_grid_width: null,
        radix: 17,
        acq_speed: undefined,
        line_moving: false
    };


    OSC.recv_signals = {};

    OSC.scalesTable = [0.01, 0.05, 0.1, 0.3, 0.5, 0.6, 0.8, 1, 3, 5, 7, 8, 10, 15, 20, 25, 50, 100];

    // // Params cache
    // OSC.params = {
    //     orig: {},
    //     local: {}
    // };

    //  OSC.laMode = 0;
    OSC.measureMode = 0;

    // Other global variables
    OSC.touch = {};
    OSC.connect_time;
    OSC.offsetForDecoded = 0;

    OSC.was_loaded = false;

    OSC.decoders_array = [];
    OSC.loaded_ind = 0;
    OSC.time_offset_str = "";
    OSC.time_scale = 1;
    OSC.unexpectedClose = true;
    OSC.scrollLogContainer = 0;
    OSC.ch1_size = 0;

    OSC.trigger_position = 0;
    OSC.flagWheelHandled = false;
    OSC.allSignalShown = false;
    OSC.state.decimate = 1;
    // OSC.demoDecodersCreated = false;
    // OSC.demo_mode = false;
    OSC.samples_sum = 512;
    OSC.samples_first = true;


    OSC.scaleWasChanged = false;
    OSC.splitted_signal = {};
    OSC.splittedAvgSignal = {};


    OSC.guiHandler = function() {
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






    OSC.g_CpuLoad = 100.0;
    OSC.g_CpuTemp = 100.0;
    OSC.g_TotalMemory = 256.0;
    OSC.g_FreeMemory = 256.0;

    setInterval(function() {
        $('#cpu_load').text(OSC.g_CpuLoad.toFixed(2) + "%");
        $('#cpu_temp').text(OSC.g_CpuTemp.toFixed(0));
        $('#totalmem_view').text((OSC.g_TotalMemory / (1024 * 1024)).toFixed(2) + "Mb");
        $('#freemem_view').text((OSC.g_FreeMemory / (1024 * 1024)).toFixed(2) + "Mb");
        $('#usagemem_view').text(((OSC.g_TotalMemory - OSC.g_FreeMemory) / (1024 * 1024)).toFixed(2) + "Mb");
    }, 1000);


    OSC.getBusByDecoderName = function(decoder_name) {
        for (var i = 1; i < 5; i++) {
            var bus_name = "bus" + i;
            if (OSC.buses[bus_name] != undefined && OSC.buses[bus_name].name != undefined && OSC.buses[bus_name].enabled) {
                switch (OSC.buses[bus_name].name) {
                    case "UART":
                    case "CAN":
                    case "I2C":
                        if (OSC.buses[bus_name].decoder == decoder_name)
                            return bus_name;
                        break;
                    case "SPI":
                        if (OSC.buses[bus_name].miso_decoder == decoder_name)
                            return bus_name;
                        if (OSC.buses[bus_name].mosi_decoder == decoder_name)
                            return bus_name;
                        break;
                }
            }
        }
        return "";
    }


    OSC.i2c_paramteters = function(new_params) {
        var param = new_params['i2c1_parameters'].value;
        console.log(param);
    }



    OSC.processParameters = function(new_params) {
        var requestRedecode = false;
        // First init after reset
        if (CLIENT.getValue("LA_MAX_FREQ") == undefined){
            LA.initCursors()
            requestRedecode = true
        }

        if (new_params['LA_MAX_FREQ'])
            COMMON.updateMaxFreq(new_params['LA_MAX_FREQ'].value)

        if (new_params['LA_CUR_FREQ']) // Need set before other parameters
            OSC.setCurrentFreq(new_params['LA_CUR_FREQ'].value)

        for (var param_name in new_params) {
            CLIENT.params.orig[param_name] = new_params[param_name];
            if (OSC.param_callbacks[param_name] !== undefined)
                OSC.param_callbacks[param_name](new_params);
        }

        // Need resend decoded data after first init UI
        if (requestRedecode){
            LA.requestRedecode()
        }
    };

    // Processes newly received data for signals
    OSC.processSignals = function(new_signals) {
        var visible_btns = [];
        var visible_plots = [];
        var visible_info = '';
        var start = +new Date();

        // Do nothing if no parameters received yet
        if ($.isEmptyObject(OSC.params.orig))
            return;
        var arrrrr = ['ch1', 'ch2', 'ch3', 'ch4', 'ch5', 'ch6', 'ch7', 'ch8'];
        var pointArr = [];
        var colorsArr = [];

        for (var sig_name in new_signals) {
            var index = arrrrr.indexOf(sig_name);

            // Ignore empty signals
            if (new_signals[sig_name].size == 0)
                continue;

            if (!OSC.enabled_channels[index])
                continue;

            var points = [];
            var color = LA.graph_colors[sig_name];
            var start_point = 0;

            for (var u = 0; u < new_signals[sig_name].value.length; u += 2) {
                var start_x = start_point;
                var start_y = new_signals[sig_name].value[u + 1];

                points.push([start_x, start_y]);

                start_point += new_signals[sig_name].value[u];
                if (start_point > 1024)
                    start_point = 1024;

                var end_x = start_point;
                var end_y = start_y;
                points.push([end_x, end_y]);
                if (end_x == 1024)
                    break;
            }
            pointArr.push(points);
            colorsArr.push(color);
            if (!OSC.state.sel_sig_name && !$('#right_menu .not-signal').hasClass('active')) {
                $('#right_menu .menu-btn.' + OSC.state.sel_sig_name).addClass('active');
            }
        }

        if (LA.graphs) {
            LA.graphs.elem.show();
            LA.graphs.plot.setColors(colorsArr);
            LA.graphs.plot.resize();
            LA.graphs.plot.setupGrid();
            LA.graphs.plot.setData(pointArr);
            LA.graphs.plot.draw();
        }

        for (var sig_name in new_signals) {
            var index = arrrrr.indexOf(sig_name);
            if (!OSC.enabled_channels[index])
                continue;
            OSC.drawSeries(index, LA.graphs.plot, LA.graphs.plot.getCanvas().getContext("2d"));
        }

        visible_plots.push(OSC.graphs.elem[0]);
        visible_info += (visible_info.length ? ',' : '') + '.' + "ch1";

        if (OSC.scaleWasChanged)
            COMMON.fflushLog();
        // Hide plots without signal
        $('#graphs .plot').not(visible_plots).hide();

        // Show only information about active signals
        $('#info').find(visible_info).show();

        // Reset resize flag
        OSC.state.resized = false;

        // Check if selected signal is still visible
        if (OSC.state.sel_sig_name && OSC.graphs[OSC.state.sel_sig_name] && !OSC.graphs[OSC.state.sel_sig_name].elem.is(':visible')) {
            $('#right_menu .menu-btn.active.' + OSC.state.sel_sig_name).removeClass('active');
            //OSC.state.sel_sig_name = nameull;
        }
    };

    // Exits from editing mode
    OSC.exitEditing = function(noclose) {

        var key_for_send = ['LA_DECIMATE','LA_PRE_TRIGGER_BUFFER_MS','LA_POST_TRIGGER_BUFFER_MS','LA_WIN_SHOW']

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
                console.log(key + ' changed from ' + CLIENT.params.orig[key].value + ' to ' + ($.type(CLIENT.params.orig[key].value) == 'boolean' ? !!value : value));
                CLIENT.parametersCache[key] = {
                    value: ($.type(CLIENT.params.orig[key].value) == 'boolean' ? !!value : value)
                };
                CLIENT.params.orig[key] = CLIENT.parametersCache[key]
                CLIENT.sendParameters();
            }
        }

        // Send params then reset editing state and hide dialog

        OSC.state.editing = false;
        if (noclose) return;
        $('.dialog:visible').hide();
        $('#right_menu').show();
    };

    // Sends to server modified parameters
    OSC.sendParams = function() {
        if ($.isEmptyObject(OSC.params.local)) {
            return false;
        }

        if (!OSC.state.socket_opened) {
            console.log('ERROR: Cannot save changes, socket not opened');
            return false;
        }

        OSC.params.local['in_command'] = {
            value: 'send_all_params'
        };

        OSC.ws.send(JSON.stringify({
            parameters: OSC.params.local
        }));
        OSC.params.local = {};

        return true;
    };







    OSC.needUpdateDecoder = function(bus, new_bus) {
        if (OSC.buses[bus] == undefined || OSC.buses[bus].name == undefined)
            return false;
        if (OSC.buses[bus].enabled && OSC.buses[bus].name == new_bus)
            return true;
        return false;
    }

    OSC.destroyDecoder = function(bus, new_bus) {
        if (OSC.buses[bus] == undefined || OSC.buses[bus].name == undefined)
            return;
        if (!OSC.buses[bus].enabled)
            return;
        if (OSC.buses[bus].name == new_bus)
            return;

        if (OSC.buses[bus].name === "UART") {
            OSC.hideInfoArrow(parseInt(OSC.buses[bus].rx) - 1);
            OSC.hideInfoArrow(parseInt(OSC.buses[bus].tx) - 1);
        } else if (OSC.buses[bus].name === "CAN") {
            OSC.hideInfoArrow(parseInt(OSC.buses[bus].can_rx) - 1);
        } else if (OSC.buses[bus].name === "I2C") {
            OSC.hideInfoArrow(parseInt(OSC.buses[bus].scl) - 1);
            OSC.hideInfoArrow(parseInt(OSC.buses[bus].sda) - 1);
        } else if (OSC.buses[bus].name === "SPI") {
            OSC.hideInfoArrow(parseInt(OSC.buses[bus].clk) - 1);
            OSC.hideInfoArrow(parseInt(OSC.buses[bus].cs) - 1);
            OSC.hideInfoArrow(parseInt(OSC.buses[bus].mosi) - 1);
            OSC.hideInfoArrow(parseInt(OSC.buses[bus].miso) - 1);
        }

        // Drop decoder signal
        var signalName = '';

        if (OSC.buses[bus].name == 'UART' || OSC.buses[bus].name == 'I2C' || OSC.buses[bus].name == 'CAN')
            signalName = OSC.buses[bus].decoder + '_signal';
        else if (OSC.buses[bus].miso_decoder !== undefined && OSC.buses[bus].miso_decoder !== "")
            signalName = OSC.buses[bus].miso_decoder + '_signal';
        else if (OSC.buses[bus].mosi_decoder !== undefined && OSC.buses[bus].mosi_decoder !== "")
            signalName = OSC.buses[bus].mosi_decoder + '_signal';

        delete OSC.signalStack[signalName];
        delete OSC.latest_signal[signalName];

        switch (OSC.buses[bus].name) {
            case "UART":
            case "CAN":
            case "I2C":
                OSC.params.local['DESTROY_DECODER'] = {
                    value: OSC.buses[bus].decoder
                };
                OSC.sendParams()
                break;
            case "SPI":
                if (OSC.buses[bus].miso_decoder !== undefined && OSC.buses[bus].miso_decoder !== "") {
                    OSC.params.local['DESTROY_DECODER'] = {
                        value: OSC.buses[bus].miso_decoder
                    };
                    OSC.sendParams();
                }
                if (OSC.buses[bus].mosi_decoder !== undefined && OSC.buses[bus].mosi_decoder !== "") {
                    OSC.params.local['DESTROY_DECODER'] = {
                        value: OSC.buses[bus].mosi_decoder
                    };
                    OSC.sendParams();
                }
                break;
        }
    }



    OSC.show_step = function(new_params) {
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



    // OSC.sendACQ = function() {
    //     OSC.params.local['SAMPLE_RATE'] = { value: OSC.state.acq_speed };
    //     OSC.params.local['DECIMATE'] = { value: OSC.state.decimate };

    //     var val = $('#pre-sample-buf-val').val();
    //     val = (val == "") ? 100 : parseInt(val);
    //     val /= 1000; // Convert milliseconds to seconds
    //     val *= OSC.state.acq_speed; // Convert to samples number

    //     OSC.params.local['PRE_SAMPLE_BUFFER'] = {
    //         value: val
    //     };

    //     OSC.sendParams();
    // }





    OSC.param_callbacks["LA_RUN"] = OSC.processRun;

    OSC.param_callbacks["LA_CUR_FREQ"] = OSC.setCurrentFreq;
    OSC.param_callbacks["LA_DECIMATE"] = OSC.setDecimation;
    OSC.param_callbacks["LA_SCALE"] = OSC.setTimeScale;
    OSC.param_callbacks["LA_VIEW_PORT_POS"] = OSC.setViewPortPos;

    OSC.param_callbacks["LA_PRE_TRIGGER_BUFFER_MS"] = OSC.setPresample;
    OSC.param_callbacks["LA_POST_TRIGGER_BUFFER_MS"] = OSC.setPostsample;
    OSC.param_callbacks["CONTROL_CONFIG_SETTINGS"] = OSC.setControlConfig;
    OSC.param_callbacks["LIST_FILE_SATTINGS"] = OSC.listSettings;

    OSC.param_callbacks["LA_PRE_TRIGGER_SAMPLES"] = OSC.setPreTriggerCountCaptured;
    OSC.param_callbacks["LA_POST_TRIGGER_SAMPLES"] = OSC.setPostTriggerCountCaptured;
    OSC.param_callbacks["LA_TOTAL_SAMPLES"] = OSC.setSampleCountCaptured;

    OSC.param_callbacks["LA_MEASURE_MODE"] = OSC.updateMeasureMode;
    OSC.param_callbacks["LA_DISPLAY_RADIX"] = OSC.updateDisplayRadix;

    OSC.param_callbacks["LA_DIN_1"] = OSC.setDIN1Enabled;
    OSC.param_callbacks["LA_DIN_2"] = OSC.setDIN2Enabled;
    OSC.param_callbacks["LA_DIN_3"] = OSC.setDIN3Enabled;
    OSC.param_callbacks["LA_DIN_4"] = OSC.setDIN4Enabled;
    OSC.param_callbacks["LA_DIN_5"] = OSC.setDIN5Enabled;
    OSC.param_callbacks["LA_DIN_6"] = OSC.setDIN6Enabled;
    OSC.param_callbacks["LA_DIN_7"] = OSC.setDIN7Enabled;
    OSC.param_callbacks["LA_DIN_8"] = OSC.setDIN8Enabled;


    OSC.param_callbacks["LA_DIN_NAME_1"] = OSC.setDIN1Name;
    OSC.param_callbacks["LA_DIN_NAME_2"] = OSC.setDIN2Name;
    OSC.param_callbacks["LA_DIN_NAME_3"] = OSC.setDIN3Name;
    OSC.param_callbacks["LA_DIN_NAME_4"] = OSC.setDIN4Name;
    OSC.param_callbacks["LA_DIN_NAME_5"] = OSC.setDIN5Name;
    OSC.param_callbacks["LA_DIN_NAME_6"] = OSC.setDIN6Name;
    OSC.param_callbacks["LA_DIN_NAME_7"] = OSC.setDIN7Name;
    OSC.param_callbacks["LA_DIN_NAME_8"] = OSC.setDIN8Name;

    OSC.param_callbacks["LA_DIN_1_TRIGGER"] = OSC.setDIN1Trigger;
    OSC.param_callbacks["LA_DIN_2_TRIGGER"] = OSC.setDIN2Trigger;
    OSC.param_callbacks["LA_DIN_3_TRIGGER"] = OSC.setDIN3Trigger;
    OSC.param_callbacks["LA_DIN_4_TRIGGER"] = OSC.setDIN4Trigger;
    OSC.param_callbacks["LA_DIN_5_TRIGGER"] = OSC.setDIN5Trigger;
    OSC.param_callbacks["LA_DIN_6_TRIGGER"] = OSC.setDIN6Trigger;
    OSC.param_callbacks["LA_DIN_7_TRIGGER"] = OSC.setDIN7Trigger;
    OSC.param_callbacks["LA_DIN_8_TRIGGER"] = OSC.setDIN8Trigger;

    OSC.param_callbacks["LA_DIN_1_POS"] = OSC.setDIN1Pos;
    OSC.param_callbacks["LA_DIN_2_POS"] = OSC.setDIN2Pos;
    OSC.param_callbacks["LA_DIN_3_POS"] = OSC.setDIN3Pos;
    OSC.param_callbacks["LA_DIN_4_POS"] = OSC.setDIN4Pos;
    OSC.param_callbacks["LA_DIN_5_POS"] = OSC.setDIN5Pos;
    OSC.param_callbacks["LA_DIN_6_POS"] = OSC.setDIN6Pos;
    OSC.param_callbacks["LA_DIN_7_POS"] = OSC.setDIN7Pos;
    OSC.param_callbacks["LA_DIN_8_POS"] = OSC.setDIN8Pos;

    OSC.param_callbacks["DECODER_1"] = LA.setBUS1Settings;
    OSC.param_callbacks["DECODER_2"] = LA.setBUS2Settings;
    OSC.param_callbacks["DECODER_3"] = LA.setBUS3Settings;
    OSC.param_callbacks["DECODER_4"] = LA.setBUS4Settings;

    OSC.param_callbacks["DECODER_ENABLED_1"] = OSC.setBUS1Enabled;
    OSC.param_callbacks["DECODER_ENABLED_2"] = OSC.setBUS2Enabled;
    OSC.param_callbacks["DECODER_ENABLED_3"] = OSC.setBUS3Enabled;
    OSC.param_callbacks["DECODER_ENABLED_4"] = OSC.setBUS4Enabled;

    OSC.param_callbacks["DECODER_DEF_UART"] = LA.setDefSetUART;
    OSC.param_callbacks["DECODER_DEF_CAN"] =  LA.setDefSetCAN;
    OSC.param_callbacks["DECODER_DEF_SPI"] =  LA.setDefSetSPI;
    OSC.param_callbacks["DECODER_DEF_I2C"] =  LA.setDefSetI2C;

    OSC.param_callbacks["DECODER_ANNOTATION_UART"] = SERIES.setAnnoSetUART;
    OSC.param_callbacks["DECODER_ANNOTATION_CAN"] =  SERIES.setAnnoSetCAN;
    OSC.param_callbacks["DECODER_ANNOTATION_SPI"] =  SERIES.setAnnoSetSPI;
    OSC.param_callbacks["DECODER_ANNOTATION_I2C"] =  SERIES.setAnnoSetI2C;

    OSC.param_callbacks["LA_CURSOR_X1"] = LA.cursorX;
    OSC.param_callbacks["LA_CURSOR_X2"] = LA.cursorX;
    OSC.param_callbacks["LA_CURSOR_X1_POS"] = LA.cursorX;
    OSC.param_callbacks["LA_CURSOR_X2_POS"] = LA.cursorX;

    OSC.param_callbacks["LA_MEASURE_STATE"] = OSC.show_step;

    OSC.param_callbacks["RP_SYSTEM_CPU_LOAD"] = OSC.setCpu;
    OSC.param_callbacks["RP_SYSTEM_TEMPERATURE"] = OSC.setCpuTemp;
    OSC.param_callbacks["RP_SYSTEM_FREE_RAM"] = OSC.setFreeRAM;
    OSC.param_callbacks["RP_SYSTEM_TOTAL_RAM"] = OSC.setTotalRAM;


    OSC.param_callbacks["LA_WIN_SHOW"] = LA.setWinShow;
    OSC.param_callbacks["LA_WIN_X"] = LA.setWinX;
    OSC.param_callbacks["LA_WIN_Y"] = LA.setWinY;
    OSC.param_callbacks["LA_WIN_W"] = LA.setWinW;
    OSC.param_callbacks["LA_WIN_H"] = LA.setWinH;

    OSC.param_callbacks["LA_LOGGER_BUS_1"] = LOGGER.setLoggerBUS1;
    OSC.param_callbacks["LA_LOGGER_BUS_2"] = LOGGER.setLoggerBUS2;
    OSC.param_callbacks["LA_LOGGER_BUS_3"] = LOGGER.setLoggerBUS3;
    OSC.param_callbacks["LA_LOGGER_BUS_4"] = LOGGER.setLoggerBUS4;
    OSC.param_callbacks["LA_LOGGER_RADIX"] = LOGGER.setLoggerRadix;



}(window.OSC = window.OSC || {}, jQuery));

// Page onload event handler
$(function() {

    var sendLogicData = function() {
        var mail = "support@redpitaya.com";
        var subject = "Decoding Help";
        var body = "DON'T FORGET TO ATTACH FILE!%0D%0A I need help for decoding this data%0D%0A";
        body += "%0D%0A%0D%0A------------------------------------%0D%0A" + "DEBUG INFO, DO NOT EDIT!%0D%0A" + "------------------------------------%0D%0A%0D%0A";
        body += "Parameters:" + "%0D%0A" + JSON.stringify({ parameters: OSC.params }) + "%0D%0A";
        body += "Browser:" + "%0D%0A" + JSON.stringify({ parameters: $.browser }) + "%0D%0A";

        var url = 'info/info.json';
        $.ajax({
            method: "GET",
            url: url
        }).done(function(msg) {
            console.log(msg.responseText);
            body += " info.json: " + "%0D%0A" + msg.responseText;
            document.location.href = "mailto:" + mail + "?subject=" + subject + "&body=" + body;
        }).fail(function(msg) {
            console.log(msg.responseText);
            body += " info.json: " + "%0D%0A" + msg.responseText;
            document.location.href = "mailto:" + mail + "?subject=" + subject + "&body=" + body;
        });
    }

    $('#porblemsLink').click(function() {
        $('#decodehelp_dialog').modal('show');
    });

    $('#download_logicdata').click(function() {
        $('#hidden_link_logicdata').get(0).click();
    });

    $('#generate_help_email').click(function() {
        sendLogicData();
    });

    $('button').bind('activeChanged', function() {
        OSC.exitEditing(true);
    });
    $('select, input').on('change', function() {
        OSC.exitEditing(true);
    });

    // Selecting active signal
    //  $('.menu-btn').on('click touchstart', function() {
    $('.menu-btn').on('click', function() {
        $('#right_menu .menu-btn').not(this).removeClass('active');
        if (!$(this).hasClass('active'))
            OSC.state.sel_sig_name = $(this).data('signal');
        else
            OSC.state.sel_sig_name = null;
        $('.y-offset-arrow').css('z-index', 10);
        $('#' + OSC.state.sel_sig_name + '_offset_arrow').css('z-index', 11);
    });

    // Opening a dialog for changing parameters
    //  $('.edit-mode').on('click touchstart', function() {
    $('.edit-mode').on('click', function() {
        OSC.state.editing = true;
        $('#right_menu').hide();
        $('#' + $(this).attr('id') + '_dialog').show();

        if ($.inArray($(this).data('signal'), ['ch1', 'ch2', 'math', 'output1', 'output2']) >= 0) {
            if (OSC.state.sel_sig_name)
                $('#right_menu .menu-btn.' + OSC.state.sel_sig_name).removeClass('active');
            if ($(this).data('signal') == 'output1' || $(this).data('signal') == 'output2' || $(this).data('signal') == 'math') {
                var out_enabled = $(this).data('signal') == 'output1' ? OSC.params.orig["OUTPUT1_STATE"].value : $(this).data('signal') == 'output2' ? OSC.params.orig["OUTPUT2_STATE"].value : OSC.params.orig["MATH_SHOW"].value;
                if (out_enabled) {
                    OSC.state.sel_sig_name = $(this).data('signal');
                    $('#right_menu .menu-btn.' + OSC.state.sel_sig_name).addClass('active');
                    $('.y-offset-arrow').css('z-index', 10);
                    $('#' + OSC.state.sel_sig_name + '_offset_arrow').css('z-index', 11);
                } else
                    OSC.state.sel_sig_name = null;
            } else {
                OSC.state.sel_sig_name = $(this).data('signal');

                //$('#right_menu .menu-btn.' + OSC.state.sel_sig_name).addClass('active');
                $('.y-offset-arrow').css('z-index', 10);
                $('#' + OSC.state.sel_sig_name + '_offset_arrow').css('z-index', 11);
            }
        }
    });



    // // Touch events
    // $(document).on('touchstart', '.plot', function(ev) {
    //     ev.preventDefault();

    //     // Multi-touch is used for zooming
    //     if (!OSC.touch.start && ev.originalEvent.touches.length > 1) {
    //         OSC.touch.zoom_axis = null;
    //         OSC.touch.start = [{
    //             clientX: ev.originalEvent.touches[0].clientX,
    //             clientY: ev.originalEvent.touches[0].clientY
    //         }, {
    //             clientX: ev.originalEvent.touches[1].clientX,
    //             clientY: ev.originalEvent.touches[1].clientY
    //         }];
    //     }
    //     // Single touch is used for changing offset
    //     else if (!OSC.state.simulated_drag) {
    //         OSC.state.simulated_drag = true;
    //         OSC.touch.offset_axis = null;
    //         OSC.touch.start = [{
    //             clientX: ev.originalEvent.touches[0].clientX,
    //             clientY: ev.originalEvent.touches[0].clientY
    //         }];
    //     }
    // });

    // $(document).on('touchmove', '.plot', function(ev) {
    //     ev.preventDefault();

    //     // Multi-touch is used for zooming
    //     if (ev.originalEvent.touches.length > 1) {

    //         OSC.touch.curr = [{
    //             clientX: ev.originalEvent.touches[0].clientX,
    //             clientY: ev.originalEvent.touches[0].clientY
    //         }, {
    //             clientX: ev.originalEvent.touches[1].clientX,
    //             clientY: ev.originalEvent.touches[1].clientY
    //         }];

    //         // Find zoom axis
    //         if (!OSC.touch.zoom_axis) {
    //             var delta_x = Math.abs(OSC.touch.curr[0].clientX - OSC.touch.curr[1].clientX);
    //             var delta_y = Math.abs(OSC.touch.curr[0].clientY - OSC.touch.curr[1].clientY);

    //             if (Math.abs(delta_x - delta_y) > 10) {
    //                 if (delta_x > delta_y) {
    //                     OSC.touch.zoom_axis = 'x';
    //                 } else if (delta_y > delta_x) {
    //                     OSC.touch.zoom_axis = 'y';
    //                 }
    //             }
    //         }

    //         // Skip first touch event
    //         if (OSC.touch.prev) {

    //             // Time zoom
    //             if (OSC.touch.zoom_axis == 'x') {
    //                 var prev_delta_x = Math.abs(OSC.touch.prev[0].clientX - OSC.touch.prev[1].clientX);
    //                 var curr_delta_x = Math.abs(OSC.touch.curr[0].clientX - OSC.touch.curr[1].clientX);

    //                 if (OSC.state.fine || Math.abs(curr_delta_x - prev_delta_x) > $(this).width() * 0.9 / OSC.time_steps.length) {
    //                     var new_scale = OSC.changeXZoom((curr_delta_x < prev_delta_x ? '+' : '-'), OSC.touch.new_scale_x, true);

    //                     if (new_scale !== null) {
    //                         OSC.touch.new_scale_x = new_scale;
    //                         $('#info_box').html('Time scale ' + OSC.convertTime(new_scale) + '/div');
    //                     }

    //                     OSC.touch.prev = OSC.touch.curr;
    //                 }
    //             }
    //             // Voltage zoom
    //             else if (OSC.touch.zoom_axis == 'y' && OSC.state.sel_sig_name) {
    //                 var prev_delta_y = Math.abs(OSC.touch.prev[0].clientY - OSC.touch.prev[1].clientY);
    //                 var curr_delta_y = Math.abs(OSC.touch.curr[0].clientY - OSC.touch.curr[1].clientY);

    //                 if (OSC.state.fine || Math.abs(curr_delta_y - prev_delta_y) > $(this).height() * 0.9 / OSC.voltage_steps.length) {
    //                     var new_scale = OSC.changeYZoom((curr_delta_y < prev_delta_y ? '+' : '-'), OSC.touch.new_scale_y, true);

    //                     if (new_scale !== null) {
    //                         OSC.touch.new_scale_y = new_scale;
    //                         $('#info_box').html('Vertical scale ' + COMMON.convertVoltage(new_scale) + '/div');
    //                     }

    //                     OSC.touch.prev = OSC.touch.curr;
    //                 }
    //             }
    //         } else if (OSC.touch.prev === undefined) {
    //             OSC.touch.prev = OSC.touch.curr;
    //         }
    //     }
    //     // Single touch is used for changing offset
    //     else if (OSC.state.simulated_drag) {

    //         // Find offset axis
    //         if (!OSC.touch.offset_axis) {
    //             var delta_x = Math.abs(OSC.touch.start[0].clientX - ev.originalEvent.touches[0].clientX);
    //             var delta_y = Math.abs(OSC.touch.start[0].clientY - ev.originalEvent.touches[0].clientY);

    //             if (delta_x > 5 || delta_y > 5) {
    //                 if (delta_x > delta_y) {
    //                     OSC.touch.offset_axis = 'x';
    //                 } else if (delta_y > delta_x) {
    //                     OSC.touch.offset_axis = 'y';
    //                 }
    //             }
    //         }

    //         if (OSC.touch.prev) {

    //             // Time offset
    //             if (OSC.touch.offset_axis == 'x') {
    //                 var delta_x = ev.originalEvent.touches[0].clientX - OSC.touch.prev[0].clientX;

    //                 if (delta_x != 0) {
    //                     //$('#time_offset_arrow').simulate('drag', { dx: delta_x, dy: 0 });
    //                     $('#time_offset_arrow').simulate('drag', {
    //                         dx: delta_x,
    //                         dy: 0
    //                     });
    //                 }
    //             }
    //             // Voltage offset
    //             else if (OSC.touch.offset_axis == 'y' && OSC.state.sel_sig_name) {
    //                 var delta_y = ev.originalEvent.touches[0].clientY - OSC.touch.prev[0].clientY;

    //                 if (delta_y != 0) {
    //                     $('#' + OSC.state.sel_sig_name + '_offset_arrow').simulate('drag', {
    //                         dx: 0,
    //                         dy: delta_y
    //                     });
    //                 }
    //             }
    //         }

    //         OSC.touch.prev = [{
    //             clientX: ev.originalEvent.touches[0].clientX,
    //             clientY: ev.originalEvent.touches[0].clientY
    //         }];
    //     }
    // });

    // $(document).on('touchend', '.plot', function(ev) {
    //     ev.preventDefault();

    //     if (OSC.state.simulated_drag) {
    //         OSC.state.simulated_drag = false;

    //         if (OSC.touch.offset_axis == 'x') {
    //             //$('#time_offset_arrow').simulate('drag', { dx: 0, dy: 0 });
    //             $('#buf_time_offset').simulate('drag', {
    //                 dx: 0,
    //                 dy: 0
    //             });
    //         } else if (OSC.touch.offset_axis == 'y' && OSC.state.sel_sig_name) {
    //             $('#' + OSC.state.sel_sig_name + '_offset_arrow').simulate('drag', {
    //                 dx: 0,
    //                 dy: 0
    //             });
    //         }

    //         delete OSC.touch.start;
    //         delete OSC.touch.prev;
    //     } else {
    //         // Send new scale
    //         if (OSC.touch.new_scale_y !== undefined) {
    //             OSC.params.local['OSC_' + OSC.state.sel_sig_name.toUpperCase() + '_SCALE'] = {
    //                 value: OSC.touch.new_scale_y
    //             };
    //             OSC.sendParams();
    //         } else if (OSC.touch.new_scale_x !== undefined) {
    //             OSC.params.local['OSC_TIME_SCALE'] = {
    //                 value: OSC.touch.new_scale_x
    //             };
    //             OSC.sendParams();
    //         }
    //     }

    //     // Reset touch information
    //     OSC.touch = {};
    //     $('#info_box').empty();
    // });

    // Preload images which are not visible at the beginning
    // $.preloadImages = function() {
    //     for (var i = 0; i < arguments.length; i++) {
    //         $('<img />').attr('src', 'img/' + arguments[i]);
    //     }
    // }
    // $.preloadImages(
    //     'node_up.png',
    //     'node_left.png',
    //     'node_right.png',
    //     'node_down.png',
    //     'fine_active.png'
    // );


    $('.btn-less').click(function() {
        var inp = $(this).find('input');
        $('.decoder-tab').hide();
        $(inp.attr('data-attr')).show();
    });




    LA.initGraph()
    LA.initHandlers()
    LA.drawGraphGrid();
    LA.initGraphBuffer();
    LA.initSubWindow()
});