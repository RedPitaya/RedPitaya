/*
 * Red Pitaya Network Manager
 *
 * Author: Artem Kokos <a.kokos@integrasources.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

(function(WIZARD, $, undefined) {
    WIZARD.state = "";
    WIZARD.connectedSSID = "";
    WIZARD.apSSID = '';
    WIZARD.r8188eu = false;

    WIZARD.checkState = function() {
        $.ajax({
                url: '/get_wlan0_state',
                type: 'GET',
                timeout: 2000
            })
            .success(function(msg) {
                msg = msg.trim()
                dongle = msg[0];
                code = msg[1];
                WIZARD.r8188eu = dongle === '2';

                if (dongle === '0') {
                    $('#wlan0_block_entry').hide();
                    $('#wlan0_block_fail').hide();
                    $('#wlan0_block_nodongle').show();
                    WIZARD.stopWaiting();
                    $('#wifi_scan_result').html("");
                } 
                else{

                    $('#wlan0_block_nodongle').hide();
                    $('#wlan0_block_fail').hide();
                    $('#wlan0_block_entry').show();

                    if (code == "1") {
                        $('#wlan0_client_mode').hide();
                        $('#wlan0_ap_mode').hide();
                        $('#wlan0_mode').hide();
                        $('#wlan0_ap_mode_work').hide();
                        $('#wlan0_client_mode_link').show();
                        $('#wlan0_mode_label').text("Client");
                        WIZARD.getConnectedWlan();
                        WIZARD.GetWlan0Status();
                    }

                    if (code == "2") {
                        $('#wlan0_client_mode').hide();
                        $('#wlan0_client_mode_link').hide();
                        $('#wlan0_mode').hide();
                        $('#wlan0_ap_mode').hide();
                        $('#wlan0_ap_mode_work').show();
                        $('#wlan0_mode_label').text("Access Point");
                        WIZARD.restoreAPSSIDIfPossible();
                        WIZARD.GetWlan0Status();
                    }
                    
                    if (code == "0") {
                        $('#wlan0_ap_mode').hide();
                        $('#wlan0_client_mode_link').hide();
                        $('#wlan0_ap_mode_work').hide();
                        if (WIZARD.r8188eu){
                            $('#wlan0_mode option:first').prop('selected', true);
                            $('#wlan0_mode').hide();
                        }else{
                            $('#wlan0_mode').show();
                        }
                        $('#wlan0_client_mode').show();
                        $('#wlan0_mode_label').text("None");
                        $("#wlan0_ssid_label").text("None");
                        $('#wlan0_address_label').text("None");
                        if ($('#wlan0_mode').val() == "#wlan0_client_mode") {
                            $('#wlan0_ap_mode').hide();
                            $('#wlan0_client_mode').show();
                        }

                        if ($('#wlan0_mode').val() == "#wlan0_ap_mode") {
                            $('#wlan0_client_mode').hide();
                            $('#wlan0_ap_mode').show();
                        }
                    }
                                   
                }
            })
    };


    WIZARD.getScanResult = function(iwlistResult) {

        $('body').addClass('loaded');
        var htmlList = "";
        if (iwlistResult.scan.length > 0){
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
        }else{
            $.ajax({
                url: '/wlan0_up',
                type: 'GET',
                timeout: 1000
            });
        }

        // Update networks list if need
        $('#wifi_loader').hide();
        if ($('#wifi_scan_result').html() != htmlList)
            $('#wifi_scan_result').html(htmlList);

        $('.btn-wifi-item').click(function() {
            $('#ssid_input_client').val($(this).attr('key'));
        });
    };

    WIZARD.startScan = function() {
        // Show loader gif
        $('#wifi_scan_result').html("");
        $('#wifi_loader').show();

        $.ajax({
                url: '/get_wnet_list',
                type: 'GET'
            })
            .done(function(msg) {
                WIZARD.getScanResult(msg);
            })
            .fail(function (jqXHR, textStatus, errorThrown) {
                console.log(textStatus,errorThrown)
            });
    };

    WIZARD.getConnectedWlan = function() {
        $.ajax({
                url: '/get_connected_wlan',
                type: 'GET',
                timeout: 1000
            })
            .success(function(msg) {
                msg = msg.trim()
                if (msg == undefined || msg == "\n" || msg == "") {
                    $("#wlan0_ssid_label").text("None");
                    return;
                }
                WIZARD.connectedSSID = msg;
                $("#wlan0_ssid_label").text(WIZARD.connectedSSID);
            });
    };

    WIZARD.GetFirstAddress = function(obj) {
        var ip = null;
        var mask = null;

        for (var i = 0; i < obj.length; ++i) {
            ip = obj[i].split(" ")[1].split("/")[0];
            mask = obj[i].split(" ")[1].split("/")[1];

            // Link-local address checking.
            // Do not use it if it is not the only one.
            if (!ip.startsWith("169.254.")) {
                // Return the first address.
                break;
            }
        }

        return {ip: ip, mask: mask};
    };

    WIZARD.ParseAddress = function(text) {
        // inet ip/mask
        var infoRegexp = /inet\s+\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\/\d+/g;
        var infoMatch = text.match(infoRegexp);
        var ip = null;
        var mask = null;

        if (infoMatch !== null) {
            var info = WIZARD.GetFirstAddress(infoMatch);
            ip = info.ip;
            mask = info.mask;
        }

        return {ip: ip, mask: mask};
    };

    // GET WLAN0 IP ADDRESS
    WIZARD.GetWlan0Status = function() {
        $.ajax({
            url: '/get_wlan0_status',
            type: 'GET'
        }).success(function(msg) {
            var info = WIZARD.ParseAddress(msg);
            if (info.ip != null) {
                $('#wlan0_address_label').text("" + info.ip + " / " + info.mask);
            }else{
                $('#wlan0_address_label').text("None");
            }

        }).done(function(msg) {});
    };

    var routingIsGot = false;
    WIZARD.GetEth0Status = function() {
        $.ajax({
            url: '/get_eth0_status',
            type: 'GET'
        }).success(function(msg) {
            var info = WIZARD.ParseAddress(msg);
            var gateway = msg.split("gateway:")[1].split("\n")[0];

            const $select = document.querySelector('#eth0_mode');
            if (!routingIsGot)
            {
                if (msg.includes("dynamic")) {
                    $select.value = "#eth0_dhcp_mode";
                    routingIsGot = true;
                }
                else {
                    $select.value = "#eth0_static_mode";
                    routingIsGot = true;
                }
            }

            if (!gateway) {
                gateway = "None";
            }

            $('#eth0_address_label').text((info.ip !== null && info.mask !== null) ? "" + info.ip + " / " + info.mask : "None");
            $('#eth0_gateway_label').text(gateway);

        }).done(function(msg) {});
    };

    WIZARD.ManualSetEth0 = function() {
        var IPaddr = $('#ip_address_and_mask_input').val();
        var Gateway = $("#gateway_input").val();
        var DNS = $("#dns_address_input").val();
        var dhcp_flag = ($('#eth0_mode').val() === "#eth0_dhcp_mode") ? true : false;

        var params = [];

        if (IPaddr !== "")
            params.push('address=' + IPaddr);
        if (Gateway !== "")
            params.push('gateway=' + Gateway);
        if (DNS !== "")
            params.push('dns=' + DNS);
        if (dhcp_flag !== false)
            params.push('dhcp=' + ((dhcp_flag) ? 'yes' : 'no'));

        var addr = '/set_eth0';

        if (params.length !== 0) {
            addr += '?';
            for (var i = 0; i < params.length; i++) {
                addr += params[i];
                if (i < (params.length - 1))
                    addr += '&';
            }
        }

        $.ajax({
            url: addr,
            type: 'GET'
        });
        //setTimeout(window.open("http://" + $('#ip_address_and_mask_input').val().split("/")[0] + "/network_manager/", "_self"), 10000); // For update this page with new IP eth0 params
    };

    WIZARD.startWaiting = function() {
        $('body').removeClass('loaded');
    };

    
    /**
     * @name stopWaiting
     * @function
     * @description Put away a time-wait animation
     */

    WIZARD.stopWaiting = function() {
        $('body').addClass('loaded');
    };

    /**
     * @name dropAP
     * @function
     * @description Drop access point
     */

    WIZARD.dropAP = function() {
        WIZARD.startWaiting(); 
        $.ajax({
            url: '/remove_ap',
            type: 'GET'
        })
        .always(function() {
            WIZARD.apSSID = '';
            WIZARD.stopWaiting();
        });
    };

    /**
     * @name getAccessPointSSID
     * @description Restore AP SSID from iw output
     */

    WIZARD.restoreAPSSIDIfPossible = function() {
        $.ajax({
            url: '/get_ap_ssid',
            type: 'GET',
            timeout: 1000
        })
            .success(function(msg) {
                WIZARD.apSSID = msg.replace(/(\r\n|\n|\r)/gm, "").match(/[^ ]+(?=$)/);
                if (WIZARD.apSSID != undefined && WIZARD.apSSID.length > 0 ) {
                    $('#wlan0_ssid_label').text(WIZARD.apSSID);
                }
            });
    };

}(window.WIZARD = window.WIZARD || {}, jQuery));

