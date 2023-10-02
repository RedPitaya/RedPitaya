(function(OSC, $, undefined) {

    OSC.srcVoltOffset = function(ch, new_params) {
        var old_params = $.extend(true, {}, OSC.params.old);
        var param_name = 'SOUR' + ch + '_VOLT_OFFS';
        if (OSC.rp_model != "Z20") {
            if (!OSC.state.editing) {
                var value = $('#' + param_name).val();
                if (value !== new_params[param_name].value) {
                    OSC.setValue($('#' + param_name), new_params[param_name].value);
                }
            }
        }
    }

    OSC.src1VoltOffset = function(new_params) {
        OSC.srcVoltOffset("1", new_params);
    }

    OSC.src2VoltOffset = function(new_params) {
        OSC.srcVoltOffset("2", new_params);
    }

    OSC.outShowOffset = function(ch, new_params) {
        var param_name_offset = "OUTPUT" + ch + "_SHOW_OFF";
        var offset = OSC.params.orig[param_name_offset] ? OSC.params.orig[param_name_offset].value : undefined;
        if (offset !== undefined && OSC.state.cursor_dragging === false){
            var graph_height = $('#graph_grid').outerHeight();
            var volt_per_px = 10 / graph_height;
            var px_offset = -(offset / volt_per_px - parseInt($('#output' + ch + '_offset_arrow').css('margin-top')) / 2);
            OSC.state.graph_grid_height = $('#graph_grid').outerHeight();
            $('#output' + ch + '_offset_arrow').css('top', (graph_height + 7) / 2 + px_offset);
        }

    }

    OSC.out1ShowOffset = function(new_params) {
        OSC.outShowOffset("1", new_params);

    }

    OSC.out2ShowOffset = function(new_params) {
        OSC.outShowOffset("2", new_params);
    }

    OSC.showOutArrow = function(ch,state) {
        if (state){
            $('#output' + ch + '_offset_arrow').show();
        }else{
            $('#output' + ch + '_offset_arrow').hide();
        }
    }

}(window.OSC = window.OSC || {}, jQuery));