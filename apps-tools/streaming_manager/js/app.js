/*
 * Red Pitaya stream server application: the page.
 *
 * The state lives in the parameters of the module (CLIENT.params.orig, see net.js) plus a few
 * page-only values (S). Every change renders the navigation and the open section again from
 * templates; an input that has the focus is left alone until it loses it. Controls carry
 * data-* attributes: data-go (a section), data-act (an action of ACT), data-param (a parameter
 * written as it is), data-bit (a bit of a mask parameter).
 */
(function () {
    'use strict';

    const C = window.CLIENT;
    const v = (n, d) => C.value(n, d);
    const pmax = (n, d) => (C.params.orig[n] && C.params.orig[n].max !== undefined ? C.params.orig[n].max : d);
    const pmin = (n, d) => (C.params.orig[n] && C.params.orig[n].min !== undefined ? C.params.orig[n].min : d);
    const has = (n) => C.params.orig[n] !== undefined;

    const esc = (x) => String(x).replace(/[&<>"']/g, (c) => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));
    const fmtRate = (r) => (r >= 1e6 ? +(r / 1e6).toFixed(3) + ' MS/s' : r >= 1e3 ? +(r / 1e3).toFixed(2) + ' kS/s' : +r.toFixed(1) + ' S/s');
    const fmtBytes = (b) => (b >= 1073741824 ? (b / 1073741824).toFixed(2) + ' GB' : b >= 1048576 ? (b / 1048576).toFixed(2) + ' MB' : b >= 1024 ? +(b / 1024).toFixed(1) + ' kB' : b + ' B');
    const MB = (b) => (b / 1048576).toFixed(2) + ' MB';

    // ---------------------------------------------------------------------------------------
    // page-only state
    const S = {
        section: 'overview',
        fileTab: 'adc',
        fileSel: {},
        files: { adc: [], dac: [], gpio: [] },
        filesError: '',
        clients: [],
        ip: '',
        sdFree: null,    // [free, total] bytes of the card
        setMsg: '',      // the answer to the last settings command
        upload: '',      // the text of a running upload
        uploadBad: false,  // that text is a refusal or a failure (red)
        uploadDir: '',   // its folder: dac or gpio
        memBlockCustom: false
    };

    const ON = 'background: #cdcccc; color: #111111; font-weight: 700';
    const OFF = '';
    const NEUTRAL = 'background: transparent; color: #cdcccc; border-color: #999';

    // ---------------------------------------------------------------------------------------
    // the streams and their states (web_server MAIN: status codes of the stop reasons)
    const ADC_STATE = { 0: 'Stopped', 1: 'Running', 2: 'SD card full', 3: 'Recording done', 4: 'Not enough memory', 5: 'Stopped: memory changed', 6: 'No active channels', 7: 'Configuration error', 8: 'Client disconnected' };
    const DAC_STATE = { 0: 'Stopped', 1: 'Running', 2: 'Done', 3: 'File is empty', 4: 'File is broken', 5: 'File is missing', 6: 'Not enough memory', 7: 'Stopped: memory changed', 8: 'No active channels', 9: 'Configuration error', 10: 'Client disconnected' };
    const GPIO_STATE = { 0: 'Stopped', 1: 'Running', 2: 'SD card full', 3: 'Recording done', 4: 'File cannot be written', 6: 'Not enough memory', 7: 'Stopped: memory changed', 8: 'Configuration error', 9: 'Client disconnected' };
    // 2 is a stop reason of the input only (gpio_in_stream_server): the output never sends it
    const GPIO_OUT_STATE = { 0: 'Stopped', 1: 'Running', 2: 'SD card full', 3: 'Done', 4: 'File is broken', 6: 'Not enough memory', 7: 'Stopped: memory changed', 8: 'File is missing or empty', 9: 'Configuration error', 10: 'Client disconnected' };
    const BAD = { adc: [2, 4, 6, 7], dac: [3, 4, 5, 6, 8, 9], gin: [2, 4, 6, 8], gout: [2, 4, 6, 8, 9] };
    // the codes of a configuration error: the board did not take the settings, the reason is in its log
    const CONFIG_ERROR = { adc: 7, dac: 9, gin: 8, gout: 9 };

    // the destination of the current or last run (0 none yet, 1 network, 2 SD): the status texts
    // and the statistics follow the run, not the destination switch
    const runMode = (key) => Number(v(key === 'adc' ? 'SS_ADC_RUN_MODE' : 'SS_GPIO_RUN_MODE', 0));
    function stream(key) {
        const def = {
            adc: { name: 'ADC', status: 'SS_STATUS', start: 'SS_START', table: ADC_STATE, sec: 'adc' },
            dac: { name: 'DAC', status: 'SS_DAC_STATUS', start: 'SS_DAC_START', table: DAC_STATE, sec: 'dac' },
            gin: { name: 'GPIO input', status: 'SS_GPIO_STATUS', start: 'SS_GPIO_START', table: GPIO_STATE, sec: 'gpio' },
            gout: { name: 'GPIO output', status: 'SS_GPIO_OUT_STATUS', start: 'SS_GPIO_OUT_START', table: GPIO_OUT_STATE, sec: 'gpio' }
        }[key];
        const code = Number(v(def.status, 0));
        const run = code === 1;
        // a code the page does not know (a newer server) is shown as a fault, never as a plain state
        const known = Object.prototype.hasOwnProperty.call(def.table, code);
        const bad = BAD[key].includes(code) || !known;
        let state = known ? def.table[code] : 'Unknown status ' + code;
        if (run && key === 'adc') state = runMode('adc') === 2 ? 'Writing to SD' : 'Streaming to the client';
        if (run && key === 'dac') state = v('SS_DAC_MODE', 0) ? 'Playing a file' : 'Playing from the client';
        if (run && key === 'gin') state = runMode('gin') === 2 ? 'Writing to SD' : 'Streaming to the client';
        if (run && key === 'gout') state = v('SS_GPIO_OUT_MODE', 0) ? 'Playing a file' : 'Playing from the client';
        // the reason is in the log of the server only
        const hint = code === CONFIG_ERROR[key] ? 'The board did not take the settings. The reason: journalctl -u redpitaya_streaming' : '';
        return Object.assign(def, {
            key, code, run, state, hint,
            badge: run ? 'background: rgba(59, 168, 69, 0.18); color: #8fdc97' : bad ? 'background: rgba(224, 103, 93, 0.18); color: #ff8f86' : 'background: #444444; color: #cdcccc',
            dot: run ? 'background: #3ba845' : bad ? 'background: #e0675d' : 'background: #d9534f',
            led: run ? 'img/green_led.png' : 'img/red_led.png',
            stateColor: run ? 'color: #8fdc97' : bad ? 'color: #ff8f86' : 'color: #a8a8a6',
            btn: run ? 'Stop' : 'Start',
            btnStyle: run ? 'background: transparent; border-color: #e0675d; color: #ff8f86' : 'background: #47b2d1; border-color: #47b2d1; color: #000000'
        });
    }

    const hasDac = () => !!v('SS_DAC_IS_ENABLE', false);
    const hasGpio = () => !!v('SS_GPIO_IS_ENABLE', false);
    const hasExt = () => !!v('SS_EXT_TRIG_IS_ENABLE', false);
    // the trigger state of the FPGA blocks (web_server SS_TRIGGER_STATUS, TriggerStatus::toJson)
    const TS = () => {
        let j = {};
        try { j = JSON.parse(v('SS_TRIGGER_STATUS', '') || '{}'); } catch (e) { j = {}; }
        return { adcArmed: !!j.adc_armed, adcTriggerCount: Number(j.adc_trigger_count || 0), dacArmed: Number(j.dac_armed || 0),
                 gpioInArmed: !!j.gpio_in_armed, gpioOutArmed: !!j.gpio_out_armed, gpioInTriggerCount: Number(j.gpio_in_trigger_count || 0),
                 gpioOutWait: Number(j.gpio_out_wait || 0) };
    };
    // trigger source bits (stream_app 0x8 of each block): DAC A/B, the level of IN1/IN2 (IN3/IN4 on
    // the 4-input board take the DAC bits), the GPIO pattern, the external input
    const SRC = { dacA: 1, dacB: 2, in1: 4, in2: 8, in3: 1, in4: 2, gpio: 16, ext: 32 };
    const inBit = (i) => [SRC.in1, SRC.in2, SRC.in3, SRC.in4][i];
    // the level window: codes of the calibrated 16 bit stream, full scale 1 V (1:1) or 20 V (1:20),
    // as the volts of the files (streaming_file.cpp)
    const fsOf = (i) => (v('SS_IS_ATTENUATOR', false) && (Number(v('SS_ATTENUATOR', 0)) & (1 << i)) ? 20 : 1);
    const toCode = (volts, fs) => Math.max(-32768, Math.min(32767, Math.round(volts / fs * 32768)));
    const fmtV = (x) => String(+x.toFixed(4));
    const adcCount = () => Math.max(1, Math.min(4, Number(v('SS_ADC_CHANNELS', 2))));
    const dacCount = () => Math.max(1, Math.min(2, Number(v('SS_DAC_CHANNELS', 2))));
    const adcRate = () => Number(v('SS_ACD_MAX', 125000000)) || 125000000;
    const adcDec = () => Math.max(1, Number(v('SS_RATE', 1)) || 1);
    const chMask = () => Number(v('SS_CHANNEL', 0));
    const chList = () => {
        const out = [];
        for (let i = 0; i < adcCount(); i++) if (chMask() & (1 << i)) out.push('IN' + (i + 1));
        return out;
    };
    // the decimations the FPGA takes: 1, 2, 4, 8, 16, then every value up to 65536
    const snapDec = (d) => {
        d = Math.round(d);
        if (!(d >= 1)) return 1;
        if (d >= 65536) return 65536;
        if (d === 3) return 4;
        if (d >= 5 && d <= 7) return 8;
        if (d >= 9 && d <= 15) return 16;
        return d;
    };
    const dacFiles = () => {
        const s = String(v('SS_DAC_FILE', ''));
        const i = s.indexOf('*');
        const cur = i < 0 ? s : s.slice(0, i);
        const list = i < 0 ? [] : s.slice(i + 1).split('\n').map((x) => x.trim()).filter((x) => x && !x.startsWith('.upload.'));
        return { cur, list };
    };
    const gpioFiles = () => {
        const s = String(v('SS_GPIO_OUT_FILE', ''));
        const i = s.indexOf('*');
        return { cur: i < 0 ? s : s.slice(0, i), list: i < 0 ? [] : s.slice(i + 1).split('\n').map((x) => x.trim()).filter((x) => x && !x.startsWith('.')) };
    };
    // the GPIO block runs at the DAC rate on the 65-16 (twice its ADC rate), else at the ADC rate
    const gpioClock = () => (adcRate() === 62500000 ? 125000000 : adcRate());
    const fileType = (name) => (/\.tdms$/i.test(name) ? 1 : /\.bin$/i.test(name) ? 2 : 0);

    // ---------------------------------------------------------------------------------------
    // templates
    const sel = (opts, cur) => opts.map(([val, label]) => `<option value="${esc(val)}"${String(val) === String(cur) ? ' selected' : ''}>${esc(label)}</option>`).join('');
    const seg = (label, items) => `<div class="seg" role="group" aria-label="${esc(label)}">${items.map(([act, arg, text, on, off]) =>
        `<button type="button" style="${on ? ON : OFF}${off && !on ? '; opacity: .45; cursor: not-allowed' : ''}" data-act="${off ? 'none' : act}" data-arg="${esc(arg)}" aria-pressed="${on}"${off ? ` disabled title="${esc(off)}"` : ''}>${esc(text)}</button>`).join('')}</div>`;
    const flip = (attrs, checked, label, small) => `<input type="checkbox" class="flipswitch${small ? ' sm' : ''}" data-on="ON" data-off="OFF" ${attrs}${checked ? ' checked' : ''}${label ? ` aria-label="${esc(label)}"` : ''}>`;
    const secHead = (title, sub, right) => `<div class="card-h"><div class="sec-h"><h1>${esc(title)}</h1><p class="sub">${sub}</p></div>${right ? `<div class="row">${right}</div>` : ''}</div>`;
    const badge = (s) => `<span class="badge" style="${s.badge}"${s.hint ? ` title="${esc(s.hint)}"` : ''}><span class="dot" style="${s.dot}"></span>${esc(s.state)}</span>`;
    // under the head of a section: why a stream did not start
    const errNote = (...ss) => ss.filter((x) => x.hint).map((x) => `<p class="hint bad" role="alert" style="margin: 0 0 12px">${esc(x.name)}: ${esc(x.hint)}</p>`).join('');
    const softBtn = (target) => `<button class="btn" type="button" style="${NEUTRAL}" data-act="soft" data-arg="${target}">Software trigger</button>`;
    // a switch row of a trigger source: label, what it is, available on this board
    const srcRow = (param, bit, label, desc, avail, why) => `<label class="srci${avail ? '' : ' srci-dis'}" title="${avail ? '' : esc(why)}">${flip(`data-act="${avail ? 'bit' : 'none'}" data-arg="${param}:${bit}"`, avail && (Number(v(param, 0)) & bit), label, true)}<div><b>${esc(label)}</b><span>${esc(avail ? desc : why)}</span></div></label>`;
    const evOpts = (own) => [[-1, 'None (own Start, Stop)']].concat(own === 'adc' ? [] : [[2, 'ADC']], own === 'dac' || !hasDac() ? [] : [[0, 'DAC']], own === 'gin' || !hasGpio() ? [] : [[4, 'GPIO input']]);
    const runBtn = (s, extra) => `<button class="btn" type="button" style="${s.btnStyle}${extra || ''}" data-act="run" data-arg="${s.key}">${s.btn}</button>`;

    function navHtml() {
        const items = [['overview', 'Overview', ''], ['adc', 'ADC', ''], ['dac', 'DAC', ''], ['gpio', 'GPIO', ''], ['trig', 'Triggers', ''], ['mem', 'Memory', ''], ['files', 'Files', 'SD'], ['set', 'Settings & clients', '']]
            .filter(([id]) => (id !== 'dac' || hasDac()) && (id !== 'gpio' || hasGpio()));
        const live = { adc: stream('adc').run, dac: hasDac() && stream('dac').run, gpio: hasGpio() && (stream('gin').run || stream('gout').run) };
        const master = { 0: 'Unknown', 1: 'Master', 2: 'Slave' }[Number(v('SS_IS_MASTER', 0))] || 'Unknown';
        return items.map(([id, label, note]) =>
            `<button class="nav-b" type="button" data-go="${id}" style="${S.section === id ? ON : ''}"${S.section === id ? ' aria-current="page"' : ''}>
               <span class="dot" style="${live[id] ? 'background: #3ba845' : 'background: transparent'}"></span><span style="flex: 1">${label}</span><span class="hint">${note}</span></button>`).join('') +
            `<div class="foot"><div class="sep"></div>
               <div class="hint kvl"><span>IP</span><b class="mono">${esc(S.ip || '-')}</b></div>
               <div class="hint kvl"><span>Mode</span><b>${master}</b></div>
               <div class="hint kvl"><span>SD free</span><b class="mono">${sdSpace() ? fmtBytes(sdSpace()[0]) : '-'}</b></div></div>`;
    }

    // ---- losses of the last run (the server resets them at a start, keeps them after the stop):
    // samples per channel lost on the board (the FPGA found no free buffer: the client, the network
    // or the SD card too slow) and, for the ADC, not taken by the SD file (its write queue was full).
    // The GPIO input has no such queue loss: a full queue ends its recording.
    const lostOf = (key) => key === 'adc'
        ? { board: Number(v('SS_ADC_LOST', 0)), file: Number(v('SS_ADC_FILE_LOST', 0)), pass: Number(v('SS_ADC_SAMPLES', 0)) }
        : { board: Number(v('SS_GPIO_LOST', 0)), file: 0, pass: Number(v('SS_GPIO_SAMPLES', 0)) };
    const lostShort = (key) => {
        const l = lostOf(key), all = l.board + l.file;
        if (!all) return '0';
        // the samples the queue dropped were given by the FPGA: the whole is given plus lost on the board
        const whole = l.pass + l.board;
        const pct = whole ? all / whole * 100 : 0;
        return all.toLocaleString('en-US') + ' · ' + (pct < 0.01 ? '<0.01' : pct.toFixed(2)) + '%';
    };
    // the losses of a network run (an SD run has them in its recording block)
    function lostHtml(key) {
        const l = lostOf(key), all = l.board + l.file;
        const n = `<b class="mono" style="color: ${all ? '#ff8f86' : '#e8e8e6'}">${all.toLocaleString('en-US')}</b>`;
        return `<p class="hint${all ? ' bad' : ''}" style="margin: 0">Lost: ${n} samples per channel` +
            (all ? '<br><span style="color: #a8a8a6">No free buffer on the board: the client or the network is slower than the data; rpsa_client marks the places in its file.</span>' : '') + '</p>';
    }

    // ---- the progress of an SD recording: the limit, the free space, the time left
    // [free, total] bytes of the SD card: the server sends them with the parameters (SS_SD_FREE,
    // SS_SD_TOTAL); an older one does not, then the request of the Files section serves
    const sdSpace = () => (Number(v('SS_SD_TOTAL', 0)) ? [Number(v('SS_SD_FREE', 0)), Number(v('SS_SD_TOTAL', 0))] : S.sdFree);
    const fmtDur = (sec) => {
        if (!Number.isFinite(sec) || sec < 0) return '-';
        sec = Math.round(sec);
        if (sec < 90) return sec + ' s';
        if (sec < 5400) return Math.round(sec / 60) + ' min';
        return Math.floor(sec / 3600) + ' h ' + String(Math.round(sec % 3600 / 60)).padStart(2, '0') + ' min';
    };
    // bytes a second of the data the stream gives to its file (before the card: the card may be slower)
    const dataRateOf = (key) => key === 'adc'
        ? adcRate() / adcDec() * chList().length * (Number(v('SS_SAVE_MODE', 0)) ? 4 : (Number(v('SS_RESOLUTION', 1)) ? 2 : 1))
        : gpioClock() / Math.max(1, Number(v('SS_GPIO_DECIMATION', 1))) * 4;
    function sdProgress(key) {
        const adc = key === 'adc';
        const s = stream(key);
        const limit = Number(v(adc ? 'SS_SAMPLES' : 'SS_GPIO_IN_SAMPLES', 0));
        const done = Number(v(adc ? 'SS_PASS_SAMPLES' : 'SS_GPIO_FILE_SAMPLES', 0));
        const perCh = adc ? adcRate() / adcDec() : gpioClock() / Math.max(1, Number(v('SS_GPIO_DECIMATION', 1)));
        const card = Number(v(adc ? 'SS_WRITE_RATE' : 'SS_GPIO_WRITE_RATE', 0));
        const sp = sdSpace(), free = sp ? sp[0] : 0, total = sp ? sp[1] : 0;
        const data = dataRateOf(key);
        const need = limit * data / perCh;  // bytes the limit takes
        // what goes to the card a second: the data, or the card when it is slower. The measured
        // speed alone overstates a card the data do not keep busy (the page cache takes them).
        const thru = card ? Math.min(card, data) : data;
        return { run: s.run, limit, done, perCh, card, data, thru, free, total, need, frac: limit ? Math.min(1, done / limit) : (total ? 1 - free / total : 0) };
    }
    const pbar = (frac, color) => `<div class="bar" style="height: 8px; margin: 2px 0"><div style="width: ${(frac * 100).toFixed(1)}%; background: ${color}"></div></div>`;
    // before a recording (or while the stream runs to the network): does the limit fit the card
    function progressHtml(key) {
        const p = sdProgress(key);
        const freeText = p.total ? fmtBytes(p.free) + ' free of ' + fmtBytes(p.total) : 'free space unknown';
        if (!p.limit) return `<p class="hint" style="margin: 0">SD card: ${freeText} · the data: ${fmtBytes(p.data)}/s</p>`;
        const fits = !p.total || p.need <= p.free;
        return `<p class="hint${fits ? '' : ' bad'}" style="margin: 0">The limit takes about ${fmtBytes(p.need)} · SD card: ${freeText}${fits ? '' : ' - it does not fit'}</p>`;
    }
    // One block for an SD recording: a bar (the limit, else the card), then Recorded, Time left,
    // SD card, Lost; a note under it only when something is wrong. Before the first run: a line.
    function recordingHtml(key) {
        const p = sdProgress(key), l = lostOf(key), all = l.board + l.file;
        const adc = key === 'adc';
        const written = Number(v(adc ? 'SS_WRITED_SIZE' : 'SS_GPIO_WRITTEN', 0));
        if (runMode(key) !== 2 || (!p.run && !p.done)) return progressHtml(key);
        const red = ' style="color: #ff8f86"';
        const slow = p.card && p.data > p.card;
        const low = p.total && p.free < p.total * 0.05;
        const left = p.limit ? fmtDur((p.limit - p.done) / p.perCh) : 'SD full in ' + fmtDur(p.free / p.thru);
        const cell = (k, b, sub, bad) => `<div class="scell"><span>${k}</span><b${bad ? red : ''}>${b}</b><span>${sub}</span></div>`;
        const cells = [
            cell('Recorded', p.limit ? Math.round(p.frac * 100) + '% of the limit' : p.done.toLocaleString('en-US') + ' samples',
                (p.limit ? p.done.toLocaleString('en-US') + ' of ' + p.limit.toLocaleString('en-US') : 'no limit') + (written ? ' · ' + fmtBytes(written) : '')),
            cell('Time left', p.run ? left : (p.limit && p.done >= p.limit ? 'done' : 'stopped'), p.run ? (p.limit ? 'to the limit at ' + fmtRate(p.perCh) : 'at ' + fmtBytes(p.thru) + '/s') : ''),
            p.run ? cell('SD card', fmtBytes(p.thru) + '/s · queue ' + Math.round(Number(v(adc ? 'SS_FILE_QUEUE' : 'SS_GPIO_FILE_QUEUE', 0)) * 100) + '%',
                p.total ? fmtBytes(p.free) + ' free of ' + fmtBytes(p.total) : '', slow || low)
                : cell('SD card', fmtBytes(p.free) + ' free', p.total ? 'of ' + fmtBytes(p.total) : '', low),
            cell('Lost', all ? lostShort(key) : '0', adc ? 'board ' + l.board.toLocaleString('en-US') + ' · queue ' + l.file.toLocaleString('en-US') : 'on the board', all > 0)];
        const notes = [];
        if (slow && p.run) notes.push(`The card writes ${fmtBytes(p.card)}/s of ${fmtBytes(p.data)}/s of data: the data are lost once the queue is full.`);
        if (low) notes.push('Less than 5% of the card is free.');
        if (all) notes.push('On the board: no free buffer, the card is slower than the data; the file marks the places.' + (adc ? ' The queue: those buffers are missing from the file, its report counts them.' : ''));
        return pbar(p.frac, low ? '#e0675d' : p.limit ? '#47b2d1' : '#7a7a78') +
            `<div class="grid" style="grid-template-columns: repeat(auto-fill, minmax(190px, 1fr)); gap: 12px">${cells.join('')}</div>` +
            notes.map((n) => `<p class="hint bad" style="margin: 0">${esc(n)}</p>`).join('');
    }
    // the overview: a word on the progress under the state
    function progressShort(key) {
        const p = sdProgress(key);
        if (!p.run || runMode(key) !== 2) return '';
        if (p.limit) return Math.round(p.frac * 100) + '% · ' + fmtDur((p.limit - p.done) / p.perCh) + ' left';
        return 'SD full in ' + fmtDur(p.free / p.thru);
    }

    // ---- overview
    function overviewHtml() {
        const adc = stream('adc');
        const rows = [Object.assign(adc, {
            k: [['Destination', v('SS_USE_FILE', false) ? 'SD · ' + ['WAV', 'TDMS', 'BIN'][Number(v('SS_FORMAT', 0))] : 'Network'],
                ['Channels', chList().join(', ') || 'none'], ['Rate', fmtRate(adcRate() / adcDec())],
                // four cells fit the row: the speed of the card goes with the written size (the queue: ADC card)
                v('SS_USE_FILE', false) ? ['Written', fmtBytes(Number(v('SS_WRITED_SIZE', 0))) + (adc.run && runMode('adc') === 2 ? ' · ' + fmtBytes(sdProgress('adc').thru) + '/s' : '')] : ['Port', '18900']]
        })];
        if (hasDac()) {
            const d = stream('dac');
            const f = dacFiles();
            rows.push(Object.assign(d, {
                k: [['Source', v('SS_DAC_MODE', 0) ? f.cur || 'no file' : 'Network'], ['Rate', fmtRate(Number(v('SS_DAC_HZ', 0)))],
                    ['Repeat', { '-1': 'Once', 0: v('SS_DAC_REPEAT_COUNT', 1) + ' times', '-2': 'Forever' }[String(v('SS_DAC_REPEAT', -1))] || '-'],
                    ['Outputs', dacCount() === 1 ? 'OUT1' : 'OUT1, OUT2']]
            }));
        }
        if (hasGpio()) {
            const gi = stream('gin'), go = stream('gout');
            const gf = gpioFiles();
            rows.push(Object.assign(gi, {
                k: [['Destination', v('SS_GPIO_IN_MODE', 0) ? 'SD · ' + ['BIN', 'CSV', 'VCD'][Number(v('SS_GPIO_IN_FORMAT', 0))] : 'Network'],
                    ['Rate', fmtRate(gpioClock() / Math.max(1, Number(v('SS_GPIO_DECIMATION', 1)))) + (v('SS_GPIO_RLE', 0) ? ', RLE' : '')],
                    ['Trigger', v('SS_GPIO_IN_TRIG_MODE', 0) ? 'Waits' : 'Off'], ['Port', '18904']]
            }));
            rows.push(Object.assign(go, {
                k: [['Source', v('SS_GPIO_OUT_MODE', 0) ? gf.cur || 'no file' : 'Network'],
                    ['Repeat', { '-1': 'Once', 0: v('SS_GPIO_OUT_REPEAT_COUNT', 1) + ' times', '-2': 'Forever' }[String(v('SS_GPIO_OUT_REPEAT', -1))] || '-'],
                    ['Trigger', v('SS_GPIO_OUT_TRIG_MODE', 0) ? 'Waits' : 'Off'], ['Waited', (TS().gpioOutWait || 0).toLocaleString('en-US') + ' clocks']]
            }));
        }
        return secHead('Overview', 'The streams of the board. Each starts on its own; the board serves one client at a time.') +
            `<div class="ovb"><section class="card srows" aria-label="Streams">${rows.map((s) => `
              <div class="srow">
                <div class="sname"><img src="${s.led}" alt="" style="width: 22px; height: 22px; flex: none">
                  <div class="scell"><b style="font-size: var(--fs-base)">${s.name}</b><span style="${s.stateColor}">${esc(s.state)}</span>${(s.key === 'adc' && v('SS_USE_FILE', false) || s.key === 'gin' && Number(v('SS_GPIO_IN_MODE', 0))) && progressShort(s.key) ? `<span class="hint">${esc(progressShort(s.key))}</span>` : ''}${(s.key === 'adc' || s.key === 'gin') && lostShort(s.key) !== '0' ? `<span style="color: #ff8f86">lost ${esc(lostShort(s.key))}</span>` : ''}</div></div>
                ${s.k.map(([k, val]) => `<div class="scell"><span>${esc(k)}</span><b title="${esc(val)}">${esc(val)}</b></div>`).join('')}
                <div class="sact">${runBtn(s, '; min-width: 72px; justify-content: center')}
                  ${s.sec ? `<button class="btn ibtn" type="button" aria-label="${s.name} settings" data-go="${s.sec}"><svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" aria-hidden="true"><line x1="4" y1="6" x2="20" y2="6"></line><line x1="4" y1="12" x2="20" y2="12"></line><line x1="4" y1="18" x2="20" y2="18"></line><circle cx="9" cy="6" r="2.2" fill="#2b2b2a"></circle><circle cx="15" cy="12" r="2.2" fill="#2b2b2a"></circle><circle cx="8" cy="18" r="2.2" fill="#2b2b2a"></circle></svg></button>` : '<span class="btn ibtn" style="visibility: hidden"></span>'}
                </div></div>`).join('')}</section>
            ${memSummaryHtml()}</div>`;
    }


    // ---- GPIO: lines (bit i - P i, bit 8 + i - N i), the input and the output
    const lineGrid = (cells) => `<div class="lines">${['P', 'N'].map((row) => `<span class="lnl">${row}</span>` +
        Array.from({ length: 8 }, (_, i) => cells(row, i, (row === 'N' ? 8 : 0) + i)).join('')).join('')}</div>`;
    const hex4 = (x) => '0x' + (x >>> 0).toString(16).toUpperCase().padStart(4, '0');
    const EDGE_UP = '<svg width="20" height="16" viewBox="0 0 20 16" aria-hidden="true" style="display: block"><path d="M2 13 H9 V3 H18" fill="none" stroke="currentColor" stroke-width="1.4" stroke-linejoin="round" stroke-linecap="round"/><path d="M6.5 8.5 L9 5.5 L11.5 8.5" fill="none" stroke="currentColor" stroke-width="1.2" stroke-linejoin="round" stroke-linecap="round"/></svg>';
    const EDGE_DOWN = '<svg width="20" height="16" viewBox="0 0 20 16" aria-hidden="true" style="display: block"><path d="M2 3 H9 V13 H18" fill="none" stroke="currentColor" stroke-width="1.4" stroke-linejoin="round" stroke-linecap="round"/><path d="M6.5 7.5 L9 10.5 L11.5 7.5" fill="none" stroke="currentColor" stroke-width="1.2" stroke-linejoin="round" stroke-linecap="round"/></svg>';
    // the pattern of a line: 0 any, 1 low, 2 high, 3 rising, 4 falling
    function patOf(k) {
        const b = 1 << k;
        if (Number(v('SS_GPIO_PAT_POS', 0)) & b) return 3;
        if (Number(v('SS_GPIO_PAT_NEG', 0)) & b) return 4;
        if (Number(v('SS_GPIO_PAT_MASK', 0)) & b) return Number(v('SS_GPIO_PAT_VALUE', 0)) & b ? 2 : 1;
        return 0;
    }
    function setPattern(pat) {
        let m = 0, val = 0, pos = 0, neg = 0;
        pat.forEach((p, k) => {
            const b = 1 << k;
            if (p === 1 || p === 2) m |= b;
            if (p === 2) val |= b;
            if (p === 3) pos |= b;
            if (p === 4) neg |= b;
        });
        C.parametersCache.SS_GPIO_PAT_MASK = { value: m };
        C.parametersCache.SS_GPIO_PAT_VALUE = { value: val };
        C.parametersCache.SS_GPIO_PAT_POS = { value: pos };
        C.parametersCache.SS_GPIO_PAT_NEG = { value: neg };
        C.sendParameters();
    }
    // locked (a reason): while the stream runs its destination stays
    const tiles = (label, aria, items, locked) => `<div class="field"><label>${label}</label><div class="dst" role="radiogroup" aria-label="${aria}"${locked ? ` title="${esc(locked)}"` : ''}>${items.map(([arg, title, desc, on]) =>
        `<button type="button" role="radio" class="dsti${on ? ' dsti-on' : ''}" aria-checked="${on}" data-act="${locked ? 'none' : 'setv'}" data-arg="${arg}"${locked && !on ? ' disabled style="opacity: .45; cursor: not-allowed"' : ''}><i></i><div><b>${title}</b><span>${desc}</span></div></button>`).join('')}</div></div>`;

    function gpioHtml() {
        const gi = stream('gin'), go = stream('gout');
        const outP = Number(v('SS_GPIO_OUTPUTS_P', 0)), outN = Number(v('SS_GPIO_OUTPUTS_N', 0)), outMask = outP | (outN << 8);
        const pol = Number(v('SS_GPIO_POLARITY', 0));
        const dec = Math.max(1, Number(v('SS_GPIO_DECIMATION', 1)));
        const rle = !!Number(v('SS_GPIO_RLE', 0));
        const inSd = !!Number(v('SS_GPIO_IN_MODE', 0)), outSd = !!Number(v('SS_GPIO_OUT_MODE', 0));
        const inTrig = !!Number(v('SS_GPIO_IN_TRIG_MODE', 0));
        const ts = TS();
        const pats = Array.from({ length: 16 }, (_, k) => patOf(k));
        const patSet = pats.some((x) => x);
        const PM = Number(v('SS_GPIO_PAT_MASK', 0)), PV = Number(v('SS_GPIO_PAT_VALUE', 0)), PP = Number(v('SS_GPIO_PAT_POS', 0)), PN = Number(v('SS_GPIO_PAT_NEG', 0));
        const srcs = Number(v('SS_GPIO_TRIG_SOURCES', SRC.gpio));
        const extra = srcs & ~SRC.gpio;
        const extraOpts = [[0, 'Nothing else (the pattern only)']];
        for (let i = 0; i < adcCount(); i++) extraOpts.push([inBit(i), 'IN' + (i + 1) + ' level']);
        if (hasDac() && v('SS_DAC_TRIG_OUTPUTS', false)) extraOpts.push([SRC.dacA, 'DAC A (OUT1 starts)'], [SRC.dacB, 'DAC B (OUT2 starts)']);
        extraOpts.push([SRC.ext, 'External (EXT TRIG input)']);
        const extraCur = extraOpts.some(([b]) => b === extra) ? extra : (extra ? 'mixed' : 0);
        if (extraCur === 'mixed') extraOpts.push(['mixed', 'Several (set by a client)']);
        const gf = gpioFiles();
        const rep = String(v('SS_GPIO_OUT_REPEAT', -1));
        const lineBtn = (param, mask) => (row, i, k) => `<button type="button" class="ln${mask & (1 << k) ? ' ln-on' : ''}" aria-pressed="${!!(mask & (1 << k))}" title="DIO${i}_${row}" data-act="gline" data-arg="${param}:${k}">${i}</button>`;
        return secHead('GPIO', 'DIO0..7_P and DIO0..7_N. The input and the output run on their own; an output line reads back its own level.') + errNote(gi, go) +
            `<section class="card"><div class="card-h"><h2 class="card-t">Lines</h2><span class="hint">click a line to switch it</span></div>
              <div class="grid" style="grid-template-columns: repeat(auto-fit, minmax(300px, 1fr))">
                <div class="field"><div class="lnh"><label>Driven by the output</label><button type="button" class="lnb" data-act="gall" data-arg="out:1">all</button><button type="button" class="lnb" data-act="gall" data-arg="out:0">none</button><span class="lnm">${hex4(outMask)}</span></div>
                  ${lineGrid(lineBtn('out', outMask))}</div>
                <div class="field"><div class="lnh"><label>Inverted on the input</label><button type="button" class="lnb" data-act="gall" data-arg="pol:1">all</button><button type="button" class="lnb" data-act="gall" data-arg="pol:0">none</button><span class="lnm">${hex4(pol)}</span></div>
                  ${lineGrid(lineBtn('pol', pol))}</div>
              </div>
              <div class="grid" style="grid-template-columns: repeat(auto-fill, minmax(170px, 1fr))">
                <div class="field"><label for="gdec">Decimation</label><input id="gdec" class="mono" value="${dec}" data-act="int" data-arg="SS_GPIO_DECIMATION"><span class="hint mono">${fmtRate(gpioClock() / dec)}${rle ? ', RLE' : ''}</span></div>
                <div class="field"><label>Run-length packing</label>${seg('Run-length packing', [['setv', 'SS_GPIO_RLE:1', 'On', rle], ['setv', 'SS_GPIO_RLE:0', 'Off', !rle]])}</div>
              </div>
            </section>
            <div class="grid" style="grid-template-columns: repeat(auto-fit, minmax(340px, 1fr))">
              <section class="card">
                <div class="card-h"><h2 class="card-t">Input</h2><div class="row">${badge(gi)}${inTrig ? softBtn(3) : ''}${runBtn(gi)}</div></div>
                ${tiles('Destination', 'GPIO input destination', [['SS_GPIO_IN_MODE:0', 'Network client', 'port 18904 · rpsa_client, the API', !inSd], ['SS_GPIO_IN_MODE:1', 'File on SD card', 'BIN, CSV or VCD', inSd]], gi.run && 'Stop the input to change the destination')}
                ${!inSd && runMode('gin') === 1 ? lostHtml('gin') : ''}
                ${inSd ? `<div class="grid" style="grid-template-columns: repeat(auto-fill, minmax(220px, 1fr))">
                    <div class="field"><label for="gfmt">Format</label><select id="gfmt" data-param="SS_GPIO_IN_FORMAT">${sel([[0, 'BIN'], [1, 'CSV'], [2, 'VCD']], v('SS_GPIO_IN_FORMAT', 0))}</select></div>
                    <div class="field"><label for="glim">Samples</label><input id="glim" class="mono" value="${esc(v('SS_GPIO_IN_SAMPLES', 0))}" data-act="int" data-arg="SS_GPIO_IN_SAMPLES"><span class="hint">0 - until the stop</span></div>
                  </div>${recordingHtml('gin')}` : ''}
                <div class="field"><label>Trigger</label><label class="check">${flip('data-act="flag" data-arg="SS_GPIO_IN_TRIG_MODE"', inTrig)}Wait for a trigger, then capture</label></div>
                ${inTrig ? `<div class="field"><div class="lnh"><label>Pattern trigger (lines that must match)</label><button type="button" class="lnb" data-act="gpatClear">clear</button><span class="lnm">${patSet ? 'mask ' + hex4(PM) + ' value ' + hex4(PV) + ' rise ' + hex4(PP) + ' fall ' + hex4(PN) : 'no line: the pattern never fires'}</span></div>
                    ${lineGrid((row, i, k) => {
                        const p = pats[k];
                        const cls = ['ln', 'ln ln-lvl', 'ln ln-lvl', 'ln ln-edg', 'ln ln-edg'][p];
                        const name = 'DIO' + i + '_' + row + ': ' + ['any', 'low', 'high', 'rising edge', 'falling edge'][p];
                        const face = p === 3 ? EDGE_UP : p === 4 ? EDGE_DOWN : ['x', '0', '1'][p];
                        return `<button type="button" class="${cls}" title="${name}" aria-label="${name}" data-act="gpat" data-arg="${k}">${face}</button>`;
                    })}
                    <span class="hint">click a line: x (any) → 0 → 1 → rising → falling</span></div>
                  <div class="grid" style="grid-template-columns: repeat(auto-fill, minmax(220px, 1fr))">
                    <div class="field"><label for="gtb">Buffers per trigger</label><input id="gtb" class="mono" value="${esc(v('SS_GPIO_TRIG_BLOCKS', 0))}" data-act="int" data-arg="SS_GPIO_TRIG_BLOCKS"><span class="hint">0 - everything after it</span></div>
                    <div class="field"><label for="gts">Also start on</label><select id="gts" data-act="gsrc">${sel(extraOpts, extraCur)}</select></div>
                  </div>
                  <p class="hint" style="margin: 0">Armed: <b class="mono" style="color: #e8e8e6">${ts.gpioInArmed ? 'yes' : 'no'}</b> · triggers seen: <b class="mono" style="color: #e8e8e6">${(ts.gpioInTriggerCount || 0).toLocaleString('en-US')}</b></p>` : ''}
              </section>
              <section class="card">
                <div class="card-h"><h2 class="card-t">Output</h2><div class="row">${badge(go)}${runBtn(go)}</div></div>
                ${tiles('Source', 'GPIO output source', [['SS_GPIO_OUT_MODE:0', 'Network client', 'port 18905 · rpsa_client, the API', !outSd], ['SS_GPIO_OUT_MODE:1', 'File on SD card', 'a BIN pattern or a recording', outSd]], go.run && 'Stop the output to change the source')}
                ${outSd ? `<div class="grid" style="grid-template-columns: repeat(auto-fill, minmax(220px, 1fr))">
                    <div class="field"><label for="gofile">File (BIN)</label>${gf.list.length
                        ? `<select id="gofile" data-act="gfile">${gf.cur && !gf.list.includes(gf.cur) ? `<option value="${esc(gf.cur)}" selected>${esc(gf.cur)} (missing)</option>` : ''}${sel(gf.list.map((x) => [x, x]), gf.cur)}</select>`
                        : '<span class="val hint">no files: upload one</span>'}</div>
                    <div class="field"><label>&#8203;</label><button class="btn" type="button" style="${NEUTRAL}" data-act="uploadGpio"><img src="img/upload.png" alt="" style="width: 18px; height: 18px">Upload file…</button></div>
                    <div class="field"><label for="gorep">Repeat</label><select id="gorep" data-param="SS_GPIO_OUT_REPEAT">${sel([[-1, 'Once'], [0, 'N times'], [-2, 'Forever']], rep)}</select></div>
                    ${rep === '0' ? `<div class="field"><label for="gocnt">Times</label><input id="gocnt" class="mono" value="${esc(v('SS_GPIO_OUT_REPEAT_COUNT', 1))}" data-act="int" data-arg="SS_GPIO_OUT_REPEAT_COUNT"></div>` : ''}
                  </div>
                  ${S.upload && S.uploadDir === 'gpio' ? `<p class="hint${S.uploadBad ? ' bad' : ''}" style="margin: 0">${esc(S.upload)}</p>` : ''}
                  <label class="check">${flip('data-act="flag" data-arg="SS_GPIO_OUT_PLAY_GAPS"', Number(v('SS_GPIO_OUT_PLAY_GAPS', 0)))}A recording: play its gaps (the lines hold), in its own time</label>` : ''}
                <div class="grid" style="grid-template-columns: repeat(auto-fill, minmax(220px, 1fr))">
                  <div class="field"><label for="gstep">Step</label><input id="gstep" class="mono" value="${esc(v('SS_GPIO_OUT_STEP', 1))}" data-act="int" data-arg="SS_GPIO_OUT_STEP"><span class="hint">clocks per word</span></div>
                  <div class="field"><label>Trigger</label><label class="check" style="height: 32px">${flip('data-act="flag" data-arg="SS_GPIO_OUT_TRIG_MODE"', Number(v('SS_GPIO_OUT_TRIG_MODE', 0)))}Wait for a trigger</label></div>
                </div>
                <p class="hint" style="margin: 0">Waited for data: <b class="mono" style="color: #e8e8e6">${(ts.gpioOutWait || 0).toLocaleString('en-US')} clocks</b>${ts.gpioOutArmed ? ' · armed' : ''}</p>
              </section>
            </div>
            <section class="card"><h2 class="card-t">Start and stop together with</h2>
              <div class="field" style="max-width: 320px"><select aria-label="GPIO events" data-param="SS_GPIO_EVENT">${sel(evOpts('gin'), v('SS_GPIO_EVENT', -1))}</select></div>
            </section>`;
    }

    // ---- triggers: who starts whom, the external input
    function trigHtml() {
        const cols = [];
        for (let i = 0; i < adcCount(); i++) cols.push(['IN' + (i + 1), inBit(i), 'adc']);
        if (hasDac()) cols.push(['DAC A', SRC.dacA, 'dac'], ['DAC B', SRC.dacB, 'dac']);
        if (hasGpio()) cols.push(['GPIO', SRC.gpio, 'gpio']);
        cols.push(['External', SRC.ext, 'ext']);
        const rows = [['ADC', 'SS_TRIG_SOURCES', !!Number(v('SS_TRIG_MODE', 0)), Number(v('SS_TRIG_BLOCKS', 0)), 'adc']];
        if (hasDac()) rows.push(['DAC', 'SS_DAC_TRIG_SOURCES', !!Number(v('SS_DAC_TRIG_MODE', 0)), null, 'dac']);
        if (hasGpio()) rows.push(['GPIO input', 'SS_GPIO_TRIG_SOURCES', !!Number(v('SS_GPIO_IN_TRIG_MODE', 0)), Number(v('SS_GPIO_TRIG_BLOCKS', 0)), 'gpio']);
        const dacOut = !!v('SS_DAC_TRIG_OUTPUTS', false);
        const cell = (row, [name, bit, kind]) => {
            // a stream does not trigger itself (the GPIO pattern is the GPIO's own trigger)
            if ((row[4] === 'dac' && kind === 'dac') || (row[4] === 'gpio' && kind === 'gpio')) return '<td class="hint">-</td>';
            if (kind === 'dac' && !dacOut) return '<td class="hint" title="needs a newer FPGA (DAC trigger outputs)">-</td>';
            return `<td>${flip(`data-act="bit" data-arg="${row[1]}:${bit}"`, Number(v(row[1], 0)) & bit, row[0] + ' from ' + name, true)}</td>`;
        };
        const ext = hasExt() ? `<section class="card"><h2 class="card-t">External trigger input</h2>
              <div class="grid" style="grid-template-columns: repeat(auto-fill, minmax(190px, 1fr))">
                <div class="field"><label for="xc">Edge detector and debounce</label><select id="xc" data-param="SS_EXT_COND">${sel([[1, 'On'], [0, 'Off (raw level)']], v('SS_EXT_COND', 1))}</select></div>
                <div class="field"><label for="xe">Edge</label><select id="xe" data-param="SS_EXT_EDGE">${sel([[0, 'Rising'], [1, 'Falling']], v('SS_EXT_EDGE', 0))}</select></div>
                <div class="field"><label for="xd">Debounce (clocks)</label><input id="xd" class="mono" value="${esc(v('SS_EXT_DEBOUNCE', 0))}" data-act="int" data-arg="SS_EXT_DEBOUNCE"><span class="hint">0 - off, up to 65535</span></div>
                <div class="field"><label for="x0">DIO0_P drives it</label><select id="x0" data-param="SS_EXT_DIO0">${sel([[1, 'On'], [0, 'Off']], v('SS_EXT_DIO0', 0))}</select></div>
              </div></section>` : '';
        return secHead('Triggers', 'Who starts whom. The external input is shared by every stream.') +
            `<section class="card"><h2 class="card-t">Sources of each stream</h2>
              <div style="overflow-x: auto"><table>
                <thead><tr><th scope="col">Stream</th><th scope="col">Waits</th>${cols.map(([n]) => `<th scope="col">${n}</th>`).join('')}<th scope="col">Blocks</th></tr></thead>
                <tbody>${rows.map((r) => `<tr><td>${r[0]}</td><td>${r[2] ? 'yes' : 'no'}</td>${cols.map((c) => cell(r, c)).join('')}<td class="mono">${r[3] === null ? '<span class="hint">-</span>' : r[3]}</td></tr>`).join('')}</tbody>
              </table></div>
              <p class="hint" style="margin: 0">A stream starts on any source switched on. Whether it waits is set in its own section.</p>
            </section>${ext}`;
    }

    // ---- memory (web_server MM_*; a block of the stream holds the block size + the header)
    const HEADER = 128;
    const MEM = [['adc', 'ADC', 'MM_ADC_SIZE', 'MM_ADC_VALID', '#47b2d1'], ['dac', 'DAC', 'MM_DAC_SIZE', 'MM_DAC_VALID', '#e0a040'],
        ['gin', 'GPIO in', 'MM_GPIO_IN_SIZE', 'MM_GPIO_IN_VALID', '#6cbf73'], ['gout', 'GPIO out', 'MM_GPIO_OUT_SIZE', 'MM_GPIO_OUT_VALID', '#b08ad8']];
    function memModel() {
        const total = Number(pmax('MM_ADC_SIZE', 0)) || 1;
        const block = Number(v('MM_BLOCK_SIZE', 65536));
        const blk = block + HEADER;
        const rows = MEM.filter(([k]) => k !== 'dac' || hasDac()).map(([k, name, p, valid, color]) => {
            const size = Number(v(p, 0));
            const n = Math.floor(size / blk);
            const on = size > 0;
            return { k, name, p, color, size, n, on, ok: !on || (n >= 12 && v(valid, true) !== false) };
        });
        const used = rows.reduce((a, r) => a + r.size, 0);
        return { total, block, blk, rows, used, over: used > total };
    }
    function memBarHtml(m) {
        const scale = Math.max(m.total, m.used);
        const parts = m.rows.map((r) => `<div style="width: ${(r.size / scale * 100).toFixed(2)}%; background: ${r.color}" title="${r.name} ${MB(r.size)}"></div>`).join('') +
            (m.over ? '' : `<div style="width: ${((m.total - m.used) / scale * 100).toFixed(2)}%; background: #5a5a58" title="free ${MB(m.total - m.used)}"></div>`);
        return `<div class="bar" role="img" aria-label="Memory: ${m.rows.map((r) => r.name + ' ' + MB(r.size)).join(', ')}">${parts}</div>`;
    }
    function memLegendHtml(m, switches) {
        const item = (dot, name, size, pct, style, sw) => `<label style="display: inline-flex; align-items: center; gap: 10px; font-size: var(--fs-sm); ${style}">${sw || ''}<span class="dot" style="background: ${dot}"></span><span>${name}</span><b class="mono" style="font-weight: 400; color: #e8e8e6">${size}</b><span class="mono" style="color: #a8a8a6">${pct}</span></label>`;
        return `<div class="row" style="gap: 12px 32px">${m.rows.map((r) => item(r.color, r.name, MB(r.size), (r.size / m.total * 100).toFixed(0) + '%', r.on ? '' : 'color: #8a8a88',
            switches ? flip(`data-act="memOn" data-arg="${r.p}"`, r.on, r.name + ' gets memory', true) : '')).join('')}${m.over
            ? item('#e0675d', 'over', MB(m.used - m.total), '+' + ((m.used - m.total) / m.total * 100).toFixed(0) + '%', 'color: #ff8f86')
            : item('#5a5a58', 'free', MB(m.total - m.used), ((m.total - m.used) / m.total * 100).toFixed(0) + '%', '')}</div>`;
    }
    function memSummaryHtml() {
        const m = memModel();
        return `<section class="card"><div class="card-h"><h2 class="card-t">DMA memory</h2><span class="hint">block ${fmtBytes(m.block)} · 12 blocks per stream minimum</span></div>
            <div style="display: flex; flex-direction: column; gap: 8px">${memBarHtml(m)}${memLegendHtml(m)}</div></section>`;
    }
    function memHtml() {
        const m = memModel();
        const maxBlock = Number(pmax('MM_BLOCK_SIZE', 0)) || 8388608;
        const presets = [2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608].filter((b) => b <= maxBlock);
        const live = Math.max(1, m.rows.filter((r) => r.on).length);
        const fits = (b) => 12 * (b + HEADER) * live <= m.total;
        const isPreset = presets.includes(m.block) && !S.memBlockCustom;
        const opts = presets.map((b) => [b, fmtBytes(b) + (fits(b) ? '' : ' - 12 blocks per stream do not fit')]).concat([['custom', 'Custom…']]);
        return secHead('Memory', 'The DMA memory of the board, split between the streams. A new block size stops every stream.') +
            `<section class="card">
              <div class="card-h">
                <div class="row" style="gap: 12px; align-items: flex-end">
                  <div class="field"><label for="mb">Block size</label><select id="mb" data-act="memBlock" style="min-width: 230px">${sel(opts, isPreset ? m.block : 'custom')}</select></div>
                  ${isPreset ? '' : `<div class="field"><label for="mbc">Custom, bytes</label><input id="mbc" class="mono" style="width: 130px" value="${m.block}" data-act="memBlockCustom"></div>`}
                </div>
                <div class="row" style="gap: 8px"><button class="btn" type="button" style="${NEUTRAL}" data-act="memMin">Minimum</button><button class="btn" type="button" style="${NEUTRAL}" data-act="memEven">Split evenly</button></div>
              </div>
              <div class="grid" style="grid-template-columns: repeat(auto-fill, minmax(210px, 1fr))">${m.rows.map((r) => `
                <div class="field" style="${r.on ? '' : 'opacity: .45'}"><label for="mem_${r.k}" style="display: flex; align-items: center; gap: 8px"><span class="dot" style="background: ${r.color}"></span>${r.name}, bytes</label>
                  <div style="display: flex; gap: 4px">
                    <button class="btn ibtn" type="button" aria-label="${r.name}: one block less" data-act="memStep" data-arg="${r.p}:-1">−</button>
                    <input id="mem_${r.k}" class="mono" style="flex: 1; min-width: 0" value="${r.size}" data-act="memSize" data-arg="${r.p}">
                    <button class="btn ibtn" type="button" aria-label="${r.name}: one block more" data-act="memStep" data-arg="${r.p}:1">+</button>
                  </div>
                  <span class="hint mono${r.ok ? '' : ' bad'}">${r.on ? `${r.n} blocks · ${MB(r.size)}${r.n < 12 ? ' · at least 12' : ''}${r.size % m.blk ? ' · ' + (r.size % m.blk) + ' B unused' : ''}` : 'off · no memory, the stream does not start'}</span>
                </div>`).join('')}</div>
              <div style="display: flex; flex-direction: column; gap: 8px">${memBarHtml(m)}${memLegendHtml(m, true)}</div>
              <p class="hint${m.over ? ' bad' : ''}" style="margin: 0">${m.over ? 'Over by ' + MB(m.used - m.total) + ': ' : ''}used ${MB(m.used)} of ${MB(m.total)}, free ${MB(Math.max(0, m.total - m.used))}. Each stream needs at least 12 blocks of ${fmtBytes(m.block)} + ${HEADER} B.</p>
            </section>`;
    }

    // ---- ADC
    function adcHtml() {
        const s = stream('adc');
        const sd = !!v('SS_USE_FILE', false);
        const n = adcCount();
        const ranges = !!v('SS_IS_ATTENUATOR', false), acdc = !!v('SS_IS_AC_DC', false);
        const att = Number(v('SS_ATTENUATOR', 0)), ac = Number(v('SS_AC_DC', 0));
        const per = adcRate() / adcDec();
        const bytes = Number(v('SS_RESOLUTION', 1)) ? 2 : 1;
        const net = per * chList().length * bytes;
        const ins = Array.from({ length: n }, (_, i) => 'IN' + (i + 1)).join(', ');
        let rows = '';
        for (let i = 0; i < n; i++) {
            const bit = 1 << i, name = 'IN' + (i + 1);
            rows += `<tr><td>${name}</td><td>${flip(`data-act="bit" data-arg="SS_CHANNEL:${bit}"`, chMask() & bit, name + ' on', true)}</td>
              <td>${ranges ? `<select aria-label="${name} range" data-act="bitsel" data-arg="SS_ATTENUATOR:${bit}">${sel([[0, '1:1 (±1 V)'], [1, '1:20 (±20 V)']], att & bit ? 1 : 0)}</select>` : '<span class="hint">one range</span>'}</td>
              <td>${acdc ? `<select aria-label="${name} coupling" data-act="bitsel" data-arg="SS_AC_DC:${bit}">${sel([[0, 'DC'], [1, 'AC']], ac & bit ? 1 : 0)}</select>` : '<span class="hint">DC only</span>'}</td></tr>`;
        }
        const tdms = sd && Number(v('SS_FORMAT', 0)) === 1;
        return secHead('ADC', `The inputs ${ins} to a network client or to a file on the SD card.`, badge(s) + (v('SS_TRIG_MODE', 0) ? softBtn(1) : '') + runBtn(s)) + errNote(s) +
            `<section class="card">
              <div class="card-h"><h2 class="card-t">Destination</h2>${seg('ADC destination', [['adcSd', 0, 'Network client', !sd, s.run && 'Stop the ADC to change the destination'], ['adcSd', 1, 'File on SD card', sd, s.run && 'Stop the ADC to change the destination']])}</div>
              ${sd ? `<div class="grid" style="grid-template-columns: repeat(auto-fill, minmax(180px, 1fr))">
                  <div class="field"><label for="fmt">Format</label><select id="fmt" data-param="SS_FORMAT">${sel([[0, 'WAV'], [1, 'TDMS'], [2, 'BIN']], v('SS_FORMAT', 0))}</select></div>
                  <div class="field"><label for="dtype">Values</label><select id="dtype" data-param="SS_SAVE_MODE">${sel([[0, 'Raw ADC codes'], [1, 'Volts']], v('SS_SAVE_MODE', 0))}</select></div>
                  <div class="field"><label for="lim">Samples per channel</label><input id="lim" class="mono" value="${esc(v('SS_SAMPLES', 0))}" data-act="int" data-arg="SS_SAMPLES"><span class="hint">0 - until the stop</span></div>
                  ${tdms ? `<div class="field"><label for="tcol">TDMS time column</label><select id="tcol" data-param="SS_TDMS_TIME">${sel([[0, 'Off'], [1, 'On']], v('SS_TDMS_TIME', 0))}</select></div>` : ''}
                </div>
                ${recordingHtml('adc')}`
              : '<p class="sub">A client takes the data on port <span class="mono">18900</span>: rpsa_client or the streaming API.</p>' + (runMode('adc') === 1 ? lostHtml('adc') : '')}
            </section>
            <div class="grid" style="grid-template-columns: repeat(auto-fit, minmax(320px, 1fr))">
              <section class="card"><h2 class="card-t">Channels</h2>
                <table><thead><tr><th scope="col">Input</th><th scope="col">On</th><th scope="col">Range</th><th scope="col">Coupling</th></tr></thead><tbody>${rows}</tbody></table>
                ${chList().length ? '' : '<p class="hint bad" style="margin: 0">No input is on: the ADC does not start.</p>'}
              </section>
              <section class="card"><h2 class="card-t">Sampling</h2>
                <div class="grid" style="grid-template-columns: repeat(auto-fill, minmax(220px, 1fr))">
                  <div class="field"><label for="dec">Decimation</label><input id="dec" class="mono" value="${adcDec()}" data-act="dec"><span class="hint mono">${fmtRate(per)} per channel</span></div>
                  <div class="field"><label for="res">Resolution</label><select id="res" data-param="SS_RESOLUTION">${sel([[1, '16 bit'], [0, '8 bit']], v('SS_RESOLUTION', 1))}</select></div>
                </div>
                <div class="row" style="gap: 20px">
                  <label class="check">${flip('data-act="flag" data-arg="SS_USE_CALIB"', Number(v('SS_USE_CALIB', 1)))}Calibration</label>
                  <label class="check">${flip('data-act="flag" data-arg="SS_CAPTURE_TIME"', Number(v('SS_CAPTURE_TIME', 0)))}Time of each buffer</label>
                </div>
                <p class="hint" style="margin: 0">Network: ${fmtBytes(net)}/s for ${chList().length} channel${chList().length === 1 ? '' : 's'} at ${bytes * 8} bit.</p>
              </section>
            </div>
            ${adcTrigHtml()}`;
    }

    // the level window of the ADC trigger in volts: the codes come from the first chosen input
    function adcLevel() {
        const mask = Number(v('SS_TRIG_SOURCES', 0));
        const ins = [];
        for (let i = 0; i < adcCount(); i++) if (mask & inBit(i)) ins.push(i);
        const ref = ins.length ? ins[0] : 0, fs = fsOf(ref);
        const lo = Number(v('SS_TRIG_LEVEL_LOW', -4)), hi = Number(v('SS_TRIG_LEVEL_HIGH', 4));
        const rising = Number(v('SS_TRIG_EDGE', 0)) === 0;
        // the board keeps the window in volts (SS_TRIG_LEVEL_V / SS_TRIG_HYST_V) and makes the codes
        if (Number(v('SS_TRIG_LEVEL_UNIT', 0)) === 1) return { ins, ref, fs, rising, level: Number(v('SS_TRIG_LEVEL_V', 0)), hyst: Number(v('SS_TRIG_HYST_V', 0)) };
        return { ins, ref, fs, rising, level: (rising ? hi : lo) * fs / 32768, hyst: (hi - lo) * fs / 32768 };
    }

    function adcTrigHtml() {
        const on = !!Number(v('SS_TRIG_MODE', 0));
        const ts = TS();
        const lv = adcLevel();
        const mask = Number(v('SS_TRIG_SOURCES', 0));
        const P = 'SS_TRIG_SOURCES';
        let ins = '';
        for (let i = 0; i < adcCount(); i++) ins += srcRow(P, inBit(i), 'IN' + (i + 1) + ' level', 'crosses the level window', true, '');
        let others = '';
        if (hasDac()) {
            const out = !!v('SS_DAC_TRIG_OUTPUTS', false), why = 'needs a newer FPGA (DAC trigger outputs)';
            others += srcRow(P, SRC.dacA, 'DAC A', 'when OUT1 starts playing', out, why) + srcRow(P, SRC.dacB, 'DAC B', 'when OUT2 starts playing', out, why);
        }
        if (hasGpio()) others += srcRow(P, SRC.gpio, 'GPIO pattern', 'the lines match the pattern (GPIO)', true, '');
        const ext = srcRow(P, SRC.ext, 'External', 'EXT TRIG input, set in Triggers', true, '');
        const blocks = Number(v('SS_TRIG_BLOCKS', 0));
        const bps = Number(v('SS_RESOLUTION', 1)) ? 2 : 1;
        const blockSamples = Math.floor(Number(v('MM_BLOCK_SIZE', 65536)) / bps);
        const per = adcRate() / adcDec();
        return `<section class="card">
          <div class="card-h"><h2 class="card-t">Trigger</h2>
            <div class="row">
              <span class="stat"><span class="dot" style="background: ${on && ts.adcArmed ? '#47b2d1' : '#a8a8a6'}"></span>${on ? (ts.adcArmed ? 'Armed' : 'Not armed') : 'Off'}</span>
              <label class="check">${flip('data-act="flag" data-arg="SS_TRIG_MODE"', on)}Wait for a trigger</label>
            </div>
          </div>
          ${on ? `
          <div class="field"><label>Start on any of</label>
            <div class="tsrc">
              <div class="grp"><span>Inputs</span>${ins}</div>
              ${others ? `<div class="grp"><span>Other streams</span>${others}</div>` : ''}
              <div class="grp"><span>Board</span>${ext}</div>
            </div>
            ${mask ? '' : '<span class="hint">No source: only the software trigger starts the ADC.</span>'}
          </div>
          ${lv.ins.length ? `<div class="pane"><div class="pane-t">Level of ${lv.ins.map((i) => 'IN' + (i + 1)).join(', ')}</div>
            <div class="grid" style="grid-template-columns: repeat(auto-fill, minmax(150px, 1fr))">
              <div class="field"><label for="tl">Level, V</label><input id="tl" class="mono" value="${fmtV(lv.level)}" data-act="trigLevel"></div>
              <div class="field"><label for="th">Hysteresis, V</label><input id="th" class="mono" value="${fmtV(lv.hyst)}" data-act="trigLevel"></div>
              <div class="field"><label>Edge</label>${seg('Trigger edge', [['setv', 'SS_TRIG_EDGE:0', 'Rising', lv.rising], ['setv', 'SS_TRIG_EDGE:1', 'Falling', !lv.rising]])}</div>
            </div></div>` : ''}
          ${mask & SRC.ext ? `<p class="hint" style="margin: 0">External: ${Number(v('SS_EXT_EDGE', 0)) ? 'falling' : 'rising'} edge, debounce ${esc(v('SS_EXT_DEBOUNCE', 0))} clocks · <a href="#" data-go="trig">change in Triggers</a></p>` : ''}
          <div class="field"><label>After a trigger</label>${seg('ADC capture after a trigger', [['setv', 'SS_TRIG_BLOCKS:0', 'Continuous', !blocks], ['trigBlocks', '', 'Blocks per trigger', !!blocks]])}</div>
          ${blocks ? `<div class="grid" style="grid-template-columns: repeat(auto-fill, minmax(170px, 1fr))">
              <div class="field"><label for="tb">Blocks per trigger</label><input id="tb" class="mono" value="${blocks}" data-act="int" data-arg="SS_TRIG_BLOCKS"><span class="hint">1 .. 65535</span></div>
              <div class="field"><label>Block</label><b class="mono val">${blockSamples.toLocaleString('en-US')} samples</b><span class="hint">${fmtBytes(Number(v('MM_BLOCK_SIZE', 65536)))}, set in Memory</span></div>
              <div class="field"><label>Series</label><b class="mono val">${(blockSamples * blocks).toLocaleString('en-US')} samples</b><span class="hint mono">${(blockSamples * blocks / per * 1000).toFixed(1)} ms</span></div>
            </div>` : ''}
          <details class="adv"><summary>Advanced: start and stop with another stream</summary>
            <div class="grid" style="grid-template-columns: repeat(auto-fill, minmax(220px, 1fr)); margin-top: 10px">
              <div class="field"><label for="tev">Start and stop together with</label><select id="tev" data-param="SS_TRIG_EVENT">${sel(evOpts('adc'), v('SS_TRIG_EVENT', -1))}</select></div>
            </div>
          </details>
          <p class="hint" style="margin: 0">Armed: <b class="mono" style="color: #e8e8e6">${ts.adcArmed ? 'yes' : 'no'}</b> · triggers seen: <b class="mono" style="color: #e8e8e6">${(ts.adcTriggerCount || 0).toLocaleString('en-US')}</b></p>` : ''}
        </section>`;
    }

    // ---- DAC
    function dacHtml() {
        const s = stream('dac');
        const sd = !!Number(v('SS_DAC_MODE', 0));
        const f = dacFiles();
        const rep = String(v('SS_DAC_REPEAT', -1));
        const gain = !!v('SS_IS_DAC_GAIN', false), g = Number(v('SS_DAC_GAIN', 0));
        let outs = '';
        for (let i = 0; i < dacCount(); i++) {
            const bit = 1 << i, name = 'OUT' + (i + 1);
            outs += `<tr><td>${name}</td><td>${gain ? `<select aria-label="${name} gain" data-act="bitsel" data-arg="SS_DAC_GAIN:${bit}">${sel([[0, 'x1'], [1, 'x5']], g & bit ? 1 : 0)}</select>` : '<span class="hint">x1 (no switch)</span>'}</td>
              <td>${flip(`data-act="bit" data-arg="SS_DAC_TRIG_MODE:${bit}"`, Number(v('SS_DAC_TRIG_MODE', 0)) & bit, name + ' waits for a trigger', true)}</td></tr>`;
        }
        const waits = Number(v('SS_DAC_TRIG_MODE', 0));
        const ts = TS();
        const armed = Number(ts.dacArmed || 0);
        let srcs = '';
        for (let i = 0; i < adcCount(); i++) srcs += srcRow('SS_DAC_TRIG_SOURCES', inBit(i), 'IN' + (i + 1) + ' level', 'the level window of the ADC trigger', true, '');
        if (hasGpio()) srcs += srcRow('SS_DAC_TRIG_SOURCES', SRC.gpio, 'GPIO pattern', 'the pattern of the GPIO input', true, '');
        srcs += srcRow('SS_DAC_TRIG_SOURCES', SRC.ext, 'External', 'EXT TRIG input, set in Triggers', true, '');
        return secHead('DAC', (dacCount() === 1 ? 'OUT1' : 'OUT1, OUT2') + ' from a network client or from a file on the SD card.', badge(s) + (waits ? softBtn(2) : '') + runBtn(s)) + errNote(s) +
            `<section class="card">
              <div class="card-h"><h2 class="card-t">Source</h2>${seg('DAC source', [['dacSd', 0, 'Network client', !sd], ['dacSd', 1, 'File on SD card', sd]])}</div>
              ${sd ? `<div class="grid" style="grid-template-columns: repeat(auto-fill, minmax(200px, 1fr))">
                  <div class="field"><label for="dfile">File</label>${f.list.length
                      ? `<select id="dfile" data-act="dacFile">${f.cur && !f.list.includes(f.cur) ? `<option value="${esc(f.cur)}" selected>${esc(f.cur)} (missing)</option>` : ''}${sel(f.list.map((x) => [x, x]), f.cur)}</select>`
                      : '<span class="val hint">no files: upload one</span>'}</div>
                  <div class="field"><label>&#8203;</label><button class="btn" type="button" style="${NEUTRAL}" data-act="upload"><img src="img/upload.png" alt="" style="width: 18px; height: 18px">Upload file…</button></div>
                  <div class="field"><label for="drep">Repeat</label><select id="drep" data-param="SS_DAC_REPEAT">${sel([[-1, 'Once'], [0, 'N times'], [-2, 'Forever']], rep)}</select></div>
                  ${rep === '0' ? `<div class="field"><label for="dcnt">Times</label><input id="dcnt" class="mono" value="${esc(v('SS_DAC_REPEAT_COUNT', 1))}" data-act="int" data-arg="SS_DAC_REPEAT_COUNT"></div>` : ''}
                </div>
                ${S.upload && S.uploadDir === 'dac' ? `<p class="hint${S.uploadBad ? ' bad' : ''}" style="margin: 0">${esc(S.upload)}</p>` : ''}
                <p class="hint" style="margin: 0">WAV, TDMS or a BIN recording of the ADC.</p>
                <label class="check">${flip('data-act="flag" data-arg="SS_DAC_PLAY_GAPS"', Number(v('SS_DAC_PLAY_GAPS', 0)))}BIN recording: play the samples it lost or waited for a trigger (as 0, in its own time)</label>`
              : '<p class="sub">A client sends the data on port <span class="mono">18903</span>; with an endless repeat the board loops what it got.</p>'}
            </section>
            <div class="grid" style="grid-template-columns: repeat(auto-fit, minmax(320px, 1fr))">
            <section class="card"><h2 class="card-t">Output</h2>
              <div class="field"><label for="drate">Sample rate (S/s)</label><input id="drate" class="mono" value="${esc(v('SS_DAC_HZ', 0))}" data-act="int" data-arg="SS_DAC_HZ"><span class="hint">${fmtRate(Number(pmin('SS_DAC_HZ', 0)))} .. ${fmtRate(Number(pmax('SS_DAC_HZ', 0)))}</span></div>
              <table><thead><tr><th scope="col">Output</th><th scope="col">Gain</th><th scope="col">Wait for a trigger</th></tr></thead><tbody>${outs}</tbody></table>
            </section>
            <section class="card"><h2 class="card-t">Trigger</h2>
              ${waits ? `<div class="field"><label>Start on any of</label><div style="display: flex; flex-direction: column; gap: 6px">${srcs}</div>
                  ${Number(v('SS_DAC_TRIG_SOURCES', 0)) ? '' : '<span class="hint">No source: only the software trigger starts the waiting outputs.</span>'}</div>`
                : '<p class="sub">No output waits for a trigger: switch it on in the table of the outputs.</p>'}
              <div class="field"><label for="dev">Start and stop together with</label><select id="dev" data-param="SS_DAC_EVENT">${sel(evOpts('dac'), v('SS_DAC_EVENT', -1))}</select></div>
              <p class="hint" style="margin: 0">Armed: <b class="mono" style="color: #e8e8e6">OUT1 ${armed & 1 ? 'yes' : 'no'}${dacCount() > 1 ? ' · OUT2 ' + (armed & 2 ? 'yes' : 'no') : ''}</b></p>
            </section>
            </div>`;
    }

    // ---- files
    function filesHtml() {
        const list = S.files[S.fileTab];
        const any = list.some((f) => S.fileSel[f.name]);
        const all = list.length > 0 && list.every((f) => S.fileSel[f.name]);
        const n = list.filter((f) => S.fileSel[f.name]).length;
        const dir = S.fileTab;
        const tabs = [['fileTab', 'adc', 'ADC recordings', S.fileTab === 'adc']];
        if (hasDac()) tabs.push(['fileTab', 'dac', 'DAC files', S.fileTab === 'dac']);
        if (hasGpio()) tabs.push(['fileTab', 'gpio', 'GPIO files', S.fileTab === 'gpio']);
        return secHead('Files on the SD card', '/home/redpitaya/streaming_files' + (sdSpace() ? ' · ' + fmtBytes(sdSpace()[0]) + ' free of ' + fmtBytes(sdSpace()[1]) : ''), seg('Folder', tabs)) +
            `<section class="card">
              <div class="row">
                ${dir !== 'adc' ? `<button class="btn" type="button" style="${NEUTRAL}" data-act="upload"><img src="img/upload.png" alt="" style="width: 18px; height: 18px">Upload…</button>` : ''}
                <button class="btn btn-d${any ? '' : ' btn-off'}" type="button" data-act="delete" aria-disabled="${!any}"><img src="img/delete.png" alt="" style="width: 18px; height: 18px">Delete selected${any ? ' (' + n + ')' : ''}</button>
                <button class="btn" type="button" style="${NEUTRAL}" data-act="refreshFiles">Refresh</button>
                <span class="hint">${{ adc: 'Recordings of the ADC with their reports (.log.txt)', dac: 'WAV, TDMS or BIN files the DAC plays', gpio: 'Recordings of the GPIO input and files for the output' }[dir]}</span>
              </div>
              ${S.upload && S.uploadDir === dir ? `<p class="hint${S.uploadBad ? ' bad' : ''}" style="margin: 0">${esc(S.upload)}</p>` : ''}
              ${S.filesError ? `<p class="hint bad" style="margin: 0">${esc(S.filesError)}</p>` : ''}
              <div style="overflow-x: auto"><table>
                <thead><tr><th scope="col" style="width: 1%"><input type="checkbox" class="cbx${any && !all ? ' cbx-mixed' : ''}" data-act="selAll"${all ? ' checked' : ''} aria-label="Select all"></th><th scope="col">Name</th><th scope="col">Size</th><th scope="col">Written</th>${dir === 'adc' ? '<th scope="col">Report</th>' : ''}<th scope="col"></th></tr></thead>
                <tbody>${list.length ? list.map((f) => `<tr class="${S.fileSel[f.name] ? 'sel' : ''}">
                  <td><input type="checkbox" class="cbx" data-act="sel" data-arg="${esc(f.name)}"${S.fileSel[f.name] ? ' checked' : ''} aria-label="Select ${esc(f.name)}"></td>
                  <td class="mono">${esc(f.name)}</td><td class="mono">${esc(f.size)}</td><td class="mono">${esc(f.date)}</td>
                  ${dir === 'adc' ? `<td>${f.reports.map(([label, file]) => `<a href="/streaming_manager/upload/adc/${encodeURIComponent(file)}" target="_blank">${label}</a>`).join(' · ') || '<span class="hint">-</span>'}</td>` : ''}
                  <td><a href="/streaming_manager/upload/${dir}/${encodeURIComponent(f.name)}" download>Download</a></td></tr>`).join('')
                  : `<tr><td colspan="6" class="hint">No files.</td></tr>`}</tbody></table></div>
            </section>`;
    }

    // ---- settings and clients
    const CLIENT_KINDS = [
        [/rpsa_client-desktop.*win\.zip$/, 'rpsa_client desktop · Windows', 'img/DeskWinGreen.png'],
        [/rpsa_client-desktop/, 'rpsa_client desktop · Linux', 'img/DeskLinGreen.png'],
        [/win\.zip$/, 'rpsa_client · Windows', 'img/ConWinGreen.png'],
        [/rp\.zip$/, 'rpsa_client · Red Pitaya', 'img/ConRPGreen.png'],
        [/arm64\.zip$/, 'rpsa_client · Raspberry Pi (arm64)', 'img/ConRPIGreen.png'],
        [/./, 'rpsa_client · Linux x86-64', 'img/ConLinGreen.png']
    ];
    function setHtml() {
        const clients = S.clients.map((f) => {
            const k = CLIENT_KINDS.find(([re]) => re.test(f));
            return `<tr title="${esc(f)}"><td><span class="row" style="gap: 10px; flex-wrap: nowrap"><img src="${k[2]}" alt="" style="width: 26px; height: 26px; flex: none">${esc(k[1])}</span></td><td style="text-align: right"><a href="/streaming_manager/clients/${encodeURIComponent(f)}" download>Download</a></td></tr>`;
        }).join('');
        return secHead('Settings and clients', 'The settings of the board as a whole, the clients and the network.') +
            `<div class="grid" style="grid-template-columns: repeat(auto-fit, minmax(320px, 1fr))">
              <section class="card"><h2 class="card-t">Settings</h2>
                <p class="sub">Changes take effect at once and are saved on the board as they change.</p>
                <div class="row">
                  <button class="btn" type="button" data-act="setDefaults">Defaults</button>
                  <button class="btn" type="button" style="${NEUTRAL}" data-act="setExport">Export JSON</button>
                  <button class="btn" type="button" style="${NEUTRAL}" data-act="setImport">Import JSON…</button>
                </div>
                ${S.setMsg ? `<p class="hint${S.setMsg.startsWith('Refused') ? ' bad' : ''}" style="margin: 0">${esc(S.setMsg)}</p>` : ''}
              </section>
              <section class="card"><h2 class="card-t">Clients</h2>
                <p class="sub">Stream to your computer with the console client or the API (C++, Python).</p>
                <table><tbody>${clients || '<tr><td class="hint">No clients on the board.</td></tr>'}</tbody></table>
              </section>
              <section class="card"><h2 class="card-t">Network</h2>
                <div class="kv">
                  <div><span>Settings</span><b>18901</b></div><div><span>Board discovery (UDP)</span><b>18902</b></div>
                  <div><span>ADC data</span><b>18900</b></div><div><span>DAC data</span><b>18903</b></div>
                  <div><span>GPIO input data</span><b>18904</b></div><div><span>GPIO output data</span><b>18905</b></div>
                </div>
                <p class="hint" style="margin: 0">One client at a time: a new client takes the board over and the streams of the old one stop. Settings are saved on the board as they change.</p>
              </section>
            </div>`;
    }

    // ---------------------------------------------------------------------------------------
    // rendering
    // The new markup is laid onto the nodes that are there: only what differs changes. A
    // rebuild (innerHTML) on every update of the stream replaced the button under the mouse,
    // its hover flickered and a click whose press and release hit different buttons was lost.
    function syncAttrs(from, to) {
        for (const { name } of Array.from(from.attributes)) {
            if (!to.hasAttribute(name)) from.removeAttribute(name);
        }
        for (const { name, value } of Array.from(to.attributes)) {
            if (from.getAttribute(name) !== value) from.setAttribute(name, value);
        }
        // the state the user changes lives in properties, not in attributes
        if (from.tagName === 'INPUT') {
            if (from.type === 'checkbox' || from.type === 'radio') {
                const on = to.hasAttribute('checked');
                if (from.checked !== on) from.checked = on;
            } else {
                const val = to.getAttribute('value') || '';
                if (from.value !== val) from.value = val;
            }
        } else if (from.tagName === 'OPTION') {
            const on = to.hasAttribute('selected');
            if (from.selected !== on) from.selected = on;
        }
    }
    function morphChildren(from, to) {
        const a = Array.from(from.childNodes), b = Array.from(to.childNodes);
        for (let i = 0; i < b.length; i++) {
            const n = b[i], o = a[i];
            if (!o) {
                from.appendChild(n);
            } else if (o.nodeType !== n.nodeType || o.nodeName !== n.nodeName) {
                from.replaceChild(n, o);
            } else if (o.nodeType === Node.ELEMENT_NODE) {
                syncAttrs(o, n);
                morphChildren(o, n);
            } else if (o.nodeValue !== n.nodeValue) {
                o.nodeValue = n.nodeValue;
            }
        }
        for (let i = a.length - 1; i >= b.length; i--) from.removeChild(a[i]);
        // a select takes the selected option once its options are in place
        if (from.tagName === 'SELECT') {
            const s = Array.from(from.options).findIndex((x) => x.hasAttribute('selected'));
            if (s >= 0 && from.selectedIndex !== s) from.selectedIndex = s;
        }
    }
    // always compared with the live DOM: a red border of flash() or a checkbox the board did
    // not take goes back to the markup
    function patch(el, html) {
        const t = document.createElement('template');
        t.innerHTML = html;
        morphChildren(el, t.content);
    }

    let pending = false;
    function render() {
        pending = false;
        const a = document.activeElement;
        // an input being edited keeps its text: render again when it loses the focus
        if (a && a.tagName === 'INPUT' && a.type !== 'checkbox' && a.closest('#main')) {
            pending = true;
            return;
        }
        if ((S.section === 'dac' && !hasDac()) || (S.section === 'gpio' && !hasGpio())) S.section = 'overview';
        patch(document.getElementById('nav'), navHtml());
        const html = { overview: overviewHtml, adc: adcHtml, dac: dacHtml, gpio: gpioHtml, trig: trigHtml, mem: memHtml, files: filesHtml, set: setHtml }[S.section]();
        const main = document.getElementById('main');
        const focusId = a && a.id && main.contains(a) ? a.id : '';
        patch(main, html);
        // a node of another kind in its place loses the focus: give it to the new one
        if (focusId && document.activeElement !== a) {
            const el = document.getElementById(focusId);
            if (el) el.focus();
        }
    }
    let raf = 0;
    const schedule = () => { if (!raf) raf = requestAnimationFrame(() => { raf = 0; render(); }); };

    // ---------------------------------------------------------------------------------------
    // actions
    const toInt = (s) => { const n = Number(String(s).trim()); return Number.isFinite(n) ? Math.round(n) : NaN; };
    const flash = (el) => { el.classList.add('bad'); el.style.borderColor = '#e0675d'; setTimeout(schedule, 800); };
    function setBit(param, bit, on) {
        const m = Number(v(param, 0));
        C.set(param, on ? m | bit : m & ~bit);
    }

    const ACT = {
        run: (el) => {
            const s = stream(el.dataset.arg);
            C.set(s.start, !s.run);
        },
        adcSd: (el) => C.set('SS_USE_FILE', el.dataset.arg === '1'),
        dacSd: (el) => C.set('SS_DAC_MODE', Number(el.dataset.arg)),
        bit: (el) => { const [p, b] = el.dataset.arg.split(':'); setBit(p, Number(b), el.checked); },
        none: () => schedule(),
        setv: (el) => { const [p, val] = el.dataset.arg.split(':'); C.set(p, Number(val)); },
        soft: (el) => C.set('SS_SOFT_TRIGGER', Number(el.dataset.arg)),
        trigLevel: () => {
            const lv = adcLevel();
            const L = Number(String(document.getElementById('tl').value).replace(',', '.'));
            const H = Number(String(document.getElementById('th').value).replace(',', '.'));
            if (!Number.isFinite(L) || !Number.isFinite(H) || H < 0) { flash(document.activeElement || document.getElementById('tl')); return; }
            const lo = lv.rising ? L - H : L, hi = lv.rising ? L : L + H;
            if (Math.abs(lo) > lv.fs || Math.abs(hi) > lv.fs) { flash(document.getElementById('tl')); return; }
            if (has('SS_TRIG_LEVEL_V')) {
                C.parametersCache.SS_TRIG_LEVEL_V = { value: L };
                C.parametersCache.SS_TRIG_HYST_V = { value: H };
            } else {  // an older module: the codes from here
                C.parametersCache.SS_TRIG_LEVEL_LOW = { value: toCode(lo, lv.fs) };
                C.parametersCache.SS_TRIG_LEVEL_HIGH = { value: toCode(hi, lv.fs) };
            }
            C.sendParameters();
        },
        trigBlocks: () => { if (!Number(v('SS_TRIG_BLOCKS', 0))) C.set('SS_TRIG_BLOCKS', 10); },
        gline: (el) => {
            const [which, k] = el.dataset.arg.split(':');
            const bit = 1 << Number(k);
            if (which === 'pol') { C.set('SS_GPIO_POLARITY', Number(v('SS_GPIO_POLARITY', 0)) ^ bit); return; }
            const mask = (Number(v('SS_GPIO_OUTPUTS_P', 0)) | (Number(v('SS_GPIO_OUTPUTS_N', 0)) << 8)) ^ bit;
            C.parametersCache.SS_GPIO_OUTPUTS_P = { value: mask & 0xFF };
            C.parametersCache.SS_GPIO_OUTPUTS_N = { value: (mask >> 8) & 0xFF };
            C.sendParameters();
        },
        gall: (el) => {
            const [which, on] = el.dataset.arg.split(':');
            if (which === 'pol') { C.set('SS_GPIO_POLARITY', on === '1' ? 0xFFFF : 0); return; }
            C.parametersCache.SS_GPIO_OUTPUTS_P = { value: on === '1' ? 0xFF : 0 };
            C.parametersCache.SS_GPIO_OUTPUTS_N = { value: on === '1' ? 0xFF : 0 };
            C.sendParameters();
        },
        gpat: (el) => {
            const pats = Array.from({ length: 16 }, (_, k) => patOf(k));
            const k = Number(el.dataset.arg);
            pats[k] = (pats[k] + 1) % 5;
            setPattern(pats);
        },
        gpatClear: () => setPattern(Array(16).fill(0)),
        gsrc: (el) => { if (el.value !== 'mixed') C.set('SS_GPIO_TRIG_SOURCES', SRC.gpio | Number(el.value)); },
        gfile: (el) => C.set('SS_GPIO_OUT_FILE', el.value),
        uploadGpio: () => pickFile('.bin', (f) => uploadFile(f, 'gpio')),
        bitsel: (el) => { const [p, b] = el.dataset.arg.split(':'); setBit(p, Number(b), el.value === '1'); },
        flag: (el) => C.set(el.dataset.arg, el.checked ? 1 : 0),
        int: (el) => {
            const p = el.dataset.arg, n = toInt(el.value);
            const lo = Number(pmin(p, -Infinity)), hi = Number(pmax(p, Infinity));
            if (isNaN(n) || n < lo || n > hi) { flash(el); return; }
            C.set(p, n);
        },
        dec: (el) => {
            const n = toInt(el.value);
            if (isNaN(n) || n < 1) { flash(el); return; }
            C.set('SS_RATE', snapDec(n));
        },
        dacFile: (el) => {
            C.parametersCache.SS_DAC_FILE = { value: el.value };
            C.parametersCache.SS_DAC_FILE_TYPE = { value: fileType(el.value) };
            C.sendParameters();
        },
        upload: () => (S.fileTab === 'gpio' && S.section === 'files' ? pickFile('.bin', (f) => uploadFile(f, 'gpio')) : pickFile('.wav,.tdms,.bin', (f) => uploadFile(f, 'dac'))),
        memBlock: (el) => {
            if (el.value === 'custom') { S.memBlockCustom = true; schedule(); return; }
            S.memBlockCustom = false;
            C.set('MM_BLOCK_SIZE', Number(el.value));
        },
        memBlockCustom: (el) => {
            const n = toInt(el.value);
            if (isNaN(n) || n < 128 || n % 128 || n > Number(pmax('MM_BLOCK_SIZE', Infinity))) { flash(el); return; }
            C.set('MM_BLOCK_SIZE', n);
        },
        memSize: (el) => {
            const n = toInt(el.value);
            if (isNaN(n) || n < 0) { flash(el); return; }
            C.set(el.dataset.arg, n);
        },
        memStep: (el) => {
            const [p, d] = el.dataset.arg.split(':');
            const m = memModel(), cur = Number(v(p, 0));
            const n = Number(d) > 0 ? (Math.floor(cur / m.blk) + 1) * m.blk : Math.max(0, (Math.ceil(cur / m.blk) - 1) * m.blk);
            C.set(p, n);
        },
        memOn: (el) => {
            const m = memModel();
            C.set(el.dataset.arg, el.checked ? 12 * m.blk : 0);
        },
        memMin: () => {
            const m = memModel();
            for (const r of m.rows) if (r.on) C.parametersCache[r.p] = { value: 12 * m.blk };
            C.sendParameters();
        },
        memEven: () => {
            const m = memModel();
            const live = m.rows.filter((r) => r.on);
            if (!live.length) return;
            const per = Math.floor(m.total / live.length / m.blk) * m.blk;
            for (const r of live) C.parametersCache[r.p] = { value: per };
            C.sendParameters();
        },
        fileTab: (el) => { S.fileTab = el.dataset.arg; S.fileSel = {}; refreshFiles(); schedule(); },
        refreshFiles: () => refreshFiles(),
        sel: (el) => { S.fileSel[el.dataset.arg] = el.checked; schedule(); },
        selAll: () => {
            const list = S.files[S.fileTab];
            const all = list.every((f) => S.fileSel[f.name]);
            S.fileSel = all ? {} : Object.fromEntries(list.map((f) => [f.name, true]));
            schedule();
        },
        delete: () => deleteSelected(),
        setDefaults: async () => {
            if (!(await C.ask({ title: 'Defaults', text: 'Set every setting back to its default? A new block size stops the streams.', ok: 'Set the defaults' }))) return;
            S.setMsg = 'Defaults…';
            C.set('SS_SETTINGS_CMD', 1);
            schedule();
        },
        setExport: () => { S.exportWanted = true; C.set('SS_SETTINGS_CMD', 2); },
        setImport: () => pickFile('.json,application/json', (f) => f.text().then((t) => {
            try { JSON.parse(t); } catch (e) { S.setMsg = 'Refused: ' + f.name + ' is not JSON'; schedule(); return; }
            S.setMsg = 'Importing ' + f.name + '…';
            C.set('SS_SETTINGS_JSON', t);
            schedule();
        }))
    };

    // clicks: sections and buttons; changes: inputs, selects, switches
    function onClick(ev) {
        const go = ev.target.closest('[data-go]');
        if (go) {
            S.section = go.dataset.go;
            if (S.section === 'files') refreshFiles();
            if (S.section === 'set') refreshClients();
            schedule();
            return;
        }
        const b = ev.target.closest('button[data-act]');
        if (b && ACT[b.dataset.act]) ACT[b.dataset.act](b, ev);
    }
    function onChange(ev) {
        const el = ev.target;
        if (el.dataset.param) {
            const n = Number(el.value);
            C.set(el.dataset.param, Number.isFinite(n) ? n : el.value);
        } else if (el.dataset.act && ACT[el.dataset.act]) {
            ACT[el.dataset.act](el, ev);
        }
    }

    // ---------------------------------------------------------------------------------------
    // HTTP: files, uploads, clients, IP
    async function listDir(dir) {
        const r = await fetch('/streaming_manager/upload/' + dir + '/', { cache: 'no-store' });
        if (r.status === 404) return [];
        if (!r.ok) throw new Error('listing ' + dir + ': HTTP ' + r.status);
        const doc = new DOMParser().parseFromString(await r.text(), 'text/html');
        const out = [];
        // nginx autoindex: <a href="name">name</a> date time size
        for (const a of doc.querySelectorAll('pre a')) {
            const href = decodeURIComponent(a.getAttribute('href') || '');
            if (!href || href.endsWith('/') || href.startsWith('.')) continue;
            const tail = (a.nextSibling && a.nextSibling.textContent || '').trim().split(/\s+/);
            out.push({ name: href, date: (tail[0] || '') + ' ' + (tail[1] || ''), size: tail[2] || '' });
        }
        return out;
    }
    async function refreshFiles() {
        S.filesError = '';
        refreshSd();
        try {
            const adc = await listDir('adc');
            const reports = new Map();
            const data = [];
            for (const f of adc) {
                const m = f.name.match(/^(.*)\.log(\.lost)?\.txt$/);
                if (m) {
                    if (!reports.has(m[1])) reports.set(m[1], []);
                    reports.get(m[1]).push([m[2] ? 'lost' : 'log', f.name]);
                } else {
                    data.push(f);
                }
            }
            S.files.adc = data.map((f) => Object.assign(f, { reports: (reports.get(f.name) || []).sort() }))
                .sort((x, y) => y.name.localeCompare(x.name));  // data_file_<date>_<time>: newest first
            S.files.dac = hasDac() ? (await listDir('dac')).sort((x, y) => x.name.localeCompare(y.name)) : [];
            S.files.gpio = hasGpio() ? (await listDir('gpio')).filter((f) => !f.name.startsWith('.')).sort((x, y) => y.name.localeCompare(x.name)) : [];
        } catch (e) {
            S.filesError = String(e.message || e);
        }
        schedule();
    }
    async function deleteSelected() {
        const dir = S.fileTab;
        const names = S.files[dir].filter((f) => S.fileSel[f.name]);
        if (!names.length || !(await C.ask({ title: 'Delete files', text: 'Delete ' + names.length + ' file' + (names.length === 1 ? '' : 's') + ' from the SD card?', ok: 'Delete', danger: true }))) return;
        const doomed = [];
        for (const f of names) {
            doomed.push(f.name);
            if (dir === 'adc') for (const [, r] of f.reports) doomed.push(r);
        }
        const failed = [];
        for (const n of doomed) {
            const r = await fetch('/streaming_manager/upload/' + dir + '/' + encodeURIComponent(n), { method: 'DELETE' }).catch(() => null);
            if (!r || !(r.ok || r.status === 404)) failed.push(n);
        }
        S.fileSel = {};
        S.filesError = failed.length ? 'Not deleted: ' + failed.join(', ') : '';
        if (dir === 'dac') C.set('SS_DAC_FILE_CTR', 1);  // the DAC file list of the module
        if (dir === 'gpio') C.set('SS_GPIO_OUT_FILE', gpioFiles().cur);  // the module answers with the list
        refreshFiles();
    }
    function pickFile(accept, then) {
        const inp = document.createElement('input');
        inp.type = 'file';
        inp.accept = accept;
        inp.onchange = () => inp.files[0] && then(inp.files[0]);
        inp.click();
    }
    // the size of a file as the folder listing gives it: bytes, or 12K, 1.2M, 3.4G
    const listedSize = (t) => (/^\d+$/.test(t) ? fmtBytes(Number(t)) : t.replace(/^([\d.]+)([KMG])$/, '$1 $2B'));
    // Why a name cannot go to the board ('' - it can): the board takes a name only, and the page
    // lists the files separated by '*' and line ends; a name starting with '.' is hidden
    function badName(name) {
        if (!name) return 'it is empty';
        if (name === '.' || name === '..') return 'it is not a file name';
        // the board counts bytes (UTF-8), not characters
        if (new TextEncoder().encode(name).length > 255) return 'it is longer than 255 bytes';
        if (name.startsWith('.')) return 'it starts with a dot (a hidden file)';
        const c = [...name].find((ch) => ch === '*' || ch === '/' || ch === '\\' || ch.charCodeAt(0) < 32 || ch.charCodeAt(0) === 127);
        return c ? 'it has ' + (c.charCodeAt(0) < 32 || c.charCodeAt(0) === 127 ? 'a control character' : c) : '';
    }
    // DAC files through /upload_dac_file (a temporary name, then a rename), GPIO files with a PUT
    async function uploadFile(file, dir) {
        S.uploadDir = dir;
        const refuse = (text) => { S.upload = text; S.uploadBad = true; schedule(); };
        const why = badName(file.name);
        if (why) return refuse(`The board does not take the name "${file.name}": ${why}. Rename the file and upload it again.`);
        const sp = sdSpace();
        if (sp && file.size > sp[0]) return refuse(`${file.name} needs ${fmtBytes(file.size)}, the SD card has ${fmtBytes(sp[0])} free.`);
        // a file of this name on the board: replace it only when asked
        const listed = (dir === 'dac' ? dacFiles() : gpioFiles()).list;
        const had = (S.files[dir] || []).find((f) => f.name === file.name);
        if (had || listed.includes(file.name)) {
            const ok = await C.ask({
                title: 'Replace file',
                text: `${file.name} is already on the board${had && had.size ? ' (' + listedSize(had.size) + ')' : ''}. Replace it with the new one (${fmtBytes(file.size)})?`,
                ok: 'Replace', cancel: 'Cancel', danger: true
            });
            if (!ok) {
                S.upload = '';
                S.uploadBad = false;
                schedule();
                return;
            }
        }
        S.uploadBad = false;
        // the GPIO folder appears with the first recording: make it for a PUT (405 when it is there)
        if (dir === 'gpio') await fetch('/streaming_manager/upload/gpio/', { method: 'MKCOL' }).catch(() => {});
        const x = new XMLHttpRequest();
        if (dir === 'dac') x.open('POST', '/upload_dac_file?file=' + encodeURIComponent(file.name));
        else x.open('PUT', '/streaming_manager/upload/gpio/' + encodeURIComponent(file.name));
        x.upload.onprogress = (e) => {
            if (e.lengthComputable) { S.upload = 'Uploading ' + file.name + ': ' + Math.round(e.loaded / e.total * 100) + '%'; schedule(); }
        };
        x.onload = () => {
            if (x.status >= 200 && x.status < 300) {
                S.upload = 'Uploaded ' + file.name;
                if (dir === 'dac') {
                    C.set('SS_DAC_FILE_CTR', 1);
                    C.parametersCache.SS_DAC_FILE = { value: file.name };
                    C.parametersCache.SS_DAC_FILE_TYPE = { value: fileType(file.name) };
                    C.sendParameters();
                } else {
                    C.set('SS_GPIO_OUT_FILE', file.name);
                }
                refreshFiles();
            } else {
                S.upload = 'Upload of ' + file.name + ' failed: ' + (x.responseText || x.status);
                S.uploadBad = true;
            }
            schedule();
        };
        x.onerror = () => { S.upload = 'Upload of ' + file.name + ' failed'; S.uploadBad = true; schedule(); };
        S.upload = 'Uploading ' + file.name + '…';
        schedule();
        x.send(file);
    }
    async function refreshSd() {
        try {
            const t = (await (await fetch('/streaming_manager_sd_free', { cache: 'no-store' })).text()).trim().split(/\s+/).map(Number);
            S.sdFree = t.length === 2 && t.every(Number.isFinite) ? t : null;
        } catch (e) {
            S.sdFree = null;
        }
        schedule();
    }
    async function refreshClients() {
        try {
            const r = await fetch('/streaming_manager_get_clients_list', { cache: 'no-store' });
            S.clients = (await r.text()).split('\n').map((x) => x.trim()).filter(Boolean);
        } catch (e) {
            S.clients = [];
        }
        schedule();
    }
    // the broadcast of the board goes to its own address: eth0, else wlan0 (not link-local)
    async function refreshIp() {
        try {
            const t = await (await fetch('/get_streaming_ip', { cache: 'no-store' })).text();
            const ips = [...t.matchAll(/inet (\d+\.\d+\.\d+\.\d+)/g)].map((m) => m[1]).filter((ip) => !ip.startsWith('169.254.'));
            S.ip = ips[0] || '';
            if (S.ip) C.set('SS_IP_ADDR', S.ip);
        } catch (e) {
            S.ip = '';
        }
        schedule();
    }

    // ---------------------------------------------------------------------------------------
    document.addEventListener('DOMContentLoaded', () => {
        const root = document.getElementById('app');
        root.addEventListener('click', onClick);
        root.addEventListener('change', onChange);
        root.addEventListener('keydown', (ev) => { if (ev.key === 'Enter' && ev.target.tagName === 'INPUT') ev.target.blur(); });
        root.addEventListener('focusout', () => setTimeout(() => { if (pending) render(); }, 0));
        C.onParameters((names) => {
            if (names.includes('SS_SETTINGS_RESULT') && Number(v('SS_SETTINGS_RESULT', 0))) {
                S.setMsg = Number(v('SS_SETTINGS_RESULT', 0)) === 1 ? 'Applied and saved on the board.' : 'Refused: the board kept its settings.';
            }
            if (names.includes('SS_SETTINGS_JSON') && S.exportWanted) {
                S.exportWanted = false;
                const a = document.createElement('a');
                a.href = URL.createObjectURL(new Blob([String(v('SS_SETTINGS_JSON', ''))], { type: 'application/json' }));
                a.download = 'streaming_config.json';
                a.click();
                setTimeout(() => URL.revokeObjectURL(a.href), 1000);
            }
            if (names.includes('SS_BOARD_MODEL') || names.includes('SS_BOARD_MAC')) {
                document.getElementById('strip').textContent = (v('SS_BOARD_MODEL', '') || 'Red Pitaya') + (v('SS_BOARD_MAC', '') ? ' · ' + v('SS_BOARD_MAC', '') : '');
            }
            // a file recording ended: its files are on the card now
            if (names.includes('SS_ADC_DATA_PASS') && !v('SS_ADC_DATA_PASS', false) && S.section === 'files') refreshFiles();
            schedule();
        });
        C.onOpen(() => { refreshIp(); refreshClients(); refreshFiles(); refreshSd(); document.getElementById('app').classList.remove('hidden'); });
        render();
    });
})();
