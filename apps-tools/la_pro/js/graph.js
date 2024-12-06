/*
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


(function(LA, $, undefined) {

    LA.scaleWasChanged = false
    LA.graphs = undefined;
    LA.graph_colors = {
        'ch1': '#dc1809',
        'ch2': '#e6890c',
        'ch3': '#fed730',
        'ch4': '#00ae73',
        'ch5': '#0760be',
        'ch6': '#846167',
        'ch7': '#6b6f61',
        'ch8': '#ebf1e7'
    };

    LA.initGraph = function() {
        if (LA.graphs === undefined) {
            LA.graphs = {};
            LA.graphs.elem = $('<div class="plot" />').css($('#graph_grid').css(['height', 'width'])).appendTo('#graphs');
            LA.graphs.plot = $.plot(LA.graphs.elem, [], {
                name: "ch1",
                series: {
                    shadowSize: 0, // Drawing is faster without shadows
                },
                yaxis: {
                    min: 0,
                    max: 9
                },
                xaxis: {
                    min: 0
                },
                grid: {
                    show: false
                },
                colors: [
                ]
            });
        }
    }

    // Draws the grid on the lowest canvas layer
    LA.drawGraphGrid = function() {
        var canvas_width = $('#graphs').width() - 2;
        var canvas_height = Math.round(canvas_width / 2);

        // var center_x = canvas_width / 2;
        // var center_y = canvas_height / 2;

        var ctx = $('#graph_grid')[0].getContext('2d');

        var x_offset = 0;
        var y_offset = 0;

        // Set canvas size
        ctx.canvas.width = canvas_width;
        ctx.canvas.height = canvas_height;

        // Set draw options
        ctx.beginPath();
        ctx.lineWidth = 1;
        ctx.strokeStyle = '#5d5d5c';

        // Draw ticks
        for (var i = 1; i < 45; i++) {
            x_offset = x_offset + (canvas_width / 45);
            y_offset = y_offset + (canvas_height / 45);

            ctx.moveTo(x_offset, canvas_height - 3);
            ctx.lineTo(x_offset, canvas_height);

            ctx.moveTo(0, y_offset);
            ctx.lineTo(3, y_offset);
        }

        // Draw lines
        x_offset = 0;
        y_offset = 0;

        for (var i = 1; i < 9; i++) {
            x_offset = x_offset + (canvas_height / 9);
            ctx.moveTo(0, x_offset);
            ctx.lineTo(canvas_width, x_offset);
        }

        ctx.stroke();

        for (var i = 1; i < 10; i++) {
            y_offset = y_offset + (canvas_width / 10);
            ctx.moveTo(y_offset, 0);
            ctx.lineTo(y_offset, canvas_height);
        }

        ctx.stroke();

        // Draw central cross
        // ctx.beginPath();
        // ctx.lineWidth = 1;
        // ctx.strokeStyle = '#999';

        // ctx.moveTo(center_x, 0);
        // ctx.lineTo(center_x, canvas_height);

        // ctx.moveTo(0, center_y);
        // ctx.lineTo(canvas_width, center_y);

        // ctx.stroke();
    };

    LA.getPlot = function() {

        if (LA.graphs && LA.graphs.elem) {
            var plot = LA.graphs.plot;
            return plot;
        }
        return undefined;
    };

    // Changes X zoom/scale for all signals
    LA.changeXZoom = function(direction, curr_scale, send_changes) {
        // Calculate time per division
        var samplerate = CLIENT.getValue("LA_CUR_FREQ")
        var scale = CLIENT.getValue("LA_SCALE")

        if (samplerate !== undefined && scale !== undefined){
            var newScaleMul = 1;

            if (direction == '+') {
                newScaleMul = scale * ((OSC.state.fine == false) ? 2 : 1.1);
                if (newScaleMul > 100) newScaleMul = 100
                // OSC.allSignalShown = false; // Reset 'do not change time_scale' flag
            } else if (direction == '-') {
                newScaleMul = scale / ((OSC.state.fine == false) ? 2 : 1.1);
                if (newScaleMul < 0.01) newScaleMul = 0.01
            } else if (direction == '1') {
                newScaleMul = 1;
            }
            if (newScaleMul !== CLIENT.getValue("LA_SCALE")){
                CLIENT.parametersCache["LA_SCALE"] = { value : newScaleMul }
                CLIENT.sendParameters();
                return true;
            }
        }
        return false;
    };

    LA.time_zoom = function(ev, offsetPx, byMouseWheel) {

        // for (var i = 1; i < 5; i++) {
        //     var bus = "bus" + i;
        //     if (OSC.buses[bus].name !== undefined && OSC.buses[bus].name == "UART" && OSC.buses[bus].enabled) {
        //         OSC.buses[bus].samplerate = OSC.state.acq_speed;
        //         OSC.params.local[OSC.buses[bus].decoder + "_parameters"] = {
        //             value: OSC.buses[bus]
        //         };
        //     }
        //     if (OSC.buses[bus].name !== undefined && OSC.buses[bus].name == "CAN" && OSC.buses[bus].enabled) {
        //         OSC.buses[bus].acq_speed = OSC.state.acq_speed;
        //         OSC.params.local[OSC.buses[bus].decoder + "_parameters"] = {
        //             value: OSC.buses[bus]
        //         };
        //     }
        // }

        OSC.state.resized = true;
        LA.scaleWasChanged = LA.changeXZoom(ev);
        OSC.guiHandler();
    }

}(window.LA = window.LA || {}, jQuery));
