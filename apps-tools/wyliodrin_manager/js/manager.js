/*
 * Red Pitaya visual programing service manager
 *
 * Author: Artem Kokos <a.kokos@integrasources.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function(VPS, $, undefined) {
    VPS.CheckServerStatus = function() {
        $.ajax({
                url: '/get_wyliodrin_status',
                type: 'GET',
                timeout: 1500
            })
            .fail(function(msg) {
                if (msg.responseText.split('\n')[1] == "active") {
                    $('#VPS_RUN').hide();
                    $('#VPS_STOP').css('display', 'inline-block');
                    $('#label-is-runnung').show();
                } else {
                    $('#VPS_STOP').hide();
                    $('#VPS_RUN').css('display', 'inline-block');
                    $('#label-is-runnung').hide();
                }
                VPS.CheckIdentificationFileStatus();
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
                    $('#label-is-not-runnung').hide();
                    $('#vp-identification-success').show();
                    $('#vp-present').show();
                    $('#vp-username').text(username);
                } else {
                    $('#vp-identification-success').hide();
                    $('#vp-present').hide();
                    $('#vp-identification-fail').show();
                    $('#vp-missing').show();
                    $('#label-is-not-runnung').show();
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
    VPS.CheckServerStatus();
    setInterval(VPS.CheckServerStatus, 3000);

    $('#VPS_RUN').click(VPS.StartServer);

    $('#VPS_STOP').click(VPS.StopServer);

    $('#vp-select-button').click(function(){
        $('#vp-select-dialog').click();
    });

    $('#vp-select-dialog').change(function () {
        $('#vp-select-path').val($('#vp-select-dialog').val()); 
    });

    $('#vp-upload-button').click(function() {

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
