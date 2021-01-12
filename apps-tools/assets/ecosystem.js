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
    var stem_ver = '';
    var new_firmware_timer = null;

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
            }, 666);

        }).fail(function(msg) { getListOfApps(); });
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
                    var htmlText = "<p id='update_required_text'><br/>New OS update is available. <br/> <a style='color:red' href='https://github.com/RedPitaya/RedPitaya/blob/master/CHANGELOG.md'>More about this update</a> &nbsp;&nbsp;&nbsp; <a style='color:red' href='/updater/'>Update now!</a>.</p>";

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

    RedPitayaOS.compareVersions = function(ver1, ver2) {
        try {
            var vararr1 = ver1.replace('.', '-');
            vararr1 = vararr1.split("-");
            var vararr2 = ver2.replace('.', '-').split("-");
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

    function blink_NewFirmware() {
        $("#NEW_FIRMWARE_ID").fadeOut(1000);
        $("#NEW_FIRMWARE_ID").fadeIn(1000);
    };

    var printRpVersion = function(msg) {
        var info = msg;
        version = info['version'];
        revision = info['revision'];
        stem_ver = info['stem_ver'];
        if (stem_ver === "STEM 10") {
            stem_ver = "STEMlab 125-10"
        } else if (stem_ver === "STEM 14") {
            stem_ver = "STEMlab 125-14"
        } else if (stem_ver === "STEM 14-Z20") {
            stem_ver = "STEMlab 125-14-Z7020"
        } else if (stem_ver === "STEM 16") {
            stem_ver = "SDRlab 122-16"
        } else if (stem_ver === "STEM 250 12") {
            stem_ver = "SIGNALlab 250-12"
        } else {
            stem_ver = "unknown"
        }

        $('#footer').html("<a style='color: #666;' href='/updater/'>" + 'Red Pitaya OS ' + version + " / " + stem_ver + " <img id=\"NEW_FIRMWARE_ID\"src=\"../assets/images/warning.png\" hidden></a><img id=\"NEED_UPDATE_LINUX_ID\"src=\"../assets/images/warning.png\" hidden>");
        $("#NEED_UPDATE_LINUX_ID").click(function(event) {
            $('#firmware_dialog').modal("show");
        });

        BrowserChecker.isOnline(function() {
            checkUpdates(info);
        });
    }

    $(document).ready(function($) {
        getListOfApps();

        $.ajax({
                method: "GET",
                url: '/get_info'
            })
            .done(function(msg) {
                printRpVersion(msg);
                stem_ver = msg['stem_ver'];
                var board_type = "";
                var linux_path = "LinuxOS";
                if (stem_ver == "STEM 16") {
                    board_type = "SDRlab-122-16/ecosystems";
                }

                if (stem_ver == "STEM 250 12") {
                    board_type = "SIGNALlab-250-12/ecosystems";
                }

                if (stem_ver == "STEM 14") {
                    board_type = "STEMlab-125-1x/ecosystems";
                }

                if (stem_ver == "STEM 14-Z20") {
                    board_type = "STEMlab-125-14-Z7020/ecosystems";
                }

                if (parseFloat(msg["linux_ver"]) !== parseFloat(msg["sd_linux_ver"])) {
                    $("#CUR_VER").text(msg["sd_linux_ver"]);
                    $("#REQ_VER").text(msg["linux_ver"]);
                    $("#NEED_UPDATE_LINUX_ID").attr("hidden", false);
                    var _href = $("#NEW_FIRMWARE_LINK_ID").attr("href");
                    $("#NEW_FIRMWARE_LINK_ID").attr("href", _href + linux_path);
                }

                if (board_type != "") {
                    $.ajax({
                            method: "GET",
                            url: '/update_list?type=' + board_type
                        })
                        .done(function(msg) {
                            var list = [];
                            var arr = msg.split('\n');
                            // example - distro  as array entry: ecosystem-0.97-13-f9094af.zip
                            // example - version as array entry: 12933621
                            for (var i = 0; i < arr.length; i += 2) {
                                if (arr[i] != "" && arr[i].startsWith("ecosystem")) {
                                    list.push(arr[i]);
                                }
                            }

                            if (list.length == 0) return;
                            list.sort();
                            var es_distro_vers = { vers_as_str: '', build: 0, ver_full: '' };
                            // example of list entry: ecosystem-0.97-13-f9094af.zip-12.23M
                            for (var i = list.length - 1; i >= 0; i--) {
                                var item = list[i].split('-');
                                var ver = item[1];
                                var build = item[2];
                                // select latest version according to common version and build
                                if (RedPitayaOS.compareVersions(ver + "." + build, es_distro_vers.vers_as_str + "." + es_distro_vers.build) === -1) {
                                    es_distro_vers.vers_as_str = ver;
                                    es_distro_vers.build = build;
                                    es_distro_vers.ver_full = item.slice(0, 4).join('-');
                                }
                            }

                            if (new_firmware_timer != null) {
                                clearInterval(new_firmware_timer);
                                new_firmware_timer = null;
                            }

                            if (RedPitayaOS.compareVersions(version, es_distro_vers.vers_as_str + "-" + es_distro_vers.build) == 1) {

                                $("#NEW_FIRMWARE_ID").attr("hidden", false);
                                $("#NEED_UPDATE_LINUX_ID").attr("hidden", true);
                                if (new_firmware_timer == null)
                                    new_firmware_timer = setInterval(blink_NewFirmware, 2000);
                            }
                        });
                }

            })
            .fail(printRpVersion);


        $('#ignore_link').click(function(event) {
            var elem = $(this)
            if (elem.attr('href') != undefined && elem.attr('href') != '#')
                window.location.replace(elem.attr('href'));
            else
                $('#ic_missing').modal('hide');
        });

    });

})(window.RedPitayaOS = window.RedPitayaOS || {}, jQuery);