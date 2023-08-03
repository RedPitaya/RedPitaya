(function(OSC, $, undefined) {

    OSC.initOSCCursors = function() {
        $('.y-offset-arrow').mousedown(function(event) {
            OSC.state.cursor_dragging = true;
        });

        $('.y-offset-arrow').mouseup(function(event) {
            OSC.state.cursor_dragging = true;
        });

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

                OSC.updateYOffset(ui, true);
            },
            stop: function(ev, ui) {
                if (!OSC.state.simulated_drag) {
                    OSC.updateYOffset(ui, true);
                    $('#info_box').empty();
                }
                OSC.state.cursor_dragging = false;
            }
        });
    }

    OSC.chOffset = function(ch_name) {
        // var params = $.extend(true, {}, OSC.orig.old);
        var param_name = "OSC_"+ ch_name+"_OFFSET";
        var scale = OSC.params.orig["OSC_"+ch_name+"_SCALE"] ? OSC.params.orig["OSC_"+ch_name+"_SCALE"].value : undefined;
        var offset = OSC.params.orig['OSC_' + ch_name + '_OFFSET'].value;
        var ch_name_l = ch_name.toLowerCase();

        //if ($('#' + ch_name_l + '_offset_arrow').is(':visible') || (OSC.state.graph_grid_height && OSC.state.graph_grid_height !== $('#graph_grid').outerHeight())) {
            if (scale !== undefined && offset !== undefined && OSC.state.cursor_dragging === false){
                var volt_per_px = (scale * 10) / $('#graph_grid').outerHeight();
                var px_offset = -(offset / volt_per_px - parseInt($('#' + ch_name_l + '_offset_arrow').css('margin-top')) / 2);
                OSC.state.graph_grid_height = $('#graph_grid').outerHeight();
                $('#' + ch_name_l + '_offset_arrow').css('top', ($('#graph_grid').outerHeight() + 7) / 2 + px_offset);
            }
        //}

        var field = $('#' + param_name);
        var units;
        if (scale != undefined) {
            if (Math.abs(scale) >= 1) {
                units = 'V';
            } else if (Math.abs(scale) >= 0.001) {
                units = 'mV';
            }
        } else
            units = $('#' + param_name+'_UNIT').html();
        if (offset){
            var multiplier = units == "mV" ? 1000 : 1;

            var probeAttenuation = OSC.params.orig['OSC_' + ch_name + '_PROBE'] !== undefined ? OSC.params.orig['OSC_' + ch_name + '_PROBE'].value : 1;
            var gain_mode = OSC.params.orig['OSC_' + ch_name + '_IN_GAIN'] !== undefined ? OSC.params.orig['OSC_' + ch_name + '_IN_GAIN'].value : 0;

            field.val(OSC.formatInputValue(offset * multiplier, probeAttenuation, units == "mV", gain_mode !== 0));
            field.attr("step",OSC.getStepValue(probeAttenuation, units == "mV", gain_mode !== 0));

        }


        OSC.triggerParam("")
        if (!OSC.state.trig_dragging)
            OSC.updateTriggerDragHandle()
        OSC.cursorY();
    }

    OSC.ch1Offset = function(new_params) {
        OSC.chOffset("CH1");
    }

    OSC.ch2Offset = function(new_params) {
        OSC.chOffset("CH2");
    }

    OSC.ch3Offset = function(new_params) {
        OSC.chOffset("CH3");
    }

    OSC.ch4Offset = function(new_params) {
        OSC.chOffset("CH4");
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
                var ref_scale = "OSC_CH"+ots+"_SCALE";
                var source_offset = OSC.params.orig['OSC_CH'+ots+'_OFFSET'].value;

                // var ref_scale = (OSC.params.orig['OSC_TRIG_SOURCE'].value == 0 ? 'OSC_CH1_SCALE' : 'OSC_CH2_SCALE');
                // var source_offset = (OSC.params.orig['OSC_TRIG_SOURCE'].value == 0 ? OSC.params.orig['OSC_CH1_OFFSET'].value : OSC.params.orig['OSC_CH2_OFFSET'].value);

                if (OSC.params.orig[ref_scale] !== undefined) {
                    var graph_height = $('#graph_grid').height();
                    var volt_per_px = (OSC.params.orig[ref_scale].value * 10) / graph_height;
                    var new_value = (graph_height / 2 - ui.position.top - (ui.helper.height() - 2) / 2 - parseInt(ui.helper.css('margin-top'))) * volt_per_px - source_offset;

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

                    if ($('#trig_dialog').is(':visible')) {
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
                    }
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
        var source_offset = OSC.params.orig['OSC_CH'+ots+'_OFFSET'] === undefined ? undefined : OSC.params.orig['OSC_CH'+ots+'_OFFSET'].value;
        var scale_value = OSC.params.orig['OSC_CH'+ots+'_SCALE'] === undefined ? undefined : OSC.params.orig['OSC_CH'+ots+'_SCALE'].value;
        var limit_value = OSC.params.orig['OSC_TRIG_LIMIT'] === undefined ? undefined : OSC.params.orig['OSC_TRIG_LIMIT'].value;

        if (source_offset !== undefined && scale_value !== undefined && limit_value !== undefined){
            var graph_height = $('#graph_grid').outerHeight();
            var graph_top = $('#graph_grid').offset().top;
            var volt_per_px = (scale_value * 10) / graph_height;
            var px_offset = ((source_offset) / volt_per_px - parseInt($('#trig_level_arrow').css('margin-top')) / 2);
            var px_limit = limit_value / volt_per_px;
            var max_value = (graph_height + 7) / 2 - px_offset + px_limit;
            var min_value = (graph_height + 7) / 2 - px_offset - px_limit;
            var limits = [0,graph_top + min_value,0,graph_top + max_value];
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
                $('#trigger_level, #trig_level_arrow').hide();
                $('#right_menu .menu-btn.trig').prop('disabled', true);
                $('#osc_trig_level_info').html('-');
                if (OSC.is_ext_trig_level_present && trig_sour == 4) {
                    var level_value = OSC.params.orig['OSC_TRIG_LEVEL'] === undefined ? undefined : OSC.params.orig['OSC_TRIG_LEVEL'].value;
                    if ((!OSC.state.editing) && level_value !== undefined) {
                        var value = $('#OSC_TRIG_LEVEL').val();
                        if (value !== level_value) {
                            var probeAttenuation = 1;
                            var jumperSettings = 1;
                            OSC.setValue($('#OSC_TRIG_LEVEL'), OSC.formatInputValue(level_value, probeAttenuation, false, jumperSettings == 20));
                            $('#OSC_TRIG_LEVEL').attr("step",OSC.getStepValue(probeAttenuation, false, jumperSettings == 20));
                        }
                    }
                }
            } else {

                var ots = trig_sour + 1;
                var source_offset = OSC.params.orig['OSC_CH'+ots+'_OFFSET'] === undefined ? undefined : OSC.params.orig['OSC_CH'+ots+'_OFFSET'].value;
                var scale_value = OSC.params.orig['OSC_CH'+ots+'_SCALE'] === undefined ? undefined : OSC.params.orig['OSC_CH'+ots+'_SCALE'].value;
                var level_value = OSC.params.orig['OSC_TRIG_LEVEL'] === undefined ? undefined : OSC.params.orig['OSC_TRIG_LEVEL'].value;

                if (source_offset != undefined && scale_value != undefined && level_value != undefined){

                    var graph_height = $('#graph_grid').outerHeight();
                    var volt_per_px = (scale_value * 10) / graph_height;
                    var px_offset = -((level_value + source_offset) / volt_per_px - parseInt($('#trig_level_arrow').css('margin-top')) / 2);
                    $('#trig_level_arrow, #trigger_level').css('top', (graph_height + 7) / 2 + px_offset).show();

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
            $("#OSC_TRIG_LEVEL").attr("max", OSC.params.orig['OSC_TRIG_LIMIT'].value);
            $("#OSC_TRIG_LEVEL").attr("min", -OSC.params.orig['OSC_TRIG_LIMIT'].value);
        }
    }

    OSC.triggerLimit = function(new_params) {
        OSC.triggerParam('OSC_TRIG_LIMIT');
        OSC.updateTriggerDragHandle();
    }


    OSC.trigLevel = function(new_params) {
        OSC.triggerParam('OSC_TRIG_LEVEL');
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
        var buf_width = graph_width - 2;
        var ratio = buf_width / (buf_width * OSC.params.orig['OSC_VIEV_PART'].value);

        OSC.params.local['OSC_TIME_OFFSET'] = { value: (zero_pos - new_left - elem_width / 2 - 1) * ms_per_px };
        OSC.sendParams();

        $('#info_box').html('Time offset ' + OSC.convertTime(new_value));
        $('#buf_time_offset').css('left', buf_width / 2 - buf_width * OSC.params.orig['OSC_VIEV_PART'].value / 2 + new_left / ratio - 4).show();
    }

    OSC.endTimeMove = function(new_left) {
        if (!OSC.state.simulated_drag) {
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
        var buf_width = graph_width - 2;
        var ratio = buf_width / (buf_width * vp);
        OSC.state.graph_grid_width = graph_width;
        $('#time_offset_arrow').css('left', arrow_left).show();
        $('#buf_time_offset').css('left', buf_width / 2 - buf_width * vp / 2 + arrow_left / ratio - 4).show();
    }

}(window.OSC = window.OSC || {}, jQuery));