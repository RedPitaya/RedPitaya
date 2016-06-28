/*
 * Red Pitaya WIZARDillWIZARDope client
 *
 * Author: Artem Kokos <a.kokos@integrasources.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

(function(WIZARD, $, undefined) {
    WIZARD.currentStep = 0;
    WIZARD.isInReboot = false;
    WIZARD.connectedESSID = "";

    WIZARD.startStep = function(step) {
        WIZARD.currentStep = step;
        WIZARD.steps[WIZARD.currentStep]();
    }

    WIZARD.nextStep = function() {
        WIZARD.startStep(WIZARD.currentStep + 1);
    }

    WIZARD.checkDongle = function() {
        setInterval(function() {
            $.ajax({
                    url: '/check_dongle',
                    type: 'GET',
                    timeout: 1500
                })
                .fail(function(msg) {
                    if (msg.responseText.startsWith("OK")) {
                        $('#wlan0_block_entry').show();
                        $('#wlan0_block_nodongle').hide();
                    } else {
                        // $('#dongle_missing').modal('show');
                        $('#wlan0_block_entry').hide();
                        $('#wlan0_block_nodongle').show();
                    }
                })
        }, 3000);
        if (WIZARD.currentStep > 2)
            WIZARD.currentStep = 0;
        WIZARD.nextStep();
    }

    WIZARD.checkWirelessTools = function() {
        setTimeout(function() {
            $.ajax({
                    url: '/check_wireless',
                    type: 'GET',
                    timeout: 1500
                })
                .fail(function(msg) {
                    if (msg.responseText.startsWith("OK"))
                        WIZARD.nextStep();
                    else
                        $('#wtools_missing').modal('show');
                });
        }, 500);
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
            // console.log(essid, encryption, level);

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

            htmlList += icon + "<div key='" + essid + "' class='btn-wifi-item btn'>" + essid + "</div>";
        }


        if (WIZARD.connectedESSID != "")
            $('.btn-wifi-item[key=' + WIZARD.connectedESSID + ']').css('color', 'red');
        if ($('#wifi_list').html() != htmlList)
            $('#wifi_list').html(htmlList);

        $('.btn-wifi-item').click(function() {
            $('#essid_input_client').val($(this).attr('key'))
            if ($('#essid_input_client').val() == WIZARD.connectedESSID)
                $('#client_connect').text('Disconnect');
            else
                $('#client_connect').text('Connect');
        });
    }

    WIZARD.startScan = function() {
        setInterval(function() {
            $.ajax({
                    url: '/get_wnet_list',
                    type: 'GET',
                })
                .fail(function(msg) {
                    WIZARD.getScanResult(msg.responseText);
                });
            $.ajax({
                    url: '/get_connected_wlan',
                    type: 'GET',
                })
                .fail(function(msg) {
                    if (msg.responseText == undefined || msg.responseText == "")
                        return;

                    WIZARD.isInReboot = false;
                    $('body').addClass('loaded');

                    var essids = msg.responseText.match(/ESSID:(".*?")/g);;
                    if (essids == null)
                        return;
                    var essid = essids[0].substr(7, essids[0].length - 8);
                    if (essid == "Red Pitaya AP")
                        return;
                    else {
                        WIZARD.connectedESSID = essid;
                        $('.btn-wifi-item[key=' + WIZARD.connectedESSID + ']').css('color', 'red');
                    }
                });
        }, 10000);
    }

    WIZARD.RenewDHCP = function() {
        $.ajax({
            url: '/set_eth0_dhcp',
            type: 'GET',
        }).fail(function(msg) {}).done(function(msg) {});
    }

    WIZARD.GetEth0Status = function() {
        $.ajax({
            url: '/get_eth0_status',
            type: 'GET',
        }).fail(function(msg) {
            var res1 = msg.responseText;
            var IPaddr = res1.match(/inet\s+\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\/2[0-90]\b/);
            IPaddr = IPaddr[0].split(" ")[1].split("/")[0];
            var Mask = res1.match(/inet\s+\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\/2[0-90]\b/);
            Mask = Mask[0].split(" ")[1].split("/")[1];
            var IPbrd = res1.match(/brd\s+\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\b/);
            IPbrd = (IPbrd != null) ? IPbrd[0].split(" ")[1] : "none";

            $('#ip_address_label').text(IPaddr);
            $('#broadcast_address_label').text(IPbrd);
            $('#net_mask_label').text(Mask);

            $('#ip_address_label_dhcp').text(IPaddr);
            $('#broadcast_address_label_dhcp').text(IPbrd);
            $('#net_mask_label_dhcp').text(Mask);

            if ($('#eth0_manual_result').css('display') === 'none') {
                $('#ip_address_input').val(IPaddr);
                $('#broadcast_address_input').val(IPbrd);
                $('#net_mask_input').val(Mask);
            }

        }).done(function(msg) {});
    }

    WIZARD.checkMasterModeWifi = function() {
        $.ajax({
            url: '/get_ap_status',
            type: 'GET',
        }).fail(function(msg) {
            if (msg.responseText == "OK\n") {
                $('#access_point_create').text("Remove");
            } else {
                $('#access_point_create').text("Create");
            }
        });
    }

    WIZARD.ManualSetEth0 = function() {
        var IPaddr = $('#ip_address_input').val();
        var Brd = $('#broadcast_address_input').val();
        var Mask = $('#net_mask_input').val();
        $.ajax({
            url: '/set_eth0_manual?ipaddr="' + IPaddr + '"&brdaddr="' + Brd + '"&netmask="' + Mask + '"',
            type: 'GET',
        });
        setTimeout(window.open("http://" + $('#ip_address_input').val() + "/network_manager/", "_self"), 10000);
    }

    WIZARD.startWaiting = function() {
        WIZARD.isInReboot = true;
        $('body').removeClass('loaded');
    }

    WIZARD.steps = [WIZARD.checkDongle, WIZARD.checkWirelessTools, WIZARD.startScan];
}(window.WIZARD = window.WIZARD || {}, jQuery));

// Page onload event handler
$(document).ready(function() {
    setInterval(WIZARD.GetEth0Status, 1000);
    setInterval(WIZARD.checkMasterModeWifi, 2000);

    $('body').addClass('loaded');
    WIZARD.startStep(0);
    $('#get_wtools').click(function(event) {
        WIZARD.startWaiting();
        $.ajax({
                url: '/install_wireless',
                type: 'GET',
            })
            .fail(function(msg) {
                WIZARD.nextStep();
            });
    });

    $('#network_apply_manual').click(WIZARD.ManualSetEth0);

    $('#network_apply_dhcp').click(WIZARD.RenewDHCP);

    $('#essid_input').keyup(function(event) {
        if ($('#essid_input_client').val() == WIZARD.connectedESSID)
            $('#client_connect').text('Disconnect');
        else
            $('#client_connect').text('Connect');
    });

    $('#client_connect').click(function(event) {
        // if (WIZARD.currentStep != 2)
        //     return;
        var essid = $('#essid_input_client').val();
        var password = $('#password_input_client').val();
        if (essid == "") {
            $('#essid_input_client').effect("shake");
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

    $('#apply_btn').click(function() {
        WIZARD.startWaiting();
        $.ajax({
            url: '/ap_mode',
            type: 'GET',
        })
    });

    $('#eth0_mode').change(function() {
        $('.eht0_entries').hide();
        $($(this).val()).show();
    });

    $('#wlan0_mode').change(function() {
        $('.wlan0_entries').hide();
        $($(this).val()).show();
    });

    $('#access_point_create').click(function() {
        if ($('#access_point_create').text() == "Create") {
            $.ajax({
                url: '/wifi_create_point?essid="' + $('#essid_input').val() + '"&password="' + $('#password_input').val() + '"',
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
