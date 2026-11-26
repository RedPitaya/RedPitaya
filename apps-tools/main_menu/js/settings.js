/*
 * Red Pitaya settings page: the sections and their data.
 *
 * Live values: the parameters of the main_menu module (settings_net.js CLIENT).
 * Board information: /get_sysinfo. Uptime, the DMA region after a reboot, the boot partition, LEDs,
 * file system check, network: /settings_status. Stored application settings: /settings_stored and
 * /settings_remove (main_menu/nginx.conf, scripts/settings.sh). Root password: /set_root_password.
 */
(function () {
    'use strict';

    const $ = (id) => document.getElementById(id);
    const MB = 1024 * 1024;
    const HISTORY_MS = 60000;
    const TEMP_LIMIT = 85;      // the marker of the gauge (Zynq-7000 commercial grade junction maximum)
    const TEMP_SCALE = 100;     // the end of the gauge
    const TOLERANCE = 5;        // the range bar of the voltages, percent of the nominal

    const S = {
        section: 'device',
        sysinfo: null,
        status: null,
        stored: [],
        temp: [],               // [time, value] of the last minute
        cpu: [],
        uptimeAt: 0,            // when /settings_status gave the uptime
        rateEdit: {}            // the rate inputs being edited are not overwritten
    };

    const esc = (s) => String(s).replace(/[&<>"']/g, (c) => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));
    const val = (n, d) => CLIENT.value(n, d);
    const num = (n) => { const v = Number(val(n, NaN)); return Number.isFinite(v) ? v : NaN; };

    const fmtBytes = (b) => {
        if (!Number.isFinite(b)) return '—';
        if (b >= 1024 * MB) return (b / (1024 * MB)).toFixed(1) + ' GB';
        if (b >= MB) return Math.round(b / MB) + ' MB';
        return Math.round(b / 1024) + ' kB';
    };
    const fmtMHz = (hz) => {
        if (!Number.isFinite(hz) || hz <= 0) return '';
        return String(Math.round(hz / 1000) / 1000);
    };
    const fmtUptime = (s) => {
        if (!Number.isFinite(s)) return '—';
        const d = Math.floor(s / 86400), h = Math.floor((s % 86400) / 3600), m = Math.floor((s % 3600) / 60);
        if (d) return d + ' d ' + h + ' h';
        if (h) return h + ' h ' + m + ' min';
        return m + ' min';
    };
    const fmtDate = (t) => {
        const d = new Date(t * 1000);
        const p = (x) => String(x).padStart(2, '0');
        return d.getFullYear() + '-' + p(d.getMonth() + 1) + '-' + p(d.getDate()) + ' ' + p(d.getHours()) + ':' + p(d.getMinutes());
    };
    const flash = (el, ms = 1500) => {
        el.classList.remove('hidden');
        clearTimeout(el._t);
        el._t = setTimeout(() => el.classList.add('hidden'), ms);
    };
    const getJSON = (url) => fetch(url, { cache: 'no-store' }).then((r) => r.json());
    const postJSON = (url, obj) => fetch(url, {
        method: 'POST', cache: 'no-store', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(obj)
    });

    // ---------------------------------------------------------------------------------------
    // navigation: #device, #sensors, ... in the address
    function go(sec) {
        if (!document.querySelector('.sec[data-sec="' + sec + '"]')) sec = 'device';
        S.section = sec;
        for (const s of document.querySelectorAll('.sec')) s.classList.toggle('on', s.dataset.sec === sec);
        for (const b of document.querySelectorAll('.nav-b')) {
            const on = b.dataset.go === sec;
            b.classList.toggle('on', on);
            if (on) b.setAttribute('aria-current', 'page'); else b.removeAttribute('aria-current');
        }
        if (location.hash !== '#' + sec) history.replaceState(null, '', '#' + sec);
        if (sec === 'maint') loadStored();
    }

    // ---------------------------------------------------------------------------------------
    // header, footer
    function renderHeader() {
        const si = S.sysinfo;
        if (si) {
            $('pill-model').textContent = si.name || si.model || '—';
            $('pill-os').textContent = si.ecosystem ? si.ecosystem.version : '—';
        }
        const calErr = val('RP_CALIB_ERROR_STATE', undefined);
        const c = $('pill-calib');
        if (calErr !== undefined) {
            c.textContent = calErr ? 'ERROR' : 'OK';
            c.className = calErr ? 'bad' : 'ok';
        }
        const clock = val('RP_SYSTEM_CLOCK_STATE', true);
        $('pill-clock').classList.toggle('hidden', clock !== false);
    }

    function renderFooter() {
        const t = num('RP_SYSTEM_TEMPERATURE');
        const c = num('RP_SYSTEM_CPU_LOAD');
        $('ft-temp').textContent = Number.isFinite(t) && t > -100 ? t.toFixed(1) + ' °C' : '—';
        $('ft-cpu').textContent = Number.isFinite(c) ? Math.round(c) + ' %' : '—';
        if (S.status) {
            $('ft-up').textContent = fmtUptime(S.status.uptime + (Date.now() - S.uptimeAt) / 1000);
        }
    }

    // ---------------------------------------------------------------------------------------
    // Device
    const COPY_ICON = '<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" aria-hidden="true"><rect x="9" y="9" width="11" height="11" rx="2"></rect><path d="M5 15V5a2 2 0 0 1 2-2h10"></path></svg>';

    // rows: [label, value, copy, class]
    function rows(list) {
        return list.map(([k, v, copy, cls]) =>
            '<tr><th>' + esc(k) + '</th><td' + (cls ? ' class="' + cls + '"' : '') + '>' + esc(v) + '</td><td class="c">' +
            (copy ? '<button class="ib" type="button" data-copy="' + esc(v) + '" aria-label="Copy ' + esc(k) + '">' + COPY_ICON + '</button>' : '') +
            '</td></tr>').join('');
    }

    function deviceData() {
        const si = S.sysinfo || {};
        const eco = si.ecosystem || {};
        const calErr = val('RP_CALIB_ERROR_STATE', undefined);
        const total = num('RP_SYSTEM_TOTAL_RAM');
        const dma = num('RP_SYSTEM_DMA_RAM');
        const sdT = num('RP_SYSTEM_TOTAL_SD'), sdF = num('RP_SYSTEM_FREE_SD');
        const boot = S.status && S.status.boot;
        const pct = (u, t) => ' (' + Math.round(u / t * 100) + ' %)';
        return {
            board: [
                ['Model', si.name || '—'],
                ['Model ID', (si.model || '—') + (si.is_model_valid === '0' ? ' (not recognized)' : ''), false, si.is_model_valid === '0' ? 'bad' : ''],
                ['MAC', si.mac || '—', !!si.mac],
                ['DNA', si.dna || '—', !!si.dna],
                ['Calibration', calErr === undefined ? '—' : (calErr ? 'ERROR' : 'OK'), false, calErr ? 'bad' : 'ok']
            ],
            soft: [
                ['Ecosystem', eco.version || '—'],
                ['Revision', eco.revision || '—', !!eco.revision],
                ['Linux', si.linux || '—']
            ],
            mem: [
                ['RAM', fmtBytes(total)],
                ['Reserved for DMA', fmtBytes(dma)],
                ['SD, system', Number.isFinite(sdT) && sdT > 0 ? fmtBytes(sdT - sdF) + ' of ' + fmtBytes(sdT) + pct(sdT - sdF, sdT) : '—'],
                ['SD, boot', boot && boot.total ? fmtBytes(boot.used) + ' of ' + fmtBytes(boot.total) + pct(boot.used, boot.total) : '—']
            ],
            fpga: Object.keys(si.fpga || {}).map((k) => [k, si.fpga[k]])
        };
    }

    function renderDevice() {
        const d = deviceData();
        $('t-board').innerHTML = rows(d.board);
        $('t-soft').innerHTML = rows(d.soft);
        $('t-mem').innerHTML = rows(d.mem);
        $('t-fpga').innerHTML = d.fpga.length ? rows(d.fpga) : rows([['—', '—']]);
        const eco = (S.sysinfo && S.sysinfo.ecosystem) || {};
        $('t-msoft').innerHTML = rows([['Ecosystem', eco.version || '—'], ['Revision', eco.revision || '—'], ['Linux', (S.sysinfo && S.sysinfo.linux) || '—']]);
    }

    // the clipboard API needs a secure context: plain http on the LAN is not one
    function copyText(text) {
        if (navigator.clipboard && window.isSecureContext) {
            return navigator.clipboard.writeText(text);
        }
        const ta = document.createElement('textarea');
        ta.value = text;
        ta.setAttribute('readonly', '');
        ta.style.position = 'fixed';
        ta.style.opacity = '0';
        document.body.appendChild(ta);
        ta.select();
        try { document.execCommand('copy'); } finally { ta.remove(); }
        return Promise.resolve();
    }

    function copyAll() {
        const d = deviceData();
        const block = (title, list) => title + '\n' + list.map(([k, v]) => '  ' + k + ': ' + v).join('\n');
        const text = [block('Board', d.board), block('Software', d.soft), block('Memory and storage', d.mem), block('FPGA images', d.fpga)].join('\n\n');
        copyText(text).then(() => flash($('copied'), 2000));
    }

    // the bug report: the module builds it (RP_COMMAND 1), its progress comes over the socket of
    // the module (port 9099, /wsm over https), then /get_bug_report gives the archive
    function bugReport() {
        const url = https() ? 'wss://' + location.host + '/wsm' : 'ws://' + location.hostname + ':9099';
        const back = document.createElement('div');
        back.className = 'dlg-back';
        back.innerHTML = '<div class="dlg" role="dialog" aria-modal="true" aria-labelledby="br-t">' +
            '<div class="dlg-h"><h4 id="br-t" class="card-t">Bug report</h4></div>' +
            '<p class="dlg-b">Collecting the logs and the state of the board…</p>' +
            '<div class="bar"><i></i></div>' +
            '<div class="dlg-f"><button type="button" class="btn btn-g">Cancel</button></div></div>';
        document.body.appendChild(back);
        const bar = back.querySelector('.bar i');
        const ws = new WebSocket(url);
        ws.binaryType = 'arraybuffer';
        const close = () => { ws.onclose = null; ws.close(); back.remove(); };
        back.querySelector('.btn').addEventListener('click', () => { CLIENT.set('RP_COMMAND', 2); close(); });
        ws.onopen = () => CLIENT.set('RP_COMMAND', 1);
        ws.onerror = () => { back.querySelector('.dlg-b').textContent = 'The bug report could not be collected.'; };
        ws.onmessage = async (ev) => {
            let m;
            try {
                m = JSON.parse(new TextDecoder().decode(new Uint8Array(ev.data)));
            } catch (e) {
                return;
            }
            if (m.build_log_step) {
                bar.style.width = m.build_log_step.value + '%';
            }
            if (m.build_log_done) {
                const ok = m.build_log_done.value == 0;
                close();
                if (!ok) return;
                await new Promise((r) => setTimeout(r, 1000));
                const blob = await fetch('/get_bug_report', { method: 'POST' }).then((r) => r.blob());
                const name = new Date().toJSON().slice(0, 22) + '.zip';
                const a = document.createElement('a');
                a.href = URL.createObjectURL(blob);
                a.download = name;
                a.click();
                setTimeout(() => URL.revokeObjectURL(a.href), 60000);
            }
        };
    }
    const https = () => location.protocol === 'https:';

    // ---------------------------------------------------------------------------------------
    // Sensors
    // One sample a second from the last value the module sent: the module sends a value only when
    // it changes and its messages come unevenly, so the samples are taken on a clock of their own.
    const SAMPLE_MS = 1000;
    function sampleHistory() {
        const now = Date.now();
        const t = num('RP_SYSTEM_TEMPERATURE');
        if (Number.isFinite(t) && t > -100) S.temp.push([now, t]);
        const c = num('RP_SYSTEM_CPU_LOAD');
        if (Number.isFinite(c)) S.cpu.push([now, c]);
        for (const arr of [S.temp, S.cpu]) {
            while (arr.length && arr[0][0] < now - HISTORY_MS) arr.shift();
        }
    }

    // a centred moving average over n samples (the noise of the temperature sensor)
    function smooth(arr, n) {
        const h = Math.floor(n / 2);
        return arr.map(([t], i) => {
            const part = arr.slice(Math.max(0, i - h), i + h + 1);
            return [t, part.reduce((a, p) => a + p[1], 0) / part.length];
        });
    }

    // An SVG path through the samples: a monotone cubic (Fritsch-Carlson), smooth but never above
    // or below the samples between two of them, so it shows no peaks that were not measured.
    function sparkPath(arr, lo, hi) {
        const now = Date.now();
        const P = arr.map(([t, v]) => [300 - (now - t) / HISTORY_MS * 300, 58 - (Math.min(hi, Math.max(lo, v)) - lo) / (hi - lo) * 56]);
        if (P.length < 2) return '';
        const n = P.length, d = [], m = [];
        for (let i = 0; i < n - 1; i++) d.push((P[i + 1][1] - P[i][1]) / (P[i + 1][0] - P[i][0] || 1));
        m[0] = d[0];
        m[n - 1] = d[n - 2];
        for (let i = 1; i < n - 1; i++) m[i] = d[i - 1] * d[i] <= 0 ? 0 : (d[i - 1] + d[i]) / 2;
        for (let i = 0; i < n - 1; i++) {
            if (d[i] === 0) { m[i] = m[i + 1] = 0; continue; }
            const a = m[i] / d[i], b = m[i + 1] / d[i], r = a * a + b * b;
            if (r > 9) { const k = 3 / Math.sqrt(r); m[i] = k * a * d[i]; m[i + 1] = k * b * d[i]; }
        }
        const f = (x) => x.toFixed(1);
        let path = 'M' + f(P[0][0]) + ',' + f(P[0][1]);
        for (let i = 0; i < n - 1; i++) {
            const dx = (P[i + 1][0] - P[i][0]) / 3;
            path += 'C' + f(P[i][0] + dx) + ',' + f(P[i][1] + m[i] * dx) + ' ' + f(P[i + 1][0] - dx) + ',' + f(P[i + 1][1] - m[i + 1] * dx) + ' ' + f(P[i + 1][0]) + ',' + f(P[i + 1][1]);
        }
        return path;
    }

    function renderSensors() {
        const t = num('RP_SYSTEM_TEMPERATURE');
        if (Number.isFinite(t) && t > -100) {
            $('s-temp').textContent = t.toFixed(1) + ' °C';
            const g = $('g-temp');
            g.style.width = Math.min(100, Math.max(0, t / TEMP_SCALE * 100)) + '%';
            g.classList.toggle('hot', t >= TEMP_LIMIT);
            const vs = S.temp.map((p) => p[1]);
            const lo = Math.floor(Math.min(...vs) - 1), hi = Math.ceil(Math.max(...vs) + 1);
            $('p-temp').setAttribute('d', sparkPath(smooth(S.temp, 3), lo, hi));
            $('s-temp-mm').textContent = 'min ' + Math.min(...vs).toFixed(1) + ' · max ' + Math.max(...vs).toFixed(1) + ' °C';
        }
        const c = num('RP_SYSTEM_CPU_LOAD');
        if (Number.isFinite(c)) {
            $('s-cpu').textContent = Math.round(c) + ' %';
            $('g-cpu').style.width = Math.min(100, Math.max(0, c)) + '%';
            const vs = S.cpu.map((p) => p[1]);
            const hi = Math.max(10, Math.ceil(Math.max(...vs) / 10) * 10);
            $('p-cpu').setAttribute('d', sparkPath(S.cpu, 0, hi));
            $('s-cpu-mm').textContent = 'max ' + Math.round(Math.max(...vs)) + ' %';
        }

        const ddrNom = val('RP_SYSTEM_VCC_DDR_NOMINAL', '');
        const rails = [
            ['VCC PINT', 'processor logic', 1.0, 'RP_SYSTEM_VCC_PINT'],
            ['VCC PAUX', 'processor auxiliary', 1.8, 'RP_SYSTEM_VCC_PAUX'],
            ['VCC BRAM', 'block RAM', 1.0, 'RP_SYSTEM_VCC_BRAM'],
            ['VCC INT', 'FPGA logic', 1.0, 'RP_SYSTEM_VCC_INT'],
            ['VCC AUX', 'FPGA auxiliary', 1.8, 'RP_SYSTEM_VCC_AUX'],
            ['VCC DDR', 'DDR memory', ddrNom !== '' ? parseFloat('1.' + ddrNom) : NaN, 'RP_SYSTEM_VCC_DDR']
        ];
        $('t-volt').innerHTML = rails.map(([name, what, nom, p]) => {
            const v = num(p);
            const known = Number.isFinite(v) && Number.isFinite(nom) && v > 0;
            let dev = known ? (v - nom) / nom * 100 : NaN;
            if (Math.abs(dev) < 0.05) dev = 0;  // no "-0.0 %"
            const inRange = known && Math.abs(dev) <= TOLERANCE;
            const pos = known ? Math.min(100, Math.max(0, 50 + dev / TOLERANCE * 50)) : 50;
            return '<tr><td><span class="n">' + name + '</span><div class="d">' + what + '</div></td>' +
                '<td class="m">' + (Number.isFinite(nom) ? nom.toFixed(2) + ' V' : '—') + '</td>' +
                '<td class="m n">' + (known ? v.toFixed(3) + ' V' : '—') + '</td>' +
                '<td class="m">' + (known ? (dev >= 0 ? '+' : '') + dev.toFixed(1) + ' %' : '—') + '</td>' +
                '<td><div class="tol"><span class="mid"></span>' + (known ? '<span class="mk" style="left: ' + pos.toFixed(1) + '%"></span>' : '') + '</div></td>' +
                '<td class="' + (inRange ? 'ok' : 'warn') + '" style="font-size: 12px; white-space: nowrap">' + (known ? (inRange ? 'in range' : 'out of range') : '') + '</td></tr>';
        }).join('');
    }

    // ---------------------------------------------------------------------------------------
    // System
    const RATES = [
        ['RP_ADC_BASE_RATE', 'rate-adc', 'tag-adc', 'reset-adc'],
        ['RP_DAC_BASE_RATE', 'rate-dac', 'tag-dac', 'reset-dac']
    ];

    function renderRates() {
        if (S.sysinfo) $('rate-model').textContent = 'Defaults of ' + (S.sysinfo.name || S.sysinfo.model);
        for (const [p, inp, tag, reset] of RATES) {
            const v = num(p), d = num(p + '_DEFAULT');
            if (!S.rateEdit[p] && Number.isFinite(v)) $(inp).value = fmtMHz(v);
            const known = Number.isFinite(v) && Number.isFinite(d) && d > 0;
            const changed = known && v !== d;
            const t = $(tag);
            t.textContent = known ? (changed ? 'changed' : 'default') : '';
            t.className = changed ? 'tag tag-w' : 'tag';
            t.classList.toggle('hidden', !known);
            const r = $(reset);
            r.textContent = 'Reset to ' + fmtMHz(d) + ' MHz';
            r.classList.toggle('hidden', !changed);
        }
    }

    function setRate(p, inp) {
        S.rateEdit[p] = false;
        const mhz = parseFloat(String($(inp).value).replace(',', '.'));
        if (!Number.isFinite(mhz) || mhz <= 0 || mhz > 2000) {
            renderRates();
            $(inp).value = fmtMHz(num(p));
            return;
        }
        CLIENT.set(p, Math.round(mhz * 1e6));
    }

    // 32 and 64 MB on every board, 128 and 256 MB with 512 MB of RAM, 512 and 768 MB with 1 GB
    const DMA_SIZES = [[32, 0], [64, 0], [128, 512], [256, 512], [512, 1024], [768, 1024]];

    function renderDma() {
        const cur = num('RP_SYSTEM_DMA_RAM');
        const ddr = num('RP_SYSTEM_DDR_MAX');
        const next = S.status && S.status.dma_next ? S.status.dma_next : cur;
        const curMB = Math.round(cur / MB), nextMB = Math.round(next / MB);
        const pending = Number.isFinite(cur) && Number.isFinite(next) && curMB !== nextMB;

        $('dma-seg').innerHTML = DMA_SIZES.map(([mb, need]) => {
            const off = Number.isFinite(ddr) && ddr < need;
            const on = mb === nextMB;
            return '<button type="button" role="radio" aria-checked="' + on + '" data-dma="' + mb + '"' + (on ? ' class="on"' : '') + (off ? ' disabled' : '') + '>' + mb + ' MB</button>';
        }).join('');
        $('dma-head').innerHTML = Number.isFinite(cur) ? 'Now ' + curMB + ' MB' + (pending ? '<span class="warn"> · ' + nextMB + ' MB after a reboot</span>' : '') : '';

        const ram = Number.isFinite(ddr) ? ddr * MB : NaN;
        const total = num('RP_SYSTEM_TOTAL_RAM'), free = num('RP_SYSTEM_FREE_RAM');
        const used = total - free;
        if (Number.isFinite(ram) && ram > 0) {
            $('m-dma').style.width = (next / ram * 100).toFixed(1) + '%';
            $('m-used').style.width = Number.isFinite(used) ? (used / ram * 100).toFixed(1) + '%' : '0';
            $('l-dma').textContent = 'DMA ' + nextMB + ' MB';
            $('l-used').textContent = 'Linux used ' + fmtBytes(used);
            $('l-free').textContent = 'free ' + fmtBytes(Math.max(0, ram - next - used));
            $('l-ram').textContent = 'RAM ' + fmtBytes(ram);
        }
        const missing = DMA_SIZES.filter(([, need]) => Number.isFinite(ddr) && ddr < need).map(([mb]) => mb + ' MB');
        $('dma-note').textContent = missing.length ? missing.join(', ') + ' need' + (missing.length === 1 ? 's' : '') + ' a board with more RAM.' : '';
        $('dma-note').classList.toggle('hidden', !missing.length);

        $('nav-dot').classList.toggle('hidden', !pending);
        $('reboot-banner').classList.toggle('hidden', !pending);
        $('reboot-text').textContent = 'Reserved memory for DMA is changed to ' + nextMB + ' MB. It takes effect after a reboot.';
    }

    // A modal wait with a spinner for an action that takes seconds; it cannot be closed while the
    // action runs. fail(text) turns it into an error with a Close button.
    function busyDialog(title, text) {
        const back = document.createElement('div');
        back.className = 'dlg-back';
        back.innerHTML = '<div class="dlg" role="alertdialog" aria-modal="true" aria-live="polite" aria-labelledby="busy-t">' +
            '<div class="dlg-h"><h4 id="busy-t" class="card-t"></h4></div>' +
            '<div class="dlg-busy"><span class="spin" aria-hidden="true"></span><p></p></div></div>';
        back.querySelector('.card-t').textContent = title;
        back.querySelector('p').textContent = text;
        document.body.appendChild(back);
        return {
            done: () => back.remove(),
            fail: (msg) => {
                const b = back.querySelector('.dlg-busy');
                b.querySelector('.spin').remove();
                const p = b.querySelector('p');
                p.textContent = msg;
                p.classList.add('bad');
                const f = document.createElement('div');
                f.className = 'dlg-f';
                f.innerHTML = '<button type="button" class="btn btn-g">Close</button>';
                f.querySelector('button').addEventListener('click', () => back.remove());
                back.querySelector('.dlg').appendChild(f);
                f.querySelector('button').focus();
            }
        };
    }

    // the device tree on the SD card is rewritten and compiled: a few seconds
    async function setDma(mb) {
        const busy = busyDialog('Reserved memory for DMA', 'Writing ' + mb + ' MB to the device tree on the SD card…');
        let error = '';
        try {
            const r = await fetch('/resizeDMA?new_size=0x' + (mb * MB).toString(16), { cache: 'no-store' });
            const text = (await r.text()).trim();
            if (!r.ok || /ERROR/.test(text)) error = text || 'The device tree was not changed (HTTP ' + r.status + ').';
        } catch (e) {
            error = 'No answer from the board.';
        }
        await loadStatus();
        if (error) busy.fail(error); else busy.done();
    }

    function renderBoard() {
        if (!S.status) return;
        $('sw-led').checked = Number(S.status.led) !== 0;
        $('sw-fsck').checked = !!S.status.fsck;
    }

    // ---------------------------------------------------------------------------------------
    // Security
    function pwState() {
        const a = $('pw1').value, b = $('pw2').value;
        const mismatch = b.length > 0 && a !== b;
        $('pw-mismatch').classList.toggle('hidden', !mismatch);
        $('pw2').classList.toggle('badb', mismatch);
        $('pw-btn').disabled = a.length === 0 || a !== b;
    }

    async function changePassword(e) {
        e.preventDefault();
        const msg = $('pw-msg');
        const a = $('pw1').value;
        if (!a || a !== $('pw2').value) return;
        $('pw-btn').disabled = true;
        msg.className = 'msg';
        msg.textContent = 'Changing…';
        try {
            const r = await postJSON('/set_root_password', { password: a });
            const text = (await r.text()).trim();
            if (!r.ok) throw new Error(text || 'The password was not changed');
            $('pw1').value = '';
            $('pw2').value = '';
            msg.className = 'msg ok';
            msg.textContent = 'The password is changed. Use it for the next login.';
        } catch (err) {
            msg.className = 'msg bad';
            msg.textContent = err.message;
        }
        pwState();
    }

    // ---------------------------------------------------------------------------------------
    // Maintenance
    function renderNet() {
        const st = S.status;
        if (!st) return;
        const line = (i) => {
            if (!i || !i.present) return ['no adapter', 'warn'];
            const up = i.state === 'up';
            return [(up ? 'up' : i.state || 'down') + (i.addr ? ' · ' + i.addr : '') + (i.addr ? (i.dhcp ? ' · DHCP' : ' · static') : ''), up ? '' : 'warn'];
        };
        const e = line(st.eth0), w = line(st.wlan0);
        const gw = (st.eth0 && st.eth0.gateway) || (st.wlan0 && st.wlan0.gateway) || '—';
        $('t-net').innerHTML = rows([['eth0', e[0], false, e[1]], ['Gateway', gw], ['wlan0', w[0], false, w[1]]]);
    }

    function renderStored() {
        const list = S.stored;
        $('t-stored').innerHTML = list.map((a) =>
            '<tr><td><span class="n">' + esc(a.name) + '</span><div class="d mono">' + esc(a.dir) + '</div></td>' +
            '<td class="m">' + (a.mtime ? fmtDate(a.mtime) : '—') + '</td>' +
            '<td><button class="btn btn-g btn-sm" type="button" data-remove="' + esc(a.dir) + '">Remove</button></td></tr>').join('');
        $('t-stored-wrap').classList.toggle('hidden', !list.length);
        $('stored-empty').classList.toggle('hidden', list.length > 0);
        $('rm-all').disabled = !list.length;
    }

    async function loadStored() {
        try {
            S.stored = await getJSON('/settings_stored');
        } catch (e) {
            S.stored = [];
        }
        renderStored();
    }

    async function removeStored(dir) {
        await postJSON('/settings_remove', dir ? { dir: dir } : { all: true });
        await loadStored();
    }

    // reboot and power off: the board goes away, the page waits for it (reboot) or says it is off
    function powerScreen(title, text, spin) {
        const d = document.createElement('div');
        d.className = 'power';
        d.innerHTML = '<div>' + (spin ? '<span class="spin" aria-hidden="true"></span>' : '') + '<b></b><p></p></div>';
        d.querySelector('b').textContent = title;
        d.querySelector('p').textContent = text;
        document.body.appendChild(d);
        return d;
    }

    async function reboot() {
        if (!(await CLIENT.ask({ title: 'Reboot', text: 'The board reboots. The running application stops.', ok: 'Reboot', danger: true }))) return;
        CLIENT.leave();
        powerScreen('Rebooting…', 'The page reconnects when the board is back.', true);
        fetch('/reboot', { cache: 'no-store' }).catch(() => {});
        // wait until nginx is gone, then until it answers again
        let wentDown = false;
        const t0 = Date.now();
        const poll = async () => {
            try {
                const r = await fetch('/check_nginx_live', { cache: 'no-store' });
                if (r.ok && (wentDown || Date.now() - t0 > 120000)) {
                    location.reload();
                    return;
                }
            } catch (e) {
                wentDown = true;
            }
            setTimeout(poll, 2000);
        };
        setTimeout(poll, 5000);
    }

    async function poweroff() {
        if (!(await CLIENT.ask({ title: 'Power off', text: 'The board shuts down. Switch the power off and on to start it again.', ok: 'Power off', danger: true }))) return;
        CLIENT.leave();
        powerScreen('The board is off', 'It is safe to pull the power now. To start the board again, switch the power off and on.', false);
        fetch('/poweroff', { cache: 'no-store' }).catch(() => {});
    }

    // ---------------------------------------------------------------------------------------
    // data
    async function loadSysinfo() {
        try {
            const r = await fetch('/get_sysinfo', { cache: 'no-store' });
            S.sysinfo = JSON.parse(await r.text());
        } catch (e) {
            setTimeout(loadSysinfo, 2000);
            return;
        }
        renderHeader();
        renderDevice();
        renderRates();
    }

    async function loadStatus() {
        try {
            S.status = await getJSON('/settings_status');
            S.uptimeAt = Date.now();
        } catch (e) {
            return;
        }
        renderDevice();
        renderDma();
        renderBoard();
        renderNet();
        renderFooter();
    }

    CLIENT.onParameters((names) => {
        const has = (p) => names.some((n) => n.startsWith(p));
        if (has('RP_CALIB') || has('RP_SYSTEM_CLOCK')) renderHeader();
        if (has('RP_SYSTEM_TEMPERATURE') || has('RP_SYSTEM_CPU') || has('RP_SYSTEM_VCC')) {
            renderFooter();
            renderSensors();
        }
        if (has('RP_SYSTEM_') || has('RP_CALIB')) renderDevice();
        if (has('RP_SYSTEM_DMA') || has('RP_SYSTEM_DDR') || has('RP_SYSTEM_TOTAL_RAM') || has('RP_SYSTEM_FREE_RAM')) renderDma();
        if (has('RP_ADC_BASE') || has('RP_DAC_BASE')) renderRates();
    });

    // ---------------------------------------------------------------------------------------
    window.addEventListener('load', () => {
        document.getElementById('nav').addEventListener('click', (e) => {
            const b = e.target.closest('[data-go]');
            if (b) go(b.dataset.go);
        });
        window.addEventListener('hashchange', () => go(location.hash.slice(1)));
        go(location.hash.slice(1) || 'device');

        document.body.addEventListener('click', (e) => {
            const t = e.target.closest('[data-act],[data-copy],[data-reset],[data-dma],[data-remove]');
            if (!t || t.disabled) return;
            if (t.dataset.copy !== undefined) copyText(t.dataset.copy).then(() => flash($('copied'), 2000));
            else if (t.dataset.reset) CLIENT.set(t.dataset.reset, num(t.dataset.reset + '_DEFAULT'));
            else if (t.dataset.dma) setDma(Number(t.dataset.dma));
            else if (t.dataset.remove) removeStored(t.dataset.remove);
            else if (t.dataset.act === 'copy-all') copyAll();
            else if (t.dataset.act === 'bug-report') bugReport();
            else if (t.dataset.act === 'reboot') reboot();
            else if (t.dataset.act === 'poweroff') poweroff();
            else if (t.dataset.act === 'remove-all') {
                CLIENT.ask({ title: 'Remove all stored settings', text: 'All applications lose their saved settings and start with the defaults. The calibration and the network settings stay.', ok: 'Remove all', danger: true })
                    .then((ok) => { if (ok) removeStored(null); });
            }
        });

        for (const [p, inp] of RATES) {
            const el = $(inp);
            el.addEventListener('focus', () => { S.rateEdit[p] = true; });
            el.addEventListener('change', () => setRate(p, inp));
            el.addEventListener('keydown', (e) => { if (e.key === 'Enter') el.blur(); });
            el.addEventListener('blur', () => { S.rateEdit[p] = false; renderRates(); });
        }

        $('sw-led').addEventListener('change', async (e) => {
            await fetch('/set_led_status?enable=' + (e.target.checked ? 'true' : 'false'), { cache: 'no-store' });
            flash($('led-saved'));
        });
        $('sw-fsck').addEventListener('change', async (e) => {
            await fetch('/set_fsck?enable=' + (e.target.checked ? 'true' : 'false'), { cache: 'no-store' });
            flash($('fsck-saved'));
        });

        $('pw1').addEventListener('input', pwState);
        $('pw2').addEventListener('input', pwState);
        $('pw-form').addEventListener('submit', changePassword);

        loadSysinfo();
        loadStatus();
        setInterval(loadStatus, 30000);
        setInterval(renderFooter, 30000);
        // the graphs move on between the updates of the values
        setInterval(() => { sampleHistory(); if (S.section === 'sensors') renderSensors(); }, SAMPLE_MS);
    });
})();
