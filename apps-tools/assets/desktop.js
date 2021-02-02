//-------------------------------------------------
//      Redpitaya desktop
//      Created by Alexey Kaygorodov
//-------------------------------------------------

;
(function(Desktop, $) {

    var applications = [];
    var sys_info_obj = undefined;

    var groups = [{
        name: "System",
        description: "System tools for configuring your Red Pitaya",
        image: "../assets/images/system.png",
        applications: ["updater", "wifi", "licmngr", "calib_app"]
    }, {
        name: "Development",
        description: "Documentation, tutorials and a lot of interesting stuff",
        image: "../assets/images/development.png",
        applications: ["visualprogramming", "scpi", "tutorials", "fpga", "apis", "capps", "cmd", "hardwaredoc", "instructions", "github", "activelearning", "jupyter"]
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
        $.ajax({
                method: "GET",
                url: '/get_info'
            })
            .done(function(result) {
                stem_ver = result['stem_ver'];

                if (stem_ver == "STEM 16") {
                    for (i = default_applications.length - 1; i >= 0; i -= 1) {
                        if (default_applications[i]["id"] === 'marketplace' ||
                            default_applications[i]["id"] === 'fpgaexamples' ||
                            default_applications[i]["id"] === 'jupyter' ||
                            default_applications[i]["id"] === 'activelearning') {
                            default_applications.splice(i, 1);
                        }
                    }
                };

                if (stem_ver == "STEM 250 12") {
                    for (i = default_applications.length - 1; i >= 0; i -= 1) {
                        if (default_applications[i]["id"] === 'marketplace' ||
                            default_applications[i]["id"] === 'fpgaexamples' ||
                            default_applications[i]["id"] === 'jupyter' ||
                            default_applications[i]["id"] === 'activelearning') {
                            default_applications.splice(i, 1);
                        }
                    }
                };

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
            });
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
            if ((currentGroup === undefined && (applications[i].group == "" || applications[i].group === undefined)) || applications[i].group == currentGroup || i == 0) {
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
        try {
            var mail_item = document.createElement("a");
            mail = "support@redpitaya.com";
            subject = "Feedback Red Pitaya OS " + RedPitayaOS.getVersion();
            var body = "%0D%0A%0D%0A------------------------------------%0D%0A" + "DEBUG INFO. DO NOT EDIT!%0D%0A" + "------------------------------------%0D%0A%0D%0A";
            body += "Browser:" + "%0D%0A" + JSON.stringify({ parameters: $.browser }).replace(/[,$&]/g, '_') + "%0D%0A";
            if (Desktop.sys_info_obj !== undefined) {
                body += "%0D%0ABoard:" + "%0D%0A" + JSON.stringify(Desktop.sys_info_obj).replace(/[,$&]/g, '_') + "%0D%0A";
            }
            mail_item.href = ("mailto:" + mail + "?subject=" + subject + "&body=" + body).replace(/ /g, '%20');
            mail_item.click();
        } catch (error) {
            console.error(error);
        }
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
        { id: "github", name: "Sources", description: "Access to open source code and programming instructions", url: "https://github.com/redpitaya", image: "../assets/images/github.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "applicationstore", name: "Red Pitaya Store", description: "Access to Red Pitaya official store", url: "http://www.redpitaya.com/Catalog", image: "../assets/images/shop.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "marketplace", name: "Application marketplace", description: "Access to open source and contributed applications", url: "http://bazaar.redpitaya.com/", image: "images/download_icon.png", check_online: true, licensable: false, callback: undefined, type: 'run' },
        { id: "feedback", name: "Feedback", description: "Tell us what you like or dislike and what you would like to see improved", url: "", image: "../assets/images/feedback.png", check_online: true, licensable: false, callback: showFeedBack, type: 'run' },
        { id: "instructions", name: "Documentation", description: "Quick start instructions, user manuals, specifications, examples & more.", url: "http://redpitaya.readthedocs.io/en/latest/index.html", image: "../assets/images/instr.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "tutorials", name: "Create own WEB application", description: "RedPitaya tutorials.", url: "http://redpitaya.readthedocs.io/en/latest/developerGuide/software/webApps.html?highlight=own%20web%20application", image: "../assets/images/tutors.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "wifi", name: "Network manager", description: "Simple way to establish wireless connection with the Red Pitaya", url: "/network_manager/", image: "../network_manager/info/icon.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "scpi", name: "SCPI server", description: "Remote access to all Red Pitaya inputs/outputs from MATLAB/LabVIEW/Scilab/Python", url: "/scpi_manager/", image: "../scpi_manager/info/icon.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "updater", name: "Red Pitaya OS Update", description: "Red Pitaya ecosystem updater", url: "/updater/", image: "../assets/images/updater.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "activelearning", name: "Teaching materials", description: "Teaching materials for Red Pitaya", url: "https://redpitaya.readthedocs.io/en/latest/teaching/teaching.html", image: "../assets/images/active-learning.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        //	{ id: "warranty_ext", name: "Warranty extension", description: "Standard Warranty Extension", url: "https://redpitaya.com/warranty_extension", image: "../assets/images/WarrantyExt.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        //        { id: "fpgaexamples", name: "FPGA", description: "Red Pitaya FPGA examples", url: "http://red-pitaya-fpga-examples.readthedocs.io/en/latest/", image: "../assets/images/active-learning.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "jupyter", name: "Python programming", description: "Jupyter notebook server for running Python applications in a browser tab", url: "/jupyter/notebooks/RedPitaya/welcome.ipynb", image: "../jupyter_manager/info/icon.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
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
        $("#reboot").click(function(event) {
            $('#reboot_dialog').modal("show");
        });
        $("#reboot_confirm").click(function(event) {
            $.get('/reboot');
            setTimeout(function() { window.close(); }, 1000);
        });
        $("#poweroff_confirm").click(function(event) {
            $.get('/poweroff');
            setTimeout(function() { window.close(); }, 1000);
        });
        $("#info").click(function(event) {


            $.ajax({
                    method: "GET",
                    url: '/get_sysinfo'
                })
                .done(function(result) {
                    try {
                        const obj = JSON.parse(result);
                        var model = obj['model'];
                        if (obj['model'].startsWith('STEM_125-14')) model = 'STEMlab 125-14';
                        if (obj['model'].startsWith('STEM_125-14-Z7020')) model = 'STEMlab 125-14-Z7020';
                        if (obj['model'].startsWith('STEM_250-12')) model = 'SIGNALlab 250-12';
                        if (obj['model'].startsWith('STEM_122-16')) model = 'SDRlab 122-16';
                        $('#SI_B_MODEL').text(model);
                        $('#SI_MAC').text(obj['mac']);
                        $('#SI_DNA').text(obj['dna']);
                        $('#SI_ECOSYSTEM').text(obj['ecosystem']['version'] + '-' + obj['ecosystem']['revision']);
                        $('#SI_LINUX').text(obj['linux']);
                        $('#sysinfo_dialog').modal("show");
                    } catch (error) {
                        console.error(error);
                    }
                })


            $('#sysinfo_dialog').modal("show");
        });

        $.ajax({
                method: "GET",
                url: '/get_sysinfo'
            })
            .done(function(result) {
                const obj = JSON.parse(result);
                Desktop.sys_info_obj = obj;
            })
            .fail(function() {
                Desktop.sys_info_obj = undefined;
            });

    });

})(window.Desktop = window.Desktop || {}, jQuery);
