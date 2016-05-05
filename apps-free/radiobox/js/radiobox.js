/*
 * Red Pitaya RadioBox client
 *
 * Author: Ulrich Habel (DF4IAH) <espero7757@gmx.net>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
*/

(function() {
  var originalAddClassMethod = jQuery.fn.addClass;
  var originalRemoveClassMethod = jQuery.fn.removeClass;
  $.fn.addClass = function(clss) {
    var result = originalAddClassMethod.apply(this, arguments);
    $(this).trigger('activeChanged', 'add');
    return result;
  };
  $.fn.removeClass = function(clss) {
    var result = originalRemoveClassMethod.apply(this, arguments);
    $(this).trigger('activeChanged', 'remove');
    return result;
  }
})();


(function(RB, $, undefined) {
  // App configuration
  RB.config = {};
  RB.config.app_id = 'radiobox';
  RB.config.server_ip = '';  // Leave empty on production, it is used for testing only
  var root_url = (RB.config.server_ip.length ? 'http://' + RB.config.server_ip : '');
  RB.config.start_app_url = root_url + '/bazaar?start=' + RB.config.app_id + '?' + location.search.substr(1);
  RB.config.stop_app_url = root_url + '/bazaar?stop=' + RB.config.app_id;
  RB.config.get_url = root_url + '/data';
  RB.config.post_url = root_url + '/data';
  //RB.config.socket_url = 'ws://' + (RB.config.server_ip.length ? RB.config.server_ip : window.location.hostname) + ':9002';  // WebSocket server URI
  RB.config.request_timeout = 3000;

  // App state
  RB.state = {
    app_started: false,
    socket_opened: false,
    pktIdx: 0,
    pktIdxMax: 4,  // XXX set count of transport frames
    mouseWheelLim: 20,
    doUpdate: false,
    blocking: false,
    sending: false,
    send_que: false,
    receiving: false,
    processing: false,
    editing: false,
    resized: false,
    eventLast: {
        id: ""
    },
    eventClickId: '',
    qrgController: {
        tx: {
            button_checked: true
        },
        rx: {
            button_checked: true
        },
        mousewheelsum: 0,
        digit: {
            e: [ 0, 0, 0, 0, 0, 1, 0, 0 ]  // reversed digits
        },
        editing: false,
        enter: false,
        frequency: 0.0
    }
  };

  // Params cache
  RB.params = {
    orig: {},
    local: {},
    init: {}
  };
  RB.params.init = {            // XXX initital data
    rb_run:                 1,  // application running
    tx_modsrc_s:            1,  // mod-source: RF Input 1 (audio signal)
    tx_modtyp_s:            1,  // TX modulation: off
    rx_modtyp_s:            5,  // RX modulation: AM-sync USB
    rbled_csp_s:          254,  // RB LEDs set to: current status of the overdrive signals
    rfout1_csp_s:          28,  // connect to TX_AMP_RF out (TX: RF signal)
    rfout2_csp_s:          80,  // connect to RX_AUDIO_OUT (RX: audio signal)
    rx_muxin_src_s:         2,  // receiver RF input set to RF Input 2

    tx_car_osc_qrg_f:  100000,  // 100 kHz
    rx_car_osc_qrg_f:  100000,  // 100 kHz

    tx_mod_osc_qrg_f:    1000,  //   1 kHz
    tx_muxin_gain_s:       80,  // slider position in % of 100% (80% = FS input with booster 1:1)
    rx_muxin_gain_s:       80,  // slider position in % of 100% (80% = FS input with booster 1:1)
    tx_qrg_sel_s:           1,  // QRG controller influence TX frequency
    rx_qrg_sel_s:           1,  // QRG controller influence RX frequency

    tx_amp_rf_gain_s:     200,  // 200 mV Vpp @ 50R results to -10 dBm
    tx_mod_osc_mag_s:     100,  // 100 % modulation by default
    rfout1_term_s:          2,  // RF Output 1: '0' neutral, '1' 50 ohms terminated, '2' open ended
    rfout2_term_s:          2,  // RF Output 2: '0' neutral, '1' 50 ohms terminated, '2' open ended
    qrg_inc_s:             50,  // Frequency range controller increment value [0%..100%]
    ovrdrv_s:               0   // Current overdrive flags of the FPGA signals
  };

  // Other global variables
  RB.ac = null;
  //RB.ws = null;
  RB.touch = {};

  RB.connect_time;

  // Starts the application on server
  RB.startApp = function() {
    $.get(
      RB.config.start_app_url
    )
    .done(function(dresult) {
      if (dresult.status == 'OK') {
        RB.ac();
        //RB.connectWebSocket();
      }
      else if (dresult.status == 'ERROR') {
        console.log(dresult.reason ? dresult.reason : 'Failure returned when connecting the web-server - can not start the application (ERR1)');
      }
      else {
        console.log('Unknown connection state - can not start the application (ERR2)');
      }
    })
    .fail(function() {
      console.log('Can not connect the web-server (ERR3)');
    });
  };

  function showModalError(err_msg, retry_btn, restart_btn, ignore_btn) {
    var err_modal = $('#modal_err');
    err_modal.find('#btn_retry_get')[retry_btn ? 'show' : 'hide']();
    err_modal.find('.btn-app-restart')[restart_btn ? 'show' : 'hide']();
    err_modal.find('#btn_ignore')[ignore_btn ? 'show' : 'hide']();
    err_modal.find('.modal-body').html(err_msg);
    err_modal.modal('show');
  }

  // Initial Ajax Connection set-up
  RB.ac = function() {
    // init the RB.params.orig parameter list
    RB.params.orig = $.extend(true, {}, RB.params.init);

    RB.state.sending = true;
    RB.state.pktIdx = 1;
    while (RB.state.pktIdx <= RB.state.pktIdxMax) {
      $.post(
        RB.config.post_url,
        JSON.stringify({ datasets: { params: cast_params2transport(RB.params.orig, RB.state.pktIdx) } })
      )
      .done(function(dresult) {
        RB.state.socket_opened = true;
        RB.state.app_started = true;
        RB.parsePacket(dresult);
      })
      .fail(function() {
        showModalError('Can not initialize the application with the default parameters.', false, true);
      });

      RB.state.pktIdx++;
    }  // while()

    setTimeout(function() {
      RB.params.local = {};
      RB.state.pktIdx = 0;
      RB.state.sending = false;
      RB.state.blocking = false;
    }, 600);
  }

  /*
  // Creates a WebSocket connection with the web server
  RB.connectWebSocket = function() {

    if (window.WebSocket) {
      RB.ws = new WebSocket(RB.config.socket_url);
    }
    else if (window.MozWebSocket) {
      RB.ws = new MozWebSocket(RB.config.socket_url);
    }
    else {
      console.log('Browser does not support WebSocket');
    }

    // Define WebSocket event listeners
    if (RB.ws) {
      RB.ws.onopen = function() {
        RB.state.socket_opened = true;
        console.log('Socket opened');

        RB.params.local['in_command'] = { value: 'send_all_params' };
        RB.ws.send(JSON.stringify({ parameters: RB.params.local }));
        RB.params.local = {};
      };

      RB.ws.onclose = function() {
        RB.state.socket_opened = false;
        console.log('Socket closed');
      };

      RB.ws.onerror = function(ev) {
        console.log('Websocket error: ', ev);
      };

      RB.ws.onmessage = function(ev) {
        if (RB.state.processing) {
          return;
        }
        RB.state.processing = true;

        var receive = JSON.parse(ev.data);

        if (receive.parameters) {
          if ((Object.keys(RB.params.orig).length == 0) && (Object.keys(receive.parameters).length == 0)) {
            RB.params.local['in_command'] = { value: 'send_all_params' };
            RB.ws.send(JSON.stringify({ parameters: RB.params.local }));
            RB.params.local = {};
          } else {
            RB.processParameters(receive.parameters);
          }
        }

        if (receive.signals) {
          RB.processSignals(receive.signals);
        }

        RB.state.processing = false;
      };
    }
  };
  */


  /* Back-end (server) to front-end communication */

  // Parse returned data result from last POST transfer
  RB.parsePacket = function(dresult) {
    if (dresult.datasets !== undefined) {
      if (dresult.datasets.params !== undefined) {
        RB.processParameters(dresult.datasets.params);
      }
      if (dresult.datasets.signals !== undefined) {
        RB.processSignals(dresult.datasets.signals);
      }
    }
  };

  // Processes newly received values for parameters
  RB.processParameters = function(new_params_transport) {
    var new_params = cast_transport2params(new_params_transport);
    var old_params = $.extend(true, {}, RB.params.orig);
    var send_all_params = Object.keys(new_params).indexOf('send_all_params') != -1;

    var isTxQrgSel = false;
    if (new_params['tx_qrg_sel_s'] !== undefined) {
      if (new_params['tx_qrg_sel_s']) {
        isTxQrgSel = true;
      }
    } else if (old_params['tx_qrg_sel_s'] !== undefined) {
      if (old_params['tx_qrg_sel_s']) {
        isTxQrgSel = true;
      }
    }

    var isRxQrgSel = false;
    if (new_params['rx_qrg_sel_s'] !== undefined) {
      if (new_params['rx_qrg_sel_s']) {
        isRxQrgSel = true;
      }
    } else if (old_params['rx_qrg_sel_s'] !== undefined) {
      if (old_params['rx_qrg_sel_s']) {
        isRxQrgSel = true;
      }
    }

    for (var param_name in new_params) {  // XXX receiving data from the back-end
      // Save new parameter value
      RB.params.orig[param_name] = new_params[param_name];
      var intVal = parseInt(RB.params.orig[param_name]);
      var dblVal = parseFloat(RB.params.orig[param_name]);
      dblVal = Math.floor(dblVal * 1e+7 + 0.5) / 1e+7;

      // console.log("CHECK RB.processParameters: param_name='" + param_name + "', content_old='" + old_params[param_name] + "', content_new='" + new_params[param_name] + "', dblVal=" + dblVal);

      if (param_name == 'rb_run') {
          if (intVal) {  // enabling RB
              $('#RB_STOP').hide();
              $('#RB_RUN').show();
              $('#RB_RUN').css('display', 'block');

          } else {  // disabling RB
              $('#RB_RUN').hide();
              $('#RB_STOP').show();
              $('#RB_STOP').css('display', 'block');
          }
      }
      else if (param_name == 'tx_modsrc_s') {
        $('#'+param_name).val(intVal);
        checkKeyDoEnable(param_name, intVal);
      }
      else if (param_name == 'tx_modtyp_s') {
        $('#'+param_name).val(intVal);
        switch (intVal) {
          case 2:
          case 3:
          case 4:
            $('#tx_mod_osc_mag_units').text('%');
            break;
          case 7:
            $('#tx_mod_osc_mag_units').text('Hz');
            break;
          case 8:
            $('#tx_mod_osc_mag_units').text('°');
            break;
          default:
            $('#tx_mod_osc_mag_units').text('( )');
        }
        checkKeyDoEnable(param_name, intVal);
      }
      else if (param_name == 'rbled_csp_s') {
        $('#'+param_name).val(intVal);
      }
      else if (param_name == 'rfout1_csp_s') {
        $('#'+param_name).val(intVal);
      }
      else if (param_name == 'rfout2_csp_s') {
        $('#'+param_name).val(intVal);
      }
      else if (param_name == 'rx_muxin_src_s') {
        $('#'+param_name).val(intVal);
        checkKeyDoEnable(param_name, intVal);
      }
      else if (param_name == 'tx_car_osc_qrg_f') {
        $('#'+param_name).val(dblVal);
        if (isTxQrgSel && !isRxQrgSel && RB.state.qrgController.editing) {
          scannerShowFrequency(dblVal);
        }
      }
      else if (param_name == 'rx_car_osc_qrg_f') {
        $('#'+param_name).val(dblVal);
        if (isRxQrgSel && !RB.state.qrgController.editing) {
          scannerShowFrequency(dblVal);
        }
      }
      else if (param_name == 'tx_mod_osc_qrg_f') {
        $('#'+param_name).val(dblVal);
      }
      else if (param_name == 'tx_amp_rf_gain_s') {
        $('#'+param_name).val(intVal);
      }
      else if (param_name == 'tx_mod_osc_mag_s') {
        $('#'+param_name).val(intVal);
      }
      else if (param_name == 'tx_muxin_gain_s') {
        $('#'+param_name).val(intVal);
      }
      else if (param_name == 'rx_muxin_gain_s') {
        $('#'+param_name).val(intVal);
      }
      else if (param_name == 'tx_qrg_sel_s') {
        if (intVal > 0) {
          $('#'+param_name).addClass('btnevttx_checked');
        } else {
          $('#'+param_name).removeClass('btnevttx_checked');
        }
      }
      else if (param_name == 'rx_qrg_sel_s') {
        if (intVal > 0) {
          $('#'+param_name).addClass('btnevtrx_checked');
        } else {
          $('#'+param_name).removeClass('btnevtrx_checked');
        }
      }
      else if (param_name == 'rx_modtyp_s') {
        $('#'+param_name).val(intVal);
        checkKeyDoEnable(param_name, intVal);
      }
      else if (param_name == 'rfout1_term_s') {
        $("input[name='" + param_name + "_rb'][value='" + intVal + "']").attr('checked', 'checked');
        //console.log('DEBUG param_name %s, value = %d', param_name, intVal);
      }
      else if (param_name == 'rfout2_term_s') {
        $("input[name='" + param_name + "_rb'][value='" + intVal + "']").attr('checked', 'checked');
        //console.log('DEBUG param_name %s, value = %d', param_name, intVal);
      }
      else if (param_name == 'qrg_inc_s') {
        // nothing
      }
      else if (param_name == 'ovrdrv_s') {
        //console.log('DEBUG param_name %s, value = %d', param_name, intVal);
        if (intVal & 0x1) {  // TX signal path
          $('#tx_muxin_gain_s').addClass('form-bg-tx-muxin-gain-ovrdrv');
        } else {
          $('#tx_muxin_gain_s').removeClass('form-bg-tx-muxin-gain-ovrdrv');
        }

        if (intVal & 0x2) {  // RX signal path
          $('#rx_muxin_gain_s').addClass('form-bg-rx-muxin-gain-ovrdrv');
        } else {
          $('#rx_muxin_gain_s').removeClass('form-bg-rx-muxin-gain-ovrdrv');
        }
      }

      /*
      if (param_name.indexOf('RB_MEAS_VAL') == 0) {
        var orig_units = $("#"+param_name).parent().children("#RB_MEAS_ORIG_UNITS").text();
        var orig_function = $("#"+param_name).parent().children("#RB_MEAS_ORIG_FOO").text();
        var orig_source = $("#"+param_name).parent().children("#RB_MEAS_ORIG_SIGNAME").text();
        var y = new_params[param_name].value;
        var z = y;
        var factor = '';

        $("#"+param_name).parent().children("#RB_MEAS_UNITS").text(factor + orig_units);
      }
      */

      /*
        // Find the field having ID equal to current parameter name
        // TODO: Use classes instead of ids, to be able to use a param name in multiple fields and to loop through all fields to set new values
        var field = $('#' + param_name);

        // Do not change fields from dialogs when user is editing something or new parameter value is the same as the old one
        if (field.closest('.menu-content').length == 0
            || (!RB.state.editing && (old_params[param_name] === undefined || old_params[param_name].value !== new_params[param_name].value))) {

          if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
                if (param_name == "RB_CH1_OFFSET")
                {
                    var units;
                    if (new_params["RB_CH1_SCALE"] != undefined)
                    {
                        if (Math.abs(new_params["RB_CH1_SCALE"].value) >= 1) {
                            units = 'V';
                        }
                        else if (Math.abs(new_params["RB_CH1_SCALE"].value) >= 0.001) {
                            units = 'mV';
                        }
                    }
                    else
                        units = $('#RB_CH1_OFFSET_UNIT').html();
                    var multiplier = units == "mV" ? 1000 : 1;
                    field.val(RB.formatValue(new_params[param_name].value * multiplier));
                } else if (param_name == "RB_CH2_OFFSET")
        */

    }

    if (send_all_params) {
      RB.sendParams();
    }
  };

  // Processes newly received data for signals
  RB.processSignalsFrmsCnt = 0;
  RB.processSignals = function(new_signals) {
    var visible_btns = [];
    var visible_plots = [];
    var visible_info = '';
    var start = +new Date();

    // Do nothing if no parameters received yet
    if ($.isEmptyObject(RB.params.orig)) {
      return;
    }

    // (Re)draw every signal
    for (sig_name in new_signals) {

      // Ignore empty signals
      if (new_signals[sig_name].size == 0) {
        continue;
      }

      /* ... */
    }

    // Reset resize flag
    RB.state.resized = false;

    var fps = 1000/(+new Date() - start);

    if (RB.processSignalsFrmsCnt++ >= 20 && RB.params.orig['DEBUG_SIGNAL_PERIOD']) {
      var new_period = 1100/fps < 25 ? 25 : 1100/fps;
      var period = {};

      period['DEBUG_SIGNAL_PERIOD'] = new_period;
      //period['DEBUG_SIGNAL_PERIOD'] = { value: new_period };
      RB.ac.send(JSON.stringify({ datasets: { params: period } }));
      //RB.ws.send(JSON.stringify({ parameters: period }));
      RB.processSignalsFrmsCnt = 0;
    }
  };


  /* Front-end to back-end (server) communication */

  // Sends to server modified parameters
  RB.sendParams = function() {
    if ($.isEmptyObject(RB.params.local)) {
      return false;
    }

    if (RB.state.sending == true) {
      // busy - drop this push, no data get lost by this
      return false;
    }

    /*
    if (!RB.state.socket_opened) {
      console.log('ERROR: Cannot save changes, socket not opened');
      return false;
    }
    */

    RB.state.sending = true;
    RB.state.pktIdx = 1;
    while (RB.state.pktIdx <= RB.state.pktIdxMax) {
      //RB.ws.send(JSON.stringify({ parameters: RB.params.local }));
      $.ajax({
        type: 'POST',
        url: RB.config.post_url,
        data: JSON.stringify({ app: { id: 'radiobox' }, datasets: { params: cast_params2transport(RB.params.local, RB.state.pktIdx) } }),
        timeout: RB.config.request_timeout,
        cache: false
      })
      .done(function(dresult) {
        // OK: Load the params received as POST result
        if (dresult.datasets !== undefined) {
          RB.parsePacket(dresult);
        }
        else if(dresult.status == 'ERROR') {
          RB.state.socket_opened = false;
          showModalError((dresult.reason ? dresult.reason : 'Failure returned when connecting the web-server - can not start the application (ERR1).'), false, true, true);
          RB.state.send_que = false;
        }
        else {
          RB.state.socket_opened = false;
          showModalError('Unknown connection state - can not start the application (ERR2).', false, true, true);
        }
      })
      .fail(function() {
        RB.state.socket_opened = false;
        showModalError('Can not connect the web-server (ERR3).', false, true, true);
      })
      .always(function() {
        RB.state.pktIdx  = 0;
        RB.state.editing = false;

        if (RB.state.send_que) {
          RB.state.send_que = false;
          setTimeout(function(refresh_data) {
            RB.sendParams(refresh_data);
          }, 100);
        }
      });
      RB.state.pktIdx++;
    }  // while ()

    setTimeout(function() {
      RB.params.local = {};
      RB.state.pktIdx = 0;
      RB.state.sending = false;
      RB.state.blocking = false;
      if (RB.state.doUpdate == true) {
        RB.state.doUpdate = false;
        RB.exitEditing(true);
      }
    }, 600);

    if (RB.params.orig['qrg_inc_s'] <= 40 || RB.params.orig['qrg_inc_s'] >= 60) {  // re-fire when range controller is active
      setTimeout(function() {
        if ($.isEmptyObject(RB.params.local)) {
          RB.params.local['qrg_inc_s'] = RB.params.orig['qrg_inc_s'];
          //console.log('INFO *** RB.sendParams: re-firing due to scanner operation. RB.params.local = ', RB.params.local);
          RB.sendParams();
        }
      }, 750);
    }
    return true;
  };


  /* Controller handling */

  // Exits from editing mode - create local parameters of changed values and send them away
  RB.exitEditing = function(noclose) {
    //console.log('INFO *** RB.exitEditing: RB.params.orig = ', RB.params.orig);
    /* btnevt handling */
    btnevt_handling();

    if (RB.state.eventClickId != '') {
      RB.state.eventLast.id = RB.state.eventClickId;
      RB.state.eventClickId = '';
      RB.state.qrgController.editing = false;
      RB.state.doUpdate = true;
      processField(RB.state.eventLast.id);
    }
    for (var key in RB.params.orig) {  // XXX controller to message handling
      processField(key);
    }

    // Send params then reset editing state and hide dialog
    RB.sendParams();
    RB.state.editing = false;
    if (noclose) {
      return;
    }

    $('.dialog:visible').hide();
    $('#right_menu').show();
  };
}(window.RB = window.RB || {}, jQuery));

