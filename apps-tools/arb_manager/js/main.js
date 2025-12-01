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

    SM.ss_max_gain = undefined;
    SM.param_callbacks = {};
    SM.parameterStack = [];
    SM.signalStack = [];

    SM.charts = [];
    SM.renamePrev = "";
    SM.renameSend = false;
    SM.previousPageUrl = undefined;
    SM.files = {};

    SM.status = {
        NONE: 0,
        REQ_UPDATE: 1,
        FILE_ERR: 2,
        FILE_ERR_TO_LONG: 3,
        FILE_ERR_PARS_ERR: 4,
        FILE_ERR_CANT_RENAME: 5,
        FILE_RENAME_DONE: 6,
        FILE_ERR_CANT_CHANGE_COLOR: 7,
        FILE_CHANGE_COLOR_DONE: 8,
        FILE_ERR_PARS_COE_ERR: 9,
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


    SM.calcSize = function(x) {
        if (x  < 1024) {
            return x + " B"
        }
        if (x  < 1024 * 1024) {
            return (x / 1024).toFixed(3)  + " kB"
        }
        return (x / (1024 * 1024)).toFixed(3)  + " MB"
    }

    SM.initPlot = function(item,values,color) {

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
                min: SM.ss_max_gain * -1,
                max: SM.ss_max_gain,
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

        var data_points = [{ data: values, color: color, label: "AVG" }];
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
    SM.uintToColor = function(uint) {
        const red = (uint >> 16) & 255;
        const green = (uint >> 8) & 255;
        const blue = uint & 255;

        // Convert RGB components to hex format
        const rHex = red.toString(16).padStart(2, '0');
        const gHex = green.toString(16).padStart(2, '0');
        const bHex = blue.toString(16).padStart(2, '0');

        // Create the hex color string
        const hexColor = "#" + rHex + gHex + bHex;

        return hexColor;
    }

    SM.hexToUintColor = function(hexColor) {
        // Remove the "#" symbol if present
        hexColor = hexColor.replace("#", "");

        // Parse the hex color components (red, green, blue)
        const red = parseInt(hexColor.substr(0, 2), 16);
        const green = parseInt(hexColor.substr(2, 2), 16);
        const blue = parseInt(hexColor.substr(4, 2), 16);

        // Calculate the Uint32 value
        const uintValue = (red << 16) + (green << 8) + blue;

        return uintValue;
    }

    SM.refreshFiles = function(param) {
        $("#files_table").empty();
        var value = param["RP_FILES_LIST"].value;

        const splitLines = value => value.split(/\r?\n/);
        SM.files = {};
        splitLines(value).forEach(function(item){
            var cols = item.trim().split('\t');
            //cols = cols.filter(item => item.trim().length > 0);
            if (cols.length >= 2){
                var fname = cols[0].toString();
                var name = cols.length  > 1 ? cols[1].toString() : "";
                var is_valid = cols.length  > 2 ? parseInt(cols[2]) : 0;
                var color = cols.length  > 3 ? parseInt(cols[3]) : 0;
                var size = cols.length  > 4 ? parseInt(cols[4]) : 0;
                var values = cols.length  > 5 ? cols[5].toString() : "";

                if (SM.files[fname] == undefined){
                    SM.files[fname] = {}
                }
                SM.files[fname]["name"] = name;
                SM.files[fname]["size"] = size;
                SM.files[fname]["color"] = SM.uintToColor(color);
                SM.files[fname]["is_valid"] = is_valid;
                var plot = document.createElement('plot');
                plot.className = "graphs";
                SM.files[fname]["plot"] = plot;

                var sigVal = values.trim().split(';');
                var sigValF = [];
                var x_axis = 0;
                for (const e of sigVal) {
                    sigValF.push([x_axis++,parseFloat(e)])
                }
                SM.files[fname]["values"] = sigValF;
            }
        });

        SM.charts = []

        for (const [key, value] of Object.entries(SM.files)) {

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

            var item_color = document.createElement('item_color');

            var input_color = document.createElement('input');
            input_color.setAttribute("fileName",key)
            input_color.setAttribute('class', 'selected-color');
            input_color.setAttribute('type', 'color');
            input_color.setAttribute('value', value["color"]);
            input_color.addEventListener("input", function() {
                console.log($(this).attr('fileName'), $(this).val())
                SM.renameSend = true;
                fname = $(this).attr('fileName')
                CLIENT.parametersCache["RP_CHANGE_COLOR"] = { value: fname + "\n" + SM.hexToUintColor($(this).val())};
                CLIENT.sendParameters();
                SM.files[fname]["color"] = $(this).val()
                SM.initPlot(SM.files[fname]["plot"],SM.files[fname]["values"],$(this).val())
            });

            var label_name = document.createElement('label');
            label_name.innerText = 'Signal name: '
            var input_item = document.createElement('input');
            input_item.setAttribute('maxlength', '10');
            input_item.value = value["name"]
            input_item.setAttribute("fileName",key)

            item5.append(label_name)
            item5.append(input_item)
            item5.append(item_color)
            item_color.append(input_color)

            if (value["is_valid"] !== 1){
                var label_error = document.createElement('label');
                item5.append(label_error)
                label_error.innerText = 'Out of range'
                label_error.style.color= '#E00000';
                label_error.style.marginLeft='10px'
            }

            var item4 = document.createElement('item4');
            span1.append(item4)
            var li = document.createElement('li');
            li.className = "run_buttons2"
            var a = document.createElement('a');
            a.innerText = '\u274C';
            li.append(a)
            item4.append(li)


            span2.append(value["plot"])

            var can = document.createElement('canvas');
            value["plot"].append(can)
            can.className = "graph_grid";

            document.getElementById('files_table').appendChild(new_row);
            SM.initPlot(value["plot"],value["values"],value['color'])
            li.setAttribute("fileName",key)
            li.addEventListener('click', function() {
                var addr = '/delete_arb_file';
                $.ajax({
                    url: addr + '?file=' + $(this).attr('fileName'),
                    type: 'GET'
                }).done(function(msg) {
                    console.log("File removed ",msg)
                    setTimeout(() => {
                        CLIENT.parametersCache["RP_FILES_LIST"] = { value: "req" };
                        CLIENT.sendParameters();
                    }, 1000);
                });

            }, false);

            input_item.addEventListener("keypress", function(event) {
                // If the user presses the "Enter" key on the keyboard
                if (event.key === "Enter") {
                    SM.renameSend = true;
                    CLIENT.parametersCache["RP_RENAME_FILE"] = { value: $(this).attr('fileName') + "\n" + $(this).val()};
                    CLIENT.sendParameters();
                }
            });

            input_item.addEventListener("focusin",  function(event) {
                SM.renamePrev = $(this).val();
                SM.renameSend = false;
            });

            input_item.addEventListener("focusout", function(event) {
                if (SM.renameSend &&  SM.renamePrev  !== ""){
                    $(this).val( SM.renamePrev )
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
            CLIENT.parametersCache["RP_FILES_LIST"] = { value: "req" };
            CLIENT.sendParameters();
        }

        if (SM.status.FILE_ERR === value){
            $('#info_dialog_label').text("Error upload file");
            $('#info_dialog').modal('show');
        }

        if (SM.status.FILE_ERR_TO_LONG === value){
            $('#info_dialog_label').text("There is a lot of data in the file.\nThere must be no more than 16384 samples");
            $('#info_dialog').modal('show');
        }

        if (SM.status.FILE_ERR_PARS_ERR === value){
            $('#info_dialog_label').text("Error parsing file. The data must be in one column in the format ±X.XXX");
            $('#info_dialog').modal('show');
        }

        if (SM.status.FILE_ERR_PARS_COE_ERR === value){
            $('#info_dialog_label').text("Error parsing file. Data must be in BIN or DEC or HEX format and no larger than the ADC bit depth");
            $('#info_dialog').modal('show');
        }

        if (SM.status.FILE_ERR_CANT_CHANGE_COLOR === value){
            $('#info_dialog_label').text("Error change color");
            $('#info_dialog').modal('show');
        }

        if (SM.status.FILE_ERR_CANT_RENAME === value){
            $(":focus").fI()
        }

        if (SM.status.FILE_RENAME_DONE === value){
            SM.renamePrev = "";
            SM.renameSend = false;
        }

    }

    SM.param_callbacks["RP_REQ_STATUS"] = SM.req_status;
    SM.param_callbacks["RP_REQ_CHECK_FILE"] = SM.check_file;
    SM.param_callbacks["RP_FILES_LIST"] = SM.refreshFiles;


}(window.SM = window.SM || {}, jQuery));




// Page onload event handler
$(function() {

    // Bind to the window resize event to redraw the graph; trigger that event to do the first drawing
    $(window).resize(function() {
        if (CLIENT.ws) {
            CLIENT.sendParameters();
        }
        SM.redrawCharts()
    }).resize();




    SM.previousPageUrl = document.referrer;
    console.log(`Previously visited page URL: ${SM.previousPageUrl}`);
    const currentUrl = window.location.href;
    if (currentUrl === SM.previousPageUrl || SM.previousPageUrl === ''){
        SM.previousPageUrl = '/'
    }
    $("#back_button").attr("href", SM.previousPageUrl)

});