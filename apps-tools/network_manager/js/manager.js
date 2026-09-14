/*
 * Red Pitaya Network Manager
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * Sections are driven by the .tab / .tablinks / .tabcontent contract from
 * apps-tools/ecosystem (the main_menu page).
 *
 * Two rules shape most of what follows:
 *
 *   Never claim success the backend has not reported. Every script here answers
 *   "ok: ..." or "error: ...", and the UI shows that answer rather than
 *   assuming the request worked because it returned 200.
 *
 *   A slow request is not a failed one. Scanning, connecting and restarting a
 *   service hold the single nginx worker for seconds, during which the status
 *   poll cannot be answered. Those timeouts say nothing about the service, so
 *   they are counted separately from real failures.
 */

(function (WIZARD, $, undefined) {

    WIZARD.present = false;      // wlan0 exists
    WIZARD.apCapable = false;    // adapter can do AP mode
    WIZARD.mode = 'none';        // none | client | ap
    WIZARD.scan = [];
    WIZARD.sortKey = 'sig';
    WIZARD.sortAsc = false;
    WIZARD.firstAnswer = false;

    /* Requests that occupy the nginx worker for seconds. While one is in
       flight the status poll cannot be answered in time, and its timeout is
       meaningless. */
    WIZARD.busy = 0;
    WIZARD.pollFailures = 0;
    WIZARD.ethFailures = 0;

    WIZARD.beginBusy = function () { WIZARD.busy++; };

    WIZARD.endBusy = function () {
        if (WIZARD.busy > 0) WIZARD.busy--;
        /* A request that was already in flight when the blocking one started
           can land after it finishes, still having timed out because of it.
           Forget what was counted around a blocking call - none of it is
           evidence that anything is actually down. */
        if (WIZARD.busy === 0) {
            WIZARD.pollFailures = 0;
            WIZARD.ethFailures = 0;
        }
    };

    /* ------------------------------------------------------------------ */
    /* helpers                                                             */
    /* ------------------------------------------------------------------ */

    function esc(s) { return $('<div>').text(s === undefined || s === null ? '' : s).html(); }

    function msg(sel, text, cls) {
        $(sel).attr('class', 'nm-msg ' + (cls || '')).text(text || '');
    }

    function bytes(n) {
        n = parseFloat(n);
        if (!isFinite(n)) return '—';
        var u = ['B', 'kB', 'MB', 'GB'], i = 0;
        while (n >= 1024 && i < u.length - 1) { n /= 1024; i++; }
        return (i === 0 ? n : n.toFixed(1)) + ' ' + u[i];
    }

    function secs(n) {
        n = parseInt(n, 10);
        if (!isFinite(n)) return '—';
        if (n < 60) return n + ' s';
        if (n < 3600) return Math.floor(n / 60) + ' min';
        return Math.floor(n / 3600) + ' h ' + Math.floor((n % 3600) / 60) + ' min';
    }

    /* key/value rows: [label, value, cssClass, markup]
       A 4th element is used as the cell's contents verbatim instead of the
       escaped value, for the one row that needs an element rather than text.
       Anything interpolated into it must already have gone through esc(). */
    function rows(sel, list) {
        var html = '';
        for (var i = 0; i < list.length; i++) {
            var v = list[i][1];
            if (v === undefined || v === null || v === '') v = '—';
            html += '<tr><th>' + esc(list[i][0]) + '</th><td'
                 + (list[i][2] ? ' class="' + list[i][2] + '"' : '') + '>'
                 + (list[i][3] !== undefined ? list[i][3] : esc(v)) + '</td></tr>';
        }
        $(sel).html(html);
    }

    /* Parse the key=value body of /get_wlan0_info. Values may contain "=", so
       split on the first one only. */
    function parseKV(text) {
        var out = {}, lines = (text || '').split('\n');
        for (var i = 0; i < lines.length; i++) {
            var p = lines[i].indexOf('=');
            if (p > 0) out[lines[i].substring(0, p)] = lines[i].substring(p + 1);
        }
        return out;
    }

    /* iw prints "-29.00"; two decimals of dBm are noise. */
    function dbm(v) {
        var n = parseFloat(v);
        return isFinite(n) ? Math.round(n) + ' dBm' : '—';
    }

    function barsFor(dbm) {
        return '<span class="nm-bars s' + barLevel(dbm) + '"><i></i><i></i><i></i><i></i></span>';
    }

    /* The same four bands the scan table's bars use, so the Status icon and the
       Scan icon never disagree about the same radio. */
    function barLevel(dbm) {
        var n = parseFloat(dbm);
        return !isFinite(n) ? 0 : n >= -53 ? 4 : n >= -71 ? 3 : n >= -81 ? 2 : 1;
    }

    var SIGNAL_WORD = ['no signal', 'weak', 'fair', 'good', 'excellent'];

    /* The Signal row on Status: the same bar glyph as the scan list, in front of
       the number, with the reading spelled out on hover. -67 dBm means nothing
       to most people; "good" does. */
    function signalCell(sig) {
        var lvl = barLevel(sig);
        var reading = dbm(sig);
        if (lvl === 0) return esc(reading);
        return '<span class="adaptive-tooltip nm-sig">'
             + barsFor(sig)
             + '<span class="tooltiptext top">' + esc(SIGNAL_WORD[lvl])
             + ' &mdash; ' + esc(reading) + ' (' + lvl + ' of 4)</span>'
             + '</span>' + esc(reading);
    }

    /* ------------------------------------------------------------------ */
    /* passphrase reveal                                                   */
    /* ------------------------------------------------------------------ */

    /* Drawn inline rather than pulled from an icon font or an image, so the
       control needs no asset and inherits the surrounding colour. */
    var EYE_OPEN = '<svg class="ico" viewBox="0 0 24 24" fill="none" stroke="currentColor"'
                 + ' stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round"'
                 + ' aria-hidden="true">'
                 + '<path d="M1.7 12S5.6 5.4 12 5.4 22.3 12 22.3 12 18.4 18.6 12 18.6 1.7 12 1.7 12z"/>'
                 + '<circle cx="12" cy="12" r="2.9"/></svg>';

    var EYE_SHUT = '<svg class="ico" viewBox="0 0 24 24" fill="none" stroke="currentColor"'
                 + ' stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round"'
                 + ' aria-hidden="true">'
                 + '<path d="M1.7 12S5.6 5.4 12 5.4 22.3 12 22.3 12 18.4 18.6 12 18.6 1.7 12 1.7 12z"/>'
                 + '<circle cx="12" cy="12" r="2.9"/>'
                 + '<line x1="3.6" y1="20.4" x2="20.4" y2="3.6"/></svg>';

    function setReveal(box, on) {
        var input = box.find('input');
        var btn = box.find('.nm-eye');
        input.attr('type', on ? 'text' : 'password');
        btn.attr('aria-pressed', on ? 'true' : 'false')
           .attr('aria-label', on ? 'Hide passphrase' : 'Show passphrase')
           .attr('title', on ? 'Hide passphrase' : 'Show passphrase')
           .html(on ? EYE_SHUT : EYE_OPEN);
    }

    WIZARD.hideAllPass = function () {
        $('.nm-pass').each(function () { setReveal($(this), false); });
    };

    WIZARD.bindReveal = function () {
        WIZARD.hideAllPass();
        $('.nm-pass .nm-eye').click(function () {
            var box = $(this).closest('.nm-pass');
            if (box.find('input').prop('disabled')) return;
            setReveal(box, box.find('input').attr('type') === 'password');
        });
    };

    /* ------------------------------------------------------------------ */
    /* tabs                                                                */
    /* ------------------------------------------------------------------ */

    /* The ecosystem original marks the active link through evt.currentTarget.
       That only holds while the browser is dispatching the event: trigger the
       same handler from code (jQuery .click()) and currentTarget is undefined,
       so the section switched but no link lit up. Resolve the link from the
       section name instead - each one names its section in its onclick - and
       the same function then works from a real click and from code alike. */
    WIZARD.showTab = function (name) {
        var i, c = document.getElementsByClassName('tabcontent');
        for (i = 0; i < c.length; i++) c[i].classList.remove('shown');

        var l = document.getElementsByClassName('tablinks');
        for (i = 0; i < l.length; i++) {
            l[i].className = l[i].className.replace(' active', '');
            if ((l[i].getAttribute('onclick') || '').indexOf("'" + name + "'") !== -1) {
                l[i].className += ' active';
            }
        }

        var sec = document.getElementById(name);
        if (sec) sec.classList.add('shown');

        /* Leaving a section puts its passphrase back behind the dots. Revealing
           one is meant to be a look, not a setting that outlives the visit. */
        if (WIZARD.hideAllPass) WIZARD.hideAllPass();

        /* Opening Wired shows what the board is set to now - unless there are
           edits in progress, which fillEthForm leaves alone. */
        if (name === 'nm_wired') WIZARD.fillEthForm();

        if (name === 'nm_scan' && !WIZARD.scan.length) WIZARD.startScan();
        if (name === 'nm_diag') WIZARD.loadDiag();
        if (name === 'nm_ap') WIZARD.loadApClients();
        if (name === 'nm_https') {
            /* Entering the section starts from what the board is running. The
               form stops refreshing itself once edited, and leaving and coming
               back is the gesture that means "show me the real state again". */
            WIZARD.httpsDirty = false;
            WIZARD.httpsSourceTouched = false;
            WIZARD.loadHttps();
        }
    };

    /* Kept for the inline onclick in index.html, which follows the ecosystem
       markup contract. */
    WIZARD.openTab = function (evt, name) { WIZARD.showTab(name); };

    /* ------------------------------------------------------------------ */
    /* state polling                                                       */
    /* ------------------------------------------------------------------ */

    WIZARD.poll = function () {
        $.ajax({ url: '/get_wlan0_state', type: 'GET', timeout: 2000 })
            .success(function (m) {
                WIZARD.pollFailures = 0;
                if (!WIZARD.firstAnswer) {
                    WIZARD.firstAnswer = true;
                    $('#nm_loading').hide();
                    $('#nm_shell').show();
                }
                $('#nm_fail').removeClass('on');

                m = (m || '').trim();
                WIZARD.present = m[0] === '1';
                WIZARD.apCapable = m[1] === '1';
                WIZARD.mode = m[2] === '1' ? 'client' : m[2] === '2' ? 'ap' : 'none';

                $('body').toggleClass('nm-noadapter', !WIZARD.present);
                $('#pill_wlan').attr('class', WIZARD.present ? 'up' : 'down')
                    .text(!WIZARD.present ? 'absent' : WIZARD.mode === 'ap' ? 'AP' :
                          WIZARD.mode === 'client' ? 'client' : 'idle');
                $('#ap_start').prop('disabled', !WIZARD.apCapable);

                WIZARD.loadWlanInfo();
            })
            .error(function () {
                /* A scan or a connect blocks the worker; those timeouts are not
                   failures. Outside them, three misses in a row before saying so. */
                if (WIZARD.busy > 0) return;
                if (++WIZARD.pollFailures < 3) return;
                $('#nm_loading').hide();
                $('#nm_shell').hide();
                $('#nm_fail').addClass('on');
            });
    };

    WIZARD.loadWlanInfo = function () {
        $.ajax({ url: '/get_wlan0_info', type: 'GET', timeout: 4000 })
            .success(function (text) {
                var d = parseKV(text);

                if (d.present !== '1') {
                    rows('#kv_wlan', [['State', 'absent']]);
                    rows('#kv_adapter', [['Adapter', 'none']]);
                    return;
                }

                var state = d.state === 'associated' ? 'associated'
                          : d.state === 'ap' ? 'access point' : 'not connected';
                var cls = d.state === 'associated' || d.state === 'ap' ? 'nm-up' : '';

                rows('#kv_wlan', [
                    ['State', state, cls],
                    ['Mode', d.type],
                    ['SSID', d.ssid],
                    ['BSSID', d.bssid],
                    ['Channel', d.channel ? d.channel + (d.freq ? ' (' + d.freq + ' MHz)' : '')
                                           + (d.chanwidth ? ', ' + d.chanwidth : '') : ''],
                    ['Signal', dbm(d.signal), '', signalCell(d.signal)],
                    ['Tx rate', d.txrate],
                    ['Rx rate', d.rxrate],
                    ['Address', d.addr],
                    ['Gateway', d.gw],
                    ['MAC', d.mac],
                    ['Power save', d.powersave],
                    ['Traffic', 'rx ' + bytes(d.rxbytes) + ' / tx ' + bytes(d.txbytes)]
                ]);

                rows('#kv_adapter', [
                    ['USB device', d.usbid ? d.usbid + (d.usbspeed ? ' · ' + d.usbspeed + ' Mbps' : '')
                                             + (d.usbpower ? ' · ' + d.usbpower : '') : ''],
                    ['Driver', d.driver],
                    ['phy', d.phy ? 'phy' + d.phy : ''],
                    ['Interface modes', d.modes],
                    ['Autosuspend', d.usbctl],
                    ['Country (configured)', d.country],
                    ['Country (kernel)', d.regdomain]
                ]);

                $('#ap_country').val(d.country || '');

                /* Prefill the wireless form once, from what is actually running,
                   so Connect does not need retyping to reconnect. */
                if (!WIZARD.ssidPrefilled && d.ssid && d.state === 'associated') {
                    WIZARD.ssidPrefilled = true;
                    if (!$('#wl_ssid').val()) $('#wl_ssid').val(d.ssid);
                }
            });
    };

    /* wired.network as sections of keys: {Network: {Address: "..."}, ...}.
       Section-aware because DNS= exists under both [Network] and [DHCPServer]
       and means different things there - the resolver this board uses, versus
       the resolver it hands to its clients. */
    function parseIni(text) {
        var out = {}, sec = '', lines = (text || '').split('\n');
        for (var i = 0; i < lines.length; i++) {
            var l = lines[i].replace(/^\s+|\s+$/g, '');
            if (!l || l.charAt(0) === '#' || l.charAt(0) === ';') continue;
            var s = l.match(/^\[(.+)\]$/);
            if (s) { sec = s[1]; out[sec] = out[sec] || {}; continue; }
            var p = l.indexOf('=');
            if (p > 0 && sec) {
                out[sec][l.substring(0, p).replace(/\s+$/, '')] =
                    l.substring(p + 1).replace(/^\s+/, '');
            }
        }
        return out;
    }

    /* systemd writes a prefix length; the form asks for a dotted mask. */
    function prefixToMask(prefix) {
        var n = parseInt(prefix, 10);
        if (!(n >= 0 && n <= 32)) return '';
        var o = [0, 0, 0, 0];
        for (var i = 0; i < 4; i++) {
            var bits = Math.min(8, Math.max(0, n - i * 8));
            o[i] = bits === 0 ? 0 : (256 - Math.pow(2, 8 - bits));
        }
        return o.join('.');
    }

    /* What the board is actually configured to do, read back out of
       wired.network so the form can show it. */
    function parseNetwork(text) {
        var raw = (text.split('config:')[1] || '');
        var ini = parseIni(raw);
        var net = ini.Network || {};
        var srv = ini.DHCPServer || {};
        var mode = /DHCPServer\s*=\s*yes/i.test(raw) ? 'server'
                 : /^\s*DHCP\s*=\s*(ipv4|yes)/mi.test(raw) ? 'dhcp'
                 : (net.Address ? 'static' : '');

        var cidr = (net.Address || '').split('/');
        return {
            mode: mode,
            address: cidr[0] || '',
            netmask: prefixToMask(cidr[1]),
            gateway: net.Gateway || (ini.Route || {}).Gateway || '',
            dns: net.DNS || '',
            pool_offset: srv.PoolOffset || '',
            pool_size: srv.PoolSize || '',
            lease: srv.DefaultLeaseTimeSec || '',
            router: srv.Router || '',
            server_dns: srv.DNS || ''
        };
    }

    /* Every IPv4 address on the interface, with its scope, newest "ip addr"
       layout or older - the fields between the prefix and "scope" vary (brd,
       metric, dynamic, noprefixroute), so anchor on "inet" and on "scope"
       rather than on their order. */
    function inetAddrs(text) {
        var out = [], re = /inet\s+([0-9.]+\/\d+)([^\n]*)/g, m;
        while ((m = re.exec(text || '')) !== null) {
            var scope = (m[2].match(/scope\s+(\S+)/) || [])[1] || '';
            out.push({ cidr: m[1], scope: scope, link: scope === 'link' || /^169\.254\./.test(m[1]) });
        }
        return out;
    }

    /* /get_eth0_status returns the raw wired.network after "config:". Report
       which of the three shapes it is rather than echoing its first line. */
    function protoOf(text) {
        var cfg = (text.split('config:')[1] || '');
        if (/DHCPServer\s*=\s*yes/i.test(cfg)) return 'DHCP server';
        if (/DHCP\s*=\s*(ipv4|yes)/i.test(cfg)) return 'DHCP client';
        if (/^\s*Address\s*=/mi.test(cfg)) return 'static address';
        if (/no .*wired\.network/i.test(cfg)) return 'unconfigured (link-local / DHCP fallback)';
        return '';
    }

    WIZARD.loadEth = function () {
        $.ajax({ url: '/get_eth0_status', type: 'GET', timeout: 3000 })
            .success(function (text) {
                WIZARD.ethFailures = 0;
                text = text || '';

                /* An interface can hold several addresses at once. Reporting
                   whichever "inet" line came first showed 169.254.x.x on a board
                   that had been given a static 200.0.0.11 - the link-local was
                   simply printed above it. Show the routable ones; mention a
                   link-local separately, since it is worth knowing it is there
                   but it is never the answer to "what is this board's address". */
                var all = inetAddrs(text);
                var real = [], ll = [];
                for (var i = 0; i < all.length; i++) {
                    (all[i].link ? ll : real).push(all[i].cidr);
                }
                var addr = real.join(', ');
                if (!addr && ll.length) addr = ll[0] + ' (link-local only)';

                var gw = (text.match(/gateway:\s*([0-9.]+)/) || [])[1];
                var up = /state UP/.test(text) || real.length > 0;

                $('#pill_eth').attr('class', up ? 'up' : 'down').text(up ? 'up' : 'down');

                WIZARD.ethConfig = parseNetwork(text);

                var list = [
                    ['State', up ? 'up' : 'down', up ? 'nm-up' : 'nm-down'],
                    ['Address', addr],
                    ['Gateway', gw],
                    ['Protocol', protoOf(text)]
                ];
                if (real.length && ll.length) {
                    list.push(['Link-local', ll.join(', ')]);
                }
                rows('#kv_eth', list);

                /* Fill the form from what is actually on the board, once, so the
                   Wired section opens showing the current configuration instead
                   of an empty form defaulted to DHCP client. */
                if (!WIZARD.ethFilled) {
                    WIZARD.ethFilled = true;
                    WIZARD.fillEthForm();
                }
            })
            .error(function () {
                /* A scan or a connect owns the single nginx worker for seconds,
                   so this poll times out through no fault of eth0 - and the wired
                   link is the one thing that certainly has not changed while the
                   radio is busy. Say nothing: keep the last known state rather
                   than flipping the pill to "?" every time someone scans.

                   Outside a blocking call, three misses in a row before
                   admitting it, for the same reason the state poll does. */
                if (WIZARD.busy > 0) return;
                if (++WIZARD.ethFailures < 3) return;
                $('#pill_eth').attr('class', 'down').text('?');
            });
    };

    /* ------------------------------------------------------------------ */
    /* scan                                                                */
    /* ------------------------------------------------------------------ */

    WIZARD.startScan = function () {
        if (!WIZARD.present) return;
        $('#scan_body').html('<tr><td class="empty" colspan="5">Scanning…</td></tr>');
        $('#scan_btn').prop('disabled', true);
        WIZARD.beginBusy();

        $.ajax({ url: '/get_wnet_list', type: 'GET', timeout: 30000 })
            .done(function (m) {
                if (typeof m === 'string') { try { m = JSON.parse(m); } catch (e) { m = null; } }
                WIZARD.scan = (m && m.scan) ? m.scan : [];
                WIZARD.renderScan();
            })
            .fail(function () {
                $('#scan_body').html('<tr><td class="empty" colspan="5">Scan failed.</td></tr>');
            })
            .always(function () {
                WIZARD.endBusy();
                $('#scan_btn').prop('disabled', false);
            });
    };

    WIZARD.renderScan = function () {
        var list = WIZARD.scan.slice();
        var k = WIZARD.sortKey, asc = WIZARD.sortAsc;
        list.sort(function (a, b) {
            var x = a[k], y = b[k];
            if (k === 'sig') { x = parseFloat(x); y = parseFloat(y); }
            else { x = String(x).toLowerCase(); y = String(y).toLowerCase(); }
            return (x < y ? -1 : x > y ? 1 : 0) * (asc ? 1 : -1);
        });

        $('#scan_count').text(list.length ? list.length : '');
        $('#scan_sub').text(list.length ? list.length + ' networks' : '');

        if (!list.length) {
            $('#scan_body').html('<tr><td class="empty" colspan="5">No networks found.</td></tr>');
            return;
        }

        var html = '';
        for (var i = 0; i < list.length; i++) {
            var n = list[i];
            var open = n.enc === 'Open';
            var cls = n.sae === 'Yes' ? 'wpa3' : open ? 'open' : '';
            html += '<tr data-ssid="' + esc(n.SSID) + '" data-open="' + (open ? '1' : '0') + '">'
                 + '<td>' + barsFor(n.sig) + '</td>'
                 + '<td class="ssid">' + esc(n.SSID) + '</td>'
                 + '<td><span class="nm-tag ' + cls + '">' + esc(n.enc) + '</span></td>'
                 + '<td class="mono">' + esc(dbm(n.sig)) + '</td>'
                 + '<td><button class="sm">Use</button></td>'
                 + '</tr>';
        }
        $('#scan_body').html(html);
    };

    /* ------------------------------------------------------------------ */
    /* access point                                                        */
    /* ------------------------------------------------------------------ */

    WIZARD.loadApClients = function () {
        if (!WIZARD.present) return;
        $.ajax({ url: '/get_ap_clients', type: 'GET', timeout: 4000 })
            .success(function (m) {
                if (typeof m === 'string') { try { m = JSON.parse(m); } catch (e) { m = null; } }

                /* Fill the addressing fields from the file the AP actually
                   uses, once, and leave them alone afterwards so a half-typed
                   address is not overwritten by the next refresh. */
                if (m && m.net && !WIZARD.apFilled) {
                    WIZARD.apFilled = true;
                    var n = m.net;
                    if (n.address) $('#ap_addr').val(n.address);
                    if (n.netmask) $('#ap_mask').val(n.netmask);
                    if (n.pool_offset) $('#ap_offset').val(n.pool_offset);
                    if (n.pool_size) $('#ap_size').val(n.pool_size);
                    if (n.lease) $('#ap_lease').val(n.lease);
                    if (n.dns) $('#ap_dns').val(n.dns);
                    WIZARD.showApPool();
                }

                var st = (m && m.stations) ? m.stations : [];
                if (!st.length) {
                    $('#ap_clients').html('<tr><td class="empty" colspan="6">No stations.</td></tr>');
                    return;
                }
                var html = '';
                for (var i = 0; i < st.length; i++) {
                    html += '<tr><td class="ssid">' + esc(st[i].addr || '—') + '</td>'
                         + '<td class="mono">' + esc(st[i].mac) + '</td>'
                         + '<td class="mono">' + esc(dbm(st[i].signal)) + '</td>'
                         + '<td class="mono">' + secs(st[i].connected) + '</td>'
                         + '<td class="mono">' + bytes(st[i].rx) + '</td>'
                         + '<td class="mono">' + bytes(st[i].tx) + '</td></tr>';
                }
                $('#ap_clients').html(html);
            });
    };

    /* ------------------------------------------------------------------ */
    /* diagnostics                                                         */
    /* ------------------------------------------------------------------ */

    /* The section is only worth a place in the rail when something is actually
       wrong. "warn" and "bad" count; "info" does not - no adapter plugged in and
       no wireless configured yet are legitimate states on a board used over
       ethernet, and showing them as findings teaches people to ignore the whole
       section. The backend counts them in "problems"; older payloads without
       that field are counted here instead.

       `quiet` is the automatic check on page load: it must not overwrite the
       table with "Running…" or steal the section the user is looking at. */
    WIZARD.loadDiag = function (quiet) {
        if (!quiet) $('#diag_checks').html('<tr><td class="d" colspan="3">Running…</td></tr>');
        WIZARD.beginBusy();
        $.ajax({ url: '/get_diag', type: 'GET', timeout: 15000 })
            .success(function (m) {
                if (typeof m === 'string') { try { m = JSON.parse(m); } catch (e) { m = null; } }
                if (!m || !m.checks) {
                    if (!quiet) {
                        $('#diag_checks').html('<tr><td class="d" colspan="3">Could not read diagnostics.</td></tr>');
                    }
                    return;
                }
                var html = '', problems = 0;
                var icon = { ok: '✓', info: '·', warn: '!', bad: '✗' };
                for (var i = 0; i < m.checks.length; i++) {
                    var c = m.checks[i];
                    if (c.level === 'warn' || c.level === 'bad') problems++;
                    html += '<tr class="' + esc(c.level) + '">'
                         + '<td class="i">' + (icon[c.level] || '') + '</td>'
                         + '<td>' + esc(c.title) + '</td>'
                         + '<td class="d">' + esc(c.detail) + '</td></tr>';
                }
                if (typeof m.problems === 'number') problems = m.problems;

                $('#diag_checks').html(html);
                WIZARD.showDiagTab(problems);
            })
            .error(function () {
                /* An endpoint that does not answer is itself a problem worth
                   seeing - but only once the user has asked for the checks. */
                if (!quiet) {
                    $('#diag_checks').html('<tr><td class="d" colspan="3">Diagnostics endpoint failed.</td></tr>');
                    WIZARD.showDiagTab(1);
                }
            })
            .always(function () { WIZARD.endBusy(); });
    };

    /* Reveal or hide the rail entry. Hiding it while its section is on screen
       would leave the user staring at a section with nothing selected in the
       rail, so fall back to Status in that case. */
    WIZARD.showDiagTab = function (problems) {
        var link = $('#tab_diag');
        if (problems > 0) {
            link.show();
            $('#diag_dot').addClass('on');
            return;
        }
        link.hide();
        $('#diag_dot').removeClass('on');
        if ($('#nm_diag').hasClass('shown')) WIZARD.showTab('nm_status');
    };

    /* ------------------------------------------------------------------ */
    /* country                                                             */
    /* ------------------------------------------------------------------ */

    WIZARD.loadCountry = function () {
        $.ajax({ url: '/get_wifi_country', type: 'GET', timeout: 3000 })
            .success(function (m) {
                var cc = (m || '').trim().toUpperCase();
                if (/^[A-Z0-9]{2}$/.test(cc)) {
                    $('#wlan0_country').val(cc);
                    $('#ap_country').val(cc);
                }
            });
    };

    WIZARD.applyCountry = function (cc) {
        msg('#country_msg', 'Applying ' + cc + '…', 'busy');
        WIZARD.beginBusy();
        $.ajax({ url: '/set_wifi_country?country=' + encodeURIComponent(cc), type: 'GET', timeout: 20000 })
            .success(function (m) {
                var t = (m || '').trim();
                /* set_country.sh answers "ok: XX" or "warning: ..." - the kernel
                   can refuse a code that regulatory.db does not carry, so show
                   what came back rather than assuming it took. */
                if (t.indexOf('ok:') === 0) {
                    msg('#country_msg', 'Country set to ' + cc, 'ok');
                    $('#ap_country').val(cc);
                } else {
                    msg('#country_msg', t, 'warn');
                }
            })
            .error(function (x) { msg('#country_msg', (x.responseText || 'request failed').trim(), 'bad'); })
            .always(function () { WIZARD.endBusy(); });
    };

    /* ------------------------------------------------------------------ */
    /* actions                                                             */
    /* ------------------------------------------------------------------ */

    function answer(sel, text, fallbackOk) {
        var t = (text || '').trim();
        if (t.indexOf('error:') === 0) { msg(sel, t, 'bad'); return false; }
        if (t.indexOf('warning:') === 0) { msg(sel, t, 'warn'); return false; }
        if (t.indexOf('ok:') === 0) { msg(sel, t.substring(3).trim(), 'ok'); return true; }
        msg(sel, fallbackOk, 'ok');
        return true;
    }

    WIZARD.connect = function () {
        var ssid = $('#wl_ssid').val();
        var pass = $('#wl_pass').val();
        var km = $('#wl_keymgmt').val();

        if (!ssid) { msg('#wl_msg', 'Enter an SSID', 'bad'); return; }
        if (km !== 'open' && pass && (pass.length < 8 || pass.length > 63)) {
            msg('#wl_msg', 'Passphrase must be 8 to 63 characters', 'bad'); return;
        }

        msg('#wl_msg', 'Connecting… this can take up to 25 s', 'busy');
        $('#wl_connect').prop('disabled', true);
        WIZARD.beginBusy();

        $.ajax({
            url: '/connect_wifi?ssid=' + encodeURIComponent(ssid)
               + '&password=' + encodeURIComponent(pass)
               + '&keymgmt=' + encodeURIComponent(km)
               + '&pmf=' + encodeURIComponent($('#wl_pmf').val())
               + '&hidden=' + encodeURIComponent($('#wl_hidden').is(':checked') ? '1' : '0'),
            type: 'GET', timeout: 70000
        })
            .success(function (m) { answer('#wl_msg', m, 'Connected'); })
            .error(function (x) { msg('#wl_msg', (x.responseText || 'request failed').trim(), 'bad'); })
            .always(function () {
                WIZARD.endBusy();
                $('#wl_connect').prop('disabled', false);
                WIZARD.loadWlanInfo();
            });
    };

    WIZARD.disconnect = function () {
        msg('#wl_msg', 'Disconnecting…', 'busy');
        WIZARD.beginBusy();
        $.ajax({ url: '/disconnect_wifi', type: 'GET', timeout: 30000 })
            .always(function () {
                WIZARD.endBusy();
                msg('#wl_msg', 'Disconnected', 'ok');
                WIZARD.loadWlanInfo();
            });
    };

    /* Same arithmetic as the wired DHCP server, on the AP's own segment. An
       empty field means create_ap.sh's default, so the range shown is real
       either way - marked, so the line does not read as though the fields were
       filled in. */
    WIZARD.showApPool = function () {
        var base = ($('#ap_addr').val() || '192.168.128.1').trim();
        var rawOff = ($('#ap_offset').val() || '').trim();
        var rawSize = ($('#ap_size').val() || '').trim();
        var off = parseInt(rawOff || '10', 10);
        var size = parseInt(rawSize || '40', 10);
        var m = base.match(/^(\d+\.\d+\.\d+)\.(\d+)$/);
        if (!m || !(off > 0) || !(size > 0) || off + size - 1 > 254) {
            $('#ap_range').text('pool does not fit the address given');
            return;
        }
        var note = (!rawOff || !rawSize) ? ' (default)' : '';
        $('#ap_range').text('clients get ' + m[1] + '.' + off
            + ' to ' + m[1] + '.' + (off + size - 1) + ', ' + size + ' addresses' + note);
    };

    WIZARD.startAP = function () {
        var ssid = $('#ap_ssid').val();
        var pass = $('#ap_pass').val();
        var q = function (v) { return encodeURIComponent((v || '').trim()); };

        if (!ssid) { msg('#ap_msg', 'Enter an SSID', 'bad'); return; }
        if (pass && (pass.length < 8 || pass.length > 63)) {
            msg('#ap_msg', 'Passphrase must be 8 to 63 characters, or empty for an open network', 'bad'); return;
        }

        /* Caught here so a bad pool does not cost an interface restart before
           create_ap.sh says no. It checks all of this again regardless - it can
           be run from a shell too. */
        var addr = ($('#ap_addr').val() || '').trim();
        var off = parseInt(($('#ap_offset').val() || '10').trim(), 10);
        var size = parseInt(($('#ap_size').val() || '40').trim(), 10);
        if (!(off > 0) || !(size > 0)) {
            msg('#ap_msg', 'First address and pool size must be positive numbers', 'bad'); return;
        }
        if (off + size - 1 > 254) {
            msg('#ap_msg', 'Pool runs past the end of the subnet: ' + off + ' + ' + size
                + ' exceeds 254', 'bad'); return;
        }
        var host = parseInt((addr || '192.168.128.1').split('.')[3], 10);
        if (host >= off && host < off + size) {
            msg('#ap_msg', 'The board’s own address is inside the pool it hands out '
                + '(' + off + '–' + (off + size - 1) + '); move the pool or the address', 'bad');
            return;
        }

        msg('#ap_msg', 'Starting…', 'busy');
        $('#ap_start').prop('disabled', true);
        WIZARD.beginBusy();

        $.ajax({
            url: '/wifi_create_point?ssid=' + encodeURIComponent(ssid)
               + '&password=' + encodeURIComponent(pass)
               + '&channel=' + encodeURIComponent($('#ap_chan').val())
               + '&hwmode=' + encodeURIComponent($('#ap_mode').val())
               + '&address=' + q($('#ap_addr').val())
               + '&netmask=' + q($('#ap_mask').val())
               + '&pool_offset=' + q($('#ap_offset').val())
               + '&pool_size=' + q($('#ap_size').val())
               + '&lease=' + q($('#ap_lease').val())
               + '&dns=' + q($('#ap_dns').val()),
            type: 'GET', timeout: 60000
        })
            .success(function (m) { answer('#ap_msg', m, 'Access point running'); })
            .error(function (x) { msg('#ap_msg', (x.responseText || 'request failed').trim(), 'bad'); })
            .always(function () {
                WIZARD.endBusy();
                $('#ap_start').prop('disabled', !WIZARD.apCapable);
                WIZARD.loadApClients();
            });
    };

    WIZARD.stopAP = function () {
        msg('#ap_msg', 'Stopping…', 'busy');
        WIZARD.beginBusy();
        $.ajax({ url: '/remove_ap', type: 'GET', timeout: 30000 })
            .always(function () {
                WIZARD.endBusy();
                msg('#ap_msg', 'Access point stopped', 'ok');
                WIZARD.loadApClients();
            });
    };

    /* What each protocol does with the four address rows.

       'on'  - editable
       'off' - shown but disabled: the setting exists in this mode, the board
               just is not the one choosing it. A DHCP client is handed address,
               netmask, gateway and DNS by the lease, so the fields stay visible
               (that is what will be configured) but accept no input that would
               be thrown away on apply.
       'no'  - not part of this mode at all, so hidden. A DHCP server has no
               gateway or upstream resolver of its own to set here; what it
               hands to clients is the Router and DNS in the block below, and
               leaving a greyed "Gateway" sitting above "Router" only invites
               the question of which one wins. */
    var ETH_FIELDS = {
        dhcp:   { addr: 'off', mask: 'off', gw: 'off', dns: 'off', dhcpd: false, head: 'Address' },
        static: { addr: 'on',  mask: 'on',  gw: 'on',  dns: 'on',  dhcpd: false, head: 'Address' },
        server: { addr: 'on',  mask: 'on',  gw: 'no',  dns: 'no',  dhcpd: true,  head: 'Server address' }
    };

    /* Put the board's current configuration into the form: select the protocol
       that is actually in effect and fill the fields belonging to it. Without
       this the section opened on an empty form reading "DHCP client", which is
       a claim about the board, and a wrong one on any board not using DHCP.

       Edits win. Once something has been typed the form is the user's until
       they apply or reset it - repopulating underneath them on every visit to
       the section would throw away work. */
    WIZARD.fillEthForm = function (force) {
        var d = WIZARD.ethConfig;
        if (!d || (WIZARD.ethDirty && !force)) return;

        if (d.mode) $('#eth_mode').val(d.mode);

        $('#eth_addr').val(d.address);
        $('#eth_mask').val(d.netmask);
        $('#eth_gw').val(d.gateway);
        $('#eth_dns').val(d.mode === 'server' ? '' : d.dns);

        $('#dhcpd_offset').val(d.pool_offset);
        $('#dhcpd_size').val(d.pool_size);
        $('#dhcpd_lease').val(d.lease);
        $('#dhcpd_router').val(d.router);
        $('#dhcpd_dns').val(d.server_dns);

        WIZARD.ethDirty = false;
        WIZARD.applyEthMode();
    };

    WIZARD.applyEthMode = function () {
        var f = ETH_FIELDS[$('#eth_mode').val()] || ETH_FIELDS.dhcp;
        var set = function (row, input, state) {
            $(row).toggle(state !== 'no').toggleClass('off', state === 'off');
            $(input).prop('disabled', state !== 'on');
        };
        set('#row_eth_addr', '#eth_addr', f.addr);
        set('#row_eth_mask', '#eth_mask', f.mask);
        set('#row_eth_gw', '#eth_gw', f.gw);
        set('#row_eth_dns', '#eth_dns', f.dns);
        $('#dhcpd_block').toggle(!!f.dhcpd);
        $('#eth_static_head').find('div').text(f.head);
        /* Whatever the last apply said was about the previous mode's fields,
           half of which are now gone from the form. */
        $('#eth_msg').attr('class', 'nm-msg').text('');
        WIZARD.showPool();
    };

    /* The [DHCPServer] pool is an offset and a count, not a range, so show what
       that actually works out to - "100 + 20" is not obviously 192.168.1.100 to
       .119 until you see it written out. */
    WIZARD.showPool = function () {
        var base = ($('#eth_addr').val() || '').trim();
        var rawOff = ($('#dhcpd_offset').val() || '').trim();
        var rawSize = ($('#dhcpd_size').val() || '').trim();
        var off = parseInt(rawOff || '100', 10);
        var size = parseInt(rawSize || '20', 10);
        var m = base.match(/^(\d+\.\d+\.\d+)\.(\d+)$/);
        if (!m || !(off > 0) || !(size > 0) || off + size - 1 > 254) {
            $('#dhcpd_range').text(base ? 'pool does not fit the address given' : '—');
            return;
        }
        /* An empty field means the endpoint's default, so the range shown is
           real either way - but say which numbers came from nowhere, or the
           line reads as though the fields were filled in. */
        var note = (!rawOff || !rawSize) ? ' (default)' : '';
        $('#dhcpd_range').text('clients get ' + m[1] + '.' + off
            + ' to ' + m[1] + '.' + (off + size - 1) + ', ' + size + ' addresses' + note);
    };

    WIZARD.applyEth = function () {
        var mode = $('#eth_mode').val();
        var q = function (v) { return encodeURIComponent((v || '').trim()); };
        var url;

        if (mode === 'dhcp') {
            url = '/set_dhcp_client_eth0';
        } else if (mode === 'static') {
            if (!$('#eth_addr').val() || !$('#eth_mask').val()) {
                msg('#eth_msg', 'Address and netmask are required', 'bad'); return;
            }
            url = '/set_static_eth0?address=' + q($('#eth_addr').val())
                + '&netmask=' + q($('#eth_mask').val())
                + '&gateway=' + q($('#eth_gw').val())
                + '&dns=' + q($('#eth_dns').val());
        } else {
            if (!$('#eth_addr').val() || !$('#eth_mask').val()) {
                msg('#eth_msg', 'Server address and netmask are required', 'bad'); return;
            }
            /* Checked here as well as on the server: the pool has to fit inside
               the subnet, and "offset 200, size 100" is a mistake worth catching
               before the interface is torn down to apply it. */
            var off = parseInt($('#dhcpd_offset').val() || '100', 10);
            var size = parseInt($('#dhcpd_size').val() || '20', 10);
            if (!(off > 0) || !(size > 0)) {
                msg('#eth_msg', 'First address and pool size must be positive numbers', 'bad'); return;
            }
            if (off + size - 1 > 254) {
                msg('#eth_msg', 'Pool runs past the end of the subnet: ' + off + ' + '
                    + size + ' exceeds 254', 'bad'); return;
            }
            var lease = ($('#dhcpd_lease').val() || '').trim();
            if (lease && !/^\d+$/.test(lease)) {
                msg('#eth_msg', 'Lease time must be a whole number of seconds', 'bad'); return;
            }

            url = '/set_dhcp_server_eth0?dhcp_address=' + q($('#eth_addr').val())
                + '&dhcp_mask=' + q($('#eth_mask').val())
                + '&pool_offset=' + off
                + '&pool_size=' + size
                + '&lease=' + q(lease)
                + '&router=' + q($('#dhcpd_router').val())
                + '&dns=' + q($('#dhcpd_dns').val());
        }

        msg('#eth_msg', 'Applying… the page may lose the board briefly', 'busy');
        WIZARD.beginBusy();
        $.ajax({ url: url, type: 'GET', timeout: 30000 })
            .success(function (m) {
                var t = (m || '').trim();
                if (t.indexOf('error') === 0) msg('#eth_msg', t, 'bad');
                else if (t.indexOf('warning') === 0) msg('#eth_msg', t, 'warn');
                else {
                    msg('#eth_msg', 'Applied', 'ok');
                    /* What is on the board is now what is in the form, so the
                       next visit may repopulate from it again. */
                    WIZARD.ethDirty = false;
                }
            })
            .error(function (x) { msg('#eth_msg', (x.responseText || 'request failed').trim(), 'bad'); })
            .always(function () { WIZARD.endBusy(); WIZARD.loadEth(); });
    };

    /* ------------------------------------------------------------------ */
    /* https                                                               */
    /* ------------------------------------------------------------------ */

    WIZARD.https = null;          /* last answer from /get_https_status */
    WIZARD.httpsDirty = false;    /* the form has edits the board has not seen */
    WIZARD.httpsSourceTouched = false;  /* the user picked a way to get a certificate */

    /* One request answers the whole section. */
    WIZARD.loadHttps = function (quiet) {
        $.ajax({ url: '/get_https_status', type: 'GET', dataType: 'json', timeout: 8000 })
            .success(function (st) {
                WIZARD.https = st || {};
                WIZARD.renderHttps();
                if (!quiet) WIZARD.fillHttpsForm();
            })
            .error(function () {
                if (!quiet) msg('#https_msg', 'Could not read the HTTPS state', 'bad');
            });
    };

    WIZARD.renderHttps = function () {
        var st = WIZARD.https;
        if (!st) return;

        $('#https_no_module').toggleClass('on', st.ssl_module !== 1);
        $('#https_self_warn').toggleClass('on',
            st.cert_present === 1 && st.self_signed === 1);

        /* Asked for and being served are separate facts: a failed reload or an
           unpressed Apply leaves them apart. */
        var pending = (st.enabled === 1) !== (st.applied === 1)
                   || (st.enabled === 1 && (st.redirect === 1) !== (st.redirect_applied === 1));

        /* Both say "asked for but not running"; only the one naming a cause is
           shown, so nobody is sent to press Apply on a doomed certificate. */
        var bootOff = st.boot_disabled === 1;
        $('#https_boot_off').toggleClass('on', bootOff);
        $('#https_pending').toggleClass('on', pending && !bootOff);

        var days = parseInt(st.days_left, 10);

        /* The dot means "needs attention", as in the diagnostics section: a
           setting that never took effect, or a certificate about to expire. */
        var attention = pending || bootOff
            || (st.enabled === 1 && st.cert_present !== 1)
            || (st.cert_present === 1 && isFinite(days) && days < 14);
        $('#https_dot').toggleClass('on', !!attention);

        var expiry = st.not_after || '';
        if (isFinite(days)) {
            expiry += days < 0 ? ' (expired)' : ' (' + days + ' days left)';
        }

        var sourceName = { self: 'self-signed', upload: 'external' };

        rows('#kv_https', [
            ['State', st.applied === 1
                ? 'serving https on port ' + st.port
                : (st.enabled === 1 ? 'enabled but not applied' : 'off')],
            ['Source', sourceName[st.cert_source] || st.cert_source],
            ['File', st.cert_file],
            ['Subject', st.cert_present === 1 ? st.subject : 'no certificate installed'],
            ['Issuer', st.cert_present === 1 ? st.issuer : ''],
            ['Valid until', st.cert_present === 1 ? expiry : '',
                isFinite(days) && days < 0 ? 'bad' : ''],
            ['Names', st.cert_present === 1 ? st.san : ''],
            ['SHA-256', st.cert_present === 1 ? st.fingerprint : '']
        ]);

        $('#https_download').prop('disabled', st.cert_present !== 1);
        $('#https_export').prop('disabled', st.cert_present !== 1 || st.key_present !== 1);
    };

    /* Same contract as fillEthForm: edits in progress are left alone. */
    WIZARD.fillHttpsForm = function (force) {
        var st = WIZARD.https;
        if (!st) return;
        if (WIZARD.httpsDirty && !force) return;

        $('#https_enabled').prop('checked', st.enabled === 1);
        $('#https_redirect').prop('checked', st.redirect === 1);
        $('#https_port').val(st.port || 443);
        if (!WIZARD.httpsSourceTouched) $('#https_source').val('self');
        if (!$('#https_cn').val()) $('#https_cn').val(st.hostname || '');

        WIZARD.httpsDirty = false;
        WIZARD.applyHttpsSource();
    };

    WIZARD.applyHttpsSource = function () {
        var src = $('#https_source').val();
        $('#https_self_block').toggle(src === 'self');
        $('#https_upload_block').toggle(src === 'upload');
        $('#https_p12_block').toggle(src === 'p12');
    };

    /* The scripts prefix their answer with "error:" or "warning:" and say why,
       so it is shown as it is. */
    function scriptMsg(sel, text, okText) {
        var t = (text || '').trim();
        if (t.indexOf('error') === 0) { msg(sel, t, 'bad'); return false; }
        if (t.indexOf('warning') === 0) { msg(sel, t, 'warn'); return true; }
        msg(sel, okText || t.replace(/^OK\s*/, '') || 'Done', 'ok');
        return true;
    }

    WIZARD.applyHttps = function () {
        var on = $('#https_enabled').is(':checked') ? 1 : 0;
        var port = ($('#https_port').val() || '443').trim();
        var redirect = $('#https_redirect').is(':checked') ? 1 : 0;
        var source = WIZARD.httpsSourceTouched ? $('#https_source').val() : '';
        if (source === 'p12') source = 'upload';

        if (!/^\d+$/.test(port) || +port < 1 || +port > 65535) {
            msg('#https_msg', 'Port must be a whole number between 1 and 65535', 'bad'); return;
        }
        if (on && WIZARD.https && WIZARD.https.ssl_module !== 1) {
            msg('#https_msg', 'This nginx build has no SSL module', 'bad'); return;
        }

        msg('#https_msg', 'Applying…', 'busy');
        WIZARD.beginBusy();
        $.ajax({
            url: '/set_https_config?enabled=' + on + '&port=' + encodeURIComponent(port)
                 + '&redirect=' + redirect + '&source=' + encodeURIComponent(source),
            type: 'GET', timeout: 30000
        })
            .success(function (m) {
                if (!scriptMsg('#https_msg', m)) return;
                WIZARD.httpsDirty = false;

                /* A link, not a redirect: a self-signed certificate has to be
                   accepted first, and a jump would land on a browser error. */
                if (on && window.location.protocol !== 'https:') {
                    var url = 'https://' + window.location.hostname
                            + (port === '443' ? '' : ':' + port) + '/network_manager/';
                    $('#https_msg').append(' &mdash; <a href="' + esc(url) + '">open over https</a>');
                }
            })
            .error(function (x) { msg('#https_msg', (x.responseText || 'request failed').trim(), 'bad'); })
            .always(function () { WIZARD.endBusy(); WIZARD.loadHttps(true); });
    };

    WIZARD.genCert = function () {
        var cn = ($('#https_cn').val() || '').trim();
        var san = ($('#https_san').val() || '').replace(/\s+/g, '');
        var days = ($('#https_days').val() || '825').trim();

        if (cn && !/^[A-Za-z0-9.-]+$/.test(cn)) {
            msg('#https_gen_msg', 'The common name may only contain letters, digits, dots and hyphens', 'bad'); return;
        }
        if (san && !/^[A-Za-z0-9.,-]+$/.test(san)) {
            msg('#https_gen_msg', 'Additional names must be a comma separated list of host names or addresses', 'bad'); return;
        }
        if (!/^\d+$/.test(days) || +days < 1 || +days > 3650) {
            msg('#https_gen_msg', 'Validity must be between 1 and 3650 days', 'bad'); return;
        }

        msg('#https_gen_msg', 'Generating…', 'busy');
        WIZARD.beginBusy();
        $.ajax({
            url: '/gen_self_signed?cn=' + encodeURIComponent(cn)
                 + '&san=' + encodeURIComponent(san) + '&days=' + days,
            type: 'GET', timeout: 60000
        })
            .success(function (m) { scriptMsg('#https_gen_msg', m, 'Certificate generated'); })
            .error(function (x) { msg('#https_gen_msg', (x.responseText || 'request failed').trim(), 'bad'); })
            .always(function () { WIZARD.endBusy(); WIZARD.loadHttps(true); });
    };

    WIZARD.installCert = function () {
        var cert = $('#https_cert_pem').val() || '';
        var key = $('#https_key_pem').val() || '';

        if (cert.indexOf('-----BEGIN') < 0 || key.indexOf('-----BEGIN') < 0) {
            msg('#https_install_msg', 'Paste both PEM blocks, including their BEGIN and END lines', 'bad'); return;
        }

        msg('#https_install_msg', 'Installing…', 'busy');
        WIZARD.beginBusy();
        $.ajax({
            url: '/install_cert', type: 'POST', timeout: 30000,
            contentType: 'application/json',
            data: JSON.stringify({ cert: cert, key: key })
        })
            .success(function (m) {
                if (scriptMsg('#https_install_msg', m, 'Certificate installed')) {
                    /* The key is on the board now; no reason to keep it in the
                       DOM. */
                    $('#https_key_pem').val('');
                    $('#https_cert_pem').val('');
                }
            })
            .error(function (x) { msg('#https_install_msg', (x.responseText || 'request failed').trim(), 'bad'); })
            .always(function () { WIZARD.endBusy(); WIZARD.loadHttps(true); });
    };

    /* The answer is a binary attachment, so it is fetched rather than
       navigated to: a failed export must show its reason on the page instead of
       replacing it. */
    WIZARD.exportBundle = function () {
        var pw = $('#https_export_pw').val() || '';
        if (pw.length < 8) {
            msg('#https_export_dlg_msg', 'The password must be at least 8 characters', 'bad'); return;
        }

        msg('#https_export_dlg_msg', 'Packing…', 'busy');
        WIZARD.beginBusy();
        var xhr = new XMLHttpRequest();
        xhr.open('POST', '/export_bundle', true);
        xhr.setRequestHeader('Content-Type', 'application/json');
        xhr.responseType = 'blob';
        xhr.onload = function () {
            WIZARD.endBusy();
            if (xhr.status !== 200) {
                /* The failure belongs in the dialog: it is still open, and the
                   password that caused it is still in it. */
                var reader = new FileReader();
                reader.onload = function () { msg('#https_export_dlg_msg', (reader.result || '').trim(), 'bad'); };
                reader.readAsText(xhr.response);
                return;
            }
            var name = 'redpitaya-' + (window.location.hostname || 'board').split('.')[0] + '.p12';
            var url = URL.createObjectURL(xhr.response);
            var a = document.createElement('a');
            a.href = url; a.download = name;
            document.body.appendChild(a); a.click(); document.body.removeChild(a);
            URL.revokeObjectURL(url);
            msg('#https_export_msg', 'Saved as ' + name, 'ok');
            msg('#https_export_dlg_msg', '');
            $('#https_export_pw').val('');
            $('#https_export_dialog').modal('hide');
        };
        xhr.onerror = function () { WIZARD.endBusy(); msg('#https_export_dlg_msg', 'request failed', 'bad'); };
        xhr.send(JSON.stringify({ password: pw }));
    };

    WIZARD.importBundle = function () {
        var file = ($('#https_p12_file')[0].files || [])[0];
        var pw = $('#https_p12_pw').val() || '';
        if (!file) { msg('#https_import_msg', 'Choose the .p12 file first', 'bad'); return; }
        if (!pw) { msg('#https_import_msg', 'Enter the password the bundle was exported with', 'bad'); return; }

        msg('#https_import_msg', 'Importing…', 'busy');
        WIZARD.beginBusy();
        var reader = new FileReader();
        reader.onload = function () {
            /* The file is binary; JSON is not, hence base64. */
            var bytes = new Uint8Array(reader.result), bin = '';
            for (var i = 0; i < bytes.length; i++) bin += String.fromCharCode(bytes[i]);

            $.ajax({
                url: '/install_bundle', type: 'POST', timeout: 30000,
                contentType: 'application/json',
                data: JSON.stringify({ p12: btoa(bin), password: pw })
            })
                .success(function (m) {
                    if (scriptMsg('#https_import_msg', m, 'Certificate installed')) {
                        $('#https_p12_pw').val('');
                        $('#https_p12_file').val('');
                    }
                })
                .error(function (x) { msg('#https_import_msg', (x.responseText || 'request failed').trim(), 'bad'); })
                .always(function () { WIZARD.endBusy(); WIZARD.loadHttps(true); });
        };
        reader.onerror = function () { WIZARD.endBusy(); msg('#https_import_msg', 'Could not read the file', 'bad'); };
        reader.readAsArrayBuffer(file);
    };

}(window.WIZARD = window.WIZARD || {}, jQuery));


