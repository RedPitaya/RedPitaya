/*
 * Red Pitaya settings page: the connection to the board.
 *
 * The page runs the main_menu application (bazaar module controllerhf.so) as the main page does:
 * its parameters (RP_SYSTEM_* of rp-web-api rp_system, the base rates, the calibration state) come
 * over the websocket /wss. The page stops the application when it closes; the main page starts it
 * again.
 *
 * CLIENT: the parameters of the application module (rp_sdk).
 * RP_CLIENT: the liveness socket (ping every second), the loader and the "connection lost" /
 * "used by another user" overlays of ../assets.
 * Taken from streaming_manager/js/net.js.
 */
(function () {
    'use strict';

    const https = location.protocol === 'https:';
    const APP = 'main_menu';

    // ---------------------------------------------------------------------------------------
    // the parameters
    const CLIENT = {
        config: {
            start_url: '/bazaar?start=' + APP + '?type=run',
            stop_url: '/bazaar?stop=' + APP,
            socket_url: (https ? 'wss://' : 'ws://') + location.host + '/wss'
        },
        state: { socket_opened: false },
        params: { orig: {} },
        parametersCache: {},
        ws: null,
        retries: 0,
        leaving: false,
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
            if (CLIENT.leaving) {
                return;
            }
            // a module that has just started opens its socket a little later: try again first
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

    CLIENT.startApp = async () => {
        if (CLIENT.leaving) {
            return;
        }
        try {
            const r = await fetch(CLIENT.config.start_url, { cache: 'no-store' });
            const st = await r.json();
            if (st.status !== 'OK') {
                throw new Error(st.reason || 'the application does not start');
            }
        } catch (e) {
            console.error('start:', e);
            setTimeout(CLIENT.startApp, 2000);
            return;
        }
        CLIENT.connectWebSocket();
        RP_CLIENT.connectWebSocket();
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
            '<div class="dlg-f"><button type="button" class="btn btn-g" data-r="0"></button>' +
            '<button type="button" class="btn ' + (danger ? 'btn-d' : 'btn-p') + '" data-r="1"></button></div></div>';
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
        back.querySelector('[data-r="0"]').focus();
    });

    // both sockets, without their reconnects
    CLIENT.closeSockets = () => {
        for (const w of [CLIENT.ws, RP_CLIENT.ws]) {
            if (w) {
                w.onclose = null;
                w.close();
            }
        }
    };

    // the board goes away (reboot, power off): no reconnect, no reload, no overlays
    CLIENT.leave = () => {
        CLIENT.leaving = true;
        RP_CLIENT.stopChecks();
        CLIENT.closeSockets();
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
        fetch('/set_client_id', { method: 'POST', body: RP_CLIENT.client_id }).catch(() => {});
        setTimeout(CLIENT.startApp, 500);
    });

    // the main page starts the application again
    window.addEventListener('beforeunload', () => {
        RP_CLIENT.stopChecks();
        CLIENT.closeSockets();
        if (!CLIENT.leaving) {
            fetch(CLIENT.config.stop_url, { cache: 'no-store', keepalive: true }).catch(() => {});
        }
    });

    window.CLIENT = CLIENT;
    window.RP_CLIENT = RP_CLIENT;
})();
