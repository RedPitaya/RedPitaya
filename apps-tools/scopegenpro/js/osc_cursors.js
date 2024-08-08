(function(OSC, $, undefined) {

    // Init drag listeners for off cursors (IN,OUT,MATH)
    OSC.initOSCCursors = function() {
        // Voltage offset arrow dragging
        $('.y-offset-arrow').draggable({
            axis: 'y',
            containment: 'parent',
            start: function() {
                OSC.state.cursor_dragging = true;
            },
            drag: function(ev, ui) {
                var margin_top = parseInt(ui.helper.css('marginTop'));
                var min_top = ((ui.helper.height() / 2) + margin_top) * -1;
                var max_top = $('#graphs').height() - margin_top;

                if (ui.position.top < min_top) {
                    ui.position.top = min_top;
                } else if (ui.position.top > max_top) {
                    ui.position.top = max_top;
                }

                OSC.updateYOffset(ui);
                OSC.cursorY()
            },
            stop: function(ev, ui) {
                if (!OSC.state.simulated_drag) {
                    OSC.updateYOffset(ui);
                    OSC.cursorY()
                    $('#info_box').empty();
                }
                OSC.state.cursor_dragging = false;
            }
        });
    }

    // OSC.chOffset = function(ch_name) {
    //     // var params = $.extend(true, {}, OSC.orig.old);
    //     var param_name = "OSC_"+ ch_name+"_OFFSET";
    //     var scale = OSC.params.orig["OSC_"+ch_name+"_SCALE"] ? OSC.params.orig["OSC_"+ch_name+"_SCALE"].value : undefined;
    //     var offset = OSC.params.orig['OSC_' + ch_name + '_OFFSET'].value;
    //     var ch_name_l = ch_name.toLowerCase();
    //     console.log("Set cursor",scale,offset)
    //     if (scale !== undefined && offset !== undefined && OSC.state.cursor_dragging === false){
    //         var graph_height = $('#graph_grid').outerHeight();
    //         OSC.state.graph_grid_height = graph_height
    //         var volt_per_px = (scale * 10) / graph_height
    //         var px_offset = -(offset / volt_per_px - parseInt($('#' + ch_name_l + '_offset_arrow').css('margin-top')) / 2);
    //         $('#' + ch_name_l + '_offset_arrow').css('top', (graph_height + 7) / 2 + px_offset);
    //     }

    //     var field = $('#' + param_name);
    //     var units;
    //     if (scale != undefined) {
    //         if (Math.abs(scale) >= 1) {
    //             units = 'V';
    //         } else if (Math.abs(scale) >= 0.001) {
    //             units = 'mV';
    //         }
    //     } else
    //         units = $('#' + param_name+'_UNIT').html();
    //     if (offset){
    //         var multiplier = units == "mV" ? 1000 : 1;

    //         var probeAttenuation = OSC.params.orig['OSC_' + ch_name + '_PROBE'] !== undefined ? OSC.params.orig['OSC_' + ch_name + '_PROBE'].value : 1;
    //         var gain_mode = OSC.params.orig['OSC_' + ch_name + '_IN_GAIN'] !== undefined ? OSC.params.orig['OSC_' + ch_name + '_IN_GAIN'].value : 0;

    //         field.val(OSC.formatInputValue(offset * multiplier, probeAttenuation, units == "mV", gain_mode !== 0));
    //         field.attr("step",OSC.getStepValue(probeAttenuation, units == "mV", gain_mode !== 0));

    //     }


    //     OSC.triggerParam("")
    //     if (!OSC.state.trig_dragging)
    //         OSC.updateTriggerDragHandle()
    //     OSC.cursorY();
    // }


    OSC.ch1Offset = function(new_params) {
        OSC.setGposOffset("CH1");
        OSC.setInOffsetPlotCh("1")
    }

    OSC.ch2Offset = function(new_params) {
        OSC.setGposOffset("CH2");
        OSC.setInOffsetPlotCh("2")
    }

    OSC.ch3Offset = function(new_params) {
        OSC.setGposOffset("CH3");
        OSC.setInOffsetPlotCh("3")
    }

    OSC.ch4Offset = function(new_params) {
        OSC.setGposOffset("CH4");
        OSC.setInOffsetPlotCh("4")
    }

    OSC.ch1OffsetZero = function(new_params) {
        OSC.setGposOffset("CH1"); // For update trigger and cursors
        OSC.setInOffsetZeroPlotCh("1")
    }

    OSC.ch2OffsetZero = function(new_params) {
        OSC.setGposOffset("CH2"); // For update trigger and cursors
        OSC.setInOffsetZeroPlotCh("2")
    }

    OSC.ch3OffsetZero = function(new_params) {
        OSC.setGposOffset("CH3"); // For update trigger and cursors
        OSC.setInOffsetZeroPlotCh("3")
    }

    OSC.ch4OffsetZero = function(new_params) {
        OSC.setGposOffset("CH4"); // For update trigger and cursors
        OSC.setInOffsetZeroPlotCh("4")
    }

    OSC.showInArrow = function(ch_name,state) {
        var ch_name_l = ch_name.toLowerCase();
        if (state){
            $('#' + ch_name_l + '_offset_arrow').show();
        }else{
            $('#' + ch_name_l + '_offset_arrow').hide();
        }
    }

    // Updates trigger level in the trigger config dialog, if opened, or saves new value
    OSC.updateTrigLevel = function(ui, save) {

        $('#trigger_level').css('top', ui.position.top);

        if (OSC.params.orig['OSC_TRIG_SOURCE'] !== undefined) {

            if (OSC.params.orig['OSC_TRIG_SOURCE'].value < OSC.adc_channes) {
                var ots = OSC.params.orig['OSC_TRIG_SOURCE'].value + 1;
                var ref_scale = "GPOS_SCALE_CH"+ots
                var source_offset = OSC.params.orig['GPOS_OFFSET_CH'+ots].value;
                var source_offset_zero = OSC.params.orig['GPOS_OFFSET_ZERO_CH'+ots].value;
                var source_inverted = OSC.params.orig['GPOS_INVERTED_CH'+ots].value;

                // var ref_scale = (OSC.params.orig['OSC_TRIG_SOURCE'].value == 0 ? 'OSC_CH1_SCALE' : 'OSC_CH2_SCALE');
                // var source_offset = (OSC.params.orig['OSC_TRIG_SOURCE'].value == 0 ? OSC.params.orig['OSC_CH1_OFFSET'].value : OSC.params.orig['OSC_CH2_OFFSET'].value);

                if (OSC.params.orig[ref_scale] !== undefined && source_inverted !== undefined && source_offset !== undefined && source_offset_zero !== undefined) {
                    var graph_height = $('#graph_grid').height();
                    var volt_per_px = (OSC.params.orig[ref_scale].value * 10) / graph_height;
                    var new_value = (graph_height / 2 - ui.position.top - ui.helper.height() / 2 - parseInt(ui.helper.css('margin-top')) + 2.5) * volt_per_px - source_offset - source_offset_zero * (source_inverted ? -1 : 1);

                    if (OSC.params.orig['OSC_TRIG_LIMIT'] !== undefined){
                        if (new_value > OSC.params.orig['OSC_TRIG_LIMIT'].value) new_value = OSC.params.orig['OSC_TRIG_LIMIT'].value
                        if (new_value < -OSC.params.orig['OSC_TRIG_LIMIT'].value) new_value = -OSC.params.orig['OSC_TRIG_LIMIT'].value
                    }

                    if (OSC.params.orig['OSC_TRIG_LIMIT'] !== undefined && (new_value > OSC.params.orig['OSC_TRIG_LIMIT'].value || new_value < -OSC.params.orig['OSC_TRIG_LIMIT'].value)) {
                        $('#info_box').html('Trigger at its limit');
                        if (new_value > OSC.params.orig['OSC_TRIG_LIMIT'].value)
                            new_value = OSC.params.orig['OSC_TRIG_LIMIT'].value
                        if (new_value < -OSC.params.orig['OSC_TRIG_LIMIT'].value)
                            new_value = -OSC.params.orig['OSC_TRIG_LIMIT'].value
                        $("#OSC_TRIG_LEVEL").attr("max", OSC.params.orig['OSC_TRIG_LIMIT'].value);
                        $("#OSC_TRIG_LEVEL").attr("min", -OSC.params.orig['OSC_TRIG_LIMIT'].value);
                    } else {
                        $('#info_box').html('Trigger level ' + OSC.convertVoltage(new_value));
                    }

                    var probeAttenuation = 1;
                        var jumperSettings = 1;
                        var ch = "CH"+(parseInt($("#OSC_TRIG_SOURCE").val()) + 1);
                    if (ch == "CH1" || ch == "CH2" || ch == "CH3" || ch == "CH4") {
                        probeAttenuation = parseInt($("#OSC_" + ch + "_PROBE option:selected").text());
                        jumperSettings = $("#OSC_" + ch + "_IN_GAIN").parent().hasClass("active") ? 1 : 20;
                    }

                    OSC.setValue($('#OSC_TRIG_LEVEL'), OSC.formatInputValue(new_value, probeAttenuation, false, jumperSettings == 20));
                    $('#OSC_TRIG_LEVEL').attr("step",OSC.getStepValue(probeAttenuation, false, jumperSettings == 20));
                    $('#OSC_TRIG_LEVEL').change();

                    if (save) {
                        OSC.params.local['OSC_TRIG_LEVEL'] = { value: new_value };
                        OSC.sendParams();
                    }
                }
            } else {
                console.log('Trigger level for source ' + OSC.params.orig['OSC_TRIG_SOURCE'].value + ' not yet supported');
            }
        }
    };

    OSC.updateTriggerDragHandle = function() {
        // Trigger level arrow dragging
        $('#trig_level_arrow').on( "mouseenter", function() {
            OSC.state.trig_in = true;
        });

        $('#trig_level_arrow').on( "mouseout", function() {
            OSC.state.trig_out= false;
        });

        $('#trig_level_arrow').draggable({
            axis: 'y',
            containment: OSC.calculateTrigLimit(),
            start: function(ev, ui) {
                OSC.state.trig_dragging = true;
            },
            drag: function(ev, ui) {
                OSC.updateTrigLevel(ui, true);
            },
            stop: function(ev, ui) {
                OSC.updateTrigLevel(ui, true);
                OSC.state.trig_dragging = false;
                $('#info_box').empty();
            }
        });
    }

    OSC.calculateTrigLimit = function(){
        var trig_sour = OSC.params.orig['OSC_TRIG_SOURCE'] == undefined ? undefined : OSC.params.orig['OSC_TRIG_SOURCE'].value;
        var ots = trig_sour + 1;
        var source_offset = OSC.params.orig['GPOS_OFFSET_CH'+ots] === undefined ? undefined : OSC.params.orig['GPOS_OFFSET_CH'+ots].value;
        var source_offset_zero = OSC.params.orig['GPOS_OFFSET_ZERO_CH'+ots] === undefined ? undefined : OSC.params.orig['GPOS_OFFSET_ZERO_CH'+ots].value;
        var scale_value = OSC.params.orig['GPOS_SCALE_CH'+ots] === undefined ? undefined : OSC.params.orig['GPOS_SCALE_CH'+ots].value;
        var limit_value = OSC.params.orig['OSC_TRIG_LIMIT'] === undefined ? undefined : OSC.params.orig['OSC_TRIG_LIMIT'].value;
        var source_inverted = OSC.params.orig['GPOS_INVERTED_CH'+ots] === undefined ? undefined : OSC.params.orig['GPOS_INVERTED_CH'+ots].value;

        if (source_offset !== undefined && source_offset_zero !== undefined && scale_value !== undefined && limit_value !== undefined && source_inverted !== undefined){
            var graph_height = $('#graph_grid').outerHeight();
            var graph_top = $('#graph_grid').offset().top;
            var volt_per_px = (scale_value * 10) / graph_height;
            var px_offset = ((source_offset + source_offset_zero * (source_inverted ? -1 : 1)) / volt_per_px - parseInt($('#trig_level_arrow').css('margin-top')) / 2);
            var px_limit = limit_value / volt_per_px ;
            var max_value = (graph_height + 8.0) / 2.0 - px_offset + px_limit;
            var min_value = (graph_height + 8.0) / 2.0 - px_offset - px_limit;
            var limits = [0,Math.max(graph_top + min_value - 1,graph_top),0, Math.min(graph_top + max_value + 1,graph_top + graph_height)];
            return limits;
        }
        return 'parent'
    }

    OSC.triggerParam = function(param_name) {
        var old_params = $.extend(true, {}, OSC.params.old);
        var trig_sour = OSC.params.orig['OSC_TRIG_SOURCE'] == undefined ? undefined : OSC.params.orig['OSC_TRIG_SOURCE'].value;

        if (!OSC.state.trig_dragging && trig_sour !== undefined) {

            var show_in_channel = OSC.params.orig['CH'+(trig_sour+1)+'_SHOW'] === undefined ? undefined : OSC.params.orig['CH'+(trig_sour+1)+'_SHOW'].value;
            if (trig_sour >= OSC.adc_channes){
                show_in_channel = undefined
            }

            // Trigger button is blured out and trigger level is hidden for source 'EXT'
            if ((trig_sour == 4) || (show_in_channel !== undefined && !show_in_channel)) {
                $('#trigger_level, #trig_level_arrow, .in_trigger_settings').hide();
                $('#right_menu .menu-btn.trig').prop('disabled', true);
                $('#osc_trig_level_info').html('-');
            } else {

                var ots = trig_sour + 1;
                var source_offset = OSC.params.orig['GPOS_OFFSET_CH'+ots] === undefined ? undefined : OSC.params.orig['GPOS_OFFSET_CH'+ots].value;
                var source_offset_zero = OSC.params.orig['GPOS_OFFSET_ZERO_CH'+ots] === undefined ? undefined : OSC.params.orig['GPOS_OFFSET_ZERO_CH'+ots].value;
                var source_inverted = OSC.params.orig['GPOS_INVERTED_CH'+ots] === undefined ? undefined : OSC.params.orig['GPOS_INVERTED_CH'+ots].value;
                var scale_value = OSC.params.orig['GPOS_SCALE_CH'+ots] === undefined ? undefined : OSC.params.orig['GPOS_SCALE_CH'+ots].value;
                var level_value = OSC.params.orig['OSC_TRIG_LEVEL'] === undefined ? undefined : OSC.params.orig['OSC_TRIG_LEVEL'].value;

                if (source_offset != undefined  && source_offset_zero != undefined && scale_value != undefined && level_value != undefined && source_inverted != undefined){

                    var graph_height = $('#graph_grid').outerHeight();
                    var volt_per_px = (scale_value * 10) / graph_height;
                    var px_offset = -((level_value + source_offset + source_offset_zero * (source_inverted ? -1 : 1)) / volt_per_px - parseInt($('#trig_level_arrow').css('margin-top')) / 2);
                    var trig_position = (graph_height + 7) / 2 + px_offset;
                    var limits = OSC.calculateTrigLimit()
                    if (0 > trig_position) trig_position = 0
                    if (graph_height < trig_position) trig_position = graph_height
                    $('#trig_level_arrow, #trigger_level').css('top', trig_position).show();
                    $('.in_trigger_settings').show();
                    $('#right_menu .menu-btn.trig').prop('disabled', false);
                    $('#osc_trig_level_info').html(OSC.convertVoltage(level_value));

                    if ((!OSC.state.editing)) {
                        var value = $('#OSC_TRIG_LEVEL').val();
                        if (value !== level_value) {
                            var probeAttenuation = 1;
                            var jumperSettings = 1;
                            var ch = "CH"+(parseInt($("#OSC_TRIG_SOURCE").val()) + 1);
                            if (ch == "CH1" || ch == "CH2" || ch == "CH3" || ch == "CH4") {
                                probeAttenuation = parseInt($("#OSC_" + ch + "_PROBE option:selected").text());
                                jumperSettings = $("#OSC_" + ch + "_IN_GAIN").parent().hasClass("active") ? 1 : 20;
                            }
                            OSC.setValue($('#OSC_TRIG_LEVEL'), OSC.formatInputValue(level_value, probeAttenuation, false, jumperSettings == 20));
                            $('#OSC_TRIG_LEVEL').attr("step",OSC.getStepValue(probeAttenuation, false, jumperSettings == 20));

                        }
                    }

                }
            }
        }
        // Trigger source
        if (param_name == 'OSC_TRIG_SOURCE' && trig_sour != undefined) {
            var x = trig_sour;
            var source = ""
            if (x == 4){
                source = "EXT"
            }else{
                source = "IN"+(x+1);
            }
            $("#OSC_TRIG_SOURCE").val(trig_sour);
            $('#osc_trig_source_ch').html(source);
        }

        if (param_name == 'OSC_TRIG_LIMIT' && OSC.params.orig['OSC_TRIG_LIMIT'] !== undefined) {
            $("#OSC_TRIG_LEVEL").attr("max", OSC.params.orig['OSC_TRIG_LIMIT'].max);
            $("#OSC_TRIG_LEVEL").attr("min", OSC.params.orig['OSC_TRIG_LIMIT'].min);
        }
    }

    OSC.triggerLimit = function(new_params) {
        OSC.triggerParam('OSC_TRIG_LIMIT');
        OSC.updateTriggerDragHandle();
    }


    OSC.trigLevel = function(new_params) {
        OSC.triggerParam('OSC_TRIG_LEVEL');
    }

    OSC.extTrigLevel = function(new_params) {
        function update(param) {
            if(param in OSC.params.orig){
                $("#"+param).attr("min", OSC.params.orig[param].min);
                $("#"+param).attr("max", OSC.params.orig[param].max);
                $("#"+param).val(OSC.params.orig[param].value);
            }
        }
        update("OSC_EXT_TRIG_LEVEL")
    }

    OSC.trigSource = function(new_params) {
        OSC.triggerParam('OSC_TRIG_SOURCE');
        if (!OSC.state.trig_dragging)
            OSC.updateTriggerDragHandle();
    }

    OSC.moveTimeOffset = function(new_left) {
        OSC.state.time_dragging = true
        var graph_width = $('#graph_grid').outerWidth();
        var elem_width = $('#time_offset_arrow').width();
        var zero_pos = (graph_width + 2) / 2;
        var ms_per_px = (OSC.params.orig['OSC_TIME_SCALE'].value * 10) / graph_width;
        var new_value = +(((zero_pos - new_left - elem_width / 2 - 1) * ms_per_px).toFixed(6));
        var buf_width = graph_width + 2;
        var ratio = buf_width / (buf_width * OSC.params.orig['OSC_VIEV_PART'].value);

        OSC.params.local['OSC_TIME_OFFSET'] = { value: (zero_pos - new_left - elem_width / 2 - 1) * ms_per_px };
        OSC.sendParams();

        $('#info_box').html('Time offset ' + OSC.convertTime(new_value));
        $('#buf_time_offset').css('left', buf_width / 2 - buf_width * OSC.params.orig['OSC_VIEV_PART'].value / 2 + (new_left + 8) / ratio  - 5).show();
    }

    OSC.endTimeMove = function(new_left) {
        if (!OSC.state.simulated_drag && OSC.params.orig['OSC_TIME_SCALE'] !== undefined) {
            var graph_width = $('#graph_grid').outerWidth();
            var elem_width = $('#time_offset_arrow').width();
            var zero_pos = (graph_width + 2) / 2;
            var ms_per_px = (OSC.params.orig['OSC_TIME_SCALE'].value * 10) / graph_width;

            OSC.params.local['OSC_TIME_OFFSET'] = { value: (zero_pos - new_left - elem_width / 2 - 1) * ms_per_px };
            OSC.sendParams();
            $('#info_box').empty();
        }
        OSC.state.time_dragging = false
    }

    OSC.timeOffset = function(new_params) {
        OSC.setTimeScaleOffset("OSC_TIME_OFFSET")
        if (OSC.state.time_dragging) return

        var ts = OSC.params.orig['OSC_TIME_SCALE'] !== undefined ? OSC.params.orig['OSC_TIME_SCALE'].value : undefined
        var toff = OSC.params.orig['OSC_TIME_OFFSET'] !== undefined ? OSC.params.orig['OSC_TIME_OFFSET'].value : undefined
        var vp = OSC.params.orig['OSC_VIEV_PART'] !== undefined ? OSC.params.orig['OSC_VIEV_PART'].value : undefined

        if (ts === undefined || vp === undefined || toff === undefined) return;

        var graph_width = $('#graph_grid').outerWidth();
        var ms_per_px = (ts * 10) / graph_width;
        var px_offset = -(toff / ms_per_px + $('#time_offset_arrow').width() / 2 + 1);
        var arrow_left = (graph_width + 2) / 2 + px_offset;
        var buf_width = graph_width + 2;
        var ratio = buf_width / (buf_width * vp);
        OSC.state.graph_grid_width = graph_width;
        $('#time_offset_arrow').css('left', arrow_left).show();
        $('#buf_time_offset').css('left', buf_width / 2 - buf_width * vp / 2 + (arrow_left + 8)/ ratio - 5).show();
    }

}(window.OSC = window.OSC || {}, jQuery));