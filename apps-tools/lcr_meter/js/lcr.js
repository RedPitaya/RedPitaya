/*
 * Red Pitaya LCR meter client
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function() {
    $.fn.rotate = function(degrees, step, current) {
        var self = $(this);
        current = current || 0;
        step = step || 5;
        current += step;
        self.css({
            '-webkit-transform' : 'rotate(' + current + 'deg)',
            '-moz-transform' : 'rotate(' + current + 'deg)',
            '-ms-transform' : 'rotate(' + current + 'deg)',
            'transform' : 'rotate(' + current + 'deg)'
        });
        if (current != degrees) {
            setTimeout(function() {
                self.rotate(degrees, step, current);
            }, 5);
        }
    };
})();

(function(LCR, $, undefined) {

    //Configure APP
    LCR.startTime = 0;

    // App state
    LCR.state = {
        socket_opened: false,
        processing: false
    };


    // Other global variables
    LCR.touch = {};

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
        //How much data to be stored.
        max_store: 100000,
        curr_store: 1,
        last_log:0
    }

    LCR.precalculate = {
        prim_value:'',
        prim_units:'',
        prim_raw:'',
        sec_value:'',
        sec_units:'',
        sec_raw:''
    }

    LCR.selected_meas = 1;

    LCR.module_disconnected = false;
    LCR.modal_opened = false;
    LCR.last_update = 0;

    LCR.param_callbacks = {};

    LCR.recalc_params = ['LCR_Z','LCR_L','LCR_C',
                        'LCR_R','LCR_P','LCR_D',
                        'LCR_Q','LCR_ESR','LCR_Z_MIN',
                        'LCR_Z_MAX','LCR_Z_AVG',
                        'LCR_L_MIN','LCR_L_MAX',
                        'LCR_L_AVG','LCR_C_MIN',
                        'LCR_C_MAX','LCR_C_AVG',
                        'LCR_R_MIN','LCR_R_MAX','LCR_R_AVG',
                        'LCR_RELATIVE_VALUE'];

    LCR.getShuntName = function() {
        let shunt_text = "ERROR";
        if (CLIENT.params.orig['LCR_SHUNT'] !== undefined){
            switch (CLIENT.params.orig['LCR_SHUNT'].value) {
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
        }
        return shunt_text
    }

    LCR.getKeyByPrimDisplayIndex = function(index){
        switch(index){
            case 0: return 'Z'
            case 1: return 'L'
            case 2: return 'C'
            case 3: return 'R'
            default: return ''
        }
    }

    LCR.getKeyByPrimDisplayUnit = function(index){
        switch(index){
            case 0: return 'Ω'
            case 1: return 'H'
            case 2: return 'F'
            case 3: return 'Ω'
            default: return ''
        }
    }

    LCR.getKeyBySecDisplayIndex = function(index){
        switch(index){
            case 0: return 'P'
            case 1: return 'D'
            case 2: return 'Q'
            case 3: return 'ESR'
            default: return ''
        }
    }

    LCR.getKeyBySecDisplayUnit = function(index){
        switch(index){
            case 0: return 'deg'
            case 1: return ''
            case 2: return ''
            case 3: return 'Ω'
            default: return ''
        }
    }

    LCR.getFreq = function() {
        let text = "ERROR";
        if (CLIENT.params.orig['LCR_FREQ'] !== undefined){
            var val = CLIENT.params.orig['LCR_FREQ'].value
            if (val < 1000) {
                text = val + " Hz"
            } else if (val < 1000000){
                text = val / 1000 + " kHz"
            } else {
                text = val / 1000000 + " MHz"
            }
        }
        return text
    }

    LCR.processParameters = function(new_params) {

        LCR.recalc_params.forEach((param) => {if (new_params[param]) new_params[param].value /= 100000.0});

        for (var param_name in new_params) {
            CLIENT.params.orig[param_name] = new_params[param_name];

            if (LCR.param_callbacks[param_name] !== undefined){
                LCR.param_callbacks[param_name](new_params,param_name);
                continue;
            }
        }

        if (LCR.data_log.save_data &&
            LCR.data_log.curr_store < LCR.data_log.max_store && $('#LCR_HOLD').is(':visible')) {

            var interval = CLIENT.getValue('LOG_INTERVAL')
            var disPrim = CLIENT.getValue('LCR_PRIM_DISP')
            var disSec = CLIENT.getValue('LCR_SEC_DISP')

            if (interval !== undefined && disPrim !== undefined && disSec !== undefined){
                if (Date.now() - LCR.data_log.last_log > interval){
                    const d = new Date();
                    var date = d.getHours().toString().padStart(2, '0') + ":" + d.getMinutes().toString().padStart(2, '0') + ":"+ d.getSeconds().toString().padStart(2, '0') + ":" + d.getMilliseconds().toString().padStart(3, '0');
                    $('#m_table tbody').prepend('<tr>'+
                        '<td class="table_num_w"><input type="checkbox" name="data"><label class="row_number">' + LCR.data_log.curr_store + '</label></td>'+
                        '<td>' + date + '</td>' +
                        '<td>' + LCR.getShuntName() + '</td>' +
                        '<td>' + LCR.getFreq() + '</td>' +
                        '<td value=' + LCR.precalculate.prim_raw  + '>' + LCR.precalculate.prim_display + ' ' + LCR.precalculate.prim_units + '</td>' +
                        '<td value=' + LCR.precalculate.sec_raw  + '>' + LCR.precalculate.sec_display + ' ' + LCR.precalculate.sec_units + '</td>' +
                        '</tr>');
                    LCR.data_log.curr_store++;
                    LCR.data_log.last_log = Date.now()
                }
            }
        }
    };

    LCR.updateRangeByPrimDisplay = function(){
        var primDis = CLIENT.getValue('LCR_PRIM_DISP')
        var name = LCR.getKeyByPrimDisplayIndex(primDis)
        var unit = LCR.getKeyByPrimDisplayUnit(primDis)

        var i;
        var op = document.getElementById("sel_range_u").getElementsByTagName("option");
        for (i = 0; i < 6; i++) {
            if (i == 3) op[i].text = unit;
            else op[i].text = op[i].text.substring(0, 1) + unit;
        }
    }

    LCR.precalculateValues = function(){
        var primDis = CLIENT.getValue('LCR_PRIM_DISP')
        var secDis = CLIENT.getValue('LCR_SEC_DISP')

        var key = LCR.getKeyByPrimDisplayIndex(primDis)
        if (key !== ''){
            var value = CLIENT.getValue('LCR_'+key)
            if (value !== undefined){
                var rangeMode = CLIENT.getValue('LCR_RANGE')
                if (rangeMode !== undefined){
                    if (rangeMode == 0) { // AUTO mode
                        var new_value = formatRangeAuto(value);
                        var over = checkOverrange(new_value.datanew,key)
                        var rValue = new_value.datanew.toFixed(3)
                        var unit = LCR.getKeyByPrimDisplayUnit(primDis)
                        var valueLabel = rValue
                        var unitLabel = new_value.units+unit
                        if (over.overrange){
                            valueLabel = "OVER R."
                            unitLabel = ""
                            if (over.overrange_dir == 1) unitLabel = "↑"
                            if (over.overrange_dir == -1) unitLabel = "↓"
                        }
                        LCR.precalculate.prim_display = valueLabel
                        LCR.precalculate.prim_units = unitLabel
                        LCR.precalculate.prim_raw = value
                    } else  {
                        var new_value = formatRangeManual(value)
                        var unit = LCR.getKeyByPrimDisplayUnit(primDis)
                        var valueLabel = new_value.datanew
                        var unitLabel = new_value.units+unit
                        if (new_value.overrange){
                            valueLabel = "OVER R."
                            unitLabel = ""
                            if (new_value.overrange_dir == 1) unitLabel = "↑"
                            if (new_value.overrange_dir == -1) unitLabel = "↓"
                        }
                        LCR.precalculate.prim_display = valueLabel
                        LCR.precalculate.prim_units = unitLabel
                        LCR.precalculate.prim_raw = value
                    }
                }

            }
        }

        var key = LCR.getKeyBySecDisplayIndex(secDis)
        if (key !== ''){
            var value = CLIENT.getValue('LCR_'+key)
            if (value !== undefined){
                var new_value = { data:value, datanew:value, units:''}
                if (key !== 'P'){
                    new_value = formatRangeAuto(value);
                }
                var over = checkOverrange(new_value.datanew,key)
                var rValue = new_value.datanew.toFixed(3)
                var unit = LCR.getKeyBySecDisplayUnit(secDis)
                var valueLabel = rValue
                var unitLabel = new_value.units+unit
                // if (over.overrange){
                //     valueLabel = "OVER R."
                //     unitLabel = ""
                //     if (over.overrange_dir == 1) unitLabel = "↑"
                //     if (over.overrange_dir == -1) unitLabel = "↓"
                // }
                LCR.precalculate.sec_display = valueLabel
                LCR.precalculate.sec_units = unitLabel
                LCR.precalculate.sec_raw = value

            }
        }
    }

    LCR.updateMainDisplay = function(){
        LCR.precalculateValues()
        if ((Date.now() - LCR.last_update) > 300){ // 3 FPS

            var primDis = CLIENT.getValue('LCR_PRIM_DISP')
            var secDis = CLIENT.getValue('LCR_SEC_DISP')

            if(primDis !== undefined){
                $("#meas_p_d").html(LCR.getKeyByPrimDisplayIndex(primDis))
            }

            if(secDis !== undefined){
                $("#meas_s_d").html(LCR.getKeyBySecDisplayIndex(secDis))
            }


            $('#lb_prim_displ').html(LCR.precalculate.prim_display)
            $('#lb_prim_displ_units').html(LCR.precalculate.prim_units);

            $('#lb_sec_displ').html(LCR.precalculate.sec_display)
            $('#lb_sec_displ_units').html(LCR.precalculate.sec_units);

            LCR.updateMinMaxAVG()
            LCR.setToleranceValue()
            LCR.setRelativeValue()

            LCR.last_update = Date.now()
        }
    }

    LCR.setPrimDisplay = function(new_params,name) {
        var dis = CLIENT.getValue('LCR_PRIM_DISP')
        $('#prim_displ_choice').find('input[type=checkbox]').prop('checked', false);
        $('#prim_displ_choice').find('input[index='+dis+']').prop('checked', true);
        LCR.updateMainDisplay()
        LCR.updateRangeByPrimDisplay()
        document.getElementById('table_p_header').innerHTML = LCR.getKeyByPrimDisplayIndex(dis)

    }

    LCR.setSecondDisplay = function(new_params,name) {
        var dis = CLIENT.getValue('LCR_SEC_DISP')
        $('#sec_displ_choice').find('input[type=checkbox]').prop('checked', false);
        $('#sec_displ_choice').find('input[index='+dis+']').prop('checked', true);
        LCR.updateMainDisplay()
        document.getElementById('table_s_header').innerHTML = LCR.getKeyBySecDisplayIndex(dis)
    }

    LCR.setRange = function(new_params,name) {
        var mode = CLIENT.getValue('LCR_RANGE')
        $('#sel_mode').find('input[type=checkbox]').prop('checked', false);
        $('#sel_mode').find('input[index='+mode+']').prop('checked', true);
        $('#meas_mode_d').html(mode == 0 ? 'Auto' : 'Manual');
    }

    LCR.setRangeF = function(new_params,name) {
        var mode = CLIENT.getValue('LCR_RANGE_F')
        if (mode !== undefined){
            $('#sel_range_f').val(mode);
            LCR.updateMainDisplay()
        }
    }

    LCR.setRangeU = function(new_params,name) {
        var mode = CLIENT.getValue('LCR_RANGE_U')
        if (mode !== undefined){
            $('#sel_range_u').val(mode);
            LCR.updateMainDisplay()
        }
    }

    LCR.setSeries = function(new_params,name) {
        var mode = CLIENT.getValue('LCR_SERIES')
        $('#parl_ser').find('input[type=checkbox]').prop('checked', false);
        $('#parl_ser').find('input[index='+mode+']').prop('checked', true);
    }

    LCR.setShunt = function(new_params,name) {
        let shunt_text = LCR.getShuntName();
        $('#lb_shunt').empty().append(shunt_text);
    }

    LCR.setShuntMode = function(new_params,name) {
        var shunt = CLIENT.getValue('LCR_SHUNT_MODE')
        if (shunt !== undefined){
            $('#LCR_SHUNT_MODE').val(shunt);
        }
    }

    LCR.setLogIntreval = function(new_params,name) {
        var interval = CLIENT.getValue('LOG_INTERVAL')
        if (interval !== undefined){
            $('#LOG_INTERVAL').val(interval);
        }
    }

    LCR.setRun = function(new_params,name) {

    }

    LCR.setFreq = function(new_params,name) {
        var freq = CLIENT.getValue('LCR_FREQ')
        if (freq !== undefined){
            $('#meas_freq_d').html(LCR.getFreq());
            $('#LCR_FREQUENCY').val(freq);
        }
    }

    LCR.setTolerance = function(new_params,name) {
        var status = CLIENT.getValue('LCR_TOLERANCE')
        $("#TOL_CAPTION").css('opacity', status ? 1 : 0.3 );
        $("#TOL_VALUE").css('opacity', status ? 1 : 0.3 );
        if (!status)
            $("#TOL_VALUE").html('-');
    }

    LCR.setToleranceValue = function() {
        var status = CLIENT.getValue('LCR_TOLERANCE')
        var value = CLIENT.getValue('LCR_TOLERANCE_VALUE')
        if (value !== undefined){
            value = Math.round(value)
            if (value < -100)
                value = -100
            if (status)
                $("#TOL_VALUE").html(value + ' %');
            else
                $("#TOL_VALUE").html('-');
        }
    }

    LCR.setRelative = function(new_params,name) {
        var status = CLIENT.getValue('LCR_RELATIVE')
        $("#REL_CAPTION").css('opacity', status ? 1 : 0.3 );
        $("#REL_VALUE").css('opacity', status ? 1 : 0.3 );
        if (!status)
            $("#REL_VALUE").html('-');
    }

    LCR.setRelativeValue = function() {
        var primDis = CLIENT.getValue('LCR_PRIM_DISP')
        var status = CLIENT.getValue('LCR_RELATIVE')
        var value = CLIENT.getValue('LCR_RELATIVE_VALUE')
        if (value !== undefined && primDis !== undefined){
            if (status){
                var new_value = formatRangeAuto(value);
                var rValue = new_value.datanew.toFixed(3)
                var unit = LCR.getKeyByPrimDisplayUnit(primDis)
                var valueLabel = rValue
                var unitLabel = new_value.units+unit
                $('#REL_VALUE').html(valueLabel + ' ' + unitLabel)
            }
            else
                $("#REL_VALUE").html('-');
        }
    }

    LCR.setExtModule = function(new_params,name) {
        var status = CLIENT.getValue('LCR_EXTMODULE_STATUS')
        if (status !== undefined){
            if (status === false) {
                $('body').removeClass('loaded');
                $('#text_message').show();
                $('#cont').hide();
                LCR.module_disconnected = true;
            } else {
                if (LCR.module_disconnected == true) {
                    CLIENT.startApp();
                    LCR.module_disconnected = false;
                    return;
                }
                $('#text_message').hide();
                $('#cont').show();
            }
        }
    }

    LCR.setLCRz = function(new_params,name) {
        if (LCR.getKeyByPrimDisplayIndex(CLIENT.getValue('LCR_PRIM_DISP')) === 'Z')
            LCR.updateMainDisplay()
    }

    LCR.setLCRl = function(new_params,name) {
        if (LCR.getKeyByPrimDisplayIndex(CLIENT.getValue('LCR_PRIM_DISP')) === 'L')
            LCR.updateMainDisplay()
    }

    LCR.setLCRc = function(new_params,name) {
        if (LCR.getKeyByPrimDisplayIndex(CLIENT.getValue('LCR_PRIM_DISP')) === 'C')
            LCR.updateMainDisplay()
    }

    LCR.setLCRr = function(new_params,name) {
        if (LCR.getKeyByPrimDisplayIndex(CLIENT.getValue('LCR_PRIM_DISP')) === 'R')
            LCR.updateMainDisplay()
    }

    LCR.setLCRp = function(new_params,name) {
        if (LCR.getKeyBySecDisplayIndex(CLIENT.getValue('LCR_SEC_DISP')) === 'P')
            LCR.updateMainDisplay()
    }

    LCR.setLCRd = function(new_params,name) {
        if (LCR.getKeyBySecDisplayIndex(CLIENT.getValue('LCR_SEC_DISP')) === 'D')
            LCR.updateMainDisplay()
    }

    LCR.setLCRq = function(new_params,name) {
        if (LCR.getKeyBySecDisplayIndex(CLIENT.getValue('LCR_SEC_DISP')) === 'Q')
            LCR.updateMainDisplay()
    }

    LCR.setLCResr = function(new_params,name) {
        if (LCR.getKeyBySecDisplayIndex(CLIENT.getValue('LCR_SEC_DISP')) === 'ESR')
            LCR.updateMainDisplay()
    }

    LCR.controlSettingsRequest = function(new_params){
        if (new_params['CONTROL_CONFIG_SETTINGS'].value === 2) {  // RESET_DONE
            location.reload();
        }

        if (new_params['CONTROL_CONFIG_SETTINGS'].value === 7) {  // LOAD_DONE
            location.reload();
        }
    }

    LCR.updateMinMaxAVG = function() {

        function set(value,mode,key)
        {
            if (value !== undefined){
                var rangeMode = CLIENT.getValue('LCR_RANGE')
                if (rangeMode !== undefined){
                    if (rangeMode == 0) { // AUTO mode
                        var new_value = formatRangeAuto(value);
                        var over = checkOverrange(new_value.datanew,key)
                        var rValue = new_value.datanew.toFixed(3)
                        var unit = LCR.getKeyByPrimDisplayUnit(primDis)
                        var valueLabel = rValue
                        var unitLabel = new_value.units+unit
                        if (over.overrange){
                            valueLabel = "OVER R."
                            unitLabel = ""
                            if (over.overrange_dir == 1) unitLabel = "↑"
                            if (over.overrange_dir == -1) unitLabel = "↓"
                        }
                        $('#meas_'+mode+'_d').html(valueLabel + " " + unitLabel)
                    } else  {
                        var new_value = formatRangeManual(value)
                        var unit = LCR.getKeyByPrimDisplayUnit(primDis)
                        var valueLabel = new_value.datanew
                        var unitLabel = new_value.units+unit
                        if (new_value.overrange){
                            valueLabel = "OVER R."
                            unitLabel = ""
                            if (new_value.overrange_dir == 1) unitLabel = "↑"
                            if (new_value.overrange_dir == -1) unitLabel = "↓"
                        }
                        $('#meas_'+mode+'_d').html(valueLabel + " " + unitLabel)
                    }
                }
            }
        }

        var primDis = CLIENT.getValue('LCR_PRIM_DISP')
        var key = LCR.getKeyByPrimDisplayIndex(primDis)
        if (key !== ''){
            set(CLIENT.getValue('LCR_'+key+"_MIN"),'min',key)
            set(CLIENT.getValue('LCR_'+key+"_MAX"),'max',key)
            set(CLIENT.getValue('LCR_'+key+"_AVG"),'avg',key)
        }
    }

    LCR.param_callbacks["LCR_PRIM_DISP"] = LCR.setPrimDisplay;
    LCR.param_callbacks["LCR_SEC_DISP"] = LCR.setSecondDisplay;
    LCR.param_callbacks["LCR_SHUNT"] = LCR.setShunt;
    LCR.param_callbacks["LCR_SHUNT_MODE"] = LCR.setShuntMode;
    LCR.param_callbacks["LCR_RUN"] = LCR.setRun;
    LCR.param_callbacks["LCR_TOLERANCE"] = LCR.setTolerance;
    LCR.param_callbacks["LCR_TOLERANCE_VALUE"] = LCR.setPrimDisplay;
    LCR.param_callbacks["LCR_RELATIVE"] = LCR.setRelative;
    LCR.param_callbacks["LCR_RELATIVE_VALUE"] = LCR.setPrimDisplay;
    LCR.param_callbacks["LCR_EXTMODULE_STATUS"] = LCR.setExtModule;
    LCR.param_callbacks["LCR_RANGE"] = LCR.setRange;
    LCR.param_callbacks["LCR_RANGE_F"] = LCR.setRangeF;
    LCR.param_callbacks["LCR_RANGE_U"] = LCR.setRangeU;
    LCR.param_callbacks["LCR_SERIES"] = LCR.setSeries;

    LCR.param_callbacks["LCR_Z"] = LCR.setLCRz;
    LCR.param_callbacks["LCR_L"] = LCR.setLCRl;
    LCR.param_callbacks["LCR_C"] = LCR.setLCRc;
    LCR.param_callbacks["LCR_R"] = LCR.setLCRr;

    LCR.param_callbacks["LCR_Z_MAX"] = LCR.updateMainDisplay;
    LCR.param_callbacks["LCR_L_MAX"] = LCR.updateMainDisplay;
    LCR.param_callbacks["LCR_C_MAX"] = LCR.updateMainDisplay;
    LCR.param_callbacks["LCR_R_MAX"] = LCR.updateMainDisplay;

    LCR.param_callbacks["LCR_Z_MIN"] = LCR.updateMainDisplay;
    LCR.param_callbacks["LCR_L_MIN"] = LCR.updateMainDisplay;
    LCR.param_callbacks["LCR_C_MIN"] = LCR.updateMainDisplay;
    LCR.param_callbacks["LCR_R_MIN"] = LCR.updateMainDisplay;


    LCR.param_callbacks["LCR_Z_AVG"] = LCR.updateMainDisplay;
    LCR.param_callbacks["LCR_L_AVG"] = LCR.updateMainDisplay;
    LCR.param_callbacks["LCR_C_AVG"] = LCR.updateMainDisplay;
    LCR.param_callbacks["LCR_R_AVG"] = LCR.updateMainDisplay;

    LCR.param_callbacks["LCR_P"] = LCR.setLCRp;
    LCR.param_callbacks["LCR_D"] = LCR.setLCRd;
    LCR.param_callbacks["LCR_Q"] = LCR.setLCRq;
    LCR.param_callbacks["LCR_ESR"] = LCR.setLCResr;

    LCR.param_callbacks["LCR_FREQ"] = LCR.setFreq;

    LCR.param_callbacks["CONTROL_CONFIG_SETTINGS"] = LCR.controlSettingsRequest;

    LCR.param_callbacks["LOG_INTERVAL"] = LCR.setLogIntreval;



}(window.LCR = window.LCR || {}, jQuery));

$(function() {

    //Header options. Prevent aggressive firefox caching
    $("html :checkbox").attr("autocomplete", "off");

    console.log('Processing on site events');

    //LCR set run state
    $('#LCR_START').on('click', function(ev) {
        ev.preventDefault();
        $('#LCR_START').hide();
        $('#LCR_HOLD').css('display', 'block');
        CLIENT.parametersCache['LCR_RUN'] = { value: true };
        CLIENT.sendParameters();
    });

    //Freeze LCR data
    $('#LCR_HOLD').on('click', function(ev) {
        ev.preventDefault();
        $('#LCR_HOLD').hide();
        $('#LCR_START').css('display', 'block');
        CLIENT.parametersCache['LCR_RUN'] = { value: false };
        CLIENT.sendParameters();
    });

    /* --------------------- CALIBRATION --------------------- */
    $('#LCR_CALIBRATE').on('click', function(ev) {
        ev.preventDefault();
        $('#LCR_HOLD').hide();
        $('#LCR_START').css('display', 'block');
        CLIENT.parametersCache['LCR_RUN'] = { value: false };
        CLIENT.sendParameters();
        $('#modal_calib_start').modal('show');
    });

    $('#bt_calib_start').on('click', function(ev) {
        ev.preventDefault();
        CLIENT.parametersCache['LCR_CALIB_MODE'] = { value: 1 };
        CLIENT.parametersCache['LCR_CALIBRATION'] = { value: true };
        $('#modal_calib_start').modal('hide');
        $('#modal_calib_open').modal('show');
        CLIENT.sendParameters();
    });

    $('#bt_calib_open').on('click', function(ev) {
        ev.preventDefault();
        CLIENT.parametersCache['LCR_CALIB_MODE'] = { value: 2 };
        CLIENT.parametersCache['LCR_CALIBRATION'] = { value: true };
        $('#modal_calib_open').modal('hide');
        $('#modal_calib_short').modal('show');
        CLIENT.sendParameters();
        setTimeout(function() {
            return;
        }, 100);
        CLIENT.parametersCache['LCR_CALIB_MODE'] = { value: 0 };
        CLIENT.parametersCache['LCR_CALIBRATION'] = { value: true };
        CLIENT.sendParameters();
    });

    /* ------------------------------------------------------- */

    //Log data
    $('#LCR_LOG').click(function(ev) {
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
        CLIENT.parametersCache['LCR_FREQ'] = { value: parseInt(this.value) };
        CLIENT.sendParameters();
    });

    $('#LCR_SHUNT_MODE').change(function() {
        CLIENT.parametersCache['LCR_SHUNT_MODE'] = { value: parseInt(this.value) };
        CLIENT.sendParameters();
    });

    $('#prim_displ_choice :checkbox').click(function() {
        clearTableAll();
        var index = $(this).attr('index')
        CLIENT.parametersCache['LCR_PRIM_DISP'] = { value: index };
        CLIENT.sendParameters();
    });

    $('#sec_displ_choice :checkbox').click(function() {
        clearTableAll();
        var index = $(this).attr('index')
        CLIENT.parametersCache['LCR_SEC_DISP'] = { value: index };
        CLIENT.sendParameters();
    });

    $('#LCR_LOG').on('click', function() {
        LCR.data_log.save_data = true;
    });

    $('#cb_tol').change(function() {
        CLIENT.parametersCache['LCR_TOLERANCE'] = { value:  $( this ).is( ":checked" )  };
        CLIENT.sendParameters();
    });

    $('#cb_rel').change(function() {
        CLIENT.parametersCache['LCR_RELATIVE'] = { value: $( this ).is( ":checked" ) };
        CLIENT.sendParameters();
    });

    $('#cb_ser').click(function() {
        CLIENT.parametersCache['LCR_SERIES'] = { value: 1 };
        CLIENT.sendParameters();
    });

    $('#cb_paralel').click(function() {
        CLIENT.parametersCache['LCR_SERIES'] = { value: 0 };
        CLIENT.sendParameters();
    });

    $('#cb_manual').click(function() {
        CLIENT.parametersCache['LCR_RANGE'] = { value: 1 }
        CLIENT.parametersCache['LCR_RANGE_F'] = { value: $('#sel_range_f :selected').val() }
        CLIENT.parametersCache['LCR_RANGE_U'] = { value: $('#sel_range_u :selected').val() }
        CLIENT.sendParameters();
    });

    $('#cb_auto').click(function() {
        CLIENT.parametersCache['LCR_RANGE'] = { value: 0 };
        CLIENT.sendParameters();
    });

    $('#sel_range_u').change(function() {
        if (CLIENT.params.orig['LCR_RANGE'].value != 0) {
            CLIENT.parametersCache['LCR_RANGE_U'] = { value: this.value };
            CLIENT.sendParameters();
        }
    });

    $('#sel_range_f').change(function() {
        CLIENT.parametersCache['LCR_RANGE_F'] = { value: this.value };
        CLIENT.sendParameters();
    });

    $('#reset_settings').click(function() {
        CLIENT.parametersCache['CONTROL_CONFIG_SETTINGS'] = { value: 1 }; // REQUEST_RESET
        CLIENT.sendParameters();
    });

    $('#CLEAR_MIN_MAX').click(function() {
        $(this).rotate(360)
        CLIENT.parametersCache['LCR_M_RESET'] = { value: true };
        CLIENT.sendParameters();
    });

    $('#LOG_INTERVAL').change(function() {
        CLIENT.parametersCache['LOG_INTERVAL'] = { value: parseInt(this.value) };
        CLIENT.sendParameters();
    });

    // Init help
    Help.init(helpListLCR);
    Help.setState("idle");

});


