/*
 * Red Pitaya stream server application: the connection to the board.
 *
 * The application runs as a service (redpitaya_streaming.service): streaming-server with the
 * backend of this page (rp_sdk). The page starts it when it is not running, talks to it through
 * nginx (/streaming_ws) and leaves it running when it closes: the streams go on. The Exit button
 * stops it.
 *
 * CLIENT: the parameters of the application module (websocket /streaming_ws, rp_sdk). The globals
 * CLIENT.state.socket_opened, CLIENT.params.orig, CLIENT.parametersCache and
 * CLIENT.sendParameters() keep the names of the old page (the regression test uses them).
 * RP_CLIENT: the liveness socket (ping every second), the loader and the "connection lost" /
 * "used by another user" overlays of ../assets.
 */
(function () {
    'use strict';

    const https = location.protocol === 'https:';

    // ---------------------------------------------------------------------------------------
    // the parameters
    const CLIENT = {
        config: {
            service_url: '/streaming_manager_service',
            socket_url: (https ? 'wss://' : 'ws://') + location.host + '/streaming_ws'
        },
        state: { socket_opened: false },
        params: { orig: {} },
        parametersCache: {},
        ws: null,
        retries: 0,
        listeners: [],
        openListeners: []
    };

    // f(changed) gets the names of the parameters that came in one message
    CLIENT.onParameters = (f) => CLIENT.listeners.push(f);
    CLIENT.onOpen = (f) => CLIENT.openListeners.push(f);

    CLIENT.value = (name, def) => {
        const p = CLIENT.params.orig[name];
        return p === undefined ? def : p.value;
    };

    CLIENT.set = (name, value) => {
        CLIENT.parametersCache[name] = { value: value };
        return CLIENT.sendParameters();
    };

    CLIENT.sendParameters = () => {
        if (!CLIENT.state.socket_opened) {
            return false;
        }
        CLIENT.ws.send(JSON.stringify({ parameters: CLIENT.parametersCache }));
        CLIENT.parametersCache = {};
        return true;
    };

    CLIENT.command = (cmd) => {
        if (CLIENT.state.socket_opened) {
            CLIENT.ws.send(JSON.stringify({ parameters: { in_command: { value: cmd } } }));
        }
    };

    CLIENT.requestParameters = () => CLIENT.command('send_all_params');

    // a frame: 4 bytes of a prefix, then JSON; EZIA/EZIB gzip, NZIA/NZIB plain
    const decoder = new TextDecoder();
    async function decode(buf) {
        const bytes = new Uint8Array(buf);
        const tag = String.fromCharCode(bytes[0], bytes[1], bytes[2], bytes[3]);
        let body = bytes.subarray(4);
        if (tag === 'EZIA' || tag === 'EZIB') {
            if (typeof DecompressionStream === 'undefined') {
                return null;
            }
            const stream = new Blob([body]).stream().pipeThrough(new DecompressionStream('gzip'));
            body = new Uint8Array(await new Response(stream).arrayBuffer());
        } else if (tag !== 'NZIA' && tag !== 'NZIB') {
            body = bytes;  // no prefix
        }
        const text = decoder.decode(body);
        return text ? JSON.parse(text) : null;
    }

    // messages are decoded one after the other, so a later value never loses to an earlier one
    let chain = Promise.resolve();
    function onMessage(ev) {
        chain = chain.then(() => decode(ev.data)).then((msg) => {
            if (!msg || !msg.parameters) {
                return;
            }
            const names = Object.keys(msg.parameters);
            for (const n of names) {
                CLIENT.params.orig[n] = msg.parameters[n];
            }
            if (names.length) {
                for (const f of CLIENT.listeners) {
                    f(names);
                }
            }
        }).catch((e) => console.error('parameters:', e));
    }

    CLIENT.connectWebSocket = () => {
        const ws = new WebSocket(CLIENT.config.socket_url);
        ws.binaryType = 'arraybuffer';
        CLIENT.ws = ws;
        ws.onopen = () => {
            CLIENT.state.socket_opened = true;
            CLIENT.retries = 0;
            CLIENT.parametersCache = {};
            // plain JSON when the browser cannot unzip it
            if (typeof DecompressionStream === 'undefined') {
                CLIENT.command('disable_zip_params');
            }
            CLIENT.requestParameters();
            for (const f of CLIENT.openListeners) {
                f();
            }
        };
        ws.onclose = () => {
            // a service that has just started opens its socket a little later: try again first
            if (!CLIENT.state.socket_opened && CLIENT.retries < 15) {
                CLIENT.retries++;
                setTimeout(CLIENT.connectWebSocket, 500);
                return;
            }
            CLIENT.state.socket_opened = false;
            setTimeout(RP_CLIENT.reloadPage, 2000);
        };
        ws.onerror = (e) => console.error('websocket:', e);
        ws.onmessage = onMessage;
    };

    // A question in the style of the page (as the dialogs of calib_app: a title with a close button,
    // the text, two buttons of one width) instead of the browser's confirm(). Resolves true for OK;
    // Escape, the close button and a click beside the dialog are Cancel, Enter is OK.
    CLIENT.ask = ({ title, text, ok = 'OK', cancel = 'Cancel', danger = false }) => new Promise((resolve) => {
        const back = document.createElement('div');
        back.className = 'dlg-back';
        back.innerHTML =
            '<div class="dlg" role="dialog" aria-modal="true" aria-labelledby="dlg-t">' +
            '<div class="dlg-h"><h4 id="dlg-t" class="card-t"></h4><button type="button" class="dlg-x" aria-label="Close">&times;</button></div>' +
            '<p class="dlg-b"></p>' +
            '<div class="dlg-f"><button type="button" class="btn ' + (danger ? 'btn-d' : 'btn-p') + '" data-r="1"></button>' +
            '<button type="button" class="btn" data-r="0"></button></div></div>';
        back.querySelector('.card-t').textContent = title;
        back.querySelector('.dlg-b').textContent = text;
        back.querySelector('[data-r="1"]').textContent = ok;
        back.querySelector('[data-r="0"]').textContent = cancel;
        const before = document.activeElement;
        const done = (r) => {
            document.removeEventListener('keydown', key, true);
            back.remove();
            if (before && before.focus) before.focus();
            resolve(r);
        };
        const key = (e) => {
            if (e.key === 'Escape') { e.preventDefault(); done(false); }
            if (e.key === 'Enter') { e.preventDefault(); done(true); }
        };
        back.addEventListener('click', (e) => {
            if (e.target === back || e.target.classList.contains('dlg-x')) done(false);
            else if (e.target.dataset.r) done(e.target.dataset.r === '1');
        });
        document.addEventListener('keydown', key, true);
        document.body.appendChild(back);
        back.querySelector('[data-r="1"]').focus();
    });

    // the streams that run (their status 1): the Exit button asks before it stops them
    CLIENT.running = () => ['SS_STATUS', 'SS_DAC_STATUS', 'SS_GPIO_STATUS', 'SS_GPIO_OUT_STATUS']
        .filter((n) => Number(CLIENT.value(n, 0)) === 1).length;

    // both sockets, without their reconnects
    const closeSockets = () => {
        for (const w of [CLIENT.ws, RP_CLIENT.ws]) {
            if (w) {
                w.onclose = null;
                w.close();
            }
        }
    };

    const service = (cmd) => fetch(CLIENT.config.service_url + (cmd ? '?cmd=' + cmd : ''),
        { method: cmd ? 'POST' : 'GET', cache: 'no-store' }).then((r) => r.json());

    // the overlay of the loader with a message of its own (no service in this OS, ...)
    const fail = (text) => {
        const d = document.getElementById('loader-wrapper');
        if (d) d.innerHTML = '<div class="service-msg">' + text + '</div>';
    };

    // The service: running - connect; stopped - stop the bazaar application (it holds the FPGA and
    // the port of the liveness socket), start the service and connect once its socket answers.
    CLIENT.startApp = async () => {
        if (CLIENT.leaving) {
            return;
        }
        let st;
        try {
            st = await service();
        } catch (e) {
            setTimeout(CLIENT.startApp, 2000);
            return;
        }
        if (!st.unit) {
            fail('This Red Pitaya OS has no streaming service. Update the OS to 3.04 or later.');
            return;
        }
        if (st.active !== 'active' && st.active !== 'activating') {
            if (st.scpi === 'active' && !(await CLIENT.ask({
                title: 'SCPI server',
                text: 'The SCPI server is running. The streaming server takes the FPGA and stops it.',
                ok: 'Stop SCPI and go on', cancel: 'Back to the applications'
            }))) {
                location.href = '/';
                return;
            }
            try {
                await fetch('/bazaar?stop=', { cache: 'no-store' });
                st = await service('start');
            } catch (e) {
                setTimeout(CLIENT.startApp, 2000);
                return;
            }
            if (st.active !== 'active' && st.active !== 'activating') {
                fail('The streaming server does not start (' + st.active + '). See: journalctl -u redpitaya_streaming');
                return;
            }
        }
        CLIENT.connectWebSocket();
        RP_CLIENT.connectWebSocket();
    };

    // Exit: stop the service and go to the applications (the root page shows this page while the
    // service runs)
    CLIENT.exitApp = async () => {
        const n = CLIENT.running();
        const go = await CLIENT.ask({
            title: 'Exit',
            text: n ? n + (n === 1 ? ' stream runs' : ' streams run') + ' on the board: the exit stops the streaming server and ' + (n === 1 ? 'it' : 'them') + '. To leave ' + (n === 1 ? 'it' : 'them') + ' running, close the tab instead.'
                : 'Stop the streaming server and go to the applications?',
            ok: 'Exit', cancel: 'Cancel', danger: n > 0
        });
        if (!go) {
            return;
        }
        // no reconnect, no reload, no start from now on
        CLIENT.leaving = true;
        RP_CLIENT.stopChecks();
        closeSockets();
        const note = document.createElement('div');
        note.className = 'dlg-back';
        note.innerHTML = '<div class="dlg" role="alertdialog" aria-live="polite"><div class="dlg-h"><h4 class="card-t">Exit</h4></div>' +
            '<p class="dlg-b">Stopping the streaming server' + (n ? ': the recordings are written to the end' : '') + '...</p></div>';
        document.body.appendChild(note);
        // the stop returns at once; the server may need a while to close its files
        try {
            let st = await service('stop');
            const end = Date.now() + 45000;
            while ((st.active === 'active' || st.active === 'deactivating') && Date.now() < end) {
                await new Promise((r) => setTimeout(r, 500));
                st = await service();
            }
        } catch (e) {
            // the root page shows this one again if the service still runs
        }
        location.href = '/';
    };

    // ---------------------------------------------------------------------------------------
    // liveness: the classes of <body> drive the overlays of ../assets (inverted: a class hides one)
    const RP_CLIENT = {
        client_id: (crypto.randomUUID ? crypto.randomUUID() : String(Math.random()).slice(2)),
        // through nginx over https and when the web login is on (its ports are closed then)
        socket_url: (https || document.cookie.indexOf('rp_ws_proxy=1') >= 0) ? (https ? 'wss://' : 'ws://') + location.host + '/wsc' : 'ws://' + location.hostname + ':9091',
        ws: null,
        ping: -1,
        lastPing: -1,
        checkTimer: null,
        liveTimer: null
    };

    const body = () => document.body.classList;
    RP_CLIENT.setNormalMode = () => { body().add('loaded', 'connection_lost', 'user_lost'); };
    RP_CLIENT.setUserLost = () => { body().add('loaded', 'connection_lost'); body().remove('user_lost'); };
    RP_CLIENT.setConnectionLost = () => { body().add('loaded', 'user_lost'); body().remove('connection_lost'); };

    RP_CLIENT.reloadPage = () => {
        if (CLIENT.leaving) {
            return;
        }
        const ctl = new AbortController();
        const t = setTimeout(() => ctl.abort(), 10000);
        fetch('/get_client_id', { cache: 'no-store', signal: ctl.signal })
            .then((r) => r.text())
            .then((id) => {
                clearTimeout(t);
                if (id.trim() === RP_CLIENT.client_id) {
                    location.reload();
                } else {
                    RP_CLIENT.stopChecks();
                    RP_CLIENT.setUserLost();
                }
            })
            .catch(() => RP_CLIENT.setConnectionLost());
    };

    RP_CLIENT.stopChecks = () => {
        clearInterval(RP_CLIENT.checkTimer);
        clearInterval(RP_CLIENT.liveTimer);
        RP_CLIENT.checkTimer = RP_CLIENT.liveTimer = null;
    };

    // no ping for 5 s: the connection is lost; once nginx answers again the page reloads
    RP_CLIENT.checkStatus = () => {
        if (RP_CLIENT.ping !== RP_CLIENT.lastPing) {
            RP_CLIENT.lastPing = RP_CLIENT.ping;
            return;
        }
        RP_CLIENT.setConnectionLost();
        if (!RP_CLIENT.liveTimer) {
            RP_CLIENT.liveTimer = setInterval(() => {
                fetch('/check_nginx_live', { cache: 'no-store' })
                    .then((r) => { if (r.ok) { RP_CLIENT.stopChecks(); RP_CLIENT.reloadPage(); } })
                    .catch(() => {});
            }, 2000);
        }
    };

    RP_CLIENT.connectWebSocket = () => {
        const ws = new WebSocket(RP_CLIENT.socket_url);
        ws.binaryType = 'arraybuffer';
        RP_CLIENT.ws = ws;
        ws.onopen = () => {
            RP_CLIENT.setNormalMode();
            RP_CLIENT.stopChecks();
            RP_CLIENT.checkTimer = setInterval(RP_CLIENT.checkStatus, 5000);
        };
        ws.onclose = () => setTimeout(RP_CLIENT.reloadPage, 2000);
        ws.onmessage = (ev) => {
            try {
                const m = JSON.parse(decoder.decode(new Uint8Array(ev.data)));
                if (m.ping) {
                    RP_CLIENT.ping = m.ping.value;
                }
            } catch (e) {
                // not a ping
            }
        };
    };

    // ---------------------------------------------------------------------------------------
    window.addEventListener('load', () => {
        const exit = document.getElementById('exit');
        if (exit) {
            exit.addEventListener('click', (e) => {
                e.preventDefault();
                CLIENT.exitApp();
            });
        }
        fetch('/set_client_id', { method: 'POST', body: RP_CLIENT.client_id }).catch(() => {});
        setTimeout(CLIENT.startApp, 500);
    });

    // the service goes on when the page closes: only the sockets close
    window.addEventListener('beforeunload', () => {
        RP_CLIENT.stopChecks();
        closeSockets();
    });

    window.CLIENT = CLIENT;
    window.RP_CLIENT = RP_CLIENT;
})();
