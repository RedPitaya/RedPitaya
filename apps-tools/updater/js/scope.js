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
	UPD.type = '0.97';

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
                    $('#step_2_version').text(msg['version']);
                    UPD.nextStep();
                })
                .fail(function(msg) {
                    $('#step_2_version').text('Unknown');
                    UPD.nextStep();
                })
        }, 500);
    }

    UPD.checkUpdates = function(type) {
        $('#select_ver').hide();
        setTimeout(function() {
            $.ajax({
                    url: '/update_list?type=' + type,
                    type: 'GET',
                })
                .fail(function(msg) {
                    var resp = msg.responseText;
                    var arr = resp.split('\n');

                    $('#retry').click(function(event) {
                        $('#step_' + UPD.currentStep).find('.error_msg').hide();
                        UPD.restartStep();
                    });

                    if (arr.length == 0 || arr.length <= 2 || arr.length % 2 != 0) {
                        if(UPD.type == "0.97")
                        {
                            $("#ecosystem_type").val("2");
                            UPD.type = "0.96";
                            UPD.checkUpdates('0.96');
                            return;
                        } else {
                            $('#step_' + UPD.currentStep).find('.step_icon').find('img').attr('src', 'img/fail.png');
                            $('#step_' + UPD.currentStep).find('.error_msg').show();
                            return;
                        }
                    }
                    var list = [];
					UPD.ecosystems = [];
                    UPD.ecosystems_sizes = [];
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
						if (UPD.isApply)
							return; // FIXME
                        $('.select_ver').hide();
						UPD.isApply = true;
                        var val = $('#ecosystem_ver').val();
                        UPD.chosen_eco = UPD.ecosystems.indexOf(val);
                        if (UPD.chosen_eco != -1) {
                            UPD.nextStep();
                            $('#apply').hide();
                            $('#and_click').hide();
                            $('#ecosystem_ver').attr('disabled', 'disabled');
                        }
                    });
					$('#ecosystem_ver').change();
                });

        }, 500);

    }

    UPD.downloadEcosystem = function() {
		if (!UPD.isApply) {
        	--UPD.currentStep; // FIXME
			return;
		}
        setTimeout(function() {
            $.ajax({
                url: '/update_download?ecosystem=' + UPD.type + '/' + UPD.ecosystems[UPD.chosen_eco],
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
                            var percent = ((size / UPD.ecosystems_sizes[UPD.chosen_eco]) * 100).toFixed(2);
                            $('#percent').text(percent + "%");
                            $('#percent').show();

                            if (size >= UPD.ecosystems_sizes[UPD.chosen_eco]) {
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
    }).fail(function(msg) {
		if (msg[0] == 'd')
			$('#ecosystem_type').append($('<option>', { value: '4', text: 'Dev'}));
    }).done(function(msg) {
		if (msg[0] == 'd')
			$('#ecosystem_type').append($('<option>', { value: '4', text: 'Dev'}));
    })
}

// Page onload event handler
$(document).ready(function() {

    // Init help
    Help.init(helpListUpdater);
    Help.setState("idle");
    
    UPD.startStep(1);
    $('body').addClass('loaded');	
	checkDev();

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
	$('#ecosystem_type').change(function(){
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
		UPD.checkUpdates(UPD.type);
	});

	$('#ecosystem_ver').change(function() {
		let cur = $('#ecosystem_ver option:selected').text().split(' ')[0];
		UPD.getChangelog(UPD.type + '/' + cur + '.changelog');
	});
})

