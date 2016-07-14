//-------------------------------------------------
//-------------------------------------------------

(function($) {

    var reloaded = $.cookie("main_forced_reload");
    if (reloaded == undefined || reloaded == "false") {
        $.cookie("main_forced_reload", "true");
        window.location.reload(true);
    }
    var apps = [];
    var version = '';

    placeElements = function() {
        var elemWidth = $('.app-item').outerWidth(true);
        var containerWidth = $('#list-container').width();
        var elemsInRow = Math.floor(containerWidth / elemWidth);
        elemsInRow = (elemsInRow == 0) ? 1 : elemsInRow;

        var elemHeight = $('.app-item').outerHeight(true);
        var containerHeight = $('#main-container').height();
        var elemsInCol = Math.floor(containerHeight / elemHeight);
        elemsInCol = (elemsInCol == 0) ? 1 : elemsInCol;



        $("ul.paging").quickPager({
            pageSize: elemsInRow * elemsInCol
        });
    }

    var refillList = function() {
        $('.app-item').unbind('click');
        $('.app-item').unbind('mouseenter');
        $('.app-item').unbind('mouseleave');
        $('#main-container').empty();
        $('#main-container').append('<ul class="paging" id="list-container"></ul>');

        $('#list-container').empty();
        for (var i = 0; i < apps.length; i++) {
            var txt = '<li class="app-item" key="' + i + '" >';
            txt += '<a href="#" class="app-link"><div class="img-container"><img class="app-icon" src="' + apps[i]['image'] + '"></div><span class="app-name">' + apps[i]['name'] + '</span></a>';
            txt += '</li>';
            $('#list-container').append(txt);
        }
        $('.app-item').click(clickApp);
        $('.app-item').mouseenter(overApp);
        $('.app-item').mouseleave(leaveApp);
    }

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
                obj['licensable'] = true;
                obj['type'] = value['type'];
                apps.push(obj);
            });

            for (var i = 0; i < default_apps.length; i++) {
                if (default_apps[i].id == "marketplace")
                    default_apps[i].url = url + 'bazaar'
                apps.push(default_apps[i]);
            }

            refillList();
            placeElements();
            $('body').addClass('loaded');
        }).fail(function(msg) { getListOfApps(); });
    }

    var clickApp = function(e) {
        var key = parseInt($(this).attr('key')) * 1;
        e.preventDefault();
        if (apps[key].check_online) {
            OnlineChecker.checkAsync(function() {
                if (!OnlineChecker.isOnline()) {
                    if (apps[key].licensable) {
                        $('#ignore_link').text('Ignore');
                        $('#ignore_link').attr('href', apps[key].url);
                        $('#lic_failed').show();
                    } else {
                        $('#ignore_link').text('Close');
                        $('#ignore_link').attr('href', "#");
                        $('#lic_failed').hide();
                    }

                    $('#ic_missing').modal('show');
                    return;
                }
                if (apps[key].url != "" && apps[key].type !== 'run') {
                    licVerify(apps[key].url);
                } else {
                    if (apps[key].url != "")
                        window.location = apps[key].url;
                }
                if (apps[key].callback !== undefined)
                    apps[key].callback();
            });
        } else {
            if (apps[key].url != "" && apps[key].type !== 'run') {
                licVerify(apps[key].url);
            } else {
                if (apps[key].url != "")
                    window.location = apps[key].url;
            }
            if (apps[key].callback !== undefined)
                apps[key].callback();
        }

    }

    var showFeedBack = function() {
        mail = "support@redpitaya.com";
        subject = "Feedback Red Pitaya OS " + version;
        var body = "%0D%0A%0D%0A------------------------------------%0D%0A" + "DEBUG INFO, DO NOT EDIT!%0D%0A" + "------------------------------------%0D%0A%0D%0A";
        body += "Browser:" + "%0D%0A" + JSON.stringify({ parameters: $.browser }) + "%0D%0A";
        document.location.href = "mailto:" + mail + "?subject=" + subject + "&body=" + body;
    }

    var overApp = function(e) {
        var key = parseInt($(this).attr('key')) * 1;
        $('#description').html(apps[key].description);
    }

    var leaveApp = function(e) {
        $('#description').html("");
    }

    var onSwipe = function(ev) {
        if ($('.simplePagerNav').length == 0)
            return;
        var rel = 1;
        if (ev.direction == Hammer.DIRECTION_LEFT)
            rel = parseInt($('.active-dot').parent().attr('rel')) * 1 + 1;
        else if (ev.direction == Hammer.DIRECTION_RIGHT) {
            var crel = parseInt($('.active-dot').parent().attr('rel')) * 1;
            if (crel == 1) return;
            rel = crel - 1;
        }
        var obj = $('.simplePageNav' + rel).find('a');
        if (obj.length == 0)
            return;
        else obj.click();
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
                                            window.location = success_url;
                                        });
                                })
                                .fail(function(msg) {
                                    console.log("LIC: ERR2");
                                    setTimeout(function() { $('body').addClass('loaded'); }, 2000);
                                    window.location = success_url;
                                });
                        } else {
                            console.log("LIC: ERR3");
                            setTimeout(function() { $('body').addClass('loaded'); }, 2000);
                            window.location = success_url;
                        }
                    })
                    .fail(function(msg) {
                        console.log("LIC: ERR4");
                        setTimeout(function() { $('body').addClass('loaded'); }, 2000);
                        window.location = success_url;
                    });
            })
            .fail(function(msg) {
                console.log("LIC: ERR4");
                setTimeout(function() { $('body').addClass('loaded'); }, 2000);
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

    $(document).ready(function($) {
        getListOfApps();

        var myElement = document.getElementById('main-container');
        var mc = new Hammer(myElement);
        mc.on('swipe', onSwipe);

        $.ajax({
            method: "GET",
            url: '/get_info'
        }).done(function(msg) {
            var info = msg;
            version = info['version'];
            $('#footer').html("<a style='color: #666;' href='/updater/'>" + 'Red Pitaya OS ' + info['version'] + "</a>");
            checkUpdates(info);
        }).fail(function(msg) {
            var info = JSON.parse(msg.responseText);
            version = info['version'];
            $('#footer').html("<a style='color: #666;' href='/updater/'>" + 'Red Pitaya OS ' + info['version'] + "</a>");
            checkUpdates(info);
        });

        $('#ignore_link').click(function(event) {
            var elem = $(this)
            if (elem.attr('href') != undefined && elem.attr('href') != '#')
                window.location.replace(elem.attr('href'));
            else
                $('#ic_missing').modal('hide');
        });

    });

    $(window).resize(function($) {
        refillList();
        placeElements();
    });

    var default_apps = [
        // { id: "visualprogramming", name: "Visual Programming", description: "Perfect tool for newcomers to have fun while learning and putting their ideas into practice", url: "http://account.redpitaya.com/try-visual-programming.php", image: "images/img_visualprog.png", check_online: true, licensable: false, callback: undefined, type: 'run' },
        { id: "github", name: "Sources", description: "Access to open source code and programming instructions", url: "https://github.com/redpitaya", image: "../assets/images/github.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "appstore", name: "Red Pitaya Store", description: "Access to Red Pitaya official store", url: "http://store.redpitaya.com/", image: "../assets/images/shop.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "marketplace", name: "Application marketplace", description: "Access to open source and contributed applications", url: "http://bazaar.redpitaya.com/", image: "images/download_icon.png", check_online: true, licensable: false, callback: undefined, type: 'run' },
        { id: "feedback", name: "Feedback", description: "Tell us what you like or dislike and what you would like to see improved", url: "", image: "../assets/images/feedback.png", check_online: true, licensable: false, callback: showFeedBack, type: 'run' },
        { id: "instructions", name: "Instructions", description: "Quick start instructions, user manuals, specifications, examples & more.", url: "http://wiki.redpitaya.com/", image: "../assets/images/instr.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "wifi", name: "Network manager", description: "Simple way to establish wireless connection with the Red Pitaya", url: "/network_manager", image: "../network_manager/info/icon.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "scpi", name: "SCPI server", description: "Remote access to all Red Pitaya inputs/outputs from MATLAB/LabVIEW/Scilab/Python", url: "/scpi_manager", image: "../scpi_manager/info/icon.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "visualprogramming", name: "Visual Programing", description: "Perfect tool for newcomers to have fun while learning and putting their ideas into practice", url: "/wyliodrin_manager", image: "../wyliodrin_manager/info/icon.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
    ];
})(jQuery);
