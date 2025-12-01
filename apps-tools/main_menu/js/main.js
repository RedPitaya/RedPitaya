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

    MAIN.cpuTemp = undefined;
    MAIN.VCC1 = undefined;
    MAIN.VCC2 = undefined;
    MAIN.VCC3 = undefined;
    MAIN.VCC4 = undefined;
    MAIN.VCC5 = undefined;
    MAIN.VCC6 = undefined;
    MAIN.VCC6nominal = undefined;

    MAIN.lastRelease = undefined;

    MAIN.adc_base_rate = 0;
    MAIN.dac_base_rate = 0;


    MAIN.processSignals = function(SIG){
    }

    MAIN.convertBytes = function(x){
        var z = 0;
        if (x < 1024) return x + " B"
        if (x < (1024 * 1024)) return (x/1024).toFixed(3) +" kB"
        if (x < (1024 * 1024 * 1024)) return (x/(1024 * 1024)).toFixed(3) +" MB"
        return (x/(1024 * 1024 * 1024)).toFixed(3) +" GB"
    }

    MAIN.convertBytesRAM = function(x){
        var z = 0;
        if (x < 1024) return x + " b"
        if (x < (1024 * 1024)) return (x/1024).toFixed() +" kB"
        if (x < (1024 * 1024 * 1024)) return (x/(1024 * 1024)).toFixed() +" MB"
        return (x/(1024 * 1024 * 1024)).toFixed() +" GB"
    }

    MAIN.processTRam= function(new_params) {
        MAIN.totalRam  = new_params['RP_SYSTEM_TOTAL_RAM'].value
        if (MAIN.freeRam != undefined){
            $('#RAM_SIZE_ID').text(MAIN.convertBytesRAM(MAIN.totalRam - MAIN.freeRam) + " / " +  MAIN.convertBytesRAM(MAIN.totalRam));
        }

    }

    MAIN.processFRam= function(new_params) {
        MAIN.freeRam = new_params['RP_SYSTEM_FREE_RAM'].value
        if (MAIN.freeRam != undefined){
            $('#RAM_SIZE_ID').text(MAIN.convertBytesRAM(MAIN.totalRam - MAIN.freeRam) + " / " +  MAIN.convertBytesRAM(MAIN.totalRam));
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
        MAIN.cpuTemp = new_params['RP_SYSTEM_TEMPERATURE'].value
        if (MAIN.cpuTemp != undefined){
            $('#TEMP_ID').text(MAIN.cpuTemp.toFixed() + " °C");
        }
    }

    MAIN.processVCC1= function(new_params) {
        MAIN.VCC1 = new_params['RP_SYSTEM_VCC_PINT'].value
        if (MAIN.VCC1 != undefined){
            $('#VCCPINT_ID').text(MAIN.VCC1.toFixed(3) + " V");
        }
    }

    MAIN.processVCC2= function(new_params) {
        MAIN.VCC2 = new_params['RP_SYSTEM_VCC_PAUX'].value
        if (MAIN.VCC2 != undefined){
            $('#VCCPAUX_ID').text(MAIN.VCC2.toFixed(3) + " V");
        }
    }

    MAIN.processVCC3= function(new_params) {
        MAIN.VCC3 = new_params['RP_SYSTEM_VCC_BRAM'].value
        if (MAIN.VCC3 != undefined){
            $('#VCCBRAM_ID').text(MAIN.VCC3.toFixed(3) + " V");
        }
    }

    MAIN.processVCC4= function(new_params) {
        MAIN.VCC4 = new_params['RP_SYSTEM_VCC_INT'].value
        if (MAIN.VCC4 != undefined){
            $('#VCCINT_ID').text(MAIN.VCC4.toFixed(3) + " V");
        }
    }

    MAIN.processVCC5 = function(new_params) {
        MAIN.VCC5 = new_params['RP_SYSTEM_VCC_AUX'].value
        if (MAIN.VCC5 != undefined){
            $('#VCCAUX_ID').text(MAIN.VCC5.toFixed(3) + " V");
        }
    }

    MAIN.processVCC6 = function(new_params) {
        if (new_params['RP_SYSTEM_VCC_DDR'] !== undefined)
            MAIN.VCC6 = new_params['RP_SYSTEM_VCC_DDR'].value
        if (new_params['RP_SYSTEM_VCC_DDR_NOMINAL'] !== undefined)
            MAIN.VCC6nominal = new_params['RP_SYSTEM_VCC_DDR_NOMINAL'].value
        if (MAIN.VCC6 != undefined){
            $('#VCCDDR_ID').text(MAIN.VCC6.toFixed(3) + " V");
        }

        if (MAIN.VCC6nominal != undefined){
            $('#VCCDDR_LAB_ID').text("VCC DDR (1." + MAIN.VCC6nominal + "V):");
        }
    }

    MAIN.setLastRelease= function(new_params) {
        MAIN.lastRelease = new_params['RP_LAST_RELEASE'].value
        if (MAIN.lastRelease != undefined){
            RedPitayaOS.getInfo(MAIN.lastRelease)
        }
    }

    MAIN.adcRateFocusOut = function(event) {
        MAIN.adcRateFocusOutValue();
    }


     MAIN.dacRateFocusOut = function(event) {
        MAIN.dacRateFocusOutValue();
    }

    MAIN.adcRateFocusOutValue = function() {
        var text = "";
        if (MAIN.adc_base_rate > 1000000)
            text = Math.round(MAIN.adc_base_rate / 1000) / 1000 + " MHz";
        else if (MAIN.adc_base_rate  > 1000)
            text = MAIN.adc_base_rate / 1000 + " kHz";
        else
            text = MAIN.adc_base_rate + " Hz";

        $("#adc_base_rate").val(text);
    }

    MAIN.dacRateFocusOutValue = function() {

        CLIENT.parametersCache["RP_DAC_BASE_RATE"] = { value:  MAIN.dac_base_rate };
        CLIENT.sendParameters();

        var text = "";
        if (MAIN.dac_base_rate > 1000000)
            text = Math.round(MAIN.dac_base_rate / 1000) / 1000 + " MHz";
        else if (MAIN.dac_base_rate > 1000)
            text = MAIN.dac_base_rate / 1000 + " kHz";
        else
            text = MAIN.dac_base_rate + " Hz";

        $("#dac_base_rate").val(text);
    }

    MAIN.setADCBaseRate = function(new_params) {
        MAIN.adc_base_rate = new_params['RP_ADC_BASE_RATE'].value
        MAIN.adcRateFocusOutValue();
    }

    MAIN.setDACBaseRate = function(new_params) {
        MAIN.dac_base_rate = new_params['RP_DAC_BASE_RATE'].value
        MAIN.dacRateFocusOutValue();
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
    MAIN.param_callbacks["RP_SYSTEM_VCC_DDR_NOMINAL"] = MAIN.processVCC6;

    MAIN.param_callbacks["RP_LAST_RELEASE"] = MAIN.setLastRelease;

    MAIN.param_callbacks["RP_ADC_BASE_RATE"] = MAIN.setADCBaseRate;
    MAIN.param_callbacks["RP_DAC_BASE_RATE"] = MAIN.setDACBaseRate;


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

    $("#adc_base_rate").focus(function() {
        $("#adc_base_rate").val(MAIN.adc_base_rate);
    });

    $("#adc_base_rate").change(function() {
        var new_clock = MAIN.adc_base_rate
        if ($("#adc_base_rate").val() > 0) {
            new_clock = $("#adc_base_rate").val()
            MAIN.adc_base_rate = new_clock
        }
        CLIENT.parametersCache["RP_ADC_BASE_RATE"] = { value:  new_clock};
        CLIENT.sendParameters();
    });

    $("#adc_base_rate").focusout(MAIN.adcRateFocusOut);

    $("#dac_base_rate").focus(function() {
        $("#dac_base_rate").val(MAIN.dac_base_rate);
    });

    $("#dac_base_rate").change(function() {
        var new_clock = MAIN.dac_base_rate
        if ($("#dac_base_rate").val() > 0) {
            new_clock = $("#dac_base_rate").val()
            MAIN.dac_base_rate = new_clock
        }
        CLIENT.parametersCache["RP_DAC_BASE_RATE"] = { value:  new_clock};
        CLIENT.sendParameters();
    });

    $("#dac_base_rate").focusout(MAIN.dacRateFocusOut);

    $("#info").click(function(event) {
        CLIENT.parametersCache["RP_ADC_BASE_RATE"] = { value:  0 };
        CLIENT.parametersCache["RP_DAC_BASE_RATE"] = { value:  0 };
        CLIENT.requestParameters()
        $("#adc_base_rate").val('-');
        $("#dac_base_rate").val('-');
    });

});