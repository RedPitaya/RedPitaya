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
        setTimeout(function(){
            $('#loader-desc').html('Getting the list of applications');
            $('body').removeClass('loaded');
            $.ajax({
                url: 'bazaar?apps=',
                cache: false,
                async: true,
                timeout:2000
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
                    obj['image_path'] = url + key + '/info/icon';
                    obj['image_sizes'] = '128;256;512';                    
                    obj['type'] = value['type'];
                    apps.push(obj);
                });

                Desktop.setApplications(apps);
            }).fail(function(msg) { getListOfApps(); });
        },2000);
    }

    RedPitayaOS.getInfo = function(last_eco){
        if (Desktop.sys_info_obj !== undefined){
            var list = [last_eco];
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
        }
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

    RedPitayaOS.printRpVersion = function(msg) {
        var info = msg;
        version = info['ecosystem']['version'];
        revision = info['ecosystem']['revision'];
        stem_ver = info['name'];
        $('#ecosystem_info').html("<a style='color: #666;' href='/updater/'>" + 'Red Pitaya OS ' + version + " / " + stem_ver + " <img id=\"NEW_FIRMWARE_ID\"src=\"../assets/images/warning.png\" hidden></a><img id=\"NEED_UPDATE_LINUX_ID\"src=\"../assets/images/warning.png\" hidden>");
        $("#NEED_UPDATE_LINUX_ID").click(function(event) {
            $('#firmware_dialog').modal("show");
        });
    }

    $(document).ready(function($) {
        getListOfApps();

        $('#ignore_link').click(function(event) {
            var elem = $(this)
            if (elem.attr('href') != undefined && elem.attr('href') != '#')
                window.location.replace(elem.attr('href'));
            else
                $('#ic_missing').modal('hide');
        });

    });

})(window.RedPitayaOS = window.RedPitayaOS || {}, jQuery);