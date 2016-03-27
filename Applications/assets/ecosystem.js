//-------------------------------------------------
//-------------------------------------------------

(function($) {

    var reloaded = $.cookie("main_forced_reload");
    if(reloaded == undefined || reloaded == "false")
    {
        $.cookie("main_forced_reload", "true");
        window.location.reload(true);
    }
    var apps = [];

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
            var url_arr = window.location.href.split("/");;
            var url = url_arr[0] + '//' + url_arr[2] + '/';
            apps = [];
            $.each(result, function(key, value) {
                var obj = {};
                obj['id'] = key;
                obj['name'] = value['name'];
                obj['description'] = value['description'];
                obj['url'] = url + key + '/?type=run';
                obj['image'] = url + key + '/info/icon.png';
                obj['check_online'] = true;
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
            if (!OnlineChecker.isOnline() && apps[key].type !== 'run' && apps[key].id !== 'scpi_server') {
                if (apps[key].licensable)
                    $('#lic_failed').show();
                else
                    $('#lic_failed').hide();

                $('#ic_missing').modal('show');
                return;
            }
        }
        if (apps[key].url != "" && apps[key].type !== 'run') {
            if (apps[key].id !== 'scpi_server') {
                licVerify(apps[key].url);
            } else {
                window.location = apps[key].url;
            }
        } else {
            if (apps[key].url != "")
                window.location = apps[key].url;
        }
        if (apps[key].callback !== undefined)
            apps[key].callback();
    }

    var showFeedBack = function() {
        mail = "support@redpitaya.com";
        subject = "Feedback";
        body = "";
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
                $.ajax({
                        method: "POST",
                        url: post_uri,
                        data: 'id_file=' + encodeURIComponent(msg)
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

    $(document).ready(function($) {
        getListOfApps();

        var myElement = document.getElementById('main-container');
        var mc = new Hammer(myElement);
        mc.on('swipe', onSwipe);

        var url_arr = window.location.href.split("/");;
        var url = url_arr[0] + '//' + url_arr[2] + '/info/info.json';
        $.ajax({
            method: "GET",
            url: url
        }).done(function(msg) {
            var info = JSON.parse(msg);
            $('#ver').html(info['description'].substring(0, info['description'].length - 1) + ' ' + info['version']);
        }).fail(function(msg) {
            var info = JSON.parse(msg.responseText);
            $('#ver').html(info['description'].substring(0, info['description'].length - 1) + ' ' + info['version']);
        });
    });

    $(window).resize(function($) {
        refillList();
        placeElements();
    });

    var default_apps = [
        { id: "visualprogramming", name: "Visual Programming", description: "Perfect tool for newcomers to have fun while learning and putting their ideas into practice", url: "http://account.redpitaya.com/try-visual-programming.php", image: "images/img_visualprog.png", check_online: true, licensable: false, callback: undefined, type: 'run' },
        { id: "github", name: "Sources", description: "Access to open source code and programming instructions", url: "https://github.com/redpitaya", image: "../assets/images/github.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "appstore", name: "Red Pitaya Store", description: "Access to Red Pitaya official store", url: "http://store.redpitaya.com/", image: "../assets/images/shop.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "marketplace", name: "Application marketplace", description: "Access to open source and contributed applications", url: "http://bazaar.redpitaya.com/", image: "images/download_icon.png", check_online: true, licensable: false, callback: undefined, type: 'run' },
        { id: "feedback", name: "Feedback", description: "Tell us what you like or dislike and what you would like to see improved", url: "", image: "../assets/images/feedback.png", check_online: true, licensable: false, callback: showFeedBack, type: 'run' },
        { id: "wifi_soon", name: "WIFI wizard", description: "Simple way to establish wireless connection with the Red Pitaya", url: "", image: "../assets/images/wifi_soon.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "la_pro_soon", name: "Logic analyzer", description: "Logic analyzer 125Msps with automatic I2C, SPI, UART decoding", url: "", image: "../assets/images/logic_analyzer_soon.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
    ];
})(jQuery);