/*
 * Red Pitaya stream service manager
 *
 * Author: Danilyuk Nikolay <n.danilyuk@integrasources.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function(OBJ, $, undefined) {
    
    OBJ.GetIP = function() {
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
            url: '/get_streaming_ip',
            type: 'GET',
        }).fail(function(msg) {
            $('#ip-addr').text(parseAddress(msg.responseText));
            ipAddressChange();
        }).done(function(msg) {
            $('#ip-addr').text(parseAddress(msg.responseText));
            ipAddressChange();
        });
    }

}(window.OBJ = window.OBJ || {}, jQuery));


function ValidateIPaddress(ipaddress) {
    if (ipaddress == '')
        return false;
    if (/^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/.test(ipaddress)) {
        return (true);
    }
    return (false);
}


// Page onload event handler
$(function() {

    // Init help
    Help.init(helpListStreamServer);
    Help.setState("idle");

    setInterval(OBJ.GetIP, 1000);
    
    $('#CLEAR_FILES').click(OBJ.DeleteFiles);

});