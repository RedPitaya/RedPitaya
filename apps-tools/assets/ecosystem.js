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
        switch(stem_ver){
            case 0:{
                stem_ver = "STEMlab 125-10"
                break;
            }
            case 1:{
                stem_ver = "STEMlab 125-14 v1.1"
                break;
            }
            case 2:{
                stem_ver = "STEMlab 125-14 v1.1"
                break;
            }
            case 3:{
                stem_ver = "SDRlab 122-16 v1.0"
                break;
            }
            case 4:{
                stem_ver = "SDRlab 122-16 v1.1"
                break;
            }
            case 5:{
                stem_ver = "STEMlab 125-14 LN v1.1"
                break;
            }
            case 6:{
                stem_ver = "STEMlab 125-14-Z7020 v1.0"
                break;
            }
            case 7:{
                stem_ver = "STEMlab 125-14-Z7020 LN v1.1"
                break;
            }
            case 8:{
                stem_ver = "STEMlab 125-14-Z7020 4-ch v1.0"
                break;
            }
            case 9:{
                stem_ver = "STEMlab 125-14-Z7020 4-ch v1.2"
                break;
            }
            case 10:{
                stem_ver = "STEMlab 125-14-Z7020 4-ch v1.3"
                break;
            }
            case 11:{
                stem_ver = "SIGNALlab 250-12 v1.1"
                break;
            }
            case 12:{
                stem_ver = "SIGNALlab 250-12 v1.2"
                break;
            }
            case 13:{
                sstem_ver = "SIGNALlab 250-12/120"
                break;
            }
            default:
                stem_ver = "unknown"
        }


        $('#ecosystem_info').html("<a style='color: #666;' href='/updater/'>" + 'Red Pitaya OS ' + version + " / " + stem_ver + " <img id=\"NEW_FIRMWARE_ID\"src=\"../assets/images/warning.png\" hidden></a><img id=\"NEED_UPDATE_LINUX_ID\"src=\"../assets/images/warning.png\" hidden>");
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
                setTimeout(printRpVersion(msg),2000);
                stem_ver = msg['stem_ver'];
                var board_type = "Unify/ecosystems";
                var linux_path = "LinuxOS";

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
                            var es_distro_vers = { vers_as_str: '0.00', build: 0, ver_full: '' };
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

        // App configuration
        RedPitayaOS.config = {};
        RedPitayaOS.config.app_id = 'main_menu';
        RedPitayaOS.config.server_ip = ''; // Leave empty on production, it is used for testing only
        RedPitayaOS.config.start_app_url = (RedPitayaOS.config.server_ip.length ? 'http://' + RedPitayaOS.config.server_ip : '') + '/bazaar?start=' + RedPitayaOS.config.app_id;
        RedPitayaOS.config.stop_app_url = (RedPitayaOS.config.server_ip.length ? 'http://' + RedPitayaOS.config.server_ip : '') + '/bazaar?stop=' + RedPitayaOS.config.app_id;


        RedPitayaOS.startApp = function() {
            $.get(
                    RedPitayaOS.config.start_app_url
                )
                .done(function(dresult) {
                    if (dresult.status == 'OK') {
                        console.log("Load main menu");
                    } else if (dresult.status == 'ERROR') {
                        console.log(dresult.reason ? dresult.reason : 'Could not start the application (ERR1)');
                        RedPitayaOS.startApp();
                    } else {
                        console.log('Could not start the application (ERR2)');
                        RedPitayaOS.startApp();
                    }
                })
                .fail(function() {
                    console.log('Could not start the application (ERR3)');
                    RedPitayaOS.startApp();
                });
        };

        RedPitayaOS.startApp();
    });

})(window.RedPitayaOS = window.RedPitayaOS || {}, jQuery);