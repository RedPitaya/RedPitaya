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
    OSC.mouseWheelEventFired = false;

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

    // Sampling rates
    OSC.max_freq = undefined;


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
        export_radix: 16,
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
            if (signals['data_rle']){
                LA.lastData = signals
                LA.lastDataRepacked = OSC.repackSignals(LA.lastData)
                needRedraw = true
            }
            var needRedraw = false
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
            if (needRedraw)
                LA.drawAllSeries()
        }
    }

    OSC.scrollDataArea = function() {
        // Scroll
        var div_index = 0;
        COMMON.oldResult = [];
        for (var i = 0; i < COMMON.savedResultArr.length; i++) {
            if ((COMMON.savedResultArr[i].abspos) < OSC.counts_offset)
                div_index++;
            else
                break;
        }
        $('#log-container').scrollTop(20 * div_index);
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

        // First init after reset
        if (CLIENT.getValue("LA_MAX_FREQ") == undefined){
            LA.initCursors()
        }

        if (new_params['LA_MAX_FREQ'])
            OSC.updateMaxFreq(new_params['LA_MAX_FREQ'].value)

        if (new_params['LA_CUR_FREQ']) // Need set before other parameters
            OSC.setCurrentFreq(new_params['LA_CUR_FREQ'].value)

        for (var param_name in new_params) {
            CLIENT.params.orig[param_name] = new_params[param_name];
            if (OSC.param_callbacks[param_name] !== undefined)
                OSC.param_callbacks[param_name](new_params);
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

        var key_for_send = ['LA_DECIMATE','LA_PRE_TRIGGER_BUFFER_MS','LA_POST_TRIGGER_BUFFER_MS']

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

    OSC.param_callbacks["DECODER_ANNOTATION_UART"] = COMMON.setAnnoSetUART;
    OSC.param_callbacks["DECODER_ANNOTATION_CAN"] =  COMMON.setAnnoSetCAN;
    OSC.param_callbacks["DECODER_ANNOTATION_SPI"] =  COMMON.setAnnoSetSPI;
    OSC.param_callbacks["DECODER_ANNOTATION_I2C"] =  COMMON.setAnnoSetI2C;

    OSC.param_callbacks["LA_CURSOR_X1"] = LA.cursorX;
    OSC.param_callbacks["LA_CURSOR_X2"] = LA.cursorX;
    OSC.param_callbacks["LA_CURSOR_X1_POS"] = LA.cursorX;
    OSC.param_callbacks["LA_CURSOR_X2_POS"] = LA.cursorX;

    OSC.param_callbacks["LA_MEASURE_STATE"] = OSC.show_step;

    OSC.param_callbacks["RP_SYSTEM_CPU_LOAD"] = OSC.setCpu;
    OSC.param_callbacks["RP_SYSTEM_TEMPERATURE"] = OSC.setCpuTemp;
    OSC.param_callbacks["RP_SYSTEM_FREE_RAM"] = OSC.setFreeRAM;
    OSC.param_callbacks["RP_SYSTEM_TOTAL_RAM"] = OSC.setTotalRAM;


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

    // Export
    $('#downl_graph').on('click', function() {
        setTimeout(OSC.SaveGraphs, 30);
    });

    $('#downl_csv').on('click', function() {
        OSC.downloadDataAsCSV("laData.csv");
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

    // Close parameters dialog after Enter key is pressed
    $('input').keyup(function(event) {
        if (event.keyCode == 13) {
            OSC.exitEditing(true);
        }
    });

    // Close parameters dialog on close button click
    //  $('.close-dialog').on('click touchstart', function() {
    $('.close-dialog').on('click', function() {
        OSC.exitEditing();
    });

    // Time offset arrow dragging
    $('#time_offset_arrow').draggable({
        axis: 'x',
        containment: 'parent',
        drag: function(ev, ui) {
            OSC.state.line_moving = true;
            var graph_width = $('#graph_grid').outerWidth();
            var zero_pos = (graph_width + 2) / 2;

            // Dragging signals
            if (!$.isEmptyObject(OSC.graphs)) {
                var graph_width = $('#graph_grid').width();
                var pxPerPoint = graph_width / 1024;
                var diff = event.pageX - curXPos;

                if (diff < 0 && (OSC.ch1_size <= ((OSC.counts_offset + 1024) / OSC.time_scale)))
                    return;

                var signalDiff = diff / pxPerPoint;
                OSC.counts_offset -= signalDiff;

                curXPos = event.pageX;
                if (OSC.counts_offset <= 0) {
                    OSC.counts_offset = 0;
                    ev.preventDefault();
                }


                OSC.offsetForDecoded = OSC.counts_offset;

                console.log(OSC.counts_offset);

                // Scroll
                OSC.scrollDataArea();
                var trigPosInPoints = (OSC.samples_sum * OSC.time_scale - OSC.counts_offset) * $('#graphs').width() / 1024 - $('#time_offset_arrow').width() / 2;
                $('#time_offset_arrow').css('left', trigPosInPoints);
                // I cry when I see this code :'(
                $('#OSC_TIME_OFFSET').text(OSC.convertTime(5 * (($('#time_offset_arrow').position().left + $('#time_offset_arrow').width() / 2) / ($('#graphs').width() / 2) - 1) * 102400 / OSC.time_scale / OSC.state.acq_speed));

            }

            // Calculate time per division
            var samplerate = OSC.state.acq_speed;
            var samples = 1024;
            var mul = 1000;
            var scale = OSC.time_scale;
            var dev_num = 10;
            var timeInMs = (((samples / scale) / samplerate) * mul);

            ms_per_px = timeInMs / graph_width;
            OSC.trigger_position = (zero_pos - ui.position.left - ui.helper.width() / 2 - 1);
            var new_value = +(((zero_pos - ui.position.left - ui.helper.width() / 2 - 1) * ms_per_px).toFixed(6));
            var buf_width = graph_width - 2;

            $('#info_box').html('Time offset ' + OSC.convertTime(new_value));
            $('#OSC_TIME_OFFSET').text(OSC.convertTime(new_value));
            OSC.time_offset_str = OSC.convertTime(new_value);
            OSC.guiHandler();
        },
        stop: function(ev, ui) {
            OSC.state.line_moving = false;
            if (!OSC.state.simulated_drag) {
                var graph_width = $('#graph_grid').outerWidth();
                var zero_pos = (graph_width + 2) / 2;

                // Calculate time per division
                var samplerate = OSC.state.acq_speed;
                var samples = 1024;
                var mul = 1000;
                var scale = OSC.time_scale;
                var dev_num = 10;
                var timeInMs = (((samples / scale) / samplerate) * mul);

                ms_per_px = timeInMs / graph_width;

                OSC.params.local['OSC_TIME_OFFSET'] = {
                    value: (zero_pos - ui.position.left - ui.helper.width() / 2 - 1) * ms_per_px
                };
                OSC.sendParams();
                $('#info_box').empty();
                OSC.guiHandler();
            }
        }
    });

    // Time offset rectangle dragging
    $('#buf_time_offset').draggable({
        axis: 'x',
        containment: 'parent',
        drag: function(ev, ui) {
            OSC.state.line_moving = true;
            var buf_width = $('#buffer').width();
            var zero_pos = (buf_width + 2) / 2;

            // Calculate time per division
            var samplerate = OSC.state.acq_speed;
            var samples = 1024;
            var mul = 1000;
            var scale = OSC.time_scale;
            var dev_num = 10;
            var timeInMs = (((samples / scale) / samplerate) * mul);

            var ms_per_px = timeInMs / buf_width;
            var ratio = buf_width / (buf_width * OSC.params.orig['OSC_VIEV_PART'].value);
            var new_value = +(((zero_pos - ui.position.left - ui.helper.width() / 2 - 1) * ms_per_px * ratio).toFixed(2));
            var px_offset = -(new_value / ms_per_px + $('#time_offset_arrow').width() / 2 + 1);

            $('#info_box').html('Time offset ' + OSC.convertTime(new_value));
            $('#OSC_TIME_OFFSET').text(OSC.convertTime(new_value));
            OSC.time_offset_str = OSC.convertTime(new_value);
            $('#time_offset_arrow').css('left', (buf_width + 2) / 2 + px_offset);

            // I cry when I see this code :'(
            $('#OSC_TIME_OFFSET').text(OSC.convertTime(5 * (($('#time_offset_arrow').position().left + $('#time_offset_arrow').width() / 2) / ($('#graphs').width() / 2) - 1) * 102400 / OSC.time_scale / OSC.state.acq_speed));
            OSC.guiHandler();
        },
        stop: function(ev, ui) {
            if (!OSC.state.simulated_drag) {
                var buf_width = $('#buffer').width();
                var zero_pos = (buf_width + 2) / 2;
                OSC.state.line_moving = false;
                // Calculate time per division
                var samplerate = OSC.state.acq_speed;
                var samples = 1024;
                var mul = 1000;
                var scale = OSC.time_scale;
                var dev_num = 10;
                var timeInMs = (((samples / scale) / samplerate) * mul);

                var ms_per_px = timeInMs / buf_width;
                $('#info_box').empty();
                OSC.guiHandler();
            }
        }
    });

    // Touch events
    $(document).on('touchstart', '.plot', function(ev) {
        ev.preventDefault();

        // Multi-touch is used for zooming
        if (!OSC.touch.start && ev.originalEvent.touches.length > 1) {
            OSC.touch.zoom_axis = null;
            OSC.touch.start = [{
                clientX: ev.originalEvent.touches[0].clientX,
                clientY: ev.originalEvent.touches[0].clientY
            }, {
                clientX: ev.originalEvent.touches[1].clientX,
                clientY: ev.originalEvent.touches[1].clientY
            }];
        }
        // Single touch is used for changing offset
        else if (!OSC.state.simulated_drag) {
            OSC.state.simulated_drag = true;
            OSC.touch.offset_axis = null;
            OSC.touch.start = [{
                clientX: ev.originalEvent.touches[0].clientX,
                clientY: ev.originalEvent.touches[0].clientY
            }];
        }
    });

    $(document).on('touchmove', '.plot', function(ev) {
        ev.preventDefault();

        // Multi-touch is used for zooming
        if (ev.originalEvent.touches.length > 1) {

            OSC.touch.curr = [{
                clientX: ev.originalEvent.touches[0].clientX,
                clientY: ev.originalEvent.touches[0].clientY
            }, {
                clientX: ev.originalEvent.touches[1].clientX,
                clientY: ev.originalEvent.touches[1].clientY
            }];

            // Find zoom axis
            if (!OSC.touch.zoom_axis) {
                var delta_x = Math.abs(OSC.touch.curr[0].clientX - OSC.touch.curr[1].clientX);
                var delta_y = Math.abs(OSC.touch.curr[0].clientY - OSC.touch.curr[1].clientY);

                if (Math.abs(delta_x - delta_y) > 10) {
                    if (delta_x > delta_y) {
                        OSC.touch.zoom_axis = 'x';
                    } else if (delta_y > delta_x) {
                        OSC.touch.zoom_axis = 'y';
                    }
                }
            }

            // Skip first touch event
            if (OSC.touch.prev) {

                // Time zoom
                if (OSC.touch.zoom_axis == 'x') {
                    var prev_delta_x = Math.abs(OSC.touch.prev[0].clientX - OSC.touch.prev[1].clientX);
                    var curr_delta_x = Math.abs(OSC.touch.curr[0].clientX - OSC.touch.curr[1].clientX);

                    if (OSC.state.fine || Math.abs(curr_delta_x - prev_delta_x) > $(this).width() * 0.9 / OSC.time_steps.length) {
                        var new_scale = OSC.changeXZoom((curr_delta_x < prev_delta_x ? '+' : '-'), OSC.touch.new_scale_x, true);

                        if (new_scale !== null) {
                            OSC.touch.new_scale_x = new_scale;
                            $('#info_box').html('Time scale ' + OSC.convertTime(new_scale) + '/div');
                        }

                        OSC.touch.prev = OSC.touch.curr;
                    }
                }
                // Voltage zoom
                else if (OSC.touch.zoom_axis == 'y' && OSC.state.sel_sig_name) {
                    var prev_delta_y = Math.abs(OSC.touch.prev[0].clientY - OSC.touch.prev[1].clientY);
                    var curr_delta_y = Math.abs(OSC.touch.curr[0].clientY - OSC.touch.curr[1].clientY);

                    if (OSC.state.fine || Math.abs(curr_delta_y - prev_delta_y) > $(this).height() * 0.9 / OSC.voltage_steps.length) {
                        var new_scale = OSC.changeYZoom((curr_delta_y < prev_delta_y ? '+' : '-'), OSC.touch.new_scale_y, true);

                        if (new_scale !== null) {
                            OSC.touch.new_scale_y = new_scale;
                            $('#info_box').html('Vertical scale ' + OSC.convertVoltage(new_scale) + '/div');
                        }

                        OSC.touch.prev = OSC.touch.curr;
                    }
                }
            } else if (OSC.touch.prev === undefined) {
                OSC.touch.prev = OSC.touch.curr;
            }
        }
        // Single touch is used for changing offset
        else if (OSC.state.simulated_drag) {

            // Find offset axis
            if (!OSC.touch.offset_axis) {
                var delta_x = Math.abs(OSC.touch.start[0].clientX - ev.originalEvent.touches[0].clientX);
                var delta_y = Math.abs(OSC.touch.start[0].clientY - ev.originalEvent.touches[0].clientY);

                if (delta_x > 5 || delta_y > 5) {
                    if (delta_x > delta_y) {
                        OSC.touch.offset_axis = 'x';
                    } else if (delta_y > delta_x) {
                        OSC.touch.offset_axis = 'y';
                    }
                }
            }

            if (OSC.touch.prev) {

                // Time offset
                if (OSC.touch.offset_axis == 'x') {
                    var delta_x = ev.originalEvent.touches[0].clientX - OSC.touch.prev[0].clientX;

                    if (delta_x != 0) {
                        //$('#time_offset_arrow').simulate('drag', { dx: delta_x, dy: 0 });
                        $('#time_offset_arrow').simulate('drag', {
                            dx: delta_x,
                            dy: 0
                        });
                    }
                }
                // Voltage offset
                else if (OSC.touch.offset_axis == 'y' && OSC.state.sel_sig_name) {
                    var delta_y = ev.originalEvent.touches[0].clientY - OSC.touch.prev[0].clientY;

                    if (delta_y != 0) {
                        $('#' + OSC.state.sel_sig_name + '_offset_arrow').simulate('drag', {
                            dx: 0,
                            dy: delta_y
                        });
                    }
                }
            }

            OSC.touch.prev = [{
                clientX: ev.originalEvent.touches[0].clientX,
                clientY: ev.originalEvent.touches[0].clientY
            }];
        }
    });

    $(document).on('touchend', '.plot', function(ev) {
        ev.preventDefault();

        if (OSC.state.simulated_drag) {
            OSC.state.simulated_drag = false;

            if (OSC.touch.offset_axis == 'x') {
                //$('#time_offset_arrow').simulate('drag', { dx: 0, dy: 0 });
                $('#buf_time_offset').simulate('drag', {
                    dx: 0,
                    dy: 0
                });
            } else if (OSC.touch.offset_axis == 'y' && OSC.state.sel_sig_name) {
                $('#' + OSC.state.sel_sig_name + '_offset_arrow').simulate('drag', {
                    dx: 0,
                    dy: 0
                });
            }

            delete OSC.touch.start;
            delete OSC.touch.prev;
        } else {
            // Send new scale
            if (OSC.touch.new_scale_y !== undefined) {
                OSC.params.local['OSC_' + OSC.state.sel_sig_name.toUpperCase() + '_SCALE'] = {
                    value: OSC.touch.new_scale_y
                };
                OSC.sendParams();
            } else if (OSC.touch.new_scale_x !== undefined) {
                OSC.params.local['OSC_TIME_SCALE'] = {
                    value: OSC.touch.new_scale_x
                };
                OSC.sendParams();
            }
        }

        // Reset touch information
        OSC.touch = {};
        $('#info_box').empty();
    });

    // Preload images which are not visible at the beginning
    $.preloadImages = function() {
        for (var i = 0; i < arguments.length; i++) {
            $('<img />').attr('src', 'img/' + arguments[i]);
        }
    }
    $.preloadImages(
        'node_up.png',
        'node_left.png',
        'node_right.png',
        'node_down.png',
        'fine_active.png'
    );




    $('.btn-less').click(function() {
        var inp = $(this).find('input');
        $('.decoder-tab').hide();
        $(inp.attr('data-attr')).show();
    });






    var laAxesMoving = false;
    var curXPos = 0;


    // $(window).on('focus', function() {
    //     OSC.checkAndShowArrows();
    //     setTimeout(function() { OSC.checkAndShowArrows(); }, 750);
    //     OSC.drawGraphGrid();
    // });

    // $(window).on('blur', function() {
    //     OSC.checkAndShowArrows();
    //     setTimeout(function() { OSC.checkAndShowArrows(); }, 750);
    // });

    // $("#graphs").mousedown(function(event) {
    //     laAxesMoving = true;
    //     curXPos = event.pageX;
    // });

    // $("#graphs").mouseup(function(event) {
    //     laAxesMoving = false;
    // });

    // $("#graphs").mouseout(function(event) {
    //     laAxesMoving = false;
    // });


    // $("#graphs").mousemove(function(event) {
    //     if (OSC.state.line_moving) return;
    //     if (laAxesMoving) {
    //         if (!$.isEmptyObject(OSC.graphs)) {
    //             var graph_width = $('#graph_grid').width();
    //             var pxPerPoint = graph_width / 1024;
    //             var diff = event.pageX - curXPos;
    //             if (diff < 0 && (OSC.ch1_size <= ((OSC.counts_offset + 1024 - diff) / OSC.time_scale)))
    //                 diff = -1 * (OSC.ch1_size * OSC.time_scale - OSC.counts_offset - 1024);

    //             var signalDiff = diff / pxPerPoint;
    //             OSC.counts_offset -= signalDiff;

    //             curXPos = event.pageX;

    //             if (OSC.counts_offset <= 0) {
    //                 diff = (signalDiff + OSC.counts_offset) * pxPerPoint;
    //                 OSC.counts_offset = 0;
    //             }
    //             OSC.offsetForDecoded = OSC.counts_offset;
    //             OSC.redrawTimeout = setTimeout(afterScroll, 1);
    //         }
    //     }
    // });

    // var applyAcqSpeed = function() {
    //     OSC.state.acq_speed = (OSC.state.acq_speed / $(this).val()) ;
    //     OSC.state.decimate = $(this).val();

    //     var val = $('#pre-sample-buf-val').val();
    //     val /= 1000;
    //     val *= OSC.state.acq_speed;
    //     if (val > 750000) {
    //         var newVal = ((750000 / OSC.state.acq_speed) * 1000).toFixed(0);
    //         $('#pre-sample-buf-val').val(newVal);
    //         setTimeout(function() { $('#pre-sample-buf-val').change(); }, 100);

    //     }

    //     OSC.sendACQ();

    //     for (var i = 1; i < 5; i++) {
    //         var bus = "bus" + i;
    //         if (OSC.buses[bus].name !== undefined && OSC.buses[bus].name == "UART" && OSC.buses[bus].enabled) {
    //             OSC.buses[bus].samplerate = OSC.state.acq_speed;
    //             OSC.params.local[OSC.buses[bus].decoder + "_parameters"] = {
    //                 value: OSC.buses[bus]
    //             };
    //         }
    //     }

    //     OSC.sendParams();

    // }

    // $('#ACQ_SPEED').change(applyAcqSpeed);

    $(".data-bus").click(function() {
        var arr = ["DATA_BUS0", "DATA_BUS1", "DATA_BUS2", "DATA_BUS3"];
        var bus = arr.indexOf($(this).attr('id'));
        OSC.log_buses[bus] = !$(this).hasClass('active');
        OSC.guiHandler();
        if (OSC.log_buses.indexOf(true) === -1)
            $('#log-container').html('');
        OSC.scaleWasChanged = true;
    });

    // $('.y-offset-arrow').mouseenter(function() {
    //     var idStr = $(this).attr('id');
    //     for (var i = 1; i < 9; i++) {
    //         var tmp = 'ch' + i + '_offset_arrow';
    //         if (tmp == idStr) {
    //             var tooltip = 'ch' + i + '_tooltip';
    //             text = OSC.accordingChanName(i);
    //             if (text != "") {
    //                 $("#" + tooltip).text(text);
    //                 $("#" + tooltip).css('visibility', 'visible');
    //             }
    //         }
    //     }
    // });

    // $('.y-offset-arrow').mouseleave(function() {
    //     var idStr = $(this).attr('id');
    //     for (var i = 1; i < 9; i++) {
    //         var tmp = 'ch' + i + '_offset_arrow';
    //         if (tmp == idStr) {
    //             var tooltip = 'ch' + i + '_tooltip';
    //             $("#" + tooltip).text("");
    //             $("#" + tooltip).css('visibility', 'hidden');
    //         }
    //     }
    // });


    // $("#ext_con_but").click(function(event) {
    //     $('#ext_connections_dialog').modal("show");
    // });

    // $('#pre-sample-buf-val').change(function(e) {
    //     var val = $('#pre-sample-buf-val').val();
    //     if (val == "") {
    //         val = 1;
    //         $('#pre-sample-buf-val').val("1");
    //     } else
    //         val = parseInt(val);

    //     if (val < 1) {
    //         val = 1;
    //         $('#pre-sample-buf-val').val(val);
    //     }

    //     val /= 1000; // Convert milliseconds to seconds
    //     val *= OSC.state.acq_speed; // Convert to samples number

    //     if (val > 750000) {
    //         var newVal = ((750000 / OSC.state.acq_speed) * 1000).toFixed(0);
    //         $('#pre-sample-buf-val').val(newVal);
    //         newVal /= 1000;
    //         newVal *= OSC.state.acq_speed;
    //         val = newVal;
    //     }

    //     applyPreSampleBufVal(val);
    // });

    // $('#stem10_btn').click(function() {
    //     OSC.setMeasureMode(1);
    //     $.cookie('measure_mode', 1, new Date(2220, 1, 1, 0, 0, 0, 0));
    // });

    // $('#stem14_btn').click(function() {

    // });
    LA.initGraph()
    LA.initHandlers()
    LA.drawGraphGrid();
    LA.initGraphBuffer();
});