function processField(key) {
  var field = $('#' + key);

  if (key == 'rb_run'){
    value = ($('#RB_RUN').is(':visible') ?  1 : 0);
  }
  else if (key == 'ovrdrv_s'){
    value = 0;
  }
  else if (key == 'tx_qrg_sel_s') {
    value = (field.hasClass('btnevttx_checked')) ?  1 : 0;
  }
  else if (key == 'rx_qrg_sel_s') {
    value = (field.hasClass('btnevtrx_checked')) ?  1 : 0;
  }

  else if (field.is('input:radio')) {
    value = parseInt($('input[name=' + key + '_rb]:checked').val());
    //console.log('DEBUG key %s is a input:radio, value = %d', key, value);
  }

  else if (field.is('input:button')) {
    //console.log('DEBUG radio-button: ' + key + ' --> from: ' + RB.params.orig[key] + '  to: ' + value + '  text: ' + $('input[name="' + key + '"]:checked').text());
    value = (field.is(":checked") ?  1 : 0);
    //console.log('DEBUG key %s is a input:button, value = %d', key, value);
  }

  else if (field.is('button')) {
    value = (field.hasClass('active') ? 1 : 0);
    //console.log('DEBUG key %s is a button, value = %d', key, value);
  }

  else if (field.is('select') || field.is('input')) {
    if (checkKeyIs_F(key)) {
      value = parseFloat(field.val());
    } else {
      value = parseInt(field.val());
    }
    //console.log('DEBUG key %s is a selector or input, value = %d', key, value);

  } else {
    console.log('DEBUG key ' + key + ' field is UNKNOWN');
  }

  //console.log('DEBUG RB.exitEditing: ' + key + ' WANT to change from ' + RB.params.orig[key] + ' to ' + value);

  // Check for specific values and enables/disables controllers
  checkKeyDoEnable(key, value);

  if (value !== undefined && value != RB.params.orig[key]) {
    var new_value = ($.type(RB.params.orig[key]) == 'boolean' ?  !!value : value);

    //console.log('INFO RB.exitEditing: ' + key + ' CHANGED from ' + RB.params.orig[key] + ' to ' + new_value);
    RB.params.local[key] = new_value;
    //RB.params.local[key] = { value: new_value };
  }
};