function formatRangeAuto(meas_data) {
    if (meas_data == 0) return { data:0, datanew:0, units:''}
    var l = Math.log10(Math.abs(meas_data)) / 3
    var z = Math.floor( l + (Math.abs(l - Math.round(l)) < 0.05 ? 0.05 : 0))
    z = Math.min(Math.max(z,-4),2)
    var devider = Math.pow(1000,z)
    var new_meas_data = meas_data / devider
    var unit = ''
    switch(z){
        case -4:
            unit = 'p'
            break;
        case -3:
            unit = 'n'
            break;
        case -2:
            unit = 'µ'
            break;
        case -1:
            unit = 'm'
            break;
        case 0:
            unit = ''
            break;
        case 1:
            unit = 'k'
            break;
        case 2:
            unit = 'M'
            break;
        }
    return  { data:meas_data, datanew:new_meas_data, units:unit}
}

function checkOverrange(value, param) {
    var min = 0
    var max = 1e9
    var over = false
    var overdir = 0
    if (param == "P"){
        min = -180
        max = 180
    }
    if (value < min) {
        over = true
        overdir = -1
    }
    if (value > max) {
        over = true
        overdir = 1
    }
    return  { datanew:value, overrange: over, overrange_dir: overdir}
}

function formatRangeManual(data) {

    var format = CLIENT.getValue('LCR_RANGE_F')
    var power = CLIENT.getValue('LCR_RANGE_U')

    if (format === undefined || power === undefined) return { data:0, datanew:0, units:'', overrange: true, overrange_dir:0}

    var suffixes = ['n', 'u', 'm', '', 'k', 'M'];
    var limits = [10.0, 100.0, 1000.0, 10000.0];
    var coff = 1e9 / Math.pow(1000,power);
    var datanew = data * coff

    var limit = limits[parseInt(format)];
    if (datanew > limit) {
        return { data:data, datanew:datanew, units:'', overrange: true,overrange_dir:1};
    }

    if (datanew < 0) {
        return { data:data, datanew:datanew, units:'', overrange: true,overrange_dir:-1};
    }

    var iFormat = format + 1
    var dFormat = 5 - iFormat

    var formatFunc = new Intl.NumberFormat('en-US', {
        minimumIntegerDigits: iFormat,
        minimumFractionDigits: dFormat,
        maximumFractionDigits:dFormat,
        roundingMode:"trunc"
    });

    var formatedData = formatFunc.format(datanew).replaceAll(',','')
    return { data:data, datanew:formatedData, units:suffixes[power], overrange: false, overrange_dir:0};

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
    var csv = $('#m_table').table2CSV()
    const link = document.createElement("a");
    link.href = 'data:text/csv;charset=UTF-8,' + encodeURIComponent(csv)
    link.download = new Date().toJSON().slice(0,22) + ".csv";
    link.click();
}