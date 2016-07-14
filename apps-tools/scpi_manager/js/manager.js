/*
 * Red Pitaya SCPI service manager
 *
 * Author: Artem Kokos <a.kokos@integrasources.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function(SCPI, $, undefined) {
    SCPI.CheckServerStatus = function() {
        $.ajax({
                url: '/get_scpi_status',
                type: 'GET',
                timeout: 1500
            })
            .fail(function(msg) {
                if (msg.responseText.split('\n')[0] == "active") {
                    $('#SCPI_RUN').hide();
                    $('#SCPI_STOP').css('display', 'block');
                    $('#label-is-runnung').hide();
                    $('#label-is-not-runnung').show();
                } else {
                    $('#SCPI_STOP').hide();
                    $('#SCPI_RUN').css('display', 'block');
                    $('#label-is-not-runnung').hide();
                    $('#label-is-runnung').show();
                }
            })
    }

    SCPI.StartServer = function() {
        $.ajax({
                url: '/start_scpi_manager',
                type: 'GET',
                timeout: 1500
            })
            .fail(function(msg) {
                if (msg.responseText) {} else {}
            })
    }

    SCPI.StopServer = function() {
        $.ajax({
                url: '/stop_scpi_manager',
                type: 'GET',
                timeout: 1500
            })
            .fail(function(msg) {
                if (msg.responseText) {} else {}
            })
    }
}(window.SCPI = window.SCPI || {}, jQuery));

// Page onload event handler
$(function() {
    SCPI.CheckServerStatus();
    setInterval(SCPI.CheckServerStatus, 3000);

    $('#SCPI_RUN').click(SCPI.StartServer);

    $('#SCPI_STOP').click(SCPI.StopServer);
});