function btnevt_handling() {
  if (RB.state.eventLast.id === undefined) {
    return;
  }

  var btn_shift_digit = undefined;

  //console.log('DEBUG btnevt_handling: eventLast.id=' + RB.state.eventLast.id);

  /* locate released button */
  if (RB.state.eventLast.id == "tx_qrg_sel_s") {
    if ($('#' + RB.state.eventLast.id).hasClass("btnevttx_checked")) {
      $('#' + RB.state.eventLast.id).removeClass("btnevttx_checked");
      RB.state.qrgController.tx.button_checked = false;
    } else {
      $('#' + RB.state.eventLast.id).addClass("btnevttx_checked");
      RB.state.qrgController.tx.button_checked = true;
    }

  } else if (RB.state.eventLast.id == "rx_qrg_sel_s") {
    if ($('#' + RB.state.eventLast.id).hasClass("btnevtrx_checked")) {
      $('#' + RB.state.eventLast.id).removeClass("btnevtrx_checked");
      RB.state.qrgController.rx.button_checked = false;
    } else {
      $('#' + RB.state.eventLast.id).addClass("btnevtrx_checked");
      RB.state.qrgController.rx.button_checked = true;
    }

  } else if (RB.state.eventLast.id == "qrg_entry_del_b") {
    if (RB.state.qrgController.editing == false) {
      qrg_digits_clear();
    } else {
      btn_shift_digit = 'x';
    }

  } else if (RB.state.eventLast.id == "qrg_entry_enter_b") {
    var zero_idx;
    var zero_cnt = 0;
    for (zero_idx = 7; zero_idx >= 0; zero_idx--) {
      if (RB.state.qrgController.digit.e[zero_idx] == 0)  zero_cnt++;
      else  break;
    }

    switch (zero_cnt) {
    case 7:  // x MHz
      RB.state.qrgController.digit.e[7] = 0;
      RB.state.qrgController.digit.e[6] = RB.state.qrgController.digit.e[0];
      RB.state.qrgController.digit.e[5] = 0;
      RB.state.qrgController.digit.e[4] = 0;
      RB.state.qrgController.digit.e[3] = 0;
      RB.state.qrgController.digit.e[2] = 0;
      RB.state.qrgController.digit.e[1] = 0;
      RB.state.qrgController.digit.e[0] = 0;
      break;
    case 6:  // xx MHz
      if (RB.state.qrgController.digit.e[1] <= 6) {
        RB.state.qrgController.digit.e[7] = RB.state.qrgController.digit.e[1];
        RB.state.qrgController.digit.e[6] = RB.state.qrgController.digit.e[0];
        RB.state.qrgController.digit.e[5] = 0;
        RB.state.qrgController.digit.e[4] = 0;
        RB.state.qrgController.digit.e[3] = 0;
        RB.state.qrgController.digit.e[2] = 0;
        RB.state.qrgController.digit.e[1] = 0;
        RB.state.qrgController.digit.e[0] = 0;
      }
      break;
    case 4:  // x xxx kHz
      RB.state.qrgController.digit.e[7] = 0;
      RB.state.qrgController.digit.e[6] = RB.state.qrgController.digit.e[3];
      RB.state.qrgController.digit.e[5] = RB.state.qrgController.digit.e[2];
      RB.state.qrgController.digit.e[4] = RB.state.qrgController.digit.e[1];
      RB.state.qrgController.digit.e[3] = RB.state.qrgController.digit.e[0];
      RB.state.qrgController.digit.e[2] = 0;
      RB.state.qrgController.digit.e[1] = 0;
      RB.state.qrgController.digit.e[0] = 0;
      break;
    case 3:  // xx xxx kHz
      if (RB.state.qrgController.digit.e[4] <= 6) {
        RB.state.qrgController.digit.e[7] = RB.state.qrgController.digit.e[4];
        RB.state.qrgController.digit.e[6] = RB.state.qrgController.digit.e[3];
        RB.state.qrgController.digit.e[5] = RB.state.qrgController.digit.e[2];
        RB.state.qrgController.digit.e[4] = RB.state.qrgController.digit.e[1];
        RB.state.qrgController.digit.e[3] = RB.state.qrgController.digit.e[0];
        RB.state.qrgController.digit.e[2] = 0;
        RB.state.qrgController.digit.e[1] = 0;
        RB.state.qrgController.digit.e[0] = 0;
      }
      break;
    }
    RB.state.qrgController.editing = false;
    RB.state.qrgController.enter = true;

  } else {
    var btn_idx;
    for (btn_idx = 0; btn_idx <= 9; btn_idx++) {
      var btn_up = "qrg_up_1e"  + btn_idx + "_b";
      var btn_dn = "qrg_dn_1e"  + btn_idx + "_b";
      var btn_dt = "qrg_entry_" + btn_idx + "_b";

      if (btn_idx <= 7 && RB.state.eventLast.id == btn_up) {
        RB.state.qrgController.digit.e[btn_idx]++;
        RB.state.qrgController.editing = false;
        RB.state.qrgController.enter = true;
        break;
      } else if (btn_idx <= 7 && RB.state.eventLast.id == btn_dn) {
        RB.state.qrgController.digit.e[btn_idx]--;
        RB.state.qrgController.editing = false;
        RB.state.qrgController.enter = true;
        break;
      } else if (RB.state.eventLast.id == btn_dt) {
        if (RB.state.qrgController.editing == false) {
          qrg_digits_clear();
        }
        btn_shift_digit = btn_idx;
        RB.state.qrgController.editing = true;
        break;
      }
    }
  }

  /* normalize frequency digits */
  var norm_idx;
  for (norm_idx = 0; norm_idx <= 7; norm_idx++) {
    if (RB.state.qrgController.digit.e[norm_idx] > 9) {
      RB.state.qrgController.digit.e[norm_idx] = 0;
      if (norm_idx < 7)  RB.state.qrgController.digit.e[norm_idx + 1]++;
    } else if (RB.state.qrgController.digit.e[norm_idx] < 0) {
      RB.state.qrgController.digit.e[norm_idx] = 9;
      if (norm_idx < 7)  RB.state.qrgController.digit.e[norm_idx + 1]--;
    }
  }

  /* shift register*/
  if (btn_shift_digit == 'x') {
    // remove one digit
    var shift_idx;
    for (shift_idx = 0; shift_idx < 7; shift_idx++) {
      RB.state.qrgController.digit.e[shift_idx] = RB.state.qrgController.digit.e[shift_idx + 1];  // shift left
    }
    RB.state.qrgController.digit.e[7] = 0;

  } else if (btn_shift_digit !== undefined) {
    // new digit concatenated
    var shift_idx;
    for (shift_idx = 7; shift_idx > 0; shift_idx--) {
      RB.state.qrgController.digit.e[shift_idx] = RB.state.qrgController.digit.e[shift_idx - 1];  // shift left
    }
    RB.state.qrgController.digit.e[0] = btn_shift_digit;
  }

  /* output digits and sum up resulting frequency */
  RB.state.qrgController.frequency = 0.0;
  var digchr = ' ';
  var dig_hide = true;
  for (idx = 7; idx >= 0; idx--) {
    var idstr = "#qrg_display_1e" + idx + "_f";
    var dig = RB.state.qrgController.digit.e[idx];

    RB.state.qrgController.frequency += dig * Math.pow(10, idx);
    if (dig > 0 || !dig_hide || idx == 0) {
      digchr = dig;
      dig_hide = false;
    }
    $(idstr).val(digchr);
  }

  if (RB.state.qrgController.enter == true) {
    var blockingOld = RB.state.blocking;

    if (RB.state.qrgController.tx.button_checked == true) {
      if (blockingOld == false) {
        RB.state.blocking = true;
        $('#tx_car_osc_qrg_f').val(RB.state.qrgController.frequency);
        $('#tx_car_osc_qrg_f').addClass('qrg-entering');
        setTimeout(function() {
          $('#tx_car_osc_qrg_f').removeClass('qrg-entering');
        }, 200);
      }
    }

    if (RB.state.qrgController.rx.button_checked == true) {
      if (blockingOld == false) {
        RB.state.blocking = true;
        $('#rx_car_osc_qrg_f').val(RB.state.qrgController.frequency);
        $('#rx_car_osc_qrg_f').addClass('qrg-entering');
        setTimeout(function() {
          $('#rx_car_osc_qrg_f').removeClass('qrg-entering');
        }, 200);
      }
    }
    RB.state.qrgController.enter = false;
  }
  RB.state.eventLast.id = undefined;
}

