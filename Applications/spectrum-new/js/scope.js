/*
 * Red Pitaya Spectrum Analizator client
 *
 * Author: Dakus <info@eskala.eu>
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
*/

(function(SPEC, $, undefined) {

  // App configuration
  SPEC.config = {};
  SPEC.config.app_id = 'spectrum-new';
  SPEC.config.server_ip = '';  // Leave empty on production, it is used for testing only
  SPEC.config.start_app_url = (SPEC.config.server_ip.length ? 'http://' + SPEC.config.server_ip : '') + '/bazaar?start=' + SPEC.config.app_id + '?' + location.search.substr(1);
  SPEC.config.stop_app_url = (SPEC.config.server_ip.length ? 'http://' + SPEC.config.server_ip : '') + '/bazaar?stop=' + SPEC.config.app_id;
//  SPEC.config.socket_url = 'ws://localhost:9002';
  SPEC.config.socket_url = 'ws://' + (SPEC.config.server_ip.length ? SPEC.config.server_ip : window.location.hostname) + ':9002';  // WebSocket server URI
  SPEC.config.socket_reconnect_timeout = 1000; // Milliseconds
  SPEC.config.graph_colors = {
    'ch1' : '#f3ec1a',
    'ch2' : '#31b44b'
  };
  SPEC.config.waterf_img_path = "/tmp/"
  SPEC.freq_unit = ['Hz', 'kHz', 'MHz'];

  SPEC.config.xmin = 0;
  SPEC.config.xmax = 63;
  SPEC.config.unit = 2;
	
  SPEC.time_steps = [
    // Hz
    1/10, 2/10, 5/10, 1, 2, 5, 10, 20, 50, 100, 200, 500
    // KHz
    //1*1000, 2*1000, 5*1000, 10*1000, 20*1000, 50*1000, 100*1000, 200*1000, 500*1000,
    // MHz
    //1*1000000, 2*1000000, 5*1000000, 10*1000000, 20*1000000, 50*1000000, 100*1000000, 200*1000000, 500*1000000,
  ];
  
  // Voltage scale steps in volts
  //TODO power steps in dBm
  SPEC.voltage_steps = [
    // dBm
    1/10, 2/10, 5/10, 1, 2, 5, 10, 20, 50, 100
  ];

  SPEC.xmin = -1000000;
  SPEC.xmax = 1000000;  
  SPEC.ymax = 20.0;
  SPEC.ymin = -120.0;

  SPEC.points_per_px = 5;             // How many points per pixel should be drawn. Set null for unlimited (will disable client side decimation).  
  SPEC.scale_points_size = 10; 
  // App state
  SPEC.state = {
    socket_opened: false,
    processing: false,
    editing: false,
    resized: false,
	cursor_dragging: false,
    sel_sig_name: null,
    fine: false
  };
  
  // Params cache
  SPEC.params = { 
    orig: {}, 
    local: {}
  };

  // Other global variables
  SPEC.ws = null;
  SPEC.graphs = {};
  SPEC.touch = {};
	
  SPEC.datasets = [];
  
  // Starts the spectrum application on server
  SPEC.startApp = function() {
    $.get(
      SPEC.config.start_app_url
    )
    .done(function(dresult) {
      if(dresult.status == 'OK') {
        SPEC.connectWebSocket();
      }
      else if(dresult.status == 'ERROR') {
        console.log(dresult.reason ? dresult.reason : 'Could not start the application (ERR1)');
      }
      else {
        console.log('Could not start the application (ERR2)');
      }
    })
    .fail(function() {
      console.log('Could not start the application (ERR3)');
    });
  };
  
  // Creates a WebSocket connection with the web server  
  SPEC.connectWebSocket = function() {
    
    if(window.WebSocket) {
      SPEC.ws = new WebSocket(SPEC.config.socket_url);
    } 
    else if(window.MozWebSocket) {
      SPEC.ws = new MozWebSocket(SPEC.config.socket_url);
    } 
    else {
      console.log('Browser does not support WebSocket');
    }
    
    // Define WebSocket event listeners
    if(SPEC.ws) {
    
      SPEC.ws.onopen = function() {
        SPEC.state.socket_opened = true;
        console.log('Socket opened');
      };
      
      SPEC.ws.onclose = function() {
        SPEC.state.socket_opened = false;
        $('#graphs .plot').hide();  // Hide all graphs
        SPEC.hideCursors();
		SPEC.hideInfo();
        console.log('Socket closed. Trying to reopen in ' + SPEC.config.socket_reconnect_timeout/1000 + ' sec...');
        
        // Try to reconnect after a defined timeout
        setTimeout(function() {
          SPEC.ws = new WebSocket(SPEC.config.socket_url);
        }, SPEC.config.socket_reconnect_timeout);
      };
      
      SPEC.ws.onerror = function(ev) {
        console.log('Websocket error: ', ev);
      };
      
      SPEC.ws.onmessage = function(ev) {
        if(SPEC.state.processing) {
          return;
        }
        SPEC.state.processing = true;
        
        var receive = JSON.parse(ev.data);

        if(receive.parameters) {
			SPEC.processParameters(receive.parameters);
			if(Object.keys(SPEC.params.orig).length < 30 && Object.keys(receive.parameters).length < 30) {
				SPEC.params.local['in_command'] = { value: 'send_all_params' };
				SPEC.ws.send(JSON.stringify({ parameters: SPEC.params.local }));
				SPEC.params.local = {};
			} else {
				SPEC.processParameters(receive.parameters);
			}
        }
        
        if(receive.signals) {
          SPEC.processSignals(receive.signals);
        }
        
        SPEC.state.processing = false;
      };
    }
  };

  // Processes newly received values for parameters
  SPEC.processParameters = function(new_params) {
	var old_params = $.extend(true, {}, SPEC.params.orig);
    
    for(var param_name in new_params) {
        // Save new parameter value
      SPEC.params.orig[param_name] = new_params[param_name];

      // Run/Stop button
      if(param_name == 'SPEC_RUN') {
        if(new_params[param_name].value === true) {
          $('#SPEC_RUN').hide();
          $('#SPEC_STOP').css('display','block');
        }
        else {
          $('#SPEC_STOP').hide();
          $('#SPEC_RUN').show();
        }
      }
      // All other parameters
      else {

		//if(param_name == 'peak1_unit')	
		{
			// 0 - Hz, 1 - kHz, 2 - MHz

			var freq_unit1 = 'Hz';
			var unit = new_params['peak1_unit'];
			if (unit)
				freq_unit1 = (unit.value == 1 ? 'k' : (unit.value == 2 ? 'M' : '')) + 'Hz';
			if (new_params['peak1_power'] && new_params['peak1_freq'] && !$('#CH1_FREEZE').hasClass('active'))
				$('#peak_ch1').val(SPEC.floatToLocalString(new_params['peak1_power'].value.toFixed(3)) + ' dBm @ ' + SPEC.floatToLocalString(new_params['peak1_freq'].value.toFixed(2)) + ' ' + freq_unit1);		
		}
		/*else*/ if (param_name == 'peak2_unit')
		{
			// 0 - Hz, 1 - kHz, 2 - MHz
			var freq_unit2 = 'Hz';
			var unit = new_params['peak2_unit'];
			if (unit)
				freq_unit2 = (unit.value == 1 ? 'k' : (unit.value == 2 ? 'M' : '')) + 'Hz';
			if (new_params['peak2_power'] && new_params['peak2_freq'] && !$('#CH2_FREEZE').hasClass('active'))
				$('#peak_ch2').val(SPEC.floatToLocalString(new_params['peak2_power'].value.toFixed(3)) + ' dBm @ ' + SPEC.floatToLocalString(new_params['peak2_freq'].value.toFixed(2)) + ' ' + freq_unit2);
		}
		/*else*/ if(param_name == 'w_idx')
		{
			// Update waterfall images
			var img_num = new_params['w_idx'].value;
			if(img_num <= 999) { 
			  img_num = ('00' + img_num).slice(-3); 
			}
			
		    $('#waterfall_ch1').attr('src', SPEC.config.waterf_img_path + 'wat1_' + img_num + '.jpg');
		    $('#waterfall_ch2').attr('src', SPEC.config.waterf_img_path + 'wat2_' + img_num + '.jpg');	
		}
		// Y cursors
        else if(param_name == 'SPEC_CURSOR_Y1' || param_name == 'SPEC_CURSOR_Y2') {
          if(! SPEC.state.cursor_dragging) {
            var y = (param_name == 'SPEC_CURSOR_Y1' ? 'y1' : 'y2');
            
            if(new_params[param_name].value) {

				var plot = SPEC.getPlot();
				if(SPEC.isVisibleChannels() && plot){

					var axes = plot.getAxes();   
					var min_y =axes.yaxis.min;  
					var max_y = axes.yaxis.max;

					var new_value = new_params[y == 'y1' ? 'SPEC_CUR1_V' : 'SPEC_CUR2_V'].value;
					var correct_val = Math.max(min_y, new_value);
					new_value = Math.min(correct_val, max_y);

					var volt_per_px = 1/axes.yaxis.scale;
					var top =  (axes.yaxis.max - new_value)/volt_per_px;
				  
					$('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').css('top', top).show();
					$('#cur_' + y + '_info').html((new_value.toFixed(Math.abs(new_value) >= 0.1 ? 2 : 3)) + 'dBm').css('margin-top', (top < 16 ? 3 : ''));
				}
            }
            else {
              $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').hide();
            }
          }
        }
        // X cursors
        else if(param_name == 'SPEC_CURSOR_X1' || param_name == 'SPEC_CURSOR_X2') {
          if(! SPEC.state.cursor_dragging) {
            var x = (param_name == 'SPEC_CURSOR_X1' ? 'x1' : 'x2');
            
            if(new_params[param_name].value) {

				var plot = SPEC.getPlot();
				if(SPEC.isVisibleChannels() && plot){

					var axes = plot.getAxes();   
					var min_x =Math.max(0, axes.xaxis.min);  
					var max_x = axes.xaxis.max;
					var new_value = -1*new_params[x == 'x1' ? 'SPEC_CUR1_T' : 'SPEC_CUR2_T'].value;
					var correct_val = Math.max(min_x, new_value);
					new_value = -1* Math.min(correct_val, max_x);
					var graph_width = $('#graph_grid').width();
					var ms_per_px = 1/axes.xaxis.scale;
					var msg_width = $('#cur_' + x + '_info').outerWidth();
					var left = (-axes.xaxis.min -  new_value)/ms_per_px;
					var unit = SPEC.freq_unit[new_params['freq_unit'].value];
					$('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').css('left', left).show();
					$('#cur_' + x + '_info')
					.html(-(new_value.toFixed(Math.abs(new_value) >= 0.1 ? 2 : Math.abs(new_value) >= 0.001 ? 4 : 6)) + unit)
					.css('margin-left', (left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));
				}
            }
            else {
              $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').hide();
            }
          }
        }
		
		var field = $('#' + param_name);
		
        // Do not change fields from dialogs when user is editing something        
        if((!SPEC.state.editing || field.closest('.menu-content').length == 0) 
	&& (old_params[param_name] === undefined || old_params[param_name].value !== new_params[param_name].value)) {
          if(field.is('select') || field.is('input:text')) {
            field.val(new_params[param_name].value);

			if(param_name == 'xmin' || param_name == 'xmax' || param_name == 'freq_unit')	
			{	
				SPEC.updateZoom();

				if (new_params['freq_unit'])
					$('#freq').html(SPEC.freq_unit[new_params['freq_unit'].value]);
				$('.freeze.active').removeClass('active');
			}
          }
          else if(field.is('button')) {
            field[new_params[param_name].value === true ? 'addClass' : 'removeClass' ]('active');
          }
          else if(field.is('input:radio')) {
            var radios = $('input[name="' + param_name + '"]');
            
            radios.closest('.btn-group').children('.btn.active').removeClass('active');
            radios.eq([+new_params[param_name].value]).prop('checked', true).parent().addClass('active');
          }
          else if(field.is('span')) {

            field.html(new_params[param_name].value);	
			if(param_name == 'SPEC_TIME_SCALE')
			{
				var unit = SPEC.freq_unit[new_params["freq_unit"].value];
				$('#freq_scale_unit').html(unit);		
				$('#SPEC_TIME_SCALE').html(new_params[param_name].value.toFixed(3));
			}
          }
        }
      }
    }
	    
    // Resize double-headed arrows showing the difference between cursors
    SPEC.updateYCursorDiff();
    SPEC.updateXCursorDiff();
  };

  // Processes newly received data for signals
  SPEC.processSignals = function(new_signals) {
    var visible_btns = [];
    var visible_plots = [];
    var visible_info = '';
    var start = +new Date();
    
    // Do nothing if no parameters received yet
    if($.isEmptyObject(SPEC.params.orig)) {
      return;
    }
    
	// Keep the datasets for frozen channels
	var frozen_dsets = [
	  (($('#CH1_FREEZE').hasClass('active') && SPEC.datasets[0] !== undefined) ? SPEC.datasets[0] : null),
	  (($('#CH2_FREEZE').hasClass('active')) ? (SPEC.datasets[1] ? SPEC.datasets[1] : SPEC.datasets[0]) : null)
	];
	SPEC.datasets = [];
	var sig_count = 0;
    // (Re)Draw every signal
$('#waterfall-holder_ch1').hide();
$('#waterfall-holder_ch2').hide();
    for(sig_name in new_signals) {
	// Ignore empty signals
      if(new_signals[sig_name].size == 0) {
        continue;
      }

      sig_count++;
      // Ignore disabled signals
      if(SPEC.params.orig[sig_name.toUpperCase() + '_SHOW'] && SPEC.params.orig[sig_name.toUpperCase() + '_SHOW'].value == false) {
		
		$('#info .left-info .info-title .' + sig_name + ', #info .left-info .info-value .'+ sig_name).hide();
		$('#waterfall-holder_' + sig_name).hide();  
		continue;
      }
      
      var points = [];
      var color = SPEC.config.graph_colors[sig_name];

	  var idx = sig_name[sig_name.length-1] - '0' - 1;
	  if(frozen_dsets[idx]){
		points = frozen_dsets[idx].data;
	  }
	  else if (SPEC.params.orig['xmax'] && SPEC.params.orig['xmin']) {
		for(var i = 0; i < new_signals[sig_name].size; i++) {
			var d = (SPEC.params.orig['xmax'].value)/(new_signals[sig_name].size - 1);
			var p = d*i;
	   	 	points.push([p, new_signals[sig_name].value[i]]);
	  	}
	  }
	  SPEC.datasets.push({ color: color, data: points}); 
	  var sig_btn = $('#right_menu .menu-btn.' + sig_name);
	  sig_btn.prop('disabled', false);
      visible_btns.push(sig_btn[0]);
      visible_info += (visible_info.length ? ',' : '') + '.' + sig_name;
	  $('#info .left-info .info-title .' + sig_name + ', #info .left-info .info-value .'+ sig_name).show();
	  $('#waterfall-holder_' + sig_name).show();
	}     

	if(SPEC.isVisibleChannels()){

      if(SPEC.graphs && SPEC.graphs.elem) {
       
		SPEC.graphs.elem.show();
        
        if(SPEC.state.resized) {
          SPEC.graphs.plot.resize();
          SPEC.graphs.plot.setupGrid();
        }
        var filtered_data = SPEC.filterData(SPEC.datasets, SPEC.graphs.plot.width());   
        SPEC.graphs.plot.setData(filtered_data);
        SPEC.graphs.plot.draw();
		SPEC.updateCursors();
		//to change position while resizing automatically
		$('.harrow').css('left', 'inherit');
		$('.varrow').css('top', 'inherit');
      }
      else {
        SPEC.graphs.elem = $('<div class="plot" />').css($('#graph_grid').css(['height','width'])).appendTo('#graphs');
	// Local optimization    
    	var filtered_data = SPEC.filterData(SPEC.datasets, SPEC.graphs.elem.width());       

	    SPEC.graphs.plot = $.plot(SPEC.graphs.elem, filtered_data, {
		colors: [SPEC.config.graph_colors['ch1'], SPEC.config.graph_colors['ch2']],    // channel1, channel2            
		series: {
        shadowSize: 0  // Drawing is faster without shadows
          },
          yaxis: {
            autoscaleMargin: 1,
            min: -120, // (sig_name == 'ch1' || sig_name == 'ch2' ? SPEC.params.orig['SPEC_' + sig_name.toUpperCase() + '_SCALE'].value * -5 : null),
            max: 20   // (sig_name == 'ch1' || sig_name == 'ch2' ? SPEC.params.orig['SPEC_' + sig_name.toUpperCase() + '_SCALE'].value * 5 : null)
          },
          xaxis: {
            min: 0
          },
          grid: {
            show: true
          }
        });
		SPEC.updateZoom();
		//update power div
		var scale = SPEC.graphs.plot.getAxes().yaxis.tickSize;
		$('#SPEC_CH1_SCALE').html(scale);
		$('#SPEC_CH2_SCALE').html(scale);
		SPEC.params.local['SPEC_CH1_SCALE'] = { value: scale };
		SPEC.params.local['SPEC_CH2_SCALE'] = { value: scale };
		SPEC.sendParams();

		var offset = SPEC.graphs.plot.getPlotOffset();
		var margins =  {};
		margins.marginLeft = offset.left + 'px';
		margins.marginRight = offset.right + 'px';
		$('.waterfall-holder').css(margins);
      }

      visible_plots.push(SPEC.graphs.elem[0]);
    
	  // Disable buttons related to inactive signals
	  $('#right_menu .menu-btn').not(visible_btns).prop('disabled', true);

	  $('.pull-right').show();
		// Reset resize flag
	  SPEC.state.resized = false;
		
	  //console.log('Duration: ' + (+new Date() - start));
	}
  };

  // Exits from editing mode
  SPEC.exitEditing = function() {
	
	if(!($('#CH1_SHOW').hasClass('active') || $('#CH2_SHOW').hasClass('active'))){
		if(SPEC.params.orig['CH1_SHOW'].value == true)
			$('#CH2_SHOW').addClass('active');
		else if(SPEC.params.orig['CH2_SHOW'].value == true)
			$('#CH1_SHOW').addClass('active');
	}

    for(var key in SPEC.params.orig) {
      var field = $('#' + key);
      var value = undefined;

      if(key == 'SPEC_RUN'){
        value = (field.is(':visible') ? 0 : 1);
      }
      else if(field.is('select') || field.is('input:text')) {
        value = field.val();
      }
      else if(field.is('button')) {
        value = (field.hasClass('active') ? 1 : 0);
      }
      else if(field.is('input:radio')) {
        value = $('input[name="' + key + '"]:checked').val();
      }

	if (SPEC.params.orig['CH1_SHOW'].value)
		$('#waterfall-holder_ch1').show();
	else
        $('#waterfall-holder_ch1').hide();

      if(value !== undefined && value != SPEC.params.orig[key].value) {
        console.log(key + ' changed from ' + SPEC.params.orig[key].value + ' to ' + ($.type(SPEC.params.orig[key].value) == 'boolean' ? !!value : value));
        SPEC.params.local[key] = { value: ($.type(SPEC.params.orig[key].value) == 'boolean' ? !!value : value) };
      }
	if (key == 'xmin')
		SPEC.config.xmin = value;
	if (key == 'xmax')
		SPEC.config.xmax = value;
	if (key == 'freq_unit')
		SPEC.config.unit = value;

	console.log(SPEC.config.xmin, SPEC.config.xmax, SPEC.config.unit);
    }
    
    // Check changes in measurement list
    var mi_count = 0;
    $('#info-meas').empty();
    $('#meas_list .meas-item').each(function(index, elem) {
      var $elem = $(elem);
      var item_val = $elem.data('value');
      
      if(item_val !== null) {
        SPEC.params.local['SPEC_MEAS_SEL' + (++mi_count)] = { value: item_val };
        $('#info-meas').append(
          '<div>' + $elem.data('operator') + '(<span class="' + $elem.data('signal').toLowerCase() + '">' + $elem.data('signal') + '</span>) <span id="SPEC_MEAS_VAL' + mi_count + '">-</span></div>'
        );
      }
    });
    
    // Send params then reset editing state and hide dialog
    SPEC.sendParams();
    SPEC.state.editing = false;
    $('.dialog:visible').hide();
    $('#right_menu').show(); 
  };

  // Sends to server modified parameters
  SPEC.sendParams = function() {
	if($.isEmptyObject(SPEC.params.local)) {
		return false;
    }
    
    if(! SPEC.state.socket_opened) {
      console.log('ERROR: Cannot save changes, socket not opened');
      return false;
    }
    
    // TEMP TEST
    //SPEC.params.local['DEBUG_PARAM_PERIOD'] = { value: 5000 };
    //SPEC.params.local['DEBUG_SIGNAL_PERIOD'] = { value: 1000 };
	
    SPEC.setDefCursorVals();
	SPEC.params.local['in_command'] = { value: 'send_all_params' };
    SPEC.ws.send(JSON.stringify({ parameters: SPEC.params.local }));
    SPEC.params.local = {};
    return true;
  };

  // Draws the grid on the lowest canvas layer
  SPEC.drawGraphGrid = function() {
    var canvas_width = $('#graphs').width() - 2;
    var canvas_height = Math.round(canvas_width / 2.3);
    
    var center_x = canvas_width / 2;
    var center_y = canvas_height / 2;
    
    var ctx = $('#graph_grid')[0].getContext('2d');
    
    var x_offset = 0;
    var y_offset = 0;
    
    // Set canvas size
    ctx.canvas.width = canvas_width;
    ctx.canvas.height = canvas_height;
    
    // Set draw options
    ctx.beginPath();
    ctx.lineWidth = 1;
    ctx.strokeStyle = '#343433';

    // Draw ticks
    for(var i = 1; i < 50; i++) {
      x_offset = x_offset + (canvas_width / 50);
      y_offset = y_offset + (canvas_height / 50);
      
      if(i == 25) {
        continue;
      }
      
      ctx.moveTo(x_offset, canvas_height - 3);
      ctx.lineTo(x_offset, canvas_height);
      
      ctx.moveTo(0, y_offset);
      ctx.lineTo(3, y_offset);
    }

    // Draw lines
    x_offset = 0;
    y_offset = 0;
    
    for(var i = 1; i < 10; i++){
      x_offset = x_offset + (canvas_height / 10);
      y_offset = y_offset + (canvas_width / 10);
      
      if(i == 5) {
        continue;
      }
      
      ctx.moveTo(y_offset, 0);
      ctx.lineTo(y_offset, canvas_height);
      
      ctx.moveTo(0, x_offset);
      ctx.lineTo(canvas_width, x_offset);
    } 
    
    ctx.stroke();
    
    // Draw central cross
    ctx.beginPath();
    ctx.lineWidth = 1;
    ctx.strokeStyle = '#343433';
    
    ctx.moveTo(center_x, 0);
    ctx.lineTo(center_x, canvas_height);
    
    ctx.moveTo(0, center_y);
    ctx.lineTo(canvas_width, center_y);
    
    ctx.stroke();
  };

   // Changes Y zoom/scale for the selected signal
  SPEC.changeYZoom = function(direction, curr_scale, send_changes) {
	if(!(SPEC.graphs && SPEC.graphs.elem))
		return null;
	
	var plot_elem = SPEC.graphs.elem;
	
	if(!SPEC.isVisibleChannels())
		return null;    
	
	var options = SPEC.graphs.plot.getOptions();
    var axes = SPEC.graphs.plot.getAxes();
    var curr_scale = axes.yaxis.tickSize;

	if((curr_scale >= SPEC.voltage_steps[ SPEC.voltage_steps.length - 1] && direction == '-') ||
		(curr_scale <= SPEC.voltage_steps[0] && direction == '+' )) 
		return null;


	var range = axes.yaxis.max - axes.yaxis.min;
	var delta = direction == '+' ? 1 : -1
	options.yaxes[0].min = axes.yaxis.min + delta*range*0.1;
	options.yaxes[0].max = axes.yaxis.max - delta*range*0.1;

	SPEC.graphs.plot.setupGrid();
	SPEC.graphs.plot.draw();
	axes = SPEC.graphs.plot.getAxes();
	var new_scale = axes.yaxis.tickSize;
    
	if(send_changes !== false) {
        SPEC.params.local['SPEC_CH1_SCALE'] = { value: new_scale };
		SPEC.params.local['SPEC_CH2_SCALE'] = { value: new_scale };
        SPEC.sendParams();
    }
	SPEC.updateWaterfallWidth();
	SPEC.updateCursors();

    return new_scale;
	
  };

  // Changes X zoom/scale for all signals
  SPEC.changeXZoom = function(direction, curr_scale, send_changes) {
    if(!(SPEC.graphs && SPEC.graphs.elem))
		return null;
	if(!SPEC.isVisibleChannels())
		return null;
	var options = SPEC.graphs.plot.getOptions();
    var axes = SPEC.graphs.plot.getAxes();
    var curr_scale = axes.xaxis.tickSize;

	if((curr_scale >= SPEC.time_steps[ SPEC.time_steps.length - 1] && direction == '-') || (curr_scale <= SPEC.time_steps[0] && direction == '+' ))
	{
		return null;
	}

	var range = axes.xaxis.max - axes.xaxis.min;
	var delta = direction == '+' ? 1 : -1

	options.xaxes[0].min = Math.max(SPEC.config.xmin, axes.xaxis.min + delta*range*0.1); 
	options.xaxes[0].max = Math.min(SPEC.config.xmax, axes.xaxis.max - delta*range*0.1);

	SPEC.params.local['xmin'] = { value: options.xaxes[0].min };
	SPEC.params.local['xmax'] = { value: options.xaxes[0].max };
	console.log(axes.xaxis.min, axes.xaxis.max, delta, curr_scale);
	SPEC.sendParams();

	SPEC.graphs.plot.setupGrid();
	SPEC.graphs.plot.draw();
	axes = SPEC.graphs.plot.getAxes();
	var new_scale = axes.xaxis.tickSize;

	if(send_changes !== false) {
		SPEC.params.local['SPEC_TIME_SCALE'] = { value: new_scale };
		SPEC.sendParams();
	}
	SPEC.updateWaterfallWidth();
	SPEC.updateCursors();

    return new_scale;
  };
	
  SPEC.resetZoom = function() {
    if(!(SPEC.graphs && SPEC.graphs.elem))
		return;
	if(!SPEC.isVisibleChannels())
		return;

	var plot = SPEC.graphs.plot; 
    var curr_options = plot.getOptions();
    curr_options.xaxes[0].min = SPEC.params.orig['xmin'].value;
    curr_options.xaxes[0].max = SPEC.params.orig['xmax'].value;
    curr_options.yaxes[0].min = SPEC.ymin;
    curr_options.yaxes[0].max = SPEC.ymax;

	SPEC.params.local['xmin'] = { value: SPEC.config.xmin };
	SPEC.params.local['xmax'] = { value: SPEC.config.xmax };
	SPEC.sendParams();

    plot.setupGrid();
    plot.draw();
    var axes = plot.getAxes();
	var cur_x_scale = axes.xaxis.tickSize;
	var cur_y_scale = axes.yaxis.tickSize;
    SPEC.params.local['SPEC_TIME_SCALE'] = {value : cur_x_scale};
    SPEC.params.local['SPEC_CH1_SCALE'] = {value : cur_y_scale};
    
    SPEC.sendParams();
	SPEC.updateWaterfallWidth();
	SPEC.updateCursors();

  };
	
  SPEC.isVisibleChannels = function(){
	return ((SPEC.params.orig.CH1_SHOW && SPEC.params.orig.CH1_SHOW.value == true)  
		|| (SPEC.params.orig['CH2_SHOW'] && SPEC.params.orig['CH2_SHOW'].value == true));
  };

  SPEC.getPlot = function(){
	
	if(SPEC.graphs && SPEC.graphs.elem){
		var plot = SPEC.graphs.plot;
 		return plot;
	}
	return null;
  };

  SPEC.updateWaterfallWidth = function(){
	var plot = SPEC.getPlot();
	if(!(SPEC.isVisibleChannels() && plot)){
		return;
	} 
	var offset = plot.getPlotOffset();
	var margins =  {};
	margins.marginLeft = offset.left + 'px';
	margins.marginRight = offset.right + 'px';

	$('.waterfall-holder').css(margins);

  };

  SPEC.updateCursors = function(){
	var plot = SPEC.getPlot();
	if(!(SPEC.isVisibleChannels() && plot)){
		return;
	} 
	var offset = plot.getPlotOffset();
	var left = offset.left+1 + 'px';
	var right = offset.right+1 + 'px';
	var top = offset.top+1 + 'px';
	var bottom = offset.bottom+9 + 'px';
	
	//update lines length
	$('.hline').css('left', left);
	$('.hline').css('right', right);
	$('.vline').css('top', top);
	$('.vline').css('bottom', bottom);

	//update arrows positions
	var diff_left =  offset.left +2+ 'px';
	var diff_top = offset.top -2+ 'px';
	var margin_left =  offset.left -7 -2 + 'px';
	var margin_top =  -7 + offset.top -2+ 'px';
	var margin_bottom =  -2 + offset.bottom + 'px';
	var line_margin_left =  offset.left  - 2+ 'px';
	var line_margin_top =  offset.top  - 2+ 'px';
	var line_margin_bottom =  offset.bottom  - 2+ 'px';

	$('.varrow').css('margin-left', margin_left);
	$('.harrow').css('margin-top', margin_top);
	$('.harrow').css('margin-bottom', margin_bottom);
	$('.vline').css('margin-left',line_margin_left);
	$('.hline').css('margin-top', line_margin_top);
	$('.hline').css('margin-bottom', line_margin_bottom);
	
	$('#cur_x_diff').css('margin-left', diff_left);
	$('#cur_y_diff').css('margin-top', diff_top);
	$('#cur_x_diff_info').css('margin-left', diff_left);
	$('#cur_y_diff_info').css('margin-top', diff_top);
	
  };
  
  SPEC.hideCursors = function(){
	$('.hline, .vline, .harrow, .varrow, .cur_info').hide();
	$('#cur_y_diff').hide();
	$('#cur_x_diff').hide();
  };

	SPEC.hideInfo = function(){
		$('.pull-right').hide();
		// Disable buttons related to inactive signals
		$('#right_menu .menu-btn').prop('disabled', true);
		$('#info').hide();
		$('.waterfall-holder').hide();
};

  SPEC.updateZoom = function() {

	if(SPEC.graphs && SPEC.graphs.elem)
	{
		var plot_elem = SPEC.graphs.elem;
		if(SPEC.isVisibleChannels())   {

			var plot = SPEC.graphs.plot;
			SPEC.params.local['xmin'] = { value: SPEC.params.orig['xmin'].value };
			SPEC.params.local['xmax'] = { value: SPEC.params.orig['xmax'].value };
		  
			var axes = plot.getAxes();
			var options = plot.getOptions();

			options.xaxes[0].min = SPEC.params.local['xmin'].value;
			options.xaxes[0].max = SPEC.params.local['xmax'].value;
			options.yaxes[0].min = axes.yaxis.min;
			options.yaxes[0].max = axes.yaxis.max;

			plot.setupGrid();
			plot.draw();
			axes = plot.getAxes();

			$('#SPEC_TIME_SCALE').html(axes.xaxis.tickSize.toFixed(3));
		}
	}
  };

// Use only data for selected channels and do downsamling (data decimation), which is required for 
  // better performance. On the canvas cannot be shown too much graph points. 
  SPEC.filterData = function(dsets, points) {

 	var filtered = [];
    var num_of_channels = 2;

    for(var l=0; l<num_of_channels; l++) {
      var sig_name = "ch" + (l+1).toLocaleString();

	  if(SPEC.params.orig[sig_name.toUpperCase() + '_SHOW'] && SPEC.params.orig[sig_name.toUpperCase() + '_SHOW'].value == false) {       
		continue;
      }

      i = Math.min(l, dsets.length - 1);
	  if (i < 0)
		return [];
      filtered.push({ color: dsets[i].color, data: [] });
      
      if(SPEC.points_per_px === null || dsets[i].data.length > points * SPEC.points_per_px) {
        var step = Math.ceil(dsets[i].data.length / (points * SPEC.points_per_px));
        var k = 0;
        for(var j=0; j<dsets[i].data.length; j++) {
          if(k > 0 && ++k < step) {
            continue;
          }
          filtered[filtered.length - 1].data.push(dsets[i].data[j]);
          k = 1;
        }
      }
      else {
        filtered[filtered.length - 1].data = dsets[i].data.slice(0);
      }
    }
   
    return filtered;
  };

  SPEC.getLocalDecimalSeparator = function(){
    var n = 1.1;
    return n.toLocaleString().substring(1,2);
  };
  
  SPEC.floatToLocalString = function(num){
    // Workaround for a bug in Safari 6 (reference: https://github.com/mleibman/SlickGrid/pull/472)
    //return num.toString().replace('.', SPEC.getLocalDecimalSeparator());
    return (num + '').replace('.', SPEC.getLocalDecimalSeparator());
  };
  
  // Sets default values for cursors, if values not yet defined
  SPEC.setDefCursorVals = function() {

	var plot = SPEC.getPlot();
	if(!(SPEC.isVisibleChannels() && plot)){
		return;
	} 
	var offset = plot.getPlotOffset();
    var graph_height = $('#graph_grid').height() - offset.top - offset.bottom;
    var graph_width = $('#graph_grid').width() - offset.left - offset.right;
    
    var axes = plot.getAxes();

    var volt_per_px = 1/axes.yaxis.scale;
	
	SPEC.updateCursors();

    // Default value for Y1 cursor is 1/4 from graph height
    if(SPEC.params.local['SPEC_CURSOR_Y1'] && SPEC.params.local['SPEC_CURSOR_Y1'].value && SPEC.params.local['SPEC_CUR1_V'] === undefined && $('#cur_y1').data('init') === undefined) {
      var cur_arrow = $('#cur_y1_arrow');
      var top = (graph_height + 7) * 0.25;
    
      SPEC.params.local['SPEC_CUR1_V'] = { value: axes.yaxis.max - top*volt_per_px };
      $('#cur_y1_arrow, #cur_y1').css('top', top).show();
      $('#cur_y1').data('init', true);
    }
    
    // Default value for Y2 cursor is 1/3 from graph height
    if(SPEC.params.local['SPEC_CURSOR_Y2'] && SPEC.params.local['SPEC_CURSOR_Y2'].value && SPEC.params.local['SPEC_CUR2_V'] === undefined && $('#cur_y2').data('init') === undefined) {
      var cur_arrow = $('#cur_y2_arrow');
      var top = (graph_height + 7) * 0.33;
      
      SPEC.params.local['SPEC_CUR2_V'] = { value: axes.yaxis.max - top*volt_per_px}; 
      
      $('#cur_y2_arrow, #cur_y2').css('top', top).show();
      $('#cur_y2').data('init', true);
    }
    
	var ms_per_px = 1/axes.xaxis.scale;
    // Default value for X1 cursor is 1/4 from graph width
    if(SPEC.params.local['SPEC_CURSOR_X1'] && SPEC.params.local['SPEC_CURSOR_X1'].value && SPEC.params.local['SPEC_CUR1_T'] === undefined && $('#cur_x1').data('init') === undefined) {
      var cur_arrow = $('#cur_x1_arrow');
      var left = graph_width * 0.25;

      SPEC.params.local['SPEC_CUR1_T'] = { value: -axes.xaxis.min - left * ms_per_px };
      $('#cur_x1_arrow, #cur_x1').css('left', left).show();
      $('#cur_x1').data('init', true);
    }
    
    // Default value for X2 cursor is 1/3 from graph width
    if(SPEC.params.local['SPEC_CURSOR_X2'] && SPEC.params.local['SPEC_CURSOR_X2'].value && SPEC.params.local['SPEC_CUR2_T'] === undefined && $('#cur_x2').data('init') === undefined) {
      var cur_arrow = $('#cur_x2_arrow');
      var left = graph_width * 0.33;
   
      SPEC.params.local['SPEC_CUR2_T'] = { value: -axes.xaxis.min - left * ms_per_px };

      $('#cur_x2_arrow, #cur_x2').css('left', left).show();
      $('#cur_x2').data('init', true);
    }
  };
  
  // Updates all elements related to a Y cursor
  SPEC.updateYCursorElems = function(ui, save) {
    var y = (ui.helper[0].id == 'cur_y1_arrow' ? 'y1' : 'y2');

	var plot = SPEC.getPlot();
	if(!(SPEC.isVisibleChannels() && plot)){
		return;
	} 
	var axes = plot.getAxes();

    var volt_per_px = 1/axes.yaxis.scale;
    var new_value = axes.yaxis.max - ui.position.top*volt_per_px;
    $('#cur_' + y + ', #cur_' + y + '_info').css('top', ui.position.top);
    $('#cur_' + y + '_info').html((new_value.toFixed(Math.abs(new_value) >= 0.1 ? 2 : 3)) + 'dBm').css('margin-top', (ui.position.top < 16 ? 3 : ''));
    
    SPEC.updateYCursorDiff();
    
    if(save) {
      SPEC.params.local[y == 'y1' ? 'SPEC_CUR1_V' : 'SPEC_CUR2_V'] = { value: new_value };
      SPEC.sendParams();
    }
  };
  
  // Updates all elements related to a X cursor
  SPEC.updateXCursorElems = function(ui, save) {
    var x = (ui.helper[0].id == 'cur_x1_arrow' ? 'x1' : 'x2');
    var plot = SPEC.getPlot();
	if(!(SPEC.isVisibleChannels() && plot)){
		return;
	} 
	var axes = plot.getAxes();
	var offset = plot.getPlotOffset();
    
    var graph_width = $('#graph_grid').width() - offset.left - offset.right;
    var ms_per_px = 1/axes.xaxis.scale;
    var msg_width = $('#cur_' + x + '_info').outerWidth();
    var new_value = -axes.xaxis.min - ui.position.left * ms_per_px;

    $('#cur_' + x + ', #cur_' + x + '_info').css('left', ui.position.left);
	
	var unit = SPEC.freq_unit[SPEC.params.orig['freq_unit'].value];
    $('#cur_' + x + '_info')
      .html(-(new_value.toFixed(Math.abs(new_value) >= 0.1 ? 2 : Math.abs(new_value) >= 0.001 ? 4 : 6)) + unit)
      .css('margin-left', (ui.position.left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));
    
    SPEC.updateXCursorDiff();
    
    if(save) {
      SPEC.params.local[x == 'x1' ? 'SPEC_CUR1_T' : 'SPEC_CUR2_T'] = { value: new_value };
      SPEC.sendParams();
    }
  };
  
  // Resizes double-headed arrow showing the difference between Y cursors
  SPEC.updateYCursorDiff = function() {
    var y1 = $('#cur_y1');
    var y2 = $('#cur_y2');
    var y1_top = parseInt(y1.css('top'));
    var y2_top = parseInt(y2.css('top'));
    var diff_px = Math.abs(y1_top - y2_top) - 6;
    
    if(y1.is(':visible') && y2.is(':visible') && diff_px > 12) {
      var top = Math.min(y1_top, y2_top);
      var value = parseFloat($('#cur_y1_info').html()) - parseFloat($('#cur_y2_info').html());
      
      $('#cur_y_diff')
        .css('top', top + 5)
        .height(diff_px)
        .show();
      $('#cur_y_diff_info')
        .html(Math.abs(+(value.toFixed(Math.abs(value) >= 0.1 ? 2 : 3))) + 'dBm')
        .css('top', top + diff_px/2 - 2)
        .show();
    }
    else {
      $('#cur_y_diff, #cur_y_diff_info').hide();
    }
  };
  
  // Resizes double-headed arrow showing the difference between X cursors
  SPEC.updateXCursorDiff = function() {
    var x1 = $('#cur_x1');
    var x2 = $('#cur_x2');
    var x1_left = parseInt(x1.css('left'));
    var x2_left = parseInt(x2.css('left'));
    var diff_px = Math.abs(x1_left - x2_left) - 9;
    
    if(x1.is(':visible') && x2.is(':visible') && diff_px > 30) {
      var left = Math.min(x1_left, x2_left);
      var value = parseFloat($('#cur_x1_info').html()) - parseFloat($('#cur_x2_info').html());
      var unit = SPEC.freq_unit[SPEC.params.orig['freq_unit'].value];
      $('#cur_x_diff')
        .css('left', left + 1)
        .width(diff_px)
        .show();
      $('#cur_x_diff_info')
        .html(Math.abs(+(value.toFixed(Math.abs(value) >= 0.1 ? 2 : Math.abs(value) >= 0.001 ? 4 : 6))) + unit)
        .show()
        .css('left', left + diff_px/2 - $('#cur_x_diff_info').width()/2 + 3);
    }
    else {
      $('#cur_x_diff, #cur_x_diff_info').hide();
    }
  };

  SPEC.updateImg = function(img, hide){
	  if(hide && $('#wait_' + $(img).attr('id')).is(':visible')){
		$('#wait_' + $(img).attr('id')).hide();
		SPEC.config.graph_colors['ch1'] = '#f3ec1a'; // hide ch1
		return;
	  }
	  if(!hide && $('#wait_' + $(img).attr('id')).is(':hidden')){
		$('#wait_' + $(img).attr('id')).show();
	    SPEC.config.graph_colors['ch1'] = '#343433';
	  }
	  
  };
  
}(window.SPEC = window.SPEC || {}, jQuery));

