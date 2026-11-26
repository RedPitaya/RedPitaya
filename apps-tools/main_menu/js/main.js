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


    MAIN.lastRelease = undefined;


    MAIN.is_clock = true;
    MAIN.calib_error = false;


    MAIN.processSignals = function(SIG){
    }

    MAIN.setLastRelease= function(new_params) {
        MAIN.lastRelease = new_params['RP_LAST_RELEASE'].value
        if (MAIN.lastRelease != undefined){
            RedPitayaOS.getInfo(MAIN.lastRelease)
        }
    }

    MAIN.setCommand = function(new_params) {
    }

    // Header info icon blinks while any of the monitored subsystems reports a problem
    MAIN.updateInfoIconState = function() {
        if (MAIN.is_clock && !MAIN.calib_error){
            $('.info-icon img').removeClass('blinking');
        }else{
            $('.info-icon img').addClass('blinking');
        }
    }

    MAIN.setClockState = function(new_params) {
        MAIN.is_clock = new_params['RP_SYSTEM_CLOCK_STATE'].value
        MAIN.updateInfoIconState();
    }

    MAIN.setCalibErrorState = function(new_params) {
        MAIN.calib_error = new_params['RP_CALIB_ERROR_STATE'].value
        MAIN.updateInfoIconState();
    }

    MAIN.param_callbacks["RP_LAST_RELEASE"] = MAIN.setLastRelease;
    MAIN.param_callbacks["RP_COMMAND"] = MAIN.setCommand;
    MAIN.param_callbacks["RP_SYSTEM_CLOCK_STATE"] = MAIN.setClockState;
    MAIN.param_callbacks["RP_CALIB_ERROR_STATE"] = MAIN.setCalibErrorState;

}(window.MAIN = window.MAIN || {}, jQuery));




// Page onload event handler
$(function() {

    $("#bug_report").click(async function (event){

        RP_WS_CLIENT.connectWebSocket(function onOpen(){
                    console.log("Open");
                    $("#build_log_dialog").modal("show");
                    CLIENT.parametersCache["RP_COMMAND"] = { value:  1}; // Start build log
                    CLIENT.sendParameters();
                },

                function onClose(){
                    console.log("Close");
                },
                function onError(){
                    console.log("Error");
                },
                function onMessage(receive){

                    if (receive.build_log_step){
                        let step = receive.build_log_step.value
                        const $progressBar = $('#build_progress_bar');
                        $progressBar.css('width', step + '%');
                        console.log("build_log_step " + step);
                    }

                    if (receive.build_log_done){
                        let done = receive.build_log_done.value
                        RP_WS_CLIENT.closeWS()
                        console.log("build_log_done " + done);

                        if (done == 0){

                            setTimeout(async function(){
                                const blob =  await fetch('/get_bug_report', {
                                    method: 'POST'
                                })
                                .then(resp => resp.blob());

                            if( window.showSaveFilePicker ) {
                                const handle = await showSaveFilePicker({
                                    suggestedName: new Date().toJSON().slice(0,22) + ".zip" });
                                const writable = await handle.createWritable();
                                await writable.write( blob );
                                writable.close();
                            }
                            else {
                                const saveImg = document.createElement( "a" );
                                saveImg.href = URL.createObjectURL( blob );;
                                saveImg.download= new Date().toJSON().slice(0,22) + ".zip";
                                saveImg.click();
                                setTimeout(() => URL.revokeObjectURL( saveImg.href ), 60000 );
                            }}
                            ,1000)

                        }
                        $("#build_log_dialog").modal('hide');
                    }
                }
        )



    });

    $("#cancel_build_log_id").click(async function (event){
        CLIENT.parametersCache["RP_COMMAND"] = { value:  2}; // Stop build log
        CLIENT.sendParameters();
    });

});