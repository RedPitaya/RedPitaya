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
  OSC.config.app_id = 'scpi_server';
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
  OSC.unexpectedClose = true;

  // Starts the oscilloscope application on server
  OSC.startApp = function() {
    $.get(
      OSC.config.start_app_url
    )
    .done(function(dresult) {
      if(dresult.status == 'OK') {
        try {
            OSC.connectWebSocket();
        } catch(e) {
            OSC.startApp();
        }
      }
      else if(dresult.status == 'ERROR') {
        console.log(dresult.reason ? dresult.reason : 'Could not start the application (ERR1)');
      OSC.startApp();
      }
      else {
        console.log('Could not start the application (ERR2)');
      OSC.startApp();
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


    OSC.convertUnpacked = function(array) {
        var CHUNK_SIZE = 0x8000; // arbitrary number here, not too small, not too big
        var index = 0;
        var length = array.length;
        var result = '';
        var slice;
        while (index < length) {
            slice = array.slice(index, Math.min(index + CHUNK_SIZE, length)); // `Math.min` is not really necessary here I think
            result += String.fromCharCode.apply(null, slice);
            index += CHUNK_SIZE;
        }
        return result;
    }

  // Processes newly received values for parameters
  OSC.processParameters = function(new_params) {
      // Run/Stop button
      for (var param_name in new_params) {
	      if(param_name == 'OSC_RUNNING') {
	        if(new_params[param_name].value === true) {
  		  	  console.log("Running");
	          $('#OSC_RUN').hide();
	          $('#OSC_STOP').css('display','block');
	          OSC.running = true;
	          $('#label-is-runnung').hide();
	          $('#label-is-not-runnung').show();
	        }
	        else {
  		  	  console.log("Stopped");
	          $('#OSC_STOP').hide();
	          $('#OSC_RUN').show();
	          OSC.running = false;
	          $('#label-is-not-runnung').hide();
	          $('#label-is-runnung').show();
	        }
	      }
	   }
  };

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

		setTimeout(function(){
      		OSC.ws.send(JSON.stringify({ parameters: OSC.params.local }));
      		OSC.params.local = {};
	  	}, 2000);
      };

      OSC.ws.onclose = function() {
        OSC.state.socket_opened = false;
        console.log('Socket closed');
        if(OSC.unexpectedClose == true) {
          $('#feedback_error').modal('show');
        }
      };

      $('#send_report_btn').on('click', function() {
        //var file = new FileReader();
        var mail = "support@redpitaya.com";
        var subject = "Feedback";
        var body = "%0D%0A%0D%0A------------------------------------%0D%0A" + "DEBUG INFO, DO NOT EDIT!%0D%0A" + "------------------------------------%0D%0A%0D%0A";
        body += "Parameters:" + "%0D%0A" + JSON.stringify({ parameters: OSC.params }) + "%0D%0A";
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
            try{
                var obj = JSON.parse(msg.responseText);
                ver = " " + obj['version'];
            } catch(e) {};

            body += " info.json: " + "%0D%0A" + msg.responseText;
            document.location.href = "mailto:" + mail + "?subject=" + subject + ver + "&body=" + body;
        } );
      });

      $('#restart_app_btn').on('click', function() {
        location.reload();
      });

      OSC.ws.onerror = function(ev) {
        console.log('Websocket error: ', ev);
      };

      var last_time = undefined;

      OSC.ws.onmessage = function(ev) {
		var start_time = +new Date();
        if(OSC.state.processing) {
          return;
        }

        try {
            var data = new Uint8Array(ev.data);
            OSC.compressed_data += data.length;

            var inflate = new Zlib.Gunzip(data);
            var decompressed = inflate.decompress();
            var arr = new Uint16Array(decompressed)
            var text = OSC.convertUnpacked(arr);
            OSC.decompressed_data += text.length;

            var receive = JSON.parse(text);
            
            OSC.processParameters(receive.parameters);
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

    OSC.params.local['in_command'] = { value: 'OSC_RUN' };
    OSC.ws.send(JSON.stringify({ parameters: OSC.params.local }));
    OSC.params.local = {};

    return true;
  };
}(window.OSC = window.OSC || {}, jQuery));

// Page onload event handler
$(function() {

    var reloaded = $.cookie("scpi_forced_reload");
    if(reloaded == undefined || reloaded == "false")
    {
        $.cookie("scpi_forced_reload", "true");
        window.location.reload(true);
    }
  // Process clicks on top menu buttons
  $('#OSC_RUN').on('click', function(ev) {
    ev.preventDefault();
    //$('#OSC_RUN').hide();
    //$('#OSC_STOP').css('display','block');
    OSC.params.local['OSC_RUN'] = { value: true };
    OSC.sendParams();
    //OSC.running = true;
  });

  $('#OSC_STOP').on('click', function(ev) {
    ev.preventDefault();
    //$('#OSC_STOP').hide();
    //$('#OSC_RUN').show();
    OSC.params.local['OSC_RUN'] = { value: false };
    OSC.sendParams();
    //OSC.running = false;
  });

  // Stop the application when page is unloaded
  window.onbeforeunload = function() {
    OSC.ws.onclose = function () {}; // disable onclose handler first
    // Stop SCPI server and close socket
    OSC.params.local['OSC_RUN'] = { value: false };
    OSC.sendParams();
    OSC.running = false;
    OSC.ws.close();
    $.ajax({
      url: OSC.config.stop_app_url,
      async: false
    });
    OSC.unexpectedClose = false;
  };

  // Everything prepared, start application
  OSC.startApp();
});
