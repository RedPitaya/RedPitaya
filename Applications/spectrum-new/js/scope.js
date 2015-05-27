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
  SPEC.config.app_id = 'spectrum';
  SPEC.config.server_ip = '';  // Leave empty on production, it is used for testing only
  SPEC.config.start_app_url = (SPEC.config.server_ip.length ? 'http://' + SPEC.config.server_ip : '') + '/bazaar?start=' + SPEC.config.app_id;
  SPEC.config.stop_app_url = (SPEC.config.server_ip.length ? 'http://' + SPEC.config.server_ip : '') + '/bazaar?stop=' + SPEC.config.app_id;
//  SPEC.config.socket_url = 'ws://localhost:9002';
  SPEC.config.socket_url = 'ws://' + (SPEC.config.server_ip.length ? SPEC.config.server_ip : window.location.hostname) + ':9002';  // WebSocket server URI
  SPEC.config.socket_reconnect_timeout = 1000; // Milliseconds
  SPEC.config.graph_colors = {
    'ch1' : '#f3ec1a',
    'ch2' : '#31b44b'
  };
  SPEC.freq_unit = ['Hz', 'kHz', 'MHz'];
	
  SPEC.time_steps = [
    // Hz
    1, 2, 5, 10, 20, 50, 100, 200, 500,
    // KHz
    1*1000, 2*1000, 5*1000, 10*1000, 20*1000, 50*1000, 100*1000, 200*1000, 500*1000,
    // MHz
    1*1000000, 2*1000000, 5*1000000, 10*1000000, 20*1000000, 50*1000000, 100*1000000, 200*1000000, 500*1000000,
  ];
  
  // Voltage scale steps in volts
  //TODO power steps in dBm
  SPEC.voltage_steps = [
    // dBm
    1, 2, 5, 10, 20, 50, 100
  ];
  SPEC.points_per_px = 5;             // How many points per pixel should be drawn. Set null for unlimited (will disable client side decimation).  
  SPEC.scale_points_size = 10; 
  // App state
  SPEC.state = {
    socket_opened: false,
    processing: false,
    editing: false,
    resized: false,
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
    
    for(var param_name in new_params) {
      
      // Do nothing if new parameter value is the same as the old one, and it is not related to measurement info or offset
      if(SPEC.params.orig[param_name] !== undefined 
          && SPEC.params.orig[param_name].value === new_params[param_name].value
        //  && param_name.indexOf('SPEC_MEAS_VAL') == -1
        //  && param_name.indexOf('_OFFSET') == -1
		  )
	  {
        continue;
      }
      
      // Save data for new parameter
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
        var field = $('#' + param_name);

		if(param_name == 'peak1_unit')	
		{
			// 0 - Hz, 1 - kHz, 2 - MHz
			var freq_unit1 = (new_params['peak1_unit'].value == 1 ? 'k' : (new_params['peak1_unit'].value == 2 ? 'M' : '')) + 'Hz';
			
			$('#peak_ch1').val(SPEC.floatToLocalString(new_params['peak1_power'].value.toFixed(3)) + ' dBm @ ' + SPEC.floatToLocalString(new_params['peak1_freq'].value.toFixed(2)) + ' ' + freq_unit1);		
		}
		else if (param_name == 'peak2_unit')
		{
			// 0 - Hz, 1 - kHz, 2 - MHz
			var freq_unit2 = (new_params['peak2_unit'].value == 1 ? 'k' : (new_params['peak2_unit'].value == 2 ? 'M' : '')) + 'Hz';
			$('#peak_ch2').val(SPEC.floatToLocalString(new_params['peak2_power'].value.toFixed(3)) + ' dBm @ ' + SPEC.floatToLocalString(new_params['peak2_freq'].value.toFixed(2)) + ' ' + freq_unit2);
		}
		else if(param_name == 'w_idx')
		{
			// Update waterfall images
			var img_num = new_params['w_idx'].value;
			if(img_num <= 999) { 
			  img_num = ('00' + img_num).slice(-3); 
			}
		/////////////////////////TODO uncomment this to change spectrum img
		   // $('#waterfall_ch1').attr('src', config.waterf_img_path + 'wat1_' + img_num + '.jpg');
		   // $('#waterfall_ch2').attr('src', config.waterf_img_path + 'wat2_' + img_num + '.jpg');	
		}
		
        // Do not change fields from dialogs when user is editing something        
        if(!SPEC.state.editing || field.closest('.menu-content').length == 0) {
          if(field.is('select') || field.is('input:text')) {
            field.val(new_params[param_name].value);

			if(param_name == 'xmin' || param_name == 'xmax' || param_name == 'freq_unit')	
			{	
				SPEC.updateZoom();

				$('#freq_scale_unit').html(SPEC.freq_unit[new_params['freq_unit'].value]);
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
				$('#SPEC_TIME_SCALE').html(new_params[param_name].value.toFixed(6));
			}
          }
        }
      }
    }
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
	  ($('#CH1_FREEZE').hasClass('active') && SPEC.datasets[0] !== undefined ? SPEC.datasets[0] : null),
	  ($('#CH2_FREEZE').hasClass('active') && SPEC.datasets[1] !== undefined ? SPEC.datasets[1] : null)
	];

	SPEC.datasets = [];
	var sig_count = 0;
    // (Re)Draw every signal
    for(sig_name in new_signals) {
      sig_count++;
      // Ignore disabled signals
      if(SPEC.params.orig[sig_name.toUpperCase() + '_SHOW'] && SPEC.params.orig[sig_name.toUpperCase() + '_SHOW'].value == false) {
        continue;
      }
      
      var points = [];
      var sig_btn = $('#right_menu .menu-btn.' + sig_name);
      var color = SPEC.config.graph_colors[sig_name];
	  sig_btn.prop('disabled', false);
      visible_btns.push(sig_btn[0]);
      visible_info += (visible_info.length ? ',' : '') + '.' + sig_name;

	  if(frozen_dsets[sig_count-1]){
		points = frozen_dsets[sig_count-1].data;
	  }
	  else {
		for(var i = 0; i < new_signals[sig_name].size; i++) {
       	 	points.push([i, new_signals[sig_name].value[i]]);
      	}
	  }
	  SPEC.datasets.push({ color: color, data: points}); 
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
		var scale = SPEC.graphs.plot.getAxes().yaxis.tickSize;//SPEC.graphs.plot.getAxes().yaxis.delta.toFixed(1);
		$('#SPEC_CH1_SCALE').html(scale);
		$('#SPEC_CH2_SCALE').html(scale);
		SPEC.params.local['SPEC_CH1_SCALE'] = { value: scale };
		SPEC.params.local['SPEC_CH2_SCALE'] = { value: scale };
		SPEC.sendParams();

      }

      visible_plots.push(SPEC.graphs.elem[0]);
    
	  // Disable buttons related to inactive signals
	  $('#right_menu .menu-btn').not(visible_btns).prop('disabled', true);
		
		// Show only information about active signals
	  $('#info .info-title > span, #info .info-value > span').not(visible_info).hide();
      $('#info').find(visible_info).show();
	  $('.pull-right').show();
		// Reset resize flag
	  SPEC.state.resized = false;
		
	  //console.log('Duration: ' + (+new Date() - start));
	}
	else{
		$('.pull-right').hide();
	}
  };

  // Exits from editing mode
  SPEC.exitEditing = function() {

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
      
      if(value !== undefined && value != SPEC.params.orig[key].value) {
        console.log(key + ' changed from ' + SPEC.params.orig[key].value + ' to ' + ($.type(SPEC.params.orig[key].value) == 'boolean' ? !!value : value));
        SPEC.params.local[key] = { value: ($.type(SPEC.params.orig[key].value) == 'boolean' ? !!value : value) };
      }
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
    if(SPEC.state.sel_sig_name != 'ch1' && SPEC.state.sel_sig_name != 'ch2') {
      return;
    }
    
    var curr_scale = (curr_scale === undefined ? SPEC.params.orig['SPEC_' + SPEC.state.sel_sig_name.toUpperCase() + '_SCALE'].value : curr_scale);
    var new_scale;
    
    for(var i=0; i < SPEC.voltage_steps.length - 1; i++) {
      
      if(SPEC.state.fine && (curr_scale == SPEC.voltage_steps[i] || (curr_scale > SPEC.voltage_steps[i] && curr_scale < SPEC.voltage_steps[i + 1]))) {
          
        // Do not allow values smaller than the lowest possible one
        if(i != 0 || direction != '-') {
          
          // For millivolts, add one millivolt
          if(curr_scale < 1) {
            new_scale = curr_scale + 1/1000 * (direction == '-' ? -1 : 1);
          }
          // For volts, add one volt
          else {
            new_scale = curr_scale + (direction == '-' ? -1 : 1);
          }
        }
        break;
      }
      
      if(!SPEC.state.fine && curr_scale == SPEC.voltage_steps[i]) {
        new_scale = SPEC.voltage_steps[direction == '-' ? (i > 0 ? i - 1 : 0) : i + 1];
        break;
      }
      else if(!SPEC.state.fine && ((curr_scale > SPEC.voltage_steps[i] && curr_scale < SPEC.voltage_steps[i + 1]) || (curr_scale == SPEC.voltage_steps[i + 1] && i == SPEC.voltage_steps.length - 2))) {
        new_scale = SPEC.voltage_steps[direction == '-' ? i : i + 1]
        break;
      }
    }
    
    if(new_scale !== undefined && new_scale > 0 && new_scale != curr_scale) {
      
      // Fix float length
      new_scale = parseFloat(new_scale.toFixed(3));
      
      if(send_changes !== false) {
        SPEC.params.local['SPEC_' + SPEC.state.sel_sig_name.toUpperCase() + '_SCALE'] = { value: new_scale };
        SPEC.sendParams();
      }
      return new_scale;
    }
    
    return null;
  };

  // Changes X zoom/scale for all signals
  SPEC.changeXZoom = function(direction, curr_scale, send_changes) {
    var curr_scale = (curr_scale === undefined ? SPEC.params.orig['SPEC_TIME_SCALE'].value : curr_scale);
    var new_scale;
    
    for(var i=0; i < SPEC.time_steps.length - 1; i++) {
      
      if(SPEC.state.fine && (curr_scale == SPEC.time_steps[i] || (curr_scale > SPEC.time_steps[i] && curr_scale < SPEC.time_steps[i + 1]))) {
          
        // Do not allow values smaller than the lowest possible one
        if(i != 0 || direction != '-') {
          
          // For Hz, add one Hz
          if(curr_scale < 1000) {
            new_scale = curr_scale + 1* (direction == '-' ? -1 : 1);
          }
          // For KHz, add one KHz
          else if(curr_scale < 1000000) {
            new_scale = curr_scale + 1*1000 * (direction == '-' ? -1 : 1);
          }
          // For MHz, add one MHz
          else if(curr_scale >= 1000000) {
            new_scale = curr_scale + 1*1000000 * (direction == '-' ? -1 : 1);
          }
        }
        break;
      }
      
      if(!SPEC.state.fine && curr_scale == SPEC.time_steps[i]) {
        new_scale = SPEC.time_steps[direction == '-' ? (i > 0 ? i - 1 : 0) : i + 1];
        break;
      }
      else if(!SPEC.state.fine && ((curr_scale > SPEC.time_steps[i] && curr_scale < SPEC.time_steps[i + 1]) || (curr_scale == SPEC.time_steps[i + 1] && i == SPEC.time_steps.length - 2))) {
        new_scale = SPEC.time_steps[direction == '-' ? i : i + 1]
        break;
      }
    }
    
    if(new_scale !== undefined && new_scale > 0 && new_scale != curr_scale) {
      
      // Fix float length
      new_scale = parseFloat(new_scale.toFixed(6));
      
      if(send_changes !== false) {
        SPEC.params.local['SPEC_TIME_SCALE'] = { value: new_scale };
        SPEC.sendParams();
      }
      return new_scale;
    }
    
    return null;
  };
	
  SPEC.isVisibleChannels = function(){
	
	var is_visible = false;

	if((SPEC.params.orig['CH1_SHOW'] && SPEC.params.orig['CH1_SHOW'].value == true)  || 
		(SPEC.params.orig['CH2_SHOW'] && SPEC.params.orig['CH2_SHOW'].value == true))
		is_visible = true;
	return is_visible;
  };

  SPEC.updateZoom = function() {

	if(SPEC.graphs && SPEC.graphs.elem)
	{
		var plot_elem = SPEC.graphs.elem;
		if(SPEC.isVisibleChannels())   {

			var plot = SPEC.graphs.plot;
			SPEC.params.local['xmin'] = { value: SPEC.params.orig['xmin'].value };
			SPEC.params.local['xmax'] = { value: SPEC.params.orig['xmax'].value};
		  
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

}(window.SPEC = window.SPEC || {}, jQuery));

// Page onload event handler
$(function() {
  
  // Process clicks on top menu buttons
  $('#SPEC_RUN').click(function(ev) {
    ev.preventDefault();
    $('#SPEC_RUN').hide();
    $('#SPEC_STOP').css('display','block');
    SPEC.params.local['SPEC_RUN'] = { value: true };
    SPEC.sendParams();
  }); 
  
  $('#SPEC_STOP').click(function(ev) {
    ev.preventDefault();
    $('#SPEC_STOP').hide();
    $('#SPEC_RUN').show(); 
    SPEC.params.local['SPEC_RUN'] = { value: false };
    SPEC.sendParams();
  });
  
  $('#SPEC_SINGLE').click(function(ev) {
    ev.preventDefault();
    SPEC.params.local['SPEC_SINGLE'] = { value: true };
    SPEC.sendParams();
  });
  
  $('#SPEC_AUTOSCALE').click(function(ev) {
    ev.preventDefault();
    SPEC.params.local['SPEC_AUTOSCALE'] = { value: true };
    SPEC.sendParams();
  });
  
  // Selecting active signal
  $('.menu-btn').click(function() {
    $('#right_menu .menu-btn').not(this).removeClass('active');
    SPEC.state.sel_sig_name = $(this).data('signal');
    
    if(SPEC.state.sel_sig_name == 'ch1') {
      $('#ch1_offset_arrow').css('z-index', 11);
      $('#ch2_offset_arrow').css('z-index', 10);
    }
    else if(SPEC.state.sel_sig_name == 'ch2') {
      $('#ch2_offset_arrow').css('z-index', 11);
      $('#ch1_offset_arrow').css('z-index', 10);
    }
  });

  // Opening a dialog for changing parameters
  $('.edit-mode').click(function() {
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
  $('.close-dialog').click(function() {
    SPEC.exitEditing();
  });
  
  // Measurement dialog
  $('#meas_done').click(function() {              
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
  $('#edge1').click(function() {
    $('#edge1').find('img').attr('src','img/edge1_active.png');
    $('#edge2').find('img').attr('src','img/edge2.png');
  });
  
  $('#edge2').click(function() {
    $('#edge2').find('img').attr('src','img/edge2_active.png');
    $('#edge1').find('img').attr('src','img/edge1.png');
  });
  
  // Joystick events
  $('#jtk_up').on('mousedown touchstart', function() { $('#jtk_btns').attr('src','img/node_up.png'); });
  $('#jtk_left').on('mousedown touchstart', function() { $('#jtk_btns').attr('src','img/node_left.png'); });
  $('#jtk_right').on('mousedown touchstart', function() { $('#jtk_btns').attr('src','img/node_right.png'); });
  $('#jtk_down').on('mousedown touchstart', function() { $('#jtk_btns').attr('src','img/node_down.png'); });
  
  $('#jtk_fine').click(function(){
    var img = $('#jtk_fine');
    
    if(img.attr('src') == 'img/fine.png') {
      img.attr('src', 'img/fine_active.png');
      SPEC.state.fine = true;
    }
    else {
      img.attr('src', 'img/fine.png');
      SPEC.state.fine = false;
    }
  });

  $(document).on('mouseup touchend', function(){ 
    $('#jtk_btns').attr('src','img/node_fine.png'); 
  });
  
  $('#jtk_up, #jtk_down').click(function(ev) {
    SPEC.changeYZoom(ev.target.id == 'jtk_down' ? '-' : '+');
  });
  
  $('#jtk_left, #jtk_right').click(function(ev) {
    SPEC.changeXZoom(ev.target.id == 'jtk_left' ? '-' : '+');
  });
  
  // Voltage offset arrows dragging
  $('.y-offset-arrow').draggable({
    axis: 'y',
    containment: 'parent',
    drag: function(ev, ui) {
      var margin_top = parseInt(ui.helper.css('marginTop'));
      var min_top = ((ui.helper.height()/2) + margin_top) * -1;
      var max_top = $('#graphs').height() - margin_top;
      
      if(ui.position.top < min_top) {
        ui.position.top = min_top;
      }
      else if(ui.position.top > max_top) {
        ui.position.top = max_top;
      }
    },
    stop: function(ev, ui) {
      var zero_pos = ($('#graph_grid').outerHeight() + 7) / 2;
      
      if(ui.helper[0].id == 'ch1_offset_arrow') {
        var volt_per_px = (SPEC.params.orig['SPEC_CH1_SCALE'].value * 10) / $('#graph_grid').outerHeight();
        SPEC.params.local['SPEC_CH1_OFFSET'] = { value: (zero_pos - ui.position.top + parseInt($('#ch1_offset_arrow').css('margin-top')) / 2) * volt_per_px };
      }
      else {
        var volt_per_px = (SPEC.params.orig['SPEC_CH2_SCALE'].value * 10) / $('#graph_grid').outerHeight();
        SPEC.params.local['SPEC_CH2_OFFSET'] = { value: (zero_pos - ui.position.top + parseInt($('#ch1_offset_arrow').css('margin-top')) / 2) * volt_per_px };
      }
      
      SPEC.sendParams();
    }
  });
  
  // Voltage offset arrows dragging
  $('#time_offset_arrow').draggable({
    axis: 'x',
    containment: 'parent',
    stop: function(ev, ui) {
      var zero_pos = ($('#graph_grid').outerWidth() + 2) / 2;
      var ms_per_px = (SPEC.params.orig['SPEC_TIME_SCALE'].value * 10) / $('#graph_grid').outerWidth();
      
      SPEC.params.local['SPEC_TIME_OFFSET'] = { value: (zero_pos - ui.position.left - ui.helper.width()/2 - 1) * ms_per_px };
      SPEC.sendParams();
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
