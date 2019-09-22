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
        var getFirstAddress = function(obj) {
            var address = null;

            for (var i = 0; i < obj.length; ++i) {
                address = obj[i].split(" ")[1].split("/")[0];

                // Link-local address checking.
                // Do not use it if it is not the only one.
                if (!address.startsWith("169.254.")) {
                    // Return the first address.
                    break;
                }
            }

            return address;
        }

        var parseAddress = function(text) {
            var res = text.split(";");
            var addressRegexp = /inet\s+\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\/\b/g;
            var ethIP = res[0].match(addressRegexp);
            var wlanIP = res[1].match(addressRegexp);
            var ip = null;

            if (ethIP != null) {
                ip = getFirstAddress(ethIP);
            } else if (wlanIP != null) {
                ip = getFirstAddress(wlanIP);
            }

            if (ip === null) {
                ip = "None";
            }

            return ip;
        }

        $.ajax({
            url: '/get_ip',
            type: 'GET',
        }).fail(function(msg) {
            $('#ip-addr').text(parseAddress(msg.responseText));
        }).done(function(msg) {
            $('#ip-addr').text(parseAddress(msg.responseText));
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
