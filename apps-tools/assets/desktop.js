//-------------------------------------------------
//      Redpitaya desktop
//      Created by Alexey Kaygorodov
//-------------------------------------------------

;
(function(Desktop, $) {

    var applications = [];

    var groups = [{
        name: "System",
        description: "System tools for configuring your Red Pitaya",
        image: "../assets/images/system.png",
        applications: ["updater", "wifi", "licmngr"]
    }, {
        name: "Development",
        description: "Documentation, tutorials and a lot of interesting stuff",
        image: "../assets/images/development.png",
        applications: ["visualprogramming", "scpi", "tutorials", "fpga", "apis", "capps", "cmd", "hardwaredoc", "instructions", "github"]
    }];
    var currentGroup = undefined;

    Desktop.init = function() {
        // Here's should be loading from custom user groups from Cookies
    }
    Desktop.save = function() {
        // Here's should be saveing from custom user groups from Cookies
    }

    Desktop.getCurrentGroup = function() {
        return currentGroup;
    }

    Desktop.setApplications = function(listOfapplications) {
        applications = [];
        $.extend(true, applications, listOfapplications);
        var url_arr = window.location.href.split("/");
        var url = url_arr[0] + '//' + url_arr[2] + '/';

        for (var i = 0; i < default_applications.length; i++) {
            if (default_applications[i].id == "marketplace")
                default_applications[i].url = url + 'bazaar'
            if (default_applications[i].url[0] == "/")
                default_applications[i].url = window.location.origin + default_applications[i].url;
            applications.push(default_applications[i]);
        }

        for (var i = 0; i < applications.length; i++) {
            applications[i].group = checkApplicationInGroup(applications[i].id);
            applications[i].is_group = false;
        }

        for (var i = 0; i < groups.length; i++) {
            var gr = {
                id: "",
                name: groups[i].name,
                description: groups[i].description,
                url: "#",
                image: groups[i].image,
                check_online: false,
                licensable: false,
                callback: openGroup,
                type: 'run',
                group: "",
                is_group: true
            };
            applications.push(gr);
        }
        applications.unshift(backButton);
        Desktop.selectGroup();
    }

    var checkApplicationInGroup = function(app_id) {
        for (var i = 0; i < groups.length; i++)
            if (groups[i].applications.indexOf(app_id) != -1)
                return groups[i].name;
        return "";
    }

    var openGroup = function(key) {
        Desktop.selectGroup(applications[+key].name);
    }

    var onBackButton = function() {
        Desktop.selectGroup();
    }

    var placeElements = function() {
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
        for (var i = 0; i < applications.length; i++) {
            if ((currentGroup === undefined && (applications[i].group == "" || applications[i].group === undefined)) || applications[i].group == currentGroup || i==0) {
                var txt = '<li class="app-item" key="' + i + '" group="' + applications[i].group + '" style="display: none;">';
                txt += '<a href="#" class="app-link"><div class="img-container"><img class="app-icon" src="' + applications[i]['image'] + '"></div><span class="app-name">' + applications[i]['name'] + '</span></a>';
                txt += '</li>';
                $('#list-container').append(txt);
            }
        }

        $('.app-item').click(clickApp);
        $('.app-item').mouseenter(overApp);
        $('.app-item').mouseleave(leaveApp);
    }

    Desktop.selectGroup = function(group) {
        currentGroup = group;
        refillList();
        placeElements();

        if (currentGroup === undefined)
            $('.app-item[key=0]').hide();
        else
            $('.app-item[key=0]').show();
        // if (group === undefined) {
        //     $('.app-item[group=]').show();
        //     $('.app-item[key=0]').hide();
        // } else {
        //     $('.app-item[group="' + group + '"]').show();
        //     $('.app-item[key=0]').show();
        // }

    }

    var clickApp = function(e) {
        var key = parseInt($(this).attr('key')) * 1;
        e.preventDefault();
        if (applications[key].check_online) {
            OnlineChecker.checkAsync(function() {
                if (!OnlineChecker.isOnline()) {
                    if (applications[key].licensable) {
                        $('#ignore_link').text('Ignore');
                        $('#ignore_link').attr('href', applications[key].url);
                        $('#lic_failed').show();
                    } else {
                        $('#ignore_link').text('Close');
                        $('#ignore_link').attr('href', "#");
                        $('#lic_failed').hide();
                    }

                    $('#ic_missing').modal('show');
                    return;
                }
                if (applications[key].url != "")
                    window.location = applications[key].url;
                if (applications[key].callback !== undefined)
                    applications[key].callback(key);
            });
        } else {
            if (applications[key].url != "")
                window.location = applications[key].url;
            if (applications[key].callback !== undefined)
                applications[key].callback(key);
        }
    }

    var showFeedBack = function() {
        mail = "support@redpitaya.com";
        subject = "Feedback Red Pitaya OS " + RedPitayaOS.getVersion();
        var body = "%0D%0A%0D%0A------------------------------------%0D%0A" + "DEBUG INFO, DO NOT EDIT!%0D%0A" + "------------------------------------%0D%0A%0D%0A";
        body += "Browser:" + "%0D%0A" + JSON.stringify({ parameters: $.browser }) + "%0D%0A";
        document.location.href = "mailto:" + mail + "?subject=" + subject + "&body=" + body;
    }

    var overApp = function(e) {
        var key = parseInt($(this).attr('key')) * 1;
        $('#description').html(applications[key].description);
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

    var default_applications = [
        // { id: "visualprogramming", name: "Visual Programming", description: "Perfect tool for newcomers to have fun while learning and putting their ideas into practice", url: "http://account.redpitaya.com/try-visual-programming.php", image: "images/img_visualprog.png", check_online: true, licensable: false, callback: undefined, type: 'run' },
        { id: "github", name: "Sources", description: "Access to open source code and programming instructions", url: "https://github.com/redpitaya", image: "../assets/images/github.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "applicationstore", name: "Red Pitaya Store", description: "Access to Red Pitaya official store", url: "http://store.redpitaya.com/", image: "../assets/images/shop.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "marketplace", name: "Application marketplace", description: "Access to open source and contributed applications", url: "http://bazaar.redpitaya.com/", image: "images/download_icon.png", check_online: true, licensable: false, callback: undefined, type: 'run' },
        { id: "feedback", name: "Feedback", description: "Tell us what you like or dislike and what you would like to see improved", url: "", image: "../assets/images/feedback.png", check_online: true, licensable: false, callback: showFeedBack, type: 'run' },
        { id: "instructions", name: "Instructions", description: "Quick start instructions, user manuals, specifications, examples & more.", url: "http://wiki.redpitaya.com/", image: "../assets/images/instr.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "tutorials", name: "Create own WEB application", description: "RedPitaya tutorials.", url: "http://wiki.redpitaya.com/index.php?title=Tutorials_overview", image: "../assets/images/tutors.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "wifi", name: "Network manager", description: "Simple way to establish wireless connection with the Red Pitaya", url: "/network_manager/", image: "../network_manager/info/icon.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "scpi", name: "SCPI server", description: "Remote access to all Red Pitaya inputs/outputs from MATLAB/LabVIEW/Scilab/Python", url: "/scpi_manager/", image: "../scpi_manager/info/icon.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "visualprogramming", name: "Visual Programing", description: "Perfect tool for newcomers to have fun while learning and putting their ideas into practice", url: "/wyliodrin_manager/", image: "../wyliodrin_manager/info/icon.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "updater", name: "Red Pitaya OS Update", description: "Red Pitaya ecosystem updater", url: "/updater/", image: "../assets/images/updater.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
    ];

    var backButton = {
        id: "back",
        name: "Back",
        description: "Return to the desktop",
        url: "#",
        image: "../assets/images/back_button.png",
        check_online: false,
        licensable: false,
        callback: onBackButton,
        type: 'run',
        group: "",
        is_group: false

    };


    $(window).resize(function($) {
        refillList();
        placeElements();

        Desktop.selectGroup();
    });

    $(document).load(function($) {
        Desktop.init();
    });

    $(document).ready(function($) {
        var myElement = document.getElementById('main-container');
        var mc = new Hammer(myElement);
        mc.on('swipe', onSwipe);
    });


})(window.Desktop = window.Desktop || {}, jQuery);
