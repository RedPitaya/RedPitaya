/*
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


(function(LA, $, undefined) {

    LA.mouseWheelEventFired = false;
    LA.move_mode = undefined;

    LA.promptFile = function(contentType, multiple) {
        var input = document.createElement("input");
        input.type = "file";
        input.accept = '.bin';
        return new Promise(function(resolve) {
            document.activeElement.onfocus = function() {
            document.activeElement.onfocus = null;
                setTimeout(resolve, 500);
            };
            input.onchange = function() {
                var files = Array.from(input.files);
                resolve(files[0]);
            };
            input.click();
        });
    }


    LA.initHandlers = function() {

        $('.trigger_type').change(function(e) {
            LA.sendTriggerFromUI(this)
        });

        $('input[type=text]:not([readonly]):not(.no-arrows)[step]').iLightInputNumber({
            mobile: false
        });

        $('#save_settings').click(function() {
            $('#save_settings_dialog').modal("show");
        });

        $('#reset_settings').click(function() {
            CLIENT.parametersCache["CONTROL_CONFIG_SETTINGS"] = { value: 1 };
            CLIENT.sendParameters()
        });

        $('#LA_REQ_SAVE_SETTINGS').on('click', function() {
            var name = $("#SETTINGS_NEW_NAME").val().trim()
            if (name !== ""){
                CLIENT.parametersCache['FILE_SATTINGS'] = { value: name };
                CLIENT.parametersCache['CONTROL_CONFIG_SETTINGS'] = { value: 4 }; // SAVE
                CLIENT.sendParameters()
            }
        });

        $('.enable-ch').click(LA.enableChannelDIN);

        $('.enable-ch-bus').click(LA.enableChannelBUS);

        $('.ch-name-inp').change(function() {
            var arr = ["CH1_NAME", "CH2_NAME", "CH3_NAME", "CH4_NAME", "CH5_NAME", "CH6_NAME", "CH7_NAME", "CH8_NAME"];
            var element = $(this);
            var ch = arr.indexOf(element.attr('id'));
            if (ch != -1) {
                CLIENT.parametersCache["LA_DIN_NAME_"+(ch+1)] = { value: element.val() };
                CLIENT.sendParameters()
            }
        })

        $('#LA_CURSOR_X1').click(function() {
            var btn = $(this);
            if (!btn.hasClass('active'))
                LA.enableCursor('1');
            else
            LA.disableCursor('1');
        });

        $('#LA_CURSOR_X2').click(function() {
            var btn = $(this);
            if (!btn.hasClass('active'))
                LA.enableCursor('2');
            else
            LA.disableCursor('2');
        });

        $('#select_mode').click(function() {
            var curVal = CLIENT.getValue("LA_MEASURE_MODE")
            if (curVal !== undefined){
                var x = curVal === 1 ? 2 : 1
                CLIENT.parametersCache["LA_MEASURE_MODE"] = { value: x };
                CLIENT.sendParameters()
            }
        });

        $("#graphs").mousewheel(function(event) {
            if (LA.mouseWheelEventFired)
                return;
            LA.changeXZoom(event.deltaY > 0 ? '+' : '-');
            LA.mouseWheelEventFired = true;
            setTimeout(function() { LA.mouseWheelEventFired = false; }, 300);
        });

        $(document).on('mousedown', '.plot', function(ev) {
            ev.preventDefault();
            // ev.stopPropagation();
            if (!LA.move_mode) {
                var rect = LA.getPoltRect()
                var newPos = LA.boundCursor(rect,{ x: ev.clientX, y: ev.clientY })
                LA.move_mode = newPos;
            }
        });

        $(document).on('mousemove', '.plot', function(ev) {
            ev.preventDefault();
            // ev.stopPropagation();
            if (!LA.move_mode) {
                return;
            }
            var plot = LA.getPlot()
            if (plot == undefined) return
            var rect = LA.getPoltRect()
            var newPos = LA.boundCursor(rect,{ x: ev.clientX, y: ev.clientY })

            var x = LA.move_mode.x - newPos.x;
            // var y = LA.move_mode.y - newPos.y;
            var options = plot.getOptions();
            var range_x   = options.xaxes[0].max - options.xaxes[0].min;
            // var range_y   = options.yaxes[0].max - options.yaxes[0].min;
            LA.move_mode  = newPos;
            var offset = x * range_x / $(this).width();
            var max = options.xaxes[0].max + offset;
            var min = options.xaxes[0].min + offset;
            var samples =  CLIENT.getValue('LA_TOTAL_SAMPLES')
            if (samples !== undefined && samples !== 0){
                LA.region_samples.start = min
                LA.region_samples.end = max
                CLIENT.params.orig['LA_VIEW_PORT_POS'] = {value: ((max - min) / 2 + min) / samples}
                LA.updatePositionBufferViewportOnly()
                LA.updateMainView()
                LA.updateXInfo()
            }
        });

        $(document).on('mouseup', '.plot', function(ev) {
            ev.preventDefault();
            ev.stopPropagation();
            LA.move_mode = undefined;
            CLIENT.parametersCache['LA_VIEW_PORT_POS'] = {value: CLIENT.getValue('LA_VIEW_PORT_POS')}
            CLIENT.sendParameters()
        });

        $(document).on('mouseup', function(ev) {
            ev.preventDefault();
            ev.stopPropagation();
            LA.move_mode = undefined;
            CLIENT.parametersCache['LA_VIEW_PORT_POS'] = {value: CLIENT.getValue('LA_VIEW_PORT_POS')}
            CLIENT.sendParameters()
        });


        $('#cur_x1_arrow, #cur_x2_arrow').draggable({
            axis: 'x',
            containment: 'parent',
            start: function(ev, ui) {
                LA.state.cursor_dragging = true;
            },
            drag: function(ev, ui) {
                LA.updateXCursorElems(ui, false);
            },
            stop: function(ev, ui) {
                LA.updateXCursorElems(ui, true);
                LA.state.cursor_dragging = false;
            }
        });

        $('.y-offset-arrow').dblclick(function(event) {
            event.stopPropagation();
            var index = event.currentTarget.getAttribute("index")
            CLIENT.parametersCache["LA_DIN_" + index + "_POS"] = { value: index };
            CLIENT.sendParameters()
        });

        // Voltage offset arrow dragging
        $('.y-offset-arrow').draggable({
            axis: 'y',
            containment: 'parent',
            start: function(ev, ui) {
            },
            drag: function(ev, ui) {
                var margin_top = Math.abs(parseInt(ui.helper.css('marginTop')));
                var min_top = ((ui.helper.height() / 2) + margin_top) * -1;
                var max_top = $('#graphs').height() - margin_top;

                if (ui.position.top < min_top) {
                    ui.position.top = min_top;
                } else if (ui.position.top > max_top) {
                    ui.position.top = max_top;
                }

                LA.updateYOffset(ui, false);
            },
            stop: function(ev, ui) {
                if (!LA.state.simulated_drag) {
                    LA.updateYOffset(ui, true);
                    $('#info_box').empty();
                }
            }
        });

        $('#buffer_time_region').draggable({
            axis: 'x',
            containment: 'parent',
            start: function(ev, ui) {
                LA.region_view_moving = true
            },
            drag: function(ev, ui) {
                LA.region_view_moving = true
                var w =$('#buffer_time_region').width()
                var fw = $('#graphs_buffer').width()
                var newPos = (ui.position.left + w/2.0) / fw
                CLIENT.params.orig['LA_VIEW_PORT_POS'] = {value: newPos}
                LA.updatePositionBufferViewport()
            },
            stop: function(ev, ui) {
                LA.region_view_moving = false
                CLIENT.parametersCache['LA_VIEW_PORT_POS'] = {value: CLIENT.getValue('LA_VIEW_PORT_POS')}
                CLIENT.sendParameters()
            }
        });

        $('#buffer').on('click', function(ev) {
            var fw = $('#graphs_buffer').width()
            var left = ev.currentTarget.getBoundingClientRect().left
            CLIENT.parametersCache['LA_VIEW_PORT_POS'] = {value: (ev.clientX - left) / fw}
            CLIENT.sendParameters()
            ev.preventDefault();
            ev.stopPropagation();
        });

            // Joystick events
        $('#jtk_up').on('mousedown touchstart', function() {
            $('#jtk_btns').attr('src', 'img/navigation_up.png');
        });
        $('#jtk_left').on('mousedown touchstart', function() {
            $('#jtk_btns').attr('src', 'img/navigation_left.png');
        });
        $('#jtk_right').on('mousedown touchstart', function() {
            $('#jtk_btns').attr('src', 'img/navigation_right.png');
        });
        $('#jtk_down').on('mousedown touchstart', function() {
            $('#jtk_btns').attr('src', 'img/navigation_down.png');
        });

        //  $('#jtk_fine').on('click touchstart', function(ev){
        $('#jtk_fine').on('click', function(ev) {
            var img = $('#jtk_fine');

            if (img.attr('src') == 'img/fine.png') {
                img.attr('src', 'img/fine_active.png');
                LA.state.fine = true;
            } else {
                img.attr('src', 'img/fine.png');
                LA.state.fine = false;
            }

            ev.preventDefault();
            ev.stopPropagation();
        });

        $(document).on('mouseup touchend', function() {
            $('#jtk_btns').attr('src', 'img/navigation.png');
        });

        $('#jtk_up, #jtk_down').on('click', function(ev) {
            ev.preventDefault();
            ev.stopPropagation();
            LA.time_zoom(ev.target.id == 'jtk_down' ? '-' : '+', 0, false)
            LA.guiHandler();
        });

        $('#jtk_left, #jtk_right').on('click', function(ev) {
            ev.preventDefault();
            ev.stopPropagation();
            LA.moveViewPort(ev.target.id == 'jtk_left' ? '-' : '+');
        });

            // Process clicks on top menu buttons
        $('#LA_RUN').on('click', function(ev) {
            ev.preventDefault();
            CLIENT.parametersCache['LA_RUN'] = { value: 1 };
            CLIENT.sendParameters();
        });

        $('#LA_STOP').on('click', function(ev) {
            ev.preventDefault();
            CLIENT.parametersCache['LA_RUN'] = { value: 0 };
            CLIENT.sendParameters();
        });

        $(window).resize(function() {
            var window_height = window.innerHeight;
            $('#main_block').css('height', window_height - 200);

            LA.initCursors()
            LA.resizePlots()
            LA.drawGraphGrid()
            LA.updateCursors()
            LA.updateChannels()
            LA.checkSubWindowPosition()
            LA.setCurrentFreq()

        }).resize();

        $("#ext_con_but").click(function(event) {
            $('#ext_connections_dialog').modal("show");
        });

        $('#graphs').dblclick(function(event) {
            event.stopPropagation();
            var s = CLIENT.getValue('LA_TOTAL_SAMPLES')
            var pre = CLIENT.getValue('LA_PRE_TRIGGER_SAMPLES')
            if (s !== undefined && pre !== undefined){
                CLIENT.parametersCache['LA_VIEW_PORT_POS'] = {value: pre / s}
                CLIENT.sendParameters()
            }
        });

        $('#sys_info').click(function() {
            var elem = $(this);
            if (elem.text() == 'SYS INFO') {
                elem.html('&check; SYS INFO');
                $('#sys_info_view').show();
            } else {
                elem.text('SYS INFO');
                $('#sys_info_view').hide();
            }
        });

        $('#apply_decoder').click(LA.applyDecoder);

        $('#DISPLAY_RADIX').change(function() {
            CLIENT.parametersCache['LA_DISPLAY_RADIX'] = {value : $(this).val()}
            CLIENT.sendParameters()
            LA.guiHandler();
        });

        $('#LOGGER_RADIX').change(function() {
            CLIENT.parametersCache['LA_LOGGER_RADIX'] = {value : $(this).val()}
            CLIENT.sendParameters()
        });

        $('#protocol_selector').change(function() {
            LA.loadBUSSettingsFromConfig()
        });

        $('.bus-settings-btn').click(function() {
            LA.startEditBus($(this).attr('id'));
        });

        $('#downl_graph').on('click', function() {
            setTimeout(COMMON.saveGraphs, 30);
        });

        // Close parameters dialog after Enter key is pressed
        $('input').keyup(function(event) {
            if (event.keyCode == 13) {
                LA.exitEditing(true);
            }
        });

        // Close parameters dialog on close button click
        $('.close-dialog').on('click', function() {
            LA.exitEditing();
        });

        $(".data-bus").click(function() {
            var arr = ["DATA_BUS0", "DATA_BUS1", "DATA_BUS2", "DATA_BUS3"];
            var bus = arr.indexOf($(this).attr('id'));
            CLIENT.parametersCache['LA_LOGGER_BUS_' + (bus +1)] = {value:!$(this).hasClass('active')}
            CLIENT.sendParameters()
        });


        $('#downl_lines').on('click', function() {
            COMMON.downloadDataAsCSV("laDataLines.csv");
        });

        $('#downl_data').on('click', function() {
            COMMON.downloadDecodedDataAsCSV("laDataProtocols.csv");
        });

        $('#upload_rle').click(function() {
            LA.promptFile().then(function(file) {
                if(file){
                    const fileReader = new FileReader(); // initialize the object
                    fileReader.readAsArrayBuffer(file); // read file as array buffer
                    fileReader.onload = (event) => {
                        console.log('Complete File read successfully!')
                        $.ajax({
                            url: '/la_pro_upload_rle_file', //Server script to process data
                            type: 'POST',
                            //Ajax events
                            //beforeSend: beforeSendHandler,
                            success: function(e) {
                                console.log("Upload done " + e);
                                setTimeout(() => {
                                    CLIENT.parametersCache["LA_RUN"] = { value: 3 };
                                    CLIENT.sendParameters();
                                }, 1000);
                            },
                            error: function(e) { console.log(e); },
                            // Form data
                            data: event.target.result,
                            //Options to tell jQuery not to process data or worry about content-type.
                            cache: false,
                            contentType: false,
                            processData: false
                        });
                    }
                }
                else
                    console.log("No file selected")
            });
        });

        var sendLogicData = function() {
            var mail = "support@redpitaya.com";
            var subject = "Decoding Help";
            var body = "DON'T FORGET TO ATTACH FILE!%0D%0A I need help for decoding this data%0D%0A";
            body += "%0D%0A%0D%0A------------------------------------%0D%0A" + "DEBUG INFO, DO NOT EDIT!%0D%0A" + "------------------------------------%0D%0A%0D%0A";
            body += "Parameters:" + "%0D%0A" + JSON.stringify({ parameters: LA.params }) + "%0D%0A";
            body += "Browser:" + "%0D%0A" + JSON.stringify({ parameters: $.browser }) + "%0D%0A";

            var url = 'info/info.json';
            $.ajax({
                method: "GET",
                url: url
            }).done(function(msg) {
                console.log(msg.responseText);
                body += " info.json: " + "%0D%0A" + msg.responseText;
                document.location.href = "mailto:" + mail + "?subject=" + subject + "&body=" + body;
            }).fail(function(msg) {
                console.log(msg.responseText);
                body += " info.json: " + "%0D%0A" + msg.responseText;
                document.location.href = "mailto:" + mail + "?subject=" + subject + "&body=" + body;
            });
        }

        $('#porblemsLink').click(function() {
            $('#decodehelp_dialog').modal('show');
        });

        $('#download_logicdata').click(function() {
            $('#hidden_link_logicdata').get(0).click();
        });

        $('#generate_help_email').click(function() {
            sendLogicData();
        });

        $('button').bind('activeChanged', function() {
            LA.exitEditing(true);
        });
        $('select, input').on('change', function() {
            LA.exitEditing(true);
        });

        $('.edit-mode').on('click', function() {
            LA.state.editing = true;
            $('#right_menu').hide();
            $('#' + $(this).attr('id') + '_dialog').show();
        });

        $('.btn-less').click(function() {
            var inp = $(this).find('input');
            $('.decoder-tab').hide();
            $(inp.attr('data-attr')).show();
        });

    }

}(window.LA = window.LA || {}, jQuery));
