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
        $('#step_' + currentStep).find('.step_icon').find('img').attr('src', 'img/success.png');
        $('#step_' + currentStep).find('.step_icon').find('img').show();
        UPD.startStep(currentStep + 1);
    }

    UPD.chekcConnection = function() {
        setTimeout(function() {
            UPD.nextStep();
            return;
            if (OnlineChecker.isOnline()) {

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
                        $('#step_' + currentStep).find('.step_icon').find('img').attr('src', 'img/fail.png');
                        $('#step_' + currentStep).find('.error_msg').show();
                        return;
                    }
                    var list = [];
                    for (var i = 0; i < arr.length; i += 2) {
                        if (arr[i] != "" && arr[i].startsWith("ecosystem")) {
                            var size = parseInt(arr[i + 1]) * 1;
                            var sizeM = size / (1024 * 1024);
                            ecosystems_sizes.push(size);
                            ecosystems.push(arr[i]);
                            list.push(arr[i] + "-" + sizeM.toFixed(2) + "M");
                        }
                    }
                    if (list.length == 0) {
                        $('#step_' + currentStep).find('.step_icon').find('img').attr('src', 'img/fail.png');
                        $('#step_' + currentStep).find('.error_msg').show();
                        return;
                    }
                    list.sort();
                    for (var i = list.length - 1; i >= 0; i--) {
                        var item = list[i].split('-');
                        var ver = item[1];
                        var build = item[2];
                        var size = item[4];
                        var html = '<option value="' + item[0] + '-' + item[1] + '-' + item[2] + '-' + item[3] + '">' + item[1] + '.' + item[2] + ' (' + item[4] + ')</option>';
                        $('#ecosystem_ver').append(html);
                    }
                    $('#ecosystem_ver').removeAttr('disabled');
                    $('.select_ver').show();
                    $('#apply').click(function(event) {
                        var val = $('#ecosystem_ver').val();
                        chosen_eco = ecosystems.indexOf(val);
                        if (chosen_eco != -1) {
                            UPD.nextStep();
                            $('#apply').hide();
                            $('#ecosystem_ver').attr('disabled', 'disabled');
                        }
                    });
                });

        }, 500);

    }

    UPD.downloadEcosystem = function() {
        setTimeout(function() {
            $.ajax({
                url: '/update_download?ecosystem=' + ecosystems[chosen_eco],
                type: 'GET',
            }).always(function() {
                $('#step_' + currentStep).find('.step_icon').find('img').hide();
                var check_progress = setInterval(function() {
                    $.ajax({
                        url: '/update_check',
                        type: 'GET',
                    }).fail(function(msg) {
                        var res = msg.responseText;
                        var s = res.split(" ")[0];
                        var size = parseInt(s) * 1;
                        if (isNaN(size)) {
                            $('#step_' + currentStep).find('.step_icon').find('img').attr('src', 'img/fail.png');
                            $('#step_' + currentStep).find('.error_msg').show();
                            clearInterval(check_progress);
                        } else {
                            var percent = ((size / ecosystems_sizes[chosen_eco]) * 100).toFixed(2);
                            $('#percent').text(percent + "%");
                            $('#percent').show();

                            if (size == ecosystems_sizes[chosen_eco]) {
                                $('#percent').hide();
                                UPD.nextStep();
                                clearInterval(check_progress);
                            }
                        }

                    });
                }, 500)
            });
        }, 500);

    }

    UPD.applyChanges = function() {
        setTimeout(function() {
            $.ajax({
                    url: '/update_extract',
                    type: 'GET',
                })
                .fail(function(msg) {
                    var text = msg.responseText;
                    if (text.startsWith("OK"))
                        UPD.nextStep();
                    else {
                        $('#step_' + currentStep).find('.step_icon').find('img').attr('src', 'img/fail.png');
                        $('#step_' + currentStep).find('.error_msg').show();
                    }
                })
        }, 500);

    }


    UPD.prepareToRun = function() {
        setTimeout(function() {
            $.ajax({
                    url: '/update_ecosystem',
                    type: 'GET',
                })
                .always(function() {
                    var prepare_check = setInterval(function() {
                        $.ajax({
                                url: '/info/info.json',
                                type: 'GET',
                                timeout: 750
                            })
                            .fail(function(msg) {
                                var res = msg.responseText;
                                var obj = undefined;
                                try {
                                    obj = JSON.parse(res);
                                    if (obj != undefined && obj['version'] !== undefined) {
                                        var eco = ecosystems[chosen_eco];
                                        var arr = eco.split('-');
                                        var ver = arr[1] + '-' + arr[2];
                                        if (obj['version'] == ver) {
                                            nextStep();
                                            clearInterval(prepare_check);
                                        } else {
                                            $('#step_' + currentStep).find('.step_icon').find('img').attr('src', 'img/fail.png');
                                            $('#step_' + currentStep).find('.error_msg').show();
                                            clearInterval(prepare_check);
                                        }

                                    }
                                } catch(e){}
                            })
                            .always(function() {
                                console.log("complete");
                            });
                    }, 1000);
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
