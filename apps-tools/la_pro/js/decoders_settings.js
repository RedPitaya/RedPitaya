/*
 * Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

(function(LA, $, undefined) {

    LA.buses = {}
    LA.buses.def = {}
    LA.state.bus_editing = 0

    LA.isChannelInUse = function(ch, ex_bus) {
        for (var i = 1; i < 5; i++) {
            if (ex_bus !== undefined && ex_bus == i)
                continue;
            var bus = 'bus' + i;
            if (LA.buses[bus] !== undefined && LA.buses[bus].name !== undefined) {
                if ((LA.buses[bus].name == "UART")
                    && (LA.buses[bus].config.rx == ch || LA.buses[bus].config.tx == ch))
                        return i;

                if (LA.buses[bus].name == "SPI" &&
                    (LA.buses[bus].config.clk == ch || LA.buses[bus].config.miso == ch || LA.buses[bus].config.mosi == ch  || LA.buses[bus].config.cs == ch))
                    return i;

                if (LA.buses[bus].name == "I2C" && (LA.buses[bus].config.sda == ch || LA.buses[bus].config.scl == ch))
                    return i;

                if (LA.buses[bus].name == "CAN" && (LA.buses[bus].config.rx == ch))
                    return i;
            }
        }
        return -1;
    }

    LA.showInfoArrow = function(ch) {
        var arrow_info = $('#ch' + (ch + 1) + '_info');
        var arrow_img = $('#img_info_arrow' + (ch + 1));
        var info_text = OSC.accordingChanName(ch + 1);

        if (info_text !== "") {
            arrow_info.text(info_text);
            arrow_img.show();
        }else{
            arrow_info.text("");
            arrow_img.hide();
        }
    }

    // LA.hideInfoArrow = function(ch) {
    //     var arrow_info = $('#ch' + (ch + 1) + '_info');
    //     var arrow_img = $('#img_info_arrow' + (ch + 1));

    //     arrow_info.text("");
    //     arrow_img.hide();
    // }

    LA.applyDecoder = function() {
        var protocol = $('#protocol_selector option:selected').text();
        $('#warning-dialog').hide();
        // Saving local data
        var bus = 'bus' + LA.state.bus_editing;
        if (protocol == "UART") {
            var baudrate = $('#uart_baudrate').val();
            var rx = $('#uart_rx').val();
            var tx = $('#uart_tx').val();

            if (baudrate <= 0) {
                $('#warn-message').text("Baudrate value is incorrect");
                $('#warning-dialog').show();
                return;
            }

            if (rx == 0 && tx == 0) {
                $('#warn-message').text("Please specify channel for decoding");
                $('#warning-dialog').show();
                return;
            }

            if (rx === tx) {
                $('#warn-message').text("You can't specify same input lines");
                $('#warning-dialog').show();
                return;
            }

            if (LA.isChannelInUse(rx, LA.state.bus_editing) != -1) {
                $('#warn-message').text($('#uart_serial option:selected').text() + " is already in use");
                $('#warning-dialog').show();
                return;
            }

            if (LA.isChannelInUse(tx, LA.state.bus_editing) != -1) {
                $('#warn-message').text($('#uart_serial option:selected').text() + " is already in use");
                $('#warning-dialog').show();
                return;
            }

            // OSC.destroyDecoder(bus, "UART");
            // var upd = OSC.needUpdateDecoder(bus, "UART");

            // if (!upd)
            //     OSC.buses[bus] = {};
            // else
            //     OSC.hideInfoArrow(parseInt(OSC.buses[bus].rx) - 1);

            // OSC.buses[bus].name = "UART";
            // OSC.buses[bus].enabled = true;

            LA.buses[bus].config.rx = parseInt(rx);
            LA.buses[bus].config.tx = parseInt(tx);

            LA.buses[bus].config.baudrate = parseInt(baudrate);
            LA.buses[bus].config.num_data_bits = parseInt($('#uart_data_length').val())
            LA.buses[bus].config.num_stop_bits = parseInt($('#uart_stop_bits').val())
            LA.buses[bus].config.parity = parseInt($('#uart_parity').val())
            LA.buses[bus].config.bitOrder = parseInt($('#uart_order').val())
            LA.buses[bus].config.invert = parseInt($('#uart_invert').val())

            LA.buses[bus].config.samplerate = CLIENT.getValue('LA_CUR_FREQ')

            CLIENT.parametersCache['DECODER_' + LA.state.bus_editing] = {value:JSON.stringify(LA.buses[bus])}
            CLIENT.parametersCache['DECODER_ENABLED_' + LA.state.bus_editing] = {value: true}
            CLIENT.sendParameters()
            // OSC.showInfoArrow(parseInt(OSC.buses[bus].rx) - 1);

            // if (!upd) {
            //     OSC.buses[bus].decoder = 'uart' + OSC.state.decoder_id;
            //     OSC.state.decoder_id++;
            // }

            // if (!upd) {
            //     OSC.params.local['CREATE_DECODER'] = {
            //         value: 'uart'
            //     };
            //     OSC.params.local['DECODER_NAME'] = {
            //         value: OSC.buses[bus].decoder
            //     };
            // } else {
            //     OSC.params.local[OSC.buses[bus].decoder + "_parameters"] = {
            //         value: OSC.buses[bus]
            //     };
            // }

            // OSC.sendParams();
        }

        if (protocol == "SPI") {

            var miso = $('#spi_miso').val();
            var mosi = $('#spi_mosi').val();
            var clk = $('#spi_clk').val();
            var cs = $('#spi_cs').val();
            var invert_bit = $('#spi_invert').val();

            if (miso == 0 && mosi == 0) {
                $('#warn-message').text("You have to specify MISO or MOSI for decoding");
                $('#warning-dialog').show();
                return;
            }

            if (clk == 0) {
                $('#warn-message').text("You have to specify CLK line");
                $('#warning-dialog').show();
                return;
            }

            if (miso == mosi || miso == clk || miso == cs ||
                mosi == clk || mosi == cs || clk == cs) {
                $('#warn-message').text("You can't specify same input lines");
                $('#warning-dialog').show();
                return;
            }

            if (LA.isChannelInUse(miso, LA.state.bus_editing) != -1) {
                $('#warn-message').text($('#spi_data option:selected').text() + " is already in use");
                $('#warning-dialog').show();
                return;
            }

            if (LA.isChannelInUse(mosi, LA.state.bus_editing) != -1) {
                $('#warn-message').text($('#spi_data option:selected').text() + " is already in use");
                $('#warning-dialog').show();
                return;
            }

            if (LA.isChannelInUse(clk, LA.state.bus_editing) != -1) {
                $('#warn-message').text($('#spi_clk option:selected').text() + " is already in use");
                $('#warning-dialog').show();
                return;
            }

            if (LA.isChannelInUse(cs, LA.state.bus_editing) != -1) {
                $('#warn-message').text($('#spi_cs option:selected').text() + " is already in use");
                $('#warning-dialog').show();
                return;
            }

            // OSC.destroyDecoder(bus, "SPI");
            // var upd = OSC.needUpdateDecoder(bus, "SPI");

            // if (!upd)
            //     OSC.buses[bus] = {};
            // else {
            //     if (OSC.buses[bus].clk !== clk)
            //         OSC.hideInfoArrow(parseInt(OSC.buses[bus].clk) - 1);
            //     if (OSC.buses[bus].miso !== miso)
            //         OSC.hideInfoArrow(parseInt(OSC.buses[bus].miso) - 1);
            //     if (OSC.buses[bus].mosi !== mosi)
            //         OSC.hideInfoArrow(parseInt(OSC.buses[bus].mosi) - 1);
            //     if (OSC.buses[bus].cs !== cs)
            //         OSC.hideInfoArrow(parseInt(OSC.buses[bus].cs) - 1);
            // }

            // OSC.buses[bus].name = "SPI";

            LA.buses[bus].config.clk = parseInt(clk)
            LA.buses[bus].config.miso = parseInt(miso)
            LA.buses[bus].config.mosi = parseInt(mosi)
            LA.buses[bus].config.cs = parseInt(cs)
            LA.buses[bus].config.invert_bit = parseInt(invert_bit)

            // OSC.showInfoArrow(parseInt(OSC.buses[bus].clk) - 1);
            // OSC.showInfoArrow(parseInt(OSC.buses[bus].miso) - 1);
            // OSC.showInfoArrow(parseInt(OSC.buses[bus].mosi) - 1);
            // OSC.showInfoArrow(parseInt(OSC.buses[bus].cs) - 1);

            LA.buses[bus].config.bit_order = parseInt($('#spi_order').val())
            LA.buses[bus].config.word_size = parseInt($('#spi_length').val())
            LA.buses[bus].config.cpol = parseInt($('#spi_cpol').val())
            LA.buses[bus].config.cpha = parseInt($('#spi_cpha').val())
            LA.buses[bus].config.cs_polarity = parseInt($('#spi_state').val())
            LA.buses[bus].config.acq_speed = CLIENT.getValue('LA_CUR_FREQ')

            CLIENT.parametersCache['DECODER_' + LA.state.bus_editing] = {value:JSON.stringify(LA.buses[bus])}
            CLIENT.parametersCache['DECODER_ENABLED_' + LA.state.bus_editing] = {value: true}
            CLIENT.sendParameters()

            // if (miso != -1) {
            //     if (OSC.buses[bus].miso_decoder == undefined || OSC.buses[bus].miso_decoder == "") {
            //         OSC.buses[bus].miso_decoder = 'spi' + OSC.state.decoder_id;
            //         OSC.params.local['CREATE_DECODER'] = {
            //             value: 'spi'
            //         };
            //         OSC.params.local['DECODER_NAME'] = {
            //             value: 'spi' + OSC.state.decoder_id
            //         };
            //         OSC.state.decoder_id++;
            //     } else {
            //         var p = OSC.buses[bus];
            //         p['data'] = OSC.buses[bus].miso;
            //         OSC.params.local[OSC.buses[bus].miso_decoder + "_parameters"] = {
            //             value: p
            //         };
            //     }
            //     OSC.sendParams();
            // } else {
            //     if (OSC.buses[bus].miso_decoder != undefined && OSC.buses[bus].miso_decoder != "") {
            //         var p = {};
            //         p['DESTROY_DECODER'] = {
            //             value: OSC.buses[bus].miso_decoder
            //         };
            //         OSC.ws.send(JSON.stringify({
            //             parameters: p
            //         }));
            //         OSC.buses[bus].miso_decoder = "";
            //     }
            // }
            // if (mosi != -1) {
            //     if (OSC.buses[bus].mosi_decoder == undefined || OSC.buses[bus].mosi_decoder == "") {
            //         OSC.buses[bus].mosi_decoder = 'spi' + OSC.state.decoder_id;
            //         OSC.params.local['CREATE_DECODER'] = {
            //             value: 'spi'
            //         };
            //         OSC.params.local['DECODER_NAME'] = {
            //             value: 'spi' + OSC.state.decoder_id
            //         };
            //         OSC.state.decoder_id++;
            //     } else {
            //         var p = OSC.buses[bus];
            //         p['data'] = OSC.buses[bus].mosi;
            //         OSC.params.local[OSC.buses[bus].mosi_decoder + "_parameters"] = {
            //             value: p
            //         };
            //     }
            //     OSC.sendParams();
            // } else {
            //     if (OSC.buses[bus].mosi_decoder != undefined && OSC.buses[bus].mosi_decoder != "") {
            //         var p = {};
            //         p['DESTROY_DECODER'] = {
            //             value: OSC.buses[bus].miso_decoder
            //         };
            //         OSC.ws.send(JSON.stringify({
            //             parameters: p
            //         }));
            //         OSC.buses[bus].mosi_decoder = "";
            //     }
            // }

        }

        if (protocol == "I2C") {
            var scl = $('#i2c_scl').val();
            var sda = $('#i2c_sda').val();
            var invert_bit = $('#i2c_invert').val();

            if (scl == 0 || sda == 0) {
                $('#warn-message').text("You have to specify both lines for decoding");
                $('#warning-dialog').show();
                return;
            }

            if (scl == sda) {
                $('#warn-message').text("You can't specify same lines");
                $('#warning-dialog').show();
                return;
            }

            if (LA.isChannelInUse(scl, LA.state.bus_editing) != -1) {
                $('#warn-message').text($('#i2c_scl option:selected').text() + " is already in use");
                $('#warning-dialog').show();
                return;
            }

            if (LA.isChannelInUse(sda, LA.state.bus_editing) != -1) {
                $('#warn-message').text($('#i2c_sda option:selected').text() + " is already in use");
                $('#warning-dialog').show();
                return;
            }

            // OSC.destroyDecoder(bus, "I2C");
            // var upd = OSC.needUpdateDecoder(bus, "I2C");

            // if (!upd)
            //     OSC.buses[bus] = {};
            // else {
            //     if (OSC.buses[bus].scl !== scl)
            //         OSC.hideInfoArrow(parseInt(OSC.buses[bus].scl) - 1);
            //     if (OSC.buses[bus].sda !== sda)
            //         OSC.hideInfoArrow(parseInt(OSC.buses[bus].sda) - 1);
            // }

            LA.buses[bus].config.scl = parseInt(scl)
            LA.buses[bus].config.sda = parseInt(sda)
            LA.buses[bus].config.address_format = parseInt($('#i2c_addr').val())
            LA.buses[bus].config.acq_speed = CLIENT.getValue('LA_CUR_FREQ')
            LA.buses[bus].config.invert_bit = parseInt(invert_bit)

            CLIENT.parametersCache['DECODER_' + LA.state.bus_editing] = {value:JSON.stringify(LA.buses[bus])}
            CLIENT.parametersCache['DECODER_ENABLED_' + LA.state.bus_editing] = {value: true}
            CLIENT.sendParameters()
            // OSC.showInfoArrow(parseInt(OSC.buses[bus].scl) - 1);
            // OSC.showInfoArrow(parseInt(OSC.buses[bus].sda) - 1);

            // if (!upd) {
            //     OSC.buses[bus].decoder = 'i2c' + OSC.state.decoder_id;
            //     OSC.state.decoder_id++;
            // }

            // if (!upd) {
            //     OSC.params.local['CREATE_DECODER'] = {
            //         value: 'i2c'
            //     };
            //     OSC.params.local['DECODER_NAME'] = {
            //         value: OSC.buses[bus].decoder
            //     };
            // } else {
            //     OSC.params.local[OSC.buses[bus].decoder + "_parameters"] = {
            //         value: OSC.buses[bus]
            //     };
            // }
            // OSC.sendParams();

        }

        if (protocol == "CAN") {
            var nom_bitrate = $('#can_nom_bitrate').val();
            var fast_bitrate = $('#can_fast_bitrate').val();
            var sample_point = $('#sample_point').val();
            var frame_limit = $('#can_frame_limit').val();
            var can_rx = $('#can_rx').val();
            var invert_bit = $('#can_invert').val();


            if (nom_bitrate <= 0) {
                $('#warn-message').text("Nominal bitrate value is incorrect");
                $('#warning-dialog').show();
                return;
            }
            if (fast_bitrate <= 0) {
                $('#warn-message').text("Fast bitrate value is incorrect");
                $('#warning-dialog').show();
                return;
            }

            if (parseInt(fast_bitrate) < parseInt(nom_bitrate)) {
                $('#warn-message').text("Fast bitrate value is incorrect. Fast bitrate must be more than nominal.");
                $('#warning-dialog').show();
                return;
            }

            if (sample_point < 0 || sample_point > 99.99 || sample_point == "") {
                $('#warn-message').text("Sample point value is incorrect. Must in (0-99.99)%");
                $('#warning-dialog').show();
                return;
            }

            if (frame_limit < 10 || frame_limit > 500 || frame_limit == "") {
                $('#warn-message').text("Meximum detected frames must in (10-500)");
                $('#warning-dialog').show();
                return;
            }

            if (can_rx == 0) {
                $('#warn-message').text("You have to specify line for decoding");
                $('#warning-dialog').show();
                return;
            }

            if (LA.isChannelInUse(can_rx, LA.state.bus_editing) != -1) {
                $('#warn-message').text($('#can_rx option:selected').text() + " is already in use");
                $('#warning-dialog').show();
                return;
            }


            // OSC.destroyDecoder(bus, "CAN");
            // var upd = OSC.needUpdateDecoder(bus, "CAN");

            // if (!upd)
            //     OSC.buses[bus] = {};
            // else
            //     OSC.hideInfoArrow(parseInt(OSC.buses[bus].can_rx) - 1);

            LA.buses[bus].config.rx = parseInt(can_rx)
            LA.buses[bus].config.nominal_bitrate = parseInt(nom_bitrate)
            LA.buses[bus].config.fast_bitrate = parseInt(fast_bitrate)
            LA.buses[bus].config.sample_point = parseFloat(sample_point)
            LA.buses[bus].config.acq_speed = CLIENT.getValue('LA_CUR_FREQ')
            LA.buses[bus].config.invert_bit = parseInt(invert_bit)
            LA.buses[bus].config.frame_limit = parseInt(frame_limit)

            CLIENT.parametersCache['DECODER_' + LA.state.bus_editing] = {value:JSON.stringify(LA.buses[bus])}
            CLIENT.parametersCache['DECODER_ENABLED_' + LA.state.bus_editing] = {value: true}
            CLIENT.sendParameters()

            // LA.showInfoArrow(parseInt(OSC.buses[bus].can_rx) - 1);

            // if (!upd) {
            //     OSC.buses[bus].decoder = 'can' + OSC.state.decoder_id;
            //     OSC.state.decoder_id++;
            // }

            // if (!upd) {
            //     OSC.params.local['CREATE_DECODER'] = {
            //         value: 'can'
            //     };
            //     OSC.params.local['DECODER_NAME'] = {
            //         value: OSC.buses[bus].decoder
            //     };
            // } else {
            //     OSC.params.local[OSC.buses[bus].decoder + "_parameters"] = {
            //         value: OSC.buses[bus]
            //     };
            // }

            // OSC.sendParams();
        }

        if (protocol == '-') {
            CLIENT.parametersCache["DECODER_" + LA.state.bus_editing ] = { value : "" }
            CLIENT.parametersCache["DECODER_ENABLED_" + LA.state.bus_editing ] = { value : false }
            CLIENT.sendParameters();
        }

        // $("#BUS" + OSC.state.bus_editing + "_ENABLED").find('img').show();
        // $("#BUS" + OSC.state.bus_editing + "_NAME").text(protocol);
        // $("#DATA_BUS" + (OSC.state.bus_editing - 1)).text(protocol);

        $('#decoder_dialog').modal('hide');
        LA.state.bus_editing = 0;
    }

    LA.updateUIFromConfig = function(){
        for(var i = 1 ; i <= 4; i++){
            var b = 'bus'+i
            if (LA.buses[b] !== undefined && LA.buses[b].name !== undefined){
                $("#BUS" + i + "_NAME").text(LA.buses[b].name);
            }else{
                $("#BUS" + i + "_NAME").text('BUS'+(i-1));
            }
        }
        for(var i = 0 ; i < 8; i++){
            LA.showInfoArrow(i)
        }
    }


    LA.loadBUSSettingsFromConfig = function(){

        var getName = function(val){
            if (val === '#uart_decoder') return 'UART'
            if (val === '#i2c_decoder') return 'I2C'
            if (val === '#spi_decoder') return 'SPI'
            if (val === '#can_decoder') return 'CAN'
            if (val === '#none_decoder') return '-'
            console.error("Unknown coder",val)
            return ''
        }

        var idx = LA.state.bus_editing
        var bus = 'bus'+idx
        var protocol = $('#protocol_selector').val()
        var name = getName(protocol);

        $('.decoder-window').hide();

        if (LA.buses[bus].name === undefined || LA.buses[bus].name !== name){
            if (name !== '-')
                LA.buses[bus] = { name :name , config : JSON.parse(LA.buses.def[name])}
        }

        if (name === 'UART'){
            $(protocol).show()

            $('#uart_data_length option[value=' + LA.buses[bus].config.num_data_bits + ']').attr('selected', 'selected');
            $('#uart_stop_bits option[value=' + LA.buses[bus].config.num_stop_bits + ']').attr('selected', 'selected');
            $('#uart_parity option[value=' + LA.buses[bus].config.parity + ']').attr('selected', 'selected');
            $('#uart_order option[value=' + LA.buses[bus].config.bitOrder + ']').attr('selected', 'selected');
            $('#uart_order option[value=' + LA.buses[bus].config.invert + ']').attr('selected', 'selected');
            $('#uart_rx option[value=' + (LA.buses[bus].config.rx) + ']').attr('selected', 'selected');
            $('#uart_tx option[value=' + (LA.buses[bus].config.tx) + ']').attr('selected', 'selected');
            $('#uart_baudrate').val(LA.buses[bus].config.baudrate);
        }

        if (name === 'SPI'){
            $(protocol).show()

            $('#spi_clk option[value=' + (LA.buses[bus].config.clk) + ']').attr('selected', 'selected');
            $('#spi_miso option[value=' + (LA.buses[bus].config.miso) + ']').attr('selected', 'selected');
            $('#spi_mosi option[value=' + (LA.buses[bus].config.mosi) + ']').attr('selected', 'selected');
            $('#spi_cs option[value=' + (LA.buses[bus].config.cs) + ']').attr('selected', 'selected');

            $('#spi_order option[value=' + LA.buses[bus].config.word_size + ']').attr('selected', 'selected');
            $('#spi_length option[value=' + LA.buses[bus].config.data_length + ']').attr('selected', 'selected');
            $('#spi_cpol option[value=' + LA.buses[bus].config.cpol + ']').attr('selected', 'selected');
            $('#spi_cpha option[value=' + LA.buses[bus].config.cpha + ']').attr('selected', 'selected');
            $('#spi_state option[value=' + LA.buses[bus].config.cs_polarity + ']').attr('selected', 'selected');
            $('#spi_invert option[value=' + LA.buses[bus].config.invert_bit + ']').attr('selected', 'selected');
        }

        if (name === 'CAN'){
            $(protocol).show()
            $('#can_rx option[value=' + (parseInt(LA.buses[bus].config.rx)) + ']').attr('selected', 'selected');
            $('#can_nom_bitrate').val(LA.buses[bus].config.nominal_bitrate);
            $('#can_fast_bitrate').val(LA.buses[bus].config.fast_bitrate);
            $('#sample_point').val(LA.buses[bus].config.sample_point);
            $('#can_frame_limit').val(LA.buses[bus].config.frame_limit);
            $('#can_invert option[value=' + LA.buses[bus].config.invert_bit + ']').attr('selected', 'selected');
        }

        if (name === 'I2C'){
            $(protocol).show()

            $('#i2c_sda option[value=' + (parseInt(LA.buses[bus].config.sda)) + ']').attr('selected', 'selected');
            $('#i2c_scl option[value=' + (parseInt(LA.buses[bus].config.scl)) + ']').attr('selected', 'selected');
            $('#i2c_addr option[value=' + LA.buses[bus].config.address_format + ']').attr('selected', 'selected');
            $('#i2c_invert option[value=' + LA.buses[bus].config.invert_bit + ']').attr('selected', 'selected');
        }
    }

    LA.startEditBus = function(bus) {

        var getProtocol = function(val){
            if (val === 'UART') return '#uart_decoder'
            if (val === 'I2C') return '#i2c_decoder'
            if (val === 'SPI') return '#spi_decoder'
            if (val === 'CAN') return '#can_decoder'
            console.error("Unknown protocol",val)
            return ''
        }

        $('#warning-dialog').hide();
        var arr = ["BUS1_SETTINGS", "BUS2_SETTINGS", "BUS3_SETTINGS", "BUS4_SETTINGS"];
        var bn = arr.indexOf(bus) + 1;
        if (bn != 0) {
            LA.state.bus_editing = bn;
            var bus = 'bus'+bn
            $('.channels_selector').empty();
            $('.channels_selector').append('<option value="0">-</option>');

            // Replace default name with DIN name
            for (var i = 1; i < 9; i++) {
                $('.channels_selector').append('<option value="' + (i) + '">' + (($('#CH' + i + '_NAME').val() != "") ? $('#CH' + i + '_NAME').val() : $('#CH' + i + '_NAME').attr('placeholder')) + '</option>');
            }

            if (LA.buses[bus].name !== undefined){
                protocol = getProtocol(LA.buses[bus].name)
                $('#protocol_selector option').removeAttr('selected');
                $('#protocol_selector option[value='+protocol+']').attr('selected', 'selected');
                $('#protocol_selector').val(protocol);
            }

            LA.loadBUSSettingsFromConfig()

            $('#decoder_dialog').modal('show');

            // if ($("BUS" + bn + "_NAME").text() != "BUS" + bn) {
            //     var bus = 'bus' + bn;

            //     $('.decoder-window').hide();

            //     // if (LA.buses[bus].name === undefined){

            //     // }

            //     if (LA.buses[bus].name !== undefined) {

            //         if (LA.buses[bus].name == "UART") {
            //             $('#protocol_selector option').removeAttr('selected');
            //             $('#protocol_selector option[value=#uart_decoder]').attr('selected', 'selected');
            //             $('#protocol_selector').val("#uart_decoder");
            //             $('#uart_decoder').show();



            //             $('#uart_data_length option[value=' + LA.buses[bus].num_data_bits + ']').attr('selected', 'selected');
            //             $('#uart_stop_bits option[value=' + LA.buses[bus].num_stop_bits + ']').attr('selected', 'selected');
            //             $('#uart_parity option[value=' + LA.buses[bus].parity + ']').attr('selected', 'selected');
            //             $('#uart_order option[value=' + LA.buses[bus].bitOrder + ']').attr('selected', 'selected');
            //             $('#uart_order option[value=' + LA.buses[bus].invert_rx + ']').attr('selected', 'selected');

            //             $('#uart_serial option[value=' + (LA.buses[bus].rx) + ']').attr('selected', 'selected');
            //             $('#uart_baudrate').val(LA.buses[bus].baudrate);
            //         } else if (LA.buses[bus].name == "CAN") {
            //             $('#protocol_selector option').removeAttr('selected');
            //             $('#protocol_selector option[value=#can_decoder]').attr('selected', 'selected');
            //             $('#protocol_selector').val("#can_decoder");
            //             $('#can_decoder').show();

            //             $('#can_rx option[value=' + (parseInt(OSC.buses[bus].can_rx)) + ']').attr('selected', 'selected');
            //             $('#can_nom_bitrate').val(LA.buses[bus].nominal_bitrate);
            //             $('#can_fast_bitrate').val(LA.buses[bus].fast_bitrate);
            //             $('#sample_point').val(LA.buses[bus].sample_point);
            //             $('#can_frame_limit').val(LA.buses[bus].frame_limit);
            //             $('#can_invert option[value=' + LA.buses[bus].invert_bit + ']').attr('selected', 'selected');

            //         } else if (LA.buses[bus].name == "I2C") {
            //             $('#protocol_selector option').removeAttr('selected');
            //             $('#protocol_selector option[value=#i2c_decoder]').attr('selected', 'selected');
            //             $('#protocol_selector').val("#i2c_decoder");
            //             $('#i2c_decoder').show();

            //             $('#i2c_sda option[value=' + (parseInt(LA.buses[bus].sda)) + ']').attr('selected', 'selected');
            //             $('#i2c_scl option[value=' + (parseInt(LA.buses[bus].scl)) + ']').attr('selected', 'selected');

            //             $('#i2c_addr option[value=' + LA.buses[bus].address_format + ']').attr('selected', 'selected');
            //             $('#i2c_invert option[value=' + LA.buses[bus].invert_bit + ']').attr('selected', 'selected');

            //         } else if (LA.buses[bus].name == "SPI") {
            //             $('#protocol_selector option').removeAttr('selected');
            //             $('#protocol_selector option[value=#spi_decoder]').attr('selected', 'selected');
            //             $('#protocol_selector').val("#spi_decoder");
            //             $('#spi_decoder').show();

            //             $('#spi_clk option[value=' + (LA.buses[bus].clk) + ']').attr('selected', 'selected');
            //             $('#spi_mosi option[value=' + (LA.buses[bus].mosi) + ']').attr('selected', 'selected');
            //             $('#spi_miso option[value=' + (LA.buses[bus].miso) + ']').attr('selected', 'selected');
            //             $('#spi_cs option[value=' + (LA.buses[bus].cs) + ']').attr('selected', 'selected');

            //             $('#spi_order option[value=' + LA.buses[bus].word_size + ']').attr('selected', 'selected');
            //             $('#spi_length option[value=' + LA.buses[bus].data_length + ']').attr('selected', 'selected');
            //             $('#spi_cpol option[value=' + LA.buses[bus].cpol + ']').attr('selected', 'selected');
            //             $('#spi_cpha option[value=' + LA.buses[bus].cpha + ']').attr('selected', 'selected');
            //             $('#spi_state option[value=' + LA.buses[bus].cs_polarity + ']').attr('selected', 'selected');
            //             $('#spi_invert option[value=' + LA.buses[bus].invert_bit + ']').attr('selected', 'selected');
            //         }
            //     }
            // }
            // $('#decoder_dialog').modal('show');

            // // Check enabled decoders, check existing logic data file and show help link
            // $.get("/lapro_copy_datafile");
            // $.get("/check_datafile_exists").done(function(res) {
            //     if (res == "OK\n")
            //         $('#porblemsLink').show();
            //     else
            //         $('#porblemsLink').hide();
            // }).fail(function(res) {});
        }
    }

    LA.setBUSSettings = function(new_params,idx){
        var j = new_params['DECODER_' + idx].value
        if (j !== ''){
            LA.buses['bus'+ idx] = JSON.parse(j)
        }else{
            LA.buses['bus'+ idx] = {}
        }
        LA.updateUIFromConfig()
    }

    LA.setBUS1Settings = function(new_params){
        LA.setBUSSettings(new_params,'1')
    }

    LA.setBUS2Settings = function(new_params){
        LA.setBUSSettings(new_params,'2')
    }

    LA.setBUS3Settings = function(new_params){
        LA.setBUSSettings(new_params,'3')
    }

    LA.setBUS4Settings = function(new_params){
        LA.setBUSSettings(new_params,'4')
    }

    LA.setDefSetUART = function(new_params){
        LA.buses.def['UART'] = new_params['DECODER_DEF_UART'].value
    }

    LA.setDefSetCAN = function(new_params){
        LA.buses.def['CAN'] = new_params['DECODER_DEF_CAN'].value
    }

    LA.setDefSetSPI = function(new_params){
        LA.buses.def['SPI'] = new_params['DECODER_DEF_SPI'].value
    }

    LA.setDefSetI2C = function(new_params){
        LA.buses.def['I2C'] = new_params['DECODER_DEF_I2C'].value
    }

}(window.LA = window.LA || {}, jQuery));
