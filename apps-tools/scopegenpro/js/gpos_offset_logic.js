(function(OSC, $, undefined) {


    // OSC.outShowOffset = function(ch, new_params) {
    //     var param_name_offset = "OUTPUT" + ch + "_SHOW_OFF";
    //     var scale = OSC.params.orig["OSC_OUTPUT"+ch+"_SCALE"] ? OSC.params.orig["OSC_OUTPUT"+ch+"_SCALE"].value : undefined;
    //     var offset = OSC.params.orig[param_name_offset] ? OSC.params.orig[param_name_offset].value : undefined;
    //     console.log("Set cursor",scale,offset)
    //     if (scale !== undefined && offset !== undefined && OSC.state.cursor_dragging === false){
    //         var graph_height = $('#graph_grid').outerHeight();
    //         OSC.state.graph_grid_height = graph_height
    //         var volt_per_px =  (scale * 10)  / graph_height;
    //         var px_offset = -(offset / volt_per_px - parseInt($('#output' + ch + '_offset_arrow').css('margin-top')) / 2);
    //         $('#output' + ch + '_offset_arrow').css('top', (graph_height + 7) / 2 + px_offset);
    //     }
    //     OSC.cursorY();
    // }

    OSC.out1ShowOffset = function(new_params) {
        OSC.setOutOffsetPlotCh("1"); // Update value in input box
        OSC.setGposOffset("OUTPUT1", new_params);
    }

    OSC.out2ShowOffset = function(new_params) {
        OSC.setOutOffsetPlotCh("2"); // Update value in input box
        OSC.setGposOffset("OUTPUT2", new_params);
    }


    // Updates Y offset in the signal config dialog, if opened, or saves new value
    OSC.updateYOffset = function(ui) {
        var id = ui.helper[0].id
        var graph_height = $('#graph_grid').outerHeight();
        var zero_pos = (graph_height + 7) / 2;

        var ch_n = ''
        var ch_name = ''
        if (id == 'ch1_offset_arrow') { ch_n = "1"; ch_name = 'CH1'; }
        if (id == 'ch2_offset_arrow') { ch_n = "2"; ch_name = 'CH2'; }
        if (id == 'ch3_offset_arrow') { ch_n = "3"; ch_name = 'CH3'; }
        if (id == 'ch4_offset_arrow') { ch_n = "4"; ch_name = 'CH4'; }
        if (id == 'output1_offset_arrow') { ch_n = "1"; ch_name = 'OUTPUT1'; }
        if (id == 'output2_offset_arrow') { ch_n = "2"; ch_name = 'OUTPUT2'; }
        if (id == 'math_offset_arrow') { ch_n = ""; ch_name = 'MATH'; }

        if (OSC.params.orig['GPOS_SCALE_' + ch_name] == undefined) return;

        var volt_per_px = (OSC.params.orig['GPOS_SCALE_' + ch_name].value * 10) / graph_height;
        var new_value = (zero_pos - ui.position.top + parseInt(ui.helper.css('margin-top')) / 2) * volt_per_px;

        OSC.params.local['GPOS_OFFSET_' + ch_name] = { value: new_value };
        OSC.params.orig['GPOS_OFFSET_' + ch_name] = { value: new_value };


        if ( id == 'ch1_offset_arrow' || id == "ch2_offset_arrow" || id == "ch3_offset_arrow" || id == "ch4_offset_arrow") {
            $('#info_box').html('IN' + ch_n + ' zero offset ' + OSC.convertVoltage(new_value));
            if ($('#in' + ch_n + '_dialog').is(':visible')) {
                OSC.setInOffsetPlotCh(ch_n)
            }
        } else if (ui.helper[0].id == 'output1_offset_arrow' || ui.helper[0].id == 'output2_offset_arrow') {
            $('#info_box').html('OUT' + ch_n + ' zero offset ' + OSC.convertVoltage(new_value));
            if ($('#out'+ch_n+'_dialog').is(':visible')){
                OSC.setOutOffsetPlotCh(ch_n)
            }
        } else if (ui.helper[0].id == 'math_offset_arrow') {
            $('#info_box').html('MATH zero offset ' + OSC.convertVoltage(new_value));
            if ($('#math_dialog').is(':visible')){
                OSC.convertValueToMathUnit(new_value);
            }
        }

        if (new_value !== undefined)
            OSC.sendParams();
    };

    OSC.setGposOffset = function(ch_name) {

        var p_name_offset = "GPOS_OFFSET_"+ ch_name;1
        var p_name_scale = "GPOS_SCALE_"+ ch_name;
        var ch_name_l = ch_name.toLowerCase();
        var graph_height = $('#graph_grid').outerHeight();
        var zero_pos = (graph_height + 7) / 2;

        var scale = OSC.params.orig[p_name_scale] ? OSC.params.orig[p_name_scale].value : undefined;
        var offset = OSC.params.orig[p_name_offset] ? OSC.params.orig[p_name_offset].value : undefined;

        if (scale !== undefined && offset !== undefined && OSC.state.cursor_dragging === false){
            OSC.state.graph_grid_height = graph_height
            var volt_per_px = (scale * 10) / graph_height
            var px_offset = -(offset / volt_per_px - parseInt($('#' + ch_name_l + '_offset_arrow').css('margin-top')) / 2);
            $('#' + ch_name_l + '_offset_arrow').css('top', zero_pos + px_offset);
        }

        OSC.triggerParam("")
        if (!OSC.state.trig_dragging)
            OSC.updateTriggerDragHandle()
        OSC.cursorY();
    }

}(window.OSC = window.OSC || {}, jQuery));