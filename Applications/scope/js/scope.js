/*
 * Red Pitaya Oscilloscope client
 *
 * Author: Dakus <info@eskala.eu>
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
*/

// App configuration
var config = {
  socket_url: 'ws://192.168.0.103:9002',    // WebSocket server URI
  socket_reconnect_timeout: 1000,           // Milliseconds
  graph_colors: {
    'ch1' : '#f3ec1a',
    'ch2' : '#31b44b',
    'out1': '#9595ca',
    'out2': '#ee3739',
    'math': '#ab4d9d',
    'trig': '#75cede'
  }
};

// App state
var state = {
  socket_opened: false,
  processing: false,
  editing: false,
  resized: false,
  sel_sig_name: null,
  fine: false
};

// Time scale steps in millisecods
var time_steps = [
  // Nanoseconds
  5/1000000, 10/1000000, 20/1000000, 50/1000000, 100/1000000, 200/1000000, 500/1000000,
  // Microseconds
  1/1000, 2/1000, 5/1000, 10/1000, 20/1000, 50/1000, 100/1000, 200/1000, 500/1000,
  // Millisecods
  1, 2, 5, 10, 20, 50, 100, 200, 500,
  // Seconds
  1*1000, 2*1000, 5*1000, 10*1000, 20*1000, 50*1000
];

// Params cache
var params = { 
  orig: {}, 
  local: {}
};

// Other global variables
var ws = null;
var graphs = {};

