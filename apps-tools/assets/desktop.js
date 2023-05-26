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
        applications: ["visualprogramming", "scpi", "tutorials", "fpga", "apis", "capps", "cmd", "hardwaredoc", "instructions", "github", "activelearning", "jupyter","web_ssh"]
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

    Desktop.convertModel = function(model){
        /*
         List of board models

            typedef enum {
                STEM_125_10_v1_0            = 0,
                STEM_125_14_v1_0            = 1,
                STEM_125_14_v1_1            = 2,
                STEM_122_16SDR_v1_0         = 3,
                STEM_122_16SDR_v1_1         = 4,
                STEM_125_14_LN_v1_1         = 5,
                STEM_125_14_Z7020_v1_0      = 6,
                STEM_125_14_Z7020_LN_v1_1   = 7,
                STEM_125_14_Z7020_4IN_v1_0  = 8,
                STEM_125_14_Z7020_4IN_v1_2  = 9,
                STEM_125_14_Z7020_4IN_v1_3  = 10,
                STEM_250_12_v1_0            = 11,
                STEM_250_12_v1_1            = 12,
                STEM_250_12_v1_2            = 13,
                STEM_250_12_120             = 14,
                STEM_250_12_v1_2a           = 15
            }  rp_HPeModels_t;
        */
        if (model == 0){
            return "STEM 10"
        }
        if (model == 1){
            return "STEM 14"
        }
        if (model == 2){
            return "STEM 14"
        }
        if (model == 3){
            return "STEM 16"
        }
        if (model == 4){
            return "STEM 16"
        }
        if (model == 5){
            return "STEM 14"
        }
        if (model == 6){
            return "STEM 14-Z20"
        }
        if (model == 7){
            return "STEM 14-Z20"
        }
        if (model == 8){
            return "STEM 14-Z20-4CH"
        }
        if (model == 9){
            return "STEM 14-Z20-4CH"
        }
        if (model == 10){
            return "STEM 14-Z20-4CH"
        }
        if (model == 11){
            return "STEM 250 12"
        }
        if (model == 12){
            return "STEM 250 12"
        }
        if (model == 13){
            return "STEM 250 12"
        }
        if (model == 14){
            return "STEM 250 12"
        }
        if (model == 15){
            return "STEM 250 12"
        }
        console.log("[FATAL ERROR] Unknown model: " + model)
        return ""
    }

    Desktop.setApplications = function(listOfapplications) {
        $.ajax({
                method: "GET",
                url: '/get_info'
            })
            .done(function(result) {
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

                applications = Desktop.filterApps(applications,Desktop.convertModel(result['stem_ver']));

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
                setTimeout(function() {
                    $('body').addClass('loaded');
                }, 666);
            }).fail(function(msg) { Desktop.setApplications(listOfapplications); });
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
        if (applications[key].url != "")
            window.location = applications[key].url;
        if (applications[key].callback !== undefined)
            applications[key].callback(key);
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
        { id: "tutorials", name: "Create own WEB application", description: "RedPitaya tutorials.", url: "https://redpitaya.readthedocs.io/en/latest/developerGuide/software/build/webapp/webApps.html", image: "../assets/images/tutors.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "wifi", name: "Network manager", description: "Simple way to establish wireless connection with the Red Pitaya", url: "/network_manager/", image: "../network_manager/info/icon.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "scpi", name: "SCPI server", description: "Remote access to all Red Pitaya inputs/outputs from MATLAB/LabVIEW/Scilab/Python", url: "/scpi_manager/", image: "../scpi_manager/info/icon.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "updater", name: "Red Pitaya OS Update", description: "Red Pitaya ecosystem updater", url: "/updater/", image: "../assets/images/updater.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "activelearning", name: "Teaching materials", description: "Teaching materials for Red Pitaya", url: "https://redpitaya-knowledge-base.readthedocs.io/en/latest/learn_fpga/fpga_learn.html", image: "../assets/images/active-learning.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "warranty_ext", name: "Unlock new benefits", description: "Keep your Red Pitaya fresh for longer", url: "https://go.redpitaya.com/refresh", image: "../assets/images/WarrantyExt.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "jupyter", name: "Python programming", description: "Jupyter notebook server for running Python applications in a browser tab", url: "/jlab/lab/tree/RedPitaya/welcome.ipynb", image: "../jupyter_manager/info/icon.png", check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "web_ssh", name: "Web Console", description: "SSH console based on the shellinabox", url: "http://" + window.location.hostname + ":4200", image: "../assets/images/ssh_icon.png", check_online: false, licensable: false, callback: undefined, type: 'run' }
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

        $("#delete_settings_id").click(function(event) {
            $.ajax({
                method: "GET",
                url: '/delete_settings'
            })
            .done(function(result) {
                $("#sysinfo_dialog").modal("hide");
            });
        });

        $("#up_boot_id").click(function(event) {
            $.ajax({
                method: "GET",
                url: '/check_bootbin'
            })
            .done(function(result) {
                console.log("/check_bootbin: res" + result);
                if (result.trim().length === 0){
                    $.ajax({
                        method: "GET",
                        url: '/copy_bootbin_1G'
                    })
                    .done(function(result) {
                        console.log("/copy_bootbin_1G: res" + result);
                        $('#up_boot_id a').text("Make Unify")
                        $('#UBOOT_MODE_ID').text("BOOT mode: SIGNALlab");
                    });
                }else{
                    $.ajax({
                        method: "GET",
                        url: '/copy_bootbin_512'
                    })
                    .done(function(result) {
                        console.log("/copy_bootbin_512: res" + result);
                        $('#up_boot_id a').text("Up to 1Gb RAM")
                        $('#UBOOT_MODE_ID').text("BOOT mode: Unify");
                    });
                }
            });
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
                        if (obj['model'].startsWith('STEM_125-10_v1.0')) model = 'STEMlab 125-10 v1.0';
                        if (obj['model'].startsWith('STEM_125-14_v1.0')) model = 'STEMlab 125-14 v1.0';
                        if (obj['model'].startsWith('STEM_125-14_Z7020_v1.0')) model = 'STEMlab 125-14-Z7020 v1.0';
                        if (obj['model'].startsWith('STEM_250-12_V1.1')) model = 'SIGNALlab 250-12 v1.1';
                        if (obj['model'].startsWith('STEM_250-12_V1.2')) model = 'SIGNALlab 250-12 v1.2';
                        if (obj['model'].startsWith('STEM_250-12_V1.2a')) model = 'SIGNALlab 250-12 v1.2a';
                        if (obj['model'].startsWith('STEM_122-16SDR_v1.0')) model = 'SDRlab 122-16 v1.0';
                        if (obj['model'].startsWith('STEM_122-16SDR_v1.1')) model = 'SDRlab 122-16 v1.1';
                        if (obj['model'].startsWith('STEM_125-14_Z7020_4IN_v1.0')) model = 'STEMlab 125-14 4-Input v1.0';
                        if (obj['model'].includes('SLAVE')) model += " / Streaming Slave";
                        $('#SI_B_MODEL').text(model);
                        $('#SI_MAC').text(obj['mac']);
                        $('#SI_DNA').text(obj['dna']);
                        $('#SI_ECOSYSTEM').text(obj['ecosystem']['version'] + '-' + obj['ecosystem']['revision']);
                        $('#SI_LINUX').text(obj['linux']);
                        $('#RAM_SIZE_ID').text(obj['mem_size']+"Mb");
                        $('#UBOOT_MODE_ID').text(obj['boot_512'] == "1"?"BOOT mode: Unify":"BOOT mode: SIGNALlab");

                        if (obj['mem_upgrade'] == "1"){
                            $('#up_boot_id a').text(obj['boot_512'] == "1"?"Up to 1Gb RAM":"Make Unify")
                            $('#up_boot_id').show()
                        }else{
                            $('#up_boot_id').hide()
                        }

                        var fpga_list = obj['fpga']
                        $("#FPGA_LIST_TABLE_ID tr").remove();
                        Object.keys(obj['fpga']).forEach(element => {
                            $('#FPGA_LIST_TABLE_ID tbody').append('<tr style="height:40px;"><td>'+ element +'</td><td>'+ fpga_list[element] +'</td></tr>');
                        });
                    } catch (error) {
                        console.error(error);
                    }
                })

            $.ajax({
                    method: "GET",
                    url: '/get_led_status'
                })
                .done(function(msg) {
                    msg = msg.trim();
                    if (msg === "1" || msg === "2"){
                        var chkBox = document.getElementById('fsck_chbox');
                        chkBox.checked = true
                        if (msg === "2"){
                            $.ajax({
                                method: "GET",
                                url: '/set_led_status?enable=true'
                            });
                        }
                    }
                    if (msg === "0"){
                        var chkBox = document.getElementById('fsck_chbox');
                        chkBox.checked = false
                    }

                    console.log("LED STATUS ",msg)
                })
                .fail(function(msg) {
                    console.log("LED STATUS fail")
                });

            $.ajax({
                method: "GET",
                url: '/get_fsck_status'
            })
            .done(function(msg) {
                msg = msg.trim().split(" ").slice(-1)[0];
                var chkBox = document.getElementById('fsck_chbox');
                chkBox.checked = (msg !== "-1");

                console.log("FSCK STATUS ",msg)
            })
            .fail(function(msg) {
                console.log("FSCK STATUS fail")
            });

            $('#sysinfo_dialog').modal("show");
        });


        $("#bug_report").click(function(event) {
            fetch('/get_bug_report')
                .then(response => response.blob())
                .then(blob => {
                const link = document.createElement("a");
                link.href = URL.createObjectURL(blob);
                link.download = new Date().toJSON().slice(0,22) + ".zip";
                link.click();
            })
            .catch(console.error);
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

            $('#fsck_chbox').click(function(event){
                var chkBox = document.getElementById('fsck_chbox');
                if (chkBox.checked){
                    $.ajax({
                        method: "GET",
                        url: '/set_fsck?enable=true'
                    });
                }else{
                    $.ajax({
                        method: "GET",
                        url: '/set_fsck?enable=false'
                    });
                }
            });

            $('#led_chbox').click(function(event){
                var chkBox = document.getElementById('led_chbox');
                if (chkBox.checked){
                    $.ajax({
                        method: "GET",
                        url: '/set_led_status?enable=true'
                    });
                }else{
                    $.ajax({
                        method: "GET",
                        url: '/set_led_status?enable=false'
                    });
                }
            });
    });

})(window.Desktop = window.Desktop || {}, jQuery);