function qrg_digits_clear() {
  var clear_idx;
  for (clear_idx = 0; clear_idx <= 7; clear_idx++) {
    RB.state.qrgController.digit.e[clear_idx] = 0;
  }
}

function scannerShowFrequency(frequency) {
  var digchr = ' ';
  var dig_hide = true;

  RB.state.qrgController.frequency = frequency;

  var show_idx;
  for (show_idx = 7; show_idx >= 0; show_idx--) {
    var idstr = "#qrg_display_1e" + show_idx + "_f";
    var dig = Math.floor(((frequency / Math.pow(10, show_idx)) % 10));
    RB.state.qrgController.digit.e[show_idx] = dig;
    if (dig > 0 || !dig_hide || show_idx == 0) {
      digchr = dig;
      dig_hide = false;
    }
    $(idstr).val(digchr);
  }
}

function qrg_inc_enable(enable) {
  if (enable) {
    $('.btnevt').removeAttr("disabled");
  } else {
    $('.btnevt').attr("disabled", "disabled");
  }
}

function checkKeyDoEnable(key, value) {  // XXX checkKeyDoEnable controllers
  if (key == 'tx_modtyp_s') {
    if (value == 1) {
      /* (none) */
      $('#tx_modsrc_o_0').removeAttr("disabled");
      $('#tx_modsrc_o_15').removeAttr("disabled");

      $('#tx_modsrc_s').attr("disabled", "disabled");
      $('#apply_tx_modsrc').attr("style", "visibility:hidden");

      $('#tx_mod_osc_qrg_f').attr("disabled", "disabled");
      $('#apply_tx_mod_osc_qrg').attr("style", "visibility:hidden");

      $('#tx_mod_osc_mag_s').attr("disabled", "disabled");
      $('#apply_tx_mod_osc_mag').attr("style", "visibility:hidden");

      $('#tx_muxin_gain_s').attr("disabled", "disabled");
      $('#apply_tx_muxin_gain').attr("style", "visibility:hidden");


    } else if (value == 2 || value == 3) {
      $('#tx_modsrc_s').removeAttr("disabled");
      $('#apply_tx_modsrc').removeAttr("style");

      $('#tx_modsrc_o_0').attr("disabled", "disabled");
      $('#tx_modsrc_o_15').attr("disabled", "disabled");

    } else {
      $('#tx_modsrc_o_0').removeAttr("disabled");
      $('#tx_modsrc_o_15').removeAttr("disabled");
      $('#tx_modsrc_s').removeAttr("disabled");
      $('#apply_tx_modsrc').removeAttr("style");
    }
  }

  if (key == 'tx_modsrc_s') {
    if (value == 15) {
      /* OSC_MOD */
      $('#tx_mod_osc_qrg_f').removeAttr("disabled");
      $('#apply_tx_mod_osc_qrg').removeAttr("style");

      $('#tx_mod_osc_mag_s').removeAttr("disabled");
      $('#apply_tx_mod_osc_mag').removeAttr("style");

      $('#tx_muxin_gain_s').attr("disabled", "disabled");
      $('#apply_tx_muxin_gain').attr("style", "visibility:hidden");

      $('#tx_modtyp_o_2').attr("disabled", "disabled");
      $('#tx_modtyp_o_3').attr("disabled", "disabled");

    } else if (value) {
      /* external */
      $('#tx_mod_osc_qrg_f').attr("disabled", "disabled");
      $('#apply_tx_mod_osc_qrg').attr("style", "visibility:hidden");

      $('#tx_mod_osc_mag_s').removeAttr("disabled");
      $('#apply_tx_mod_osc_mag').removeAttr("style");

      $('#tx_muxin_gain_s').removeAttr("disabled");
      $('#apply_tx_muxin_gain').removeAttr("style");

      $('#tx_modtyp_o_2').removeAttr("disabled");
      $('#tx_modtyp_o_3').removeAttr("disabled");

    } else {
      /* (none) */
      $('#tx_mod_osc_qrg_f').attr("disabled", "disabled");
      $('#apply_tx_mod_osc_qrg').attr("style", "visibility:hidden");

      $('#tx_mod_osc_mag_s').attr("disabled", "disabled");
      $('#apply_tx_mod_osc_mag').attr("style", "visibility:hidden");

      $('#tx_muxin_gain_s').attr("disabled", "disabled");
      $('#apply_tx_muxin_gain').attr("style", "visibility:hidden");

      $('#tx_modtyp_o_2').attr("disabled", "disabled");
      $('#tx_modtyp_o_3').attr("disabled", "disabled");
    }
  }

  if (key == 'rx_modtyp_s') {
    if (value != 1) {
      /* active */
      $('#rx_muxin_src_s').removeAttr("disabled");
      $('#apply_rx_muxin_src').removeAttr("style");

      $('#rx_muxin_gain_s').removeAttr("disabled");
      $('#apply_rx_muxin_gain').removeAttr("style");

    } else {
      /* (none) */
      $('#rx_muxin_src_s').attr("disabled", "disabled");
      $('#apply_rx_muxin_src').attr("style", "visibility:hidden");

      $('#rx_muxin_gain_s').attr("disabled", "disabled");
      $('#apply_rx_muxin_gain').attr("style", "visibility:hidden");
    }
  }
}

