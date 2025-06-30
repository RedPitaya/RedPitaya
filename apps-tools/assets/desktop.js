//-------------------------------------------------
//      Redpitaya desktop
//      Created by Alexey Kaygorodov
//-------------------------------------------------

;
(function(Desktop, $) {

    var applications = [];
    var sys_info_obj = undefined;
    Desktop.sys_info_obj = undefined;

    var base_ram = "512"

    var groups = [{
        name: "System",
        description: "System tools for configuring your Red Pitaya",
        image_path: "../assets/images/pack/system",
        image_sizes: "128;256;512",
        applications: ["updater", "wifi", "licmngr", "calib_app"]
    }, {
        name: "Development",
        description: "Documentation, tutorials and a lot of interesting stuff",
        image_path: "../assets/images/pack/development",
        image_sizes: "128;256;512",
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

    Desktop.setApplications = function(listOfapplications) {
        $.ajax({
                method: "GET",
                url: '/get_sysinfo'
            })
            .done(function(result) {
                result = JSON.parse(result);
                Desktop.sys_info_obj = result;
                setTimeout(RedPitayaOS.printRpVersion(result),2000);
                var linux_path = "LinuxOS";
                if (parseFloat(result["ecosystem"]["linux_ver"]) !== parseFloat(result["linux"])) {
                    $("#CUR_VER").text(result["linux"]);
                    $("#REQ_VER").text(result["ecosystem"]["linux_ver"]);
                    $("#NEED_UPDATE_LINUX_ID").attr("hidden", false);
                    var _href = $("#NEW_FIRMWARE_LINK_ID").attr("href");
                    $("#NEW_FIRMWARE_LINK_ID").attr("href", _href + linux_path);
                }

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

                applications = Desktop.filterApps(applications,result['stem_ver']);

                for (var i = 0; i < groups.length; i++) {
                    var gr = {
                        id: "",
                        name: groups[i].name,
                        description: groups[i].description,
                        url: "#",
                        image_path: groups[i].image_path,
                        image_sizes: groups[i].image_sizes,
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
                txt += '<a href="#" class="app-link"><div class="img-container"><img class="app-icon" path="' + applications[i]['image_path'] + '" sizes="' + applications[i]['image_sizes'] + '"></div><span class="app-name">' + applications[i]['name'] + '</span></a>';
                txt += '</li>';
                $('#list-container').append(txt);
            }
        }

        $('.app-item').click(clickApp);
        $('.app-item').mouseenter(overApp);
        $('.app-item').mouseleave(leaveApp);
        initImageLoaders();
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
        { id: "github", name: "Sources", 
            description: "Access to open source code and programming instructions", 
            url: "https://github.com/redpitaya", 
            image_path: "../assets/images/pack/github",
            image_sizes: "128;256;512",
            check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "applicationstore", name: "Red Pitaya Store", 
            description: "Access to Red Pitaya official store", 
            url: "https://redpitaya.com/shop", 
            image_path: "../assets/images/pack/shop",
            image_sizes: "128;256;512",
            check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "marketplace", name: "Application marketplace", 
            description: "Access to open source and contributed applications", 
            url: "http://bazaar.redpitaya.com/", 
            image_path: "images/pack/download_icon",
            image_sizes: "128;256;512",
            check_online: true, licensable: false, callback: undefined, type: 'run' },
        { id: "feedback", name: "Feedback", 
            description: "Tell us what you like or dislike and what you would like to see improved", 
            url: "", 
            image_path: "../assets/images/pack/feedback",
            image_sizes: "128;256;512",
            check_online: true, licensable: false, callback: showFeedBack, type: 'run' },
        { id: "instructions", name: "Documentation", 
            description: "Quick start instructions, user manuals, specifications, examples & more.", 
            url: "http://redpitaya.readthedocs.io/en/latest/index.html", 
            image_path: "../assets/images/pack/instr",
            image_sizes: "128;256;512",
            check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "tutorials", name: "Create own WEB application", 
            description: "RedPitaya tutorials.", 
            url: "https://redpitaya.readthedocs.io/en/latest/developerGuide/software/build/webapp/webApps.html", 
            image_path: "../assets/images/pack/tutors",
            image_sizes: "128;256;512",
            check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "wifi", name: "Network manager", 
            description: "Simple way to establish wireless connection with the Red Pitaya", 
            url: "/network_manager/", 
            image_path: "../network_manager/info/icon",
            image_sizes: "128;256;512",
            check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "scpi", name: "SCPI server", 
            description: "Remote access to all Red Pitaya inputs/outputs from MATLAB/LabVIEW/Scilab/Python", 
            url: "/scpi_manager/", 
            image_path: "../scpi_manager/info/icon",
            image_sizes: "128;256;512",
            check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "updater", name: "Red Pitaya OS Update", 
            description: "Red Pitaya ecosystem updater", 
            url: "/updater/", 
            image_path: "../updater/info/icon",
            image_sizes: "128;256;512",
            check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "activelearning", name: "Teaching materials", 
            description: "Teaching materials for Red Pitaya", 
            url: "https://redpitaya-knowledge-base.readthedocs.io/en/latest/learn_fpga/fpga_learn.html", 
            image_path: "../assets/images/pack/active-learning",
            image_sizes: "128;256;512",
            check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "warranty_ext", name: "Unlock new benefits", 
            description: "Keep your Red Pitaya fresh for longer", 
            url: "https://go.redpitaya.com/refresh", 
            image_path: "../assets/images/pack/WarrantyExt",
            image_sizes: "128;256;512",
            check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "jupyter", name: "Python programming", 
            description: "Jupyter notebook server for running Python applications in a browser tab", 
            url: "/jlab/lab/tree/RedPitaya/welcome.ipynb", 
            image_path: "../jupyter_manager/info/icon",
            image_sizes: "128;256;512",
            check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "web_ssh", name: "Web Console", 
            description: "SSH console based on the shellinabox", 
            url: "http://" + window.location.hostname + ":4200",
            image_path: "../assets/images/pack/ssh_icon",
            image_sizes: "128;256;512",
            check_online: false, licensable: false, callback: undefined, type: 'run' },
        { id: "pyrpl", name: "PyRPL", 
            description: "PyRPL turns your RedPitaya into a powerful DSP device", 
            url: "https://redpitaya.readthedocs.io/en/latest/appsFeatures/applications/pyrpl/pyrpl.html", 
            image_path: "../assets/images/pack/pyrpl",
            image_sizes: "128;256;512",
            check_online: false, licensable: false, callback: undefined, type: 'run' }
    ];

    var backButton = {
        id: "back",
        name: "Back",
        description: "Return to the desktop",
        url: "#",
        image_path: "../assets/images/pack/back_button",
        image_sizes: "128;256;512",
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
                        $('#up_boot_id a').text( base_ram + "MB RAM")
                        $('#UBOOT_MODE_ID').text("BOOT mode: 1GB RAM");
                    });
                }else{
                    $.ajax({
                        method: "GET",
                        url: '/copy_bootbin_512'
                    })
                    .done(function(result) {
                        console.log("/copy_bootbin_512: res" + result);
                        $('#up_boot_id a').text("Up to 1GB RAM")
                        $('#UBOOT_MODE_ID').text("BOOT mode: " + base_ram + "MB RAM");
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
                        var is_slave = obj['is_slave'];
                        if (model.startsWith('STEMlab 125-10 v1.0')) { base_ram = "256"; }
                        if (is_slave.includes('slave mode')) model += " / Streaming Slave";
                        $('#SI_B_MODEL').text(model);
                        $('#SI_MAC').text(obj['mac']);
                        $('#SI_DNA').text(obj['dna']);
                        $('#SI_ECOSYSTEM').text(obj['ecosystem']['version'] + '-' + obj['ecosystem']['revision']);
                        $('#SI_LINUX').text(obj['linux']);
                        $('#UBOOT_MODE_ID').text(obj['boot_512'] == "1" ? "BOOT mode: " + base_ram + "MB RAM" : "BOOT mode: 1GB RAM");

                        if (obj['mem_upgrade'] == "1"){
                            $('#up_boot_id a').text(obj['boot_512'] == "1" ? "Up to 1GB RAM" : base_ram + "MB RAM")
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

        $("#ext_con_but").click(function(event) {
            $('#ext_connections_dialog').modal("show");
        });

        $("#bug_report").click(async function (event){

            const blob =  await fetch('/get_bug_report', {
                    method: 'POST'
                })
                .then(resp => resp.blob());

            if( window.showSaveFilePicker ) {
                const handle = await showSaveFilePicker({
                    suggestedName: new Date().toJSON().slice(0,22) + ".zip" });
                const writable = await handle.createWritable();
                await writable.write( blob );
                writable.close();
            }
            else {
                const saveImg = document.createElement( "a" );
                saveImg.href = URL.createObjectURL( blob );;
                saveImg.download= new Date().toJSON().slice(0,22) + ".zip";
                saveImg.click();
                setTimeout(() => URL.revokeObjectURL( saveImg.href ), 60000 );
                alert("The debug file has been generated. Check the downloads section.");
            }
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
