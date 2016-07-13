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
                if (msg.responseText.split('\n')[0] != "inactive") {
                    $('#VPS_RUN').hide();
                    $('#VPS_STOP').css('display', 'block');
                    $('#label-is-runnung').hide();
                    $('#label-is-not-runnung').show();
                } else {}
            })
    }

    VPS.StartServer = function() {
        $.ajax({
                url: '/start_wyliodrin_status',
                type: 'GET',
                timeout: 1500
            })
            .fail(function(msg) {
                if (msg.responseText) {} else {}
            })
    }

    VPS.StopServer = function() {
        $.ajax({
                url: '/stop_wyliodrin_status',
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
    setInterval(VPS.CheckServerStatus, 3000);

    $('#VPS_RUN').click(VPS.StartServer);

    $('#VPS_STOP').click(VPS.StopServer);
});
