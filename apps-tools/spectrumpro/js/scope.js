/*
 * Red Pitaya Spectrum Analizator client
 *
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

        // App configuration
        SPEC.param_callbacks = {};
        SPEC.config = {};

        SPEC.latest_signal = {};
        SPEC.rp_model = undefined;
        SPEC.rp_model_id = undefined;

        SPEC.draw_every_frame = true
        SPEC.show_pack_per_sec = true

        SPEC.config.gen_enable = undefined;
        SPEC.channelsCount = 2;
        SPEC.arb_list = undefined;
        SPEC.hi_z_mode = false;
        SPEC.gen_max_amp = 1;


        // App state
        SPEC.state = {
            cursor_dragging: false,
            sweep_ch1:false,
            sweep_ch2:false,
            sweep_ch1_time:1,
            sweep_ch2_time:1
        };

        SPEC.graphs = {};
        SPEC.waterfalls = [];

        SPEC.freeze = [false,false,false,false];
        SPEC.resizeTimer = undefined


        SPEC.setModel = function(_value) {
            if (SPEC.rp_model === undefined) {
                console.log("Model",_value["RP_MODEL_STR"].value)
                SPEC.rp_model = _value["RP_MODEL_STR"].value;

                $('#BODY').load((SPEC.rp_model === "Z20_125_4CH" ? "4ch_adc.html" : "2ch_adc.html"), function() {
                    $("#back_button").attr("href", SPEC.previousPageUrl)

                    console.log( "Load was performed." );
                    initImageLoaders();
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
                    UI.initUIItems(_value);
                    SPEC.initHandlers();

                    SPEC.waterfalls[0] = $.createWaterfall($("#waterfall_ch1"), $('#waterfall-holder_ch1').width(), 60);
                    SPEC.waterfalls[1] = $.createWaterfall($("#waterfall_ch2"), $('#waterfall-holder_ch2').width(), 60);
                    if (SPEC.rp_model === "Z20_125_4CH"){
                        SPEC.waterfalls[2] = $.createWaterfall($("#waterfall_ch3"), $('#waterfall-holder_ch3').width(), 60);
                        SPEC.waterfalls[3] = $.createWaterfall($("#waterfall_ch4"), $('#waterfall-holder_ch4').width(), 60);
                    }

                    SPEC.updateInterfaceFor250(SPEC.rp_model);
                    SPEC.updateInterfaceForZ20(SPEC.rp_model);
                    SPEC.initCursors();
                    CLIENT.requestParameters();
                });
            }

        };


        SPEC.guiHandler = function() {
            const MODES = 7
            if (SPEC.averager == undefined)
                SPEC.averager = new DataAverager(4, MODES, 10, 1000,2);

            const FRAME_SKIP_SETTINGS = {
                enabled: true,
                skipEvery: 2,
                maxStackSize: 10,
                performanceThreshold: 3
            };

            SPEC.draw_every_frame = CLIENT.signalStack.length < FRAME_SKIP_SETTINGS.performanceThreshold;

            if (!SPEC.frameCounter) SPEC.frameCounter = 0;
            SPEC.frameCounter++;

            let shouldRender = true;
            if (FRAME_SKIP_SETTINGS.enabled && CLIENT.signalStack.length >= FRAME_SKIP_SETTINGS.performanceThreshold) {
                shouldRender = (SPEC.frameCounter % FRAME_SKIP_SETTINGS.skipEvery) === 0;
            }

            let peeks = {}
            peeks['f'] = {}
            peeks['p'] = {}
            peeks['m'] = {}

            if (CLIENT.signalStack.length > 0) {
                let signals = CLIENT.signalStack[CLIENT.signalStack.length - 1];

                for(let ch = 1; ch <= 4; ch++){
                    let freeze = SPEC.freeze[ch-1];
                    peeks['f'][ch-1] = []
                    peeks['p'][ch-1] = []
                    // Do not update minimum and maximum for frozen signals.
                    if (freeze != undefined && freeze == false){
                        let mode = signals["settings"].value[0]
                        for(let i = 0; i < CLIENT.signalStack.length; i++){
                            let name = "ch" + ch + "_view";
                            if (CLIENT.signalStack[i][name] && CLIENT.signalStack[i][name]['processed'] != true){
                                SPEC.minMaxController.processing(name,CLIENT.signalStack[i][name].value,CLIENT.signalStack[i]["settings"].value[0]);
                                SPEC.averager.addSynchronizedChannelData(ch-1,[CLIENT.signalStack[i]['peak_freq'].value, CLIENT.signalStack[i]['peak_power'].value])
                                mode = CLIENT.signalStack[i]["settings"].value[0]
                                CLIENT.signalStack[i][name]['processed'] = true
                            }
                        }
                        let f = SPEC.averager.getAverages2D(0)
                        let p = SPEC.averager.getAverages2D(1)
                        peeks['f'][ch-1] = f[ch-1]
                        peeks['p'][ch-1] = p[ch-1]
                        peeks['m'][ch-1] = mode
                    } else {
                        signals['ch'+ch+'_view'] = SPEC.latest_signal['ch'+ch+'_view'];
                        signals['ch'+ch+'_view_min']  = SPEC.latest_signal['ch'+ch+'_view_min'];
                        signals['ch'+ch+'_view_max']  = SPEC.latest_signal['ch'+ch+'_view_max'];
                        signals['ch'+ch+'_view'] = SPEC.latest_signal['ch'+ch+'_view'];

                        for(let m = 0; m < MODES; m++){
                            signals['peak_freq'].value[m + (ch-1) * MODES] = SPEC.latest_signal['peak_freq'].value[m + (ch-1) * MODES]
                            signals['peak_power'].value[m + (ch-1) * MODES] = SPEC.latest_signal['peak_power'].value[m + (ch-1) * MODES]
                            peeks['f'][ch-1].push(SPEC.latest_signal['peak_freq'].value[m + (ch-1) * MODES])
                            peeks['p'][ch-1].push(SPEC.latest_signal['peak_power'].value[m + (ch-1) * MODES])
                            peeks['m'][ch-1] = SPEC.latest_signal["settings"].value[0]
                        }
                    }
                }

                SPEC.minMaxController.getSignals(signals);
                SPEC.latest_signal = Object.assign({}, signals);

                if (shouldRender) {
                    CLIENT.fps++;
                    SPEC.processSignals(signals);
                    SPEC.peakUnit(peeks);
                }

                CLIENT.signalStack.splice(0, 1);

                if (CLIENT.signalStack.length > FRAME_SKIP_SETTINGS.maxStackSize) {
                    CLIENT.signalStack = CLIENT.signalStack.slice(-FRAME_SKIP_SETTINGS.maxStackSize);
                }

                if (!SPEC.draw_every_frame && !FRAME_SKIP_SETTINGS.enabled){
                    CLIENT.signalStack.length = [];
                }
            }

            if (SPEC.frameCounter > 1000) SPEC.frameCounter = 0;
        };

        var performanceHandler = function() {
            let fps = ""
            if (CLIENT.fps){
                fps = " - FPS:" + CLIENT.fps
            }
            if (SPEC.show_pack_per_sec){
                fps += "/" + CLIENT.packs
            }

            $('#throughput_view2').text((CLIENT.compressed_data / 1024).toFixed(2) + "kB/s" + fps);
            if ($('#connection_icon').attr('src') !== '../assets/images/good_net.png')
                $('#connection_icon').attr('src', '../assets/images/good_net.png');
            $('#connection_meter').attr('title', 'It seems like your connection is ok');
            CLIENT.compressed_data = 0;
            CLIENT.decompressed_data = 0;
            CLIENT.fps = 0
            CLIENT.packs = 0
        }

        SPEC.updateJoystickPosition = function(){
            let height = $("#menu-root").height();
            let g_height = $("#main_block").height() - 300 - height;
            height = Math.max(20,g_height);
            $("#joystick").css('top',height);
            $("#auto_scale").css('top',height);
        };

        setInterval(performanceHandler, 1000);


        // For firefox

        SPEC.fireEvent = function(obj, evt) {
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

        SPEC.adcSpeed = function(new_params) {
            // 0 - Hz, 1 - kHz, 2 - MHz
            if (new_params['ADC_FREQ']) {
                let result = '';
                let unit = Math.floor(SPEC.getBaseLog(1000, new_params['ADC_FREQ'].value));
                let x = (unit == 1 ? 'k' : (unit == 2 ? 'M' : '')) + 'S/s';
                var multiplier = unit == 1  ? 1000 : (unit == 2  ? 1000000 : 1);
                result = SPEC.floatToLocalString((new_params['ADC_FREQ'].value / multiplier).toFixed(2)) + ' ' + x;
                $('#adc_speed').val(result);
            }
        }

        SPEC.setRBW = function(new_params) {
            // 0 - Hz, 1 - kHz, 2 - MHz
            if (new_params['RBW']) {
                let result = '';
                let unit = Math.floor(SPEC.getBaseLog(1000, new_params['RBW'].value));
                let x = (unit == 1 ? 'k' : (unit == 2 ? 'M' : '')) + 'Hz';
                var multiplier = unit == 1  ? 1000 : (unit == 2  ? 1000000 : 1);
                result = SPEC.floatToLocalString((new_params['RBW'].value / multiplier).toFixed(2)) + ' ' + x;
                $('#rbw').val(result);
            }
        }

        SPEC.peakUnit = function(peeks) {

            for(let ch = 0; ch < SPEC.channelsCount; ch++){
                let result = '';
                let mode = peeks['m'][ch]
                let dBlabel = SPEC.y_axis_label_by_mode(mode);
                let dB = peeks['p'][ch][mode];
                result = SPEC.floatToLocalString(dB.toFixed(3)) + ' ' + dBlabel + ' @ ' +
                         SPEC.floatToLocalString(SPEC.convertFreqToText(peeks['f'][ch][mode]));
                $('#peak_ch'+[ch]).val(result);
            }
        }

        SPEC.setXAxisMode = function(new_params) {
            if (new_params['xAxisLogMode']) {
                UI_GRAPH.setXAxisMode(new_params['xAxisLogMode'].value);
                $('#xAxisLogMode').val(new_params['xAxisLogMode'].value)
            }
        }

        SPEC.ch1Visile = function(value){
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
                $('#CH'+ch+'_SHOW')['addClass']('active');
            } else {
                $('#info .left-info .info-title .ch' + ch + ', #info .left-info .info-value .ch' + ch).show();
                $('#waterfall-holder_ch' + ch).hide();
                $('#right_menu .menu-btn.ch' + ch).prop('disabled', true);
                $('#CH'+ch+'_SHOW')['removeClass']('active');
            }
        }

        SPEC.setOutState = function(new_params,param_name){
            if (param_name == "OUTPUT1_STATE" || param_name == "OUTPUT2_STATE") {
                var value = new_params[param_name].value === true ? 1 : 0;
                if (value == 1) {
                    $('#' + param_name + '_ON').css("display","inline");
                    $('#' + param_name + '_ON').closest('.menu-btn').addClass('state-on');
                    $('#'+ param_name)['addClass']('active');
                } else {
                    $('#' + param_name + '_ON').css("display","none");
                    $('#' + param_name + '_ON').closest('.menu-btn').removeClass('state-on');
                    $('#'+ param_name)['removeClass']('active');
                }
            }
        }

        SPEC.setMinMax = function(values,name){
            if (values[name].value == true) {
                $('#'+name)['addClass']('active');
            } else {
                $('#'+name)['removeClass']('active');
            }
            SPEC.minMaxController.resetByName(name)
            $(window).resize();
        }

        SPEC.setADCCount = function(value){
            if ('ADC_COUNT' in value && value['ADC_COUNT'].value != undefined) {
                SPEC.channelsCount = value['ADC_COUNT'].value;
            }
        }

        SPEC.setGain = function(new_params,param_name) {
            var radios = $('input[name="' + param_name + '"]');
            radios.closest('.btn-group').children('.btn.active').removeClass('active');
            radios.eq([+new_params[param_name].value]).prop('checked', true).parent().addClass('active');
            UI_GRAPH.updateYAxis();
            UI_GRAPH.changeYZoom();
        }

        SPEC.setAcDc = function(new_params,param_name) {
            var radios = $('input[name="' + param_name + '"]');
            radios.closest('.btn-group').children('.btn.active').removeClass('active');
            radios.eq([+new_params[param_name].value]).prop('checked', true).parent().addClass('active');
            UI_GRAPH.updateYAxis();
            UI_GRAPH.changeYZoom();
        }

        SPEC.setFilter = function(new_params,param_name) {
            var radios = $('input[name="' + param_name + '"]');
            radios.closest('.btn-group').children('.btn.active').removeClass('active');
            radios.eq([+new_params[param_name].value]).prop('checked', true).parent().addClass('active');
            UI_GRAPH.updateYAxis();
            UI_GRAPH.changeYZoom();
        }

        SPEC.setProbe = function(new_params,param_name) {
            var value = $('#' + param_name).val();
            if (value !== new_params[param_name].value) {
                $('#' + param_name).val(new_params[param_name].value);
            }
            UI_GRAPH.updateYAxis();
            UI_GRAPH.changeYZoom();
        }

        SPEC.setCursorsX = function(new_params,param_name) {
            if (new_params[param_name].value == true) {
                $('#'+param_name)['addClass']('active');
            } else {
                $('#'+param_name)['removeClass']('active');
            }
            SPEC.cursorX(new_params)
        }

        SPEC.setCursorsY = function(new_params,param_name) {
            if (new_params[param_name].value == true) {
                $('#'+param_name)['addClass']('active');
            } else {
                $('#'+param_name)['removeClass']('active');
            }
            SPEC.cursorY(new_params)
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
                        CLIENT.parametersCache['FILE_SATTINGS'] = { value: $(this).attr('file_name') };
                        CLIENT.parametersCache['CONTROL_CONFIG_SETTINGS'] = { value: 6 }; // LOAD
                        CLIENT.sendParameters()
                    };
                    img.onclick = function(event) {
                        event.stopPropagation();
                        CLIENT.parametersCache['FILE_SATTINGS'] = { value: $(this).parent().attr('file_name') };
                        CLIENT.parametersCache['CONTROL_CONFIG_SETTINGS'] = { value: 5 }; // DELETE
                        CLIENT.sendParameters()
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

        SPEC.param_callbacks["CH1_SHOW_MIN"] = SPEC.setMinMax;
        SPEC.param_callbacks["CH1_SHOW_MAX"] = SPEC.setMinMax;
        SPEC.param_callbacks["CH2_SHOW_MIN"] = SPEC.setMinMax;
        SPEC.param_callbacks["CH2_SHOW_MAX"] = SPEC.setMinMax;
        SPEC.param_callbacks["CH3_SHOW_MIN"] = SPEC.setMinMax;
        SPEC.param_callbacks["CH3_SHOW_MAX"] = SPEC.setMinMax;
        SPEC.param_callbacks["CH4_SHOW_MIN"] = SPEC.setMinMax;
        SPEC.param_callbacks["CH4_SHOW_MAX"] = SPEC.setMinMax;

        SPEC.param_callbacks["CH1_IN_GAIN"] = SPEC.setGain;
        SPEC.param_callbacks["CH2_IN_GAIN"] = SPEC.setGain;
        SPEC.param_callbacks["CH3_IN_GAIN"] = SPEC.setGain;
        SPEC.param_callbacks["CH4_IN_GAIN"] = SPEC.setGain;

        SPEC.param_callbacks["CH1_IN_FILTER"] = SPEC.setFilter;
        SPEC.param_callbacks["CH2_IN_FILTER"] = SPEC.setFilter;
        SPEC.param_callbacks["CH3_IN_FILTER"] = SPEC.setFilter;
        SPEC.param_callbacks["CH4_IN_FILTER"] = SPEC.setFilter;

        SPEC.param_callbacks["CH1_IN_AC_DC"] = SPEC.setAcDc;
        SPEC.param_callbacks["CH2_IN_AC_DC"] = SPEC.setAcDc;
        SPEC.param_callbacks["CH3_IN_AC_DC"] = SPEC.setAcDc;
        SPEC.param_callbacks["CH4_IN_AC_DC"] = SPEC.setAcDc;

        SPEC.param_callbacks["CH1_PROBE"] = SPEC.setProbe;
        SPEC.param_callbacks["CH2_PROBE"] = SPEC.setProbe;
        SPEC.param_callbacks["CH3_PROBE"] = SPEC.setProbe;
        SPEC.param_callbacks["CH4_PROBE"] = SPEC.setProbe;


        SPEC.param_callbacks["SPEC_RUN"] = SPEC.processRun;
        SPEC.param_callbacks["ADC_FREQ"] = SPEC.adcSpeed;
        SPEC.param_callbacks["RBW"] = SPEC.setRBW;
        SPEC.param_callbacks["SPEC_CURSOR_Y1"] = SPEC.setCursorsY;
        SPEC.param_callbacks["SPEC_CURSOR_Y2"] = SPEC.setCursorsY;
        SPEC.param_callbacks["SPEC_CURSOR_X1"] = SPEC.setCursorsX;
        SPEC.param_callbacks["SPEC_CURSOR_X2"] = SPEC.setCursorsX;

        SPEC.param_callbacks["SOUR1_VOLT"] = GEN.srcVolt;
        SPEC.param_callbacks["SOUR2_VOLT"] = GEN.srcVolt;
        SPEC.param_callbacks["SOUR1_FREQ_FIX"] = GEN.srcFreq;
        SPEC.param_callbacks["SOUR2_FREQ_FIX"] = GEN.srcFreq;
        SPEC.param_callbacks["SOUR1_PHAS"] = GEN.srcPhase;
        SPEC.param_callbacks["SOUR2_PHAS"] = GEN.srcPhase;
        SPEC.param_callbacks["SOUR1_DCYC"] = GEN.srcDCyc;
        SPEC.param_callbacks["SOUR2_DCYC"] = GEN.srcDCyc;
        SPEC.param_callbacks["SOUR1_FUNC"] = GEN.srcFunc;
        SPEC.param_callbacks["SOUR2_FUNC"] = GEN.srcFunc;


        SPEC.param_callbacks["SOUR1_VOLT_OFFS"] = GEN.srcVoltOffset;
        SPEC.param_callbacks["SOUR2_VOLT_OFFS"] = GEN.srcVoltOffset;

        SPEC.param_callbacks["xAxisLogMode"] = SPEC.setXAxisMode;
        SPEC.param_callbacks["EXT_CLOCK_ENABLE"] = SPEC.setPllMode;
        SPEC.param_callbacks["SOUR1_IMPEDANCE"] = GEN.sour1Imp;
        SPEC.param_callbacks["SOUR2_IMPEDANCE"] = GEN.sour2Imp;
        SPEC.param_callbacks["SOUR1_SWEEP_STATE"] = GEN.sweepResetButton;
        SPEC.param_callbacks["SOUR2_SWEEP_STATE"] = GEN.sweepResetButton;
        SPEC.param_callbacks["SOUR1_SWEEP_FREQ_START"] = GEN.sweepFreq;
        SPEC.param_callbacks["SOUR2_SWEEP_FREQ_START"] = GEN.sweepFreq;
        SPEC.param_callbacks["SOUR1_SWEEP_FREQ_END"] = GEN.sweepFreq;
        SPEC.param_callbacks["SOUR2_SWEEP_FREQ_END"] = GEN.sweepFreq;
        SPEC.param_callbacks["SOUR1_SWEEP_TIME"] = GEN.sweepTime;
        SPEC.param_callbacks["SOUR2_SWEEP_TIME"] = GEN.sweepTime;
        SPEC.param_callbacks["SOUR1_SWEEP_MODE"] = GEN.sweepMode;
        SPEC.param_callbacks["SOUR2_SWEEP_MODE"] = GEN.sweepMode;
        SPEC.param_callbacks["SOUR1_SWEEP_REP"] = GEN.sweepRep;
        SPEC.param_callbacks["SOUR2_SWEEP_REP"] = GEN.sweepRep;
        SPEC.param_callbacks["SOUR1_SWEEP_DIR"] = GEN.sweepDir;
        SPEC.param_callbacks["SOUR2_SWEEP_DIR"] = GEN.sweepDir;
        SPEC.param_callbacks["SOUR1_SWEEP_INF"] = GEN.sweepInf;
        SPEC.param_callbacks["SOUR2_SWEEP_INF"] = GEN.sweepInf;

        SPEC.param_callbacks["SOUR1_RISE"] = GEN.riseFallTime;
        SPEC.param_callbacks["SOUR1_FALL"] = GEN.riseFallTime;
        SPEC.param_callbacks["SOUR2_RISE"] = GEN.riseFallTime;
        SPEC.param_callbacks["SOUR2_FALL"] = GEN.riseFallTime;
        SPEC.param_callbacks["RP_MODEL_STR"] = SPEC.setModel;
        SPEC.param_callbacks["ADC_COUNT"] = SPEC.setADCCount;
        SPEC.param_callbacks["CONTROL_CONFIG_SETTINGS"] = SPEC.controlSettingsRequest;
        SPEC.param_callbacks["LIST_FILE_SATTINGS"] = SPEC.listSettings;



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

        SPEC.param_callbacks["xmin"] = function(new_params){
            var field = $('#xmin');
            if (field.is(':focus'))
                field.val(new_params["xmin"].value);
            else
                field.val(SPEC.convertFreqToText(new_params["xmin"].value));
            field.attr('raw_val',new_params["xmin"].value)
            UI_GRAPH.resetZoom();
        };

        SPEC.param_callbacks["xmax"] = function(new_params){
            var field = $('#xmax');
            if (field.is(':focus'))
                field.val(new_params["xmax"].value);
            else
                field.val(SPEC.convertFreqToText(new_params["xmax"].value));
            field.attr('raw_val',new_params["xmax"].value)
            UI_GRAPH.resetZoom();
        };

        SPEC.param_callbacks["y_axis_mode"] = function(new_params){
            var z = "dbm";
            if (new_params['y_axis_mode'].value ===1) z = "v";
            if (new_params['y_axis_mode'].value ===2) z = "dbu";
            if (new_params['y_axis_mode'].value ===3) z = "dbV";
            if (new_params['y_axis_mode'].value ===4) z = "dbuV";
            if (new_params['y_axis_mode'].value ===5) z = "mW";
            if (new_params['y_axis_mode'].value ===6) z = "dBW";
            $('#BDM_DBU_FUNC').val(z);
            SPEC.processSignals(SPEC.latest_signal)
            UI_GRAPH.changeYAxisMode(z);
            UI_GRAPH.updateYAxis();
        }

        SPEC.param_callbacks["DBU_IMP_FUNC"] = function(new_params,param_name){
            $('#' + param_name).val(new_params[param_name].value);
        }

        SPEC.param_callbacks["SPEC_WINDOW_MODE"] = function(new_params,param_name){
            $('#' + param_name).val(new_params[param_name].value);
        }

        SPEC.param_callbacks["SPEC_BUFFER_SIZE"] = function(new_params,param_name){
            $('#' + param_name).val(new_params[param_name].value);
        }

        SPEC.param_callbacks["SPEC_CUT_DC"] = function(new_params,param_name){
            if (new_params[param_name].value == true) {
                $('#'+param_name)['addClass']('active');
            } else {
                $('#'+param_name)['removeClass']('active');
            }
        }

        SPEC.param_callbacks["CH1_OUT_GAIN"] = function(new_params){
            SPEC.processParametersZ250('CH1_OUT_GAIN', new_params['CH1_OUT_GAIN'].value);
        }

        SPEC.param_callbacks["CH2_OUT_GAIN"] = function(new_params){
            SPEC.processParametersZ250('CH2_OUT_GAIN', new_params['CH2_OUT_GAIN'].value);
        }

        SPEC.param_callbacks["EXT_CLOCK_LOCKED"] = function(new_params){
            SPEC.updateExtClockLocked(new_params['EXT_CLOCK_LOCKED'].value);
        }


        SPEC.param_callbacks["OUTPUT1_STATE"] = SPEC.setOutState;
        SPEC.param_callbacks["OUTPUT2_STATE"] = SPEC.setOutState;


        // Stub functions, these parameters must be processed at the very beginning
        SPEC.param_callbacks["RP_MODEL"] = function(){}
        SPEC.param_callbacks["ARB_LIST"] = function(){}
        SPEC.param_callbacks["SOUR_VOLT_MAX"] = function(){}
        SPEC.param_callbacks["SOUR_IMPEDANCE_Z_MODE"] = function(){}
        SPEC.param_callbacks["SOUR1_IMPEDANCE"] = function(){}
        SPEC.param_callbacks["SOUR2_IMPEDANCE"] = function(){}
        SPEC.param_callbacks["SWEEP_RESET"] = function(){}
        SPEC.param_callbacks["SOUR_X5_GAIN"] = function(){}
        SPEC.param_callbacks["SPEC_IS_FILTER"] = function(){}
        SPEC.param_callbacks["SPEC_IS_AC_DC"] = function(){}
        SPEC.param_callbacks["SPEC_IS_HV_LV"] = function(){}
        SPEC.param_callbacks["FILE_SATTINGS"] = function(){}
        SPEC.param_callbacks["SPEC_CUR1_V"] = function(){}
        SPEC.param_callbacks["SPEC_CUR2_V"] = function(){}
        SPEC.param_callbacks["SPEC_CUR1_T"] = function(){}
        SPEC.param_callbacks["SPEC_CUR2_T"] = function(){}

        SPEC.param_callbacks["out_command"] = function(){}
        SPEC.param_callbacks["requestFullData"] = function(){}


        // Processes newly received values for parameters
        SPEC.processParameters = function(new_params) {
            var old_params = $.extend(true, {}, CLIENT.params.orig);

            if (new_params['SOUR_VOLT_MAX']){
                SPEC.gen_max_amp = new_params['SOUR_VOLT_MAX'].value
            }

            if (new_params['SOUR_IMPEDANCE_Z_MODE']){
                SPEC.hi_z_mode = new_params['SOUR_IMPEDANCE_Z_MODE'].value
            }

            if (new_params['ARB_LIST'] && SPEC.arb_list === undefined){
                SPEC.arb_list = new_params['ARB_LIST'].value;
                if (SPEC.arb_list !== "")
                    UI.updateARBFunc(SPEC.arb_list)
            }

            if (new_params['SOUR1_IMPEDANCE'] && new_params['SOUR1_IMPEDANCE'].value != undefined) {
                UI.updateMaxLimitOnLoad("CH1", new_params['SOUR1_IMPEDANCE'].value);
            }

            if (new_params['SOUR2_IMPEDANCE'] && new_params['SOUR2_IMPEDANCE'].value != undefined) {
                UI.updateMaxLimitOnLoad("CH2", new_params['SOUR2_IMPEDANCE'].value);
            }

            if (new_params['RP_MODEL']){
                SPEC.rp_model_id = new_params['RP_MODEL'].value
                setBoardPinOut(SPEC.rp_model_id)
            }

            if (!new_params['RP_MODEL_STR']){
                if (SPEC.rp_model === undefined){
                    CLIENT.requestParameters();
                    return;
                }
            }

            for (var param_name in new_params) {
                CLIENT.params.orig[param_name] = new_params[param_name];
                if (SPEC.param_callbacks[param_name] !== undefined){
                    SPEC.param_callbacks[param_name](new_params,param_name);
                    continue;
                }else{
                    console.log("Missing callback for " + param_name)
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

            if (new_signals['ch_xaxis'] == undefined) {
                return;
            }

            let ch_xaxis = new_signals['ch_xaxis']
            let pointArr = []
            let pointWaterfall = {}

            // Draw first min and max signals
            for (sig_name in new_signals) {
                // Ignore empty signals
                if (new_signals[sig_name] == undefined || new_signals[sig_name].size == 0 || !SPEC.isVisibleSignal(sig_name)) {
                    continue;
                }

                if (new_signals['settings'] && new_signals['settings'].value[0] !== CLIENT.getValue('y_axis_mode')) {
                    continue;
                }

                if (sig_name.includes('_min') || sig_name.includes('_max')){
                    let color = UI_GRAPH.graph_colors[sig_name]
                    pointArr.push({data: undefined, data_gl_x: ch_xaxis.value, data_gl_y:new_signals[sig_name].value, lines: { show_gl:true }, color : color });
                }
            }

            for (sig_name in new_signals) {
                // Ignore empty signals
                if (new_signals[sig_name] == undefined || new_signals[sig_name].size == 0 || !SPEC.isVisibleSignal(sig_name)) {
                    continue;
                }

                if (new_signals['settings'] && new_signals['settings'].value[0] !== CLIENT.getValue('y_axis_mode')) {
                    continue;
                }

                if (!(sig_name.includes('_min') || sig_name.includes('_max'))){
                    let color = UI_GRAPH.graph_colors[sig_name]
                    pointArr.push({data: undefined, data_gl_x: ch_xaxis.value, data_gl_y:new_signals[sig_name].value, lines: { show_gl:true }, color : color });
                }

                // Update watefalls
                if (SPEC.waterfalls.length > 0 && SPEC.isVisibleChannels()){
                    if (sig_name == 'ch1_view' || sig_name == 'ch2_view' || sig_name == 'ch3_view' || sig_name == 'ch4_view') {
                        pointWaterfall[sig_name] = new_signals[sig_name].value
                    }
                }
            }

            if (pointArr.length == 0){
                // Draw empty signal
                pointArr.push({data: undefined, data_gl_x: ch_xaxis.value, data_gl_y:undefined, lines: { show_gl:true }, color : "#00000000" });
            }
            //
            if (UI_GRAPH.x_axis_mode === 1) {
                SPEC.updateXInfo();
            }

            UI_GRAPH.unlockUpdateYLimit();

            if (SPEC.graphs && SPEC.graphs.elem) {
                let minxNonZero = 0
                for (var i = 0; i < ch_xaxis.value.length; i++) {
                    if (minxNonZero != ch_xaxis.value[i]){
                        minxNonZero = ch_xaxis.value[i]
                        break
                    }
                }
                var axes = SPEC.graphs.plot.getAxes();
                axes.xaxis.firstNonzero = minxNonZero
                SPEC.graphs.elem.show();
                SPEC.graphs.plot.setData(pointArr);
                SPEC.graphs.plot.resize();
                if (SPEC.glMode && SPEC.glMode.isInit){
                    var c_w = SPEC.graphs.plot.width()
                    var c_h = SPEC.graphs.plot.height()
                    SPEC.glMode.setNewSizeWGL(c_w,c_h)
                }
                SPEC.graphs.plot.setupGrid();
                SPEC.graphs.plot.draw();
                SPEC.updateCursors();


                for(let ch = 1; ch <= 4; ch++){
                    let chname = 'ch' + ch + '_view'
                    if (chname in pointWaterfall) {
                        let ww = $('#waterfall_ch' + ch).width()
                        SPEC.waterfalls[ch-1].setSize(ww, 60);
                        if (SPEC.freeze[ch-1] == false) {
                            SPEC.waterfalls[ch - 1].addData(SPEC.graphs.plot, ch_xaxis.value, pointWaterfall[chname]);
                        }
                        SPEC.waterfalls[ch-1].draw();
                    }
                }
            } else {
                if ($('#graph_grid').length){
                    SPEC.graphs.elem = $('<div class="plot" />').css($('#graph_grid').css(['width', 'height'])).appendTo('#graphs');
                    SPEC.graphs.plot = $.plot(SPEC.graphs.elem, pointArr, {
                        series: {
                            shadowSize: 0, // Drawing is faster without shadows
                            lineWidth: 1,
                            lines: {
                                lineWidth: 1
                            }
                        },
                        yaxis: {
                            min: -130,
                            max: 20,
                            tickFormatter: function (val, axis) {
                                return Number.parseFloat(val).toFixed(2);
                            }
                        },
                        xaxis: {
                            min: 0,
                            ticks: UI_GRAPH.getXAxisScale ,
                            tickFormatter: UI_GRAPH.funcxTickFormat,
                            transform: function(v) {
                                if (SPEC.graphs.plot == undefined) return v
                                if (CLIENT.getValue("xAxisLogMode") !== 0 && v !== 0){
                                    var axes = SPEC.graphs.plot.getAxes();
                                    let rmin = CLIENT.getValue("xmin")
                                    let min = rmin != 0 ? rmin : axes.xaxis.firstNonzero
                                    let max = CLIENT.getValue("xmax")
                                    return UI_GRAPH.convertXLog(v,min, max);
                                }
                                else
                                    return v;
                            },
                            inverseTransform: function(v){
                                if (SPEC.graphs.plot == undefined) return v
                                if (CLIENT.getValue("xAxisLogMode") !== 0 && v !== 0){
                                    var axes = SPEC.graphs.plot.getAxes();
                                    let rmin = CLIENT.getValue("xmin")
                                    let min = rmin != 0 ? rmin : axes.xaxis.firstNonzero
                                    let max = CLIENT.getValue("xmax")
                                    return UI_GRAPH.reverseXLog(v,min, max);
                                }
                                else
                                    return v;
                            }
                        },
                        yaxes: [
                            { font: { color: "#ffffffff" } }
                        ],
                        xaxes: [
                            { font: { color: "#ffffffff" } }
                        ],
                        grid: {
                            show: true,
                            borderColor: '#888888',
                            tickColor: '#888888',
                            labelMargin:12
                        },
                        hooks: {
                            drawSeries: function(plot, ctx, series) {
                                if (SPEC.glMode && SPEC.glMode.isInit){
                                    SPEC.glMode.drawSeries(plot, ctx, series)
                                }
                            }
                        },
                    });

                    if (UI_GRAPH.updateZoom()){
                        const observer = new MutationObserver(function(mutations) {
                            mutations.forEach(function(mutation) {
                                if (mutation.addedNodes.length || mutation.removedNodes.length) {
                                    SPEC.updateWaterfallLabels();
                                }
                            });
                        });

                        observer.observe(document.getElementsByClassName('x1Axis')[0], {
                            childList: true,
                            subtree: true
                        });
                    }

                    SPEC.glMode = new GLMode()
                    SPEC.glMode.init()

                    SPEC.updateWaterfallWidth();
                    reset_zoom_flag = true;
                }

            }
            $('.pull-right').show();

            // Need reset if Y axis changed or after init
            if (reset_zoom_flag) {
                UI_GRAPH.resetZoom();
            }
        };

    // Exits from editing mode
    SPEC.exitEditing = function(noclose,key) {
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

            if (key.includes('_IN_GAIN1')) key = key.slice(0, -1);
            if (key.includes('_IN_FILTER1')) key = key.slice(0, -1);
            if (key.includes('_IN_AC_DC1')) key = key.slice(0, -1);

            if (key in CLIENT.params.orig) {

                if (key == 'xmin' || key == 'xmax') return;
                if (key == 'CH1_FREEZE' || key == 'CH2_FREEZE' || key == 'CH3_FREEZE' || key == 'CH4_FREEZE') return;
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
                    CLIENT.client_log(key + ' changed from ' + CLIENT.params.orig[key].value + ' to ' + ($.type(CLIENT.params.orig[key].value) == 'boolean' ? !!value : value));
                    CLIENT.parametersCache[key] = { value: ($.type(CLIENT.params.orig[key].value) == 'boolean' ? !!value : value) };
                }
            }

        if (needUpdateYAxis){
            UI_GRAPH.updateYAxis();
        }

        if (needResetZoom) {
            UI_GRAPH.resetZoom();
        }

        // Send params then reset editing state and hide dialog
        CLIENT.sendParameters();
        if (noclose) return;
        $('.dialog:visible').hide();
        $('#right_menu').show();
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

    SPEC.updateWaterfallWidth = function(needDraw) {
        var newh  = 95 * SPEC.visibleCount() + 10;
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
        margins.marginRight = (offset.right + 20) + 'px';
        $('.waterfall-holder').css(margins);
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
            modifynode.css("top","65px")
            let l = parseInt(modifynode.css("left")) - 15;
            modifynode.css("left",l + "px")
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
            SPEC.fireEvent(a, 'click');
        });
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

    // Bind to the window resize event to redraw the graph; trigger that event to do the first drawing
    $(window).resize(function() {
        var root_plot = $('#root_plot');
        var graph_grid = $('#graph_grid');
        var css = root_plot.css(['width', 'height']);

        if (css){
            graph_grid.css(css);

            var plot = SPEC.getPlot();
            if (plot){
                var css = root_plot.css(['width', 'height']);
                $('.plot').css(css);
                SPEC.updateWaterfallWidth(false);
            }

            SPEC.initCursors()
            SPEC.updateJoystickPosition()
            SPEC.processSignals(SPEC.latest_signal)
        }

        if (SPEC.resizeTimer) {
            clearTimeout(SPEC.resizeTimer);
        }
        SPEC.resizeTimer = setTimeout(() => {
            setBoardPinOut(SPEC.rp_model_id)
        }, 500);
    }).resize();



    // Init help
    Help.init(helpListSpectrum);
    Help.setState("idle");

    SPEC.previousPageUrl = document.referrer;
    console.log(`Previously visited page URL: ${SPEC.previousPageUrl}`);

    const currentUrl = window.location.href;
    if (currentUrl === SPEC.previousPageUrl || SPEC.previousPageUrl === ''){
        SPEC.previousPageUrl = '/'
    }
})
