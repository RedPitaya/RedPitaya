/*
 * Red Pitaya UPDillUPDope client
 *
 * Author: Dakus <info@eskala.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function(UPD, $, undefined) {
    UPD.currentStep = 1;
    var ecosystems = [];
    var ecosystems_sizes = [];
    var chosen_eco = -1;

    UPD.currentVer = undefined;

    UPD.startStep = function(step) {
        UPD.currentStep = step;
        $('#step_' + UPD.currentStep).css("color", "#CCC");
        $('#step_' + UPD.currentStep).find('.step_icon').find('img').attr('src', 'img/loader.gif');
        $('#step_' + UPD.currentStep).find('.step_icon').find('img').show();
        switch (UPD.currentStep) {
            case 1:
                UPD.checkConnection();
                break;
            case 2:
                UPD.checkVersion();
                break;
            case 3:
                UPD.checkUpdates();
                break;
            case 4:
                // UPD.nextStep();
                UPD.downloadEcosystem();
                break;
            case 5:
                UPD.applyChanges();
                break;
            case 6:
                UPD.prepareToRun();
                break;
            case 7:
                UPD.closeUpdate();
                break;
        }
    }

    UPD.nextStep = function() {
        $('#step_' + UPD.currentStep).find('.step_icon').find('img').attr('src', 'img/success.png');
        $('#step_' + UPD.currentStep).find('.step_icon').find('img').show();
        UPD.startStep(UPD.currentStep + 1);
    }

    UPD.checkConnection = function() {
        setTimeout(function() {
            if (OnlineChecker.isOnline()) {
                UPD.nextStep();
            } else {
                $('#step_1').find('.step_icon').find('img').attr('src', 'img/fail.png');
				$('#step_1').find('.error_msg').show();				
			}
        }, 3500);
    }

    UPD.checkVersion = function() {
        setTimeout(function() {
            $.ajax({
                    url: '/get_info',
                    type: 'GET',
                    timeout: 1500
                })
                .done(function(msg) {
                    UPD.currentVer = msg['version'];
                    $('#step_2_version').text(msg['version']);
                    UPD.nextStep();
                })
                .fail(function(msg) {
                    $('#step_2_version').text('Unknown');
                    UPD.nextStep();
                })
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
                        $('#step_' + UPD.currentStep).find('.step_icon').find('img').attr('src', 'img/fail.png');
                        $('#step_' + UPD.currentStep).find('.error_msg').show();
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
                        $('#step_' + UPD.currentStep).find('.step_icon').find('img').attr('src', 'img/fail.png');
                        $('#step_' + UPD.currentStep).find('.error_msg').show();
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
                            $('#and_click').hide();
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
                $('#step_' + UPD.currentStep).find('.step_icon').find('img').hide();
                var check_progress = setInterval(function() {
                    $.ajax({
                        url: '/update_check',
                        type: 'GET',
                    }).fail(function(msg) {
                        var res = msg.responseText;
                        var s = res.split(" ")[0];
                        var size = parseInt(s) * 1;
                        if (isNaN(size)) {
                            $('#step_' + UPD.currentStep).find('.step_icon').find('img').attr('src', 'img/fail.png');
                            $('#step_' + UPD.currentStep).find('.error_msg').show();
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
                }, 1000)
            });
        }, 1000);

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
                        $('#step_' + UPD.currentStep).find('.step_icon').find('img').attr('src', 'img/fail.png');
                        $('#step_' + UPD.currentStep).find('.error_msg').show();
                    }
                })
        }, 500);

    }


    UPD.prepareToRun = function() {
        setTimeout(function() {
            $('#step_' + UPD.currentStep).find('.warn_msg').show();
            $.ajax({
                    url: '/update_ecosystem',
                    type: 'GET',
                })
                .always(function() {
                    setTimeout(function (){
                        var prepare_check = setInterval(function() {
                            $.ajax({
                                    url: '/get_info',
                                    type: 'GET',
                                    timeout: 1500
                                })
                                .done(function(msg) {
                                    if (msg != undefined && msg['version'] !== undefined) {
                                        var eco = ecosystems[chosen_eco];
                                        var arr = eco.split('-');
                                        var ver = arr[1] + '-' + arr[2];
                                        if (msg['version'] == ver) {
                                            UPD.nextStep();
                                            clearInterval(prepare_check);
                                        } else {
                                            $('#step_' + UPD.currentStep).find('.step_icon').find('img').attr('src', 'img/fail.png');
                                            $('#step_' + UPD.currentStep).find('.error_msg').show();
                                            clearInterval(prepare_check);
                                        }

                                    }
                                })
                        }, 2500);
                    }, 15000);
                });
        }, 500);
    }

    UPD.closeUpdate = function() {
        setTimeout(function() {
            var url_arr = window.location.href.split("/");;
            var url = url_arr[0] + '//' + url_arr[2];
            location.replace(url)
        }, 3500);
    }

}(window.UPD = window.UPD || {}, jQuery));

// Page onload event handler
$(document).ready(function() {
    UPD.startStep(1);
    $('body').addClass('loaded');
    $('#ecosystem_ver').change(function() {
        $('#step_' + UPD.currentStep).find('.warn_msg').hide();
        if (UPD.currentVer == undefined)
            return;
        var val = $('#ecosystem_ver').val();
        var arr = val.split('-');

        var oldMajor = parseFloat(UPD.currentVer.split('-')[0]) * 1;
        var oldMinor = parseFloat(UPD.currentVer.split('-')[1]) * 1;

        var majorVer = parseFloat(arr[1]) * 1
        var minorVer = parseFloat(arr[2]) * 1

        if (oldMajor >= majorVer) {
            if (oldMajor > majorVer)
                $('#step_' + UPD.currentStep).find('.warn_msg').show();
            else
            if (oldMinor > minorVer)
                $('#step_' + UPD.currentStep).find('.warn_msg').show();
        }

    });
})
