/*
 * Red Pitaya calib_app
 *
 * Author: Danilyuk Nikolay <n.danilyuk@integrasources.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function(OBJ, $, undefined) {

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


    OBJ.makeid = function(length) {
        var result           = '';
        var characters       = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
        var charactersLength = characters.length;
        for ( var i = 0; i < length; i++ ) {
           result += characters.charAt(Math.floor(Math.random() * charactersLength));
        }
        return result;
     }

}(window.OBJ = window.OBJ || {}, jQuery));


// Page onload event handler
$(function() {

    $('#B_AUTO_MODE').on('click', function(ev) {
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

    
    
});