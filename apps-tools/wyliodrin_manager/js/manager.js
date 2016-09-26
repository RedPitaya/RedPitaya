/*
 * Red Pitaya visual programing service manager
 *
 * Author: Artem Kokos <a.kokos@integrasources.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function(VPS, $, undefined) {

    VPS.UpdateServerStatus = function(status){
        $('#label-is-runnung').hide();
        $('#label-is-stopped-nl').hide();
        $('#label-is-stopped').hide();
        $('#label-is-updating').hide();

        if (status == "running"){
            $('#label-is-runnung').show();
        }
        else if (status == "no-license"){
            $('#label-is-stopped-nl').show();
        }
        else if (status == "stopped"){
            $('#label-is-stopped').show();
        }
        else if (status == "updating"){
            $('#label-is-updating').show();
        }
    }

    VPS.CheckServerStatus = function() {
        $.ajax({
                url: '/get_wyliodrin_status',
                type: 'GET',
                timeout: 1500
            })
            .fail(function(msg) {
                if (msg.responseText.includes('active')) {
                    $('#VPS_RUN').hide();
                    $('#VPS_STOP').css('display', 'inline-block');
                    VPS.UpdateServerStatus("running");
                } else {  
                    $('#VPS_STOP').hide();
                    $('#VPS_RUN').css('display', 'inline-block');
                    VPS.UpdateServerStatus("stopped");
                }
            })
    }

    VPS.CheckIdentificationFileStatus = function() {
        $.ajax({
                url: '/get_identification_status',
                type: 'GET',
                timeout: 1500
            })
            .fail(function(msg) {
                if (msg.responseText.length > 1) {
                    var username = msg.responseText.split('\n')[0].match("([A-Za-z])[^\"]*(?=@)")[0];
                    $('#vp-identification-fail').hide();
                    $('#vp-missing').hide();
                    $('#vp-identification-success').show();
                    $('#vp-present').show();
                    $('#vp-username').text(username);
                    VPS.CheckServerStatus();
                } else {
                    $('#vp-identification-success').hide();
                    $('#vp-present').hide();       
                    $('#vp-missing').show();
                    $('#vp-identification-fail').show();
                    VPS.UpdateServerStatus("no-license");
                    VPS.StopServer();
                } 
            })
    }

    VPS.StartServer = function() {
        $.ajax({
                url: '/start_wyliodrin_server',
                type: 'GET',
                timeout: 1500
            })
            .fail(function(msg) {
                if (msg.responseText) {} else {}
            })
    }

    VPS.StopServer = function() {
        $.ajax({
                url: '/stop_wyliodrin_server',
                type: 'GET',
                timeout: 1500
            })
            .fail(function(msg) {
                if (msg.responseText) {} else {}
            })
    }
}(window.VPS = window.VPS || {}, jQuery));




// Page onload event handler
$(function() {
    VPS.UpdateServerStatus("updating");
    VPS.CheckIdentificationFileStatus();
    setInterval(VPS.CheckIdentificationFileStatus, 3000);

    $('#VPS_RUN').click(VPS.StartServer);

    $('#VPS_STOP').click(VPS.StopServer);

    $('#vp-select-button').click(function(){
        $('#vp-select-dialog').click();
    });

    $('#vp-select-dialog').change(function () {
        $('#vp-select-path').val($('#vp-select-dialog').val()); 
    });

    $('#vp-upload-button').click(function() {
        VPS.UpdateServerStatus("updating");
        var formData = new FormData($('#vp-upload-form')[0]);
        $.ajax({
            url: '/upload_wyliodrin_file',
            type: 'POST',
            success: function(e) { console.log(e); },
            error: function(e) { console.log(e); },
            data: formData,
            cache: false,
            contentType: false,
            processData: false
        }); 
    });
    
});