// Page onload event handler
$(function() {
  
// Initialize FastClick to remove the 300ms delay between a physical tap and the firing of a click event on mobile browsers
  new FastClick(document.body);
  
  // Process clicks on top menu buttons
  $('#SPEC_RUN').on('click touchstart', function() {
    //ev.preventDefault();
    $('#SPEC_RUN').hide();
    $('#SPEC_STOP').css('display','block');
    SPEC.params.local['SPEC_RUN'] = { value: true };
    SPEC.sendParams();
  }); 
  
  $('#SPEC_STOP').on('click touchstart', function() {
    //ev.preventDefault();
    $('#SPEC_STOP').hide();
    $('#SPEC_RUN').show(); 
    SPEC.params.local['SPEC_RUN'] = { value: false };
    SPEC.sendParams();
  });
  
  $('#SPEC_SINGLE').on('click touchstart', function() {
    ev.preventDefault();
    SPEC.params.local['SPEC_SINGLE'] = { value: true };
    SPEC.sendParams();
  });
  
  $('#SPEC_AUTOSCALE').on('click touchstart', function() {
    ev.preventDefault();
    SPEC.params.local['SPEC_AUTOSCALE'] = { value: true };
    SPEC.sendParams();
  });

  // Opening a dialog for changing parameters
  $('.edit-mode').on('click touchstart', function() {
    SPEC.state.editing = true;
    $('#right_menu').hide();
    $('#' + $(this).attr('id') + '_dialog').show();  
  });
  
  // Close parameters dialog after Enter key is pressed
  $('input').keyup(function(event){
    if(event.keyCode == 13){
      SPEC.exitEditing();
    }
  });
  
  // Close parameters dialog on close button click
  $('.close-dialog').on('click touchstart', function() {
    SPEC.exitEditing();
  });
  
  // Measurement dialog
  $('#meas_done').on('click touchstart', function() {             
    var meas_signal = $('#meas_dialog input[name="meas_signal"]:checked');
    
    if(meas_signal.length) {
      var operator_name = $('#meas_operator option:selected').html();
      var operator_val = parseInt($('#meas_operator').val());
      var signal_name = meas_signal.val();
      var item_id = 'meas_' + operator_name + '_' + signal_name;
      
      // Check if the item already exists
      if($('#' + item_id).length > 0) {
        return;
      }
      
      // Add new item
      $('<div id="' + item_id + '" class="meas-item">' + operator_name + ' (' + signal_name + ')</div>').data({ 
        value: (signal_name == 'CH1' ? operator_val : (signal_name == 'CH2' ? operator_val + 1 : null)),  // Temporarily set null for MATH signal, because no param yet defined fot that signal
        operator: operator_name,
        signal: signal_name
      }).prependTo('#meas_list');
    }
  });

  $(document).on('click', '.meas-item', function() {
    $(this).remove();
  });
  
  // Process events from other controls in parameters dialogs
  $('#edge1').on('click touchstart', function() {
    $('#edge1').find('img').attr('src','img/edge1_active.png');
    $('#edge2').find('img').attr('src','img/edge2.png');
  });
  
  $('#edge2').on('click touchstart', function() {
    $('#edge2').find('img').attr('src','img/edge2_active.png');
    $('#edge1').find('img').attr('src','img/edge1.png');
  });
  
  // Joystick events
  $('#jtk_up').on('mousedown touchstart', function() { $('#jtk_btns').attr('src','img/node_up.png'); });
  $('#jtk_left').on('mousedown touchstart', function() { $('#jtk_btns').attr('src','img/node_left.png'); });
  $('#jtk_right').on('mousedown touchstart', function() { $('#jtk_btns').attr('src','img/node_right.png'); });
  $('#jtk_down').on('mousedown touchstart', function() { $('#jtk_btns').attr('src','img/node_down.png'); });
  $('#jtk_fine').on('mousedown touchstart', function() { $('#jtk_fine').attr('src','img/reset_active.png'); });
  

  $(document).on('mouseup touchend', function(){ 
    $('#jtk_btns').attr('src','img/node_fine.png'); 
	$('#jtk_fine').attr('src','img/reset.png');
  });
  
  $('#jtk_fine').on('click touchstart', function(ev) {
	SPEC.resetZoom();
  });
  
  $('#jtk_up, #jtk_down').on('click touchstart', function(ev) {
    SPEC.changeYZoom(ev.target.id == 'jtk_down' ? '-' : '+');
  });
  
  $('#jtk_left, #jtk_right').on('click touchstart', function(ev) {
    SPEC.changeXZoom(ev.target.id == 'jtk_left' ? '-' : '+');
  });
  
  // Y cursor arrows dragging
  $('#cur_y1_arrow, #cur_y2_arrow').draggable({
    axis: 'y',
    containment: 'parent',
    start: function(ev, ui) {
      SPEC.state.cursor_dragging = true;
    },
    drag: function(ev, ui) {
      SPEC.updateYCursorElems(ui, false);
    },
    stop: function(ev, ui) {
      SPEC.updateYCursorElems(ui, true);
      SPEC.state.cursor_dragging = false;
    }
  });
  
  // X cursor arrows dragging
  $('#cur_x1_arrow, #cur_x2_arrow').draggable({
    axis: 'x',
    containment: 'parent',
    start: function(ev, ui) {
      SPEC.state.cursor_dragging = true;
    },
    drag: function(ev, ui) {
      SPEC.updateXCursorElems(ui, false);
    },
    stop: function(ev, ui) {
      SPEC.updateXCursorElems(ui, true);
      SPEC.state.cursor_dragging = false;
    }
  });
  
  // Touch events
  $(document).on('touchstart', '.plot', function(ev) {
    ev.preventDefault();
    
    if(!SPEC.touch.start && ev.originalEvent.touches.length > 1) {
      SPEC.touch.zoom_axis = null;
      SPEC.touch.start = [
        { clientX: ev.originalEvent.touches[0].clientX, clientY: ev.originalEvent.touches[0].clientY }, 
        { clientX: ev.originalEvent.touches[1].clientX, clientY: ev.originalEvent.touches[1].clientY }
      ];
    }
  });
  
  $(document).on('touchmove', '.plot', function(ev) {
    ev.preventDefault();
    
    if(ev.originalEvent.touches.length < 2) {
      return;
    }
    
    SPEC.touch.curr = [
      { clientX: ev.originalEvent.touches[0].clientX, clientY: ev.originalEvent.touches[0].clientY }, 
      { clientX: ev.originalEvent.touches[1].clientX, clientY: ev.originalEvent.touches[1].clientY }
    ];
    
    // Find zoom axis
    if(! SPEC.touch.zoom_axis) {
      var delta_x = Math.abs(SPEC.touch.curr[0].clientX - SPEC.touch.curr[1].clientX);
      var delta_y = Math.abs(SPEC.touch.curr[0].clientY - SPEC.touch.curr[1].clientY);
      
      if(Math.abs(delta_x - delta_y) > 10) {
        if(delta_x > delta_y) {
          SPEC.touch.zoom_axis = 'x';
        }
        else if(delta_y > delta_x) {
          SPEC.touch.zoom_axis = 'y';
        }
      }
    }
    
    // Skip first touch event
    if(SPEC.touch.prev) {
      
      // Time zoom
      if(SPEC.touch.zoom_axis == 'x') {
        var prev_delta_x = Math.abs(SPEC.touch.prev[0].clientX - SPEC.touch.prev[1].clientX);
        var curr_delta_x = Math.abs(SPEC.touch.curr[0].clientX - SPEC.touch.curr[1].clientX);
        
        if(SPEC.state.fine || Math.abs(curr_delta_x - prev_delta_x) > $(this).width() * 0.9 / SPEC.time_steps.length) {
          var new_scale = SPEC.changeXZoom((curr_delta_x < prev_delta_x ? '-' : '+'), SPEC.touch.new_scale_x, false);
          
          if(new_scale !== null) {
            SPEC.touch.new_scale_x = new_scale;
            $('#new_scale').html('X scale: ' + new_scale);
          }
          
          SPEC.touch.prev = SPEC.touch.curr;
        }
      }
      // Voltage zoom
      else if(SPEC.touch.zoom_axis == 'y') {
        var prev_delta_y = Math.abs(SPEC.touch.prev[0].clientY - SPEC.touch.prev[1].clientY);
        var curr_delta_y = Math.abs(SPEC.touch.curr[0].clientY - SPEC.touch.curr[1].clientY);
        
        if(SPEC.state.fine || Math.abs(curr_delta_y - prev_delta_y) > $(this).height() * 0.9 / SPEC.voltage_steps.length) {
          var new_scale = SPEC.changeYZoom((curr_delta_y < prev_delta_y ? '-' : '+'), SPEC.touch.new_scale_y, false);
          
          if(new_scale !== null) {
            SPEC.touch.new_scale_y = new_scale;
            $('#new_scale').html('Y scale: ' + new_scale);
          }
          
          SPEC.touch.prev = SPEC.touch.curr;
        }
      }
    }
    else if(SPEC.touch.prev === undefined) {
      SPEC.touch.prev = SPEC.touch.curr;
    }
  });
  
  $(document).on('touchend', '.plot', function(ev) {
    ev.preventDefault();
    
    // Send new scale
    if(SPEC.touch.new_scale_y !== undefined) {
      SPEC.params.local['SPEC_' + SPEC.state.sel_sig_name.toUpperCase() + '_SCALE'] = { value: SPEC.touch.new_scale_y };
      SPEC.sendParams();
    }
    else if(SPEC.touch.new_scale_x !== undefined) {
      SPEC.params.local['SPEC_TIME_SCALE'] = { value: SPEC.touch.new_scale_x };
      SPEC.sendParams();
    }
    
    // Reset touch information
    SPEC.touch = {};
    $('#new_scale').empty();
  });

  // Prevent native touch activity like scrolling
  $('html, body').on('touchstart touchmove', function(ev) {
    ev.preventDefault();
  });
  
  // Preload images which are not visible at the beginning
  $.preloadImages = function() {
    for(var i = 0; i < arguments.length; i++) {
      $('<img />').attr('src', 'img/' + arguments[i]);
    }
  }
  $.preloadImages('edge1_active.png','edge2_active.png','node_up.png','node_left.png','node_right.png','node_down.png','fine_active.png');
  SPEC.drawGraphGrid();
  // Bind to the window resize event to redraw the graph; trigger that event to do the first drawing
  $(window).resize(function() {
    
    // Redraw the grid (it is important to do this before resizing graph holders)
    SPEC.drawGraphGrid();
    
    // Resize the graph holders
    $('.plot').css($('#graph_grid').css(['height','width']));
    
    // Hide all graphs, they will be shown next time signal data is received
    $('#graphs .plot').hide();
    
    // Hide all offset arrows
    $('.y-offset-arrow, #time_offset_arrow').hide();
    
	SPEC.updateCursors();

    // Set the resized flag
    SPEC.state.resized = true;
    
  }).resize();
  
  // Stop the application when page is unloaded
  window.onbeforeunload = function() {
    $.ajax({
      url: SPEC.config.stop_app_url,
      async: false
    });
  };
  
  // Everything prepared, start application
  SPEC.startApp();

});
