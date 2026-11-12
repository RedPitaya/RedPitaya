/*
 * Red Pitaya stream server application: the connection to the board.
 *
 * CLIENT: the parameters of the application module (websocket /wss, rp_sdk). The globals
 * CLIENT.state.socket_opened, CLIENT.params.orig, CLIENT.parametersCache and
 * CLIENT.sendParameters() keep the names of the old page (the regression test uses them).
 * RP_CLIENT: the liveness socket (ping every second), the loader and the "connection lost" /
 * "used by another user" overlays of ../assets.
 */
(function () {
    'use strict';

    const APP = 'streaming_manager';
    const https = location.protocol === 'https:';

    // ---------------------------------------------------------------------------------------
    // the parameters
    const CLIENT = {
        config: {
            start_app_url: '/bazaar?start=' + APP + '?type=run',
            stop_app_url: '/bazaar?stop=' + APP,
            socket_url: (https ? 'wss://' : 'ws://') + location.host + '/wss'
        },
        state: { socket_opened: false, app_started: false },
        params: { orig: {} },
        parametersCache: {},
        ws: null,
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
            CLIENT.state.socket_opened = false;
            setTimeout(RP_CLIENT.reloadPage, 2000);
        };
        ws.onerror = (e) => console.error('websocket:', e);
        ws.onmessage = onMessage;
    };

    CLIENT.startApp = () => {
        fetch(CLIENT.config.start_app_url, { cache: 'no-store' })
            .then((r) => r.json())
            .then((r) => {
                if (r.status === 'OK') {
                    CLIENT.state.app_started = true;
                    CLIENT.connectWebSocket();
                    RP_CLIENT.connectWebSocket();
                } else {
                    console.warn('start of the application:', r.reason || r.status);
                    setTimeout(CLIENT.startApp, 2000);
                }
            })
            .catch(() => setTimeout(CLIENT.startApp, 2000));
    };

    // ---------------------------------------------------------------------------------------
    // liveness: the classes of <body> drive the overlays of ../assets (inverted: a class hides one)
    const RP_CLIENT = {
        client_id: (crypto.randomUUID ? crypto.randomUUID() : String(Math.random()).slice(2)),
        socket_url: https ? 'wss://' + location.host + '/wsc' : 'ws://' + location.hostname + ':9091',
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

    window.addEventListener('beforeunload', () => {
        if (CLIENT.ws) {
            CLIENT.ws.onclose = null;
            CLIENT.ws.close();
        }
        if (RP_CLIENT.ws) {
            RP_CLIENT.ws.onclose = null;
            RP_CLIENT.ws.close();
        }
        // synchronous: the module must stop before the page goes
        try {
            const x = new XMLHttpRequest();
            x.open('GET', CLIENT.config.stop_app_url, false);
            x.send();
        } catch (e) {
            // the page goes anyway
        }
    });

    window.CLIENT = CLIENT;
    window.RP_CLIENT = RP_CLIENT;
})();
