(function(LOGGER, $, undefined) {
    LOGGER.log_data_raw = [];
    LOGGER.log_array = []
    LOGGER.log_header = []

    LOGGER.COLUMNS = {
        LINE_NAME       : 1,
        TIME_OFFSET     : 2,
        SAMPLES_START   : 3,
        SAMPLES_LEN     : 4,
        TIME_LEN        : 5,
        DATA            : 6,
        DATA_COMMAND    : 7,
        COMMAND         : 8
    };

    LOGGER.setupDataToLogger = function(){
        LOGGER.log_data_raw = []
        for(var ch = 1; ch <= 4; ch++){
            var data = LA.decodedData[ch]
            for (var line in data.values) {
                var values = data.values[line]
                for (var item in values) {
                    var new_item = values[item]
                    new_item['bus'] = ch
                    new_item['protocol'] = data['name']
                    new_item['line'] = line
                    LOGGER.log_data_raw.push(new_item)
                }
            }
        }
        LOGGER.log_data_raw.sort((a, b) => {
            if (a.s < b.s) return -1
            if (a.s > b.s) return 1
            return 0
        });

        LOGGER.buildLog()
    }


    LOGGER.buildLog = function(){
        // const sbt = CLIENT.getValue('LA_PRE_TRIGGER_SAMPLES')
        const radix = CLIENT.getValue('LA_LOGGER_RADIX')
        var log = []
        var divArray = []
        for(var idx in LOGGER.log_data_raw){
            const item = LOGGER.log_data_raw[idx]
            if (LOGGER.getLogBUS(item.bus)){
                // item["trig_off"] = item.s - (sbt !== undefined ? sbt : 0)
                log.push(item)
            }
        }
        log.sort((a, b) => {
            if (a.s < b.s) return -1
            if (a.s > b.s) return 1
            return 0
        });

        for(var idx in log){
            var value = "ERROR"
            if (log[idx].protocol == "SPI"){
                value = SPI.getValue(log[idx],radix)
            }
            if (log[idx].protocol == "CAN"){
                value = CAN.getValue(log[idx],radix)
            }
            if (log[idx].protocol == "UART"){
                value = UART.getValue(log[idx],radix)
            }
            if (log[idx].protocol == "I2C"){
                value = I2C.getValue(log[idx],radix)
            }
            value = '[' + log[idx].protocol + '<' + log[idx].line  +'>] ' + value
            divArray.push("<div class='data_row' offset='" + log[idx].s + "'>" + value + "</div>")
        }
        LOGGER.log_array = log
        $('#log-container').html(divArray);
        LOGGER.scrollDataArea()
        LOGGER.buildDecoderData()
    }

    LOGGER.clearLog = function() {
        $('#log-container').empty();
    }

    LOGGER.getColumns = function(width){
        var c = []
        c.push(LOGGER.COLUMNS.LINE_NAME)
        if (width < 340){
            c.push(LOGGER.COLUMNS.DATA_COMMAND)
        }else{
            c.push(LOGGER.COLUMNS.COMMAND)
            c.push(LOGGER.COLUMNS.DATA)
            if (width > 500){
                c.push(LOGGER.COLUMNS.TIME_OFFSET)
            }
            if (width > 560){
                c.push(LOGGER.COLUMNS.TIME_LEN)
            }
            if (width > 800){
                c.push(LOGGER.COLUMNS.SAMPLES_LEN)
            }
            if (width > 900){
                c.push(LOGGER.COLUMNS.SAMPLES_START)
            }
        }
        return c
    }

    LOGGER.getFixedWidth = function(col){
        if (col == LOGGER.COLUMNS.TIME_OFFSET){
            return 100;
        }

        if (col == LOGGER.COLUMNS.TIME_LEN){
            return 90;
        }

        if (col == LOGGER.COLUMNS.LINE_NAME){
            return 90;
        }

        if (col == LOGGER.COLUMNS.DATA_COMMAND){
            return 0;
        }

        if (col == LOGGER.COLUMNS.COMMAND){
            return 0;
        }

        if (col == LOGGER.COLUMNS.DATA){
            return 120;
        }

        if (col == LOGGER.COLUMNS.SAMPLES_START){
            return 130;
        }

        if (col == LOGGER.COLUMNS.SAMPLES_LEN){
            return 130;
        }
    }

    LOGGER.setDecodedSize = function(){
        const tw = $('#decoded_table').width() - 20
        var fixedW = 0;
        var fixedC = 0
        for(var idx in LOGGER.log_header){
            var l = LOGGER.getFixedWidth(LOGGER.log_header[idx])
            if (l !== 0){
                fixedW += l
                fixedC++
            }
        }
        if (LOGGER.log_header.includes(LOGGER.COLUMNS.TIME_OFFSET)){
            var w = LOGGER.getFixedWidth(LOGGER.COLUMNS.TIME_OFFSET)
            if (w == 0){
                w = (tw - fixedW) / (LOGGER.log_header.length - fixedC)
            }
            $('.column_' + LOGGER.COLUMNS.TIME_OFFSET).each(function(i, obj) {
                obj.style.width  = w + "px"
            });
        }

        if (LOGGER.log_header.includes(LOGGER.COLUMNS.TIME_LEN)){
            var w = LOGGER.getFixedWidth(LOGGER.COLUMNS.TIME_LEN)
            if (w == 0){
                w = (tw - fixedW) / (LOGGER.log_header.length - fixedC)
            }
            $('.column_' + LOGGER.COLUMNS.TIME_LEN).each(function(i, obj) {
                obj.style.width  = w + "px"
            });
        }

        if (LOGGER.log_header.includes(LOGGER.COLUMNS.LINE_NAME)){
            var w = LOGGER.getFixedWidth(LOGGER.COLUMNS.LINE_NAME)
            if (w == 0){
                w = (tw - fixedW) / (LOGGER.log_header.length - fixedC)
            }
            $('.column_' + LOGGER.COLUMNS.LINE_NAME).each(function(i, obj) {
                obj.style.width  = w + "px"
            });

        }

        if (LOGGER.log_header.includes(LOGGER.COLUMNS.DATA_COMMAND)){
            var w = LOGGER.getFixedWidth(LOGGER.COLUMNS.DATA_COMMAND)
            if (w == 0){
                w = (tw - fixedW) / (LOGGER.log_header.length - fixedC)
            }
            $('.column_' + LOGGER.COLUMNS.DATA_COMMAND).each(function(i, obj) {
                obj.style.width  = w + "px"
            });
        }

        if (LOGGER.log_header.includes(LOGGER.COLUMNS.COMMAND)){
            var w = LOGGER.getFixedWidth(LOGGER.COLUMNS.COMMAND)
            if (w == 0){
                w = (tw - fixedW) / (LOGGER.log_header.length - fixedC)
            }
            $('.column_' + LOGGER.COLUMNS.COMMAND).each(function(i, obj) {
                obj.style.width  = w + "px"
            });
        }

        if (LOGGER.log_header.includes(LOGGER.COLUMNS.DATA)){
            var w = LOGGER.getFixedWidth(LOGGER.COLUMNS.DATA)
            if (w == 0){
                w = (tw - fixedW) / (LOGGER.log_header.length - fixedC)
            }
            $('.column_' + LOGGER.COLUMNS.DATA).each(function(i, obj) {
                obj.style.width  = w + "px"
            });

        }

        if (LOGGER.log_header.includes(LOGGER.COLUMNS.SAMPLES_START)){
            var w = LOGGER.getFixedWidth(LOGGER.COLUMNS.SAMPLES_START)
            if (w == 0){
                w = (tw - fixedW) / (LOGGER.log_header.length - fixedC)
            }
            $('.column_' + LOGGER.COLUMNS.SAMPLES_START).each(function(i, obj) {
                obj.style.width  = w + "px"
            });

        }

        if (LOGGER.log_header.includes(LOGGER.COLUMNS.SAMPLES_LEN)){
            var w = LOGGER.getFixedWidth(LOGGER.COLUMNS.SAMPLES_LEN)
            if (w == 0){
                w = (tw - fixedW) / (cols.length - fixedC)
            }
            $('.column_' + LOGGER.COLUMNS.SAMPLES_LEN).each(function(i, obj) {
                obj.style.width  = w + "px"
            });

        }
    }

    LOGGER.buildDecodedHeader = function(){
        const tw = $('#decoded_table').width() - 20
        const cols = LOGGER.getColumns(tw)
        if (cols.toString() === LOGGER.log_header.toString()) return false
        LOGGER.log_header = cols
        var body = document.getElementById("decoded_header");
        body.innerHTML = ''
        var tr = document.createElement("tr");
        body.appendChild(tr);


        if (LOGGER.log_header.includes(LOGGER.COLUMNS.TIME_OFFSET)){
            var td = document.createElement("th");
            td.classList.add("column_"+LOGGER.COLUMNS.TIME_OFFSET);
            td.innerText = "Time offset"
            tr.appendChild(td)
        }

        if (LOGGER.log_header.includes(LOGGER.COLUMNS.TIME_LEN)){
            var td = document.createElement("th");
            td.classList.add("column_"+LOGGER.COLUMNS.TIME_LEN);
            td.innerText = "Time"
            tr.appendChild(td)
        }

        if (LOGGER.log_header.includes(LOGGER.COLUMNS.LINE_NAME)){
            var td = document.createElement("th");
            td.classList.add("column_"+LOGGER.COLUMNS.LINE_NAME);
            td.innerText = "Line"
            tr.appendChild(td)
        }

        if (LOGGER.log_header.includes(LOGGER.COLUMNS.DATA_COMMAND)){
            var td = document.createElement("th");
            td.classList.add("column_"+LOGGER.COLUMNS.DATA_COMMAND);
            td.innerText = "Info"
            tr.appendChild(td)
        }

        if (LOGGER.log_header.includes(LOGGER.COLUMNS.COMMAND)){
            var td = document.createElement("th");
            td.classList.add("column_"+LOGGER.COLUMNS.COMMAND);
            td.innerText = "Info"
            tr.appendChild(td)
        }

        if (LOGGER.log_header.includes(LOGGER.COLUMNS.DATA)){
            var td = document.createElement("th");
            td.classList.add("column_"+LOGGER.COLUMNS.DATA);
            td.innerText = "Data"
            tr.appendChild(td)
        }

        if (LOGGER.log_header.includes(LOGGER.COLUMNS.SAMPLES_START)){
            var td = document.createElement("th");
            td.classList.add("column_"+LOGGER.COLUMNS.SAMPLES_START);
            td.innerText = "Samples start"
            tr.appendChild(td)
        }

        if (LOGGER.log_header.includes(LOGGER.COLUMNS.SAMPLES_LEN)){
            var td = document.createElement("th");
            td.classList.add("column_"+LOGGER.COLUMNS.SAMPLES_LEN);
            td.innerText = "Samples count"
            tr.appendChild(td)
        }
        return true
    }

    LOGGER.onClickRow = function(ev){
        console.log(this)
        CLIENT.parametersCache['LA_VIEW_PORT_POS'] = {value: this.getAttribute('pos')}
        CLIENT.sendParameters()
        ev.preventDefault()
        ev.stopPropagation()
    }

    LOGGER.buildDecoderData = function(){
        const tw = $('#decoded_table').width() - 20
        const radix = CLIENT.getValue('LA_LOGGER_RADIX')
        const ts = CLIENT.getValue('LA_PRE_TRIGGER_SAMPLES')
        const total_samp = CLIENT.getValue('LA_TOTAL_SAMPLES')
        var samplerate = CLIENT.getValue("LA_CUR_FREQ")
        var body = document.getElementById("decoded_body");
        body.innerHTML = ''
        var fixedW = 0;
        var fixedC = 0
        if (total_samp == undefined) return
        for(var idx in LOGGER.log_header){
            var l = LOGGER.getFixedWidth(LOGGER.log_header[idx])
            if (l !== 0){
                fixedW += l
                fixedC++
            }
        }
        for(var z = 0; z < LOGGER.log_array.length; z++){
            const item =  LOGGER.log_array[z]

            var tr = document.createElement("tr");
            tr.setAttribute("pos",item.s / total_samp)
            body.appendChild(tr);
            tr.onmouseup = LOGGER.onClickRow

            if (LOGGER.log_header.includes(LOGGER.COLUMNS.TIME_OFFSET)){
                var td = document.createElement("td");
                td.classList.add("column_"+LOGGER.COLUMNS.TIME_OFFSET);
                if (samplerate !== undefined){
                    var t = 1.0 / samplerate * (item.s - ts)
                    td.innerText = COMMON.convertTimeFromSec(t)
                }else{
                    td.innerText = "ERROR"
                }
                tr.appendChild(td)
            }

            if (LOGGER.log_header.includes(LOGGER.COLUMNS.TIME_LEN)){
                var td = document.createElement("td");
                td.classList.add("column_"+LOGGER.COLUMNS.TIME_LEN);
                if (samplerate !== undefined){
                    var t = 1.0 / samplerate * (item.l)
                    td.innerText = COMMON.convertTimeFromSec(t)
                }else{
                    td.innerText = "ERROR"
                }
                tr.appendChild(td)
            }

            if (LOGGER.log_header.includes(LOGGER.COLUMNS.LINE_NAME)){
                var x = item.protocol.toUpperCase() + '/' + item.ln.toUpperCase() + ' <' + item.line + '>'
                var td = document.createElement("td");
                td.classList.add("column_"+LOGGER.COLUMNS.LINE_NAME);
                td.innerText = x
                tr.appendChild(td)
            }

            if (LOGGER.log_header.includes(LOGGER.COLUMNS.DATA_COMMAND)){
                var x = "ERROR"
                var td = document.createElement("td");
                td.classList.add("column_"+LOGGER.COLUMNS.DATA_COMMAND);

                if (item.protocol == "SPI"){
                    x = SPI.getValue(item,radix)
                }
                if (item.protocol == "CAN"){
                    x = CAN.getValue(item,radix)
                }
                if (item.protocol == "UART"){
                    x = UART.getValue(item,radix)
                }
                if (item.protocol == "I2C"){
                    x = I2C.getValue(item,radix)
                }
                td.innerText = x
                tr.appendChild(td)
            }

            if (LOGGER.log_header.includes(LOGGER.COLUMNS.COMMAND)){
                var x = "ERROR"
                var td = document.createElement("td");
                td.classList.add("column_"+LOGGER.COLUMNS.COMMAND);

                if (item.protocol == "SPI"){
                    x = SPI.getAnnotation(item.c)
                }
                if (item.protocol == "CAN"){
                    x = CAN.getAnnotation(item.c)
                }
                if (item.protocol == "UART"){
                    x = UART.getAnnotation(item.c)
                }
                if (item.protocol == "I2C"){
                    x = I2C.getAnnotation(item.c)
                }

                td.innerText = x
                tr.appendChild(td)
            }

            if (LOGGER.log_header.includes(LOGGER.COLUMNS.DATA)){
                var x = item.protocol.toUpperCase() + '/' + item.ln.toUpperCase()
                var td = document.createElement("td");
                td.classList.add("column_"+LOGGER.COLUMNS.DATA);
                td.innerText = COMMON.formatData(item.d, "", "", radix)[0]
                tr.appendChild(td)
            }

            if (LOGGER.log_header.includes(LOGGER.COLUMNS.SAMPLES_START)){
                var td = document.createElement("td");
                td.classList.add("column_"+LOGGER.COLUMNS.SAMPLES_START);
                td.innerText = item.s
                tr.appendChild(td)
            }

            if (LOGGER.log_header.includes(LOGGER.COLUMNS.SAMPLES_LEN)){
                var td = document.createElement("td");
                td.classList.add("column_"+LOGGER.COLUMNS.SAMPLES_LEN);
                td.innerText = item.l
                tr.appendChild(td)
            }
        }
        LOGGER.setDecodedSize()
    }

    LOGGER.clearDecoderValues = function(){
        var body = document.getElementById("decoded_body");
        body.innerHTML = ''
    }

    LOGGER.loadDecoderValues = function(){
        if (LOGGER.buildDecodedHeader()){
            LOGGER.buildDecoderData()
        }else{
            LOGGER.setDecodedSize()
        }
    }


    LOGGER.getLogBUS = function(idx) {
        const v = CLIENT.getValue('LA_LOGGER_BUS_'+idx)
        if (v !== undefined) return v
        return false
    }

    LOGGER.setLoggerBUS = function(idx,state){
        var z = $('#DATA_BUS'+idx)
        if (state){
            if (!z.hasClass('active')){
                z.addClass('active')
            }
        }else{
            z.removeClass('active')
        }
        LOGGER.buildLog()
    }

    LOGGER.setLoggerBUS1 = function(new_param){
        LOGGER.setLoggerBUS(0,new_param['LA_LOGGER_BUS_1'].value)
    }

    LOGGER.setLoggerBUS2 = function(new_param){
        LOGGER.setLoggerBUS(1,new_param['LA_LOGGER_BUS_2'].value)
    }

    LOGGER.setLoggerBUS3 = function(new_param){
        LOGGER.setLoggerBUS(2,new_param['LA_LOGGER_BUS_3'].value)
    }

    LOGGER.setLoggerBUS4 = function(new_param){
        LOGGER.setLoggerBUS(3,new_param['LA_LOGGER_BUS_4'].value)
    }

    LOGGER.setLoggerRadix = function(new_param){
        $('#LOGGER_RADIX').val(new_param['LA_LOGGER_RADIX'].value)
        LOGGER.buildLog()
    }

    LOGGER.scrollDataArea = function() {
        const start = LA.region_samples.start
        // Scroll
        var div_index = 0;
        for (var i = 0; i < LOGGER.log_array.length; i++) {
            if (LOGGER.log_array[i].s < start)
                div_index++;
            else
                break;
        }
        $('#log-container').scrollTop(20 * div_index);
    }


}(window.LOGGER = window.LOGGER || {}, jQuery));