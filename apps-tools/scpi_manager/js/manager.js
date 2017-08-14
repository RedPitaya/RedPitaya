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

    SCPI.GetIP = function() {
        $.ajax({
            url: '/get_ip',
            type: 'GET',
        }).fail(function(msg) {

            var res = msg.responseText.split(";");

            var ethIP = res[0].match(/inet\s+\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\/2[0-90]\b/);
            var wlanIP = res[1].match(/inet\s+\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\/2[0-90]\b/);

            if (ethIP != null){
                ethIP = ethIP[0].split(" ")[1].split("/")[0];
                $('#ip-addr').text(ethIP);
            }
            else if (wlanIP != null){
                wlanIP = wlanIP[0].split(" ")[1].split("/")[0];
                $('#ip-addr').text(wlanIP);
            }
            else $('#ip-addr').text("None");

            

        }).done(function(msg) {

            var res = msg.responseText.split(";");

            var ethIP = res[0].match(/inet\s+\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\/2[0-90]\b/);
            var wlanIP = res[1].match(/inet\s+\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\/2[0-90]\b/);

            if (ethIP != null){
                ethIP = ethIP[0].split(" ")[1].split("/")[0];
                $('#ip-addr').text(ethIP);
            }
            else if (wlanIP != null){
                wlanIP = wlanIP[0].split(" ")[1].split("/")[0];
                $('#ip-addr').text(wlanIP);
            }
            else $('#ip-addr').text("None");

        });
    }

}(window.SCPI = window.SCPI || {}, jQuery));




// Page onload event handler
$(function() {

    // Init help
    Help.init(helpListSCPI);
    Help.setState("idle");

    SCPI.CheckServerStatus();
    setInterval(SCPI.GetIP, 1000);
    setInterval(SCPI.CheckServerStatus, 3000);

    $('#SCPI_RUN').click(SCPI.StartServer);

    $('#SCPI_STOP').click(SCPI.StopServer);

    $('#SCPI_EXAMPLES').click(function(){
        window.open('http://redpitaya.readthedocs.io/en/latest/appsFeatures/remoteControl/remoteControl.html','_blank');
    });
});
