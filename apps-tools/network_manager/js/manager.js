/*
 * Red Pitaya MANAGERillMANAGERope client
 *
 * Author: Artem Kokos <a.kokos@integrasources.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

(function(MANAGER, $, undefined) {
    MANAGER.currentStep = 0;
    MANAGER.isInReboot = false;
    MANAGER.connectedESSID = "";

    MANAGER.startStep = function(step) {
        MANAGER.currentStep = step;
        MANAGER.steps[MANAGER.currentStep]();
    }

    MANAGER.nextStep = function() {
        MANAGER.startStep(MANAGER.currentStep + 1);
    }

    MANAGER.checkDongle = function() {
        setTimeout(function() {
            $.ajax({
                    url: '/check_dongle',
                    type: 'GET',
                    timeout: 1500
                })
                .fail(function(msg) {
                    if (msg.responseText.startsWith("OK"))
                        MANAGER.nextStep();
                    else
                        $('#dongle_missing').modal('show');
                })
        }, 500);
    }

    MANAGER.checkWirelessTools = function() {
        setTimeout(function() {
            $.ajax({
                    url: '/check_wireless',
                    type: 'GET',
                    timeout: 1500
                })
                .fail(function(msg) {
                    if (msg.responseText.startsWith("OK"))
                        MANAGER.nextStep();
                    else
                        $('#wtools_missing').modal('show');
                });
        }, 500);
    }

    MANAGER.getScanResult = function(iwlistResult) {
        if (iwlistResult == undefined || iwlistResult == "")
            return;

        MANAGER.isInReboot = false;
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


        if (MANAGER.connectedESSID != "")
            $('.btn-wifi-item[key=' + MANAGER.connectedESSID + ']').css('color', 'red');
        if ($('#wifi_list').html() != htmlList)
            $('#wifi_list').html(htmlList);

        $('.btn-wifi-item').click(function() {
            $('#essid_enter').val($(this).attr('key'))
            if ($('#essid_enter').val() == MANAGER.connectedESSID)
                $('#connect_btn').text('Disconnect');
            else
                $('#connect_btn').text('Connect');
        });
    }

    MANAGER.startScan = function() {
        setInterval(function() {
            $.ajax({
                    url: '/get_wnet_list',
                    type: 'GET',
                })
                .fail(function(msg) {
                    MANAGER.getScanResult(msg.responseText);
                });
            $.ajax({
                    url: '/get_connected_wlan',
                    type: 'GET',
                })
                .fail(function(msg) {
                    if (msg.responseText == undefined || msg.responseText == "")
                        return;

                    MANAGER.isInReboot = false;
                    $('body').addClass('loaded');

                    var essids = msg.responseText.match(/ESSID:(".*?")/g);;
                    if (essids == null)
                        return;
                    var essid = essids[0].substr(7, essids[0].length - 8);
                    if (essid == "Red Pitaya AP")
                        return;
                    else {
                        MANAGER.connectedESSID = essid;
                        $('.btn-wifi-item[key=' + MANAGER.connectedESSID + ']').css('color', 'red');
                    }
                });
        }, 10000);
    }

    MANAGER.GetEth0Status = function() {
        $.ajax({
                url: '/get_eth0_status',
                type: 'GET',
            }).fail(function(msg) {
                console.log(msg);
                res1 = msg.responseText.split("          "); // 10 spaces in start of string
                res2 = res1[1].split(" ");
                res2.splice(0, 1);
                res2.splice(1, 1);
                res2.splice(2, 1);
                res2[2] = res2[2].split("\n\n")[0]; // Remove two \n characters from last string in array
                $('#ip_address_label').text(res2[0].split(":")[1]);
                $('#broadcast_address_label').text(res2[1].split(":")[1]);
                $('#net_mask_label').text(res2[2].split(":")[1]);
            }).done(function(msg) {
            });
    }

    MANAGER.startWaiting = function() {
        MANAGER.isInReboot = true;
        $('body').removeClass('loaded');
    }

    MANAGER.steps = [MANAGER.checkDongle, MANAGER.checkWirelessTools, MANAGER.startScan];
}(window.MANAGER = window.MANAGER || {}, jQuery));

// Page onload event handler
$(document).ready(function() {
    MANAGER.GetEth0Status();
    $('body').addClass('loaded');
    MANAGER.startStep(0);
    $('#get_wtools').click(function(event) {
        MANAGER.startWaiting();
        $.ajax({
                url: '/install_wireless',
                type: 'GET',
            })
            .fail(function(msg) {
                MANAGER.nextStep();
            });
    });

    $('#essid_enter').keyup(function(event) {
        if ($('#essid_enter').val() == MANAGER.connectedESSID)
            $('#connect_btn').text('Disconnect');
        else
            $('#connect_btn').text('Connect');
    });

    $('#connect_btn').click(function(event) {
        if (MANAGER.currentStep != 2)
            return;
        var essid = $('#essid_enter').val();
        var password = $('#passw_enter').val();
        if (essid == "") {
            $('#essid_enter').effect("shake");
            return;
        }
        if ($(this).text() == "Connect") {
            MANAGER.startWaiting();
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
                    MANAGER.connectedESSID = '';
                });
            $('#connect_btn').text('Connect');
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
        MANAGER.startWaiting();
        $.ajax({
            url: '/ap_mode',
            type: 'GET',
        })
    });

    $('#eth0_mode').change(function() {
        $('.eht0_entries').hide();
        $($(this).val()).show();
        MANAGER.GetEth0Status();
    });
});
