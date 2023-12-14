/*
 * Red Pitaya arb manager
 *
 * Author: Danilyuk Nikolay <n.danilyuk@integrasources.eu>
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


(function(SM, $, undefined) {

    // Params cache
    SM.params = {
        orig: {},
        local: {}
    };
    SM.ss_status_last = -1;
    SM.ss_rate = -1;
    SM.ss_max_rate = -1;
    SM.ss_max_rate_devider = -1;
    SM.param_callbacks = {};
    SM.parameterStack = [];
    SM.signalStack = [];
    // Parameters cache
    SM.parametersCache = {};

    // App configuration
    SM.config = {};
    SM.config.app_id = 'arb_manager';
    SM.config.server_ip = ''; // Leave empty on production, it is used for testing only

    SM.config.start_app_url = window.location.origin + '/bazaar?start=' + SM.config.app_id;
    SM.config.stop_app_url = window.location.origin + '/bazaar?stop=' + SM.config.app_id;
    SM.config.socket_url = 'ws://' + window.location.host + '/wss';

    SM.charts = [];

    SM.status = {
        NONE: 0,
        REQ_UPDATE: 1,
        FILE_ERR: 2,
        FILE_ERR_TO_LONG: 3,
        FILE_ERR_PARS_ERR: 4
    };

    // App state
    SM.state = {
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

    SM.reconnect = function() {
        setTimeout(() => {
            SM.startApp();
        }, 1000);
    }
    SM.startApp = function() {
        $.get(
                SM.config.start_app_url
            )
            .done(function(dresult) {
                if (dresult.status == 'OK') {
                    try {
                        SM.connectWebSocket();
                        console.log("Load manager");
                    } catch (e) {
                        SM.reconnect();
                    }
                } else if (dresult.status == 'ERROR') {
                    console.log(dresult.reason ? dresult.reason : 'Could not start the application (ERR1)');
                    SM.reconnect();
                } else {
                    console.log('Could not start the application (ERR2)');
                    SM.reconnect();
                }
            })
            .fail(function() {
                console.log('Could not start the application (ERR3)');
                SM.reconnect();
            });
    };



    //Write email
    SM.formEmail = function() {
        //var file = new FileReader();
        var mail = "support@redpitaya.com";
        var subject = "Crash report Red Pitaya OS";
        var body = "%0D%0A%0D%0A------------------------------------%0D%0A" + "DEBUG INFO, DO NOT EDIT!%0D%0A" + "------------------------------------%0D%0A%0D%0A";
        body += "Parameters:" + "%0D%0A" + JSON.stringify({ parameters: SM.parametersCache }) + "%0D%0A";
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


    // Creates a WebSocket connection with the web server
    SM.connectWebSocket = function() {

        if (window.WebSocket) {
            SM.ws = new WebSocket(SM.config.socket_url);
            SM.ws.binaryType = "arraybuffer";
        } else if (window.MozWebSocket) {
            SM.ws = new MozWebSocket(SM.config.socket_url);
            SM.ws.binaryType = "arraybuffer";
        } else {
            console.log('Browser does not support WebSocket');
        }

        // Define WebSocket event listeners
        if (SM.ws) {
            SM.ws.onopen = function() {
                console.log('Socket opened');
                var element = document.getElementById("loader-wrapper");
                if (element !== null){
                    element.parentNode.removeChild(element);
                }
                $('#main').removeAttr("style");
                SM.state.socket_opened = true;
                SM.requestAllParameters();
                SM.unexpectedClose = true;
            };

            SM.ws.onclose = function() {
                SM.state.socket_opened = false;
                console.log('Socket closed');
                SM.reconnect();
            };

            SM.ws.onerror = function(ev) {
                if (!SM.state.socket_opened)
                    SM.reconnect();
                console.log('Websocket error: ', ev);
            };

            SM.ws.onmessage = function(ev) {
                try {
                    var data = new Uint8Array(ev.data);
                    //   BA.compressed_data += data.length;
                    var inflate = pako.inflate(data);
                    var text = String.fromCharCode.apply(null, new Uint8Array(inflate));

                    // BA.decompressed_data += text.length;
                    var receive = JSON.parse(text);
                    //Recieving parameters
                    if (receive.parameters) {
                        SM.parameterStack.push(receive.parameters);
                    }

                    if (receive.signals) {
                        SM.signalStack.push(receive.signals);
                    }

                } catch (e) {
                    //BA.state.processing = false;
                    console.log(e);
                } finally {
                    //BA.state.processing = false;
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


    // Sends to server parameters
    SM.sendParameters = function() {
        if (!SM.state.socket_opened) {
            console.log('ERROR: Cannot save changes, socket not opened');
            return false;
        }

        SM.ws.send(JSON.stringify({ parameters: SM.parametersCache }));
        console.log("Send ", SM.parametersCache)
        SM.parametersCache = {};
        return true;
    };

    SM.requestAllParameters = function() {
        if (!SM.state.socket_opened) {
            console.log('ERROR: Cannot save changes, socket not opened');
            return false;
        }
        SM.parametersCache = {};
        SM.parametersCache["in_command"] = { value: "send_all_params" };
        SM.ws.send(JSON.stringify({ parameters: SM.parametersCache }));
        console.log("Send ", SM.parametersCache)
        SM.parametersCache = {};
        return true;
    };


    SM.change_status = function(new_params) {
        console.log(new_params)
    }

    //Handlers
    var signalsHandler = function() {
        if (SM.signalStack.length > 0) {
            SM.signalStack.splice(0, 1);
        }
        if (SM.signalStack.length > 2)
            SM.signalStack.length = [];
    }


    SM.processParameters = function(new_params) {

        if (Object.keys(new_params).length !== 0)
            console.log(new_params)

        for (var param_name in new_params) {
            SM.params.orig[param_name] = new_params[param_name];
            if (SM.param_callbacks[param_name] !== undefined)
                SM.param_callbacks[param_name](new_params);
        }

    };

    var parametersHandler = function() {
        if (SM.parameterStack.length > 0) {
            SM.processParameters(SM.parameterStack[0]);
            SM.parameterStack.splice(0, 1);
        }
    }

    SM.calcSize = function(x) {
        if (x  < 1024) {
            return x + " b"
        }
        if (x  < 1024 * 1024) {
            return (x / 1024).toFixed(3)  + " kb"
        }
        return (x / (1024 * 1024)).toFixed(3)  + " Mb"
    }

    SM.initPlot = function(item,values) {

        var elem = $('<div " class="plot" style="width:100%;height:100%;position: absolute;margin-top: auto;left: 0px;"/>');

        elem.appendTo(item)

        var t = null;
        var options = {
            series: {
                shadowSize: 0,
                lineWidth: 1,
                lines: {
                    lineWidth: 1
                }
            },
            yaxes: [{
                show: false,
                min: -1,
                max: 1,
                labelWidth: 5,
                tickDecimals: 1,
                //   alignTicksWithAxis: 1,
                position: "left"
            }],
            xaxis: {
                show: false,
                color: '#aaaaaa',
                tickColor: '#aaaaaa',
                ticks: t,
                // transform: function(v) {
                //     if (BA.scale)
                //         return Math.log(v + 0.0001); // move away from zero
                //     else
                //         return v;

                // },
                tickDecimals: 0,
                reserveSpace: false,
                // tickFormatter: funcxTickFormat,
                min: null,
                max: null,
            },
            grid: {
                show: true,
                color: '#aaaaaa',
                borderColor: '#6a6a6a',
                tickColor: '#aaaaaa',
                tickColor: '#aaaaaa',
                markingsColor: '#aaaaaa'
            },
            legend: {
                show: false,
                position: "sw",
                backgroundOpacity: 0.15
            }
        };

        var data_points = [{ data: values, color: '#C2BC14', label: "AVG" }];
        var plot = $.plot(elem, data_points, options);
        elem.show();
        plot.resize();
        plot.setupGrid();
        plot.draw()
        SM.charts.push(plot)
        $('.flot-text').css('color', '#aaaaaa');
    }

    SM.redrawCharts = function(ch) {
        for (const item of SM.charts) {
            item.resize()
            item.setupGrid()
            item.draw()
        }
    }

    SM.refreshFiles = function(param) {
        $("#files_table").empty();
        var value = param["RP_FILES_LIST"].value;

        const splitLines = value => value.split(/\r?\n/);
        var files = {}
        splitLines(value).forEach(function(item){
            var cols = item.trim().split('\t');
            //cols = cols.filter(item => item.trim().length > 0);
            if (cols.length >= 2){
                var fname = cols[0].toString();
                var name = cols.length  > 1 ? cols[1].toString() : "";
                var size = cols.length  > 2 ? parseInt(cols[2]) : 0;
                var values = cols.length  > 3 ? cols[3].toString() : "";

                if (files[fname] == undefined){
                    files[fname] = {}
                }
                files[fname]["name"] = name;
                files[fname]["size"] = size;

                var sigVal = values.trim().split(';');
                var sigValF = [];
                var x_axis = 0;
                for (const e of sigVal) {
                    sigValF.push([x_axis++,parseFloat(e)])
                }
                files[fname]["values"] = sigValF;
            }
        });
        console.log(files)
        SM.charts = []

        for (const [key, value] of Object.entries(files)) {

            var new_row = document.createElement('div');
            new_row.className = "filecell"
            var sub_row = document.createElement('div');
            new_row.append(sub_row)
            var span1 = document.createElement('span1');
            sub_row.append(span1)
            var span2 = document.createElement('span2');
            sub_row.append(span2)

            var item5 = document.createElement('item5');
            span1.append(item5)
            var label_name = document.createElement('label');
            item5.append(label_name)
            label_name.innerText = 'Signal name: '
            var input_item = document.createElement('input');
            item5.append(input_item)
            input_item.setAttribute('maxlength', '10');
            input_item.value = value["name"]
            input_item.setAttribute("fileName",key)

            var item4 = document.createElement('item4');
            span1.append(item4)
            var li = document.createElement('li');
            li.className = "run_buttons2"
            var a = document.createElement('a');
            a.innerText = '\u274C';
            li.append(a)
            item4.append(li)
            var plot = document.createElement('plot');
            plot.className = "graphs";

            span2.append(plot)

            var can = document.createElement('canvas');
            plot.append(can)
            can.className = "graph_grid";

            document.getElementById('files_table').appendChild(new_row);
            SM.initPlot(plot,value["values"])
            li.setAttribute("fileName",key)
            li.addEventListener('click', function() {
                var addr = '/delete_arb_file';
                $.ajax({
                    url: addr + '?file=' + $(this).attr('fileName'),
                    type: 'GET'
                }).done(function(msg) {
                    console.log("File removed ",$(this).attr('fileName'))
                    SM.parametersCache["RP_FILES_LIST"] = { value: "req" };
                    SM.sendParameters();
                });

            }, false);

            input_item.addEventListener("keypress", function(event) {
                // If the user presses the "Enter" key on the keyboard
                if (event.key === "Enter") {
                    SM.parametersCache["RP_RENAME_FILE"] = { value: $(this).attr('fileName') + "\n" + $(this).val()};
                    SM.sendParameters();
                }
            });

        }

    }



    SM.check_file = function(param) {
        var value = param["RP_REQ_CHECK_FILE"].value;
        console.log(value)
    }

    SM.req_status = function(param) {
        var value = param["RP_REQ_STATUS"].value;
        if (SM.status.REQ_UPDATE === value){
            SM.parametersCache["RP_FILES_LIST"] = { value: "req" };
            SM.sendParameters();
        }
    }

    //Set handlers timers
    setInterval(signalsHandler, 50);
    setInterval(parametersHandler, 50);

    SM.param_callbacks["RP_REQ_STATUS"] = SM.req_status;
    SM.param_callbacks["RP_REQ_CHECK_FILE"] = SM.check_file;
    SM.param_callbacks["RP_FILES_LIST"] = SM.refreshFiles;


}(window.SM = window.SM || {}, jQuery));




// Page onload event handler
$(function() {

    // Bind to the window resize event to redraw the graph; trigger that event to do the first drawing
    $(window).resize(function() {
        if (SM.ws) {
            SM.sendParameters();
        }
        SM.redrawCharts()
    }).resize();


    // Stop the application when page is unloaded
    $(window).on('beforeunload', function(event) {
        var target = document.activeElement.href
        console.log(document.activeElement.href)
        if (!target.includes("/arb_mananger/")){
            SM.ws.onclose = function() {}; // disable onclose handler first
            SM.ws.close();
            $.get(
                SM.config.stop_app_url
            )
        }
    });



    // Everything prepared, start application
    SM.startApp();

});