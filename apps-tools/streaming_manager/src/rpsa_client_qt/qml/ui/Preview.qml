import QtQuick

// The signal of a board: what ChartDataHolder (cdh) made of a block of the stream, drawn from
// -1 to 1 (the full range of the inputs). The holder takes a block of every running board at once
// and thins it out; the view takes it when it is new and lets the holder know.
// With fit the Y range follows the signal (1-2-5 steps around zero): it grows at once and
// shrinks only after the signal stayed small for a second, so the picture does not jump.
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

    onPaint: {
        const ctx = getContext("2d")
        ctx.reset()
        const w = width, h = height
        if (grid) {
            ctx.strokeStyle = Qt.rgba(153 / 255, 153 / 255, 153 / 255, 0.18)
            ctx.lineWidth = 1
            ctx.beginPath()
            for (let i = 1; i < 10; i++) { const x = Math.round(w * i / 10) + 0.5; ctx.moveTo(x, 0); ctx.lineTo(x, h) }
            for (let j = 1; j < 8; j++) { const y = Math.round(h * j / 8) + 0.5; ctx.moveTo(0, y); ctx.lineTo(w, y) }
            ctx.stroke()
        }
        ctx.strokeStyle = Qt.rgba(153 / 255, 153 / 255, 153 / 255, 0.45)
        ctx.beginPath(); ctx.moveTo(0, Math.round(h / 2) + 0.5); ctx.lineTo(w, Math.round(h / 2) + 0.5); ctx.stroke()
        const k = fit ? 1 / range : 1
        ctx.lineWidth = 1.4
        for (let c = 0; c < 4; c++) {
            const pts = points[c]
            if (!pts || pts.length < 2) continue
            ctx.strokeStyle = Theme.channelColors[c]
            ctx.beginPath()
            const n = pts.length
            for (let i = 0; i < n; i++) {
                const x = i * w / (n - 1)
                const v = Math.max(-1, Math.min(1, pts[i] * k))
                const y = h / 2 - v * (h / 2 - 2)
                if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y)
            }
            ctx.stroke()
        }
    }

    // the time across (top right) and the scale while Y is fitted (top left), on a plate
    // too low a preview (the list) shows no plates: they would hide the signal
    readonly property bool tags: height >= 80
    component Tag: Rectangle {
        property alias text: t.text
        y: 3
        width: t.implicitWidth + 8
        height: t.implicitHeight + 2
        radius: 3
        color: Theme.plot
        border.color: Theme.borderSoft
        Text { id: t; anchors.centerIn: parent; color: Theme.muted; font.pixelSize: Theme.fsXs; font.family: Theme.mono }
    }
    Tag {
        visible: root.tags && root.hasData && root.windowSec > 0
        x: root.width - width - 4
        text: root.timeText(root.windowSec)
    }
    Tag {
        visible: root.tags && root.fit && root.hasData
        x: 4
        text: "Y " + root.scaleText
    }
}