function checkKeyIs_F(key) {
  return (key.lastIndexOf("_f") == (key.length - 2));
}


// Page onload event handler
$(function() {
  $('#modal-warning').hide();

  /*
  $('button').bind('activeChanged', function() {
    console.log('DEBUG button.bind.activeChanged()\n');
    RB.exitEditing(true);
  });

  $('input:radio').bind('activeChanged', function() {
    console.log('DEBUG input:radio.bind.activeChanged()\n');
    RB.exitEditing(true);
  });
  */

  $('select, input').on('change', function() {
    //console.log('DEBUG select,input.on.change()\n');
    RB.exitEditing(true);
  });

  // Initialize FastClick to remove the 300ms delay between a physical tap and the firing of a click event on mobile browsers
  //new FastClick(document.body);

  // Process clicks on top menu buttons
  //$('#RB_RUN').on('click touchstart', function(ev) {
  $('#RB_RUN').on('click', function(ev) {
    ev.preventDefault();
    $('#RB_RUN').hide();
    $('#RB_STOP').show();
    $('#RB_STOP').css('display','block');
    //RB.params.local['rb_run'] = { value: false };
    RB.params.local['rb_run'] = 0;
    RB.sendParams();
  });

  //$('#RB_STOP').on('click touchstart', function(ev) {
  $('#RB_STOP').on('click', function(ev) {
    ev.preventDefault();
    $('#RB_STOP').hide();
    $('#RB_RUN').show();
    $('#RB_RUN').css('display','block');
    RB.params.local['rb_run'] = 1;
    RB.sendParams();
  });

  /*
  // Selecting active signal
  //$('.menu-btn').on('click touchstart', function() {
  $('.menu-btn').on('click', function() {
    $('#right_menu .menu-btn').not(this).removeClass('active');
    if (!$(this).hasClass('active'))
        RB.state.sel_sig_name = $(this).data('signal');
    else
        RB.state.sel_sig_name = null;
    $('.y-offset-arrow').css('z-index', 10);
    $('#' + RB.state.sel_sig_name + '_offset_arrow').css('z-index', 11);
  });
  */

  $('.btnevt, .btn').on('click', function() {
    //console.log('DEBUG .btn.on("click")()\n');
    var btn = $(this);
    setTimeout(function() {
      btn.blur();
    }, 10);
  });

  $('.btnevt, .btn, .rangeevt').mouseup(function(ev) {
    //console.log('DEBUG .btn.mouseup(), id = ' + ev.target.id);
    RB.state.eventLast.id = ev.target.id;
    setTimeout(function() {
      //updateLimits();
      //formatVals();
      RB.exitEditing(true);
    }, 250);
  });

  $('.btnevt, .btn, .rangeevt').on('input', function(ev) {
    //console.log('DEBUG .btn.on("input", f())\n');
    RB.state.eventLast.id = ev.target.id;
    setTimeout(function() {
      //updateLimits();
      //formatVals();
      RB.exitEditing(true);
    }, 20);
  });

  // Close parameters dialog after Enter key is pressed
  $('input').keyup(function(ev) {
    //console.log('DEBUG input.keyup(ev)\n');
    if (ev.keyCode == 13) {
      RB.exitEditing(true);
    }
  });

  // Close parameters dialog on close button click
  //$('.close-dialog').on('click touchstart', function() {
  $('.close-dialog').on('click', function() {
    RB.exitEditing();
  });

  // Mousewheel elements
  $('.mswheelbtn').mousewheel(function(ev, delta) {
    //console.log('DEBUG mousewheel(ev, delta): delta = ' + delta + '\n');
    ev.stopPropagation();
    ev.preventDefault();
    RB.state.qrgController.mousewheelsum += delta;

    var lim = RB.state.mouseWheelLim;
    var btnId = 'qrg_';
    if (RB.state.qrgController.mousewheelsum <= -lim) {
      RB.state.qrgController.mousewheelsum += lim;
      btnId += 'up_1e';
    } else if (RB.state.qrgController.mousewheelsum >= lim) {
      RB.state.qrgController.mousewheelsum -= lim;
      btnId += 'dn_1e';
    } else {
      return;
    }
    var id = ev.target.id;
    if (id !== undefined && id.length >= 3) {
      btnId += id.substr(id.length - 3,1) + '_b';
    } else {
      return;
    }
    setTimeout(function() {
      //console.log('DEBUG mousewheel(ev, delta) click(): id = ' + id + ', btnId = ' + btnId + ', delta = ' + delta + '\n');
      //$('#' + btnId).attr('style', 'background:#000000;');
      //$('#' + btnId).click();
      RB.state.eventClickId = btnId;
      RB.exitEditing(true);
    }, 10);
  });

  $('.mswheelrange').mousewheel(function(ev, delta) {
    //console.log('DEBUG mousewheel(ev, delta): delta = ' + delta + '\n');
    var mw = $(this);
    var max = 100;
    var deltaCor = delta;

    // make mousewheel more sensitive
    if (deltaCor > 0) {
      deltaCor += RB.state.mouseWheelLim >> 1;
    } else if (deltaCor < 0) {
      deltaCor -= RB.state.mouseWheelLim >> 1;
    }

    ev.stopPropagation();
    ev.preventDefault();
    setTimeout(function() {
      var cur = parseInt(mw.val()) - (deltaCor / RB.state.mouseWheelLim);
      if (cur < 0) {
        cur = 0;
      } else if (cur > max) {
        cur = max;
      }
      mw.val(cur);
      RB.exitEditing(true);
    }, 10);
  });

  /*
  // Touch events
  $(document).on('touchstart', '.plot', function(ev) {
    ev.preventDefault();

    // Multi-touch is used for zooming
    if (!RB.touch.start && ev.originalEvent.touches.length > 1) {
      RB.touch.zoom_axis = null;
      RB.touch.start = [
        { clientX: ev.originalEvent.touches[0].clientX, clientY: ev.originalEvent.touches[0].clientY },
        { clientX: ev.originalEvent.touches[1].clientX, clientY: ev.originalEvent.touches[1].clientY }
      ];
    }
    // Single touch is used for changing offset
    else if (! RB.state.simulated_drag) {
      RB.state.simulated_drag = true;
      RB.touch.offset_axis = null;
      RB.touch.start = [
        { clientX: ev.originalEvent.touches[0].clientX, clientY: ev.originalEvent.touches[0].clientY }
      ];
    }
  });
  */

  // Preload images which are not visible at the beginning
  $.preloadImages = function() {
    for (var i = 0; i < arguments.length; i++) {
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
    /*
    RB.params.local['in_command'] = 'send_all_params';
    RB.sendParams();
    */
    /*
    if (RB.ws) {
      RB.params.local['in_command'] = { value: 'send_all_params' };
      RB.ws.send(JSON.stringify({ parameters: RB.params.local }));
      RB.params.local = {};
    }
    */
    RB.state.resized = true;
  }).resize();

  // Stop the application when page is unloaded
  window.onbeforeunload = function() {
    RB.state.app_started = false;
    RB.state.socket_opened = false;
    $.ajax({
      url: RB.config.stop_app_url,
      async: false
    });
  };

  // Everything prepared, start application
  RB.startApp();
});

