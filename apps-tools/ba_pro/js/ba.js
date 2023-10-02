/*
 * Red Pitaya Bode analyzer client
 *
 * Author: Dakus <info@eskala.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


(function() {
    var originalAddClassMethod = jQuery.fn.addClass;
    var originalRemoveClassMethod = jQuery.fn.removeClass;
    $.fn.addClass = function(clss) {
        var result = originalAddClassMethod.apply(this, arguments);
        $(this).trigger('activeChanged', 'add');
        return result;
    };
    $.fn.removeClass = function(clss) {
        var result = originalRemoveClassMethod.apply(this, arguments);
        $(this).trigger('activeChanged', 'remove');
        return result;
    }
})();


//Bode analyser
(function(BA, $, undefined) {

    // Params cache
    BA.params = {
        orig: {},
        local: {}
    };

    BA.scale = true;
    BA.curGraphScale = true;
    BA.param_callbacks = {};

    // App configuration
    BA.config = {};
    BA.config.app_id = 'ba_pro';
    BA.config.server_ip = ''; // Leave empty on production, it is used for testing only
    BA.config.start_app_url = (BA.config.server_ip.length ? 'http://' + BA.config.server_ip : '') + '/bazaar?start=' + BA.config.app_id + '?' + location.search.substr(1);
    BA.config.stop_app_url = (BA.config.server_ip.length ? 'http://' + BA.config.server_ip : '') + '/bazaar?stop=' + BA.config.app_id;
    BA.config.socket_url = 'ws://' + (BA.config.server_ip.length ? BA.config.server_ip : window.location.hostname) + '/wss'; // WebSocket server URI
    BA.client_id = undefined;
    // App state
    BA.state = {
        socket_opened: false,
        processing: false,
        editing: false,
        trig_dragging: false,
        cursor_dragging: false,
        mouseover: false,
        resized: false,
        graph_grid_height: null,
        graph_grid_width: null,
        demo_label_visible: false,
        cursor_dragging: false
    };

    BA.firstValGot = { 'BA_SIGNAL_1': false, 'BA_SIGNAL_2': false };

    BA.lastSignals = { 'BA_SIGNAL_1': undefined, 'BA_SIGNAL_2': undefined, 'BA_SIGNAL_1_BAD': undefined, 'BA_SIGNAL_2_BAD': undefined };

    // Current step and freq
    BA.currentFreq = 0;
    BA.currentStep = 0;

    // Parameters cache
    BA.parametersCache = {};

    //Graph cache
    BA.graphCache = undefined;

    // Other global variables
    BA.ws = null;

    BA.running = true;
    BA.calibrating = false;
    BA.unexpectedClose = false;

    BA.parameterStack = [];
    BA.signalStack = [];

    BA.compressed_data = 0;
    BA.decompressed_data = 0;
    BA.cur_freq = 0;
    BA.input_threshold = 0;

    var g_PacketsRecv = 0;
    var g_CpuLoad = 100.0;
    var g_TotalMemory = 256.0;
    var g_FreeMemory = 256.0;


    BA.cursorsRelative = {
        x1: 0.33,
        x2: 0.66,
        y1: 0.33,
        y2: 0.66,
        z1: 0.33,
        z2: 0.66
    };

    BA.graphSize = {
        left: 0,
        right: 0,
        top: 0,
        bottom: 0,
    };




    // Starts the body analyzer application on server
    BA.startApp = function() {
        $.get(
                BA.config.start_app_url
            )
            .done(function(dresult) {
                if (dresult.status == 'OK') {
                    try {
                        BA.connectWebSocket();
                    } catch (e) {
                        BA.startApp();
                    }
                } else if (dresult.status == 'ERROR') {
                    console.log(dresult.reason ? dresult.reason : 'Could not start the application (ERR1)');
                    BA.startApp();
                } else {
                    console.log('Could not start the application (ERR2)');
                    BA.startApp();
                }
            })
            .fail(function() {
                console.log('Could not start the application (ERR3)');
                BA.startApp();
            });
    };




    //Show license dialog
    var showLicenseDialog = function() {
        if (BA.state.demo_label_visible)
            $('#get_lic').modal('show');
    }


    //Write email
    BA.formEmail = function() {
        //var file = new FileReader();
        var mail = "support@redpitaya.com";
        var subject = "Crash report Red Pitaya OS";
        var body = "%0D%0A%0D%0A------------------------------------%0D%0A" + "DEBUG INFO, DO NOT EDIT!%0D%0A" + "------------------------------------%0D%0A%0D%0A";
        body += "Parameters:" + "%0D%0A" + JSON.stringify({ parameters: BA.parametersCache }) + "%0D%0A";
        body += "Browser:" + "%0D%0A" + JSON.stringify({ parameters: $.browser }) + "%0D%0A";

        var url = 'info/info.json';
        $.ajax({
            method: "GET",
            url: url
        }).done(function(msg) {
            body += " info.json: " + "%0D%0A" + msg.responseText;
        }).fail(function(msg) {
            var info_json = msg.responseText
            var ver = '';
            try {
                var obj = JSON.parse(msg.responseText);
                ver = " " + obj['version'];
            } catch (e) {};

            body += " info.json: " + "%0D%0A" + msg.responseText;
            document.location.href = "mailto:" + mail + "?subject=" + subject + ver + "&body=" + body;
        });
    }


    BA.checkStatusTimer = undefined;
    BA.changeStatusForRestart = false;
    BA.changeStatusStep = 0;

    BA.reloadPage = function() {
        $.ajax({
            method: "GET",
            url: "/get_client_id",
            timeout: 2000
        }).done(function(msg) {
            if (msg.trim() === BA.client_id) {
                location.reload();
            } else {
                $('body').removeClass('user_lost');
                BA.stopCheckStatus();
            }
        }).fail(function(msg) {
            console.log(msg);
            $('body').removeClass('connection_lost');
        });
    }

    BA.startCheckStatus = function() {
        if (BA.checkStatusTimer === undefined) {
            BA.changeStatusStep = 0;
            BA.checkStatusTimer = setInterval(BA.checkStatus, 4000);
        }
    }

    BA.stopCheckStatus = function() {
        if (BA.checkStatusTimer !== undefined) {
            clearInterval(BA.checkStatusTimer);
            BA.checkStatusTimer = undefined;
        }
    }

    BA.checkStatus = function() {
        $.ajax({
            method: "GET",
            url: "/check_status",
            timeout: 2000
        }).done(function(msg) {
            switch (BA.changeStatusStep) {
                case 0:
                    BA.changeStatusStep = 1;
                    break;
                case 2:
                    BA.reloadPage();
                    break;
            }
        }).fail(function(msg) {
            // check status. If don't have good state after start. We lock system.
            $('body').removeClass('connection_lost');
            switch (BA.changeStatusStep) {
                case 0:
                    BA.changeStatusStep = -1;
                    break;
                case 1:
                    BA.changeStatusStep = 2;
                    break;
            }

        });
    }


    // Creates a WebSocket connection with the web server
    BA.connectWebSocket = function() {

        if (window.WebSocket) {
            BA.ws = new WebSocket(BA.config.socket_url);
            BA.ws.binaryType = "arraybuffer";
        } else if (window.MozWebSocket) {
            BA.ws = new MozWebSocket(BA.config.socket_url);
            BA.ws.binaryType = "arraybuffer";
        } else {
            console.log('Browser does not support WebSocket');
        }

        // Define WebSocket event listeners
        if (BA.ws) {
            BA.ws.onopen = function() {
                console.log('Socket opened');

                BA.state.socket_opened = true;

                BA.sendParameters();

                //setTimeout(showLicenseDialog, 2500);
                BA.unexpectedClose = true;
                $('body').addClass('loaded');
                $('body').addClass('connection_lost');
                $('body').addClass('user_lost');
                BA.startCheckStatus();
            };

            BA.ws.onclose = function() {
                BA.state.socket_opened = false;
                $('#graph_amplitude .plot').hide(); // Hide all graphs
                $('#graph_phase .plot').hide(); // Hide all graphs
                console.log('Socket closed');
                if (BA.unexpectedClose == true) {
                    setTimeout(BA.reloadPage, '1000');
                }
            };

            BA.ws.onerror = function(ev) {
                if (!BA.state.socket_opened)
                    BA.startApp();
                console.log('Websocket error: ', ev);
            };

            var last_time = undefined;
            BA.ws.onmessage = function(ev) {
                var start_time = +new Date();
                if (BA.state.processing) {
                    return;
                }
                BA.state.processing = true;

                try {
                    var data = new Uint8Array(ev.data);
                    BA.compressed_data += data.length;
                    var inflate = pako.inflate(data);
                    var text = String.fromCharCode.apply(null, new Uint8Array(inflate));

                    BA.decompressed_data += text.length;
                    var receive = JSON.parse(text);

                    //Recieving parameters
                    if (receive.parameters) {
                        BA.parameterStack.push(receive.parameters);
                        if (Object.keys(receive.parameters).length == 0) {
                            BA.sendParameters();
                        }
                    }

                    //Recieve signals
                    if (receive.signals) {
                        g_PacketsRecv++;
                        BA.signalStack.push(receive.signals);
                    }

                    BA.state.processing = false;
                } catch (e) {
                    BA.state.processing = false;
                    console.log(e);
                } finally {
                    BA.state.processing = false;
                }
            };
        }
    };

    // For Firefox
    function fireEvent(obj, evt) {
        var fireOnThis = obj;
        if (document.createEvent) {
            var evObj = document.createEvent('MouseEvents');
            evObj.initEvent(evt, true, false);
            fireOnThis.dispatchEvent(evObj);

        } else if (document.createEventObject) {
            var evObj = document.createEventObject();
            fireOnThis.fireEvent('on' + evt, evObj);
        }
    }

    BA.SaveGraphs = function() {
        html2canvas($('body'), {
            background: '#343433', // Like background of BODY
            onrendered: function(canvas) {
                var a = document.createElement('a');
                a.href = canvas.toDataURL('image/jpeg').replace('image/jpeg', 'image/octet-stream');
                a.download = 'graphs.jpg';
                fireEvent(a, 'click');
            }
        });
    }

    BA.downloadDataAsCSV = function(filename) {
        var header = 'Frequency [Hz], Amplitude [dB], Phase [deg]';
        var row_delim = '\n';
        var strings = header + row_delim;

        var amplitude_signal = BA.lastSignals['BA_SIGNAL_1'];
        var phase_signal = BA.lastSignals['BA_SIGNAL_2'];

        if (amplitude_signal.length === phase_signal.length) {
            for (var i = 0; i < amplitude_signal.length; ++i) {
                strings += amplitude_signal[i][0] + ', ' + amplitude_signal[i][1] + ', ' + phase_signal[i][1] + row_delim;
            }
        }

        var a = document.createElement('a');
        a.setAttribute('download', filename);
        a.setAttribute('href', 'data:' + 'text/html' + ';charset=utf-8,' + encodeURIComponent(strings));
        fireEvent(a, 'click');
    }

    // Sends to server parameters
    BA.sendParameters = function() {
        if (!BA.state.socket_opened) {
            console.log('ERROR: Cannot save changes, socket not opened');
            return false;
        }

        BA.parametersCache["in_command"] = { value: "send_all_params" };
        BA.ws.send(JSON.stringify({ parameters: BA.parametersCache }));
        BA.parametersCache = {};
        return true;
    };




    // Draws grid on the lowest canvas layer
    BA.drawGrid = function() {
        var canvas_width = $('#graph_bode').width() - 2;
        var canvas_height = Math.round(canvas_width / 2);

        //Draw grid
        var ctx = $('#graph_bode_grid')[0].getContext('2d');

        // Set canvas size
        ctx.canvas.width = canvas_width;
        ctx.canvas.height = canvas_height;

        return;
    };




    BA.enableX = function(cursor_name, new_params) {
        var old_params = $.extend(true, {}, BA.params.old);
        if (!BA.state.cursor_dragging && !BA.state.mouseover) {
            var x = (cursor_name == 'BA_CURSOR_X1' ? 'x1' : 'x2');

            if (new_params[cursor_name].value) {
                $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').show();
            } else {
                $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').hide();
            }
            BA.updateXLinesAndArrows();
        }
    }




    BA.moveX = function(ui) {

        if (ui.helper[0].id == 'cur_x1_arrow') {
            BA.cursorsRelative.x1 = ($("#cur_x1_arrow").offset().left - BA.graphSize.left) / (BA.graphSize.right - BA.graphSize.left);
            if (BA.cursorsRelative.x1 < 0)
                BA.cursorsRelative.x1 = 0;
            if (BA.cursorsRelative.x1 > 1)
                BA.cursorsRelative.x1 = 1;

        } else if (ui.helper[0].id == 'cur_x2_arrow') {
            BA.cursorsRelative.x2 = ($("#cur_x2_arrow").offset().left - BA.graphSize.left) / (BA.graphSize.right - BA.graphSize.left);
            if (BA.cursorsRelative.x2 < 0)
                BA.cursorsRelative.x2 = 0;
            if (BA.cursorsRelative.x2 > 1)
                BA.cursorsRelative.x2 = 1;
        }
        BA.updateXLinesAndArrows();
    }




    BA.enableY = function(cursor_name, new_params) {
        var old_params = $.extend(true, {}, BA.params.old);
        if (!BA.state.cursor_dragging && !BA.state.mouseover) {
            var y = (cursor_name == 'BA_CURSOR_Y1' ? 'y1' : 'y2');

            if (new_params[cursor_name].value) {
                $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').show();
            } else {
                $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').hide();
            }
            BA.updateYLinesAndArrows();
        }
    }




    BA.moveY = function(ui) {

        if (ui.helper[0].id == 'cur_y1_arrow') {
            BA.cursorsRelative.y1 = ($("#cur_y1_arrow").offset().top - BA.graphSize.top) / (BA.graphSize.bottom - BA.graphSize.top);
            if (BA.cursorsRelative.y1 < 0)
                BA.cursorsRelative.y1 = 0;
            if (BA.cursorsRelative.y1 > 1)
                BA.cursorsRelative.y1 = 1;

        } else if (ui.helper[0].id == 'cur_y2_arrow') {
            BA.cursorsRelative.y2 = ($("#cur_y2_arrow").offset().top - BA.graphSize.top) / (BA.graphSize.bottom - BA.graphSize.top);
            if (BA.cursorsRelative.y2 < 0)
                BA.cursorsRelative.y2 = 0;
            if (BA.cursorsRelative.y2 > 1)
                BA.cursorsRelative.y2 = 1;
        }
        BA.updateYLinesAndArrows();
    }




    BA.enableZ = function(cursor_name, new_params) {
        var old_params = $.extend(true, {}, BA.params.old);
        if (!BA.state.cursor_dragging && !BA.state.mouseover) {
            var z = (cursor_name == 'BA_CURSOR_Z1' ? 'z1' : 'z2');

            if (new_params[cursor_name].value) {
                $('#cur_' + z + '_arrow, #cur_' + z + ', #cur_' + z + '_info').show();
            } else {
                $('#cur_' + z + '_arrow, #cur_' + z + ', #cur_' + z + '_info').hide();
            }
            BA.updateZLinesAndArrows();
        }
    }




    BA.moveZ = function(ui) {

        if (ui.helper[0].id == 'cur_z1_arrow') {
            BA.cursorsRelative.z1 = ($("#cur_z1_arrow").offset().top - BA.graphSize.top) / (BA.graphSize.bottom - BA.graphSize.top);
            if (BA.cursorsRelative.z1 < 0)
                BA.cursorsRelative.z1 = 0;
            if (BA.cursorsRelative.z1 > 1)
                BA.cursorsRelative.z1 = 1;

        } else if (ui.helper[0].id == 'cur_z2_arrow') {
            BA.cursorsRelative.z2 = ($("#cur_z2_arrow").offset().top - BA.graphSize.top) / (BA.graphSize.bottom - BA.graphSize.top);
            if (BA.cursorsRelative.z2 < 0)
                BA.cursorsRelative.z2 = 0;
            if (BA.cursorsRelative.z2 > 1)
                BA.cursorsRelative.z2 = 1;
        }
        BA.updateZLinesAndArrows();
    }




    function funcxTickFormat(val, axis) {

        if (Math.abs(val) >= 0 && Math.abs(val) < 1000)
            return (val * 1).toFixed(0) + " ";
        else if (Math.abs(val) >= 1000 && Math.abs(val) < 1000000)
            return (val / 1000).toFixed(2) + "k";
        else if (Math.abs(val) >= 1000000)
            return (val / 1000000).toFixed(2) + "M";
    }




    //Draw one signal
    BA.prepareOneSignal = function(signal_name) {
        var one_signal = BA.signalStack[0][signal_name];
        var bad_signal = BA.signalStack[0]['BA_BAD_SIGNAL'];

        // Ignore empty signals
        if (one_signal.size == 0)
            return;

        // Create points mas
        var points = [];
        var points_bad = [];
        var startFreq = 0,
            endFreq = 0,
            steps = 0,
            step = 0,
            val = 0;

        startFreq = parseInt($('#BA_START_FREQ').val()) * 1;
        endFreq = parseInt($('#BA_END_FREQ').val()) * 1;
        steps = parseInt($('#BA_STEPS').val()) * 1;
        step = (endFreq - startFreq) / (steps - 1);


        for (var i = 0; i < one_signal.size; i++) {
            var freq = startFreq + i * step;
            if (BA.scale) {
                var a = Math.log10(startFreq);
                var b = Math.log10(endFreq);
                var c = (b - a) / (steps - 1);
                freq = Math.pow(10, c * i + a);
            }
            BA.freq = freq;
            if (i > 0) {
                if (bad_signal.value[i] != bad_signal.value[i - 1]) {
                    var f = (freq + freq_old) / 2.0;
                    var s = (one_signal.value[i] + one_signal.value[i - 1]) / 2.0;
                    points.push([f, s]);
                    points_bad.push([f, s]);
                    if (bad_signal.value[i] == 0) {
                        points_bad.push([undefined, undefined]);
                    } else {
                        points.push([undefined, undefined]);
                    }
                }
            }

            if (bad_signal.value[i] == 0) {
                points.push([freq, one_signal.value[i]]);
            } else {
                points_bad.push([freq, one_signal.value[i]]);
            }
            BA.lastSignals[signal_name] = points;
            BA.lastSignals[signal_name + "_BAD"] = points_bad;
            freq_old = freq;
        }



        // AUTOSCALE if switched on
        if ($('#BA_AUTOSCALE_BTN').hasClass('active')) {
            // For autoscale of max Y value
            var l = BA.lastSignals[signal_name].length;
            var v = l > 0 ? parseFloat(BA.lastSignals[signal_name][l - 1][1]).toFixed(3) * 1 : 0;
            if ($('#BA_SHOWALL_BTN').hasClass('active')) {
                l = one_signal['value'].length;
                v = l > 0 ? parseFloat(one_signal['value'][l - 1]).toFixed(3) * 1 : 0;
            }
            var v2 = 0;
            var yaxis;


            // Choose right yaxis
            if (signal_name == "BA_SIGNAL_1")
                yaxis = BA.graphCache.plot.getAxes().yaxis;
            else
                yaxis = BA.graphCache.plot.getAxes().y2axis;


            // For autoscale of max Y value
            if (v >= 0) {
                v2 = parseFloat($('#' + signal_name + '_MAX').val()) / 1.2;
                if (l == 1) {
                    $('#' + signal_name + '_MAX').val(((1 + v) * 1.2).toFixed(3));
                    yaxis.options.max = (1 + v) * 1.2;
                } else if (v > v2) {
                    $('#' + signal_name + '_MAX').val((v * 1.2).toFixed(3));
                    yaxis.options.max = v * 1.2;
                }
            } else {
                var v2 = parseFloat($('#' + signal_name + '_MAX').val()) * 1.2;
                if (l == 1) {
                    $('#' + signal_name + '_MAX').val(((1 + v) / 1.2).toFixed(3));
                    yaxis.options.max = (1 + v) / 1.2;
                } else if (v > v2) {
                    $('#' + signal_name + '_MAX').val((v / 1.2).toFixed(3));
                    yaxis.options.max = v / 1.2;
                }
            }


            // For autoscale of min Y value
            if (v >= 0) {
                v2 = parseFloat($('#' + signal_name + '_MIN').val()) * 1.2;
                if (l == 1) {
                    $('#' + signal_name + '_MIN').val((1.2 / (-1 + v)).toFixed(3));
                    yaxis.options.min = 1.2 / (-1 + v);
                } else if (v < v2) {
                    $('#' + signal_name + '_MIN').val((1.2 / v).toFixed(3));
                    yaxis.options.min = 1.2 / v;
                }
            } else {
                v2 = parseFloat($('#' + signal_name + '_MIN').val()) / 1.2;
                if (l == 1) {
                    $('#' + signal_name + '_MIN').val((1.2 * (-1 + v)).toFixed(3));
                    yaxis.options.min = 1.2 * (-1 + v);
                } else if (v < v2) {
                    $('#' + signal_name + '_MIN').val((1.2 * v).toFixed(3));
                    yaxis.options.min = 1.2 * v;
                }
            }
        }
    }


    BA.initPlot = function(update) {
        delete BA.graphCache;
        $('#bode_plot').remove();


        var yMin1 = parseFloat($('#BA_SIGNAL_1_MIN').val()) * 1;
        var yMax1 = parseFloat($('#BA_SIGNAL_1_MAX').val()) * 1;

        var yMin2 = parseFloat($('#BA_SIGNAL_2_MIN').val()) * 1;
        var yMax2 = parseFloat($('#BA_SIGNAL_2_MAX').val()) * 1;

        BA.graphCache = {};
        BA.graphCache.elem = $('<div id="bode_plot" class="plot" />').css($('#graph_bode_grid').css(['height', 'width'])).appendTo('#graph_bode');

        var t = [];
        if ($("#BA_SCALE1").hasClass("active"))
            t = [1, 10, 100, 200, 400, 600, 800, 1000, 2000, 4000, 6000, 8000, 10000, 20000, 40000, 60000, 80000, 100000, 200000, 400000, 600000, 800000, 1000000, 2000000, 4000000, 6000000, 8000000, 10000000, , 20000000, 40000000, 60000000, 80000000, 100000000];
        else
            t = null;

        var options = {
            series: {
                shadowSize: 0
            },
            yaxes: [{
                    min: yMin1,
                    max: yMax1,
                    labelWidth: 30,
                    alignTicksWithAxis: 1,
                    position: "left"
                },
                {
                    min: yMin2,
                    max: yMax2,
                    labelWidth: 30,
                    alignTicksWithAxis: 2,
                    position: "right"
                }
            ],
            xaxis: {
                color: '#aaaaaa',
                tickColor: '#aaaaaa',
                ticks: t,
                transform: function(v) {
                    if (BA.scale)
                        return Math.log(v + 0.0001); // move away from zero
                    else
                        return v;

                },
                tickDecimals: 0,
                reserveSpace: false,
                tickFormatter: funcxTickFormat,
                min: null,
                max: null,
            },
            grid: {
                show: true,
                color: '#aaaaaa',
                borderColor: '#aaaaaa',
                tickColor: '#aaaaaa',
                tickColor: '#aaaaaa',
                markingsColor: '#aaaaaa'
            },
            legend: {
                position: "sw",
                backgroundOpacity: 0.15
            }
        };


        if ($("#BA_SCALE0").hasClass("active")) {
            options.xaxis.transform = null;
        }

        var lastsig1 = [];
        var lastsig2 = [];
        var lastsig1_bad = [];
        var lastsig2_bad = [];
        if (update == true) {
            lastsig1 = BA.lastSignals["BA_SIGNAL_1"];
            lastsig2 = BA.lastSignals["BA_SIGNAL_2"];
           lastsig1_bad = BA.lastSignals["BA_SIGNAL_1_BAD"];
           lastsig2_bad = BA.lastSignals["BA_SIGNAL_2_BAD"];
        }
        var data_points = [{ data: lastsig1, color: '#f3ec1a', label: "Amplitude" }, { data: lastsig2, color: '#31b44b', label: "Phase", yaxis: 2 }];
        if ($('#BA_SHOWALL_BTN').hasClass('active')) {
            data_points.push({ data: lastsig1_bad, color: '#d26500', label: "Invalid amplitude" });
            data_points.push({ data: lastsig2_bad, color: '#685b00', label: "Invalid phase", yaxis: 2 });
        }
        BA.graphCache.plot = $.plot(BA.graphCache.elem, data_points, options);
        $('.flot-text').css('color', '#aaaaaa');
    }


    //Draw signals
    BA.drawSignals = function() {

        if (BA.running == true) {

            var lastsig1 = [];
            var lastsig2 = [];
            var lastsig1_bad = [];
            var lastsig2_bad = [];

            // If there is graph on screen
            if (BA.graphCache == undefined) {
                BA.initPlot(false);
            }


            // Prepare every signal
            BA.prepareOneSignal('BA_SIGNAL_1');
            BA.prepareOneSignal('BA_SIGNAL_2');

            lastsig1 = BA.lastSignals["BA_SIGNAL_1"];
            lastsig2 = BA.lastSignals["BA_SIGNAL_2"];
            lastsig1_bad = BA.lastSignals["BA_SIGNAL_1_BAD"];
            lastsig2_bad = BA.lastSignals["BA_SIGNAL_2_BAD"];

            BA.graphCache.elem.show();
            BA.graphCache.plot.resize();
            BA.graphCache.plot.setupGrid();
            var data_points = [{ data: lastsig1, color: '#f3ec1a', label: "Amplitude" }, { data: lastsig2, color: '#31b44b', label: "Phase", yaxis: 2 }];
            if ($('#BA_SHOWALL_BTN').hasClass('active')) {
                data_points.push({ data: lastsig1_bad, color: '#d26500', label: "Invalid amplitude" });
                data_points.push({ data: lastsig2_bad, color: '#685b00', label: "Invalid phase", yaxis: 2 });
            }
            BA.graphCache.plot.setData(data_points);
            BA.graphCache.plot.draw();
            BA.updateLinesAndArrows();

            // Reset resize flag
            BA.state.resized = false;
        }
    };


    BA.enableCursor = function(x) {
        var x2 = (x[1] == '1') ? x[0] + '2' : x[0] + '1';
        var d = (x[1] == '1') ? '1' : '2';
        $('cur_' + x).show();
        $('cur_' + x + '_info').show();
        $('cur_' + x + '_arrow').show();

        if ($('cur_' + x2).is(':visible')) {
            $('cur_' + x[0] + '_diff').show();
            $('cur_' + x[0] + '_diff_info').show();
        }

        BA.params.local['BA_CURSOR_' + x[0].toUpperCase() + '1'] = { value: 1 };
        BA.params.local['BA_CURSOR_' + x[0].toUpperCase() + '2'] = { value: 1 };

        BA.params.local['BA_CUR1_T'] = { value: 1 };
        BA.params.local['BA_CUR2_T'] = { value: 1 };

        if (x[0] == 'x') {
            BA.enableX('BA_CURSOR_' + x[0].toUpperCase() + d, BA.params.local);
            BA.enableX('BA_CURSOR_' + x[0].toUpperCase() + d, BA.params.local);
        } else if (x[0] == 'y') {
            BA.enableY('BA_CURSOR_' + x[0].toUpperCase() + d, BA.params.local);
            BA.enableY('BA_CURSOR_' + x[0].toUpperCase() + d, BA.params.local);
        } else if (x[0] == 'z') {
            BA.enableZ('BA_CURSOR_' + x[0].toUpperCase() + d, BA.params.local);
            BA.enableZ('BA_CURSOR_' + x[0].toUpperCase() + d, BA.params.local);
        }

        BA.params.local = {};
    };

    BA.disableCursor = function(x) {
        var d = (x[1] == '1') ? '1' : '2';

        $('cur_' + x).hide();
        $('cur_' + x + '_info').hide();
        $('cur_' + x + '_arrow').hide();
        $('cur_' + x[0] + '_diff').hide();
        $('cur_' + x[0] + '_diff_info').hide();

        BA.params.local['BA_CURSOR_' + x[0].toUpperCase() + '1'] = { value: 0 };
        BA.params.local['BA_CURSOR_' + x[0].toUpperCase() + '2'] = { value: 0 };

        if (x[0] == 'x') {
            BA.enableX('BA_CURSOR_' + x[0].toUpperCase() + d, BA.params.local);
            BA.enableX('BA_CURSOR_' + x[0].toUpperCase() + d, BA.params.local);
        } else if (x[0] == 'y') {
            BA.enableY('BA_CURSOR_' + x[0].toUpperCase() + d, BA.params.local);
            BA.enableY('BA_CURSOR_' + x[0].toUpperCase() + d, BA.params.local);
        } else if (x[0] == 'z') {
            BA.enableZ('BA_CURSOR_' + x[0].toUpperCase() + d, BA.params.local);
            BA.enableZ('BA_CURSOR_' + x[0].toUpperCase() + d, BA.params.local);
        }

        BA.params.local = {};
    };



    //Handlers
    var signalsHandler = function() {
        if (BA.signalStack.length > 0) {
            BA.drawSignals();
            BA.signalStack.splice(0, 1);
        }
        if (BA.signalStack.length > 2)
            BA.signalStack.length = [];
    }

    BA.processParameters = function(new_params) {
        var old_params = $.extend(true, {}, BA.params.orig);

        var send_all_params = Object.keys(new_params).indexOf('send_all_params') != -1;
        for (var param_name in new_params) {
            BA.params.orig[param_name] = new_params[param_name];
            if (BA.param_callbacks[param_name] !== undefined)
                BA.param_callbacks[param_name](new_params);
        }
        // Resize double-headed arrows showing the difference between cursors
    };

    var parametersHandler = function() {
        if (BA.parameterStack.length > 0) {
            BA.processParameters(BA.parameterStack[0]);
            BA.parameterStack.splice(0, 1);
        }
    }


    //Set handlers timers
    setInterval(signalsHandler, 10);
    setInterval(parametersHandler, 10);




    BA.process_run = function(new_params) {
        if (new_params['BA_MEASURE_START'].value === false) {
            $('#BA_STOP').hide();
            $('#BA_RUN').show();
            BA.running = false;
            $('#measuring-status').hide();
        } else {
            $('#BA_STOP').css('display', 'block');
            $('#BA_RUN').hide();
            BA.running = true;
            $('#measuring-status').show();
        }
    }

    BA.calib_stop = function(new_params) {
        if ((new_params['BA_CALIBRATE_START'].value === false) && (BA.calibrating == true)) {
            BA.calibrating = false;
            $('body').addClass('loaded');
            $('#calibration').hide();
        }
    }

    BA.calib_enable = function(new_params) {
        if ((new_params['BA_CALIBRATE_ENABLE'].value === false)) {
            $("#BA_CALIB_STATE").attr("src", "img/red_led.png");
        } else {
            $("#BA_CALIB_STATE").attr("src", "img/green_led.png");
        }
    }

    BA.change_cur_freq = function(new_params) {

        var val = new_params['BA_CURRENT_FREQ'].value;
        BA.cur_freq = val;
        var result = "";

        if (val >= 1000000) {
            val = val / 1000000;
            result = val.toFixed(2) + " MHz";
        } else if (val >= 1000) {
            val = val / 1000;
            result = val.toFixed(2) + " kHz";
        } else {
            result = val + " Hz";
        }

        $('#BA_CUR_FREQ').text(result);
    }

    BA.change_cur_step = function(new_params) {
        $('#BA_CUR_STEP').text(new_params['BA_CURRENT_STEP'].value);
    }

    BA.change_input_threshold = function(new_params) {
        var x = parseFloat(BA.input_threshold);
        var y = parseFloat(new_params['BA_INPUT_THRESHOLD'].value);
        if (x !== y) {
            $('#BA_INPUT_THRESHOLD').val(y);
            BA.input_threshold = y;
        }
    }

    BA.updateGraphSize = function() {
        BA.graphSize.left = $("#graphs_holder").offset().left + 78;
        BA.graphSize.right = BA.graphSize.left + $("#graphs_holder").width() - 81;
        BA.graphSize.top = $("#graph_bode").offset().top - 1;
        BA.graphSize.bottom = BA.graphSize.top + $("#graph_bode").height() - 37;
    }


    // X-cursors
    BA.updateXLinesAndArrows = function() {

        var axes = BA.graphCache.plot.getAxes();
        var diff_px = 0;
        let unit = ' Hz';
        let min = axes.xaxis.min;
        let max = axes.xaxis.max;
        let new_val = 0;

        if (min < 1)
            min = 1;


        if (BA.curGraphScale) {
            var a = Math.log10(min);
            var b = Math.log10(max);
            var c = (b - a) * BA.cursorsRelative.x1
            new_val = Math.pow(10, a + c);
        } else {
            new_val = (max - min) * BA.cursorsRelative.x1 + min;
        }

        $('#cur_x1_arrow').offset({
            left: BA.graphSize.left + ((BA.graphSize.right - BA.graphSize.left) * BA.cursorsRelative.x1),
            top: BA.graphSize.bottom - 17
        });
        $('#cur_x1, #cur_x1_info').offset({
            left: $('#cur_x1_arrow').offset().left + $('#cur_x1_arrow').width() / 2
        });
        $('#cur_x1_info').offset({ top: BA.graphSize.top + $('#cur_x1_info').outerHeight() + 30 });
        $('#cur_x1_info').html(new_val.toFixed(2) + unit).data('cleanval', +new_val);
        $('#cur_x1').height(BA.graphSize.bottom - BA.graphSize.top);


        if (BA.curGraphScale) {
            var a = Math.log10(min);
            var b = Math.log10(max);
            var c = (b - a) * BA.cursorsRelative.x2
            new_val = Math.pow(10, a + c);

        } else {
            new_val = (max - min) * BA.cursorsRelative.x2 + min;
        }

        $('#cur_x2_arrow').offset({
            left: BA.graphSize.left + ((BA.graphSize.right - BA.graphSize.left) * BA.cursorsRelative.x2),
            top: BA.graphSize.bottom - 17
        });
        $('#cur_x2, #cur_x2_info').offset({
            left: $('#cur_x2_arrow').offset().left + $('#cur_x2_arrow').width() / 2
        });
        $('#cur_x2_info').offset({ top: BA.graphSize.top + $('#cur_x2_info').outerHeight() + 30 });
        $('#cur_x2_info').html(new_val.toFixed(2) + unit).data('cleanval', +new_val);
        $('#cur_x2').height(BA.graphSize.bottom - BA.graphSize.top);


        diff_px = Math.abs($('#cur_x1').offset().left - $('#cur_x2').offset().left) - 6;
        if ($('#cur_x1').is(':visible') && $('#cur_x2').is(':visible') && diff_px > 12) {
            var left = Math.min($('#cur_x1').offset().left, $('#cur_x2').offset().left) + 3;
            var value = $('#cur_x1_info').data('cleanval') - $('#cur_x2_info').data('cleanval');

            $('#cur_x_diff')
                .offset({ left: left })
                .width(diff_px)
                .show();
            $('#cur_x_diff_info')
                .html(Math.abs(value).toFixed(2) + unit)
                .offset({ left: left + diff_px / 2 - 2 - $('#cur_x_diff_info').outerWidth() / 2 })
                .show();
        } else {
            $('#cur_x_diff, #cur_x_diff_info').hide();
        }
    }


    // Y-cursors
    BA.updateYLinesAndArrows = function() {
        var diff_px = 0;
        let unit = ' dB';
        let min = parseFloat($('#BA_SIGNAL_1_MIN').val());
        let max = parseFloat($('#BA_SIGNAL_1_MAX').val());
        let new_val = 0;


        new_val = (min - max) * BA.cursorsRelative.y1 + max;

        $('#cur_y1_arrow').offset({
            left: BA.graphSize.right - $('#cur_y1_arrow').width() / 2 - 2,
            top: BA.graphSize.top + ((BA.graphSize.bottom - BA.graphSize.top) * BA.cursorsRelative.y1)
        });

        $('#cur_y1, #cur_y1_info').offset({
            top: $('#cur_y1_arrow').offset().top + $('#cur_y1_arrow').height() / 2
        });
        $('#cur_y1_info').offset({ left: BA.graphSize.left + $('#cur_y1_info').outerWidth() });
        $('#cur_y1_info').html(new_val.toFixed(2) + unit).data('cleanval', +new_val);
        $('#cur_y1').width(BA.graphSize.right - BA.graphSize.left);


        new_val = (min - max) * BA.cursorsRelative.y2 + max;

        $('#cur_y2_arrow').offset({
            left: BA.graphSize.right - $('#cur_y2_arrow').width() / 2 - 2,
            top: BA.graphSize.top + ((BA.graphSize.bottom - BA.graphSize.top) * BA.cursorsRelative.y2)
        });

        $('#cur_y2, #cur_y2_info').offset({
            top: $('#cur_y2_arrow').offset().top + $('#cur_y2_arrow').height() / 2
        });
        $('#cur_y2_info').offset({ left: BA.graphSize.left + $('#cur_y2_info').outerWidth() });
        $('#cur_y2_info').html(new_val.toFixed(2) + unit).data('cleanval', +new_val);;
        $('#cur_y2').width(BA.graphSize.right - BA.graphSize.left);


        diff_px = Math.abs($('#cur_y1').offset().top - $('#cur_y2').offset().top) - 6;
        if ($('#cur_y1').is(':visible') && $('#cur_y2').is(':visible') && diff_px > 12) {
            var top = Math.min($('#cur_y1').offset().top, $('#cur_y2').offset().top) + 3;
            var value = $('#cur_y1_info').data('cleanval') - $('#cur_y2_info').data('cleanval');

            $('#cur_y_diff')
                .offset({ left: BA.graphSize.left + 50, top: top })
                .height(diff_px)
                .show();
            $('#cur_y_diff_info')
                .html(Math.abs(value).toFixed(2) + unit)
                .offset({ left: $('#cur_y_diff').offset().left, top: top + diff_px / 2 - 2 })
                .show();
        } else {
            $('#cur_y_diff, #cur_y_diff_info').hide();
        }
    }


    // Z-cursors
    BA.updateZLinesAndArrows = function() {
        var diff_px = 0;
        let unit = ' deg';
        let min = parseFloat($('#BA_SIGNAL_2_MIN').val());
        let max = parseFloat($('#BA_SIGNAL_2_MAX').val());
        let new_val = 0;


        new_val = (min - max) * BA.cursorsRelative.z1 + max;

        $('#cur_z1_arrow').offset({
            left: BA.graphSize.left + $('#cur_z1_arrow').width() / 2 - 6,
            top: BA.graphSize.top + ((BA.graphSize.bottom - BA.graphSize.top) * BA.cursorsRelative.z1)
        });

        $('#cur_z1, #cur_z1_info').offset({
            top: $('#cur_z1_arrow').offset().top + $('#cur_z1_arrow').height() / 2
        });
        $('#cur_z1_info').offset({ left: BA.graphSize.right - $('#cur_z1_info').outerWidth() - 22 });
        $('#cur_z1_info').html(new_val.toFixed(2) + unit).data('cleanval', +new_val);
        $('#cur_z1').width(BA.graphSize.right - BA.graphSize.left);


        new_val = (min - max) * BA.cursorsRelative.z2 + max;

        $('#cur_z2_arrow').offset({
            left: BA.graphSize.left + $('#cur_z2_arrow').width() / 2 - 6,
            top: BA.graphSize.top + ((BA.graphSize.bottom - BA.graphSize.top) * BA.cursorsRelative.z2)
        });

        $('#cur_z2, #cur_z2_info').offset({
            top: $('#cur_z2_arrow').offset().top + $('#cur_z2_arrow').height() / 2
        });
        $('#cur_z2_info').offset({ left: BA.graphSize.right - $('#cur_z2_info').outerWidth() - 22 });
        $('#cur_z2_info').html(new_val.toFixed(2) + unit).data('cleanval', +new_val);;
        $('#cur_z2').width(BA.graphSize.right - BA.graphSize.left);


        diff_px = Math.abs($('#cur_z1').offset().top - $('#cur_z2').offset().top) - 6;
        if ($('#cur_z1').is(':visible') && $('#cur_z2').is(':visible') && diff_px > 12) {
            var top = Math.min($('#cur_z1').offset().top, $('#cur_z2').offset().top) + 3;
            var value = $('#cur_z1_info').data('cleanval') - $('#cur_z2_info').data('cleanval');

            $('#cur_z_diff')
                .offset({ left: BA.graphSize.right - 50, top: top })
                .height(diff_px)
                .show();
            $('#cur_z_diff_info')
                .html(Math.abs(value).toFixed(2) + unit)
                .offset({ left: $('#cur_z_diff').offset().left, top: top + diff_px / 2 - 2 })
                .show();
        } else {
            $('#cur_z_diff, #cur_z_diff_info').hide();
        }
    }


    BA.updateLinesAndArrows = function() {
        BA.updateXLinesAndArrows();
        BA.updateYLinesAndArrows();
        BA.updateZLinesAndArrows();
    }


    BA.updateCursors = function() {
        if (BA.graphCache === undefined)
            return;
        var plot = BA.graphCache.plot;
        var offset = plot.getPlotOffset();
        var left = offset.left + 1 + 'px';
        var right = offset.right + 1 + 'px';
        var top = offset.top + 1 + 'px';
        var bottom = offset.bottom + 9 + 'px';

        //update lines length
        $('.hline').css('left', left);
        $('.hline').css('right', right);
        $('.vline').css('top', top);
        $('.vline').css('bottom', bottom);

        //update arrows positions
        var diff_left = offset.left + 2 + 'px';
        var diff_top = offset.top - 2 + 'px';
        var margin_left = offset.left - 7 - 2 + 'px';
        var margin_top = -7 + offset.top - 2 + 'px';
        var margin_bottom = -2 + offset.bottom + 'px';
        var line_margin_left = offset.left - 2 + 'px';
        var line_margin_top = offset.top - 2 + 'px';
        var line_margin_bottom = offset.bottom - 2 + 'px';

        $('.varrow').css('margin-left', margin_left);
        $('.harrow').css('margin-top', margin_top);
        $('.harrow').css('margin-bottom', margin_bottom);
        $('.vline').css('margin-left', line_margin_left);
        $('.hline').css('margin-top', line_margin_top);
        $('.hline').css('margin-bottom', line_margin_bottom);

        $('#cur_x_diff').css('margin-left', diff_left);
        $('#cur_y_diff').css('margin-top', diff_top);
        $('#cur_z_diff').css('margin-top', diff_top);
        $('#cur_x_diff_info').css('margin-left', diff_left);
        $('#cur_y_diff_info').css('margin-top', diff_top);
        $('#cur_z_diff_info').css('margin-top', diff_top);
    };

    BA.modelProcess = function(value) {
        if (value["RP_MODEL_STR"].value === "Z20_250_12") {
            $("#CALIB_BODE_IMG").attr("src", "img/bode_calib_250.png");
        }
        if (value["RP_MODEL_STR"].value === "Z20_250_12_120") {
            $("#CALIB_BODE_IMG").attr("src", "img/bode_calib_250.png");
        }
    }


    BA.param_callbacks["BA_MEASURE_START"] = BA.process_run;
    BA.param_callbacks["BA_CALIBRATE_START"] = BA.calib_stop;
    BA.param_callbacks["BA_CURRENT_FREQ"] = BA.change_cur_freq;
    BA.param_callbacks["BA_CURRENT_STEP"] = BA.change_cur_step;
    BA.param_callbacks["BA_CALIBRATE_ENABLE"] = BA.calib_enable;
    BA.param_callbacks["BA_INPUT_THRESHOLD"] = BA.change_input_threshold;
    BA.param_callbacks["RP_MODEL_STR"] = BA.modelProcess;


}(window.BA = window.BA || {}, jQuery));




// Page onload event handler
$(function() {

    var reloaded = $.cookie("ba_forced_reload");
    if (reloaded == undefined || reloaded == "false") {
        $.cookie("ba_forced_reload", "true");
        window.location.reload(true);
    }

    BA.client_id = 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
        var r = Math.random() * 16 | 0,
            v = c == 'x' ? r : (r & 0x3 | 0x8);
        return v.toString(16);
    });

    $.ajax({
        url: '/set_client_id', //Server script to process data
        type: 'POST',
        //Ajax events
        //beforeSend: beforeSendHandler,
        success: function(e) { console.log(e); },
        error: function(e) { console.log(e); },
        // Form data
        data: BA.client_id,
        //Options to tell jQuery not to process data or worry about content-type.
        cache: false,
        contentType: false,
        processData: false
    });

    BA.checkStatus();

    // X cursor arrows dragging
    $('#cur_x1_arrow, #cur_x2_arrow').draggable({
        axis: 'x',
        containment: 'parent',
        start: function(ev, ui) {
            BA.state.line_moving = true;
            BA.state.cursor_dragging = true;
        },
        drag: function(ev, ui) {
            BA.moveX(ui);
        },
        stop: function(ev, ui) {
            BA.moveX(ui);
            BA.state.line_moving = false;
            BA.state.cursor_dragging = false;
        }
    });


    // Y cursor arrows dragging
    $('#cur_y1_arrow, #cur_y2_arrow').draggable({
        axis: 'y',
        containment: 'parent',
        start: function(ev, ui) {
            BA.state.cursor_dragging = true;
        },
        drag: function(ev, ui) {
            BA.moveY(ui);
        },
        stop: function(ev, ui) {
            BA.moveY(ui);
            BA.state.cursor_dragging = false;
        }
    });


    // Z cursor arrows dragging
    $('#cur_z1_arrow, #cur_z2_arrow').draggable({
        axis: 'y',
        containment: 'parent',
        start: function(ev, ui) {
            BA.state.cursor_dragging = true;
        },
        drag: function(ev, ui) {
            BA.moveZ(ui);
        },
        stop: function(ev, ui) {
            BA.moveZ(ui);
            BA.state.cursor_dragging = false;
        }
    });

    // Export
    $('#downl_graph').on('click', function() {
        setTimeout(BA.SaveGraphs, 30);
    });

    $('#downl_csv').on('click', function() {
        BA.downloadDataAsCSV("baData.csv");
    });

    // Process clicks on top menu buttons
    //Run button
    $('#BA_RUN').on('click', function(ev) {

        if ($('#BA_AUTOSCALE_BTN').hasClass('active')) {
            $('#BA_SIGNAL_1_MAX').val(0);
            $('#BA_SIGNAL_1_MIN').val(0);
            $('#BA_SIGNAL_2_MAX').val(0);
            $('#BA_SIGNAL_2_MIN').val(0);

        }

        BA.initPlot(false);
        BA.firstValGot.BA_SIGNAL_1 = false;
        BA.firstValGot.BA_SIGNAL_2 = false;
        delete BA.lastSignals['BA_SIGNAL_1'];
        delete BA.lastSignals['BA_SIGNAL_2'];
        ev.preventDefault();
        //$('#BA_RUN').hide();
        //$('#BA_STOP').css('display', 'block');
        //$('#measuring-status').show();
        BA.parametersCache["BA_START_FREQ"] = { value: $("#BA_START_FREQ").val() };
        BA.parametersCache["BA_END_FREQ"] = { value: $('#BA_END_FREQ').val() };
        BA.parametersCache["BA_STEPS"] = { value: $('#BA_STEPS').val() };
        BA.parametersCache["BA_MEASURE_START"] = { value: true };
        BA.sendParameters();
        //BA.running = true;
        BA.curGraphScale = BA.scale;
    });

    //Stop button
    $('#BA_STOP').on('click', function(ev) {
        ev.preventDefault();
        //$('#BA_STOP').hide();
        //$('#BA_RUN').show();
        //$('#measuring-status').hide();
        BA.parametersCache["BA_MEASURE_START"] = { value: false };
        BA.sendParameters();
        //BA.running = false;
    });

    //Loader wrapper
    $('#loader-wrapper').on('click', function(ev) {
        ev.preventDefault();
        if (BA.calibrating == true) {
            $('body').addClass('loaded');
            $('#calibration').hide();
            BA.parametersCache["BA_CALIBRATE_START"] = { value: 0 };
            BA.parametersCache["BA_MEASURE_START"] = { value: false };
            BA.sendParameters();
            BA.calibrating = false;
        }
    });




    // Open changing parameters dialogs
    $('.edit-mode').on('click', function() {
        if (BA.running)
            return;
        BA.state.editing = true;
        $('#right_menu').hide();
        $('#' + $(this).attr('id') + '_dialog').show();
    });

    // Close changing paramerers dialogs
    $('.close-dialog').on('click', function() {
        BA.state.editing = false;
        $('.dialog:visible').hide();
        $('#right_menu').show();
    });


    //Draw graph
    BA.drawGrid();


    // Bind to the window resize event to redraw the graph; trigger that event to do the first drawing
    $(window).resize(function() {
        var divider = 1.6;
        var window_width = window.innerWidth;
        var window_height = window.innerHeight;

        if (window_width > 768 && window_height > 580) {
            var global_width = window_width - 30,
                global_height = global_width / divider;
            if (window_height < global_height) {
                global_height = window_height - 70 * divider;
                global_width = global_height * divider;
            }

            $('#global_container').css('width', global_width);
            $('#global_container').css('height', global_height);


            BA.drawGrid();
            var main_width = $('#main').outerWidth(true);
            var main_height = $('#main').outerHeight(true);
            $('#global_container').css('width', main_width);
            $('#global_container').css('height', main_height);

            BA.drawGrid();
            main_width = $('#main').outerWidth(true);
            main_height = $('#main').outerHeight(true);
            window_width = window.innerWidth;
            window_height = window.innerHeight;
            if (main_height > (window_height - 80)) {
                $('#global_container').css('height', window_height - 80);
                $('#global_container').css('width', divider * (window_height - 80));
                BA.drawGrid();
                $('#global_container').css('width', $('#main').outerWidth(true) - 2);
                $('#global_container').css('height', $('#main').outerHeight(true) - 2);
                BA.drawGrid();
            }
        }

        $('#global_container').offset({ left: (window_width - $('#global_container').width()) / 2 });

        // Resize the graph holders
        $('.plot').css($('#graph_bode_grid').css(['height', 'width']));

        // Hide all graphs, they will be shown next time signal data is received
        $('#graph_bode .plot').hide();

        if (BA.ws) {
            BA.sendParameters();
        }

        BA.updateGraphSize();

        // Set the resized flag
        BA.state.resized = true;

        if ((BA.lastSignals['BA_SIGNAL_1'] != undefined) || (BA.lastSignals['BA_SIGNAL_2'] != undefined)) {
            BA.initPlot(true);

            setTimeout(function() {
                BA.updateLinesAndArrows();
            }, 120);

            BA.updateCursors();
        }
    }).resize();




    // Stop the application when page is unloaded
    $(window).on('beforeunload', function() {
        BA.ws.onclose = function() {}; // disable onclose handler first
        BA.ws.close();
        $.ajax({
            url: BA.config.stop_app_url,
            async: false
        });
    });




    //Crash buttons
    $('#send_report_btn').on('click', function() { BA.formEmail() });
    $('#restart_app_btn').on('click', function() { location.reload() });

    $('#BA_AUTOSCALE_BTN').click(function() {
        if ($(this).hasClass('active'))
            $(this).removeClass('active');
        else
            $(this).addClass('active');
    });

    $('#BA_SHOWALL_BTN').click(function() {
        if ($(this).hasClass('active'))
            $(this).removeClass('active');
        else
            $(this).addClass('active');
    });

    $('#BA_CURSOR_X1').click(function() {
        var btn = $(this);
        if (!btn.hasClass('active'))
            BA.enableCursor('x1');
        else
            BA.disableCursor('x1');
        BA.updateLinesAndArrows();
    });

    $('#BA_CURSOR_X2').click(function() {
        var btn = $(this);
        if (!btn.hasClass('active'))
            BA.enableCursor('x2');
        else
            BA.disableCursor('x2');
        BA.updateLinesAndArrows();
    });

    $('#BA_CURSOR_Y1').click(function() {
        var btn = $(this);
        if (!btn.hasClass('active'))
            BA.enableCursor('y1');
        else
            BA.disableCursor('y1');
        BA.updateLinesAndArrows();
    });

    $('#BA_CURSOR_Y2').click(function() {
        var btn = $(this);
        if (!btn.hasClass('active'))
            BA.enableCursor('y2');
        else
            BA.disableCursor('y2');
        BA.updateLinesAndArrows();
    });

    $('#BA_CURSOR_Z1').click(function() {
        var btn = $(this);
        if (!btn.hasClass('active'))
            BA.enableCursor('z1');
        else
            BA.disableCursor('z1');
        BA.updateLinesAndArrows();
    });

    $('#BA_CURSOR_Z2').click(function() {
        var btn = $(this);
        if (!btn.hasClass('active'))
            BA.enableCursor('z2');
        else
            BA.disableCursor('z2');
        BA.updateLinesAndArrows();
    });


    // Init help
    Help.init(helpListBA);
    Help.setState("idle");

    // Everything prepared, start application
    BA.startApp();

    // // Start measuring after loading. Must be last in this file!
    // setTimeout(function() {
    //         //$('#BA_RUN').hide();
    //         //$('#BA_STOP').css('display', 'block');
    //         //$('#measuring-status').show();
    //         BA.parametersCache["BA_MEASURE_START"] = { value: true };
    //         BA.sendParameters();
    //         //BA.running = true;
    //     },
    //     1000);
});