import QtQuick

// The signal of a board: what ChartDataHolder (cdh) made of a block of the stream, drawn from
// -1 to 1 (the full range of the inputs). The holder takes a block of every running board at once
// and thins it out; the view takes it when it is new and lets the holder know.
Canvas {
    id: root
    property var board
    property int channels: 2
    property bool grid: true
    property bool active: true
    property var points: [[], [], [], []]
    property bool hasData: false

    function refresh() {
        if (!board || !active) return
        if (!cdh.getChartNeedUpdate(board.ip)) return
        const p = []
        for (let i = 0; i < 4; i++) p.push(i < channels ? cdh.getChartSignal(board.ip, i) : [])
        cdh.clearChartBuffer(board.ip)
        points = p
        hasData = p.some(a => a.length > 0)
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
        ctx.lineWidth = 1.4
        for (let c = 0; c < 4; c++) {
            const pts = points[c]
            if (!pts || pts.length < 2) continue
            ctx.strokeStyle = Theme.channelColors[c]
            ctx.beginPath()
            const n = pts.length
            for (let i = 0; i < n; i++) {
                const x = i * w / (n - 1)
                const v = Math.max(-1, Math.min(1, pts[i]))
                const y = h / 2 - v * (h / 2 - 2)
                if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y)
            }
            ctx.stroke()
        }
    }
}
