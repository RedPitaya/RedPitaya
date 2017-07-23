(function(APP, $, undefined) {
    
    // App configuration
    APP.config = {};
    APP.config.app_id = '5.read_voltage_gain_offset';
    APP.config.app_url = '/bazaar?start=' + APP.config.app_id + '?' + location.search.substr(1);
    APP.config.socket_url = 'ws://' + window.location.hostname + ':9002';

    // WebSocket
    APP.ws = null;

    // Plot
    APP.plot = {};

    // Signal stack
    APP.signalStack = [];

    // Parameters
    APP.processing = false;
    APP.gain = 1;
    APP.offset = 0;



    // Starts template application on server
    APP.startApp = function() {

        $.get(APP.config.app_url)
            .done(function(dresult) {
                if (dresult.status == 'OK') {
                    APP.connectWebSocket();
                } else if (dresult.status == 'ERROR') {
                    console.log(dresult.reason ? dresult.reason : 'Could not start the application (ERR1)');
                    APP.startApp();
                } else {
                    console.log('Could not start the application (ERR2)');
                    APP.startApp();
                }
            })
            .fail(function() {
                console.log('Could not start the application (ERR3)');
                APP.startApp();
            });
    };




    APP.connectWebSocket = function() {

        //Create WebSocket
        if (window.WebSocket) {
            APP.ws = new WebSocket(APP.config.socket_url);
            APP.ws.binaryType = "arraybuffer";
        } else if (window.MozWebSocket) {
            APP.ws = new MozWebSocket(APP.config.socket_url);
            APP.ws.binaryType = "arraybuffer";
        } else {
            console.log('Browser does not support WebSocket');
        }


        // Define WebSocket event listeners
        if (APP.ws) {

            APP.ws.onopen = function() {
                $('#hello_message').text("Hello, Red Pitaya!");
                console.log('Socket opened');   

                //Set initial gain
                APP.setGain();

                //Set initial offset
                APP.setOffset();
            };

            APP.ws.onclose = function() {
                console.log('Socket closed');
            };

            APP.ws.onerror = function(ev) {
                $('#hello_message').text("Connection error");
                console.log('Websocket error: ', ev);         
            };

            APP.ws.onmessage = function(ev) {
                console.log('Message recieved');


                //Capture signals
                if (APP.processing) {
                    return;
                }
                APP.processing = true;

                try {
                    var data = new Uint8Array(ev.data);
                    var inflate = pako.inflate(data);
                    var text = String.fromCharCode.apply(null, new Uint8Array(inflate));
                    var receive = JSON.parse(text);

                    if (receive.parameters) {
                        
                    }

                    if (receive.signals) {
                        APP.signalStack.push(receive.signals);
                    }
                    APP.processing = false;
                } catch (e) {
                    APP.processing = false;
                    console.log(e);
                } finally {
                    APP.processing = false;
                }



            };
        }
    };




    //Set gain
    APP.setGain = function() {

        APP.gain = $('#gain_set').val();

        var local = {};
        local['GAIN'] = { value: APP.gain };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#gain_value').text(APP.gain);

    };




    //Set gain
    APP.setOffset = function() {

        APP.offset = $('#offset_set').val();

        var local = {};
        local['OFFSET'] = { value: APP.offset };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#offset_value').text(APP.offset);

    };




    // Processes newly received data for signals
    APP.processSignals = function(new_signals) {

        var pointArr = [];
        var voltage;


        // Draw signals
        for (sig_name in new_signals) {

            // Ignore empty signals
            if (new_signals[sig_name].size == 0) continue;

            var points = [];
            for (var i = 0; i < new_signals[sig_name].size; i++) {
                    points.push([i, new_signals[sig_name].value[i]]);
            }

            pointArr.push(points);

            voltage = new_signals[sig_name].value[new_signals[sig_name].size - 1];
        }

        //Update value
        $('#value').text(parseFloat(voltage).toFixed(2) + "V");

        // Update graph
        APP.plot.setData(pointArr);
        APP.plot.resize();
        APP.plot.setupGrid();
        APP.plot.draw();
    };




    //Handler
    APP.signalHandler = function() {
        if (APP.signalStack.length > 0) {
            APP.processSignals(APP.signalStack[0]);
            APP.signalStack.splice(0, 1);
        }
        if (APP.signalStack.length > 2)
            APP.signalStack.length = [];
    }
    setInterval(APP.signalHandler, 15);


}(window.APP = window.APP || {}, jQuery));




// Page onload event handler
$(function() {
    
    //Init plot
    APP.plot = $.plot($("#placeholder"), [], { 
                series: {
                    shadowSize: 0, // Drawing is faster without shadows
                },
                yaxis: {
                    min: 0,
                    max: 5
                },
                xaxis: {
                    min: 0,
                    max: 1024,
                    show: false
                }
    });


    // Gain change
    $("#gain_set").on("change input", function() {

        APP.setGain();
    });


    // Offset change
    $("#offset_set").on("change input", function() {

        APP.setOffset(); 
    });


    // Start application
    APP.startApp();
});
