/*
 * Red Pitaya Oscilloscope client
 *
 * Author: Dakus <info@eskala.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function(UPD, $, undefined) {
    var currentStep = 1;
    var ecosystems = [];
    var ecosystems_sizes = [];
    var chosen_eco = -1;
    UPD.startStep = function(step) {
        currentStep = step;
        $('#step_' + currentStep).css("color", "#CCC");
        $('#step_' + currentStep).find('.step_icon').find('img').attr('src', 'img/loader.gif');
        $('#step_' + currentStep).find('.step_icon').find('img').show();
        switch (currentStep) {
            case 1:
                UPD.chekcConnection();
                break;
            case 2:
                UPD.checkUpdates();
                break;
            case 3:
                UPD.downloadEcosystem();
                break;
            case 4:
                UPD.applyChanges();
                break;
            case 5:
                UPD.prepareToRun();
                break;
            case 6:
                UPD.closeUpdate();
                break;
        }
    }

    UPD.nextStep = function() {
        UPD.startStep(currentStep + 1);
    }

    UPD.chekcConnection = function() {
        setTimeout(function() {
            UPD.nextStep();
            return;
            if (OnlineChecker.isOnline()) {
                $('#step_1').find('.step_icon').find('img').attr('src', 'img/success.png');
                UPD.nextStep();
            } else
                $('#step_1').find('.step_icon').find('img').attr('src', 'img/fail.png');
            $('#step_1').find('.error_msg').show();
        }, 500);
    }

    UPD.checkUpdates = function() {
        setTimeout(function() {
            $.ajax({
                    url: '/update_list',
                    type: 'GET',
                })
                .fail(function(msg) {
                    var resp = msg.responseText;
                    var arr = resp.split('\n');
                    if (arr.length == 0 || arr.length <= 2 || arr.length % 2 != 0) {
                        $('#step_2').find('.step_icon').find('img').attr('src', 'img/fail.png');
                        $('#step_2').find('.error_msg').show();
                        return;
                    }
                    var list = [];
                    for (var i = 0; i < arr.length; i + 2) {
                        if (arr[i] != "" && arr[i].startsWith("ecosystem")) {
                            var size = parseInt(arr[i]) * 1;
                            var sizeM = size / (1024 * 1024);
                            ecosystems_sizes.push(size);
                            ecosystems.push(arr[i]);
                            list.push(arr[i] + " " + sizeM.toFixed(2) + "M");
                        }
                    }
                    if (list.length == 0) {
                        $('#step_2').find('.step_icon').find('img').attr('src', 'img/fail.png');
                        $('#step_2').find('.error_msg').show();
                        return;
                    }
                    list.sort();
                });

        }, 500);

    }

    UPD.downloadEcosystem = function() {
        setTimeout(function() {
            UPD.nextStep();
            return;
            $.ajax({
                    url: '/path/to/file',
                    type: 'GET',
                })
                .done(function() {
                    console.log("success");
                })
                .fail(function() {
                    console.log("error");
                })
                .always(function() {
                    console.log("complete");
                });
        }, 500);

    }

    UPD.applyChanges = function() {
        setTimeout(function() {
            UPD.nextStep();
            return;
            $.ajax({
                    url: '/path/to/file',
                    type: 'GET',
                })
                .done(function() {
                    console.log("success");
                })
                .fail(function() {
                    console.log("error");
                })
                .always(function() {
                    console.log("complete");
                });
        }, 500);

    }


    UPD.prepareToRun = function() {
        setTimeout(function() {
            UPD.nextStep();
            return;
            $.ajax({
                    url: '/path/to/file',
                    type: 'GET',
                })
                .done(function() {
                    console.log("success");
                })
                .fail(function() {
                    console.log("error");
                })
                .always(function() {
                    console.log("complete");
                });
        }, 500);

    }

    UPD.closeUpdate = function() {
        setTimeout(function() {
            var url_arr = window.location.href.split("/");;
            var url = url_arr[0] + '//' + url_arr[2];
            location.replace(url)
        }, 2000);
    }

}(window.UPD = window.UPD || {}, jQuery));

// Page onload event handler
$(document).ready(function() {
    UPD.startStep(1);
})
