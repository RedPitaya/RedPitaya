/*
 * Red Pitaya main_menu
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


//Bode analyser
(function(MAIN, $, undefined) {


    MAIN.scale = true;
    MAIN.param_callbacks = {};

    MAIN.running = true;
    MAIN.calibrating = false;
    MAIN.unexpectedClose = false;
    MAIN.totalRam = undefined;
    MAIN.freeRam = undefined;
    MAIN.dmaRam = undefined;
    MAIN.totalSD = undefined;
    MAIN.freeSD = undefined;
    MAIN.cpuLoad = undefined;


    MAIN.processSignals = function(SIG){
    }

    MAIN.convertBytes = function(x){
        var z = 0;
        if (x < 1024) return x + " b"
        if (x < (1024 * 1024)) return (x/1024).toFixed(3) +" kb"
        if (x < (1024 * 1024 * 1024)) return (x/(1024 * 1024)).toFixed(3) +" Mb"
        return (x/(1024 * 1024 * 1024)).toFixed(3) +" Gb"
    }

    MAIN.convertBytesRAM = function(x){
        var z = 0;
        if (x < 1024) return x + " b"
        if (x < (1024 * 1024)) return (x/1024).toFixed() +" kb"
        if (x < (1024 * 1024 * 1024)) return (x/(1024 * 1024)).toFixed() +" Mb"
        return (x/(1024 * 1024 * 1024)).toFixed() +" Gb"
    }

    MAIN.processTRam= function(new_params) {
        MAIN.totalRam  = new_params['RP_SYSTEM_TOTAL_RAM'].value
        if (MAIN.freeRam != undefined){
            $('#RAM_SIZE_ID').text(MAIN.convertBytesRAM(MAIN.freeRam) + " / " +  MAIN.convertBytesRAM(MAIN.totalRam));
        }

    }

    MAIN.processFRam= function(new_params) {
        MAIN.freeRam = new_params['RP_SYSTEM_FREE_RAM'].value
        if (MAIN.freeRam != undefined){
            $('#RAM_SIZE_ID').text(MAIN.convertBytesRAM(MAIN.freeRam) + " / " +  MAIN.convertBytesRAM(MAIN.totalRam));
        }
    }

    MAIN.processDMARam= function(new_params) {
        MAIN.dmaRam = new_params['RP_SYSTEM_DMA_RAM'].value
        if (MAIN.dmaRam != undefined){
            $('#DMA_RAM_SIZE_ID').text(MAIN.convertBytesRAM(MAIN.dmaRam));
            $('#RP_DMA_RAM').val(MAIN.dmaRam / (1024 * 1024));
        }
    }

    MAIN.processDDRMAXRam= function(new_params) {
        var maxRam = new_params['RP_SYSTEM_DDR_MAX'].value

        if (maxRam < 1024){
            var nodes = document.getElementsByClassName("ram512");
            [...nodes].forEach((element, index, array) => {
                                        element.parentNode.removeChild(element);
                                    });
        }

        if (maxRam < 512){
            var nodes = document.getElementsByClassName("ram256");
            [...nodes].forEach((element, index, array) => {
                                        element.parentNode.removeChild(element);
                                    });
        }

    }

    MAIN.processTSD= function(new_params) {
        MAIN.totalSD  = new_params['RP_SYSTEM_TOTAL_SD'].value
        if (MAIN.freeSD != undefined){
            $('#SD_SIZE_ID').text(MAIN.convertBytes(MAIN.freeSD) + " / " +  MAIN.convertBytes(MAIN.totalSD));
        }

    }

    MAIN.processFSD= function(new_params) {
        MAIN.freeSD = new_params['RP_SYSTEM_FREE_SD'].value
        if (MAIN.totalSD != undefined){
            $('#SD_SIZE_ID').text(MAIN.convertBytes(MAIN.freeSD) + " / " +  MAIN.convertBytes(MAIN.totalSD));
        }
    }

    MAIN.processCPULoad= function(new_params) {
        MAIN.cpuLoad = new_params['RP_SYSTEM_CPU_LOAD'].value
        if (MAIN.cpuLoad != undefined){
            $('#CPU_LOAD_ID').text(MAIN.cpuLoad.toFixed() + " %");
        }
    }

    MAIN.processTemp= function(new_params) {
        MAIN.cpuLoad = new_params['RP_SYSTEM_TEMPERATURE'].value
        if (MAIN.cpuLoad != undefined){
            $('#TEMP_ID').text(MAIN.cpuLoad.toFixed() + " °C");
        }
    }

    MAIN.processVCC1= function(new_params) {
        MAIN.cpuLoad = new_params['RP_SYSTEM_VCC_PINT'].value
        if (MAIN.cpuLoad != undefined){
            $('#VCCPINT_ID').text(MAIN.cpuLoad.toFixed(3) + " V");
        }
    }

    MAIN.processVCC2= function(new_params) {
        MAIN.cpuLoad = new_params['RP_SYSTEM_VCC_PAUX'].value
        if (MAIN.cpuLoad != undefined){
            $('#VCCPAUX_ID').text(MAIN.cpuLoad.toFixed(3) + " V");
        }
    }

    MAIN.processVCC3= function(new_params) {
        MAIN.cpuLoad = new_params['RP_SYSTEM_VCC_BRAM'].value
        if (MAIN.cpuLoad != undefined){
            $('#VCCBRAM_ID').text(MAIN.cpuLoad.toFixed(3) + " V");
        }
    }

    MAIN.processVCC4= function(new_params) {
        MAIN.cpuLoad = new_params['RP_SYSTEM_VCC_INT'].value
        if (MAIN.cpuLoad != undefined){
            $('#VCCINT_ID').text(MAIN.cpuLoad.toFixed(3) + " V");
        }
    }

    MAIN.processVCC5= function(new_params) {
        MAIN.cpuLoad = new_params['RP_SYSTEM_VCC_AUX'].value
        if (MAIN.cpuLoad != undefined){
            $('#VCCAUX_ID').text(MAIN.cpuLoad.toFixed(3) + " V");
        }
    }

    MAIN.processVCC6= function(new_params) {
        MAIN.cpuLoad = new_params['RP_SYSTEM_VCC_DDR'].value
        if (MAIN.cpuLoad != undefined){
            $('#VCCDDR_ID').text(MAIN.cpuLoad.toFixed(3) + " V");
        }
    }

    MAIN.param_callbacks["RP_SYSTEM_TOTAL_RAM"] = MAIN.processTRam;
    MAIN.param_callbacks["RP_SYSTEM_FREE_RAM"] = MAIN.processFRam;
    MAIN.param_callbacks["RP_SYSTEM_DMA_RAM"] = MAIN.processDMARam;
    MAIN.param_callbacks["RP_SYSTEM_DDR_MAX"] = MAIN.processDDRMAXRam;
    MAIN.param_callbacks["RP_SYSTEM_CPU_LOAD"] = MAIN.processCPULoad;
    MAIN.param_callbacks["RP_SYSTEM_TEMPERATURE"] = MAIN.processTemp;

    MAIN.param_callbacks["RP_SYSTEM_TOTAL_SD"] = MAIN.processTSD;
    MAIN.param_callbacks["RP_SYSTEM_FREE_SD"] = MAIN.processFSD;


    MAIN.param_callbacks["RP_SYSTEM_VCC_PINT"] = MAIN.processVCC1;
    MAIN.param_callbacks["RP_SYSTEM_VCC_PAUX"] = MAIN.processVCC2;
    MAIN.param_callbacks["RP_SYSTEM_VCC_BRAM"] = MAIN.processVCC3;
    MAIN.param_callbacks["RP_SYSTEM_VCC_INT"] = MAIN.processVCC4;
    MAIN.param_callbacks["RP_SYSTEM_VCC_AUX"] = MAIN.processVCC5;
    MAIN.param_callbacks["RP_SYSTEM_VCC_DDR"] = MAIN.processVCC6;

}(window.MAIN = window.MAIN || {}, jQuery));




// Page onload event handler
$(function() {

    $("#RP_DMA_RAM").change(function() {
        var newSize = "0x"+($("#RP_DMA_RAM option:selected").val() * 1024 * 1024).toString(16);
        console.log("Set new DMA size " + newSize )
        $.ajax({
            url: '/resizeDMA?new_size=' + newSize,
            type: 'GET'
        })
    });

});