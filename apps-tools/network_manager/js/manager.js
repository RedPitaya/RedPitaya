/*
 * Red Pitaya Network Manager
 *
 * Author: Artem Kokos <a.kokos@integrasources.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

(function(WIZARD, $, undefined) {
    WIZARD.isInReboot = false;
    WIZARD.connectedSSID = "";
    WIZARD.WIFIConnected = false;

    WIZARD.checkDongle = function() {
        $.ajax({
                url: '/check_dongle',
                type: 'GET',
                timeout: 1500
            })
            .success(function(msg) {
                if (msg.startsWith("OK")) {
                    var check = false;

                    if ($('#wlan0_block_entry').css('display') == 'none')
                        check = true;

                    $('#wlan0_block_entry').show();
                    $('#wlan0_block_nodongle').hide();

                    if (check) {
                        WIZARD.startScan();
                        check = false;
                    }
                } else {
                    $('#wlan0_block_entry').hide();
                    $('#wlan0_block_nodongle').show();
                }
            })
    }

    WIZARD.getScanResult = function(iwlistResult) {

        WIZARD.isInReboot = false;
        $('body').addClass('loaded');

        var htmlList = "";
        for (i in iwlistResult.scan) {
            var ssid       =  iwlistResult.scan[i].SSID;
            var encryption = (iwlistResult.scan[i].enc == "Open") ? false : true;
            var level      =  iwlistResult.scan[i].sig

            htmlList += "<div>";
            var lock = (encryption) ? "<img src='img/wifi-icons/lock.png' width=15>" : "";
            if      (level < -81)  icon = "<div style='width: 40px; float: left;'><img src='img/wifi-icons/connection_0.png' width=25>" + lock + "</div>";
            else if (level < -71)  icon = "<div style='width: 40px; float: left;'><img src='img/wifi-icons/connection_1.png' width=25>" + lock + "</div>";
            else if (level < -53)  icon = "<div style='width: 40px; float: left;'><img src='img/wifi-icons/connection_2.png' width=25>" + lock + "</div>";
            else                   icon = "<div style='width: 40px; float: left;'><img src='img/wifi-icons/connection_3.png' width=25>" + lock + "</div>";

            htmlList += icon + "<div key='" + ssid + "' class='btn-wifi-item btn'>" + ssid + "&nbsp;</div>";
            htmlList += "</div>";
        }

        // Update networks list if need
        if ($('#wifi_list').html() != htmlList)
            $('#wifi_list').html(htmlList);

        // Mark connected SSID
        if (WIZARD.connectedSSID != "")
            $('.btn-wifi-item[key="' + WIZARD.connectedSSID + '"]').css('color', 'red');
        else
            $('#client_connect').text('Connect');

        $('.btn-wifi-item').click(function() {
            $('#ssid_input_client').val($(this).attr('key'))
            if ($('#ssid_input_client').val() == WIZARD.connectedSSID)
                $('#client_connect').text('Disconnect');
            else
                $('#client_connect').text('Connect');
        });
    }

    WIZARD.startScan = function() {
        // Show loader gif
        $('#wifi_list').html("<div style='float: left; width: 100%; text-align: center;'><img src='/assets/images/loader.gif' width='35px'></div>");

        $.ajax({
                url: '/get_wnet_list',
                type: 'GET',
            })
            .done(function(msg) {
                WIZARD.getScanResult(msg);
            });
    }

    WIZARD.getConnectedWlan = function() {
        $.ajax({
                url: '/get_connected_wlan',
                type: 'GET',
            })
            .success(function(msg) {
                if (msg == undefined || msg == "\n" || msg == "") {
                    WIZARD.WIFIConnected = false;
                    $("#wlan0_ssid_label").text("None");
                    return;
                }

                WIZARD.isInReboot = false;
                WIZARD.WIFIConnected = true;
                $('body').addClass('loaded');

                var ssids = msg.match(/SSID:(.*)/g);
                if (ssids == null) {
                    $("#wlan0_ssid_label").text("None");
                    return;
                }
                var ssid = ssids[0].substr(6, ssids[0].length - 6);
                if (ssid == "Red Pitaya AP")
                    return;
                else {
                    WIZARD.connectedSSID = ssid;

                    // Mark connected SSID
                    if (WIZARD.connectedSSID != "")
                        $('.btn-wifi-item[key="' + WIZARD.connectedSSID + '"]').css('color', 'red');
                    else {
                        $('#client_connect').text('Connect');
                    }

                    $("#wlan0_ssid_label").text(WIZARD.connectedSSID);
                }
            });
    }

    WIZARD.GetWlan0Status = function() {
        $.ajax({
            url: '/get_wlan0_status',
            type: 'GET',
        }).success(function(msg) {
            var IPaddr = msg.match(/inet\s+\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\/2[0-90]\b/);

            if (IPaddr == null) {
                if (!WIZARD.WIFIConnected){
                    $('#wlan0_address_label').text("None");
                }
                return;
            }

            IPaddr = IPaddr[0].split(" ")[1].split("/")[0];
            var Mask = msg.match(/inet\s+\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\/2[0-90]\b/);
            Mask = Mask[0].split(" ")[1].split("/")[1];

            $('#wlan0_address_label').text(IPaddr + " / " + Mask);

        }).done(function(msg) {});
    }

    WIZARD.GetEth0Status = function() {
        $.ajax({
            url: '/get_eth0_status',
            type: 'GET',
        }).success(function(msg) {
            var gateway = msg.split("gateway:")[1].split("\n")[0];
            var IPaddr = msg.match(/inet\s+\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\/2[0-90]\b/);
            IPaddr = IPaddr[0].split(" ")[1].split("/")[0];
            var Mask = msg.match(/inet\s+\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\/2[0-90]\b/);
            Mask = Mask[0].split(" ")[1].split("/")[1];

            $('#eth0_address_label').text(IPaddr + " / " + Mask);
            $('#eth0_gateway_label').text(gateway);

        }).done(function(msg) {});
    }

    WIZARD.checkMasterModeWifi = function() {
        $.ajax({
            url: '/get_ap_status',
            type: 'GET',
        }).success(function(msg) {
            if (msg.includes("AP")) {
                $('#access_point_create').text("Remove");
                $('#wlan0_mode_label').text("Access Point");
                // $('#wlan0_address_label').text("192.168.128.1");
            } else {
                $('#access_point_create').text("Create");
                $('#wlan0_mode_label').text((WIZARD.WIFIConnected ? "Client" : "None"));
            }
        });
    }

    WIZARD.ManualSetEth0 = function() {
        var IPaddr = $('#ip_address_and_mask_input').val();
        var Gateway = $("#gateway_input").val();
        var DNS = $("#dns_address_input").val();
        var dhcp_flag = ($("#eth0_mode").val() == "#eth0_dhcp_mode") ? true : false;

        var params = [];

        if (IPaddr != "")
            params.push('address=' + IPaddr);
        if (Gateway != "")
            params.push('gateway=' + Gateway);
        if (DNS != "")
            params.push('dns=' + DNS);
        if (dhcp_flag != false)
            params.push('dhcp=' + ((dhcp_flag) ? 'yes' : 'no'));

        var addr = '/set_eth0';

        if (params.length != 0) {
            addr += '?';
            for (var i = 0; i < params.length; i++) {
                addr += params[i];
                if (i < (params.length - 1))
                    addr += '&';
            }
        }

        $.ajax({
            url: addr,
            type: 'GET',
        });
        //setTimeout(window.open("http://" + $('#ip_address_and_mask_input').val().split("/")[0] + "/network_manager/", "_self"), 10000); // For update this page with new IP eth0 params
    }

    WIZARD.startWaiting = function() {
        WIZARD.isInReboot = true;
        $('body').removeClass('loaded');
    }

    WIZARD.stopWaiting = function() {
	WIZARD.isInReboot = false;
	$('body').addClass('loaded');
    }
}(window.WIZARD = window.WIZARD || {}, jQuery));




checkSSID = function(ssid) {
    if (ssid.length > 0) {
        return true;
    }
    $('#ssid_check_len').show();
    return false
};

checkPassword = function(pass) {
    if (pass.length >= 8) {
        for (var i = 0; i < pass.length; i++){
            var code = pass.charCodeAt(i);
            if (code < 32 || code > 126){
                $('#pass_check_sym').show();
                return false;
            }
        }
        return true;
    }
    $('#pass_check_len').show();
    return false;
};




// Page onload event handler
$(document).ready(function() {

     // Init help
    Help.init(helpListNM);
    Help.setState("idle");
    
    WIZARD.startScan();
    // setInterval(WIZARD.startScan, 2500);
    setInterval(WIZARD.GetEth0Status, 1000);
    setInterval(WIZARD.GetWlan0Status, 1000);
    setInterval(WIZARD.checkMasterModeWifi, 2000);
    setInterval(WIZARD.getConnectedWlan, 2000);
    setInterval(WIZARD.checkDongle, 3000);

    $('body').addClass('loaded');

    $('#network_apply').click(WIZARD.ManualSetEth0);

    $('#refresh_list_btn').click(WIZARD.startScan);

    $('#ssid_input').keyup(function(event) {
        if ($('#ssid_input_client').val() == WIZARD.connectedSSID)
            $('#client_connect').text('Disconnect');
        else
            $('#client_connect').text('Connect');
    });

    $('#client_connect').click(function(event) {
        var ssid = $('#ssid_input_client').val();
        var password = $('#password_input_client').val();
        if (ssid == "") {
            // $('#ssid_input_client').effect("shake");
            return;
        }
        if ($(this).text() == "Connect") {
            WIZARD.startWaiting();
            $.ajax({
                url: '/connect_wifi?ssid="' + ssid + '"&password="' + password + '"',
                type: 'GET',
            })
        } else {
	    let lastSSID = WIZARD.connectedSSID;
	    WIZARD.startWaiting();
            $.ajax({
                    url: '/disconnect_wifi',
                    type: 'GET',
                })
                .always(function() {
                    WIZARD.connectedSSID = '';
                });
            $('#client_connect').text('Connect');
            WIZARD.connectedSSID = "";
            $('#wifi_list').html();
	        $('.btn-wifi-item[key="' + lastSSID + '"]').css('color', '#cdcccc');
	        setTimeout(function() {
		        WIZARD.stopWaiting();
	        }, 3000);
        }
    });

    $('#wifi_mode').click(function() {
        $('.ap-main-container').hide();
        $('.wifi-main-container').show();
    });

    $('#ap_mode').click(function() {
        $('.wifi-main-container').hide();
        $('.ap-main-container').show();
    });

    $('#eth0_mode').change(function() {
        if ($(this).val() == "#eth0_static_mode") {
            $($(this).val()).show();
        } else {
            $("#eth0_static_mode").hide();
        }
    });

    $('#wlan0_mode').change(function() {
        $(".wlan0_entries").hide();
        $($(this).val()).show();
    });

    $('#access_point_create').click(function() {
        if ($('#access_point_create').text() == "Create") {
        	$('#ssid_check_len').hide();
        	$('#pass_check_len').hide();
            $('#pass_check_sym').hide();
            var ssid_check = checkSSID( $('#ssid_input').val() );
            var pass_check = checkPassword( $('#password_input').val() );

        	if (ssid_check && pass_check){
	            WIZARD.startWaiting();
	            $.ajax({
	                url: '/wifi_create_point?ssid=' + $('#ssid_input').val() + '&password=' + $('#password_input').val() + '',
	                type: 'GET',
	            })
		    .always(function(){
			WIZARD.stopWaiting();
		    })
		    .success(function(){
			$('#ssid_input').val("");
			$('#password_input').val("");
		    });
        	}
        } else {
            $.ajax({
                url: '/remove_ap',
                type: 'GET',
            });
        }
    });

    $('#clear_entry').click(function() {
        $('#ssid_input_client').val("");
        $('#password_input_client').val("");
    });
});
