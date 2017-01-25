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

            if (window.Worker) {
            	var worker = new Worker("../assets/licVerifyWorker.js");
            	worker.postMessage(null);
            	worker.onmessage = function(e) {
            		console.log(e.data);
				};
				worker.onerror = function(e) {
            		console.log(e.message);
				};
			}

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

    var printRpVersion = function(msg) {
        var info = msg;
        version = info['version'];
        revision = info['revision'];
        stem_ver = info['stem_ver'];
        $('#footer').html("<a style='color: #666;' href='/updater/'>" + 'Red Pitaya OS ' + version + " / " + stem_ver + "</a>");

        BrowserChecker.isOnline(function()
            {
                checkUpdates(info);
            });
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

})(window.RedPitayaOS = window.RedPitayaOS || {}, jQuery);
