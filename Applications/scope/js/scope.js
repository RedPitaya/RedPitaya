/*
 * Red Pitaya Oscilloscope client
 *
 * Author: Dakus <info@eskala.eu>
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
*/

(function(OSC, $, undefined) {

  // App configuration
  OSC.config = {};
  OSC.config.app_id = 'scope';
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
    5/1000000, 10/1000000, 20/1000000, 50/1000000, 100/1000000, 200/1000000, 500/1000000,
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
  
  // App state
  OSC.state = {
    socket_opened: false,
    processing: false,
    editing: false,
    trig_dragging: false,
    cursor_dragging: false,
    resized: false,
    sel_sig_name: null,
    fine: false
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
  OSC.connectWebSocket = function() {
    
    if(window.WebSocket) {
      OSC.ws = new WebSocket(OSC.config.socket_url);
    } 
    else if(window.MozWebSocket) {
      OSC.ws = new MozWebSocket(OSC.config.socket_url);
    } 
    else {
      console.log('Browser does not support WebSocket');
    }
    
    // Define WebSocket event listeners
    if(OSC.ws) {
    
      OSC.ws.onopen = function() {
        OSC.state.socket_opened = true;
        console.log('Socket opened');
      };
      
      OSC.ws.onclose = function() {
        OSC.state.socket_opened = false;
        $('#graphs .plot').hide();  // Hide all graphs
        console.log('Socket closed');
      };
      
      OSC.ws.onerror = function(ev) {
        console.log('Websocket error: ', ev);
      };
      
      OSC.ws.onmessage = function(ev) {
        if(OSC.state.processing) {
          return;
        }
        OSC.state.processing = true;
        
        var receive = JSON.parse(ev.data);

        if(receive.parameters) {
          OSC.processParameters(receive.parameters);
        }
        
        if(receive.signals) {
          OSC.processSignals(receive.signals);
        }
        
        OSC.state.processing = false;
      };
    }
  };

  // Processes newly received values for parameters
  OSC.processParameters = function(new_params) {
    var old_params = $.extend(true, {}, OSC.params.orig);
    
    for(var param_name in new_params) {
      
      // Save new parameter value
      OSC.params.orig[param_name] = new_params[param_name];
      
      // Run/Stop button
      if(param_name == 'OSC_RUN') {
        if(new_params[param_name].value === true) {
          $('#OSC_RUN').hide();
          $('#OSC_STOP').css('display','block');
        }
        else {
          $('#OSC_STOP').hide();
          $('#OSC_RUN').show();
        }
      }
      // Buffer position
      else if(param_name == 'OSC_VIEW_POS') {
        var full_width = $('#buffer').width() - 4;
        var visible_width = full_width * new_params['OSC_VIEV_PART'].value;
        
        $('#buffer .buf-red-line').width(visible_width).show();
        $('#buffer .buf-red-line-holder').css('left', full_width * new_params['OSC_VIEW_POS'].value - visible_width / 2);
      }
      // Buffer size parameter is processed upper
      else if(param_name == 'OSC_VIEV_PART') {
        continue;
      }
      // All other parameters
      else {
        
        // Show/hide Y offset arrows
        if(param_name == 'OSC_CH1_OFFSET') {
          if(new_params['CH1_SHOW'].value) {
            
            // Change arrow position only if arrow is hidden or old/new values are not the same
            if(!$('#ch1_offset_arrow').is(':visible') 
                || old_params[param_name].value != new_params[param_name].value 
                || old_params['OSC_CH1_SCALE'].value != new_params['OSC_CH1_SCALE'].value) {
              var volt_per_px = (new_params['OSC_CH1_SCALE'].value * 10) / $('#graph_grid').outerHeight();
              var px_offset = -(new_params['OSC_CH1_OFFSET'].value / volt_per_px - parseInt($('#ch1_offset_arrow').css('margin-top')) / 2);

              $('#ch1_offset_arrow').css('top', ($('#graph_grid').outerHeight() + 7) / 2 + px_offset).show();
            }
          }
          else {
            $('#ch1_offset_arrow').hide();
          }
        }
        else if(param_name == 'OSC_CH2_OFFSET') {
          if(new_params['CH2_SHOW'].value) {
            
            // Change arrow position only if arrow is hidden or old/new values are not the same
            if(!$('#ch2_offset_arrow').is(':visible') || old_params[param_name].value != new_params[param_name].value) {
              var volt_per_px = (new_params['OSC_CH2_SCALE'].value * 10) / $('#graph_grid').outerHeight();
              var px_offset = -(new_params['OSC_CH2_OFFSET'].value / volt_per_px - parseInt($('#ch2_offset_arrow').css('margin-top')) / 2);

              $('#ch2_offset_arrow').css('top', ($('#graph_grid').outerHeight() + 7) / 2 + px_offset).show();
            }
          }
          else {
            $('#ch2_offset_arrow').hide();
          }
        }
        // Time offset arrow
        else if(param_name == 'OSC_TIME_OFFSET') {
          
          // Change arrow position only if arrow is hidden or old/new values are not the same
          if(!$('#time_offset_arrow').is(':visible') || old_params[param_name].value != new_params[param_name].value) {
            var ms_per_px = (new_params['OSC_TIME_SCALE'].value * 10) / $('#graph_grid').outerWidth();
            var px_offset = -(new_params['OSC_TIME_OFFSET'].value / ms_per_px + $('#time_offset_arrow').width()/2 + 1);

            $('#time_offset_arrow').css('left', ($('#graph_grid').outerWidth() + 2) / 2 + px_offset).show();
          }
        }
        // Trigger level
        else if(param_name == 'OSC_TRIG_LEVEL') {
          if(! OSC.state.trig_dragging) {
            
            // Trigger button is blured out and trigger level is hidden for source 'EXT'
            if(new_params['OSC_TRIG_SOURCE'].value > 1) {
              $('#trigger_level, #trig_level_arrow').hide();
              $('#right_menu .menu-btn.trig').prop('disabled', true);
              $('#osc_trig_level_info').html('- ');
            }
            else {
              var ref_scale = (new_params['OSC_TRIG_SOURCE'].value == 0 ? 'OSC_CH1_SCALE' : 'OSC_CH2_SCALE');
              var source_offset = (new_params['OSC_TRIG_SOURCE'].value == 0 ? new_params['OSC_CH1_OFFSET'].value : new_params['OSC_CH2_OFFSET'].value);
              var graph_height = $('#graph_grid').outerHeight();
              var volt_per_px = (new_params[ref_scale].value * 10) / graph_height;
              var px_offset = -((new_params[param_name].value + source_offset) / volt_per_px - parseInt($('#trig_level_arrow').css('margin-top')) / 2);
              
              $('#trig_level_arrow, #trigger_level').css('top', (graph_height + 7) / 2 + px_offset).show();
              $('#right_menu .menu-btn.trig').prop('disabled', false);
              $('#osc_trig_level_info').html(new_params[param_name].value);
            }
          }
        }
        // Trigger source
        else if(param_name == 'OSC_TRIG_SOURCE') {
          $('#osc_trig_source_ch').html(new_params[param_name].value == 0 ? 'IN1' : (new_params[param_name].value == 1 ? 'IN2' : 'EXT'));
        }
        // Trigger edge/slope
        else if(param_name == 'OSC_TRIG_SLOPE') {
          $('#osc_trig_edge_img').attr('src', (new_params[param_name].value == 1 ? 'img/trig-edge-up.png' : 'img/trig-edge-down.png'));
        }
        // Y cursors
        else if(param_name == 'OSC_CURSOR_Y1' || param_name == 'OSC_CURSOR_Y2') {
          if(! OSC.state.cursor_dragging) {
            var y = (param_name == 'OSC_CURSOR_Y1' ? 'y1' : 'y2');
            
            if(new_params[param_name].value) {
              var new_value = new_params[y == 'y1' ? 'OSC_CUR1_V' : 'OSC_CUR2_V'].value;
              var ref_scale = (new_params['OSC_CURSOR_SRC'].value == 0 ? 'OSC_CH1_SCALE' : (new_params['OSC_CURSOR_SRC'].value == 1 ? 'OSC_CH2_SCALE' : 'OSC_MATH_SCALE'));
              var source_offset = new_params[new_params['OSC_CURSOR_SRC'].value == 0 ? 'OSC_CH1_OFFSET' : (new_params['OSC_CURSOR_SRC'].value == 1 ? 'OSC_CH2_OFFSET' : 'OSC_MATH_OFFSET')].value;
              var graph_height = $('#graph_grid').height();
              var volt_per_px = (new_params[ref_scale].value * 10) / graph_height;
              var px_offset = -((new_params[y == 'y1' ? 'OSC_CUR1_V' : 'OSC_CUR2_V'].value + source_offset) / volt_per_px - parseInt($('#cur_' + y + '_arrow').css('margin-top')) / 2);
              var top = (graph_height + 7) / 2 + px_offset;
              
              $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').css('top', top).show();
              $('#cur_' + y + '_info').html(+(new_value.toFixed(Math.abs(new_value) >= 0.1 ? 2 : 3)) + 'V').css('margin-top', (top < 16 ? 3 : ''));
            }
            else {
              $('#cur_' + y + '_arrow, #cur_' + y + ', #cur_' + y + '_info').hide();
            }
          }
        }
        // X cursors
        else if(param_name == 'OSC_CURSOR_X1' || param_name == 'OSC_CURSOR_X2') {
          if(! OSC.state.cursor_dragging) {
            var x = (param_name == 'OSC_CURSOR_X1' ? 'x1' : 'x2');
            
            if(new_params[param_name].value) {
              var new_value = new_params[x == 'x1' ? 'OSC_CUR1_T' : 'OSC_CUR2_T'].value;
              var graph_width = $('#graph_grid').width();
              var ms_per_px = (new_params['OSC_TIME_SCALE'].value * 10) / graph_width;
              var px_offset = -((new_value + new_params['OSC_TIME_OFFSET'].value) / ms_per_px - parseInt($('#cur_' + x + '_arrow').css('margin-left')) / 2 - 2.5);
              var msg_width = $('#cur_' + x + '_info').outerWidth();
              var left = (graph_width + 2) / 2 + px_offset;
              
              $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').css('left', left).show();
              $('#cur_' + x + '_info')
                .html(-(new_value.toFixed(Math.abs(new_value) >= 0.1 ? 2 : Math.abs(new_value) >= 0.001 ? 4 : 6)) + 'ms')
                .css('margin-left', (left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));
            }
            else {
              $('#cur_' + x + '_arrow, #cur_' + x + ', #cur_' + x + '_info').hide();
            }
          }
        }
        
        // Find the field having ID equal to current parameter name
        // TODO: Use classes instead of ids, to be able to use a param name in multiple fields and to loop through all fields to set new values
        var field = $('#' + param_name);
        
        // Do not change fields from dialogs when user is editing something or new parameter value is the same as the old one
        if((!OSC.state.editing || field.closest('.menu-content').length == 0) 
            && (old_params[param_name] === undefined || old_params[param_name].value !== new_params[param_name].value)) {
          
          if(field.is('select') || field.is('input:text')) {
            field.val(new_params[param_name].value);
          }
          else if(field.is('button')) {
            field[new_params[param_name].value === true ? 'addClass' : 'removeClass' ]('active');
          }
          else if(field.is('input:radio')) {
            var radios = $('input[name="' + param_name + '"]');
            
            radios.closest('.btn-group').children('.btn.active').removeClass('active');
            
            if(param_name == 'OSC_TRIG_SLOPE') {
              if(new_params[param_name].value == 0) {
                $('#edge1').find('img').attr('src','img/edge1.png');
                $('#edge2').addClass('active').find('img').attr('src','img/edge2_active.png').end().find('#OSC_TRIG_SLOPE1').prop('checked', true);
              }
              else {
                $('#edge1').addClass('active').find('img').attr('src','img/edge1_active.png').end().find('#OSC_TRIG_SLOPE').prop('checked', true);
                $('#edge2').find('img').attr('src','img/edge2.png');
              }
            }
            else {
              radios.eq([+new_params[param_name].value]).prop('checked', true).parent().addClass('active');
            }
          }
          else if(field.is('span')) {
            field.html(new_params[param_name].value);
          }
        }
        
        if(param_name == 'SOUR1_VOLT' || param_name == 'SOUR2_VOLT') {
          $('#' + param_name + '_info').html(new_params[param_name].value);
        }
      }
    }
    
    // Resize double-headed arrows showing the difference between cursors
    OSC.updateYCursorDiff();
    OSC.updateXCursorDiff();
  };

  // Processes newly received data for signals
  OSC.processSignals = function(new_signals) {
    var visible_btns = [];
    var visible_plots = [];
    var visible_info = '';
    //var start = +new Date();
    
    // Do nothing if no parameters received yet
    if($.isEmptyObject(OSC.params.orig)) {
      return;
    }
    
    // (Re)Draw every signal
    for(sig_name in new_signals) {
      
      // Ignore empty signals
      if(new_signals[sig_name].size == 0) {
        continue;
      }
      
      // Ignore disabled signals
      if(OSC.params.orig[sig_name.toUpperCase() + '_SHOW'] && OSC.params.orig[sig_name.toUpperCase() + '_SHOW'].value == false) {
        continue;
      }
      
      // Ignore math signal if no operator defined
      if(sig_name == 'math' && (!OSC.params.orig['OSC_MATH_OP'] || OSC.params.orig['OSC_MATH_OP'].value == 0)) {
        continue;
      }
      
      var points = [];
      var sig_btn = $('#right_menu .menu-btn.' + sig_name);
      var color = OSC.config.graph_colors[sig_name];
      
      for(var i=0; i<new_signals[sig_name].size; i++) {
        points.push([i, new_signals[sig_name].value[i]]);
      }
      
      if(OSC.graphs[sig_name]) {
        OSC.graphs[sig_name].elem.show();
        
        if(OSC.state.resized) {
          OSC.graphs[sig_name].plot.resize();
          OSC.graphs[sig_name].plot.setupGrid();
        }
        
        OSC.graphs[sig_name].plot.setData([points]);
        OSC.graphs[sig_name].plot.draw();
      }
      else {
        OSC.graphs[sig_name] = {};
        OSC.graphs[sig_name].elem = $('<div class="plot" />').css($('#graph_grid').css(['height','width'])).appendTo('#graphs');
        OSC.graphs[sig_name].plot = $.plot(OSC.graphs[sig_name].elem, [points], {
          series: {
            shadowSize: 0,  // Drawing is faster without shadows
            color: color
          },
          yaxis: {
            min: -5, // (sig_name == 'ch1' || sig_name == 'ch2' ? OSC.params.orig['OSC_' + sig_name.toUpperCase() + '_SCALE'].value * -5 : null),
            max: 5   // (sig_name == 'ch1' || sig_name == 'ch2' ? OSC.params.orig['OSC_' + sig_name.toUpperCase() + '_SCALE'].value * 5 : null)
          },
          xaxis: {
            min: 0
          },
          grid: {
            show: false
          }
        });
      }
      
      sig_btn.prop('disabled', false);
      visible_btns.push(sig_btn[0]);
      visible_plots.push(OSC.graphs[sig_name].elem[0]);
      visible_info += (visible_info.length ? ',' : '') + '.' + sig_name;
    }
    
    // Hide plots without signal
    $('#graphs .plot').not(visible_plots).hide();
    
    // Disable buttons related to inactive signals
    $('#right_menu .menu-btn').not(visible_btns).not('.not-signal').prop('disabled', true);
    
    // Show only information about active signals
    $('#info .info-title > span, #info .info-value > span').not(visible_info).hide();
    $('#info').find(visible_info).show();
    
    // Reset resize flag
    OSC.state.resized = false;
    
    // Check if selected signal is still visible 
    if(OSC.state.sel_sig_name && OSC.graphs[OSC.state.sel_sig_name] && !OSC.graphs[OSC.state.sel_sig_name].elem.is(':visible')) {
      $('#right_menu .menu-btn.active.' + OSC.state.sel_sig_name).removeClass('active');
      OSC.state.sel_sig_name = null;
    }
    
    //console.log('Duration: ' + (+new Date() - start));
  };

  // Exits from editing mode
  OSC.exitEditing = function() {

    for(var key in OSC.params.orig) {
      var field = $('#' + key);
      var value = undefined;

      if(key == 'OSC_RUN'){
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
      
      if(value !== undefined && value != OSC.params.orig[key].value) {
        console.log(key + ' changed from ' + OSC.params.orig[key].value + ' to ' + ($.type(OSC.params.orig[key].value) == 'boolean' ? !!value : value));
        OSC.params.local[key] = { value: ($.type(OSC.params.orig[key].value) == 'boolean' ? !!value : value) };
      }
    }
    
    // Check changes in measurement list
    var mi_count = 0;
    $('#info-meas').empty();
    $('#meas_list .meas-item').each(function(index, elem) {
      var $elem = $(elem);
      var item_val = $elem.data('value');
      
      if(item_val !== null) {
        OSC.params.local['OSC_MEAS_SEL' + (++mi_count)] = { value: item_val };
        $('#info-meas').append(
          '<div>' + $elem.data('operator') + '(<span class="' + $elem.data('signal').toLowerCase() + '">' + $elem.data('signal') + '</span>) <span id="OSC_MEAS_VAL' + mi_count + '">-</span></div>'
        );
      }
    });
    
    // Send params then reset editing state and hide dialog
    OSC.sendParams();
    OSC.state.editing = false;
    $('.dialog:visible').hide();
    $('#right_menu').show(); 
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
    
    // Send new values and reset the local params object
    OSC.ws.send(JSON.stringify({ parameters: OSC.params.local }));
    OSC.params.local = {};
    
    return true;
  };

  // Draws the grid on the lowest canvas layer
  OSC.drawGraphGrid = function() {
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
    ctx.strokeStyle = '#5d5d5c';

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
    ctx.strokeStyle = '#999';
    
    ctx.moveTo(center_x, 0);
    ctx.lineTo(center_x, canvas_height);
    
    ctx.moveTo(0, center_y);
    ctx.lineTo(canvas_width, center_y);
    
    ctx.stroke();
  };

  // Changes Y zoom/scale for the selected signal
  OSC.changeYZoom = function(direction, curr_scale, send_changes) {
    
    // Output 1/2 signals do not have offset
    // TODO: New parameters added OUTPUT1_SHOW_OFF and OUTPUT2_SHOW_OFF
    if($.inArray(OSC.state.sel_sig_name, ['ch1', 'ch2', 'math']) < 0) {
      return;
    }
    
    var curr_scale = (curr_scale === undefined ? OSC.params.orig['OSC_' + OSC.state.sel_sig_name.toUpperCase() + '_SCALE'].value : curr_scale);
    var new_scale;
    
    for(var i=0; i < OSC.voltage_steps.length - 1; i++) {
      
      if(OSC.state.fine && (curr_scale == OSC.voltage_steps[i] 
          || (curr_scale > OSC.voltage_steps[i] && curr_scale < OSC.voltage_steps[i + 1])
          || (curr_scale == OSC.voltage_steps[i + 1] && direction == '-'))) {
        
        new_scale = curr_scale + (OSC.voltage_steps[i + 1] / 100) * (direction == '-' ? -1 : 1);
        
        // Do not allow values smaller than the lowest possible one
        if(new_scale < OSC.voltage_steps[0]) {
          new_scale = OSC.voltage_steps[0];
        }
        
        break;
      }
      
      if(!OSC.state.fine && curr_scale == OSC.voltage_steps[i]) {
        new_scale = OSC.voltage_steps[direction == '-' ? (i > 0 ? i - 1 : 0) : i + 1];
        break;
      }
      else if(!OSC.state.fine && ((curr_scale > OSC.voltage_steps[i] && curr_scale < OSC.voltage_steps[i + 1]) || (curr_scale == OSC.voltage_steps[i + 1] && i == OSC.voltage_steps.length - 2))) {
        new_scale = OSC.voltage_steps[direction == '-' ? i : i + 1];
        break;
      }
    }
    
    if(new_scale !== undefined && new_scale > 0 && new_scale != curr_scale) {
      
      // Fix float length
      new_scale = parseFloat(new_scale.toFixed(OSC.state.fine ? 5 : 3));
      
      if(send_changes !== false) {
        OSC.params.local['OSC_' + OSC.state.sel_sig_name.toUpperCase() + '_SCALE'] = { value: new_scale };
        OSC.sendParams();
      }
      return new_scale;
    }
    
    return null;
  };

  // Changes X zoom/scale for all signals
  OSC.changeXZoom = function(direction, curr_scale, send_changes) {
    var curr_scale = (curr_scale === undefined ? OSC.params.orig['OSC_TIME_SCALE'].value : curr_scale);
    var new_scale;
    
    for(var i=0; i < OSC.time_steps.length - 1; i++) {
      
      if(OSC.state.fine && (curr_scale == OSC.time_steps[i] 
          || (curr_scale > OSC.time_steps[i] && curr_scale < OSC.time_steps[i + 1])
          || (curr_scale == OSC.time_steps[i + 1] && direction == '-'))) {
        
        new_scale = curr_scale + (OSC.time_steps[i + 1] / 100) * (direction == '-' ? -1 : 1);
        
        // Do not allow values smaller than the lowest possible one
        if(new_scale < OSC.time_steps[0]) {
          new_scale = OSC.time_steps[0];
        }
        
        break;
      }
      
      if(!OSC.state.fine && curr_scale == OSC.time_steps[i]) {
        new_scale = OSC.time_steps[direction == '-' ? (i > 0 ? i - 1 : 0) : i + 1];
        break;
      }
      else if(!OSC.state.fine && ((curr_scale > OSC.time_steps[i] && curr_scale < OSC.time_steps[i + 1]) || (curr_scale == OSC.time_steps[i + 1] && i == OSC.time_steps.length - 2))) {
        new_scale = OSC.time_steps[direction == '-' ? i : i + 1]
        break;
      }
    }
    
    if(new_scale !== undefined && new_scale > 0 && new_scale != curr_scale) {
      
      // Fix float length
      new_scale = parseFloat(new_scale.toFixed(OSC.state.fine ? 8 : 6));
      
      if(send_changes !== false) {
        OSC.params.local['OSC_TIME_SCALE'] = { value: new_scale };
        OSC.sendParams();
      }
      return new_scale;
    }
    
    return null;
  };

  // Sets default values for cursors, if values not yet defined
  OSC.setDefCursorVals = function() {
    var graph_height = $('#graph_grid').height();
    var graph_width = $('#graph_grid').width();
    
    var source = (OSC.params.local['OSC_CURSOR_SRC'] ? OSC.params.local['OSC_CURSOR_SRC'].value : OSC.params.orig['OSC_CURSOR_SRC']);
    var ref_scale = (source == 0 ? 'OSC_CH1_SCALE' : (source == 1 ? 'OSC_CH2_SCALE' : 'OSC_MATH_SCALE'));
    var volt_per_px = (OSC.params.orig[ref_scale].value * 10) / graph_height;
    
    // Default value for Y1 cursor is 1/4 from graph height
    if(OSC.params.local['OSC_CURSOR_Y1'] && OSC.params.local['OSC_CURSOR_Y1'].value && OSC.params.local['OSC_CUR1_V'] === undefined && $('#cur_y1').data('init') === undefined) {
      var cur_arrow = $('#cur_y1_arrow');
      var top = (graph_height + 7) * 0.25;
      
      OSC.params.local['OSC_CUR1_V'] = { value: (graph_height / 2 - top - (cur_arrow.height() - 2) / 2 - parseInt(cur_arrow.css('margin-top'))) * volt_per_px };
      
      $('#cur_y1_arrow, #cur_y1').css('top', top).show();
      $('#cur_y1').data('init', true);
    }
    
    // Default value for Y2 cursor is 1/3 from graph height
    if(OSC.params.local['OSC_CURSOR_Y2'] && OSC.params.local['OSC_CURSOR_Y2'].value && OSC.params.local['OSC_CUR2_V'] === undefined && $('#cur_y2').data('init') === undefined) {
      var cur_arrow = $('#cur_y2_arrow');
      var top = (graph_height + 7) * 0.33;
      
      OSC.params.local['OSC_CUR2_V'] = { value: (graph_height / 2 - top - (cur_arrow.height() - 2) / 2 - parseInt(cur_arrow.css('margin-top'))) * volt_per_px };
      
      $('#cur_y2_arrow, #cur_y2').css('top', top).show();
      $('#cur_y2').data('init', true);
    }
    
    // Default value for X1 cursor is 1/4 from graph width
    if(OSC.params.local['OSC_CURSOR_X1'] && OSC.params.local['OSC_CURSOR_X1'].value && OSC.params.local['OSC_CUR1_T'] === undefined && $('#cur_x1').data('init') === undefined) {
      var cur_arrow = $('#cur_x1_arrow');
      var left = graph_width * 0.25;
      var ms_per_px = (OSC.params.orig['OSC_TIME_SCALE'].value * 10) / graph_width;
      
      OSC.params.local['OSC_CUR1_T'] = { value: (graph_width / 2 - left - (cur_arrow.width() - 2) / 2 - parseInt(cur_arrow.css('margin-left'))) * ms_per_px };
      
      $('#cur_x1_arrow, #cur_x1').css('left', left).show();
      $('#cur_x1').data('init', true);
    }
    
    // Default value for X2 cursor is 1/3 from graph width
    if(OSC.params.local['OSC_CURSOR_X2'] && OSC.params.local['OSC_CURSOR_X2'].value && OSC.params.local['OSC_CUR2_T'] === undefined && $('#cur_x2').data('init') === undefined) {
      var cur_arrow = $('#cur_x2_arrow');
      var left = graph_width * 0.33;
      var ms_per_px = (OSC.params.orig['OSC_TIME_SCALE'].value * 10) / graph_width;
      
      OSC.params.local['OSC_CUR2_T'] = { value: (graph_width / 2 - left - (cur_arrow.width() - 2) / 2 - parseInt(cur_arrow.css('margin-left'))) * ms_per_px };
      
      $('#cur_x2_arrow, #cur_x2').css('left', left).show();
      $('#cur_x2').data('init', true);
    }
  };
  
  // Updates all elements related to a Y cursor
  OSC.updateYCursorElems = function(ui, save) {
    var y = (ui.helper[0].id == 'cur_y1_arrow' ? 'y1' : 'y2');
    var ref_scale = (OSC.params.orig['OSC_CURSOR_SRC'].value == 0 ? 'OSC_CH1_SCALE' : (OSC.params.orig['OSC_CURSOR_SRC'].value == 1 ? 'OSC_CH2_SCALE' : 'OSC_MATH_SCALE'));
    var source_offset = OSC.params.orig[OSC.params.orig['OSC_CURSOR_SRC'].value == 0 ? 'OSC_CH1_OFFSET' : (OSC.params.orig['OSC_CURSOR_SRC'].value == 1 ? 'OSC_CH2_OFFSET' : 'OSC_MATH_OFFSET')].value;
    var graph_height = $('#graph_grid').height();
    var volt_per_px = (OSC.params.orig[ref_scale].value * 10) / graph_height;
    var new_value = (graph_height / 2 - ui.position.top - (ui.helper.height() - 2) / 2 - parseInt(ui.helper.css('margin-top'))) * volt_per_px - source_offset;
    
    $('#cur_' + y + ', #cur_' + y + '_info').css('top', ui.position.top);
    $('#cur_' + y + '_info').html(+(new_value.toFixed(Math.abs(new_value) >= 0.1 ? 2 : 3)) + 'V').css('margin-top', (ui.position.top < 16 ? 3 : ''));
    
    OSC.updateYCursorDiff();
    
    if(save) {
      OSC.params.local[y == 'y1' ? 'OSC_CUR1_V' : 'OSC_CUR2_V'] = { value: new_value };
      OSC.sendParams();
    }
  };
  
  // Updates all elements related to a X cursor
  OSC.updateXCursorElems = function(ui, save) {
    var x = (ui.helper[0].id == 'cur_x1_arrow' ? 'x1' : 'x2');
    var graph_width = $('#graph_grid').width();
    var ms_per_px = (OSC.params.orig['OSC_TIME_SCALE'].value * 10) / graph_width;
    var msg_width = $('#cur_' + x + '_info').outerWidth();
    var new_value = (graph_width / 2 - ui.position.left - (ui.helper.width() - 2) / 2 - parseInt(ui.helper.css('margin-left'))) * ms_per_px - OSC.params.orig['OSC_TIME_OFFSET'].value;
    
    $('#cur_' + x + ', #cur_' + x + '_info').css('left', ui.position.left);
    $('#cur_' + x + '_info')
      .html(-(new_value.toFixed(Math.abs(new_value) >= 0.1 ? 2 : Math.abs(new_value) >= 0.001 ? 4 : 6)) + 'ms')
      .css('margin-left', (ui.position.left + msg_width > graph_width - 2 ? -msg_width - 1 : ''));
    
    OSC.updateXCursorDiff();
    
    if(save) {
      OSC.params.local[x == 'x1' ? 'OSC_CUR1_T' : 'OSC_CUR2_T'] = { value: new_value };
      OSC.sendParams();
    }
  };
  
  // Resizes double-headed arrow showing the difference between Y cursors
  OSC.updateYCursorDiff = function() {
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
        .html(Math.abs(+(value.toFixed(Math.abs(value) >= 0.1 ? 2 : 3))) + 'V')
        .css('top', top + diff_px/2 - 2)
        .show();
    }
    else {
      $('#cur_y_diff, #cur_y_diff_info').hide();
    }
  };
  
  // Resizes double-headed arrow showing the difference between X cursors
  OSC.updateXCursorDiff = function() {
    var x1 = $('#cur_x1');
    var x2 = $('#cur_x2');
    var x1_left = parseInt(x1.css('left'));
    var x2_left = parseInt(x2.css('left'));
    var diff_px = Math.abs(x1_left - x2_left) - 9;
    
    if(x1.is(':visible') && x2.is(':visible') && diff_px > 30) {
      var left = Math.min(x1_left, x2_left);
      var value = parseFloat($('#cur_x1_info').html()) - parseFloat($('#cur_x2_info').html());
      
      $('#cur_x_diff')
        .css('left', left + 1)
        .width(diff_px)
        .show();
      $('#cur_x_diff_info')
        .html(Math.abs(+(value.toFixed(Math.abs(value) >= 0.1 ? 2 : Math.abs(value) >= 0.001 ? 4 : 6))) + 'ms')
        .show()
        .css('left', left + diff_px/2 - $('#cur_x_diff_info').width()/2 + 3);
    }
    else {
      $('#cur_x_diff, #cur_x_diff_info').hide();
    }
  };
  
  // Updates Y offset in the signal config dialog, if opened, or saves new value
  OSC.updateYOffset = function(ui, save) {
    var graph_height = $('#graph_grid').outerHeight();
    var zero_pos = (graph_height + 7) / 2;
    var new_value;
    
    if(ui.helper[0].id == 'ch1_offset_arrow') {
      var volt_per_px = (OSC.params.orig['OSC_CH1_SCALE'].value * 10) / graph_height;
      
      new_value = (zero_pos - ui.position.top + parseInt(ui.helper.css('margin-top')) / 2) * volt_per_px;
      $('#info_box').html('New IN1 scale: ' + (+(new_value.toFixed(2))));
      
      if($('#in1_dialog').is(':visible')) {
        $('#OSC_CH1_OFFSET').val(+(new_value.toFixed(2)));
      }
      else if(save) {
        OSC.params.local['OSC_CH1_OFFSET'] = { value: new_value };
      }
    }
    else {
      var volt_per_px = (OSC.params.orig['OSC_CH2_SCALE'].value * 10) / graph_height;
      
      new_value = (zero_pos - ui.position.top + parseInt(ui.helper.css('margin-top')) / 2) * volt_per_px;
      $('#info_box').html('New IN2 scale: ' + (+(new_value.toFixed(2))));
      
      if($('#in2_dialog').is(':visible')) {
        $('#OSC_CH2_OFFSET').val(+(new_value.toFixed(2)));
      }
      else if(save) {
        OSC.params.local['OSC_CH2_OFFSET'] = { value: new_value };
      }
    }
    
    if(new_value !== undefined && save) {
      OSC.sendParams();
    }
  };
  
  // Updates trigger level in the trigger config dialog, if opened, or saves new value
  OSC.updateTrigLevel = function(ui, save) {    
  
    $('#trigger_level').css('top', ui.position.top);
    
    if(OSC.params.orig['OSC_TRIG_SOURCE'] !== undefined) {
      
      if(OSC.params.orig['OSC_TRIG_SOURCE'].value < 2) {
        var ref_scale = (OSC.params.orig['OSC_TRIG_SOURCE'].value == 0 ? 'OSC_CH1_SCALE' : 'OSC_CH2_SCALE');
        var source_offset = (OSC.params.orig['OSC_TRIG_SOURCE'].value == 0 ? OSC.params.orig['OSC_CH1_OFFSET'].value : OSC.params.orig['OSC_CH2_OFFSET'].value);
        
        if(OSC.params.orig[ref_scale] !== undefined) {
          var graph_height = $('#graph_grid').height();
          var volt_per_px = (OSC.params.orig[ref_scale].value * 10) / graph_height;
          var new_value = (graph_height / 2 - ui.position.top - (ui.helper.height() - 2) / 2 - parseInt(ui.helper.css('margin-top'))) * volt_per_px - source_offset;
          
          $('#info_box').html('New trigger level: ' + (+(new_value.toFixed(2))));
          
          if($('#trig_dialog').is(':visible')) {
            $('#OSC_TRIG_LEVEL').val(+(new_value.toFixed(2)));
          }
          else if(save) {
            OSC.params.local['OSC_TRIG_LEVEL'] = { value: new_value };
            OSC.sendParams();
          }
        }
      }
      else {
        console.log('Trigger level for source ' + OSC.params.orig['OSC_TRIG_SOURCE'].value + ' not yet supported');
      }
    }
  };
  
}(window.OSC = window.OSC || {}, jQuery));

// Page onload event handler
$(function() {
  
  // Initialize FastClick to remove the 300ms delay between a physical tap and the firing of a click event on mobile browsers
  new FastClick(document.body);
  
  // Process clicks on top menu buttons
  $('#OSC_RUN').on('click touchstart', function(ev) {
    ev.preventDefault();
    $('#OSC_RUN').hide();
    $('#OSC_STOP').css('display','block');
    OSC.params.local['OSC_RUN'] = { value: true };
    OSC.sendParams();
  }); 
  
  $('#OSC_STOP').on('click touchstart', function(ev) {
    ev.preventDefault();
    $('#OSC_STOP').hide();
    $('#OSC_RUN').show(); 
    OSC.params.local['OSC_RUN'] = { value: false };
    OSC.sendParams();
  });
  
  $('#OSC_SINGLE').on('click touchstart', function(ev) {
    ev.preventDefault();
    OSC.params.local['OSC_SINGLE'] = { value: true };
    OSC.sendParams();
  });
  
  $('#OSC_AUTOSCALE').on('click touchstart', function(ev) {
    ev.preventDefault();
    OSC.params.local['OSC_AUTOSCALE'] = { value: true };
    OSC.sendParams();
  });
  
  // Selecting active signal
  $('.menu-btn').on('click touchstart', function() {
    $('#right_menu .menu-btn').not(this).removeClass('active');
    OSC.state.sel_sig_name = $(this).data('signal');
    
    if(OSC.state.sel_sig_name == 'ch1') {
      $('#ch1_offset_arrow').css('z-index', 11);
      $('#ch2_offset_arrow').css('z-index', 10);
    }
    else if(OSC.state.sel_sig_name == 'ch2') {
      $('#ch2_offset_arrow').css('z-index', 11);
      $('#ch1_offset_arrow').css('z-index', 10);
    }
  });

  // Opening a dialog for changing parameters
  $('.edit-mode').on('click touchstart', function() {
    OSC.state.editing = true;
    $('#right_menu').hide();
    $('#' + $(this).attr('id') + '_dialog').show();  
  });
  
  // Close parameters dialog after Enter key is pressed
  $('input').keyup(function(event){
    if(event.keyCode == 13){
      OSC.exitEditing();
    }
  });
  
  // Close parameters dialog on close button click
  $('.close-dialog').on('click touchstart', function() {
    OSC.exitEditing();
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
        value: (signal_name == 'CH1' ? operator_val : (signal_name == 'CH2' ? operator_val + 1 : operator_val + 2)),
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
  
  $('#jtk_fine').on('click touchstart', function(){
    var img = $('#jtk_fine');
    
    if(img.attr('src') == 'img/fine.png') {
      img.attr('src', 'img/fine_active.png');
      OSC.state.fine = true;
    }
    else {
      img.attr('src', 'img/fine.png');
      OSC.state.fine = false;
    }
  });

  $(document).on('mouseup touchend', function(){ 
    $('#jtk_btns').attr('src','img/node_fine.png'); 
  });
  
  $('#jtk_up, #jtk_down').on('click touchstart', function(ev) {
    if(OSC.state.sel_sig_name == 'math') {
      OSC.changeYZoom(ev.target.id == 'jtk_down' ? '-' : '+');
    }
    else {
      OSC.changeYZoom(ev.target.id == 'jtk_down' ? '+' : '-');
    }
  });
  
  $('#jtk_left, #jtk_right').on('click touchstart', function(ev) {
    OSC.changeXZoom(ev.target.id == 'jtk_left' ? '+' : '-');
  });
  
  // Voltage offset arrow dragging
  $('.y-offset-arrow').draggable({
    axis: 'y',
    containment: 'parent',
    drag: function(ev, ui) {
      var margin_top = parseInt(ui.helper.css('marginTop'));
      var min_top = ((ui.helper.height() / 2) + margin_top) * -1;
      var max_top = $('#graphs').height() - margin_top;
      
      if(ui.position.top < min_top) {
        ui.position.top = min_top;
      }
      else if(ui.position.top > max_top) {
        ui.position.top = max_top;
      }
      
      OSC.updateYOffset(ui, false);
    },
    stop: function(ev, ui) {
      OSC.updateYOffset(ui, true);
      $('#info_box').empty();
    }
  });
  
  // Time offset arrow dragging
  $('#time_offset_arrow').draggable({
    axis: 'x',
    containment: 'parent',
    drag: function(ev, ui) {
      var graph_width = $('#graph_grid').outerWidth();
      var zero_pos = (graph_width + 2) / 2;
      var ms_per_px = (OSC.params.orig['OSC_TIME_SCALE'].value * 10) / graph_width;
      var new_value = (zero_pos - ui.position.left - ui.helper.width()/2 - 1) * ms_per_px;
      
      $('#info_box').html('New time offset: ' + (+(new_value.toFixed(2))));
    },
    stop: function(ev, ui) {
      var graph_width = $('#graph_grid').outerWidth();
      var zero_pos = (graph_width + 2) / 2;
      var ms_per_px = (OSC.params.orig['OSC_TIME_SCALE'].value * 10) / graph_width;
      
      OSC.params.local['OSC_TIME_OFFSET'] = { value: (zero_pos - ui.position.left - ui.helper.width()/2 - 1) * ms_per_px };
      OSC.sendParams();
      $('#info_box').empty();
    }
  });
  
  // Trigger level arrow dragging
  $('#trig_level_arrow').draggable({
    axis: 'y',
    containment: 'parent',
    start: function(ev, ui) {
      OSC.state.trig_dragging = true;
    },
    drag: function(ev, ui) {
      OSC.updateTrigLevel(ui, false);
    },
    stop: function(ev, ui) {
      OSC.updateTrigLevel(ui, true);
      OSC.state.trig_dragging = false;
      $('#info_box').empty();
    }
  });
  
  // Y cursor arrows dragging
  $('#cur_y1_arrow, #cur_y2_arrow').draggable({
    axis: 'y',
    containment: 'parent',
    start: function(ev, ui) {
      OSC.state.cursor_dragging = true;
    },
    drag: function(ev, ui) {
      OSC.updateYCursorElems(ui, false);
    },
    stop: function(ev, ui) {
      OSC.updateYCursorElems(ui, true);
      OSC.state.cursor_dragging = false;
    }
  });
  
  // X cursor arrows dragging
  $('#cur_x1_arrow, #cur_x2_arrow').draggable({
    axis: 'x',
    containment: 'parent',
    start: function(ev, ui) {
      OSC.state.cursor_dragging = true;
    },
    drag: function(ev, ui) {
      OSC.updateXCursorElems(ui, false);
    },
    stop: function(ev, ui) {
      OSC.updateXCursorElems(ui, true);
      OSC.state.cursor_dragging = false;
    }
  });
  
  // Touch events
  $(document).on('touchstart', '.plot', function(ev) {
    ev.preventDefault();
    
    if(!OSC.touch.start && ev.originalEvent.touches.length > 1) {
      OSC.touch.zoom_axis = null;
      OSC.touch.start = [
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
    
    OSC.touch.curr = [
      { clientX: ev.originalEvent.touches[0].clientX, clientY: ev.originalEvent.touches[0].clientY }, 
      { clientX: ev.originalEvent.touches[1].clientX, clientY: ev.originalEvent.touches[1].clientY }
    ];
    
    // Find zoom axis
    if(! OSC.touch.zoom_axis) {
      var delta_x = Math.abs(OSC.touch.curr[0].clientX - OSC.touch.curr[1].clientX);
      var delta_y = Math.abs(OSC.touch.curr[0].clientY - OSC.touch.curr[1].clientY);
      
      if(Math.abs(delta_x - delta_y) > 10) {
        if(delta_x > delta_y) {
          OSC.touch.zoom_axis = 'x';
        }
        else if(delta_y > delta_x) {
          OSC.touch.zoom_axis = 'y';
        }
      }
    }
    
    // Skip first touch event
    if(OSC.touch.prev) {
      
      // Time zoom
      if(OSC.touch.zoom_axis == 'x') {
        var prev_delta_x = Math.abs(OSC.touch.prev[0].clientX - OSC.touch.prev[1].clientX);
        var curr_delta_x = Math.abs(OSC.touch.curr[0].clientX - OSC.touch.curr[1].clientX);
        
        if(OSC.state.fine || Math.abs(curr_delta_x - prev_delta_x) > $(this).width() * 0.9 / OSC.time_steps.length) {
          var new_scale = OSC.changeXZoom((curr_delta_x < prev_delta_x ? '-' : '+'), OSC.touch.new_scale_x, false);
          
          if(new_scale !== null) {
            OSC.touch.new_scale_x = new_scale;
            $('#info_box').html('X scale: ' + new_scale);
          }
          
          OSC.touch.prev = OSC.touch.curr;
        }
      }
      // Voltage zoom
      else if(OSC.touch.zoom_axis == 'y' && OSC.state.sel_sig_name) {
        var prev_delta_y = Math.abs(OSC.touch.prev[0].clientY - OSC.touch.prev[1].clientY);
        var curr_delta_y = Math.abs(OSC.touch.curr[0].clientY - OSC.touch.curr[1].clientY);
        
        if(OSC.state.fine || Math.abs(curr_delta_y - prev_delta_y) > $(this).height() * 0.9 / OSC.voltage_steps.length) {
          var new_scale = OSC.changeYZoom((curr_delta_y < prev_delta_y ? '+' : '-'), OSC.touch.new_scale_y, false);
          
          if(new_scale !== null) {
            OSC.touch.new_scale_y = new_scale;
            $('#info_box').html('Y scale: ' + new_scale);
          }
          
          OSC.touch.prev = OSC.touch.curr;
        }
      }
    }
    else if(OSC.touch.prev === undefined) {
      OSC.touch.prev = OSC.touch.curr;
    }
  });
  
  $(document).on('touchend', '.plot', function(ev) {
    ev.preventDefault();
    
    // Send new scale
    if(OSC.touch.new_scale_y !== undefined) {
      OSC.params.local['OSC_' + OSC.state.sel_sig_name.toUpperCase() + '_SCALE'] = { value: OSC.touch.new_scale_y };
      OSC.sendParams();
    }
    else if(OSC.touch.new_scale_x !== undefined) {
      OSC.params.local['OSC_TIME_SCALE'] = { value: OSC.touch.new_scale_x };
      OSC.sendParams();
    }
    
    // Reset touch information
    OSC.touch = {};
    $('#info_box').empty();
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
  $.preloadImages(
    'edge1_active.png',
    'edge2_active.png',
    'node_up.png',
    'node_left.png',
    'node_right.png',
    'node_down.png',
    'fine_active.png',
    'trig-edge-up.png',
    'trig-edge-down.png'
  );
  
  // Bind to the window resize event to redraw the graph; trigger that event to do the first drawing
  $(window).resize(function() {
    
    // Redraw the grid (it is important to do this before resizing graph holders)
    OSC.drawGraphGrid();
    
    // Resize the graph holders
    $('.plot').css($('#graph_grid').css(['height','width']));
    
    // Hide all graphs, they will be shown next time signal data is received
    $('#graphs .plot').hide();
    
    // Hide offset arrows, trigger level line and arrow
    $('.y-offset-arrow, #time_offset_arrow, #trig_level_arrow, #trigger_level').hide();
    
    // Reset left position for trigger level arrow, it is added by jQ UI draggable
    $('#trig_level_arrow').css('left', '');
    
    // Set the resized flag
    OSC.state.resized = true;
    
  }).resize();
  
  // Stop the application when page is unloaded
  window.onbeforeunload = function() {
    $.ajax({
      url: OSC.config.stop_app_url,
      async: false
    });
  };
  
  // Everything prepared, start application
  OSC.startApp();

});