// Page onload event handler
$(function() {
  
  // Process clicks on top menu buttons
  $('#OSC_RUN').click(function(ev) {
    ev.preventDefault();
    $('#OSC_RUN').hide();
    $('#OSC_STOP').css('display','block');
    params.local['OSC_RUN'] = { value: true };
    sendParams();
  }); 
  
  $('#OSC_STOP').click(function(ev) {
    ev.preventDefault();
    $('#OSC_STOP').hide();
    $('#OSC_RUN').show(); 
    params.local['OSC_RUN'] = { value: false };
    sendParams();
  });
  
  $('#OSC_SINGLE').click(function(ev) {
    ev.preventDefault();
    params.local['OSC_SINGLE'] = { value: true };
    sendParams();
  });
  
  $('#OSC_AUTOSCALE').click(function(ev) {
    ev.preventDefault();
    params.local['OSC_AUTOSCALE'] = { value: true };
    sendParams();
  });
  
  // Selecting active signal
  $('.menu-btn').click(function() {
    $('#right_menu .menu-btn').not(this).removeClass('active');
    state.sel_sig_name = $(this).data('signal');
  });

  // Opening a dialog for changing parameters
  $('.edit-mode').click(function() {
    state.editing = true;
    $('#right_menu').hide();
    $('#' + $(this).attr('id') + '_dialog').show();  
  });
  
  // Close parameters dialog after Enter key is pressed
  $('input').keyup(function(event){
    if(event.keyCode == 13){
      exitEditing();
    }
  });
  
  // Close parameters dialog on close button click
  $('.close-dialog').click(function() {
    exitEditing();
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
  $('#edge1').click(function(){
    $('#edge1').find('img').attr('src','img/edge1_active.png');
    $('#edge2').find('img').attr('src','img/edge2.png');
  });
  
  $('#edge2').click(function(){
    $('#edge2').find('img').attr('src','img/edge2_active.png');
    $('#edge1').find('img').attr('src','img/edge1.png');
  });
  
  // Joystick events
  $('#jtk_up').mousedown(function(){ $('#jtk_btns').attr('src','img/node_up.png'); });
  $('#jtk_left').mousedown(function(){ $('#jtk_btns').attr('src','img/node_left.png'); });
  $('#jtk_right').mousedown(function(){ $('#jtk_btns').attr('src','img/node_right.png'); });
  $('#jtk_down').mousedown(function(){ $('#jtk_btns').attr('src','img/node_down.png'); });
  
  $('#jtk_fine').click(function(){
    var img = $('#jtk_fine');
    
    if(img.attr('src') == 'img/fine.png') {
      img.attr('src', 'img/fine_active.png');
      state.fine = true;
    }
    else {
      img.attr('src', 'img/fine.png');
      state.fine = false;
    }
  });

  $(document).mouseup(function(){ 
    $('#jtk_btns').attr('src','img/node_fine.png'); 
  });
  
  $('#jtk_up, #jtk_down').click(changeYOffset);
  $('#jtk_left, #jtk_right').click(changeXOffset);

  // Preload images which are not visible at the beginning
  $.preloadImages = function() {
    for(var i = 0; i < arguments.length; i++) {
      $('<img />').attr('src', 'img/' + arguments[i]);
    }
  }
  $.preloadImages('edge1_active.png','edge2_active.png','node_up.png','node_left.png','node_right.png','node_down.png','fine_active.png');
  
  // Bind to the window resize event to redraw the graph; trigger that event to do the first drawing
  $(window).resize(function() {
    
    // Redraw the grid (it is important to do this before resizing graph holders)
    drawGraphGrid();
    
    // Resize the graph holders
    $('.plot').css($('#graph_grid').css(['height','width']));
    
    // Hide all graphs, they will be shown next time signal data is received
    $('#graphs .plot').hide();
    
    // Set the resized flag
    state.resized = true;
    
  }).resize();
  
  // WebSocket connection with the web server
  connectWebSocket();

  // Define WebSocket event listeners
  if(ws) {
  
    ws.onopen = function() {
      state.socket_opened = true;
      console.log('Socket opened');
    };
    
    ws.onclose = function() {
      state.socket_opened = false;
      $('#graphs .plot').hide();  // Hide all graphs
      
      console.log('Socket closed. Trying to reopen in ' + config.socket_reconnect_timeout/1000 + ' sec...');
      
      // Try to reconnect after a defined timeout
      setTimeout(function() {
        ws = new WebSocket(config.socket_url);
      }, config.socket_reconnect_timeout);
    };
    
    ws.onerror = function(ev) {
      console.log('Websocket error: ', ev);
    };
    
    ws.onmessage = function(ev) {
      if(state.processing) {
        return;
      }
      state.processing = true;
      
      var receive = JSON.parse(ev.data);

      if(receive.parameters) {
        processParameters(receive.parameters);
      }
      
      if(receive.signals) {
        processSignals(receive.signals);
      }
      
      state.processing = false;
    };
  }

});

// Creates a WebSocket connection with the web server  
function connectWebSocket() {
  if(window.WebSocket) {
    ws = new WebSocket(config.socket_url);
  } 
  else if(window.MozWebSocket) {
    ws = new MozWebSocket(config.socket_url);
  } 
  else {
    console.log('Browser does not support WebSocket');
  }
}

// Processes newly received values for parameters
function processParameters(new_params) {
  
  for(var param_name in new_params) {
    // Do nothing if new parameter value is the same as the old one and is not related to measurement info
    if(params.orig[param_name] !== undefined && params.orig[param_name].value === new_params[param_name].value && param_name.indexOf('OSC_MEAS_VAL') == -1) {
      continue;
    }
    
    // Save data for new parameter
    params.orig[param_name] = new_params[param_name];
      
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
    else {
      var field = $('#' + param_name);
      
      // Do not change fields from dialogs when user is editing something        
      if(!state.editing || field.closest('#right_menu').length == 0) {
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
            radios.eq([new_params[param_name].value]).prop('checked', true).parent().addClass('active');
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
}

// Processes newly received data for signals
function processSignals(new_signals) {
  var visible_btns = [];
  var visible_plots = [];
  var visible_info = '';
  var start = +new Date();
  
  // Do nothing if no parameters received yet
  if($.isEmptyObject(params.orig)) {
    return;
  }
  
  // (Re)Draw every signal
  for(sig_name in new_signals) {
    
    // Ignore disabled signals
    if(params.orig[sig_name.toUpperCase() + '_SHOW'].value == false) {
      continue;
    }
    
    var points = [];
    var sig_btn = $('#right_menu .menu-btn.' + sig_name);
    var color = config.graph_colors[sig_name];
    
    for(var i=0; i<new_signals[sig_name].size; i++) {
      points.push([i, new_signals[sig_name].value[i]]);
    }
    
    if(graphs[sig_name]) {
      graphs[sig_name].elem.show();
      
      if(state.resized) {
        graphs[sig_name].plot.resize();
        graphs[sig_name].plot.setupGrid();
      }
      
      // TODO: Uncomment here and delete the TEMPLOT block below if/else.
      graphs[sig_name].plot.setData([points]);
      graphs[sig_name].plot.draw();
    }
    else {
      graphs[sig_name] = {};
      graphs[sig_name].elem = $('<div class="plot" />').css($('#graph_grid').css(['height','width'])).appendTo('#graphs');
      
      /*
      * TODO: Uncomment here and delete the TEMPLOT block below if/else.
      */
      graphs[sig_name].plot = $.plot(graphs[sig_name].elem, [points], {
        series: {
          shadowSize: 0,  // Drawing is faster without shadows
          color: color
        },
        yaxis: {
          min: params.orig['OSC_' + sig_name.toUpperCase() + '_SCALE'].value * -5,
          max: params.orig['OSC_' + sig_name.toUpperCase() + '_SCALE'].value * 5
        },
        xaxis: {
          min: 0
        },
        grid: {
          show: false
        }
      });
      
    }
    
    // TEMPLOT block start - testing
    /*
    graphs[sig_name].plot = $.plot(graphs[sig_name].elem, [points], {
      series: { shadowSize: 0, color: color },
      grid: { show: false }
    });
    */
    // TEMPLOT block end
    
    sig_btn.prop('disabled', false);
    visible_btns.push(sig_btn[0]);
    visible_plots.push(graphs[sig_name].elem[0]);
    visible_info += (visible_info.length ? ',' : '') + '.' + sig_name;
  }
  
  // Hide plots without signal
  $('#graphs .plot').not(visible_plots).hide();
  
  // Disable buttons related to inactive signals
  $('#right_menu .menu-btn').not(visible_btns).prop('disabled', true);
  
  // Show only information about active signals
  $('#info .info-title > span, #info .info-value > span').not(visible_info).hide();
  $('#info').find(visible_info).show();
  
  // Reset resize flag
  state.resized = false;
  
  // Check if selected signal is still visible 
  if(state.sel_sig_name && graphs[state.sel_sig_name] && !graphs[state.sel_sig_name].elem.is(':visible')) {
    $('#right_menu .menu-btn.active.' + state.sel_sig_name).removeClass('active');
    state.sel_sig_name = null;
  }
  
  console.log('Duration: ' + (+new Date() - start));
}

// Exits from editing mode
function exitEditing() {

  for(var key in params.orig) {
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
    
    if(value !== undefined && value != params.orig[key].value) {
      console.log(key + ' changed from ' + params.orig[key].value + ' to ' + ($.type(params.orig[key].value) == 'boolean' ? !!value : value));
      params.local[key] = { value: ($.type(params.orig[key].value) == 'boolean' ? !!value : value) };
    }
  }
  
  // Check changes in measurement list
  var mi_count = 0;
  $('#info-meas').empty();
  $('#meas_list .meas-item').each(function(index, elem) {
    var $elem = $(elem);
    var item_val = $elem.data('value');
    
    if(item_val !== null) {
      params.local['OSC_MEAS_SEL' + (++mi_count)] = { value: item_val };
      $('#info-meas').append(
        '<div>' + $elem.data('operator') + '(<span class="' + $elem.data('signal').toLowerCase() + '">' + $elem.data('signal') + '</span>) <span id="OSC_MEAS_VAL' + mi_count + '">-</span></div>'
      );
    }
  });
  
  // Send params then reset editing state and hide dialog
  sendParams();
  state.editing = false;
  $('.dialog:visible').hide();
  $('#right_menu').show(); 
}

// Sends to server modified parameters
function sendParams() {
  if($.isEmptyObject(params.local)) {
    return false;
  }
  
  if(! state.socket_opened) {
    console.log('ERROR: Cannot save changes, socket not opened');
    return false;
  }
  
  // TEMP TEST
  //params.local['DEBUG_PARAM_PERIOD'] = { value: 5000 };
  params.local['DEBUG_SIGNAL_PERIOD'] = { value: 100 };
  //params.local['OSC_CH1_SCALE'] = { value: 1 };
  //params.local['OSC_TIME_SCALE'] = { value: 2 };
  //params.local['OSC_CH1_PROBE'] = { value: 1 };
  
  ws.send(JSON.stringify({ parameters: params.local }));
  params.local = {};
  return true;
}

// Draws the grid on the lowest canvas layer
function drawGraphGrid() {
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
}

// Changes vertical offset for the selected signal
function changeYOffset(ev) {
  if(! state.sel_sig_name) {
    return;
  }
  
  if(state.sel_sig_name == 'ch1') {
    params.local['OSC_CH1_OFFSET'] = { value: params.orig['OSC_CH1_OFFSET'].value + (state.fine ? 0.1 : 1) * (ev.target.id == 'jtk_up' ? 1 : -1) };
    sendParams();
  }
  else if(state.sel_sig_name == 'ch2') {
    params.local['OSC_CH2_OFFSET'] = { value: params.orig['OSC_CH2_OFFSET'].value + (state.fine ? 0.1 : 1) * (ev.target.id == 'jtk_up' ? 1 : -1) };
    sendParams();
  }
  else {
    console.log('Offset change for ' + state.sel_sig_name + ' not yet supported');
  }
}

// Changes horizontal offset for all signals
// When fine state is enabled, change offset by 10% of scale, otherwise change by scale value
function changeXOffset(ev) {
  var diff = (state.fine ? params.orig['OSC_TIME_SCALE'].value/10 : params.orig['OSC_TIME_SCALE'].value);
  var offset = params.orig['OSC_TIME_OFFSET'].value + diff * (ev.target.id == 'jtk_left' ? -1 : 1);
  
  // When server sends OSC_TIME_SCALE = 0, new offset will be the same as old one, so no need to send it
  if(offset != params.orig['OSC_TIME_OFFSET'].value) {
    console.log('OSC_TIME_OFFSET: old = ' + params.orig['OSC_TIME_OFFSET'].value + ' | new = ' + offset);
    params.local['OSC_TIME_OFFSET'] = { value: offset };
    sendParams();
  }
}

