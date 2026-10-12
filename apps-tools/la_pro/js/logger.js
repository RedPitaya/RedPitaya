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
            if (data === undefined) continue
            for (var line in data.values) {
                var values = data.values[line]
                for (var i = 0; i < values.length; i++) {
                    var new_item = values[i]
                    new_item['bus'] = ch
                    new_item['protocol'] = data['name']
                    new_item['line'] = line
                    LOGGER.log_data_raw.push(new_item)
                }
            }
        }
        LOGGER.buildLog()
    }


    // Row text of the DATA list
    LOGGER.logRowText = function(item, radix){
        var value = "ERROR"
        if (item.protocol == "SPI") value = SPI.getValue(item, radix)
        if (item.protocol == "CAN") value = CAN.getValue(item, radix)
        if (item.protocol == "UART") value = UART.getValue(item, radix)
        if (item.protocol == "I2C") value = I2C.getValue(item, radix)
        return '[' + item.protocol + '<' + item.line + '>] ' + value
    }

    LOGGER.LOG_ROW_HEIGHT = 20  // px, .data_row

    LOGGER.buildLog = function(){
        var log = []
        for (var i = 0; i < LOGGER.log_data_raw.length; i++) {
            const item = LOGGER.log_data_raw[i]
            if (LOGGER.getLogBUS(item.bus))
                log.push(item)
        }
        log.sort((a, b) => a.s - b.s);
        LOGGER.log_array = log
        LOGGER.renderLog(true)
        LOGGER.scrollDataArea()
        LOGGER.buildDecoderData()
    }

    // Only the rows in sight exist in the DOM: a spacer gives the list its full height
    LOGGER.renderLog = function(rebuild){
        var box = document.getElementById('log-container')
        if (!box) return
        if (rebuild || !LOGGER.log_spacer) {
            box.innerHTML = ''
            LOGGER.log_spacer = document.createElement('div')
            LOGGER.log_spacer.style.position = 'relative'
            LOGGER.log_rows = document.createElement('div')
            LOGGER.log_rows.style.position = 'absolute'
            LOGGER.log_rows.style.left = '0'
            LOGGER.log_rows.style.right = '0'
            LOGGER.log_spacer.appendChild(LOGGER.log_rows)
            box.appendChild(LOGGER.log_spacer)
            box.onscroll = function() { LOGGER.renderLog(false) }
        }
        const h = LOGGER.LOG_ROW_HEIGHT
        const n = LOGGER.log_array.length
        LOGGER.log_spacer.style.height = (n * h) + 'px'
        const first = Math.max(0, Math.floor(box.scrollTop / h) - 10)
        const last = Math.min(n, Math.ceil((box.scrollTop + box.clientHeight) / h) + 10)
        if (!rebuild && LOGGER.log_window && LOGGER.log_window[0] === first && LOGGER.log_window[1] === last) return
        LOGGER.log_window = [first, last]
        const radix = CLIENT.getValue('LA_LOGGER_RADIX')
        const frag = document.createDocumentFragment()
        for (var i = first; i < last; i++) {
            var div = document.createElement('div')
            div.className = 'data_row'
            div.setAttribute('offset', LOGGER.log_array[i].s)
            div.textContent = LOGGER.logRowText(LOGGER.log_array[i], radix)
            frag.appendChild(div)
        }
        LOGGER.log_rows.style.top = (first * h) + 'px'
        LOGGER.log_rows.replaceChildren(frag)
    }

    LOGGER.clearLog = function() {
        $('#log-container').empty();
        LOGGER.log_spacer = undefined
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
            return 180;
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

    // Text of the table cells of one item, in the order of the current columns
    LOGGER.tableCells = function(item, cols, ctx){
        var cells = []
        for (var k = 0; k < cols.length; k++) {
            var c = cols[k], x = ""
            if (c == LOGGER.COLUMNS.TIME_OFFSET) {
                x = ctx.samplerate !== undefined ? COMMON.convertTimeFromSec(1.0 / ctx.samplerate * (item.s - ctx.ts)) : "ERROR"
            } else if (c == LOGGER.COLUMNS.TIME_LEN) {
                x = ctx.samplerate !== undefined ? COMMON.convertTimeFromSec(1.0 / ctx.samplerate * item.l) : "ERROR"
            } else if (c == LOGGER.COLUMNS.LINE_NAME) {
                x = item.protocol.toUpperCase() + '/' + item.ln.toUpperCase() + ' <' + item.line + '>'
            } else if (c == LOGGER.COLUMNS.DATA_COMMAND || c == LOGGER.COLUMNS.COMMAND) {
                var dec = {SPI: SPI, CAN: CAN, UART: UART, I2C: I2C}[item.protocol]
                x = dec === undefined ? "ERROR" : (c == LOGGER.COLUMNS.COMMAND ? dec.getAnnotation(item.c) : dec.getValue(item, ctx.radix))
            } else if (c == LOGGER.COLUMNS.DATA) {
                x = COMMON.formatData(item.d, "", "", ctx.radix)[0]
            } else if (c == LOGGER.COLUMNS.SAMPLES_START) {
                x = item.s
            } else if (c == LOGGER.COLUMNS.SAMPLES_LEN) {
                x = item.l
            }
            cells.push(String(x).replace(/(\r\n|\r|\n)/g, ''))
        }
        return cells
    }

    LOGGER.TABLE_ROW_HEIGHT = 19  // px, measured from a rendered row in renderTable

    // Rows passing the filter; the table shows them through renderTable
    LOGGER.buildDecoderData = function(){
        const filter = $('#LA_DATA_FILTER').val()
        const total_samp = CLIENT.getValue('LA_TOTAL_SAMPLES')
        var body = document.getElementById("decoded_body");
        LOGGER.table_rows = []
        if (total_samp == undefined) {
            body.innerHTML = ''
            return
        }
        LOGGER.table_ctx = {radix: CLIENT.getValue('LA_LOGGER_RADIX'), ts: CLIENT.getValue('LA_PRE_TRIGGER_SAMPLES'),
                            samplerate: CLIENT.getValue("LA_CUR_FREQ"), total: total_samp}
        for (var z = 0; z < LOGGER.log_array.length; z++) {
            if (filter === "" || LOGGER.tableCells(LOGGER.log_array[z], LOGGER.log_header, LOGGER.table_ctx).some(x => x.includes(filter)))
                LOGGER.table_rows.push(z)
        }
        if (!body.onscroll)
            body.onscroll = function() { LOGGER.renderTable(false) }
        LOGGER.renderTable(true)
    }

    // Only the rows in sight exist in the DOM, between two spacer rows
    LOGGER.renderTable = function(rebuild){
        var body = document.getElementById("decoded_body");
        if (!body || LOGGER.table_rows === undefined) return
        const h = LOGGER.TABLE_ROW_HEIGHT
        const n = LOGGER.table_rows.length
        const first = Math.max(0, Math.floor(body.scrollTop / h) - 10)
        const last = Math.min(n, Math.ceil((body.scrollTop + Math.max(body.clientHeight, 400)) / h) + 10)
        if (!rebuild && LOGGER.table_window && LOGGER.table_window[0] === first && LOGGER.table_window[1] === last) return
        LOGGER.table_window = [first, last]
        const frag = document.createDocumentFragment()
        const spacer = (height) => {
            var tr = document.createElement("tr")
            tr.style.height = height + 'px'
            tr.style.border = '0'
            return tr
        }
        frag.appendChild(spacer(first * h))
        for (var r = first; r < last; r++) {
            const item = LOGGER.log_array[LOGGER.table_rows[r]]
            var tr = document.createElement("tr");
            tr.setAttribute("pos", item.s / LOGGER.table_ctx.total)
            tr.onmouseup = LOGGER.onClickRow
            const cells = LOGGER.tableCells(item, LOGGER.log_header, LOGGER.table_ctx)
            for (var k = 0; k < cells.length; k++) {
                var td = document.createElement("td");
                td.classList.add("column_" + LOGGER.log_header[k]);
                td.classList.add("text-clip")
                td.innerText = cells[k]
                tr.appendChild(td)
            }
            frag.appendChild(tr)
        }
        frag.appendChild(spacer((n - last) * h))
        body.replaceChildren(frag)
        // the spacers must match the real row height
        if (last > first && body.children[1] && body.children[1].offsetHeight > 0 && body.children[1].offsetHeight !== h) {
            LOGGER.TABLE_ROW_HEIGHT = body.children[1].offsetHeight
            LOGGER.renderTable(true)
            return
        }
        LOGGER.setDecodedSize()
    }

    LOGGER.clearDecoderValues = function(){
        var body = document.getElementById("decoded_body");
        body.innerHTML = ''
        LOGGER.table_window = undefined
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
        // first row at or after the start of the view
        var lo = 0, hi = LOGGER.log_array.length
        while (lo < hi) {
            const mid = (lo + hi) >> 1
            if (LOGGER.log_array[mid].s < start) lo = mid + 1; else hi = mid
        }
        $('#log-container').scrollTop(LOGGER.LOG_ROW_HEIGHT * lo);
        LOGGER.renderLog(false)
    }


}(window.LOGGER = window.LOGGER || {}, jQuery));