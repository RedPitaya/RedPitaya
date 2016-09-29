//-------------------------------------------------
//-------------------------------------------------

(function(RedPitayaOS, $) {

    var reloaded = $.cookie("main_forced_reload");
    if (reloaded == undefined || reloaded == "false") {
        $.cookie("main_forced_reload", "true");
        window.location.reload(true);
    }
    var apps = [];
    var version = '';
    var revision = '';

    var getListOfApps = function() {
        $('#loader-desc').html('Getting the list of applications');
        $('body').removeClass('loaded');
        $.ajax({
            url: 'bazaar?apps=',
            cache: false,
            async: true
        }).done(function(result) {
            var url_arr = window.location.href.split("/");
            var url = url_arr[0] + '//' + url_arr[2] + '/';
            apps = [];
            $.each(result, function(key, value) {
                var obj = {};
                obj['id'] = key;
                obj['name'] = value['name'];
                obj['description'] = value['description'];
                obj['url'] = url + key + '/?type=run';
                obj['image'] = url + key + '/info/icon.png';

                $.ajax({
                    url: obj['image'],
                    cache: false,
                    async: true
                }).fail(function(msg) {
                    getListOfApps();
                });
                obj['licensable'] = false;

                obj['type'] = value['type'];
                apps.push(obj);
            });

            Desktop.setApplications(apps);

            setTimeout(function() {
                $('body').addClass('loaded');
            }, 500);

        }).fail(function(msg) { getListOfApps(); });
    }

    var licVerify = function(success_url) {
        var post_uri = 'http://store.redpitaya.com/upload_id_file/';
        var req_uri = 'http://store.redpitaya.com/get_lic/?rp_mac=';
        $('#loader-desc').html('Preparing application to run');
        $('body').removeClass('loaded');
        $.ajax({
                method: "GET",
                url: "idfile.id"
            })
            .done(function(msg) {
                var obj = jQuery.parseJSON(msg);
                if (obj != undefined && obj != null && obj.mac_address != undefined && obj.mac_address != null)
                    req_uri = req_uri + obj.mac_address;
                if (obj != undefined && obj != null && obj.zynq_id != undefined && obj.zynq_id != null) {
                    req_uri = req_uri + "&rp_dna=" + obj.zynq_id;
                }
                $.ajax({
                        method: "POST",
                        url: post_uri,
                        data: 'id_file=' + encodeURIComponent(msg) + '&version=2'
                    }).done(function(msg) {
                        if (msg == "OK") {
                            $.ajax({
                                    method: "GET",
                                    url: req_uri
                                }).done(function(msg) {
                                    var res_msg = msg + "\r\n";
                                    $.ajax({
                                            method: "POST",
                                            dataType: 'json',
                                            data: {
                                                'lic.lic': res_msg
                                            },
                                            contentType: 'application/json; charset=utf-8',
                                            url: "/lic_upload",
                                        })
                                        .done(function(msg) {})
                                        .fail(function(msg) {
                                            setTimeout(function() { $('body').addClass('loaded'); }, 2000);
                                            if (success_url != undefined)
                                                window.location = success_url;
                                        });
                                })
                                .fail(function(msg) {
                                    console.log("LIC: ERR2");
                                    setTimeout(function() { $('body').addClass('loaded'); }, 2000);
                                    if (success_url != undefined)
                                        window.location = success_url;
                                });
                        } else {
                            console.log("LIC: ERR3");
                            setTimeout(function() { $('body').addClass('loaded'); }, 2000);
                            if (success_url != undefined)
                                window.location = success_url;
                        }
                    })
                    .fail(function(msg) {
                        console.log("LIC: ERR4");
                        setTimeout(function() { $('body').addClass('loaded'); }, 2000);
                        if (success_url != undefined)
                            window.location = success_url;
                    });
            })
            .fail(function(msg) {
                console.log("LIC: ERR4");
                setTimeout(function() { $('body').addClass('loaded'); }, 2000);
                if (success_url != undefined)
                    window.location = success_url;
            });
    }

    var checkUpdates = function(current) {
        $.ajax({
                url: '/update_list',
                type: 'GET',
            })
            .fail(function(msg) {
                var resp = msg.responseText;
                var arr = resp.split('\n');
                if (arr.length == 0 || arr.length <= 2 || arr.length % 2 != 0) {
                    return;
                }
                var ecosystems = [];
                var ver = current['version'].split("-");
                var cMajor = parseFloat(ver[0]) * 1;
                var cMinor = parseFloat(ver[1]) * 1;
                var showUpdatePopup = false;
                for (var i = 0; i < arr.length; i += 2) {
                    if (arr[i] != "" && arr[i].startsWith("ecosystem")) {
                        var ecosystem = arr[i].split('-');
                        var vMajor = parseFloat(ecosystem[1]) * 1;
                        var vMinor = parseFloat(ecosystem[2]) * 1;
                        if (vMajor >= cMajor) {
                            if (vMajor > cMajor)
                                showUpdatePopup = true;
                            else {
                                if (vMinor > cMinor)
                                    showUpdatePopup = true;
                            }
                        }
                    }
                }
                if (showUpdatePopup) {
                    var htmlText = "<p id='update_required_text'><br/>New OS update is available. <br/> <a style='color:red' href='http://wiki.redpitaya.com/index.php?title=More_about_Red_Pitaya_OS_releases'>More about this update</a> &nbsp;&nbsp;&nbsp; <a style='color:red' href='/updater/'>Update now!</a>.</p>";

                    PopupStack.add(htmlText);
                }
            })
    }

    RedPitayaOS.getVersion = function() {
    	return version;
    }


    RedPitayaOS.getRevision = function() {
    	return revision;
    }

    var printRpVersion = function(msg) {
        var info = msg;
        version = info['version'];
        revision = info['revision'];
        $('#footer').html("<a style='color: #666;' href='/updater/'>" + 'Red Pitaya OS ' + info['version'] + "</a>");
        checkUpdates(info);
    }

    $(document).ready(function($) {
        getListOfApps();
        $.ajax({
                method: "GET",
                url: '/get_info'
            })
            .done(printRpVersion)
            .fail(printRpVersion);

        $('#ignore_link').click(function(event) {
            var elem = $(this)
            if (elem.attr('href') != undefined && elem.attr('href') != '#')
                window.location.replace(elem.attr('href'));
            else
                $('#ic_missing').modal('hide');
        });

    });

    licVerify(undefined)
})(window.RedPitayaOS = window.RedPitayaOS || {}, jQuery);
