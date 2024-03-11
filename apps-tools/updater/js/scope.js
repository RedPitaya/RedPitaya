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
    UPD.ecosystems = [];
    UPD.ecosystems_sizes = [];
    UPD.chosen_eco = -1;
    UPD.isApply = false;

    UPD.currentVer = undefined;
    UPD.EcosystemLinuxVer = undefined;
    UPD.SdLinuxVer = undefined;
    UPD.type = 'stemlab';
    UPD.path_fw = '';
    UPD.timerCheck = undefined;


    UPD.startStep = function(step) {
        UPD.currentStep = step;
        $('#step_' + UPD.currentStep).css("color", "#CCC");
        $('#step_' + UPD.currentStep).find('.step_icon').find('img').attr('src', 'img/loader.gif');
        $('#step_' + UPD.currentStep).find('.step_icon').find('img').show();
        $('#select_ver').hide();
        switch (UPD.currentStep) {
            case 1:
                UPD.checkConnection();
                break;
            case 2:
                UPD.checkVersion();
                break;
            case 3:
                UPD.checkUpdates(UPD.type);
                break;
            case 4:
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

    UPD.restartStep = function() {
        UPD.startStep(UPD.currentStep);
    }

    UPD.nextStep = function() {
        $('#step_' + UPD.currentStep).find('.step_icon').find('img').attr('src', 'img/success.png');
        $('#step_' + UPD.currentStep).find('.step_icon').find('img').show();
        UPD.startStep(UPD.currentStep + 1);
    }

    UPD.setIcon = function() {
        $('#step_' + UPD.currentStep).find('.step_icon').find('img').attr('src', 'img/success.png');
        $('#step_' + UPD.currentStep).find('.step_icon').find('img').show();
    }


    UPD.checkConnection = function() {
        OnlineChecker.checkAsync(function() {
            if (OnlineChecker.isOnline()) {
                UPD.nextStep();
            } else {
                $('#step_1').find('.step_icon').find('img').attr('src', 'img/fail.png');
                $('#step_1').find('.error_msg').show();
            }
        });
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
                    UPD.SdLinuxVer = msg['sd_linux_ver'];
                    UPD.EcosystemLinuxVer = msg['linux_ver'];

                    $('#step_2_version').text(msg['version']);
                    UPD.nextStep();
                })
                .fail(function(msg) {
                    $('#step_2_version').text('Unknown');
                    UPD.nextStep();
                })
        }, 500);
    }

    UPD.compareVersions = function(ver1, ver2) {
        try {
            var vararr1 = ver1.split('.').join('-').split("-");
            var vararr2 = ver2.split('.').join('-').split("-");
            if (vararr1.length != vararr2.length) return 0;
            for (var i = 0; i < vararr1.length; i++) {
                if (parseInt(vararr1[i]) > parseInt(vararr2[i])) return -1;
                if (parseInt(vararr1[i]) < parseInt(vararr2[i])) return 1;
            }
        } catch (error) {
            console.log(error)
        }
        return 0;
    }

    UPD.checkUpdates = function(type) {
        $('#select_ver').hide();
        setTimeout(function() {
            $.ajax({
                    url: '/update_list?type=' + type,
                    type: 'GET',
                })
                .done(function(msg) {
                    var resp = msg;
                    var arr = resp.split('\n');

                    $('#retry').click(function(event) {
                        $('#step_' + UPD.currentStep).find('.error_msg').hide();
                        UPD.restartStep();
                    });

                    // Request resending. Reasons:
                    // - no available distributives for selected type
                    // - invalid response format
                    if (arr.length == 0 || arr.length % 2 != 0) {
                        $('#step_' + UPD.currentStep).find('.step_icon').find('img').attr('src', 'img/fail.png');
                        $('#step_' + UPD.currentStep).find('.error_msg').show();
                        return;
                    }
                    var list = [];
                    UPD.ecosystems = [];
                    UPD.ecosystems_sizes = [];
                    // example - distro  as array entry: ecosystem-0.97-13-f9094af.zip
                    // example - version as array entry: 12933621
                    for (var i = 0; i < arr.length; i += 2) {
                        if (arr[i] != "" && arr[i].startsWith("ecosystem")) {
                            var size = parseInt(arr[i + 1]) * 1;
                            var sizeM = size / (1024 * 1024);
                            UPD.ecosystems_sizes.push(size);
                            UPD.ecosystems.push(arr[i]);
                            list.push(arr[i] + "-" + sizeM.toFixed(2) + "M");
                        }
                    }

                    if (list.length == 0) {
                        $('#step_' + UPD.currentStep).find('.step_icon').find('img').attr('src', 'img/fail.png');
                        $('#step_' + UPD.currentStep).find('.error_msg').show();
                        return;
                    } else {
                        $('#step_' + UPD.currentStep).find('.error_msg').hide();
                    }
                    list.sort();
                    $('#ecosystem_ver').empty();
                    var es_distro_size = 0;
                    var es_distro_vers = { vers_as_str: '0.0', build: 0, ver_full: '' };
                    // example of list entry: ecosystem-0.97-13-f9094af.zip-12.23M
                    for (var i = list.length - 1; i >= 0; i--) {
                        var item = list[i].split('-');
                        var ver = item[1];
                        var build = item[2];
                        var size = item[4];
                        // select latest version according to common version and build
                        if (UPD.compareVersions(ver + "." + build, es_distro_vers.vers_as_str + "." + es_distro_vers.build) === -1) {
                            //                        if (ver > es_distro_vers.vers_as_str || (ver === es_distro_vers.vers_as_str && build > es_distro_vers.build)) {
                            es_distro_vers.vers_as_str = ver;
                            es_distro_vers.build = build;
                            es_distro_size = size;
                            es_distro_vers.ver_full = item.slice(0, 4).join('-');
                        }
                    }
                    var distro_desc = es_distro_vers.vers_as_str + '-' + es_distro_vers.build + '(' + es_distro_size + ')';
                    var distro_desc_short = es_distro_vers.vers_as_str + '-' + es_distro_vers.build;
                    if (UPD.compareVersions(distro_desc_short,UPD.currentVer) === 1) {
                        $('#used_last_version').show();
                        $('#select_ver').hide();
                        $('#step_4').hide();
                        $('#step_5').hide();
                        $('#step_6').hide();
                        $('#step_7').hide();
                        UPD.setIcon();

                    } else {
                        $('#distro_dsc').text(distro_desc);
                        $('#ecosystem_ver').removeAttr('disabled');
                        $('#select_ver').show();
                        $('#apply').click(function(event) {
                            if (UPD.isApply)
                                return; // FIXME
                            $('.select_ver').hide();
                            UPD.isApply = true;
                            var val = es_distro_vers.ver_full;
                            UPD.chosen_eco = UPD.ecosystems.indexOf(val);
                            if (UPD.chosen_eco != -1) {
                                UPD.nextStep();
                                $('#apply').hide();
                                $('#and_click').hide();
                                $('#ecosystem_ver').attr('disabled', 'disabled');
                            }
                        });
                        $('#ecosystem_ver').change();
                    }
                    $.ajax({
                        url: '/check_linux_os?type=' + type+"&ecosystem="+es_distro_vers.ver_full,
                        type: 'GET',
                    })
                    .done(function(msg) {
                        console.log(msg)
                        if (msg.trim() !== "" && msg.trim() !== UPD.EcosystemLinuxVer.trim()){
                            $("#not_supported_linux").show();
                        }
                    });
                });
        }, 500);
    }


    UPD.downloadEcosystem = function() {
        if (!UPD.isApply) {
            --UPD.currentStep; // FIXME
            return;
        }

        console.log("Check download");
        $('#step_' + UPD.currentStep).find('.step_icon').find('img').hide();



        $.ajax({
            url: '/update_download?ecosystem=' + UPD.type + '/' + UPD.ecosystems[UPD.chosen_eco],
            type: 'GET',
        }).done(function(res) {
            console.log(res);
            $('#percent').text("0%");

            UPD.timerCheck = setInterval(function() {
                var url_arr = window.location.href.split("/");;
                var url = url_arr[0] + '//' + url_arr[2];
                $.ajax({
                    url:  url + ':81/update_check',
                    type: 'GET',
                }).done(function(msg) {
                    if (msg.includes("NONE")) {
                        $('#percent').text("0%");
                        $('#percent').show();
                        return;
                    }
                    var res = msg;
                    var s = res.split(" ")[0];
                    var size = parseInt(s) * 1;
                    if (isNaN(size)) {
                        $('#step_' + UPD.currentStep).find('.step_icon').find('img').attr('src', 'img/fail.png');
                        $('#step_' + UPD.currentStep).find('.error_msg').show();
                        clearInterval(UPD.timerCheck);
                    } else {
                        var percent = ((size / UPD.ecosystems_sizes[UPD.chosen_eco]) * 100).toFixed(2);
                        $('#percent').text(percent + "%");
                        $('#percent').show();
                    }

                    if (msg.includes("OK")){
                        clearInterval(UPD.timerCheck);
                        $('#percent').hide();
                       UPD.nextStep();
                    }

                    if (msg.includes("FAIL")){
                        clearInterval(UPD.timerCheck);
                        $('#step_' + UPD.currentStep).find('.step_icon').find('img').attr('src', 'img/fail.png');
                        $('#step_' + UPD.currentStep).find('.error_msg').show();
                    }
                });
            }, 1000);


        }).fail(function(msg) {
            $('#step_' + UPD.currentStep).find('.step_icon').find('img').attr('src', 'img/fail.png');
            $('#step_' + UPD.currentStep).find('.error_msg').show();
            clearInterval(UPD.timerCheck);
        });

    }

    UPD.applyChanges = function() {
        setTimeout(function() {
            $.ajax({
                    url: '/update_extract',
                    type: 'GET',
                })
                .done(function(msg) {
                    var text = msg;
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
                    setTimeout(function() {
                        var prepare_check = setInterval(function() {
                            $.ajax({
                                    url: '/get_info',
                                    type: 'GET',
                                    timeout: 1500
                                })
                                .done(function(msg) {
                                    if (msg != undefined && msg['version'] !== undefined) {
                                        var eco = UPD.ecosystems[UPD.chosen_eco];
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

UPD.getChangelog = function(ecosystem) {
    $.ajax({
            url: '/update_changelog?id=' + ecosystem,
            type: 'GET',
        })
        .done(function(msg) {
            if (msg.length < 3)
                msg = "Changelog is empty";
            $('#changelog_text').html(msg);
        })
}

function checkDev() {
    $.ajax({
        url: '/updater/dev',
        type: 'GET',
    }).always(function(msg) {
        if (msg[0] == 'd')
            $('#ecosystem_type').append($('<option>', { value: '4', text: 'Dev' }));
    });
}

// Page onload event handler
$(document).ready(function() {

    // Init help
    Help.init(helpListUpdater);
    Help.setState("idle");

    UPD.startStep(1);
    $('body').addClass('loaded');

    // TODO: is it needed to allow download dev-distros?
    // checkDev();

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
            else if (oldMinor > minorVer)
                $('#step_' + UPD.currentStep).find('.warn_msg').show();
        }

    });

    $.ajax({
            method: "GET",
            url: '/get_info'
        })
        .done(function(result) {
            stem_ver = result['stem_ver'];
            UPD.type = "Unify/ecosystems";
            UPD.path_fw = "Unify";
            $("#change_log_link").attr("href", "https://github.com/RedPitaya/RedPitaya/blob/master/CHANGELOG.md");
        });

    $('#ecosystem_type').change(function() {
        /*
        Reason: exist one branch with fixed name for download distros with latest OS version
		if ($(this).val() == '1') {
			$('#warn').hide();
			UPD.type = '0.97';
		} else if ($(this).val() == '2') {
			$('#warn').hide();
			UPD.type = '0.96';
		} else if ($(this).val() == '3') {
			$('#warn').show();
			UPD.type = 'beta_0.97';
		} else if ($(this).val() == '4') {
			$('#warn').show();
			UPD.type = 'dev';
		}
        */
        UPD.checkUpdates(UPD.type);
    });

    $('#ecosystem_ver').change(function() {
        var cur = $('#ecosystem_ver option:selected').text().split(' ')[0];
        UPD.getChangelog(UPD.type + '/' + cur + '.changelog');
    });
})