/*
$(".limits").change(function() {
  if (['SOUR1_PHAS', 'SOUR1_DCYC', 'SOUR2_PHAS', 'SOUR2_DCYC'].indexOf($(this).attr('id')) != -1) {
    var min = 0;
    var max = $(this).attr('id').indexOf('DCYC') > 0 ? 100 : 180;

    if (isNaN($(this).val()) || $(this).val() < min)
      $(this).val(min);
    else if ($(this).val() > max)
      $(this).val(max);

  } else {
    var min = $(this).attr('id').indexOf('OFFS') > 0 ? -1 : 0;
    var max = 1;
    if (isNaN($(this).val()) || $(this).val() < min)
      $(this).val(min == -1 ? 0 : 1);
    else if (isNaN($(this).val()) || $(this).val() > max)
      $(this).val(min == -1 ? 0 : 1);
  }
}).change();
*/

/*
function updateLimits() {
    { // RB_CH1_OFFSET limits
      var probeAttenuation = parseInt($("#RB_CH1_PROBE option:selected").text());
      var jumperSettings = $("#RB_CH1_IN_GAIN").parent().hasClass("active") ? 1 : 20;
      var units = $('#RB_CH1_OFFSET_UNIT').html();
      var multiplier = units == "mV" ? 1000 : 1;
      var newMin = -1 * 10 * jumperSettings * probeAttenuation * multiplier;
      var newMax =  1 * 10 * jumperSettings * probeAttenuation * multiplier;
      $("#RB_CH1_OFFSET").attr("min", newMin);
      $("#RB_CH1_OFFSET").attr("max", newMax);
    }
}
*/

/*
$('#RB_CH1_OFFSET_UNIT').bind("DOMSubtreeModified",function() {
  updateLimits();
  formatVals();
});
*/

$( document ).ready(function() {
  /*
  updateLimits();
  formatVals();
  */
});


function cast_4xfloat_to_1xdouble(quad)
{
  var IEEE754_DOUBLE_EXP_BIAS = 1023;
  var IEEE754_DOUBLE_EXP_BITS = 11;
  var IEEE754_DOUBLE_MNT_BITS = 52;
  var RB_CELL_MNT_BITS        = 20;

  var d = 0.0;
  var se_i = quad.se;
  var hi_i = quad.hi;
  var mi_i = quad.mi;
  var lo_i = quad.lo;

  if (quad.se || quad.hi || quad.mi || quad.lo) {
    /* normalized data */
    var mant_idx;
    for (mant_idx = 0; mant_idx < IEEE754_DOUBLE_MNT_BITS; ++mant_idx) {
      if (mant_idx >= (IEEE754_DOUBLE_MNT_BITS - RB_CELL_MNT_BITS)) {  // bits 51:32
        if (quad.hi & 0x1) {
          d += 1.0;
        }
        quad.hi >>= 1;
      } else if (mant_idx >= (IEEE754_DOUBLE_MNT_BITS - (RB_CELL_MNT_BITS << 1))) {  // bits 31:12
        if (quad.mi & 0x1) {
          d += 1.0;
        }
        quad.mi >>= 1;
      } else {  // bits 11:0
        if (quad.lo & 0x1) {
          d += 1.0;
        }
        quad.lo >>= 1;
      }
      d /= 2.0;
    }
    d += 1.0;  // hidden '1' of IEEE mantissa

    // exponent shifter
    var exp = quad.se & 0x7ff;
    var sgn = quad.se >> (IEEE754_DOUBLE_EXP_BITS);
    if (quad.se > IEEE754_DOUBLE_EXP_BIAS) {
      while (quad.se > IEEE754_DOUBLE_EXP_BIAS) {
        d *= 2.0;
        quad.se -= 1;
      }
    } else if (quad.se < IEEE754_DOUBLE_EXP_BIAS) {
      while (quad.se < IEEE754_DOUBLE_EXP_BIAS) {
        d /= 2.0;
        quad.se += 1;
      }
    }

    if (sgn) {
      d = -d;
    }
  }

  //console.log('INFO cast_4xfloat_to_1xdouble: out(d=', d, ') <-- in(quad.se=', se_i, ', quad.hi=', hi_i, ', quad.mi=', mi_i, ', quad.lo=', lo_i, ')\n');
  return d;
}