$(document).ready(function () {

    if (window.Help && window.helpListNM) { Help.init(helpListNM); Help.setState('idle'); }
    $('body').addClass('loaded');

    WIZARD.poll();
    WIZARD.loadEth();
    WIZARD.loadCountry();
    WIZARD.applyEthMode();
    WIZARD.bindReveal();
    setInterval(WIZARD.poll, 2000);
    setInterval(WIZARD.loadEth, 5000);

    /* Whether the Diagnostic WiFi entry belongs in the rail is only knowable
       after the checks have run, so run them once on load. Deferred a little:
       get_diag shells out to dmesg and systemctl, and nginx has one worker, so
       running it immediately would stall the first status render. */
    setTimeout(function () { WIZARD.loadDiag(true); }, 2500);

    $('#wlan0_country').change(function () { WIZARD.applyCountry($(this).val()); });

    $('#scan_btn').click(WIZARD.startScan);
    $('#diag_refresh').click(function () { WIZARD.loadDiag(); });
    $('#wl_connect').click(WIZARD.connect);
    $('#wl_disconnect').click(WIZARD.disconnect);
    $('#ap_addr,#ap_offset,#ap_size').on('input', WIZARD.showApPool);
    $('#ap_start').click(WIZARD.startAP);
    $('#ap_stop').click(WIZARD.stopAP);

    /* Any edit in the wired form makes it the user's, so revisiting the section
       stops overwriting it from the board. */
    var ETH_INPUTS = '#eth_mode,#eth_addr,#eth_mask,#eth_gw,#eth_dns,'
                   + '#dhcpd_offset,#dhcpd_size,#dhcpd_lease,#dhcpd_router,#dhcpd_dns';
    $(ETH_INPUTS).on('input change', function () { WIZARD.ethDirty = true; });

    $('#eth_mode').change(WIZARD.applyEthMode);
    $('#eth_addr,#dhcpd_offset,#dhcpd_size').on('input', WIZARD.showPool);
    $('#eth_apply').click(WIZARD.applyEth);

    /* Reset means "back to what the board is running", not "blank". Blanking
       was of no use to anyone: the fields it emptied then had to be retyped
       from the very configuration the page can read. */
    $('#eth_reset').click(function () {
        $('#eth_msg').attr('class', 'nm-msg').text('');
        WIZARD.fillEthForm(true);
        if (!WIZARD.ethConfig || !WIZARD.ethConfig.mode) {
            $('#eth_addr,#eth_mask,#eth_gw,#eth_dns').val('');
            $('#dhcpd_offset,#dhcpd_size,#dhcpd_lease,#dhcpd_router,#dhcpd_dns').val('');
            WIZARD.ethDirty = false;
            WIZARD.applyEthMode();
        }
    });

    var HTTPS_INPUTS = '#https_enabled,#https_port,#https_redirect,#https_source';
    $(HTTPS_INPUTS).on('input change', function () { WIZARD.httpsDirty = true; });

    $('#https_source').change(function () {
        WIZARD.httpsSourceTouched = true;
        WIZARD.applyHttpsSource();
    });
    $('#https_apply').click(WIZARD.applyHttps);
    $('#https_gen').click(WIZARD.genCert);
    $('#https_install').click(WIZARD.installCert);
    $('#https_export').click(function () {
        if ($(this).prop('disabled')) return;
        $('#https_export_pw').val('');
        $('#https_export_msg').attr('class', 'nm-msg').text('');
        $('#https_export_dlg_msg').attr('class', 'nm-msg').text('');
        $('#https_export_dialog').modal('show');
    });
    $('#https_export_confirm').click(function (e) { e.preventDefault(); WIZARD.exportBundle(); });
    $('#https_import').click(WIZARD.importBundle);

    /* A navigation, not an ajax call: the response is an attachment, so the
       browser saves it and the page stays where it is. */
    $('#https_download').click(function () {
        if ($(this).prop('disabled')) return;
        $('#https_download_msg').attr('class', 'nm-msg ok').text('Saved');
        window.location = '/download_cert';
    });
    /* Fetched once on load for the rail dot, deferred for the same reason as
       the diagnostics: one nginx worker, and this shells out to openssl. */
    setTimeout(function () { WIZARD.loadHttps(true); }, 3000);

    /* Sorting: same column toggles direction, a new column starts descending
       for signal and ascending for text. */
    $('#scan_table thead th[data-k]').click(function () {
        var k = $(this).data('k');
        if (WIZARD.sortKey === k) WIZARD.sortAsc = !WIZARD.sortAsc;
        else { WIZARD.sortKey = k; WIZARD.sortAsc = (k !== 'sig'); }
        $('#scan_table thead th').removeClass('sorted asc');
        $(this).addClass('sorted').toggleClass('asc', WIZARD.sortAsc);
        WIZARD.renderScan();
    });

    /* A scan row fills the wireless form and moves there, rather than
       connecting straight away: the passphrase is still needed. */
    $('#scan_body').on('click', 'tr', function () {
        var ssid = $(this).data('ssid');
        if (!ssid) return;
        $('#scan_body tr').removeClass('sel');
        $(this).addClass('sel');
        $('#wl_ssid').val(ssid);
        if ($(this).data('open') === 1 || $(this).data('open') === '1') {
            $('#wl_keymgmt').val('open');
            $('#wl_pass').val('');
        } else if ($('#wl_keymgmt').val() === 'open') {
            $('#wl_keymgmt').val('auto');
        }
        WIZARD.showTab('nm_wireless');
        $('#wl_pass').focus();
    });

    /* file:// and IP-only access give no hostname; an empty pill is just a box. */
    var host = window.location.hostname;
    if (host) $('#pill_host').text(host); else $('#pill_host').hide();
});
