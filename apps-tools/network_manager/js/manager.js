/*
 * Red Pitaya Network Manager
 *
 * Author: Artem Kokos <a.kokos@integrasources.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

(function(WIZARD, $, undefined) {
    WIZARD.isInReboot = false;
    WIZARD.connectedESSID = "";
    WIZARD.WIFIConnected = false;

    WIZARD.checkDongle = function() {
        $.ajax({
                url: '/check_dongle',
                type: 'GET',
                timeout: 1500
            })
            .fail(function(msg) {
                if (msg.responseText.startsWith("OK")) {
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
        if (iwlistResult == undefined || iwlistResult == "")
            return;

        WIZARD.isInReboot = false;
        $('body').addClass('loaded');

        var essids = iwlistResult.match(/ESSID:(".*?")/g);
        var encryptions = iwlistResult.match(/Encryption key:((?:[a-z][a-z]+))/g);
        var sigLevel = iwlistResult.match(/Signal level\=(\d+)/g);
        if (essids == null)
            return;

        var htmlList = "";

        for (var i = 0; i < essids.length; i++) {
            var essid = essids[i].substr(7, essids[i].length - 8);
            var encryption = (encryptions[i].substr(15) == "on") ? true : false;
            var level = parseInt(sigLevel[i].substr(13)) * 1;

            htmlList += "<div>";

            var icon = "";
            var lock = (encryption) ? "<img src='img/wifi-icons/lock.png' width=15>" : "";
            if (level < 25)
                icon = "<div style='width: 40px; float: left;'><img src='img/wifi-icons/connection_0.png' width=25>" + lock + "</div>";
            else if (level < 50)
                icon = "<div style='width: 40px; float: left;'><img src='img/wifi-icons/connection_1.png' width=25>" + lock + "</div>";
            else if (level < 75)
                icon = "<div style='width: 40px; float: left;'><img src='img/wifi-icons/connection_2.png' width=25>" + lock + "</div>";
            else
                icon = "<div style='width: 40px; float: left;'><img src='img/wifi-icons/connection_3.png' width=25>" + lock + "</div>";

            htmlList += icon + "<div key='" + essid + "' class='btn-wifi-item btn'>" + essid + "&nbsp;</div>";
            htmlList += "</div>";
        }

        // Update networks list if need
        if ($('#wifi_list').html() != htmlList)
            $('#wifi_list').html(htmlList);

        // Mark connected ESSID
        if (WIZARD.connectedESSID != "")
            $('.btn-wifi-item[key="' + WIZARD.connectedESSID + '"]').css('color', 'red');
        else
            $('#client_connect').text('Connect');

        $('.btn-wifi-item').click(function() {
            $('#essid_input_client').val($(this).attr('key'))
            if ($('#essid_input_client').val() == WIZARD.connectedESSID)
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
            .fail(function(msg) {
                WIZARD.getScanResult(msg.responseText);
            });
    }

    WIZARD.getConnectedWlan = function() {
        $.ajax({
                url: '/get_connected_wlan',
                type: 'GET',
            })
            .fail(function(msg) {
                if (msg.responseText == undefined || msg.responseText == "\n" || msg.responseText == "") {
                    WIZARD.WIFIConnected = false;
                    $("#wlan0_essid_label").text("None");
                    return;
                }

                WIZARD.isInReboot = false;
                WIZARD.WIFIConnected = true;
                $('body').addClass('loaded');

                var essids = msg.responseText.match(/ESSID:(".*?")/g);
                if (essids == null) {
                    $("#wlan0_essid_label").text("None");
                    return;
                }
                var essid = essids[0].substr(7, essids[0].length - 8);
                if (essid == "Red Pitaya AP")
                    return;
                else {
                    WIZARD.connectedESSID = essid;

                    // Mark connected ESSID
                    if (WIZARD.connectedESSID != "")
                        $('.btn-wifi-item[key="' + WIZARD.connectedESSID + '"]').css('color', 'red');
                    else {
                        $('#client_connect').text('Connect');
                    }

                    $("#wlan0_essid_label").text(WIZARD.connectedESSID);
                }
            });
    }

    WIZARD.GetWlan0Status = function() {
        $.ajax({
            url: '/get_wlan0_status',
            type: 'GET',
        }).fail(function(msg) {
            var res1 = msg.responseText;
            var IPaddr = res1.match(/inet\s+\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\/2[0-90]\b/);

            if (IPaddr == null) {
                if (!WIZARD.WIFIConnected)
                    $('#wlan0_address_label').text("None");
                return;
            }

            IPaddr = IPaddr[0].split(" ")[1].split("/")[0];
            var Mask = res1.match(/inet\s+\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\/2[0-90]\b/);
            Mask = Mask[0].split(" ")[1].split("/")[1];

            $('#wlan0_address_label').text(IPaddr + " / " + Mask);

        }).done(function(msg) {});
    }

    WIZARD.GetEth0Status = function() {
        $.ajax({
            url: '/get_eth0_status',
            type: 'GET',
        }).fail(function(msg) {
            var res1 = msg.responseText;
            var gateway = msg.responseText.split("gateway:")[1].split("\n")[0];
            var IPaddr = res1.match(/inet\s+\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\/2[0-90]\b/);
            IPaddr = IPaddr[0].split(" ")[1].split("/")[0];
            var Mask = res1.match(/inet\s+\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\/2[0-90]\b/);
            Mask = Mask[0].split(" ")[1].split("/")[1];

            $('#eth0_address_label').text(IPaddr + " / " + Mask);
            $('#eth0_gateway_label').text(gateway);

        }).done(function(msg) {});
    }

    WIZARD.checkMasterModeWifi = function() {
        $.ajax({
            url: '/get_ap_status',
            type: 'GET',
        }).fail(function(msg) {
            if (msg.responseText == "OK\n") {
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
}(window.WIZARD = window.WIZARD || {}, jQuery));

// Page onload event handler
$(document).ready(function() {
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

    $('#essid_input').keyup(function(event) {
        if ($('#essid_input_client').val() == WIZARD.connectedESSID)
            $('#client_connect').text('Disconnect');
        else
            $('#client_connect').text('Connect');
    });

    $('#client_connect').click(function(event) {
        var essid = $('#essid_input_client').val();
        var password = $('#password_input_client').val();
        if (essid == "") {
            // $('#essid_input_client').effect("shake");
            return;
        }
        if ($(this).text() == "Connect") {
            WIZARD.startWaiting();
            $.ajax({
                url: '/connect_wifi?essid="' + essid + '"&password="' + password + '"',
                type: 'GET',
            })
        } else {
            $.ajax({
                    url: '/disconnect_wifi',
                    type: 'GET',
                })
                .always(function() {
                    WIZARD.connectedESSID = '';
                });
            $('#essid_input_client').text('Connect');
            WIZARD.connectedESSID = "";
            $('#wifi_list').html();
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
            $.ajax({
                url: '/wifi_create_point?essid=' + $('#essid_input').val() + '&password=' + $('#password_input').val() + '',
                type: 'GET',
            });
        } else {
            $.ajax({
                url: '/remove_ap',
                type: 'GET',
            });
        }
    });

    $('#clear_entry').click(function() {
        $('#essid_input_client').val("");
        $('#password_input_client').val("");
    });
});