function cast_1xdouble_to_4xfloat(d)
{
  var IEEE754_DOUBLE_EXP_BIAS = 1023;
  var IEEE754_DOUBLE_MNT_BITS = 52;
  var RB_CELL_MNT_BITS        = 20;

  var di = d;
  var quad = { se: 0, hi: 0, mi: 0, lo: 0 };
  var ctr = 0;

  //console.log('INFO cast_1xdouble_to_4xfloat (1): in(d=', d, ')\n');

  if (d == 0.0) {
    /* use unnormalized zero instead */
    //console.log('INFO cast_1xdouble_to_4xfloat (9) (zero): out(se=', quad.se, ', hi=', quad.hi, ', mi=', quad.mi, ', lo=', quad.lo, ')\n');
    return quad;
  }

  if (d < 0.0) {
    d = -d;
    quad.se = (IEEE754_DOUBLE_EXP_BIAS + 1) << 1;  // that is the sign bit
  }

  // determine the exponent
  quad.se += IEEE754_DOUBLE_EXP_BIAS;
  if (d >= 2.0) {
    while ((d >= 2.0) && (ctr < 1023)) {
      d /= 2.0;
      quad.se += 1;
      ctr++;
    }

  } else if (d < 1.0) {
    while ((d < 1.0)  && (ctr < 1023)) {
      d *= 2.0;
      quad.se -= 1;
      ctr++;
    }
  }

  // hidden '1' of IEEE mantissa is discarded
  d -= 1.0;
  //console.log('INFO cast_1xdouble_to_4xfloat (2): mantisssa w/o hidden 1  d=', d, ')\n');

  // scan the mantissa
  var mant_idx;
  for (mant_idx = IEEE754_DOUBLE_MNT_BITS - 1; mant_idx >= 0; --mant_idx) {
    if (mant_idx >= (IEEE754_DOUBLE_MNT_BITS - RB_CELL_MNT_BITS)) {  // bits 51:32
      quad.hi <<= 1;
    } else if (mant_idx >= (IEEE754_DOUBLE_MNT_BITS - (RB_CELL_MNT_BITS << 1))) {  // bits 31:12
      quad.mi <<= 1;
    } else {
      quad.lo <<= 1;
    }

    d *= 2.0;
    if (d >= 1.0) {
      d -= 1.0;
      if (mant_idx >= (IEEE754_DOUBLE_MNT_BITS - RB_CELL_MNT_BITS)) {  // bits 51:32
        quad.hi |= 1;
      } else if (mant_idx >= (IEEE754_DOUBLE_MNT_BITS - (RB_CELL_MNT_BITS << 1))) {  // bits 31:12
        quad.mi |= 1;
      } else {
        quad.lo |= 1;
      }
    }
  }

  //console.log('INFO cast_1xdouble_to_4xfloat: out(se=', quad.se, ', hi=', quad.hi, ', mi=', quad.mi, ', lo=', quad.lo, ') <-- in(d=', di, ')\n');
  return quad;
}

function cast_params2transport(params, pktIdx)
{  // XXX params --> transport
  var transport = { };

  transport['pktIdx']  = pktIdx;

  // max. eight single variables or two quad variables with each packet
  switch (pktIdx) {  // XXX pktIdx packaging
  case 1:
    if (params['rb_run'] !== undefined) {
      transport['rb_run'] = params['rb_run'];
    }

    if (params['tx_modsrc_s'] !== undefined) {
      transport['tx_modsrc_s'] = params['tx_modsrc_s'];
    }

    if (params['tx_modtyp_s'] !== undefined) {
      transport['tx_modtyp_s'] = params['tx_modtyp_s'];
    }

    if (params['rx_modtyp_s'] !== undefined) {
      transport['rx_modtyp_s'] = params['rx_modtyp_s'];
    }

    if (params['rbled_csp_s'] !== undefined) {
      transport['rbled_csp_s'] = params['rbled_csp_s'];
    }

    if (params['rfout1_csp_s'] !== undefined) {
      transport['rfout1_csp_s'] = params['rfout1_csp_s'];
    }

    if (params['rfout2_csp_s'] !== undefined) {
      transport['rfout2_csp_s'] = params['rfout2_csp_s'];
    }

    if (params['rx_muxin_src_s'] !== undefined) {
      transport['rx_muxin_src_s'] = params['rx_muxin_src_s'];
    }
    break;

  case 2:
    if (params['tx_car_osc_qrg_f'] !== undefined) {
      var quad = cast_1xdouble_to_4xfloat(params['tx_car_osc_qrg_f']);
      transport['SE_tx_car_osc_qrg_f'] = quad.se;
      transport['HI_tx_car_osc_qrg_f'] = quad.hi;
      transport['MI_tx_car_osc_qrg_f'] = quad.mi;
      transport['LO_tx_car_osc_qrg_f'] = quad.lo;
    }

    if (params['rx_car_osc_qrg_f'] !== undefined) {
      var quad = cast_1xdouble_to_4xfloat(params['rx_car_osc_qrg_f']);
      transport['SE_rx_car_osc_qrg_f'] = quad.se;
      transport['HI_rx_car_osc_qrg_f'] = quad.hi;
      transport['MI_rx_car_osc_qrg_f'] = quad.mi;
      transport['LO_rx_car_osc_qrg_f'] = quad.lo;
    }
    break;

  case 3:
    if (params['tx_mod_osc_qrg_f'] !== undefined) {
      var quad = cast_1xdouble_to_4xfloat(params['tx_mod_osc_qrg_f']);
      transport['SE_tx_mod_osc_qrg_f'] = quad.se;
      transport['HI_tx_mod_osc_qrg_f'] = quad.hi;
      transport['MI_tx_mod_osc_qrg_f'] = quad.mi;
      transport['LO_tx_mod_osc_qrg_f'] = quad.lo;
    }

    if (params['tx_muxin_gain_s'] !== undefined) {
      transport['tx_muxin_gain_s'] = params['tx_muxin_gain_s'];
    }

    if (params['rx_muxin_gain_s'] !== undefined) {
      transport['rx_muxin_gain_s'] = params['rx_muxin_gain_s'];
    }

    if (params['tx_qrg_sel_s'] !== undefined) {
      transport['tx_qrg_sel_s'] = params['tx_qrg_sel_s'];
    }

    if (params['rx_qrg_sel_s'] !== undefined) {
      transport['rx_qrg_sel_s'] = params['rx_qrg_sel_s'];
    }
    break;

  case 4:
    if (params['tx_amp_rf_gain_s'] !== undefined) {
      transport['tx_amp_rf_gain_s'] = params['tx_amp_rf_gain_s'];
    }

    if (params['tx_mod_osc_mag_s'] !== undefined) {
      transport['tx_mod_osc_mag_s'] = params['tx_mod_osc_mag_s'];
    }

    if (params['rfout1_term_s'] !== undefined) {
      transport['rfout1_term_s'] = params['rfout1_term_s'];
    }

    if (params['rfout2_term_s'] !== undefined) {
      transport['rfout2_term_s'] = params['rfout2_term_s'];
    }

    if (params['qrg_inc_s'] !== undefined) {
      var value = params['qrg_inc_s'];
      if ((value <= 40) || (value >= 60)) {  // +/-10 %
        qrg_inc_enable(0);
      } else {
        qrg_inc_enable(1);
        value = 50;
      }
      transport['qrg_inc_s']      = value;
      RB.params.orig['qrg_inc_s'] = value;
    }

    if (params['ovrdrv_s'] !== undefined) {
      transport['ovrdrv_s'] = params['ovrdrv_s'];
    }
    break;

  default:
    // no limitation of output data
    break;
  }

  //console.log('INFO cast_params2transport: out(transport=', transport, ') <-- in(params=', params, ')\n');
  return transport;
}

