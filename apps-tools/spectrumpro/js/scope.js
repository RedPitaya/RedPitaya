/*
 * Red Pitaya Spectrum Analizator client
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
    $.fn.textWidth = function() {
        var width = 0;
        var calc = '<span style="display: block; width: 100%; overflow-y: scroll; white-space: nowrap;" class="textwidth"><span>' + $(this).html() + '</span></span>';
        $('body').append(calc);
        var last = $('body').find('span.textwidth:last');
        if (last) {
                var lastcontent = last.find('span');
                width = lastcontent.width();
                last.remove();
        }
        return width;
    };
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

(function(SPEC, $, undefined) {

        SPEC.ch_max = [false,false,false,false];
        SPEC.ch_min = [false,false,false,false];

        // App configuration
        SPEC.param_callbacks = {};
        SPEC.config = {};
        // SPEC.config.app_id = 'spectrumpro';
        // SPEC.config.server_ip = ''; // Leave empty on production, it is used for testing only
        // SPEC.config.start_app_url = window.location.origin + '/bazaar?start=' + SPEC.config.app_id;
        // SPEC.config.stop_app_url = window.location.origin + '/bazaar?stop=' + SPEC.config.app_id;
        // SPEC.config.socket_url = 'ws://' + window.location.host + '/wss';

        // SPEC.config.socket_reconnect_timeout = 1000; // Milliseconds
        SPEC.config.graph_colors = {
            'ch1_view': '#f3ec1a',
            'ch2_view': '#31b44b',
            'ch3_view': '#ee3739',
            'ch4_view': '#3af7f7',
            'ch1_view_min': '#787407',
            'ch2_view_min': '#1b642a',
            'ch3_view_min': '#750a0c',
            'ch4_view_min': '#057a7a',

            'ch1_view_max': '#787407',
            'ch2_view_max': '#1b642a',
            'ch3_view_max': '#750a0c',
            'ch4_view_max': '#057a7a',
        };
        SPEC.config.waterf_img_path = "/tmp/ram/"
        SPEC.freq_unit = ['Hz', 'kHz', 'MHz'];
        SPEC.latest_signal = {};
        SPEC.clear = false;
        SPEC.rp_model = undefined;
        SPEC.client_id = undefined;
        SPEC.lastUpdate = 0;

        SPEC.config.y_axis_mode = "dbm";
        SPEC.config.dbu_imp = 50.0;
        SPEC.config.xmin = 0;
        SPEC.config.xmax = 62500000;
        SPEC.config.unit = 2;
        SPEC.config.updateDelay = 200;


        SPEC.config.attenuator_ch = ["0","0","0","0"];

        SPEC.config.gen_enable = undefined;
        SPEC.channelsCount = 2;
        SPEC.arb_list = undefined;


        // SPEC.compressed_data = 0;
        // SPEC.decompressed_data = 0;
        SPEC.refresh_times = [];

        // SPEC.parameterStack = [];
        // SPEC.signalStack = [];
        SPEC.compressed_data = 0;
        SPEC.decompressed_data = 0;

        SPEC.points_per_px = 2; // How many points per pixel should be drawn. Set null for unlimited (will disable client side decimation).
        SPEC.scale_points_size = 10;
        // App state
        SPEC.state = {
            socket_opened: false,
            processing: false,
            editing: false,
            resized: false,
            cursor_dragging: false,
            sel_sig_name: null,
            fine: false,
            //demo_label_visible: false
            sweep_ch1:false,
            sweep_ch2:false,
            sweep_ch1_time:1,
            sweep_ch2_time:1
        };

        SPEC.graphs = {};
        SPEC.waterfalls = [];

        SPEC.datasets = [];

        SPEC.result = {};
        SPEC.result.peakFreq = [-1,-1,-1,-1];
        SPEC.result.peakPower = [-1,-1,-1,-1];


        SPEC.setModel = function(_value) {
            if (SPEC.rp_model === undefined) {
                console.log("Model",_value["RP_MODEL_STR"].value)
                SPEC.rp_model = _value["RP_MODEL_STR"].value;

                $('#BODY').load((SPEC.rp_model === "Z20_125_4CH" ? "4ch_adc.html" : "2ch_adc.html"), function() {
                    $("#back_button").attr("href", SPEC.previousPageUrl)

                    console.log( "Load was performed." );

                    const ob = new ResizeObserver(function(entries) {
                        SPEC.updateJoystickPosition();
                    });
                    ob.observe(document.querySelector("#menu-root"));

                    const ob_plot_root = new ResizeObserver(function(entries) {
                        $(window).resize();
                    });
                    ob_plot_root.observe(document.querySelector("#root_plot"));

                    if (SPEC.arb_list !== undefined)
                        UI.updateARBFunc(SPEC.arb_list)

                    UI.initHandlers();
                    SPEC.initHandlers();
                    SPEC.waterfalls[0] = $.createWaterfall($("#waterfall_ch1"), $('#waterfall-holder_ch1').width(), 60);
                    SPEC.waterfalls[1] = $.createWaterfall($("#waterfall_ch2"), $('#waterfall-holder_ch2').width(), 60);
                    SPEC.waterfalls[2] = $.createWaterfall($("#waterfall_ch3"), $('#waterfall-holder_ch2').width(), 60);
                    SPEC.waterfalls[3] = $.createWaterfall($("#waterfall_ch4"), $('#waterfall-holder_ch2').width(), 60);
                    SPEC.updateInterfaceFor250(SPEC.rp_model);
                    SPEC.updateInterfaceForZ20(SPEC.rp_model);
                    SPEC.requestParameters();
                    SPEC.initCursors();
                });
            }

        };

        function base64ToFloatArray(base64String) {
            // Decode the base64 string to a byte array
            const b64ToBuffer = (b64) => Uint8Array.from(atob(b64), c => c.charCodeAt(0)).buffer;
            bytes = b64ToBuffer(base64String)
            // Create a Float32Array from the byte array
            const floatArray = new Float32Array(bytes.byteLength / 4);

            // Convert the byte array to a Float32Array
            for (let i = 0; i < floatArray.length; i++) {
              const byteIndex = i * 4;
              floatArray[i] = new DataView(bytes).getFloat32(byteIndex,true);
            }

            return floatArray;
        }

        SPEC.guiHandler = function() {

            if (CLIENT.signalStack.length > 0) {
                let p = performance.now();
                if ((p - SPEC.lastUpdate) > SPEC.config.updateDelay){
                    let signals = Object.assign({}, CLIENT.signalStack[0]);

                    if (SPEC.clear) {
                        signals = {};
                        SPEC.clear = false;
                    }
                    for (const property in signals) {
                        if (signals[property]['type']){
                            if (signals[property]['type'] == 'f'){
                                signals[property]['value'] = base64ToFloatArray(signals[property]['value'] )
                            }
                        }
                    }
                    // create ch3, ch4, ch5, ch6 signals for min/max values
                    SPEC.processSignals(signals);
                    CLIENT.signalStack.splice(0, 1);
                    SPEC.peakUnit();

                    SPEC.lastUpdate = p;
                }
            }
            if (CLIENT.signalStack.length > 2)
                CLIENT.signalStack.length = [];
        }

        var performanceHandler = function() {
            $('#throughput_view2').text((CLIENT.compressed_data / 1024).toFixed(2) + "kB/s");
            if ($('#connection_icon').attr('src') !== '../assets/images/good_net.png')
                $('#connection_icon').attr('src', '../assets/images/good_net.png');
            $('#connection_meter').attr('title', 'It seems like your connection is ok');
            CLIENT.compressed_data = 0;
            CLIENT.decompressed_data = 0;
        }

        SPEC.updateJoystickPosition = function(){
            let height = $("#menu-root").height();
            let g_height = $("#main_block").height() - 340 - height;
            let xpos = Math.max(g_height,0)
            $("#joystick").css('top',xpos);
        };

        setInterval(performanceHandler, 1000);


        // For firefox

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

        SPEC.xMax = function(new_params) {
            if (new_params['xmax'].value > 5) {
                if (SPEC.config.gen_enable === true && CLIENT.params.orig['freq_unit'] && CLIENT.params.orig['freq_unit'].value == 2) {
                    $('#xmax').val(5);
                    new_params['xmax'].value = 5;
                    CLIENT.params.local['xmax'] = { value: 5 };
                    SPEC.sendParams();
                }
            }
        }

        function getBaseLog(x, y) {
            return Math.log(y) / Math.log(x);
        }

        SPEC.savePeak1Unit = function(new_params) {
            if (new_params['peak1_power'] && new_params['peak1_freq']){
                SPEC.result.peakFreq[0] = new_params['peak1_freq'].value;
                SPEC.result.peakPower[0] = new_params['peak1_power'].value;
            }
        }

        SPEC.savePeak2Unit = function(new_params) {
            if (new_params['peak2_power'] && new_params['peak2_freq']){
                SPEC.result.peakFreq[1] = new_params['peak2_freq'].value;
                SPEC.result.peakPower[1] = new_params['peak2_power'].value;
            }
        }

        SPEC.savePeak3Unit = function(new_params) {
            if (new_params['peak3_power'] && new_params['peak3_freq']){
                SPEC.result.peakFreq[2] = new_params['peak3_freq'].value;
                SPEC.result.peakPower[2] = new_params['peak3_power'].value;
            }
        }

        SPEC.savePeak4Unit = function(new_params) {
            if (new_params['peak4_power'] && new_params['peak4_freq']){
                SPEC.result.peakFreq[3] = new_params['peak4_freq'].value;
                SPEC.result.peakPower[3] = new_params['peak4_power'].value;
            }
        }

        SPEC.adcSpeed = function(new_params) {
            // 0 - Hz, 1 - kHz, 2 - MHz
            if (new_params['ADC_FREQ']) {
                var freq_unit = 'Hz';
                var unit = Math.floor(getBaseLog(1000, new_params['ADC_FREQ'].value));
                freq_unit = (unit == 1 ? 'k' : (unit == 2 ? 'M' : '')) + 'S/s';

                var result = '';
                var multiplier = (freq_unit.charAt(0) == 'k') ? 1000 : (freq_unit.charAt(0) == 'M') ? 1000000 : 1;
                result = SPEC.floatToLocalString((new_params['ADC_FREQ'].value / multiplier).toFixed(2)) + ' ' + freq_unit;
                $('#adc_speed').val(result);
            }
        }

        SPEC.setRBW = function(new_params) {
            // 0 - Hz, 1 - kHz, 2 - MHz
            if (new_params['RBW']) {
                var freq_unit = 'Hz';
                var unit = Math.floor(getBaseLog(1000, new_params['RBW'].value));
                freq_unit = (unit == 1 ? 'k' : (unit == 2 ? 'M' : '')) + 'Hz';

                var result = '';
                var multiplier = (freq_unit.charAt(0) == 'k') ? 1000 : (freq_unit.charAt(0) == 'M') ? 1000000 : 1;
                result = SPEC.floatToLocalString((new_params['RBW'].value / multiplier).toFixed(2)) + ' ' + freq_unit;
                $('#rbw').val(result);
            }
        }

        SPEC.peakUnit = function(new_params) {
            // 0 - Hz, 1 - kHz, 2 - MHz
            var max_freq_value = 50e6;
            if (SPEC.rp_model === "Z20_250_12") max_freq_value = 60e6;
            if (SPEC.rp_model === "Z20_250_12_120") max_freq_value = 120e6;

            for(let ch = 0; ch < SPEC.channelsCount; ch++){
                if (!$('#CH' + ch + '_FREEZE').hasClass('active')) {
                    var freq_unit1 = 'Hz';
                    var unit = Math.floor(getBaseLog(1000, SPEC.result.peakFreq[ch]));
                    freq_unit1 = (unit == 1 ? 'k' : (unit == 2 ? 'M' : '')) + 'Hz';

                    var result = '';
                    var multiplier = (freq_unit1.charAt(0) == 'k') ? 1000 : (freq_unit1.charAt(0) == 'M') ? 1000000 : 1;
                    if (SPEC.result.peakPower[ch] < -120 || SPEC.result.peakPower[ch] > 2000)
                        result = 'OVER RANGE';
                    else if (((SPEC.result.peakFreq[ch]) < 0) || ((SPEC.result.peakFreq[ch]) > max_freq_value))
                        result = 'OVER RANGE';
                    else {
                        var dBlabel = SPEC.y_axis_label();
                        var dB = SPEC.result.peakPower[ch];
                        result = SPEC.floatToLocalString(dB.toFixed(3)) + ' ' + dBlabel + ' @ ' + SPEC.floatToLocalString((SPEC.result.peakFreq[ch] / multiplier).toFixed(2)) + ' ' + freq_unit1;
                    }
                    $('#peak_ch'+[ch]).val(result);
                }
            }
        }

        SPEC.srcVoltOffset = function(ch, new_params) {
            var old_params = $.extend(true, {}, CLIENT.params.old);
            var param_name = 'SOUR' + ch + '_VOLT_OFFS';
            if ((!SPEC.state.editing && (old_params[param_name] !== undefined && old_params[param_name].value == new_params[param_name].value))) {
                var value = $('#' + param_name).val();
                if (value !== new_params[param_name].value) {
                    SPEC.setValue($('#' + param_name), new_params[param_name].value);
                }
            }
        }

        SPEC.src1VoltOffset = function(new_params) {
            SPEC.srcVoltOffset("1", new_params);
        }

        SPEC.src2VoltOffset = function(new_params) {
            SPEC.srcVoltOffset("2", new_params);
        }

        SPEC.src3VoltOffset = function(new_params) {
            SPEC.srcVoltOffset("3", new_params);
        }

        SPEC.src4VoltOffset = function(new_params) {
            SPEC.srcVoltOffset("4", new_params);
        }

        SPEC.setXAxisMode = function(new_params) {
            if (new_params['xAxisLogMode']) {
                UI_GRAPH.setXAxisMode(new_params['xAxisLogMode'].value);
            }
        }

        SPEC.setPllMode = function(new_params){
            if (new_params['EXT_CLOCK_ENABLE'].value == true){
                $('#EXT_CLOCK_ENABLE').html('&check; EXT. CLOCK');
                $('#ext_clock_enable_view').show();
            }else{
                $('#EXT_CLOCK_ENABLE').html('EXT. CLOCK');
                $('#ext_clock_enable_view').hide();
            }
        }

        SPEC.sweepResetButton = function(new_params) {
            if ('SOUR1_SWEEP_STATE' in new_params){
                SPEC.state.sweep_ch1 = new_params['SOUR1_SWEEP_STATE'].value
            }
            if ('SOUR2_SWEEP_STATE' in new_params){
                SPEC.state.sweep_ch2 = new_params['SOUR2_SWEEP_STATE'].value
            }
            if (SPEC.state.sweep_ch1 || SPEC.state.sweep_ch2){
                $(".sweep_button").show();
            }else{
                $(".sweep_button").hide();
            }
        }

        SPEC.sweepTime = function(new_params) {
            if ('SOUR1_SWEEP_TIME' in new_params){
                SPEC.state.sweep_ch1_time = new_params['SOUR1_SWEEP_TIME'].value
                if (!$("#SOUR1_SWEEP_TIME").hasClass("focus")){
                    $("#SOUR1_SWEEP_TIME").val(SPEC.convertTimeToText(SPEC.state.sweep_ch1_time));
                }
            }
            if ('SOUR2_SWEEP_TIME' in new_params){
                SPEC.state.sweep_ch2_time = new_params['SOUR2_SWEEP_TIME'].value
                if (!$("#SOUR2_SWEEP_TIME").hasClass("focus")){
                    $("#SOUR2_SWEEP_TIME").val(SPEC.convertTimeToText(SPEC.state.sweep_ch2_time));
                }
            }
        }

        SPEC.riseFallTime = function(new_params) {
            if('SOUR1_RISE' in new_params){
                $("#SOUR1_RISE").val(new_params['SOUR1_RISE'].value);
                $("#SOUR1_RISE").attr("min", new_params['SOUR1_RISE'].min);
                $("#SOUR1_RISE").attr("max", new_params['SOUR1_RISE'].max);
                $("#SOUR1_RISE").attr("step", new_params['SOUR1_RISE'].min);
            }
            if('SOUR1_FALL' in new_params){
                $("#SOUR1_FALL").val(new_params['SOUR1_FALL'].value);
                $("#SOUR1_FALL").attr("min", new_params['SOUR1_FALL'].min);
                $("#SOUR1_FALL").attr("max", new_params['SOUR1_FALL'].max);
                $("#SOUR1_FALL").attr("step", new_params['SOUR1_FALL'].min);
            }
            if('SOUR2_RISE' in new_params){
                $("#SOUR2_RISE").val(new_params['SOUR2_RISE'].value);
                $("#SOUR2_RISE").attr("min", new_params['SOUR2_RISE'].min);
                $("#SOUR2_RISE").attr("max", new_params['SOUR2_RISE'].max);
                $("#SOUR2_RISE").attr("step", new_params['SOUR2_RISE'].min);
            }
            if('SOUR2_FALL' in new_params){
                $("#SOUR2_FALL").val(new_params['SOUR2_FALL'].value);
                $("#SOUR2_FALL").attr("min", new_params['SOUR2_FALL'].min);
                $("#SOUR2_FALL").attr("max", new_params['SOUR2_FALL'].max);
                $("#SOUR2_FALL").attr("step", new_params['SOUR2_FALL'].min);
            }
        }

        SPEC.ch1Visile = function(value){
            console.log(value)
            SPEC.chVisible(1,value);
            $(window).resize();
        }

        SPEC.ch2Visile = function(value){
            SPEC.chVisible(2,value);
            $(window).resize();
        }

        SPEC.ch3Visile = function(value){
            SPEC.chVisible(3,value);
            $(window).resize();
        }

        SPEC.ch4Visile = function(value){
            SPEC.chVisible(4,value);
            $(window).resize();
        }

        SPEC.chVisible = function(ch,value){
            if (value['CH'+ch+'_SHOW'].value == true) {
                $('#info .left-info .info-title .ch' + ch + ', #info .left-info .info-value .ch' + ch).hide();
                $('#waterfall-holder_ch' + ch).show();
                $('#right_menu .menu-btn.ch' + ch).prop('disabled', false);
            } else {
                $('#info .left-info .info-title .ch' + ch + ', #info .left-info .info-value .ch' + ch).show();
                $('#waterfall-holder_ch' + ch).hide();
                $('#right_menu .menu-btn.ch' + ch).prop('disabled', true);
            }
        }

        SPEC.setADCCount = function(value){
            if ('ADC_COUNT' in value && value['ADC_COUNT'].value != undefined) {
                SPEC.channelsCount = value['ADC_COUNT'].value;
            }
        }

        SPEC.controlSettingsRequest = function(new_params){
            if (new_params['CONTROL_CONFIG_SETTINGS'].value === 2) {  // RESET_DONE
                location.reload();
            }

            if (new_params['CONTROL_CONFIG_SETTINGS'].value === 7) {  // LOAD_DONE
                location.reload();
            }
        }

        SPEC.listSettings = function(new_params){
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
                        CLIENT.params.local['FILE_SATTINGS'] = { value: $(this).attr('file_name') };
                        CLIENT.params.local['CONTROL_CONFIG_SETTINGS'] = { value: 6 }; // LOAD
                        SPEC.sendParams();
                    };
                    img.onclick = function(event) {
                        event.stopPropagation();
                        CLIENT.params.local['FILE_SATTINGS'] = { value: $(this).parent().attr('file_name') };
                        CLIENT.params.local['CONTROL_CONFIG_SETTINGS'] = { value: 5 }; // DELETE
                        SPEC.sendParams();
                    };
                    var r1 = document.getElementById('settings_dropdown');
                    if (r1!= null)
                        r1.appendChild(li);

                }
            })
        }

        SPEC.param_callbacks["CH1_SHOW"] = SPEC.ch1Visile;
        SPEC.param_callbacks["CH2_SHOW"] = SPEC.ch2Visile;
        SPEC.param_callbacks["CH3_SHOW"] = SPEC.ch3Visile;
        SPEC.param_callbacks["CH4_SHOW"] = SPEC.ch4Visile;

        SPEC.param_callbacks["SPEC_RUN"] = SPEC.processRun;
        SPEC.param_callbacks["peak1_freq"] = SPEC.savePeak1Unit;
        SPEC.param_callbacks["peak2_freq"] = SPEC.savePeak2Unit;
        SPEC.param_callbacks["peak3_freq"] = SPEC.savePeak3Unit;
        SPEC.param_callbacks["peak4_freq"] = SPEC.savePeak4Unit;
        SPEC.param_callbacks["ADC_FREQ"] = SPEC.adcSpeed;
        SPEC.param_callbacks["RBW"] = SPEC.setRBW;
        SPEC.param_callbacks["SPEC_CURSOR_Y1"] = SPEC.cursorY;
        SPEC.param_callbacks["SPEC_CURSOR_Y2"] = SPEC.cursorY;
        SPEC.param_callbacks["SPEC_CURSOR_X1"] = SPEC.cursorX;
        SPEC.param_callbacks["SPEC_CURSOR_X2"] = SPEC.cursorX;
        SPEC.param_callbacks["SOUR1_VOLT_OFFS"] = SPEC.src1VoltOffset;
        SPEC.param_callbacks["SOUR2_VOLT_OFFS"] = SPEC.src2VoltOffset;
        SPEC.param_callbacks["SOUR3_VOLT_OFFS"] = SPEC.src3VoltOffset;
        SPEC.param_callbacks["SOUR4_VOLT_OFFS"] = SPEC.src4VoltOffset;
        SPEC.param_callbacks["xAxisLogMode"] = SPEC.setXAxisMode;
        SPEC.param_callbacks["EXT_CLOCK_ENABLE"] = SPEC.setPllMode;
        SPEC.param_callbacks["SOUR1_SWEEP_STATE"] = SPEC.sweepResetButton;
        SPEC.param_callbacks["SOUR2_SWEEP_STATE"] = SPEC.sweepResetButton;
        SPEC.param_callbacks["SOUR1_SWEEP_TIME"] = SPEC.sweepTime;
        SPEC.param_callbacks["SOUR2_SWEEP_TIME"] = SPEC.sweepTime;
        SPEC.param_callbacks["SOUR1_RISE"] = SPEC.riseFallTime;
        SPEC.param_callbacks["SOUR1_FALL"] = SPEC.riseFallTime;
        SPEC.param_callbacks["SOUR2_RISE"] = SPEC.riseFallTime;
        SPEC.param_callbacks["SOUR2_FALL"] = SPEC.riseFallTime;
        SPEC.param_callbacks["RP_MODEL_STR"] = SPEC.setModel;
        SPEC.param_callbacks["ADC_COUNT"] = SPEC.setADCCount;
        SPEC.param_callbacks["CONTROL_CONFIG_SETTINGS"] = SPEC.controlSettingsRequest;
        SPEC.param_callbacks["LIST_FILE_SATTINGS"] = SPEC.listSettings;

        SPEC.param_callbacks["CH1_PROBE"] = function(new_params){UI_GRAPH.updateYAxis(); UI_GRAPH.changeYZoom();}
        SPEC.param_callbacks["CH2_PROBE"] = function(new_params){UI_GRAPH.updateYAxis(); UI_GRAPH.changeYZoom();}
        SPEC.param_callbacks["CH3_PROBE"] = function(new_params){UI_GRAPH.updateYAxis(); UI_GRAPH.changeYZoom();}
        SPEC.param_callbacks["CH4_PROBE"] = function(new_params){UI_GRAPH.updateYAxis(); UI_GRAPH.changeYZoom();}


        SPEC.param_callbacks["SOUR1_TEMP_RUNTIME"] = function(new_params){
            if ('SOUR1_TEMP_RUNTIME' in new_params && new_params['SOUR1_TEMP_RUNTIME'].value != undefined) {
                SPEC.updateOverheatBlock(1, new_params['SOUR1_TEMP_RUNTIME'].value);
            }
        };

        SPEC.param_callbacks["SOUR2_TEMP_RUNTIME"] = function(new_params){
            if ('SOUR2_TEMP_RUNTIME' in new_params && new_params['SOUR2_TEMP_RUNTIME'].value != undefined) {
                SPEC.updateOverheatBlock(2, new_params['SOUR2_TEMP_RUNTIME'].value);
            }
        };

        SPEC.param_callbacks["SOUR1_TEMP_LATCHED"] = function(new_params){
            if ('SOUR1_TEMP_LATCHED' in new_params && new_params['SOUR1_TEMP_LATCHED'].value != undefined) {
                SPEC.updateOverheatInfo(1, new_params['SOUR1_TEMP_LATCHED'].value);
            }
        };

        SPEC.param_callbacks["SOUR2_TEMP_LATCHED"] = function(new_params){
            if ('SOUR2_TEMP_LATCHED' in new_params && new_params['SOUR2_TEMP_LATCHED'].value != undefined) {
                SPEC.updateOverheatInfo(2, new_params['SOUR2_TEMP_LATCHED'].value);
            }
        };

        // Processes newly received values for parameters
        SPEC.processParameters = function(new_params) {
            var old_params = $.extend(true, {}, CLIENT.params.orig);

            if (new_params['ARB_LIST'] && SPEC.arb_list === undefined){
                SPEC.arb_list = new_params['ARB_LIST'].value;
                if (SPEC.arb_list !== "")
                    UI.updateARBFunc(SPEC.arb_list)
            }

            if (new_params['CH1_OUT_GAIN'] && new_params['CH1_OUT_GAIN'].value != undefined) {
                SPEC.processParametersZ250('CH1_OUT_GAIN', new_params['CH1_OUT_GAIN'].value);
            }

            if (new_params['CH2_OUT_GAIN'] && new_params['CH2_OUT_GAIN'].value != undefined) {
                SPEC.processParametersZ250('CH2_OUT_GAIN', new_params['CH2_OUT_GAIN'].value);
            }

            if (new_params['SOUR1_IMPEDANCE'] && new_params['SOUR1_IMPEDANCE'].value != undefined) {
                SPEC.updateMaxLimitOnLoad("CH1", new_params['SOUR1_IMPEDANCE'].value);
            }

            if (new_params['SOUR2_IMPEDANCE'] && new_params['SOUR2_IMPEDANCE'].value != undefined) {
                SPEC.updateMaxLimitOnLoad("CH2", new_params['SOUR2_IMPEDANCE'].value);
            }

            if (new_params['EXT_CLOCK_LOCKED'] && new_params['EXT_CLOCK_LOCKED'].value != undefined) {
                SPEC.updateExtClockLocked(new_params['EXT_CLOCK_LOCKED'].value);
            }

            if (new_params['y_axis_mode'] && new_params['y_axis_mode'].value != undefined) {
                var z = "dbm";
                if (new_params['y_axis_mode'].value ===4) z = "dbuV";
                if (new_params['y_axis_mode'].value ===3) z = "dbV";
                if (new_params['y_axis_mode'].value ===2) z = "dbu";
                if (new_params['y_axis_mode'].value ===1) z = "v";
                if (new_params['y_axis_mode'].value ===5) z = "mW";
                if (new_params['y_axis_mode'].value ===6) z = "dBW";
                $('#BDM_DBU_FUNC').val(z);
                UI_GRAPH.changeYAxisMode(z);
                UI_GRAPH.updateYAxis();
            }

            if (!new_params['RP_MODEL_STR']){
                if (SPEC.rp_model === undefined){
                    SPEC.requestParameters();
                    return;
                }
            }

            for (var param_name in new_params) {
                // Save new parameter value

                CLIENT.params.orig[param_name] = new_params[param_name];
                var field = $('#' + param_name);

                if (SPEC.param_callbacks[param_name] !== undefined)
                    SPEC.param_callbacks[param_name](new_params);

                // Do not change fields from dialogs when user is editing something
                if ((old_params[param_name] === undefined || old_params[param_name].value !== new_params[param_name].value)) {
                    if (field.is('select') || field.is('input:text')) {
                        if (['SOUR1_DCYC', 'SOUR2_DCYC'].indexOf(param_name) != -1) {
                            field.val(new_params[param_name].value.toFixed(1));
                        } else if (['SOUR1_PHAS', 'SOUR2_PHAS'].indexOf(param_name) != -1) {
                            field.val(new_params[param_name].value.toFixed(0));
                        } else {
                            if (param_name == 'SOUR1_SWEEP_TIME' || param_name == 'SOUR2_SWEEP_TIME') continue;
                            if (param_name == 'xmin' || param_name == 'xmax' || param_name == 'freq_unit') {

                                if (param_name == 'xmin' || param_name == 'xmax'){
                                    var z = Math.pow(1000,SPEC.config.unit);
                                    if ('freq_unit' in new_params) {
                                        z = Math.pow(1000,new_params['freq_unit'].value);
                                    }
                                    field.val(new_params[param_name].value / z);
                                }else{
                                    field.val(new_params[param_name].value);
                                }

                                UI_GRAPH.updateZoom();
                                if (new_params['freq_unit'])
                                    $('#freq').html('Frequency [' + SPEC.freq_unit[new_params['freq_unit'].value] + ']');
                                $('.freeze.active').removeClass('active');
                            }else{
                                field.val(new_params[param_name].value);
                            }
                        }
                    } else if (field.is('button')) {
                        field[new_params[param_name].value === true ? 'addClass' : 'removeClass']('active');
                        //switch green light for output signals
                        if (param_name == "OUTPUT1_STATE" || param_name == "OUTPUT2_STATE") {
                            var sig_name = param_name == "OUTPUT1_STATE" ? 'output1' : 'output2';
                            if (new_params[param_name].value === true) {
                                if (SPEC.state.sel_sig_name)
                                    $('#right_menu .menu-btn.' + SPEC.state.sel_sig_name).removeClass('active');
                                SPEC.state.sel_sig_name = sig_name;

                                $('#right_menu .menu-btn.' + SPEC.state.sel_sig_name).addClass('active');
                                $('.y-offset-arrow').css('z-index', 10);
                                $('#' + SPEC.state.sel_sig_name + '_offset_arrow').css('z-index', 11);
                            } else {
                                if (SPEC.state.sel_sig_name == sig_name) {
                                    $('#right_menu .menu-btn.' + SPEC.state.sel_sig_name).removeClass('active');
                                    SPEC.state.sel_sig_name = null;
                                }
                            }

                            var value = new_params[param_name].value === true ? 1 : 0;
                            if (value == 1) {
                                $('#' + param_name + '_ON').css("display","inline");
                                $('#' + param_name + '_ON').closest('.menu-btn').addClass('state-on');
                            } else {
                                $('#' + param_name + '_ON').css("display","none");
                                $('#' + param_name + '_ON').closest('.menu-btn').removeClass('state-on');
                            }
                        }
                    } else if (field.is('input:radio')) {
                        var radios = $('input[name="' + param_name + '"]');

                        radios.closest('.btn-group').children('.btn.active').removeClass('active');
                        radios.eq([+new_params[param_name].value]).prop('checked', true).parent().addClass('active');
                    } else if (field.is('span')) {

                        field.html(new_params[param_name].value);
                    }
                }
            }

            // Resize double-headed arrows showing the difference between cursors
            SPEC.updateYCursorDiff();
            SPEC.updateXCursorDiff();
        };

        // Processes newly received data for signals
        SPEC.processSignals = function(new_signals) {
            var reset_zoom_flag = false;

            // Do nothing if no parameters received yet
            if ($.isEmptyObject(CLIENT.params.orig)) {
                return;
            }

            SPEC.datasets = [ // For logic
                null,
                null,
                null,
                null,
                null,
                null,
                null,
                null,
                null,
                null,
                null,
                null
            ];

            var bufferDataset = []; // For output
            var sig_count = 0;
            if (!('signal_mode' in new_signals) && SPEC.graphs && SPEC.graphs.elem) {
                SPEC.graphs.plot.setData(bufferDataset);
                SPEC.graphs.plot.draw();
                return;
            }

            if (new_signals['signal_mode'] && new_signals['signal_mode'].value[0] !== SPEC.y_axis_mode()) return;

            if ('ch_xaxis_full' in new_signals && new_signals['ch_xaxis_full'].size > 0){
                SPEC.downloadDataAsCSV("spectrumSignal.csv",new_signals);
                return;
            }

            UI_GRAPH.updateMinMaxXAxis(new_signals['ch_xaxis'].value);
            const start1 = performance.now();

            var pointsX = [];
            var pointsXAxis = [];
            for (var i = 0; i < new_signals['ch_xaxis'].size; i++) {
                var x = new_signals['ch_xaxis'].value[i] / Math.pow(1000,SPEC.config.unit);
                if (UI_GRAPH.x_axis_mode === 1) {
                    x = UI_GRAPH.convertXLog(x);
                }
                pointsX.push(x);
                pointsXAxis.push([x,undefined]);
            }
            bufferDataset.push({ color: "#000000", data: pointsXAxis });

            for (sig_name in new_signals) {
                // Ignore empty signals
                if (new_signals[sig_name].size == 0 || !SPEC.isVisibleSignal(sig_name)) {
                    continue;
                }

                sig_count++;

                var points = [];
                var water_fall_point = {"x_min":undefined , "x_max":undefined , "y_min":undefined , "y_max":undefined,"x_array":[],"y_array":[]};
                var color = SPEC.config.graph_colors[sig_name];

                var idx = sig_name[sig_name.length - 1] - '0' - 1;
                for (var i = 0; i < new_signals[sig_name].size; i++) {
                    var point_value = new_signals[sig_name].value[i];
                    reset_zoom_flag |= UI_GRAPH.checkYAxisLimit(point_value);
                    var x = Math.round(pointsX[i] * 10000)/ 10000;
                    var y = Math.round(point_value * 10000)/ 10000;
                    x = isNaN(x) ? 0 : x;
                    y = isNaN(y) ? 0 : y;

                    points.push([x,y]);
                    water_fall_point["x_array"].push(x);
                    water_fall_point["y_array"].push(y);

                    if (water_fall_point["x_min"] === undefined){
                        water_fall_point["x_min"] = x;
                    }else if (water_fall_point["x_min"] >= x){
                        water_fall_point["x_min"] = x;
                    }

                    if (water_fall_point["x_max"] === undefined){
                        water_fall_point["x_max"] = x;
                    }else if (water_fall_point["x_max"] <= x){
                        water_fall_point["x_max"] = x;
                    }

                    if (water_fall_point["y_min"] === undefined){
                        water_fall_point["y_min"] = y;
                    }else if (water_fall_point["y_min"] >= y){
                        water_fall_point["y_min"] = y;
                    }

                    if (water_fall_point["y_max"] === undefined){
                        water_fall_point["y_max"] = y;
                    }else if (water_fall_point["y_max"] <= y){
                        water_fall_point["y_max"] = y;
                    }
                }

                SPEC.datasets[idx] = ({ color: color, data: points });
                bufferDataset.push({ color: color, data: points });

                // Update watefalls
                if (SPEC.waterfalls.length > 0 && SPEC.isVisibleChannels()){
                    if (sig_name == 'ch1_view') {
                        SPEC.waterfalls[0].setSize($('.waterfall-graph').width(), 60);
                        if (!$('#CH1_FREEZE').hasClass('active')) {
                            SPEC.waterfalls[0].addData2(water_fall_point);
                        }
                        SPEC.waterfalls[0].draw();
                    }
                    if (sig_name == 'ch2_view') {
                        SPEC.waterfalls[1].setSize($('.waterfall-graph').width(), 60);
                        if (!$('#CH2_FREEZE').hasClass('active')) {
                            SPEC.waterfalls[1].addData2(water_fall_point);
                        }
                        SPEC.waterfalls[1].draw();
                    }

                    if (sig_name == 'ch3_view') {
                        SPEC.waterfalls[2].setSize($('.waterfall-graph').width(), 60);
                        if (!$('#CH3_FREEZE').hasClass('active')) {
                            SPEC.waterfalls[2].addData2(water_fall_point);
                        }
                        SPEC.waterfalls[2].draw();

                    }

                    if (sig_name == 'ch4_view') {
                        SPEC.waterfalls[3].setSize($('.waterfall-graph').width(), 60);
                        if (!$('#CH3_FREEZE').hasClass('active')) {
                            SPEC.waterfalls[3].addData2(water_fall_point);
                        }
                        SPEC.waterfalls[3].draw();
                    }
                }
            }

            if (UI_GRAPH.x_axis_mode === 1) {
                SPEC.updateXInfo();
            }

            UI_GRAPH.unlockUpdateYLimit();
            if (bufferDataset.length > 0) {

                if (SPEC.graphs && SPEC.graphs.elem) {

                    SPEC.graphs.elem.show();
                    SPEC.graphs.plot.setData(bufferDataset);
                    if (SPEC.state.resized) {
                        SPEC.graphs.plot.resize();
                        SPEC.graphs.plot.setupGrid();
                        SPEC.updateCursors();
                    }
                    SPEC.graphs.plot.draw();
                    SPEC.initCursors();
                    $('.harrow').css('left', 'inherit');
                    $('.varrow').css('top', 'inherit');
                    $('#main_block').css('visibility', 'visible');

                } else {
                    if ($('#graph_grid').length){
                        SPEC.graphs.elem = $('<div class="plot" />').css($('#graph_grid').css(['width', 'height'])).appendTo('#graphs');
                        SPEC.graphs.plot = $.plot(SPEC.graphs.elem, bufferDataset, {
                            // colors: [SPEC.config.graph_colors['ch1_view'], SPEC.config.graph_colors['ch2_view']], // channel1, channel2
                            series: {
                                shadowSize: 0, // Drawing is faster without shadows
                                lineWidth: 1,
                                lines: {
                                    lineWidth: 1
                                }
                            },
                            yaxis: {
                                labelWidth: 30,
                                autSPECaleMargin: 1,
                                min: -130,
                                max: 20,
                                tickFormatter: function (val, axis) {
                                    return Number.parseFloat(val).toFixed(2);
                                }
                            },
                            xaxis: {
                                min: 0,
                            //    ticks:xaxisTicks
                                tickFormatter: function (v, axis) {
                                    if (UI_GRAPH.x_axis_mode === 1)
                                        v = UI_GRAPH.convertLog(v);
                                    var scale = Math.pow(1000,SPEC.config.unit);
                                    var roundValue = 100;
                                    if (SPEC.config.xmin / scale < 1 && SPEC.config.xmax / scale < 1){
                                        roundValue = 1000;
                                    }
                                    return Math.round(v * roundValue) / roundValue;
                                }
                            },
                            yaxes: [
                                { font: { color: "#888888" } }
                            ],
                            xaxes: [
                                { font: { color: "#888888" } }
                            ],
                            grid: {
                                show: true,
                                borderColor: '#888888',
                                tickColor: '#888888',
                            }
                        });

                        $('.x1Axis').bind("DOMSubtreeModified", function() {
                            SPEC.updateWaterfallLabels();
                        });

                        UI_GRAPH.updateZoom();

                        SPEC.updateWaterfallWidth();
                        var offset = SPEC.graphs.plot.getPlotOffset();
                        SPEC.sendParameters({ 'view_port_width': $('.plot').width()- offset.left - offset.right});
                        reset_zoom_flag = true;
                        SPEC.drawGraphGrid();
                        SPEC.requestParameters();
                    }
                }
                $('.pull-right').show();
                // Reset resize flag
                SPEC.state.resized = false;
            }
            if (reset_zoom_flag) {
                UI_GRAPH.resetZoom();
            }
        };

    // Exits from editing mode
    SPEC.exitEditing = function(noclose) {
            if (!SPEC.isVisibleChannels()) {
                var sig_btn = $('#right_menu .menu-btn.ch4');
                sig_btn.prop('disabled', true);
                var sig_btn = $('#right_menu .menu-btn.ch3');
                sig_btn.prop('disabled', true);
                var sig_btn = $('#right_menu .menu-btn.ch2');
                sig_btn.prop('disabled', true);
                var sig_btn = $('#right_menu .menu-btn.ch1');
                sig_btn.prop('disabled', true);
            }
            var needResetZoom = false;
            var needUpdateYAxis = false;

            for (var key in CLIENT.params.orig) {
                var field = $('#' + key);
                var value = undefined;

                if (key == 'SPEC_RUN') {
                    value = (field.is(':visible') ? 0 : 1);
                } else if (field.is('select') || field.is('input:text')) {
                    value = field.val();
                } else if (field.is('button')) {
                    value = (field.hasClass('active') ? 1 : 0);
                } else if (field.is('input:radio')) {
                    value = $('input[name="' + key + '"]:checked').val();
                }

                if (key == 'xmin'){
                    value = value * Math.pow(1000,SPEC.config.unit);
                    SPEC.config.xmin = value;
                    SPEC.updateWaterfallWidth();
                }

                if (key == 'xmax'){
                    value = value * Math.pow(1000,SPEC.config.unit);
                    SPEC.config.xmax = value;
                    SPEC.updateWaterfallWidth();
                }

                if (key == 'freq_unit'){
                    SPEC.config.unit = value;
                }

                if (key == 'BDM_DBU_FUNC'){
                    UI_GRAPH.lockUpdateYLimit();
                    needUpdateYAxis = true
                }

                if (key == "SOUR1_SWEEP_TIME") {
                    if (!String(value).includes("s")){
                        SPEC.state.sweep_ch1_time = value
                    }
                    value = SPEC.state.sweep_ch1_time
                }

                if (key == "SOUR2_SWEEP_TIME") {
                    if (!String(value).includes("s")){
                        SPEC.state.sweep_ch2_time = value
                    }
                    value = SPEC.state.sweep_ch2_time
                }

                if (value !== undefined && value != CLIENT.params.orig[key].value) {
                    console.log(key + ' changed from ' + CLIENT.params.orig[key].value + ' to ' + ($.type(CLIENT.params.orig[key].value) == 'boolean' ? !!value : value));
                    CLIENT.params.local[key] = { value: ($.type(CLIENT.params.orig[key].value) == 'boolean' ? !!value : value) };
                }

                if (key == 'CH1_IN_GAIN') {
                    if (SPEC.config.attenuator_ch[0] !== value) {
                        SPEC.config.attenuator_ch[0] = value;
                        needResetZoom = true
                        needUpdateYAxis = true
                    }
                }

                if (key == 'CH2_IN_GAIN') {
                    if (SPEC.config.attenuator_ch[1] !== value) {
                        SPEC.config.attenuator_ch[1] = value;
                        needResetZoom = true
                        needUpdateYAxis = true
                    }
                }

                if (key == 'CH3_IN_GAIN') {
                    if (SPEC.config.attenuator_ch[2] !== value) {
                        SPEC.config.attenuator_ch[2] = value;
                        needResetZoom = true
                        needUpdateYAxis = true
                    }
                }
                if (key == 'CH4_IN_GAIN') {
                    if (SPEC.config.attenuator_ch[3] !== value) {
                        SPEC.config.attenuator_ch[3] = value;
                        needResetZoom = true
                        needUpdateYAxis = true
                    }
                }
            }


        if (needUpdateYAxis){
            UI_GRAPH.updateYAxis();
        }

        if (needResetZoom) {
            UI_GRAPH.resetZoom();
        }


        //console.trace();
        // Send params then reset editing state and hide dialog
        SPEC.sendParams();
        SPEC.state.editing = false;
        if (noclose) return;
        $('.dialog:visible').hide();
        $('#right_menu').show();
    };


    SPEC.requestParameters = function() {
        if (!CLIENT.state.socket_opened) {
            console.log('ERROR: Cannot save changes, socket not opened');
            return false;
        }

        CLIENT.params.local['in_command'] = { value: 'send_all_params' };
        CLIENT.ws.send(JSON.stringify({ parameters: CLIENT.params.local }));
        CLIENT.parameterStack = [];
        CLIENT.params.local = {};
        CLIENT.params.orig = {};
        return true;
    };

    // Sends to server modified parameters
    SPEC.sendParams = function() {
        if ($.isEmptyObject(CLIENT.params.local)) {
            return false;
        }

        if (!CLIENT.state.socket_opened) {
            console.log('ERROR: Cannot save changes, socket not opened');
            return false;
        }
        console.log(CLIENT.params.local);
        CLIENT.ws.send(JSON.stringify({ parameters: CLIENT.params.local }));
        CLIENT.params.local = {};
        return true;
    };

    SPEC.sendParameters = function(values) {
        for (var k in values) {
            CLIENT.params.local[k] = { value: values[k] };
        }

        if ($.isEmptyObject(CLIENT.params.local)) {
            return false;
        }

        if (!CLIENT.state.socket_opened) {
            console.log('ERROR: Cannot save changes, socket not opened');
            return false;
        }
        console.log(CLIENT.params.local);
        CLIENT.ws.send(JSON.stringify({ parameters: CLIENT.params.local }));
        CLIENT.params.local = {};
        return true;
    };

    // Draws the grid on the lowest canvas layer
    SPEC.drawGraphGrid = function() {
        var grid = $('#root_plot')
        if (grid.length === 0) return
        // var graph_holder_width = $('#main_block').width();
        // var graph_holder_height = $('#main_block').height() - 80 * SPEC.visibleCount() - 50;

        // grid.css('width',graph_holder_width)
        // grid.css('height',graph_holder_height)

        var canvas_width = grid.width();
        var canvas_height = grid.height();

        var center_x = canvas_width / 2;
        var center_y = canvas_height / 2;
        var ctx = $('#graph_grid')[0].getContext('2d');

        var x_offset = 0;
        var y_offset = 0;

        // // Set canvas size
        // ctx.canvas.width = canvas_width;
        // ctx.canvas.height = canvas_height;

        // Set draw options
        ctx.beginPath();
        ctx.lineWidth = 1;
        ctx.strokeStyle = '#343433';

        // Draw ticks
        for (var i = 1; i < 50; i++) {
            x_offset = x_offset + (canvas_width / 50);
            y_offset = y_offset + (canvas_height / 50);

            if (i == 25) {
                continue;
            }

            ctx.moveTo(x_offset, canvas_height - 3);
            ctx.lineTo(x_offset, canvas_height);

            ctx.moveTo(0, y_offset);
            ctx.lineTo(3, y_offset);
        }

        // Draw lines
        x_offset = 0;
        y_offset = 0;

        for (var i = 1; i < 10; i++) {
            x_offset = x_offset + (canvas_height / 10);
            y_offset = y_offset + (canvas_width / 10);

            if (i == 5) {
                continue;
            }

            ctx.moveTo(y_offset, 0);
            ctx.lineTo(y_offset, canvas_height);

            ctx.moveTo(0, x_offset);
            ctx.lineTo(canvas_width, x_offset);
        }

        ctx.stroke();

        // Draw central cross
        ctx.beginPath();
        ctx.lineWidth = 1;
        ctx.strokeStyle = '#343433';

        ctx.moveTo(center_x, 0);
        ctx.lineTo(center_x, canvas_height);

        ctx.moveTo(0, center_y);
        ctx.lineTo(canvas_width, center_y);

        ctx.stroke();
    };

    SPEC.isVisibleChannels = function() {
        for(let i = 1; i <= SPEC.channelsCount ; i++){
            if (CLIENT.params.orig['CH'+i+'_SHOW'] && CLIENT.params.orig['CH'+i+'_SHOW'].value == true)
                return true;
        }
        return false;
    };

    SPEC.getPlot = function() {

        if (SPEC.graphs && SPEC.graphs.elem) {
            var plot = SPEC.graphs.plot;
            return plot;
        }
        return null;
    };

    SPEC.updateMinMaxPlot = function(minx,maxx){
        SPEC.updateWaterfallWidth();
    }

    SPEC.updateWaterfallWidth = function(needDraw) {
        var newh  = 80 * SPEC.visibleCount() + 20;
        if ($('#root_waterfall').height() !== newh){
            $('#root_waterfall').css('height', newh);
        }

        var plot = SPEC.getPlot();
        if (!(SPEC.isVisibleChannels() && plot)) {
            return;
        }
        var offset = plot.getPlotOffset();
        var margins = {};
        margins.marginLeft = offset.left + 'px';
        margins.marginRight = offset.right + 'px';

        $('.waterfall-holder').css(margins);
        $('.waterfall-graph').css('width', $('.plot').width()- offset.left - offset.right);
    };

    SPEC.updateWaterfallLabels = function(needDraw) {
        $("#waterfall-label_ch1").empty();
        $("#waterfall-label_ch2").empty();
        if (SPEC.channelsCount > 2){
            $("#waterfall-label_ch3").empty();
            $("#waterfall-label_ch4").empty();
        }
        $('.x1Axis').children().each(function () {
            if (this === undefined) return;
            var modifynode = $(this).clone();
            modifynode.css("top","20px")
            if (SPEC.channelsCount >= 1){
                var newnode = $(modifynode).clone();
                $("#waterfall-label_ch1").append(newnode);
            }
            if (SPEC.channelsCount >= 2){
                var newnode = $(modifynode).clone();
                $("#waterfall-label_ch2").append(newnode);
            }
            if (SPEC.channelsCount >= 3){
                var newnode = $(modifynode).clone();
                $("#waterfall-label_ch3").append(newnode);
            }
            if (SPEC.channelsCount >= 4){
                var newnode = $(modifynode).clone();
                $("#waterfall-label_ch4").append(newnode);
            }

        });
    }


    SPEC.hideCursors = function() {
        $('.hline, .vline, .harrow, .varrow, .cur_info').hide();
        $('#cur_y_diff').hide();
        $('#cur_x_diff').hide();
    };

    SPEC.hideInfo = function() {
        $('.pull-right').hide();
        // Disable buttons related to inactive signals
        $('#right_menu .menu-btn').prop('disabled', true);
        $('#info').hide();
        $('.waterfall-holder').hide();
    };


    // SPEC.handleCodePoints = function(array) {
    //     var CHUNK_SIZE = 0x8000; // arbitrary number here, not too small, not too big
    //     var index = 0;
    //     var length = array.length;
    //     var result = '';
    //     var slice;
    //     while (index < length) {
    //         slice = array.slice(index, Math.min(index + CHUNK_SIZE, length)); // `Math.min` is not really necessary here I think
    //         result += String.fromCharCode.apply(null, slice);
    //         index += CHUNK_SIZE;
    //     }
    //     return result;
    // }

    SPEC.getLocalDecimalSeparator = function() {
        var n = 1.1;
        return n.toLocaleString().substring(1, 2);
    };

    SPEC.floatToLocalString = function(num) {
        // Workaround for a bug in Safari 6 (reference: https://github.com/mleibman/SlickGrid/pull/472)
        return (num + '').replace('.', SPEC.getLocalDecimalSeparator());
    };

    SPEC.SaveGraphsPNG = function() {
        html2canvas(document.querySelector("body"), {backgroundColor: '#343433'}).then(canvas => {
            var a = document.createElement('a');
            // toDataURL defaults to png, so we need to request a jpeg, then convert for file download.
            a.href = canvas.toDataURL("image/jpeg").replace("image/jpeg", "image/octet-stream");
            a.download = 'graphs.jpg';
            // a.click(); // Not working with firefox
            fireEvent(a, 'click');
        });
    }

    SPEC.downloadDataAsCSV = function(filename,signals) {
        var strings = "";
        var ch1_show = 'ch1_full' in signals && signals['ch1_full'].size > 0;
        var ch2_show = 'ch2_full' in signals && signals['ch2_full'].size > 0;
        var ch3_show = 'ch3_full' in signals && signals['ch3_full'].size > 0;
        var ch4_show = 'ch4_full' in signals && signals['ch4_full'].size > 0;

        var ch1_min_show = 'ch1_min_full' in signals && signals['ch1_min_full'].size >0 && ch1_show;
        var ch2_min_show = 'ch2_min_full' in signals && signals['ch2_min_full'].size >0 && ch2_show;
        var ch3_min_show = 'ch3_min_full' in signals && signals['ch3_min_full'].size >0 && ch3_show;
        var ch4_min_show = 'ch4_min_full' in signals && signals['ch4_min_full'].size >0 && ch4_show;

        var ch1_max_show = 'ch1_max_full' in signals && signals['ch1_max_full'].size >0 && ch1_show;
        var ch2_max_show = 'ch2_max_full' in signals && signals['ch2_max_full'].size >0 && ch2_show;
        var ch3_max_show = 'ch3_max_full' in signals && signals['ch3_max_full'].size >0 && ch3_show;
        var ch4_max_show = 'ch4_max_full' in signals && signals['ch4_max_full'].size >0 && ch4_show;

        if (!ch1_show && !ch2_show && !ch3_show && !ch4_show) return;

        var lens    = signals['ch_xaxis_full'].size;
        var x_axis  = signals['ch_xaxis_full'].value;
        var ch1     = ch1_show ? signals['ch1_full'].value : undefined;
        var ch2     = ch2_show ? signals['ch2_full'].value : undefined;
        var ch3     = ch3_show ? signals['ch3_full'].value : undefined;
        var ch4     = ch4_show ? signals['ch4_full'].value : undefined;

        var ch1_min = ch1_min_show ? signals['ch1_min_full'].value : undefined;
        var ch2_min = ch2_min_show ? signals['ch2_min_full'].value : undefined;
        var ch3_min = ch3_min_show ? signals['ch3_min_full'].value : undefined;
        var ch4_min = ch4_min_show ? signals['ch4_min_full'].value : undefined;

        var ch1_max = ch1_max_show ? signals['ch1_max_full'].value : undefined;
        var ch2_max = ch2_max_show ? signals['ch2_max_full'].value : undefined;
        var ch3_max = ch3_max_show ? signals['ch3_max_full'].value : undefined;
        var ch4_max = ch4_max_show ? signals['ch4_max_full'].value : undefined;

        var col_delim = ", ";
        var row_delim = "\n";

        strings += "Frequency [" + SPEC.freq_unit[SPEC.config.unit] + "]";

        if (ch1_show)
            strings += ", IN1";
        if (ch1_min_show)
            strings += ", IN1 min";
        if (ch1_max_show)
            strings += ", IN1 max";
        if (ch2_show)
            strings += ", IN2";
        if (ch2_min_show)
            strings += ", IN2 min";
        if (ch2_max_show)
            strings += ", IN2 max";
        if (ch3_show)
            strings += ", IN3";
        if (ch3_min_show)
            strings += ", IN3 min";
        if (ch3_max_show)
            strings += ", IN3 max";
        if (ch4_show)
            strings += ", IN4";
        if (ch4_min_show)
            strings += ", IN4 min";
        if (ch4_max_show)
            strings += ", IN4 max";
        strings += row_delim;



        for (var i = 0; i < lens; i++) {
            if (x_axis[i] < SPEC.config.xmin || x_axis[i] > SPEC.config.xmax) continue;
            strings += x_axis[i] / Math.pow(1000,SPEC.config.unit) ;

            if (ch1_show)
                strings += col_delim + ch1[i];
            if (ch1_min_show)
                strings += col_delim + ch1_min[i];
            if (ch1_max_show)
                strings += col_delim + ch1_max[i];
            if (ch2_show)
                strings += col_delim + ch2[i];
            if (ch2_min_show)
                strings += col_delim + ch2_min[i];
            if (ch2_max_show)
                strings += col_delim + ch2_max[i];
            if (ch3_show)
                strings += col_delim + ch3[i];
            if (ch3_min_show)
                strings += col_delim + ch3_min[i];
            if (ch3_max_show)
                strings += col_delim + ch3_max[i];
            if (ch4_show)
                strings += col_delim + ch4[i];
            if (ch4_min_show)
                strings += col_delim + ch4_min[i];
            if (ch4_max_show)
                strings += col_delim + ch4_max[i];

            strings += row_delim;
        };

        var link = document.createElement('a');
        link.setAttribute('download', filename);
        link.setAttribute('href', 'data:' + 'text/html' + ';charset=utf-8,' + encodeURIComponent(strings));
        fireEvent(link, 'click');
    }

}(window.SPEC = window.SPEC || {}, jQuery));

// Page onload event handler
$(function() {
    var reloaded = $.cookie("spectrum_forced_reload");
    if (reloaded == undefined || reloaded == "false") {
        $.cookie("spectrum_forced_reload", "true");
        window.location.reload(true);
    }

    // Preload images which are not visible at the beginning
    $.preloadImages = function() {
        for (var i = 0; i < arguments.length; i++) {
            $('<img />').attr('src', 'img/' + arguments[i]);
        }
    }

    $.preloadImages('node_up.png', 'node_left.png', 'node_right.png', 'node_down.png');
    SPEC.drawGraphGrid();

    // Bind to the window resize event to redraw the graph; trigger that event to do the first drawing
    $(window).resize(function() {
        SPEC.updateWaterfallWidth(false);

        var root_plot = $('#root_plot');
        var graph_grid = $('#graph_grid');
        var css = root_plot.css(['width', 'height']);

        if (css){
            graph_grid.css(css);

            var plot = SPEC.getPlot();
            if (plot){
                var css = root_plot.css(['width', 'height']);
                $('.plot').css(css);
                var offset = plot.getPlotOffset();
                SPEC.sendParameters({ 'view_port_width': $('.plot').width()- offset.left - offset.right});
            }


            $('#main_block').css('visibility', 'hidden');

            SPEC.drawGraphGrid();
            SPEC.initCursors();

            // // Set the resized flag
            SPEC.state.resized = true;
            SPEC.updateJoystickPosition();

        }
    }).resize();



    // Init help
    Help.init(helpListSpectrum);
    Help.setState("idle");
    UI_GRAPH.minMaxChange = SPEC.updateMinMaxPlot;
    // Everything prepared, start application

    SPEC.previousPageUrl = document.referrer;
    console.log(`Previously visited page URL: ${SPEC.previousPageUrl}`);

    SPEC.previousPageUrl = document.referrer;
    console.log(`Previously visited page URL: ${SPEC.previousPageUrl}`);
    const currentUrl = window.location.href;
    if (currentUrl === SPEC.previousPageUrl || SPEC.previousPageUrl === ''){
        SPEC.previousPageUrl = '/'
    }
})
