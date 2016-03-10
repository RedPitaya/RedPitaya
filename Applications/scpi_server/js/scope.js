/*
 * Red Pitaya Oscilloscope client
 *
 * Author: Dakus <info@eskala.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
*/

(function(){
    var originalAddClassMethod = jQuery.fn.addClass;
    var originalRemoveClassMethod = jQuery.fn.removeClass;
    $.fn.addClass = function(clss){
        var result = originalAddClassMethod.apply(this, arguments);
        $(this).trigger('activeChanged', 'add');
        return result;
    };
    $.fn.removeClass = function(clss){
        var result = originalRemoveClassMethod.apply(this, arguments);
        $(this).trigger('activeChanged', 'remove');
        return result;
    }
})();

(function(OSC, $, undefined) {

  // App configuration
  OSC.config = {};
  OSC.config.app_id = 'scopegenpro';
  OSC.config.server_ip = '';  // Leave empty on production, it is used for testing only
  OSC.config.start_app_url = (OSC.config.server_ip.length ? 'http://' + OSC.config.server_ip : '') + '/bazaar?start=' + OSC.config.app_id + '?' + location.search.substr(1);
  OSC.config.stop_app_url = (OSC.config.server_ip.length ? 'http://' + OSC.config.server_ip : '') + '/bazaar?stop=' + OSC.config.app_id;
  OSC.config.socket_url = 'ws://' + (OSC.config.server_ip.length ? OSC.config.server_ip : window.location.hostname) + ':9002';  // WebSocket server URI
  OSC.config.graph_colors = {
    'ch1' : '#f3ec1a',
    'ch2' : '#31b44b',
    'output1': '#9595ca',
    'output2': '#ee3739',
    'math': '#ab4d9d',
    'trig': '#75cede'
  };

  // Time scale steps in millisecods
  OSC.time_steps = [
    // Nanoseconds
    100/1000000, 200/1000000, 500/1000000,
    // Microseconds
    1/1000, 2/1000, 5/1000, 10/1000, 20/1000, 50/1000, 100/1000, 200/1000, 500/1000,
    // Millisecods
    1, 2, 5, 10, 20, 50, 100, 200, 500,
    // Seconds
    1*1000, 2*1000, 5*1000, 10*1000, 20*1000, 50*1000
  ];

  // Voltage scale steps in volts
  OSC.voltage_steps = [
    // Millivolts
    1/1000, 2/1000, 5/1000, 10/1000, 20/1000, 50/1000, 100/1000, 200/1000, 500/1000,
    // Volts
    1, 2, 5
  ];

  OSC.bad_connection = [ false, false, false, false ]; // time in s.

  OSC.compressed_data = 0;
  OSC.decompressed_data = 0;
  OSC.refresh_times = [];

  OSC.counts_offset = 0;

  // Sampling rates
  OSC.sample_rates = ['125M', '15.625M', '1.953M', '122.070k', '15.258k', '1.907k'];

  // App state
  OSC.state = {
    socket_opened: false,
    processing: false,
    editing: false,
    trig_dragging: false,
    cursor_dragging: false,
    resized: false,
    sel_sig_name: 'ch1',
    fine: false,
	graph_grid_height: null,
	graph_grid_width: null,
	calib: 0,
	demo_label_visible: false
  };

  // Params cache
  OSC.params = {
    orig: {},
    local: {}
  };

  // Other global variables
  OSC.ws = null;
  OSC.graphs = {};
  OSC.touch = {};

  OSC.connect_time;

  OSC.inGainValue1 = '-';
  OSC.inGainValue2 = '-';
  OSC.loaderShow = false;
  OSC.running = true;

  // Starts the oscilloscope application on server
  OSC.startApp = function() {
    $.get(
      OSC.config.start_app_url
    )
    .done(function(dresult) {
      if(dresult.status == 'OK') {
		 OSC.connectWebSocket();
      }
      else if(dresult.status == 'ERROR') {
        console.log(dresult.reason ? dresult.reason : 'Could not start the application (ERR1)');
        location.reload();
      }
      else {
        console.log('Could not start the application (ERR2)');
        location.reload();
      }
    })
    .fail(function() {
      console.log('Could not start the application (ERR3)');
      OSC.startApp();
    });
  };

	Date.prototype.format = function (mask, utc) {
		return dateFormat(this, mask, utc);
	};

  var g_count = 0;
  var g_time = 0;
  var g_iter = 10;
  var g_delay = 200;
  var g_counter = 0;
  var g_CpuLoad = 100.0;
  var g_TotalMemory = 256.0;
  var g_FreeMemory = 256.0;

  setInterval(function(){
  	if (!OSC.state.socket_opened)
  		return;
	var now = new Date();
	var now_str = now.getHours() +":"+ now.getMinutes() +":"+ now.getSeconds()+":"+now.getMilliseconds();
	var times = "";
	for(var i=0; i<OSC.refresh_times.length; i++)
		times += OSC.refresh_times[i] + " ";

	if (OSC.refresh_times.length < 3)
		OSC.bad_connection[g_counter] = true;
	else
		OSC.bad_connection[g_counter] = false;

	g_counter++;
	if(g_counter == 4) g_counter = 0;


	if($('#weak_conn_msg').is(':visible'))
	{
		if(!OSC.bad_connection[0] && !OSC.bad_connection[1] && !OSC.bad_connection[2] && !OSC.bad_connection[3])
			$('#weak_conn_msg').hide();
	}
	else
	{
		if(OSC.bad_connection[0] && OSC.bad_connection[1] && OSC.bad_connection[2] && OSC.bad_connection[3])
			$('#weak_conn_msg').show();
	}


	OSC.compressed_data = 0;
	OSC.decompressed_data = 0;
	OSC.refresh_times = [];
  }, 1000);

  // Creates a WebSocket connection with the web server
  OSC.connectWebSocket = function() {

    if(window.WebSocket) {
      OSC.ws = new WebSocket(OSC.config.socket_url);
      OSC.ws.binaryType = "arraybuffer";
    }
    else if(window.MozWebSocket) {
      OSC.ws = new MozWebSocket(OSC.config.socket_url);
      OSC.ws.binaryType = "arraybuffer";
    }
    else {
      console.log('Browser does not support WebSocket');
    }

    // Define WebSocket event listeners
    if(OSC.ws) {

      OSC.ws.onopen = function() {
        OSC.state.socket_opened = true;
        console.log('Socket opened');

		OSC.params.local['in_command'] = { value: 'send_all_params' };
		OSC.ws.send(JSON.stringify({ parameters: OSC.params.local }));
		OSC.params.local = {};

		setTimeout(function(){
		  	var ch1_cookie_value = $.cookie("scope_osc_ch1_in_gain");
		  	var ch2_cookie_value = $.cookie("scope_osc_ch2_in_gain");

      		OSC.params.local = {};
		  	if(ch1_cookie_value == '1'){
		  		$("#OSC_CH1_IN_GAIN").parent().removeClass("active")
		  		$("#OSC_CH1_IN_GAIN1").parent().addClass("active")
		  		OSC.params.local['OSC_CH1_IN_GAIN'] = { value: 1 };
		  	}
		  	else if(ch1_cookie_value == '0'){
		  		$("#OSC_CH1_IN_GAIN1").parent().removeClass("active")
		  		$("#OSC_CH1_IN_GAIN").parent().addClass("active")
		  		OSC.params.local['OSC_CH1_IN_GAIN'] = { value: 0 };
		  	}

		  	if(ch2_cookie_value == '1'){
		  		$("#OSC_CH2_IN_GAIN").parent().removeClass("active")
		  		$("#OSC_CH2_IN_GAIN1").parent().addClass("active")
		  		OSC.params.local['OSC_CH2_IN_GAIN'] = { value: 1 };
		  	}
		  	else if(ch2_cookie_value == '0'){
		  		$("#OSC_CH2_IN_GAIN1").parent().removeClass("active")
		  		$("#OSC_CH2_IN_GAIN").parent().addClass("active")
		  		OSC.params.local['OSC_CH2_IN_GAIN'] = { value: 0 };
		  	}
      		OSC.ws.send(JSON.stringify({ parameters: OSC.params.local }));
      		OSC.params.local = {};
	  	}, 2000);

		setTimeout(function(){
			if (OSC.state.demo_label_visible)
				$('#get_lic').modal('show');
		}, 2500);

      };

      OSC.ws.onclose = function() {
        OSC.state.socket_opened = false;
        $('#graphs .plot').hide();  // Hide all graphs
        console.log('Socket closed');
      };

      OSC.ws.onerror = function(ev) {
        console.log('Websocket error: ', ev);
      };

      var last_time = undefined;
      OSC.ws.onmessage = function(ev) {
		var start_time = +new Date();
        if(OSC.state.processing) {
          return;
        }
        OSC.state.processing = true;

		try {
			var data = new Uint8Array(ev.data);
			OSC.compressed_data += data.length;
			var inflate = new Zlib.Gunzip(data);
			var text = String.fromCharCode.apply(null, new Uint16Array(inflate.decompress()));

			OSC.decompressed_data += text.length;

			var receive = JSON.parse(text);

			if(receive.parameters) {
			  if((Object.keys(OSC.params.orig).length == 0) && (Object.keys(receive.parameters).length == 0)) {
				OSC.params.local['in_command'] = { value: 'send_all_params' };
				OSC.ws.send(JSON.stringify({ parameters: OSC.params.local }));
				OSC.params.local = {};
			  } else {
			  	if('CPU_LOAD' in receive.parameters && receive.parameters['CPU_LOAD'].value != undefined)
			  		g_CpuLoad = receive.parameters['CPU_LOAD'].value;

			  	if('TOTAL_RAM' in receive.parameters && receive.parameters['TOTAL_RAM'].value != undefined)
			  		g_TotalMemory = receive.parameters['TOTAL_RAM'].value;

			  	if('FREE_RAM' in receive.parameters && receive.parameters['FREE_RAM'].value != undefined)
			  		g_FreeMemory = receive.parameters['FREE_RAM'].value;

				OSC.processParameters(receive.parameters);

				if (OSC.params.orig['is_demo'])
				{
					if (!OSC.state.demo_label_visible)
					{
						OSC.state.demo_label_visible = true;
						$('#demo_label').show();
					}
				} else {
					if (OSC.state.demo_label_visible)
					{
						OSC.state.demo_label_visible = false;
						$('#demo_label').hide();
					}
				}
			  }
			}

			if(receive.signals) {
				++g_count;
				OSC.processSignals(receive.signals);
				if(last_time == undefined)
					last_time = new Date();

				var diff = new Date() - last_time; //-start_time;
				last_time = new Date();
				g_time = diff;
				OSC.refresh_times.push(diff);

				if (g_count == g_iter && OSC.params.orig['DEBUG_SIGNAL_PERIOD']) {

					g_delay = (g_time/g_count); // TODO
					var period = {};
					period['DEBUG_SIGNAL_PERIOD'] = { value: g_delay*3 };
					OSC.ws.send(JSON.stringify({ parameters: period }));
					g_time = 0;
					g_count = 0;
				}
			}
			OSC.state.processing = false;

		}
		catch (e) {
			OSC.state.processing = false;
			console.log(e);
		}
		finally {
			OSC.state.processing = false;
		}
      };
    }
  };

  // Sends to server modified parameters
  OSC.sendParams = function() {
    if($.isEmptyObject(OSC.params.local)) {
      return false;
    }

    if(! OSC.state.socket_opened) {
      console.log('ERROR: Cannot save changes, socket not opened');
      return false;
    }

    OSC.setDefCursorVals();

    // TEMP TEST
    // TODO: Set the update period depending on device type
    //OSC.params.local['DEBUG_PARAM_PERIOD'] = { value: 200 };
    //OSC.params.local['DEBUG_SIGNAL_PERIOD'] = { value: 100 };

    OSC.params.local['in_command'] = { value: 'send_all_params' };
    // Send new values and reset the local params object
//    if (OSC.params.local['OSC_MATH_OFFSET'])
//		OSC.params.local['OSC_MATH_OFFSET'].value *= OSC.div;
    OSC.ws.send(JSON.stringify({ parameters: OSC.params.local }));
    OSC.params.local = {};

    return true;
  };
}(window.OSC = window.OSC || {}, jQuery));

// Page onload event handler
$(function() {

  // Process clicks on top menu buttons
  $('#OSC_RUN').on('click', function(ev) {
    ev.preventDefault();
    $('#OSC_RUN').hide();
    $('#OSC_STOP').css('display','block');
    OSC.params.local['OSC_RUN'] = { value: true };
    OSC.sendParams();
    OSC.running = true;
  });

  $('#OSC_STOP').on('click', function(ev) {
    ev.preventDefault();
    $('#OSC_STOP').hide();
    $('#OSC_RUN').show();
    OSC.params.local['OSC_RUN'] = { value: false };
    OSC.sendParams();
    OSC.running = false;
  });

  // Stop the application when page is unloaded
  window.onbeforeunload = function() {
    OSC.ws.onclose = function () {}; // disable onclose handler first
    OSC.ws.close();
    $.ajax({
      url: OSC.config.stop_app_url,
      async: false
    });
  };

  // Everything prepared, start application
  OSC.startApp();
});