function cast_transport2params(transport)
{  // XXX transport --> params
  var params = { };

  if (transport['rb_run'] !== undefined) {
    params['rb_run'] = transport['rb_run'];
  }

  if (transport['tx_modsrc_s'] !== undefined) {
    params['tx_modsrc_s'] = transport['tx_modsrc_s'];
  }

  if (transport['tx_modtyp_s'] !== undefined) {
    params['tx_modtyp_s'] = transport['tx_modtyp_s'];
  }

  if (transport['rx_modtyp_s'] !== undefined) {
    params['rx_modtyp_s'] = transport['rx_modtyp_s'];
  }

  if (transport['rbled_csp_s'] !== undefined) {
    params['rbled_csp_s'] = transport['rbled_csp_s'];
  }

  if (transport['rfout1_csp_s'] !== undefined) {
    params['rfout1_csp_s'] = transport['rfout1_csp_s'];
  }

  if (transport['rfout2_csp_s'] !== undefined) {
    params['rfout2_csp_s'] = transport['rfout2_csp_s'];
  }

  if (transport['rx_muxin_src_s'] !== undefined) {
    params['rx_muxin_src_s'] = transport['rx_muxin_src_s'];
  }

  if (transport['LO_tx_car_osc_qrg_f'] !== undefined) {
    var quad = { };
    quad.se = transport['SE_tx_car_osc_qrg_f'];
    quad.hi = transport['HI_tx_car_osc_qrg_f'];
    quad.mi = transport['MI_tx_car_osc_qrg_f'];
    quad.lo = transport['LO_tx_car_osc_qrg_f'];
    params['tx_car_osc_qrg_f'] = cast_4xfloat_to_1xdouble(quad);
  }

  if (transport['LO_rx_car_osc_qrg_f'] !== undefined) {
    var quad = { };
    quad.se = transport['SE_rx_car_osc_qrg_f'];
    quad.hi = transport['HI_rx_car_osc_qrg_f'];
    quad.mi = transport['MI_rx_car_osc_qrg_f'];
    quad.lo = transport['LO_rx_car_osc_qrg_f'];
    params['rx_car_osc_qrg_f'] = cast_4xfloat_to_1xdouble(quad);
  }

  if (transport['LO_tx_mod_osc_qrg_f'] !== undefined) {
    var quad = { };
    quad.se = transport['SE_tx_mod_osc_qrg_f'];
    quad.hi = transport['HI_tx_mod_osc_qrg_f'];
    quad.mi = transport['MI_tx_mod_osc_qrg_f'];
    quad.lo = transport['LO_tx_mod_osc_qrg_f'];
    params['tx_mod_osc_qrg_f'] = cast_4xfloat_to_1xdouble(quad);
  }

  if (transport['tx_amp_rf_gain_s'] !== undefined) {
    params['tx_amp_rf_gain_s'] = transport['tx_amp_rf_gain_s'];
  }

  if (transport['tx_mod_osc_mag_s'] !== undefined) {
    params['tx_mod_osc_mag_s'] = transport['tx_mod_osc_mag_s'];
  }

  if (transport['tx_muxin_gain_s'] !== undefined) {
    params['tx_muxin_gain_s'] = transport['tx_muxin_gain_s'];
  }

  if (transport['rx_muxin_gain_s'] !== undefined) {
    params['rx_muxin_gain_s'] = transport['rx_muxin_gain_s'];
  }

  if (transport['tx_qrg_sel_s'] !== undefined) {
    params['tx_qrg_sel_s'] = transport['tx_qrg_sel_s'];
  }

  if (transport['rx_qrg_sel_s'] !== undefined) {
    params['rx_qrg_sel_s'] = transport['rx_qrg_sel_s'];
  }

  if (transport['rfout1_term_s'] !== undefined) {
    params['rfout1_term_s'] = transport['rfout1_term_s'];
  }

  if (transport['rfout2_term_s'] !== undefined) {
    params['rfout2_term_s'] = transport['rfout2_term_s'];
  }

  if (transport['qrg_inc_s'] !== undefined) {
    params['qrg_inc_s'] = transport['qrg_inc_s'];
  }

  if (transport['ovrdrv_s'] !== undefined) {
    params['ovrdrv_s'] = transport['ovrdrv_s'];
  }

  //console.log('INFO cast_transport2params: out(params=', params, ') <-- in(transport=', transport, ')\n');
  return params;
}


(function ($) {
  $.fn.iLightInputNumber = function (options) {
    var inBox = '.input-number-box',
      newInput = '.input-number',
      moreVal = '.input-number-more',
      lessVal = '.input-number-less';

    this.each(function () {
      var el = $(this);
      $('<div class="' + inBox.substr(1) + '"></div>').insertAfter(el);
      var parent = el.find('+ ' + inBox);
      parent.append(el);
      var classes = el.attr('class');

      el.addClass(classes);
      var attrValue;

      parent.append('<div class=' + moreVal.substr(1) + '></div>');
      parent.append('<div class=' + lessVal.substr(1) + '></div>');
    }); //end each

    var value,
        step;

    var interval = null,
        timeout = null;

    function ToggleValue(input) {
      input.val(parseInt(input.val(), 10) + d);
      console.log(input);
    }

    $('body').on('mousedown', moreVal, function () {
      var el = $(this);
      var input = el.siblings(newInput);
      moreValFn(input);
      timeout = setTimeout(function() {
        interval = setInterval(function() { moreValFn(input); }, 50);
      }, 200);
    });

    $('body').on('mousedown', lessVal, function () {
      var el = $(this);
      var input = el.siblings(newInput);
      lessValFn(input);
      timeout = setTimeout(function() {
        interval = setInterval(function() { lessValFn(input); }, 50);
      }, 200);
    });

    $(moreVal +', '+ lessVal).on("mouseup mouseout", function() {
      clearTimeout(timeout);
      clearInterval(interval);
    });


    function getLimits(input) {
      var min = parseFloat(input.attr('min'));
      var max = parseFloat(input.attr('max'));
      return {'min': min, 'max': max};
    }

    function moreValFn(input) {
      var max;
      var limits = getLimits(input);
      max = limits.max;
      checkInputAttr(input);

      var newValue = value + step;
      var parts = step.toString().split('.');
      var signs = parts.length < 2 ? 0 : parts[1].length;
      newValue = parseFloat(newValue.toFixed(signs));

      if (newValue > max) {
        newValue = max;
      }
      changeInputsVal(input, newValue);
    }

    function lessValFn(input) {
      var min;
      var limits = getLimits(input);
      min = limits.min;

      checkInputAttr(input);

      var newValue = value - step;
      var parts = step.toString().split('.');
      var signs = parts.length < 2 ? 0 : parts[1].length;
      newValue = parseFloat(newValue.toFixed(signs));
      if (newValue < min) {
        newValue = min;
      }
      changeInputsVal(input, newValue);
    }

    function changeInputsVal(input, newValue) {
      input.val(newValue);
      RB.exitEditing(true);
    }


    function checkInputAttr(input) {
      value = parseFloat(input.val());

      if (!($.isNumeric(value))) {
        value = 0;
      } else {
        step = 1;
      }
    }

    $(newInput).change(function () {
      var input = $(this);

      checkInputAttr(input);
      var limits = getLimits(input);
      var min = limits.min;
      var max = limits.max;

      var parts = step.toString().split('.');
      var signs = parts.length < 2 ? 0 : parts[1].length;
      value = parseFloat(value.toFixed(signs));

      if (value < min) {
        value = min;
      } else if (value > max) {
        value = max;
      }

      if (!($.isNumeric(value))) {
        value = 0;
      }
      input.val(value);
    });

    $(newInput).keydown(function(e) {
      var input = $(this);
      var k = e.keyCode;
      if (k == 38) {
        moreValFn(input);
      } else if (k == 40) {
        lessValFn(input);
      }
    });
  };
})(jQuery);

$('input[type=text]').iLightInputNumber({
    mobile: false
});
