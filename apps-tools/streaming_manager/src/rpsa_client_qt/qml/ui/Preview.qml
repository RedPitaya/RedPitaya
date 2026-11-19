import QtQuick

// The signal of a board: what ChartDataHolder (cdh) made of a block of the stream, drawn from
// -1 to 1 (the full range of the inputs). The holder takes a block of every running board at once
// and thins it out; the view takes it when it is new and lets the holder know.
// With fit the Y range follows the signal (1-2-5 steps around zero): it grows at once and
// shrinks only after the signal stayed small for a second, so the picture does not jump.
// Axes (streaming_improvements 4.1): volts of the range of each input on the left (a column of
// labels for each range; one input's column in its colour), the time from the start of the block
// at the bottom, both in 1-2-5 steps. A low preview (the list) has none: they would cover it.
Canvas {
    id: root
    property var board
    property int channels: 2
    property bool grid: true
    property bool active: true
    property var points: [[], [], [], []]
    property bool hasData: false
    property bool fit: false
    // samples of the stream per point (0: the whole block on the width), the rate of a channel
    // and the samples of a block: the time the width shows
    property int decimation: 1
    property double sampleRate: 0
    property int blockSamples: 0
    property int shownPoints: 0
    // volts of the full range of IN1..IN4 (1:1 1 V, 1:20 20 V, a board of one range its own) and
    // the inputs on: the labels of the Y axis
    property var fullScales: [1, 1, 1, 1]
    property var inputsOn: [true, true, false, false]
    // the spectrum instead of the time (streaming_improvements 4.5): the FFT of the whole block,
    // made by the holder (cdh.getSpectrum), in dBV of the range of each input
    property bool spectrum: false
    property var spec: ({})
    onSpectrumChanged: { spec = ({}); hasData = false; requestPaint() }
    readonly property double windowSec: sampleRate <= 0 ? 0
        : (decimation > 0 ? shownPoints * decimation : blockSamples) / sampleRate
    function timeText(t) {
        if (t <= 0) return ""
        if (t < 1e-3) return +(t * 1e6).toPrecision(3) + " µs"
        if (t < 1) return +(t * 1e3).toPrecision(3) + " ms"
        return +t.toPrecision(3) + " s"
    }
    // the shown half range, of the full range of the inputs (1: -1 .. 1)
    property real range: 1
    property double smallSince: 0
    readonly property string scaleText: range >= 1 ? "Full" : "×" + +(1 / range).toFixed(0)
    onFitChanged: { range = 1; smallSince = 0; requestPaint() }
    onFullScalesChanged: requestPaint()
    onInputsOnChanged: requestPaint()

    // the inputs of each range shown: [{fs, ins: [0, 1]}], the first range first
    readonly property var groups: {
        const g = []
        for (let c = 0; c < channels; c++) {
            if (!inputsOn[c]) continue
            const fs = fullScales[c] || 1
            const found = g.find(x => x.fs === fs)
            if (found) found.ins.push(c)
            else g.push({ fs: fs, ins: [c] })
        }
        if (g.length === 0) g.push({ fs: fullScales[0] || 1, ins: [] })
        return g
    }
    // "±1 V (IN1, IN2)": what the Y range is now, for a tooltip
    readonly property string rangeText: groups.map(x => "±" + voltText(x.fs * (fit ? range : 1), true)
        + (x.ins.length ? " (" + x.ins.map(c => "IN" + (c + 1)).join(", ") + ")" : "")).join(", ")

    // the label colours of IN1..IN4: the trace colours, IN2 lighter (its green is too dark for text)
    readonly property var labelColors: ["#f3ec1a", "#5fcf72", "#e0a060", "#e07ddc"]

    // 1-2-5: the smallest step of at least min
    function niceStep(min) {
        if (!(min > 0)) return 1
        const p = Math.pow(10, Math.floor(Math.log10(min)))
        for (const m of [1, 2, 5, 10])
            if (m * p >= min * 0.9999) return m * p
        return 10 * p
    }
    // volts as a label: mV below 0.2 V of range, the unit only where asked
    function voltText(v, unit, mv) {
        const useMv = mv === undefined ? Math.abs(v) < 0.2 && v !== 0 : mv
        const x = useMv ? v * 1000 : v
        const s = (Math.abs(x) < 1e-9 ? "0" : String(+x.toPrecision(3))).replace("-", "−")
        return s + (unit ? (useMv ? " mV" : " V") : "")
    }

    function fitRange(p) {
        let peak = 0
        for (let c = 0; c < p.length; c++)
            for (let i = 0; i < p[c].length; i++) peak = Math.max(peak, Math.abs(p[c][i]))
        const steps = [0.001, 0.002, 0.005, 0.01, 0.02, 0.05, 0.1, 0.2, 0.5, 1]
        let want = 1
        for (const r of steps) if (peak * 1.1 <= r) { want = r; break }
        const now = Date.now()
        if (want >= range) { range = want; smallSince = 0 }
        else if (smallSince === 0) smallSince = now
        else if (now - smallSince > 1000) { range = want; smallSince = 0 }
    }

    function refresh() {
        if (!board || !active) return
        if (!cdh.getChartNeedUpdate(board.ip)) return
        if (spectrum) {
            spec = cdh.getSpectrum(board.ip)
            cdh.clearChartBuffer(board.ip)
            hasData = !!(spec.points && spec.points.some(a => a.length > 0))
            requestPaint()
            return
        }
        const p = []
        for (let i = 0; i < 4; i++) p.push(i < channels ? cdh.getChartSignal(board.ip, i) : [])
        cdh.clearChartBuffer(board.ip)
        points = p
        hasData = p.some(a => a.length > 0)
        shownPoints = Math.max(...p.map(a => a.length))
        if (fit && hasData) fitRange(p)
        requestPaint()
    }

    Timer {
        interval: 50
        running: root.visible && root.active
        repeat: true
        onTriggered: root.refresh()
    }
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    onWindowSecChanged: requestPaint()

    readonly property bool axes: grid && height >= 80
    readonly property int fontPx: Theme.fsXs

    // the spectrum: dBV (1 V RMS) on the left in 20 dB steps over 140 dB, 0 to half the sample
    // rate at the bottom, the highest peak of each input on a plate
    function paintSpectrum(ctx) {
        ctx.font = fontPx + "px " + Theme.mono.split(",")[0]
        const on = []
        for (let c = 0; c < channels; c++)
            if (inputsOn[c] && spec.points && spec.points[c] && spec.points[c].length > 1) on.push(c)
        const offs = [0, 1, 2, 3].map(c => 20 * Math.log10(fullScales[c] || 1))
        let topDb = 0
        for (const c of (on.length ? on : [0])) topDb = Math.max(topDb, Math.ceil(offs[c] / 20 - 1e-9) * 20)
        const botDb = topDb - 140
        const labelH = fontPx + 6
        const top = Math.ceil(fontPx / 2) + 1
        const ph = height - top - labelH - 2
        const labels = []
        let wmax = 0
        for (let db = topDb; db >= botDb; db -= 20) {
            const t = (db === 0 ? "0" : String(db).replace("-", "−")) + (db === topDb || db === botDb ? " dBV" : "")
            labels.push({ db: db, t: t })
            wmax = Math.max(wmax, ctx.measureText(t).width)
        }
        const left = Math.ceil(wmax) + 10
        const w = width - left - 4
        const yOf = (db) => top + (topDb - db) / 140 * ph
        const half = sampleRate / 2
        // the grid and the frame
        ctx.strokeStyle = Qt.rgba(153 / 255, 153 / 255, 153 / 255, 0.18)
        ctx.lineWidth = 1
        ctx.beginPath()
        for (let db = topDb - 20; db > botDb; db -= 20) { const y = Math.round(yOf(db)) + 0.5; ctx.moveTo(left, y); ctx.lineTo(left + w, y) }
        let fStep = 0
        if (half > 0) {
            fStep = niceStep(half * 80 / Math.max(1, w))
            const fGrid = fStep / 2 * w / half >= 30 ? fStep / 2 : fStep
            for (let f = fGrid; f < half - 1e-9; f += fGrid) { const x = Math.round(left + f / half * w) + 0.5; ctx.moveTo(x, top); ctx.lineTo(x, top + ph) }
        }
        ctx.stroke()
        ctx.strokeStyle = Qt.rgba(153 / 255, 153 / 255, 153 / 255, 0.45)
        ctx.beginPath(); ctx.moveTo(left + 0.5, top); ctx.lineTo(left + 0.5, top + ph); ctx.lineTo(left + w, top + ph); ctx.stroke()
        // the labels
        ctx.fillStyle = Theme.muted
        ctx.textAlign = "right"; ctx.textBaseline = "middle"
        for (const l of labels) ctx.fillText(l.t, left - 6, yOf(l.db))
        const unit = half >= 1e6 ? 1e6 : half >= 1e3 ? 1e3 : 1
        const unitText = unit === 1e6 ? " MHz" : unit === 1e3 ? " kHz" : " Hz"
        if (half > 0) {
            ctx.textBaseline = "top"
            const n = Math.floor(half / fStep + 1e-9)
            for (let i = 0; i <= n; i++) {
                const tx = left + i * fStep / half * w
                const s = String(+(i * fStep / unit).toPrecision(3)) + (i === n ? unitText : "")
                ctx.textAlign = i === 0 ? "left" : (tx + ctx.measureText(s).width / 2 > left + w ? "right" : "center")
                ctx.fillText(s, i === 0 ? tx + 2 : Math.min(tx, left + w), top + ph + 3)
            }
        }
        // the traces
        ctx.save()
        ctx.beginPath(); ctx.rect(left, top, w, ph); ctx.clip()
        ctx.lineWidth = 1.2
        for (const c of on) {
            const pts = spec.points[c]
            ctx.strokeStyle = Theme.channelColors[c]
            ctx.beginPath()
            const n = pts.length
            for (let i = 0; i < n; i++) {
                const x = left + i * w / (n - 1)
                const y = yOf(Math.max(botDb - 10, pts[i] + offs[c]))
                if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y)
            }
            ctx.stroke()
        }
        ctx.restore()
        // the peaks
        if (on.length && spec.n > 0 && sampleRate > 0) {
            const lines = on.map(c => {
                const f = spec.peakBin[c] * sampleRate / spec.n
                const fu = f >= 1e6 ? (f / 1e6).toFixed(3) + " MHz" : f >= 1e3 ? (f / 1e3).toFixed(2) + " kHz" : f.toFixed(1) + " Hz"
                return { c: c, t: "IN" + (c + 1) + " peak " + fu + "  " + (spec.peakDb[c] + offs[c]).toFixed(1).replace("-", "−") + " dBV" }
            })
            let pw = 0
            for (const l of lines) pw = Math.max(pw, ctx.measureText(l.t).width)
            const bx = left + w - pw - 18, by = top + 4, lh = fontPx + 4
            ctx.fillStyle = Theme.plot
            ctx.strokeStyle = Theme.borderSoft
            ctx.fillRect(bx, by, pw + 14, lines.length * lh + 6)
            ctx.strokeRect(bx + 0.5, by + 0.5, pw + 13, lines.length * lh + 5)
            ctx.textAlign = "left"; ctx.textBaseline = "top"
            lines.forEach((l, i) => { ctx.fillStyle = labelColors[l.c]; ctx.fillText(l.t, bx + 7, by + 4 + i * lh) })
        }
    }

    onPaint: {
        const ctx = getContext("2d")
        ctx.reset()
        if (spectrum) {
            paintSpectrum(ctx)
            return
        }
        const k = fit ? 1 / range : 1
        ctx.font = fontPx + "px " + Theme.mono.split(",")[0]
        // the plot: the left of the label columns, above the time labels
        let left = 0
        const cols = []
        const labelH = fontPx + 6
        const bottom = axes ? labelH + 2 : 0
        const ph = height - bottom - (axes ? Math.ceil(fontPx / 2) + 1 : 0)
        const top = axes ? Math.ceil(fontPx / 2) + 1 : 0
        let yStep = 0.5, yGrid = 0.25
        if (axes) {
            // the label step of the first range: 40 px at least between labels
            const g0 = groups[0]
            const half0 = g0.fs / k
            yStep = niceStep(2 * half0 * 40 / Math.max(1, ph))
            yGrid = yStep / 2 * ph / (2 * half0) >= 18 ? yStep / 2 : yStep
            for (const g of groups) {
                const half = g.fs / k
                const step = yStep * g.fs / g0.fs
                const mv = half < 0.2
                const labels = []
                let wmax = 0
                for (let v = Math.floor(half / step) * step; v >= -half - step * 1e-6; v -= step) {
                    const atEnd = Math.abs(Math.abs(v) - half) < step * 1e-6 || Math.abs(v) + step > half + step * 1e-6
                    const t = voltText(v, atEnd && v !== 0, mv)
                    labels.push({ v: v, t: t })
                    wmax = Math.max(wmax, ctx.measureText(t).width)
                }
                const color = g.ins.length === 1 ? labelColors[g.ins[0]] : Theme.muted
                cols.push({ g: g, half: half, labels: labels, w: Math.ceil(wmax) + 10, color: color })
                left += Math.ceil(wmax) + 10
            }
        }
        const w = width - left - (axes ? 4 : 0)
        const yOf = (frac) => top + ph / 2 - frac * (ph / 2 - (axes ? 0 : 2))
        // the grid: the first range's volts, the time steps
        if (grid) {
            ctx.strokeStyle = Qt.rgba(153 / 255, 153 / 255, 153 / 255, 0.18)
            ctx.lineWidth = 1
            ctx.beginPath()
            if (axes) {
                const half0 = groups[0].fs / k
                for (let v = Math.floor(half0 / yGrid) * yGrid; v >= -half0 - 1e-9; v -= yGrid) {
                    if (Math.abs(v) < 1e-9) continue
                    const y = Math.round(yOf(v / half0)) + 0.5
                    ctx.moveTo(left, y); ctx.lineTo(left + w, y)
                }
            } else {
                for (let j = 1; j < 8; j++) { const y = Math.round(ph * j / 8) + 0.5; ctx.moveTo(0, y); ctx.lineTo(w, y) }
            }
            let tStep = 0, tGrid = 0
            if (axes && windowSec > 0) {
                tStep = niceStep(windowSec * 80 / Math.max(1, w))
                tGrid = tStep / 2 * w / windowSec >= 30 ? tStep / 2 : tStep
                for (let t = tGrid; t < windowSec - 1e-12; t += tGrid) {
                    const x = Math.round(left + t / windowSec * w) + 0.5
                    ctx.moveTo(x, top); ctx.lineTo(x, top + ph)
                }
            } else if (!axes) {
                for (let i = 1; i < 10; i++) { const x = Math.round(w * i / 10) + 0.5; ctx.moveTo(x, 0); ctx.lineTo(x, ph) }
            }
            ctx.stroke()
            if (axes) {
                // the frame of the axes and the labels
                ctx.strokeStyle = Qt.rgba(153 / 255, 153 / 255, 153 / 255, 0.45)
                ctx.beginPath()
                ctx.moveTo(left + 0.5, top); ctx.lineTo(left + 0.5, top + ph)
                ctx.lineTo(left + w, top + ph)
                ctx.stroke()
                ctx.textBaseline = "middle"
                ctx.textAlign = "right"
                let x = 0
                for (const c of cols) {
                    x += c.w
                    ctx.fillStyle = c.color
                    for (const l of c.labels)
                        ctx.fillText(l.t, x - 6, yOf(l.v / c.half))
                }
                if (windowSec > 0) {
                    ctx.fillStyle = Theme.muted
                    ctx.textBaseline = "top"
                    const unit = windowSec < 1e-3 ? 1e6 : windowSec < 1 ? 1e3 : 1
                    const unitText = unit === 1e6 ? " µs" : unit === 1e3 ? " ms" : " s"
                    const n = Math.floor(windowSec / tStep + 1e-9)
                    for (let i = 0; i <= n; i++) {
                        const t = i * tStep
                        const tx = left + t / windowSec * w
                        const s = String(+(t * unit).toPrecision(3)) + (i === n ? unitText : "")
                        ctx.textAlign = i === 0 ? "left" : (tx + ctx.measureText(s).width / 2 > left + w ? "right" : "center")
                        ctx.fillText(s, i === 0 ? tx + 2 : Math.min(tx, left + w), top + ph + 3)
                    }
                }
            }
        }
        ctx.strokeStyle = Qt.rgba(153 / 255, 153 / 255, 153 / 255, 0.45)
        ctx.beginPath(); ctx.moveTo(left, Math.round(yOf(0)) + 0.5); ctx.lineTo(left + w, Math.round(yOf(0)) + 0.5); ctx.stroke()
        ctx.lineWidth = 1.4
        ctx.save()
        ctx.beginPath(); ctx.rect(left, top, w, ph); ctx.clip()
        for (let c = 0; c < 4; c++) {
            const pts = points[c]
            if (!pts || pts.length < 2) continue
            ctx.strokeStyle = Theme.channelColors[c]
            ctx.beginPath()
            const n = pts.length
            for (let i = 0; i < n; i++) {
                const x = left + i * w / (n - 1)
                const v = Math.max(-1, Math.min(1, pts[i] * k))
                const y = yOf(v)
                if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y)
            }
            ctx.stroke()
        }
        ctx.restore()
    }
}
