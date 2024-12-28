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

    LA.requestRedecode = function(){
        CLIENT.parametersCache['LA_RUN'] = { value: 2 };
        CLIENT.sendParameters();
    }

    LA.getDecoderName = function(ch){
        var bus = 'bus' + ch;
        return LA.buses[bus] ? LA.buses[bus].name : ""
    }

    LA.isChannelInUse = function(ch, ex_bus) {
        if (ch == 0) return -1
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
        var info_text = COMMON.accordingChanName(ch + 1);

        if (info_text !== "") {
            arrow_info.text(info_text);
            arrow_img.show();
        }else{
            arrow_info.text("");
            arrow_img.hide();
        }
    }

    LA.repackDecodedData = function(data, idx){
        var settings = LA.buses['bus'+idx]
        if (settings !== undefined){
            if (settings.name !== undefined){
                if (settings.name == "UART"){
                    var rx = settings.config.rx
                    var tx = settings.config.tx
                    var rx_data = []
                    var tx_data = []
                    for (let i = 0; i < data.value.length; i++) {
                        var item = data.value[i];
                        if (item.ln === 'rx'){
                            rx_data.push(item)
                        }
                        if (item.ln === 'tx'){
                            tx_data.push(item)
                        }
                    }
                    var ret = {}
                    if (rx !== 0)
                        ret[rx] = rx_data
                    if (tx !== 0)
                        ret[tx] = tx_data
                    return ret
                }

                if (settings.name == "CAN"){
                    var rx = settings.config.rx
                    var rx_data = []
                    for (let i = 0; i < data.value.length; i++) {
                        var item = data.value[i];
                        if (item.ln === 'rx'){
                            rx_data.push(item)
                        }
                    }
                    var ret = {}
                    ret[rx] = rx_data
                    return ret
                }

                if (settings.name == "I2C"){
                    var sda = settings.config.sda
                    var sda_data = []
                    for (let i = 0; i < data.value.length; i++) {
                        var item = data.value[i];
                        if (item.ln === 'sda'){
                            sda_data.push(item)
                        }
                    }
                    var ret = {}
                    ret[sda] = sda_data
                    return ret
                }

                if (settings.name == "SPI"){
                    var miso = settings.config.miso
                    var mosi = settings.config.mosi
                    var miso_data = []
                    var mosi_data = []
                    for (let i = 0; i < data.value.length; i++) {
                        var item = data.value[i];
                        if (item.ln === 'miso'){
                            miso_data.push(item)
                        }
                        if (item.ln === 'mosi'){
                            mosi_data.push(item)
                        }
                    }
                    var ret = {}
                    if (miso !== 0)
                        ret[miso] = miso_data
                    if (mosi !== 0)
                        ret[mosi] = mosi_data
                    return ret
                }
            }
        }
    }

    LA.applyDecoder = function() {
        var protocol = $('#protocol_selector option:selected').text();
        $('#warning-dialog').hide();
        // Saving local data
        var bus = 'bus' + LA.state.bus_editing;
        if (protocol == "UART") {
            var baudrate = $('#uart_baudrate').val();
            var rx = $('#uart_rx').val();
            var tx = $('#uart_tx').val();
            // var uart_speed = $('#uart_adc_speed').val();


            // if (uart_speed <= 0) {
            //     $('#warn-message').text("Samplerate must be greater than 0");
            //     $('#warning-dialog').show();
            //     return;
            // }

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

            LA.buses[bus].config.rx = parseInt(rx);
            LA.buses[bus].config.tx = parseInt(tx);

            LA.buses[bus].config.baudrate = parseInt(baudrate);
            LA.buses[bus].config.num_data_bits = parseInt($('#uart_data_length').val())
            LA.buses[bus].config.num_stop_bits = parseInt($('#uart_stop_bits').val())
            LA.buses[bus].config.parity = parseInt($('#uart_parity').val())
            LA.buses[bus].config.bitOrder = parseInt($('#uart_order').val())
            LA.buses[bus].config.invert = parseInt($('#uart_invert').val())
            // LA.buses[bus].config.acq_speed = parseInt(uart_speed)

            CLIENT.parametersCache['DECODER_' + LA.state.bus_editing] = {value:JSON.stringify(LA.buses[bus])}
            CLIENT.parametersCache['DECODER_ENABLED_' + LA.state.bus_editing] = {value: true}
            CLIENT.sendParameters()
        }

        if (protocol == "SPI") {

            var miso = $('#spi_miso').val();
            var mosi = $('#spi_mosi').val();
            var clk = $('#spi_clk').val();
            var cs = $('#spi_cs').val();
            var invert_bit = $('#spi_invert').val();
            // var spi_speed = $('#spi_adc_speed').val();

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

            // if (spi_speed <= 0) {
            //     $('#warn-message').text("Samplerate must be greater than 0");
            //     $('#warning-dialog').show();
            //     return;
            // }


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

            LA.buses[bus].config.clk = parseInt(clk)
            LA.buses[bus].config.miso = parseInt(miso)
            LA.buses[bus].config.mosi = parseInt(mosi)
            LA.buses[bus].config.cs = parseInt(cs)
            LA.buses[bus].config.invert_bit = parseInt(invert_bit)

            LA.buses[bus].config.bit_order = parseInt($('#spi_order').val())
            LA.buses[bus].config.word_size = parseInt($('#spi_length').val())
            LA.buses[bus].config.cpol = parseInt($('#spi_cpol').val())
            LA.buses[bus].config.cpha = parseInt($('#spi_cpha').val())
            LA.buses[bus].config.cs_polarity = parseInt($('#spi_state').val())
            // LA.buses[bus].config.acq_speed = parseInt(spi_speed)

            CLIENT.parametersCache['DECODER_' + LA.state.bus_editing] = {value:JSON.stringify(LA.buses[bus])}
            CLIENT.parametersCache['DECODER_ENABLED_' + LA.state.bus_editing] = {value: true}
            CLIENT.sendParameters()
        }

        if (protocol == "I2C") {
            var scl = $('#i2c_scl').val();
            var sda = $('#i2c_sda').val();
            var invert_bit = $('#i2c_invert').val();
            // var i2c_speed = $('#i2c_adc_speed').val();

            // if (i2c_speed <= 0) {
            //     $('#warn-message').text("Samplerate must be greater than 0");
            //     $('#warning-dialog').show();
            //     return;
            // }


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

            LA.buses[bus].config.scl = parseInt(scl)
            LA.buses[bus].config.sda = parseInt(sda)
            LA.buses[bus].config.address_format = parseInt($('#i2c_addr').val())
            // LA.buses[bus].config.acq_speed = parseInt(i2c_speed)
            LA.buses[bus].config.invert_bit = parseInt(invert_bit)

            CLIENT.parametersCache['DECODER_' + LA.state.bus_editing] = {value:JSON.stringify(LA.buses[bus])}
            CLIENT.parametersCache['DECODER_ENABLED_' + LA.state.bus_editing] = {value: true}
            CLIENT.sendParameters()
        }

        if (protocol == "CAN") {
            var nom_bitrate = $('#can_nom_bitrate').val();
            var fast_bitrate = $('#can_fast_bitrate').val();
            var sample_point = $('#sample_point').val();
            var can_rx = $('#can_rx').val();
            var invert_bit = $('#can_invert').val();
            // var can_speed = $('#can_adc_speed').val();


            // if (can_speed <= 0) {
            //     $('#warn-message').text("Samplerate must be greater than 0");
            //     $('#warning-dialog').show();
            //     return;
            // }

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

            LA.buses[bus].config.rx = parseInt(can_rx)
            LA.buses[bus].config.nominal_bitrate = parseInt(nom_bitrate)
            LA.buses[bus].config.fast_bitrate = parseInt(fast_bitrate)
            LA.buses[bus].config.sample_point = parseFloat(sample_point)
            // LA.buses[bus].config.acq_speed = parseInt(can_speed)
            LA.buses[bus].config.invert_bit = parseInt(invert_bit)

            CLIENT.parametersCache['DECODER_' + LA.state.bus_editing] = {value:JSON.stringify(LA.buses[bus])}
            CLIENT.parametersCache['DECODER_ENABLED_' + LA.state.bus_editing] = {value: true}
            CLIENT.sendParameters()

        }

        if (protocol == '-') {
            CLIENT.parametersCache["DECODER_" + LA.state.bus_editing ] = { value : "" }
            CLIENT.parametersCache["DECODER_ENABLED_" + LA.state.bus_editing ] = { value : false }
            CLIENT.sendParameters();
        }

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
            $('#uart_invert option[value=' + LA.buses[bus].config.invert + ']').attr('selected', 'selected');
            $('#uart_rx option[value=' + (LA.buses[bus].config.rx) + ']').attr('selected', 'selected');
            $('#uart_tx option[value=' + (LA.buses[bus].config.tx) + ']').attr('selected', 'selected');
            $('#uart_baudrate').val(LA.buses[bus].config.baudrate);
            // $('#uart_adc_speed').val(LA.buses[bus].config.acq_speed);
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
            // $('#spi_adc_speed').val(LA.buses[bus].config.acq_speed);

        }

        if (name === 'CAN'){
            $(protocol).show()
            $('#can_rx option[value=' + (parseInt(LA.buses[bus].config.rx)) + ']').attr('selected', 'selected');
            $('#can_nom_bitrate').val(LA.buses[bus].config.nominal_bitrate);
            $('#can_fast_bitrate').val(LA.buses[bus].config.fast_bitrate);
            $('#sample_point').val(LA.buses[bus].config.sample_point);
            $('#can_invert option[value=' + LA.buses[bus].config.invert_bit + ']').attr('selected', 'selected');
            // $('#can_adc_speed').val(LA.buses[bus].config.acq_speed);

        }

        if (name === 'I2C'){
            $(protocol).show()

            $('#i2c_sda option[value=' + (parseInt(LA.buses[bus].config.sda)) + ']').attr('selected', 'selected');
            $('#i2c_scl option[value=' + (parseInt(LA.buses[bus].config.scl)) + ']').attr('selected', 'selected');
            $('#i2c_addr option[value=' + LA.buses[bus].config.address_format + ']').attr('selected', 'selected');
            $('#i2c_invert option[value=' + LA.buses[bus].config.invert_bit + ']').attr('selected', 'selected');
            // $('#i2c_adc_speed').val(LA.buses[bus].config.acq_speed);

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