checkSSID = function(ssid) {
    if (ssid.length > 0) {
        return true;
    }
    $('#ssid_check_len').show();
    return false
};

checkSSID_C = function(ssid) {
    if (ssid.length > 0) {
        return true;
    }
    $('#ssid_check_len_c').show();
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

checkPassword_C = function(pass) {
    if (pass.length >= 8) {
        for (var i = 0; i < pass.length; i++){
            var code = pass.charCodeAt(i);
            if (code < 32 || code > 126){
                $('#pass_check_sym_c').show();
                return false;
            }
        }
        return true;
    }
    $('#pass_check_len_c').show();
    return false;
};

// Page onload event handler
$(document).ready(function() {

     // Init help
    Help.init(helpListNM);
    Help.setState("idle");


    setInterval(WIZARD.checkState, 2000);
    setInterval(WIZARD.GetEth0Status, 2000);
  
    $('body').addClass('loaded');
    $('#network_apply').click(WIZARD.ManualSetEth0);
    $('#refresh_list_btn').click(WIZARD.startScan);


    /**
     * @event onclick
     */

    $('#client_connect').click(function(event) {
        $('#ssid_check_len_c').hide();
        $('#pass_check_len_c').hide();
        $('#pass_check_sym_c').hide();
        var ssid = $('#ssid_input_client').val();
        var password = $('#password_input_client').val();

        var ssid_check = checkSSID_C( ssid );
        var pass_check = checkPassword_C( password );
        if (ssid_check && pass_check) {
        if ( $('#client_connect').text() === "Connect") {
                WIZARD.state = "to_client";
                WIZARD.startWaiting();
                $.ajax({
                    url: '/connect_wifi?ssid="' + ssid + '"&password="' + password + '"',
                    type: 'GET'
                })
                .always(function() {
                   WIZARD.stopWaiting();
                });
            }
        }
                
    });

    $('#client_disconnect').click(function(event) {
        
        var lastSSID = WIZARD.connectedSSID;
        WIZARD.state = "to_normal";
        WIZARD.startWaiting();
        $.ajax({
            url: '/disconnect_wifi',
            type: 'GET'
        })
        .always(function() {
            WIZARD.connectedSSID = '';
            WIZARD.stopWaiting();
        });      
    });

    $('#wifi_mode').click(function() {
        $('.ap-main-container').hide();
        $('.wifi-main-container').show();
    });

    $('#ap_mode').click(function() {
        $('.wifi-main-container').hide();
        $('.ap-main-container').show();
    });

    $('#wlan0_mode').change(function() {
        $(".wlan0_entries").hide();
        $($(this).val()).show();
    });

    $('#eth0_mode').change(function() {
        if ($(this).val() === "#eth0_static_mode") {
            $($(this).val()).show();
        } else {
            $("#eth0_static_mode").hide();
        }
    });

    $('#access_point_create').click(function() {
        if ($('#access_point_create').text() === "Create") {
        	$('#ssid_check_len').hide();
        	$('#pass_check_len').hide();
            $('#pass_check_sym').hide();

            var ssid_input = $('#ssid_input');
            var pass_input = $('#password_input');

            var ssid_check = checkSSID( ssid_input.val() );
            var pass_check = checkPassword( pass_input.val() );

        	if (ssid_check && pass_check){
                WIZARD.state = "to_ap";
                WIZARD.startWaiting();
                $.ajax({
                    url: '/wifi_create_point?ssid=' + ssid_input.val() + '&password=' + pass_input.val() + '',
                    type: 'GET'
                })
                    .always(function() {
                        WIZARD.stopWaiting();
                    })
                    .success(function() {
                        WIZARD.apSSID = ssid_input.val();
                        ssid_input.val('');
                        pass_input.val('');
                    });
        	}
        } 
    });

    $('#ap_mode_stop').click(function() {
        WIZARD.dropAP();
        $('#wlan0_address_label').text('');
    });
    

    $('#clear_entry').click(function() {
        $('#ssid_input_client').val("");
        $('#password_input_client').val("");
    });

    $('#client_reboot').click(function(event) {
        
        WIZARD.startWaiting(); 
        $.ajax({
            url: '/reboot',
            type: 'GET'
        })
        .always(function() {
            setTimeout(function(){
                window.location.reload(1);
             }, 30000);
        });      
    });
});
