/*
 * Red Pitaya Oscilloscope client
 *
 * Author: Luka Golinar <luka.golinar@gmail.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
*/

(function(OSC, $, undefined){

	console.log('Starting JS script');
	//Configure APP
	LCR.config = {};
	LCR.config.app_id = 'lcr_meter';
	LCR.config.server_ip = '';
	LCR.config.start_app_url = (LCR.config.server_ip.length ? 'http://' + LCR.config.server_ip : '') + '/bazaar?start=' + LCR.config.app_id;
	LCR.config.socket_url = 'ws://' + (LCR.config.server_ip.length ? LCR.config.server_ip : window.location.hostname) + ':9002';  // WebSocket server URI

	// App state
	LCR.state = {
		socket_opened: false,
		processing: false,
		editing: false,
		trig_dragging: false,
		cursor_dragging: false,
		resized: false,
		sel_sig_name: null,
		fine: false,
		graph_grid_height: null,
		graph_grid_width: null,
		calib: 0
	};

	// Params cache
	LCR.params = {
		orig: {},
	    local: {}
	};

	// Other global variables
  	LCR.ws = null;
  	LCR.touch = {};

  	LCR.connect_time;

  	LCR.startApp = function(){
  		$.get(LCR.config.start_app_url)
  		.done(function(dresult) {
  			if(dresult.status == 'OK'){
  				LCR.connectWebSocket();
  			}
  			else if(dresult.status == 'ERROR'){
  				console.log(dresult.reason ? dresult.reason : 'Could not start the application (ERR1)');
  			}else{
  				console.log('Could not start application (ERR2)');
  			}
  		})
  		.fail(function() {
      		console.log('Could not start the application (ERR3)');
    	});
  	};

	LCR.connectWebSocket = function(){
		if(window.WebSocket){
			LCR.ws = new WebSocket(LCR.config.socket_url);
		}else if(window.MozWebSocket){
			LCR.ws = new MozWebSocket(LCR.config.socket_url);
		}else{
			console.log('Browser does not support WebSocket');
		}
	

		//Define WebSocket event listeners
		if(LCR.ws){
			LCR.ws.onopen = function() {
				LCR.state.socket_opened = true;
				console.log('Socket opened');

				LCR.params.local['in_command'] = { value: 'send_all_params' };
			OSC.ws.send(JSON.stringify({ parameters: OSC.params.local }));
			OSC.params.local = {};
			};

			LCR.ws.onclose() = function(){
				LCR.state.socket_opened = false;
				$('#lb_prim_displ').val('0');
				$('#lb_prim_displ_units').val('-');
				$('#lb_sec_displ').val('0');
				$('#lb_sec_displ_units').val('-');
				console.log('Socket closed');
			};

			LCR.ws.onerror = function(ev) {
	        	console.log('Websocket error: ', ev);
	      	};

	      	LCR.ws.onmessage = function(ev){
	      		if(LCR.state.processing){ return; }

	      		LCR.state.processing = true;
	      		var receive = JSON.parse(ev.data);
	      		if(receive.parameters){
	      			if((Object.keys(LCR.params.orig).length == 0)
	      				&& (Object.keys(receive.parameters).length == 0)){

	      				LCR.params.local['in_command'] = {value: 'send_all_params'};
	      				LCR.ws.send(JSON.stringify({parameters: LCR.params.local}));
	      				LCR.params.local = {};
	      			}else{
	      				LCR.processParameters(receive.parameters);
	      			}
	      		}

	      		LCR.state.processing = false;
	      	};
		}
	};
});