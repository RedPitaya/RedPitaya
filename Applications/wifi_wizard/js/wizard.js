/*
 * Red Pitaya WIZARDillWIZARDope client
 *
 * Author: Dakus <info@eskala.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function(WIZARD, $, undefined) {
    WIZARD.currentStep = 0;

    WIZARD.startStep = function(step) {
        WIZARD.currentStep = step;
        WIZARD.steps[WIZARD.currentStep]();
    }

    WIZARD.nextStep = function() {
        WIZARD.startStep(WIZARD.currentStep + 1);
    }

    WIZARD.checkDongle = function() {
        setTimeout(function() {
            $.ajax({
                    url: '/check_dongle',
                    type: 'GET',
                    timeout: 1500
                })
                .fail(function(msg) {
                    if (msg.responseText.startsWith("OK"))
                        WIZARD.nextStep();
                    else
                        $('#dongle_missing').modal('show');
                })
        }, 500);
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

        var essids = iwlistResult.match(/ESSID:(".*?")/g);
        var encryptions = iwlistResult.match(/Encryption key:((?:[a-z][a-z]+))/g);
        var sigLevel = iwlistResult.match(/Signal level\=(\d+)/g);

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
        if ($('#wifi_list').html() != htmlList)
            $('#wifi_list').html(htmlList);

        $('.btn-wifi-item').click(function() {
            $('#essid_enter').val($(this).attr('key'))
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

        }, 10000);
    }

    WIZARD.steps = [WIZARD.checkDongle, WIZARD.checkWirelessTools, WIZARD.startScan];
}(window.WIZARD = window.WIZARD || {}, jQuery));

// Page onload event handler
$(document).ready(function() {
    $('body').addClass('loaded');
    WIZARD.startStep(0);
    $('#get_wtools').click(function(event) {
        $.ajax({
                url: '/install_wireless',
                type: 'GET',
            })
            .fail(function(msg) {
                WIZARD.nextStep();
            });
    });

    $('#connect_btn').click(function(event) {
        if(WIZARD.currentStep != 2)
            return;
        var essid = $('#essid_enter').val();
        if(essid == "")
            $('#essid_enter').effect("shake");
    });
});
