/*
 * Red Pitaya calib_app
 *
 * Author: Danilyuk Nikolay <n.danilyuk@integrasources.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function(OBJ, $, undefined) {

    OBJ.model = undefined;

    OBJ.setMainMenu = function(_visible) {
        if (_visible) {
            $("#main_menu_body").show();
        }else{
            $("#main_menu_body").hide();
        }
    }

    OBJ.setAutoMode = function(_visible) {
        if (_visible) {
            $("#auto_mode_body").show();
        }else{
            $("#auto_mode_body").hide();
        }
    }

    OBJ.setADCMode = function(_visible) {
        if (OBJ.model !== undefined) {
            if (OBJ.model === "Z10") {
                if (_visible){
                    $("#adc_125_mode_body").show();
                }else{
                    $("#adc_125_mode_body").hide();
                }
            }
            if (OBJ.model === "Z20_250_12") {
                if (_visible){
                    $("#adc_250_mode_body").show();
                }else{
                    $("#adc_250_mode_body").hide();
                }
            }
        }
    }

    OBJ.makeid = function(length) {
        var result           = '';
        var characters       = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
        var charactersLength = characters.length;
        for ( var i = 0; i < length; i++ ) {
           result += characters.charAt(Math.floor(Math.random() * charactersLength));
        }
        return result;
     }

    OBJ.setCH1Awg = function(_value){
        OBJ.amSetCH1Avg(_value.value);    
    }

    OBJ.setCH2Awg = function(_value){
        OBJ.amSetCH2Avg(_value.value);  
    }

    OBJ.setCH1Max = function(_value){
        OBJ.amSetCH1Max(_value.value);    
    }

    OBJ.setCH2Max = function(_value){
        OBJ.amSetCH2Max(_value.value);  
    }

    OBJ.showMainMenu = function(){
        OBJ.setAutoMode(false);
        OBJ.setMainMenu(true);
    }

    OBJ.setModel = function(_value){
        if (OBJ.model === undefined) OBJ.model = _value.value;
        OBJ.amSetModel(_value);
    }

}(window.OBJ = window.OBJ || {}, jQuery));


// Page onload event handler
$(function() {

    $('#B_AUTO_MODE').on('click', function(ev) {
        SM.param_callbacks["ch1_avg"] = OBJ.setCH1Awg;
        SM.param_callbacks["ch2_avg"] = OBJ.setCH2Awg;  
        SM.param_callbacks["ch1_max"] = OBJ.setCH1Max;
        SM.param_callbacks["ch2_max"] = OBJ.setCH2Max;  
        
        OBJ.setMainMenu(false);
        OBJ.setAutoMode(true);
        OBJ.amClearTable();
        OBJ.amStartCalibration();
    });

    $('#B_ADC_MODE').on('click', function(ev) {
        OBJ.setMainMenu(false);
    });

    $('#B_DAC_MODE').on('click', function(ev) {
        OBJ.setMainMenu(false);
    });

    $('#B_CANCEL_CALIB').on('click', function(ev) {
        SM.param_callbacks["ch1_avg"] = undefined;
        SM.param_callbacks["ch2_avg"] = undefined;  
        SM.param_callbacks["ch1_max"] = undefined;
        SM.param_callbacks["ch2_max"] = undefined; 
        OBJ.showMainMenu();
        SM.parametersCache["SS_NEXTSTEP"] = { value: -2 };
        SM.parametersCache["ref_volt"] = {value:0}; // SS_NEXTSTEP work only in pair ref_volt
        SM.sendParameters();
    });

    SM.param_callbacks["RP_MODEL_STR"] = OBJ.setModel;
    

 
    
});