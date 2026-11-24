import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import RpsaClient
import "ui"

// A recording (BIN, WAV, TDMS, CSV) in a window of its own (streaming_improvements 4.7, the
// viewer): the samples of a window of it with the losses and the trigger waits marked, the whole
// of it below, a selection to export, marks found at a level or set by hand and their table. The
// window closes when the file goes. What keeps it waiting longer than a second shows a dialog
// with its progress. Positions are samples on the time line.
ApplicationWindow {
    id: win
    objectName: "viewer"
    property string path: ""
    property string board: ""

    width: 1280
    height: 820
    minimumWidth: 860
    minimumHeight: 560
    visible: true
    color: Theme.bg
    title: (info.name || path.split(/[\\/]/).pop()) + " — Red Pitaya streaming client"

    RecordingView { id: rv }
    Component.onCompleted: rv.open(path)
    // closed by the user while an export runs: asked first; closed because the file goes: at once
    property bool force: false
    function closeNow() { force = true; close() }
    property bool closed: false
    onClosing: (close) => {
        // once (a close asked again while it goes)
        if (closed) return
        if (rv.exporting.running && !force) {
            close.accepted = false
            stopDialog.open()
            return
        }
        // the file closed now (Windows deletes no open file), not when the window is destroyed
        closed = true
        rv.release()
        gone()
    }
    // it closes indeed (closing is also sent when the close is refused)
    signal gone()
    // the parts of a recording in parts (streaming_improvements 4.11): the one before or after
    // opens in a window of its own (Recordings.qml)
    signal openPart(string path)
    readonly property var partList: {
        const me = recordings.list.find(r => r.path === win.path)
        return me && me.partOf ? recordings.list.filter(r => r.partOf === me.partOf).sort((a, b) => a.part - b.part) : []
    }
    readonly property int partIdx: partList.findIndex(r => r.path === win.path)

    readonly property var info: rv.info
    readonly property double total: info.total || 0
    readonly property double rate: info.rate || 0
    readonly property var inputs: info.inputs || []
    readonly property bool ready: !!info.ready
    readonly property var busy: rv.busy
    // the marks, and where ◀ ▶ go: the losses or the marks
    property string navMode: "losses"
    property int markIdx: -1
    function markName(i) { return markList[i] && markList[i].name ? markList[i].name : "M" + (i + 1) }
    function goMark(i) {
        if (i < 0 || i >= markList.length) return
        markIdx = i
        navMode = "marks"
        const span = (to - from) < total / 4 ? (to - from) : Math.min(total, 4000)
        setWindow(markList[i].pos - span / 2, markList[i].pos + span / 2)
        table.showRow("marks", i)
    }
    function nextMark(dir) {
        if (!markList.length) return
        if (markIdx >= 0 && markIdx < markList.length && Math.abs(markList[markIdx].pos - (from + to) / 2) <= (to - from) / 2) { goMark(Math.max(0, Math.min(markList.length - 1, markIdx + dir))); return }
        const c = (from <= 0 && to >= total) ? (dir > 0 ? -1 : total + 1) : (from + to) / 2
        if (dir > 0) { const i = markList.findIndex(m => m.pos > c + 1); goMark(i >= 0 ? i : markList.length - 1) }
        else { let i = -1; for (let k = 0; k < markList.length; k++) if (markList[k].pos < c - 1) i = k; goMark(i >= 0 ? i : 0) }
    }
    function goNext(dir) { if (navMode === "marks") nextMark(dir); else nextLoss(dir) }
    // the level of the find, drawn while it is given
    property double findLevel: NaN
    property bool showTable: true
    // id -> false: the input is not drawn
    property var hidden: ({})
    // the window shown
    property double from: 0
    property double to: 0
    onTotalChanged: { from = 0; to = total }
    // the selection: [selFrom, selTo)
    property double selFrom: 0
    property double selTo: 0
    readonly property bool hasSel: selTo > selFrom
    property bool fitY: false
    // the BIN has the codes, not the gain: the volts of the full scale (1 or 20)
    property double fullScale: 1
    property var shown: ({})
    // plain arrays (see RecordingView::marksList)
    property var gapList: []
    property var markList: []
    Connections {
        target: rv
        function onOverviewChanged() { win.gapList = rv.gapsList() }
        function onMarksChanged() { win.markList = rv.marksList() }
    }
    readonly property var losses: gapList.filter(g => g.lost > 0)
    property int lossIdx: -1
    property string message: ""
    property bool messageBad: false

    // the file went (deleted here, in the list or by another program): the view of it closes
    Timer { interval: 1000; running: true; repeat: true; onTriggered: if (win.path !== "" && !rv.exists()) win.closeNow() }

    // the samples of the window again, once for all that changed
    property bool pending: false
    function refresh() {
        if (pending) return
        pending = true
        Qt.callLater(() => {
            pending = false
            shown = rv.window(from, to, Math.max(1, Math.round(plot.width)))
            plot.requestPaint()
        })
    }
    onFromChanged: refresh()
    onToChanged: refresh()
    Connections {
        target: rv
        function onOverviewChanged() { win.refresh(); strip.requestPaint() }
    }

    readonly property double minSpan: Math.min(16, total)
    function setWindow(a, b) {
        let span = Math.max(minSpan, Math.min(total, b - a))
        a = Math.max(0, Math.min(total - span, a))
        from = a
        to = a + span
    }
    function zoom(f, at) {
        const c = at === undefined ? 0.5 : at
        const p = from + (to - from) * c
        const span = (to - from) * f
        setWindow(p - span * c, p - span * c + span)
    }
    function goLoss(i) {
        if (i < 0 || i >= losses.length) return
        lossIdx = i
        navMode = "losses"
        table.showRow("losses", i)
        const g = losses[i]
        const len = g.lost + g.wait
        const span = Math.max(len * 6, 2000)
        setWindow(g.at + len / 2 - span / 2, g.at + len / 2 + span / 2)
    }
    function nextLoss(dir) {
        if (!losses.length) return
        // from the whole recording: the first or the last; else the next from the middle of the window
        const whole = from <= 0 && to >= total
        const c = whole ? (dir > 0 ? -1 : total + 1) : (from + to) / 2
        if (dir > 0) {
            const i = losses.findIndex(g => g.at + (g.lost + g.wait) / 2 > c + 1)
            goLoss(i >= 0 ? i : losses.length - 1)
        } else {
            let i = -1
            for (let k = 0; k < losses.length; k++) if (losses[k].at + (losses[k].lost + losses[k].wait) / 2 < c - 1) i = k
            goLoss(i >= 0 ? i : 0)
        }
    }

    // times: m:ss.fff on the time line of the recording (samples when the rate is not known)
    function secs(p) { return rate > 0 ? p / rate : p }
    function fmtT(p, d) {
        if (!(rate > 0)) return String(Math.round(p))
        const s = p / rate
        const m = Math.floor(s / 60)
        const r = (s - m * 60).toFixed(d)
        return m + ":" + r.padStart(d > 0 ? d + 3 : 2, "0")
    }
    function digits(span) {
        const sp = secs(span)
        return Math.max(0, Math.min(9, Math.ceil(-Math.log10(Math.max(sp, 1e-9) / 5)) + 1))
    }
    function parseT(text) {
        let s = text.trim()
        let m = 0
        const i = s.indexOf(":")
        if (i >= 0) { m = Number(s.slice(0, i)); s = s.slice(i + 1) }
        const v = Number(s)
        if (s === "" || isNaN(v) || isNaN(m) || v < 0 || m < 0) return NaN
        return rate > 0 ? Math.round((m * 60 + v) * rate) : Math.round(v)
    }
    function dur(n) {
        if (!(rate > 0)) return Math.round(n) + " samples"
        const s = n / rate
        if (s < 1e-3) return (+(s * 1e6).toFixed(s < 1e-5 ? 3 : 1)) + " µs"
        if (s < 1) return (+(s * 1e3).toFixed(s < 1e-2 ? 3 : 1)) + " ms"
        if (s < 60) return (+s.toFixed(3)) + " s"
        if (s < 3600) return Math.floor(s / 60) + " min " + Math.round(s % 60) + " s"
        return Math.floor(s / 3600) + " h " + Math.floor(s % 3600 / 60) + " min"
    }
    function color(id) { return Theme.channelColors[(id - 1 + 4) % 4] }
    function say(text, bad) { message = text; messageBad = bad }

    // the steps of the UI script (main.qml: view:<step>): zoom:<s>:<s> | sel:<s>:<s> | loss | fit |
    // menu | export | exportgo:<format> | stop (the export) | close | delete | find:<level V>:<edge> |
    // mark (in the middle of the window) | nextmark | tab:marks|losses | reopen (release and open, as
    // after a delete that failed)
    function script(p) {
        if (p[0] === "zoom") setWindow(Number(p[1]) * rate, Number(p[2]) * rate)
        else if (p[0] === "sel") { selFrom = Number(p[1]) * rate; selTo = Number(p[2]) * rate }
        else if (p[0] === "loss") nextLoss(1)
        else if (p[0] === "fit") fitY = true
        else if (p[0] === "menu") fileMenu.popupFor(fileBtn)
        else if (p[0] === "export") exportDialog.ask()
        else if (p[0] === "exportgo") { exportDialog.ask(); exportDialog.format = p[1] || "CSV"; exportDialog.go() }
        else if (p[0] === "delete") deleteDialog.open()
        else if (p[0] === "stop") rv.stopExport()
        else if (p[0] === "find") { searchBar.level = p[1] || "0"; searchBar.edge = p[2] || "rising"; searchBar.go() }
        else if (p[0] === "mark") rv.addMark((from + to) / 2, inputs[0] || 0)
        else if (p[0] === "nextmark") nextMark(1)
        else if (p[0] === "tab") table.tab = p[1] || "marks"
        else if (p[0] === "reopen") { rv.release(); rv.open(win.path) }
        else if (p[0] === "close") win.close()
    }

    // the range of the values: the full scale, or what the window has
    readonly property var yRange: {
        const vs = info.volts ? 1 : fullScale
        if (!fitY || !shown.channels) {
            if (!info.volts) return [-vs, vs]
            let m = 0.01
            const o = rv.overview
            for (const k of (o.lo || [])) for (const v of k) m = Math.max(m, Math.abs(v))
            for (const k of (o.hi || [])) for (const v of k) m = Math.max(m, Math.abs(v))
            return [-m, m]
        }
        let lo = Infinity, hi = -Infinity
        for (const ch of shown.channels) {
            if (hidden[ch.id]) continue
            for (const v of (ch.values || ch.lo)) lo = Math.min(lo, v)
            for (const v of (ch.values || ch.hi)) hi = Math.max(hi, v)
        }
        if (!isFinite(lo)) return [-vs, vs]
        lo *= vs; hi *= vs
        const pad = Math.max((hi - lo) * 0.06, 1e-4)
        return [lo - pad, hi + pad]
    }
    onYRangeChanged: plot.requestPaint()
    onHiddenChanged: { plot.requestPaint(); strip.requestPaint() }
    onMarkListChanged: { plot.requestPaint(); strip.requestPaint() }
    onMarkIdxChanged: plot.requestPaint()
    onFindLevelChanged: plot.requestPaint()
    onSelFromChanged: { plot.requestPaint(); strip.requestPaint() }
    onSelToChanged: { plot.requestPaint(); strip.requestPaint() }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        anchors.topMargin: 14
        spacing: 8

        // the recording
        RowLayout {
            Layout.fillWidth: true
            spacing: 14
            ColumnLayout {
                spacing: 2
                Layout.fillWidth: true
                Text { text: win.info.name || ""; color: Theme.title; font.pixelSize: Theme.fsLg; font.bold: true; font.family: Theme.mono; elide: Text.ElideMiddle; Layout.fillWidth: true }
                Text {
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    textFormat: Text.StyledText
                    color: Theme.muted
                    font.pixelSize: Theme.fsSm
                    text: rv.error !== "" || !win.ready ? (win.busy.text || "") : [win.info.kind, win.board, win.rate > 0 ? win.dur(win.total) : Theme.count(win.total) + " samples", win.inputs.map(i => "IN" + i).join(" "),
                            win.rate > 0 ? Theme.rate(win.rate) : "", win.info.volts ? "volts" : win.info.bits ? win.info.bits + " bit" : ""].filter(s => s).join(" · ")
                          + " · " + (!win.info.gapsFrom ? "losses not known (no report beside it)"
                              : win.info.lost > 0 ? "<font color=\"" + Theme.dangerText + "\">lost " + Theme.count(win.info.lost) + " samples/ch in " + win.info.places + (win.info.places === 1 ? " place" : " places") + "</font>"
                              : "<font color=\"" + Theme.okText + "\">nothing lost</font>")
                          + (win.info.gapsFrom && win.info.kind !== "BIN" ? " (from " + win.info.gapsFrom + ")" : "")
                          + (win.info.waited > 0 ? " · waited for a trigger " + Theme.count(win.info.waited) : "")
                }
            }
            RowLayout {
                visible: win.partList.length > 1
                spacing: 6
                Btn { kind: "outline"; compact: true; text: "◀"; enabled: win.partIdx > 0; Accessible.name: "The part before"; onClicked: win.openPart(win.partList[win.partIdx - 1].path) }
                Text { text: "part " + (win.partIdx + 1) + " of " + win.partList.length; color: Theme.value; font.pixelSize: Theme.fsSm; font.family: Theme.mono }
                Btn { kind: "outline"; compact: true; text: "▶"; enabled: win.partIdx >= 0 && win.partIdx < win.partList.length - 1; Accessible.name: "The part after"; onClicked: win.openPart(win.partList[win.partIdx + 1].path) }
            }
            // the window of the plot on the time line of the recording
            Text {
                visible: rv.error === ""
                readonly property int d: win.digits(win.to - win.from)
                text: win.fmtT(win.from, d) + " – " + win.fmtT(win.to, d) + " · " + win.dur(win.to - win.from)
                color: Theme.value
                font.family: Theme.mono
                font.pixelSize: Theme.fsSm
            }
            Btn { id: fileBtn; kind: "outline"; text: "File ▾"; active: fileMenu.opened; onClicked: fileMenu.popupFor(fileBtn) }
        }

        // the controls of the plot
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            visible: rv.error === ""
            Seg { small: true; options: ["Full Y", "Fit Y"]; currentIndex: win.fitY ? 1 : 0; onPicked: (i) => win.fitY = i === 1 }
            Select {
                visible: !win.info.volts
                implicitWidth: 120
                implicitHeight: 24 + Theme.fsAdd
                options: [[1, "±1 V"], [20, "±20 V"]]
                value: win.fullScale
                onPicked: (x) => win.fullScale = Number(x)
                ToolTip.visible: hovered
                ToolTip.text: "The range of the inputs when it was recorded: the file keeps the codes, not the gain"
            }
            Repeater {
                model: win.inputs
                delegate: Check {
                    required property var modelData
                    text: "IN" + modelData
                    checked: !win.hidden[modelData]
                    onClicked: { const h = Object.assign({}, win.hidden); h[modelData] = !checked; win.hidden = h }
                    // the colour of the input under its name (where the text of the box starts)
                    Rectangle {
                        x: parent.contentItem.x + parent.contentItem.leftPadding
                        anchors.bottom: parent.bottom
                        width: parent.contentItem.contentWidth
                        height: 3
                        radius: 1
                        color: win.color(parent.modelData)
                    }
                }
            }
            Item { Layout.fillWidth: true }
            Label2 { text: "Go to"; kind: "hint" }
            Seg { small: true; options: ["Losses", "Marks"]; currentIndex: win.navMode === "marks" ? 1 : 0; onPicked: (i) => win.navMode = i === 1 ? "marks" : "losses" }
            readonly property int navCount: win.navMode === "marks" ? win.markList.length : win.losses.length
            readonly property int navIdx: win.navMode === "marks" ? win.markIdx : win.lossIdx
            Btn { kind: "outline"; compact: true; text: "◀"; enabled: parent.navCount > 0; Accessible.name: "The one before"; onClicked: win.goNext(-1) }
            Text {
                text: !win.ready ? "…" : parent.navCount === 0 ? "none" : (parent.navIdx >= 0 ? parent.navIdx + 1 : "–") + " of " + parent.navCount
                color: parent.navCount === 0 ? Theme.muted : win.navMode === "marks" ? "#d9a6f0" : Theme.dangerText
                font.family: Theme.mono
                font.pixelSize: Theme.fsSm
            }
            Btn { kind: "outline"; compact: true; text: "▶"; enabled: parent.navCount > 0; Accessible.name: "The one after"; onClicked: win.goNext(1) }
            Rectangle { width: 1; height: 22; color: Theme.border }
            Btn { kind: "outline"; compact: true; text: "Whole"; onClicked: win.setWindow(0, win.total) }
            Btn { kind: "outline"; compact: true; text: "−"; Accessible.name: "Zoom out"; onClicked: win.zoom(2) }
            Btn { kind: "outline"; compact: true; text: "+"; Accessible.name: "Zoom in"; onClicked: win.zoom(0.5) }
            Btn { kind: "outline"; compact: true; text: "Table"; active: win.showTable; onClicked: win.showTable = !win.showTable }
        }

        // find a level: marks where an input goes through it, as the trigger of a board
        Rectangle {
            id: searchBar
            visible: rv.error === "" && win.ready
            Layout.fillWidth: true
            implicitHeight: searchRow.implicitHeight + 16
            color: Theme.card
            border.color: Theme.border
            radius: Theme.radius
            property int input: win.inputs.length ? win.inputs[0] : 1
            property string edge: "rising"
            property string level: ""
            property string hyst: "0.01"
            property string gapMs: "0"
            property string scope: "whole"
            property string status: ""
            property bool statusBad: false
            readonly property double levelV: Number(level)
            readonly property double hystV: Number(hyst)
            readonly property bool valid: level.trim() !== "" && isFinite(levelV) && isFinite(hystV) && hystV >= 0 && isFinite(Number(gapMs)) && Number(gapMs) >= 0
            onLevelChanged: win.findLevel = valid ? levelV : NaN
            Component.onCompleted: win.findLevel = valid ? levelV : NaN
            function go() {
                if (!valid) { status = "Give the level and the hysteresis in volts, not closer than in ms"; statusBad = true; return }
                const r = scope === "window" ? [win.from, win.to] : scope === "selection" && win.hasSel ? [win.selFrom, win.selTo] : [0, win.total]
                const gap = win.rate > 0 ? Number(gapMs) / 1000 * win.rate : Number(gapMs)
                const e = rv.find(input, edge, levelV, hystV, win.info.volts ? 1 : win.fullScale, gap, r[0], r[1])
                status = e !== "" ? e : "Finding…"
                statusBad = e !== ""
                win.findLevel = levelV
            }
            Connections {
                target: rv
                function onFound(count, message) {
                    searchBar.status = message + " · the marks of the last find replace those before"
                    searchBar.statusBad = false
                    win.navMode = "marks"
                    win.markIdx = -1
                    table.tab = "marks"
                }
            }
            // two groups: the second goes to a line of its own when the window is narrow or the text large
            ColumnLayout {
                id: searchRow
                anchors.fill: parent
                anchors.margins: 8
                anchors.leftMargin: 12
                spacing: 6
                Flow {
                    Layout.fillWidth: true
                    spacing: 10
                    RowLayout {
                        spacing: 10
                        Text { text: "Find"; color: Theme.text; font.bold: true; font.pixelSize: Theme.fsMd }
                        Select {
                            implicitWidth: 86
                            implicitHeight: 24 + Theme.fsAdd
                            options: win.inputs.map(i => [i, "IN" + i])
                            value: searchBar.input
                            onPicked: (x) => searchBar.input = Number(x)
                            Accessible.name: "Input"
                        }
                        Seg { small: true; options: ["Rising", "Falling", "Both"]; currentIndex: ["rising", "falling", "both"].indexOf(searchBar.edge); onPicked: (i) => searchBar.edge = ["rising", "falling", "both"][i] }
                        Label2 { text: "Level"; kind: "hint" }
                        TextIn { free: true; mono: true; implicitWidth: 80; topPadding: 0; bottomPadding: 0; implicitHeight: Math.max(24 + Theme.fsAdd, Math.ceil(font.pixelSize * 1.5) + 4); text: searchBar.level; bad: searchBar.level.trim() !== "" && !isFinite(searchBar.levelV); onTextChanged: searchBar.level = text; onAccepted: searchBar.go(); Accessible.name: "Level in volts" }
                        Label2 { text: "V ± "; kind: "hint" }
                        TextIn { free: true; mono: true; implicitWidth: 70; topPadding: 0; bottomPadding: 0; implicitHeight: Math.max(24 + Theme.fsAdd, Math.ceil(font.pixelSize * 1.5) + 4); text: searchBar.hyst; bad: !(searchBar.hystV >= 0); onTextChanged: searchBar.hyst = text; onAccepted: searchBar.go(); Accessible.name: "Hysteresis in volts" }
                        Label2 { text: "V · apart ≥"; kind: "hint"; ToolTip.visible: apartHover.hovered; ToolTip.text: "Not closer than this to the mark before"; HoverHandler { id: apartHover } }
                        TextIn { free: true; mono: true; implicitWidth: 60; topPadding: 0; bottomPadding: 0; implicitHeight: Math.max(24 + Theme.fsAdd, Math.ceil(font.pixelSize * 1.5) + 4); text: searchBar.gapMs; bad: !(Number(searchBar.gapMs) >= 0); onTextChanged: searchBar.gapMs = text; onAccepted: searchBar.go(); Accessible.name: "Not closer than, ms" }
                        Label2 { text: win.rate > 0 ? "ms" : "samples"; kind: "hint" }
                    }
                    RowLayout {
                        spacing: 10
                        Seg { small: true; options: win.hasSel ? ["Whole", "Window", "Selection"] : ["Whole", "Window"]; currentIndex: ["whole", "window", "selection"].indexOf(searchBar.scope); onPicked: (i) => searchBar.scope = ["whole", "window", "selection"][i] }
                        Btn { kind: "primary"; compact: true; text: "Find"; enabled: searchBar.valid && win.busy.what !== "find"; onClicked: searchBar.go() }
                        Btn { kind: "outline"; compact: true; text: "+ Mark"; ToolTip.visible: hovered; ToolTip.text: "A mark in the middle of the window; or double click on the plot"; onClicked: { rv.addMark(Math.round((win.from + win.to) / 2), searchBar.input) } }
                        Btn { kind: "outline"; compact: true; text: "Clear"; enabled: win.markList.length > 0; ToolTip.visible: hovered; ToolTip.text: "All the marks go"; onClicked: { rv.clearMarks(); win.markIdx = -1 } }
                    }
                }
                Label2 { Layout.fillWidth: true; visible: searchBar.status !== ""; kind: searchBar.statusBad ? "bad" : "hint"; wrapMode: Text.NoWrap; elide: Text.ElideRight; text: searchBar.status; ToolTip.visible: statusHover.hovered && text !== ""; ToolTip.text: text; HoverHandler { id: statusHover } }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12
        // the window
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.plot
            border.color: Theme.border
            radius: Theme.radius
            Label2 { anchors.centerIn: parent; visible: rv.error !== ""; kind: "bad"; text: rv.error }
            // the values
            Item {
                id: yAxis
                visible: rv.error === ""
                x: 0; y: plot.y
                width: 60; height: plot.height
                Repeater {
                    model: 5
                    delegate: Text {
                        required property int index
                        readonly property double v: win.yRange[1] - (win.yRange[1] - win.yRange[0]) * index / 4
                        y: Math.max(0, Math.min(yAxis.height - height, yAxis.height * index / 4 - height / 2))
                        anchors.right: parent.right
                        anchors.rightMargin: 6
                        text: Theme.volts(v)
                        color: Theme.muted
                        font.family: Theme.mono
                        font.pixelSize: Theme.fsXs
                    }
                }
            }
            Canvas {
                id: plot
                visible: rv.error === ""
                x: 60; y: 10
                width: parent.width - 60 - 12
                height: parent.height - 10 - 22
                onWidthChanged: win.refresh()
                onHeightChanged: requestPaint()
                renderStrategy: Canvas.Cooperative

                function xOf(p) { return (p - win.from) / Math.max(1, win.to - win.from) * width }
                // the gaps that are in [a, b): the first index by a binary search
                function gapsIn(a, b) {
                    const g = win.gapList
                    let lo = 0, hi = g.length
                    while (lo < hi) { const m = (lo + hi) >> 1; if (g[m].at + g[m].lost + g[m].wait <= a) lo = m + 1; else hi = m }
                    const out = []
                    for (let i = lo; i < g.length && g[i].at < b; i++) out.push(g[i])
                    return out
                }
                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()
                    const W = width, H = height
                    const y0 = win.yRange[0], y1 = win.yRange[1]
                    const vs = win.info.volts ? 1 : win.fullScale
                    const yOf = (v) => H - (v * vs - y0) / (y1 - y0) * H
                    // the grid
                    ctx.strokeStyle = Qt.rgba(153 / 255, 153 / 255, 153 / 255, 0.18)
                    ctx.lineWidth = 1
                    ctx.beginPath()
                    for (let i = 1; i < 10; i++) { const x = Math.round(W * i / 10) + 0.5; ctx.moveTo(x, 0); ctx.lineTo(x, H) }
                    for (let i = 1; i < 4; i++) { const y = Math.round(H * i / 4) + 0.5; ctx.moveTo(0, y); ctx.lineTo(W, y) }
                    ctx.stroke()
                    ctx.strokeStyle = Theme.border
                    ctx.beginPath(); ctx.moveTo(0.5, 0); ctx.lineTo(0.5, H); ctx.lineTo(W, H - 0.5); ctx.stroke()
                    if (y0 < 0 && y1 > 0) { const z = Math.round(H - (-y0) / (y1 - y0) * H) + 0.5; ctx.beginPath(); ctx.moveTo(0, z); ctx.lineTo(W, z); ctx.stroke() }
                    // the selection
                    if (win.hasSel) {
                        const a = Math.max(-2, xOf(win.selFrom)), b = Math.min(W + 2, xOf(win.selTo))
                        if (b > 0 && a < W) {
                            ctx.fillStyle = Qt.rgba(71 / 255, 178 / 255, 209 / 255, 0.14)
                            ctx.fillRect(a, 0, Math.max(1, b - a), H)
                            ctx.strokeStyle = Theme.accent
                            ctx.lineWidth = 2
                            ctx.beginPath(); ctx.moveTo(a, 0); ctx.lineTo(a, H); ctx.moveTo(b, 0); ctx.lineTo(b, H); ctx.stroke()
                        }
                    }
                    // the losses and the waits
                    const gaps = gapsIn(win.from, win.to)
                    ctx.lineWidth = 1
                    for (const g of gaps) {
                        if (g.lost > 0) {
                            const a = xOf(g.at), b = xOf(g.at + g.lost)
                            ctx.fillStyle = Qt.rgba(224 / 255, 103 / 255, 93 / 255, 0.22)
                            ctx.fillRect(a, 0, Math.max(1, b - a), H)
                            if (b - a > 3) {
                                ctx.strokeStyle = Theme.danger
                                ctx.setLineDash([4, 3])
                                ctx.beginPath(); ctx.moveTo(a + 0.5, 0); ctx.lineTo(a + 0.5, H); ctx.moveTo(b - 0.5, 0); ctx.lineTo(b - 0.5, H); ctx.stroke()
                                ctx.setLineDash([])
                            }
                        }
                        if (g.wait > 0) {
                            const a = xOf(g.at + g.lost), b = xOf(g.at + g.lost + g.wait)
                            ctx.fillStyle = Qt.rgba(232 / 255, 184 / 255, 74 / 255, 0.14)
                            ctx.fillRect(a, 0, Math.max(1, b - a), H)
                        }
                    }
                    if (gaps.length > 0 && gaps.length <= 8) {
                        ctx.font = Theme.fsXs + "px monospace"
                        ctx.textBaseline = "top"
                        for (const g of gaps) {
                            const t = g.lost > 0 ? "lost " + Theme.count(g.lost) : "waited " + Theme.count(g.wait)
                            const x = Math.max(2, Math.min(W - 90, xOf(g.at + (g.lost + g.wait) / 2) - 30))
                            const w = ctx.measureText(t).width + 12
                            ctx.fillStyle = Theme.plot
                            ctx.fillRect(x, 6, w, Theme.fsXs + 6)
                            ctx.strokeStyle = g.lost > 0 ? Theme.danger : Theme.amber
                            ctx.strokeRect(x + 0.5, 6.5, w, Theme.fsXs + 6)
                            ctx.fillStyle = g.lost > 0 ? Theme.dangerText : Theme.amberText
                            ctx.fillText(t, x + 6, 9)
                        }
                    }
                    // the level of the find, the marks
                    if (isFinite(win.findLevel) && win.findLevel > y0 && win.findLevel < y1) {
                        const yl = Math.round(H - (win.findLevel - y0) / (y1 - y0) * H) + 0.5
                        ctx.strokeStyle = "#b47ad1"
                        ctx.lineWidth = 1
                        ctx.setLineDash([6, 4])
                        ctx.beginPath(); ctx.moveTo(0, yl); ctx.lineTo(W, yl); ctx.stroke()
                        ctx.setLineDash([])
                        ctx.font = Theme.fsXs + "px monospace"
                        ctx.fillStyle = "#d9a6f0"
                        ctx.textBaseline = "bottom"
                        const lt = Theme.volts(win.findLevel)
                        ctx.fillText(lt, W - ctx.measureText(lt).width - 6, yl - 2)
                    }
                    const ml = win.markList
                    let m0 = 0, m1 = ml.length
                    while (m0 < m1) { const m = (m0 + m1) >> 1; if (ml[m].pos < win.from) m0 = m + 1; else m1 = m }
                    const shownMarks = []
                    for (let i = m0; i < ml.length && ml[i].pos <= win.to; i++) shownMarks.push(i)
                    ctx.lineWidth = 2
                    for (const i of shownMarks) {
                        const x = Math.round(xOf(ml[i].pos)) + 0.5
                        ctx.strokeStyle = i === win.markIdx ? "#d9a6f0" : "#b47ad1"
                        ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, H); ctx.stroke()
                    }
                    if (shownMarks.length <= 30) {
                        ctx.font = Theme.fsXs + "px monospace"
                        ctx.textBaseline = "top"
                        for (const i of shownMarks) {
                            const t = win.markName(i)
                            const w = ctx.measureText(t).width + 12
                            const x = Math.max(2, Math.min(W - w - 2, xOf(ml[i].pos) + 4))
                            const y = 6 + (Theme.fsXs + 10)
                            ctx.fillStyle = i === win.markIdx ? "#3b2f45" : Theme.plot
                            ctx.fillRect(x, y, w, Theme.fsXs + 6)
                            ctx.strokeStyle = i === win.markIdx ? "#d9a6f0" : "#b47ad1"
                            ctx.lineWidth = 1
                            ctx.strokeRect(x + 0.5, y + 0.5, w, Theme.fsXs + 6)
                            ctx.fillStyle = i === win.markIdx ? "#f1dcfa" : "#d9a6f0"
                            ctx.fillText(t, x + 6, y + 3)
                        }
                    }
                    // the samples: the line breaks in the gaps
                    const d = win.shown
                    if (!d.channels) return
                    const n = d.to - d.from
                    // [a, b) in a gap; a grows, so the gaps are walked once a channel
                    let gi = 0
                    const inGap = (a, b) => {
                        while (gi < gaps.length && gaps[gi].at + gaps[gi].lost + gaps[gi].wait <= a) gi++
                        return gi < gaps.length && a >= gaps[gi].at && b <= gaps[gi].at + gaps[gi].lost + gaps[gi].wait
                    }
                    ctx.lineWidth = 1.4
                    for (const ch of d.channels) {
                        if (win.hidden[ch.id]) continue
                        gi = 0
                        ctx.strokeStyle = win.color(ch.id)
                        ctx.beginPath()
                        if (ch.values) {
                            const step = W / Math.max(1, ch.values.length)
                            let on = false
                            for (let i = 0; i < ch.values.length; i++) {
                                const p = d.from + i
                                if (inGap(p, p + 1)) { on = false; continue }
                                const x = xOf(p + 0.5), y = yOf(ch.values[i])
                                if (on) ctx.lineTo(x, y); else ctx.moveTo(x, y)
                                on = true
                                if (step > 8) ctx.rect(x - 1.5, y - 1.5, 3, 3)
                            }
                        } else {
                            const m = ch.lo.length
                            let pl = NaN, ph = NaN
                            for (let j = 0; j < m; j++) {
                                const a = d.from + n * j / m, b = d.from + n * (j + 1) / m
                                if (inGap(a, b)) { pl = NaN; continue }
                                // the column joins the one before
                                const lo = isNaN(pl) ? ch.lo[j] : Math.min(ch.lo[j], ph)
                                const hi = isNaN(pl) ? ch.hi[j] : Math.max(ch.hi[j], pl)
                                const x = j * W / m + 0.5
                                ctx.moveTo(x, yOf(hi) - 0.5)
                                ctx.lineTo(x, yOf(lo) + 0.5)
                                pl = ch.lo[j]; ph = ch.hi[j]
                            }
                        }
                        ctx.stroke()
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton
                    property string mode: ""
                    property double anchorP: 0
                    property double startX: 0
                    property double startFrom: 0
                    function posOf(x) { return win.from + (win.to - win.from) * Math.max(0, Math.min(1, x / width)) }
                    function edgeAt(x) {
                        if (!win.hasSel) return ""
                        if (Math.abs(x - plot.xOf(win.selFrom)) < 5) return "a"
                        if (Math.abs(x - plot.xOf(win.selTo)) < 5) return "b"
                        return ""
                    }
                    cursorShape: mode === "pan" ? Qt.ClosedHandCursor : edgeAt(mouseX) !== "" || mode === "a" || mode === "b" ? Qt.SizeHorCursor : Qt.CrossCursor
                    onWheel: (e) => win.zoom(e.angleDelta.y > 0 ? 0.8 : 1.25, e.x / width)
                    onPressed: (e) => {
                        const edge = edgeAt(e.x)
                        if (edge !== "") mode = edge
                        else if (e.modifiers & Qt.ShiftModifier) { mode = "sel"; anchorP = posOf(e.x); win.selFrom = anchorP; win.selTo = anchorP }
                        else { mode = "pan"; startX = e.x; startFrom = win.from }
                    }
                    onPositionChanged: (e) => {
                        if (mode === "") return
                        const p = posOf(e.x)
                        if (mode === "sel") { win.selFrom = Math.round(Math.min(anchorP, p)); win.selTo = Math.round(Math.max(anchorP, p)) }
                        else if (mode === "a") { win.selFrom = Math.round(Math.min(p, win.selTo)) }
                        else if (mode === "b") { win.selTo = Math.round(Math.max(p, win.selFrom)) }
                        else {
                            const span = win.to - win.from
                            win.setWindow(startFrom - (e.x - startX) / width * span, startFrom - (e.x - startX) / width * span + span)
                        }
                    }
                    onReleased: mode = ""
                    // a mark by hand where the pointer is
                    onDoubleClicked: (e) => rv.addMark(Math.round(posOf(e.x)), searchBar.input)
                }
            }
            // the times
            Item {
                visible: rv.error === ""
                x: plot.x; y: plot.y + plot.height + 3
                width: plot.width; height: 16
                Repeater {
                    model: 6
                    delegate: Text {
                        required property int index
                        x: index === 0 ? 0 : index === 5 ? parent.width - width : parent.width * index / 5 - width / 2
                        text: win.fmtT(win.from + (win.to - win.from) * index / 5, win.digits(win.to - win.from))
                        color: Theme.muted
                        font.family: Theme.mono
                        font.pixelSize: Theme.fsXs
                    }
                }
            }
            Label2 {
                anchors.centerIn: parent
                visible: rv.error === "" && !win.ready
                kind: "hint"
                text: win.busy.text || "Opening"
            }
        }

        MarkTable { id: table; visible: win.showTable && win.ready && rv.error === ""; Layout.preferredWidth: 420; Layout.fillHeight: true }
        }

        // the selection
        Rectangle {
            visible: rv.error === ""
            Layout.fillWidth: true
            implicitHeight: selRow.implicitHeight + 12
            radius: Theme.radius
            color: win.hasSel ? Qt.rgba(71 / 255, 178 / 255, 209 / 255, 0.10) : "transparent"
            border.color: win.hasSel ? Qt.rgba(71 / 255, 178 / 255, 209 / 255, 0.5) : Theme.border
            RowLayout {
                id: selRow
                anchors.fill: parent
                anchors.margins: 6
                anchors.leftMargin: 12
                spacing: 12
                Text { text: "Selection"; color: win.hasSel ? Theme.accent : Theme.muted; font.bold: true; font.pixelSize: Theme.fsMd }
                Text {
                    visible: win.hasSel
                    readonly property int d: Math.max(3, win.digits(win.selTo - win.selFrom))
                    text: win.fmtT(win.selFrom, d) + " – " + win.fmtT(win.selTo, d) + " · " + win.dur(win.selTo - win.selFrom) + " · " + Theme.count(win.selTo - win.selFrom) + " samples/ch"
                    color: Theme.value
                    font.family: Theme.mono
                    font.pixelSize: Theme.fsMd
                }
                Label2 {
                    Layout.fillWidth: true
                    kind: win.message !== "" ? (win.messageBad ? "bad" : "hint") : "hint"
                    wrapMode: Text.NoWrap
                    elide: Text.ElideMiddle
                    text: win.message !== "" ? win.message : win.hasSel ? "the edges move" : "shift + drag on the plot or on the whole recording below"
                }
                RowLayout {
                    visible: !!rv.exporting.running
                    spacing: 6
                    Rectangle {
                        implicitWidth: 120; height: 6; radius: 3; color: Theme.border
                        Rectangle { width: parent.width * (rv.exporting.progress || 0); height: parent.height; radius: 3; color: Theme.accent }
                    }
                    Text { text: Math.round((rv.exporting.progress || 0) * 100) + " %"; color: Theme.value; font.pixelSize: Theme.fsSm; font.family: Theme.mono }
                    Btn { kind: "outline"; compact: true; text: "Stop"; onClicked: rv.stopExport() }
                }
                Btn { kind: "primary"; compact: true; text: "Export…"; enabled: win.ready && rv.error === "" && !rv.exporting.running; onClicked: exportDialog.ask() }
                Btn { kind: "outline"; compact: true; text: "Zoom to it"; enabled: win.hasSel; onClicked: win.setWindow(win.selFrom, win.selTo) }
                Btn { kind: "outline"; compact: true; text: "✕"; enabled: win.hasSel; Accessible.name: "Clear the selection"; onClicked: { win.selFrom = 0; win.selTo = 0 } }
            }
        }

        // the whole recording
        Label2 { visible: rv.error === ""; kind: "hint"; text: "The whole recording: drag the window, or click where to go" }
        Rectangle {
            visible: rv.error === ""
            Layout.fillWidth: true
            implicitHeight: 64 + 14 + 12
            color: Theme.plot
            border.color: Theme.border
            radius: Theme.radius
            Canvas {
                id: strip
                x: 60; y: 6
                width: parent.width - 60 - 12
                height: 64
                onWidthChanged: requestPaint()
                Connections { target: win; function onFromChanged() { strip.requestPaint() } function onToChanged() { strip.requestPaint() } }
                function xOf(p) { return p / Math.max(1, win.total) * width }
                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()
                    const W = width, H = height
                    const o = rv.overview
                    if (o.lo) {
                        const B = o.buckets
                        // the full scale always (Fit Y is for the window)
                        let m = 1
                        if (win.info.volts) { m = 0.01; for (let k = 0; k < o.lo.length; k++) for (let i = 0; i < B; i++) m = Math.max(m, Math.abs(o.lo[k][i]), Math.abs(o.hi[k][i])) }
                        const yOf = (v) => Math.max(0, Math.min(H, H / 2 - v / m * H / 2))
                        for (let k = 0; k < o.lo.length; k++) {
                            const id = win.inputs[k]
                            if (win.hidden[id]) continue
                            ctx.strokeStyle = win.color(id)
                            ctx.globalAlpha = 0.6
                            ctx.beginPath()
                            for (let x = 0; x < W; x++) {
                                const s = Math.floor(x * B / W), e = Math.max(s + 1, Math.floor((x + 1) * B / W))
                                let lo = Infinity, hi = -Infinity
                                for (let i = s; i < e && i < B; i++) { lo = Math.min(lo, o.lo[k][i]); hi = Math.max(hi, o.hi[k][i]) }
                                ctx.moveTo(x + 0.5, yOf(hi) - 0.5)
                                ctx.lineTo(x + 0.5, yOf(lo) + 0.5)
                            }
                            ctx.stroke()
                        }
                        ctx.globalAlpha = 1
                    }
                    // the losses
                    ctx.strokeStyle = Theme.danger
                    ctx.lineWidth = 2
                    ctx.beginPath()
                    let last = -10
                    for (const g of win.losses) {
                        const x = Math.round(xOf(g.at))
                        if (x - last < 2) continue
                        last = x
                        ctx.moveTo(x, 0); ctx.lineTo(x, H)
                    }
                    ctx.stroke()
                    // the marks
                    ctx.strokeStyle = "#b47ad1"
                    ctx.beginPath()
                    last = -10
                    for (const m of win.markList) {
                        const x = Math.round(xOf(m.pos))
                        if (x - last < 2) continue
                        last = x
                        ctx.moveTo(x, 0); ctx.lineTo(x, H)
                    }
                    ctx.stroke()
                    // the selection and the window
                    if (win.hasSel) {
                        ctx.fillStyle = Qt.rgba(71 / 255, 178 / 255, 209 / 255, 0.3)
                        ctx.fillRect(xOf(win.selFrom), 0, Math.max(2, xOf(win.selTo) - xOf(win.selFrom)), H)
                    }
                    const a = xOf(win.from), b = xOf(win.to)
                    const w = Math.max(6, b - a)
                    ctx.strokeStyle = Theme.value
                    ctx.lineWidth = 2
                    ctx.strokeRect(Math.min(W - w, Math.max(1, (a + b) / 2 - w / 2)), 1, w - 1, H - 2)
                }
                MouseArea {
                    anchors.fill: parent
                    property bool sel: false
                    property double anchorP: 0
                    cursorShape: Qt.PointingHandCursor
                    function posOf(x) { return win.total * Math.max(0, Math.min(1, x / width)) }
                    function go(x) { const span = win.to - win.from; const c = posOf(x); win.setWindow(c - span / 2, c + span / 2) }
                    onPressed: (e) => {
                        sel = (e.modifiers & Qt.ShiftModifier) !== 0
                        if (sel) { anchorP = posOf(e.x); win.selFrom = anchorP; win.selTo = anchorP } else go(e.x)
                    }
                    onPositionChanged: (e) => {
                        if (!pressed) return
                        if (sel) { const p = posOf(e.x); win.selFrom = Math.round(Math.min(anchorP, p)); win.selTo = Math.round(Math.max(anchorP, p)) } else go(e.x)
                    }
                }
            }
            Item {
                x: strip.x; y: strip.y + strip.height + 1
                width: strip.width; height: 14
                Repeater {
                    model: 6
                    delegate: Text {
                        required property int index
                        x: index === 0 ? 0 : index === 5 ? parent.width - width : parent.width * index / 5 - width / 2
                        text: win.fmtT(win.total * index / 5, win.rate > 0 && win.total / win.rate >= 50 ? 0 : 2)
                        color: Theme.muted
                        font.family: Theme.mono
                        font.pixelSize: Theme.fsXs
                    }
                }
            }
        }
        Text {
            visible: rv.error === ""
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            textFormat: Text.StyledText
            color: Theme.muted
            font.pixelSize: Theme.fsSm
            text: "<font color=\"" + Theme.danger + "\">▮</font> samples lost (the board or this PC): the line breaks there &nbsp;&nbsp; "
                + "<font color=\"" + Theme.amber + "\">▮</font> waited for a trigger &nbsp;&nbsp; wheel: zoom where the pointer is · drag: move · shift + drag: select"
        }
    }

    // what keeps the window waiting longer than a second: a dialog with its progress
    property double busySince: 0
    onBusyChanged: {
        if (busy.what) { if (!busySince) busySince = Date.now() }
        else { busySince = 0; if (busyDialog.opened) busyDialog.close() }
    }
    Timer { interval: 200; repeat: true; running: !!win.busy.what; onTriggered: if (Date.now() - win.busySince > 1000 && !busyDialog.opened) busyDialog.open() }
    // the open stopped: the window goes
    Connections { target: rv; function onChanged() { if (rv.error === "stopped") win.closeNow() } }
    Dialog2 {
        id: busyDialog
        closePolicy: Popup.NoAutoClose
        width: Math.min(560, win.width - 48)
        height: Math.min(busyCol.implicitHeight + 40 + chromeHeight, maxHeight)
        title: win.busy.what === "find" ? "Finding the level" : "Opening the recording"
        subtitle: win.info.name || ""
        // closed by its ✕: what runs stops
        onClosed: if (win.busy.what) rv.stopBusy()
        ColumnLayout {
            id: busyCol
            x: 20; y: 18
            width: parent.width - 40
            spacing: 10
            Label2 { text: win.busy.text || ""; Layout.fillWidth: true; wrapMode: Text.Wrap }
            Rectangle {
                Layout.fillWidth: true
                height: 8; radius: 4; color: Theme.border
                Rectangle { width: parent.width * Math.max(0, Math.min(1, win.busy.progress || 0)); height: parent.height; radius: 4; color: Theme.accent }
            }
            Label2 {
                kind: "hint"
                Layout.fillWidth: true
                text: win.busy.what === "find" ? "Stop keeps the marks found until then." : "Read once: the next open of this recording is at once. Stop closes the window."
            }
        }
        footer: [
            Item { Layout.fillWidth: true },
            Btn { kind: "outline"; text: "Stop"; onClicked: rv.stopBusy() }
        ]
    }

    // the table of the marks and of the losses
    component MarkTable: Rectangle {
        id: tbl
        property string tab: "marks"
        readonly property var rows: tab === "marks" ? win.markList : win.losses
        function showRow(t, i) { tab = t; Qt.callLater(() => list.positionViewAtIndex(i, ListView.Contain)) }
        color: Theme.card
        border.color: Theme.border
        radius: Theme.radius
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 1
            spacing: 0
            RowLayout {
                Layout.fillWidth: true
                Layout.margins: 8
                Layout.leftMargin: 12
                spacing: 10
                Seg { small: true; options: ["Marks " + win.markList.length, "Losses " + win.losses.length]; currentIndex: tbl.tab === "marks" ? 0 : 1; onPicked: (i) => tbl.tab = i === 0 ? "marks" : "losses" }
                Item { Layout.fillWidth: true }
                Btn { kind: "outline"; compact: true; text: "Export CSV…"; enabled: tbl.rows.length > 0; onClicked: tableDlg.open() }
            }
            Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }
            TRow { head: true; marks: tbl.tab === "marks"; Layout.fillWidth: true }
            Rectangle { Layout.fillWidth: true; height: 1; color: Theme.borderStrong }
            ListView {
                id: list
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: tbl.rows
                ScrollBar.vertical: ScrollBar {}
                delegate: TRow {
                    required property var modelData
                    required property int index
                    marks: tbl.tab === "marks"
                    width: list.width
                    r: modelData
                    i: index
                }
                Label2 {
                    anchors.centerIn: parent
                    width: parent.width - 40
                    horizontalAlignment: Text.AlignHCenter
                    visible: tbl.rows.length === 0
                    kind: "hint"
                    text: tbl.tab === "marks" ? "No marks: find a level above, or double click on the plot" : win.info.gapsFrom ? "Nothing lost" : "The losses are not known (no report beside the file)"
                }
            }
            Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }
            Label2 {
                Layout.fillWidth: true
                Layout.margins: 8
                Layout.leftMargin: 12
                kind: "hint"
                text: tbl.tab === "marks" ? "Click a row: the window goes there. Right click: rename, delete the mark." : "Click a row: the window goes there."
            }
        }
    }
    // a row; head: the names of the columns
    component TRow: Item {
        id: row
        property bool head: false
        property var r: null
        property int i: -1
        property bool marks: true
        readonly property bool cur: !head && i === (marks ? win.markIdx : win.lossIdx)
        implicitHeight: 28 + Theme.fsAdd
        Rectangle {
            anchors.fill: parent
            visible: !row.head
            color: row.cur ? (row.marks ? "#3b2f45" : "#4a3432") : rowMouse.containsMouse ? Theme.hover : "transparent"
            Rectangle { visible: row.cur; width: 3; height: parent.height; color: row.marks ? "#b47ad1" : Theme.danger }
            Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.borderSoft }
        }
        MouseArea {
            id: rowMouse
            anchors.fill: parent
            enabled: !row.head
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onClicked: (e) => {
                if (row.marks) win.goMark(row.i); else win.goLoss(row.i)
                if (e.button === Qt.RightButton && row.marks) markMenu.popupAt(row.i, rowMouse, e.x, e.y)
            }
        }
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 6
            TCell { w: 0.7; t: row.head ? "#" : row.marks ? win.markName(row.i) : String(row.i + 1) }
            TCell { w: 1.6; mono: true; t: row.head ? "TIME" : win.fmtT(row.marks ? row.r.pos : row.r.at, 6) }
            TCell { w: 0.8; t: row.head ? (row.marks ? "INPUT" : "LOST") : row.marks ? (row.r.input ? "IN" + row.r.input + (row.r.edge === "rising" ? " ↗" : row.r.edge === "falling" ? " ↘" : "") : "-") : Theme.count(row.r.lost) }
            TCell { w: 1.1; mono: true; t: row.head ? (row.marks ? "AFTER THE ONE BEFORE" : "WAITED") : row.marks ? (row.i > 0 ? win.dur(row.r.pos - win.markList[row.i - 1].pos) : "-") : (row.r.wait ? Theme.count(row.r.wait) : "-") }
            TCell { w: 0.7; visible: row.marks; t: row.head ? "FROM" : row.r.from === "find" ? "find" : "by hand" }
        }
    }
    component TCell: Text {
        property string t: ""
        property real w: 1
        property bool mono: false
        text: t
        Layout.fillWidth: true
        Layout.preferredWidth: 100 * w
        elide: Text.ElideRight
        color: parent.parent.head ? Theme.muted : parent.parent.cur ? Theme.value : Theme.text
        font.pixelSize: parent.parent.head ? Theme.fsXs : Theme.fsSm
        font.family: mono && !parent.parent.head ? Theme.mono : Qt.application.font.family
    }
    FileDialog {
        id: tableDlg
        fileMode: FileDialog.SaveFile
        nameFilters: ["CSV (*.csv)"]
        defaultSuffix: "csv"
        currentFile: app.pathToUrl(String(win.path).replace(/\.[^.\\/]*$/, "") + (table.tab === "marks" ? "_marks_table.csv" : "_losses.csv"))
        onAccepted: {
            const e = rv.exportTable(selectedFile, table.tab !== "marks")
            win.say(e !== "" ? e : "Saved to " + app.urlToPath(selectedFile), e !== "")
        }
    }
    // a mark: rename, delete
    Popup {
        id: markMenu
        property int index: -1
        function popupAt(i, item, px, py) { index = i; parent = item; x = px; y = py; open() }
        padding: 4
        width: 200
        background: Rectangle { color: Theme.card; border.color: Theme.borderStrong; radius: 4 }
        contentItem: Column {
            spacing: 0
            MarkMenuRow { text: "Rename…"; onPicked: renameMarkDialog.ask(markMenu.index) }
            MarkMenuRow { text: "Delete"; bad: true; onPicked: { rv.removeMark(markMenu.index); win.markIdx = -1 } }
        }
    }
    component MarkMenuRow: AbstractButton {
        id: mmr
        property bool bad: false
        signal picked()
        width: 192
        height: 30 + Theme.fsAdd
        hoverEnabled: true
        onClicked: { markMenu.close(); picked() }
        background: Rectangle { color: mmr.hovered ? Theme.hover : "transparent"; radius: 3 }
        contentItem: Text { leftPadding: 10; text: mmr.text; verticalAlignment: Text.AlignVCenter; color: mmr.bad ? Theme.dangerText : Theme.text; font.pixelSize: Theme.fsBase }
    }
    Dialog2 {
        id: renameMarkDialog
        property int index: -1
        property string text: ""
        function ask(i) { index = i; text = win.markList[i] ? win.markList[i].name : ""; open() }
        function go() { rv.renameMark(index, text); close() }
        width: Math.min(440, win.width - 48)
        height: Math.min(renMarkCol.implicitHeight + 40 + chromeHeight, maxHeight)
        title: "Name the mark"
        subtitle: renameMarkDialog.index >= 0 ? win.markName(renameMarkDialog.index) + " at " + win.fmtT(win.markList[renameMarkDialog.index] ? win.markList[renameMarkDialog.index].pos : 0, 6) : ""
        ColumnLayout {
            id: renMarkCol
            x: 20; y: 18
            width: parent.width - 40
            TextIn { free: true; Layout.fillWidth: true; text: renameMarkDialog.text; placeholderText: "empty: M and its number"; onTextChanged: renameMarkDialog.text = text; onAccepted: renameMarkDialog.go() }
        }
        footer: [
            Item { Layout.fillWidth: true },
            Btn { kind: "outline"; text: "Cancel"; onClicked: renameMarkDialog.close() },
            Btn { kind: "primary"; text: "Save"; onClicked: renameMarkDialog.go() }
        ]
    }

    // File: the recording of this window
    Popup {
        id: fileMenu
        function popupFor(item) { parent = item; x = item.width - width; y = item.height + 4; open() }
        padding: 4
        width: 230
        background: Rectangle { color: Theme.card; border.color: Theme.borderStrong; radius: 4 }
        contentItem: Column {
            spacing: 0
            MenuRow { text: "Show in folder"; onPicked: recordings.showInFolder(win.path) }
            MenuRow { text: "Copy path"; onPicked: recordings.copyPath(win.path) }
            MenuRow { text: "Open the report"; enabled: fileMenu.opened && recordings.companions(win.path).some(c => c.endsWith(".log.txt")); onPicked: recordings.openFile(win.path + ".log.txt") }
            Rectangle { width: parent.width; height: 1; color: Theme.border }
            MenuRow { text: "Delete…"; bad: true; onPicked: deleteDialog.open() }
        }
    }
    component MenuRow: AbstractButton {
        id: mr
        property bool bad: false
        signal picked()
        width: 222
        height: 30 + Theme.fsAdd
        hoverEnabled: true
        onClicked: { fileMenu.close(); picked() }
        background: Rectangle { color: mr.hovered && mr.enabled ? Theme.hover : "transparent"; radius: 3 }
        contentItem: Text {
            leftPadding: 10
            text: mr.text
            verticalAlignment: Text.AlignVCenter
            color: !mr.enabled ? Theme.dim : mr.bad ? Theme.dangerText : Theme.text
            font.pixelSize: Theme.fsBase
        }
    }

    Dialog2 {
        id: deleteDialog
        width: Math.min(560, win.width - 48)
        height: Math.min(delCol.implicitHeight + 40 + chromeHeight, maxHeight)
        title: "Delete this recording?"
        ColumnLayout {
            id: delCol
            x: 20; y: 18
            width: parent.width - 40
            spacing: 8
            Text {
                readonly property var extra: deleteDialog.opened ? recordings.companions(win.path).map(c => c.slice(win.path.length)) : []
                text: (win.info.name || "") + (extra.length ? " (+ " + extra.join(", ") + ")" : "")
                color: Theme.muted
                font.family: Theme.mono
                font.pixelSize: Theme.fsSm
                wrapMode: Text.WrapAnywhere
                Layout.fillWidth: true
            }
            Label2 { kind: "hint"; text: "This window closes. CSV, WAV and TDMS made from it stay."; Layout.fillWidth: true }
        }
        footer: [
            Item { Layout.fillWidth: true },
            Btn { kind: "outline"; text: "Cancel"; onClicked: deleteDialog.close() },
            Btn {
                kind: "danger"; text: "Delete"
                onClicked: {
                    deleteDialog.close()
                    rv.release()
                    const e = recordings.remove([win.path])
                    if (e === "") { win.closeNow(); return }
                    // not deleted (another program has it open): the view goes on
                    rv.open(win.path)
                    win.say(e, true)
                }
            }
        ]
    }

    Dialog2 {
        id: stopDialog
        width: Math.min(480, win.width - 48)
        height: Math.min(stopCol.implicitHeight + 40 + chromeHeight, maxHeight)
        title: "Stop the export?"
        ColumnLayout {
            id: stopCol
            x: 20; y: 18
            width: parent.width - 40
            Label2 { kind: "hint"; text: "An export to " + (rv.exporting.output || "").split(/[\\/]/).pop() + " runs. The window closes when it is stopped; the part written goes."; Layout.fillWidth: true }
        }
        footer: [
            Item { Layout.fillWidth: true },
            Btn { kind: "outline"; text: "Go on"; onClicked: stopDialog.close() },
            Btn { kind: "danger"; text: "Stop and close"; onClicked: { stopDialog.close(); win.closeNow() } }
        ]
    }

    // Export: the selection (or the window, or the whole) to a file of its own
    Dialog2 {
        id: exportDialog
        property string format: "CSV"
        property var chosen: ({})
        property bool index: true
        property bool time: true
        property string fromText: ""
        property string toText: ""
        property string output: ""
        property string error: ""
        // the positions given (selection, window, whole) stay exact while their text is not
        // edited: the text rounds them to its decimals
        property double exactA: NaN
        property double exactB: NaN
        property string exactFromText: ""
        property string exactToText: ""
        readonly property double a: fromText === exactFromText && !isNaN(exactA) ? exactA : Math.max(0, win.parseT(fromText))
        readonly property double b: toText === exactToText && !isNaN(exactB) ? exactB : Math.min(win.total, win.parseT(toText))
        readonly property bool rangeOk: !isNaN(a) && !isNaN(b) && b > a && a >= 0 && b <= win.total
        readonly property var ins: win.inputs.filter(i => chosen[i])
        width: Math.min(620, win.width - 48)
        height: Math.min(expCol.implicitHeight + 40 + chromeHeight, maxHeight)
        title: win.hasSel ? "Export the selection" : "Export"
        subtitle: "of " + (win.info.name || "")
        function setRange(p, q) {
            const d = Math.max(6, win.digits(q - p))
            exactA = Math.max(0, Math.round(p))
            exactB = Math.min(win.total, Math.round(q))
            exactFromText = win.fmtT(exactA, d)
            exactToText = win.fmtT(exactB, d)
            fromText = exactFromText
            toText = exactToText
        }
        function ask() {
            error = ""
            const c = {}
            for (const i of win.inputs) c[i] = !win.hidden[i]
            chosen = c
            if (win.hasSel) setRange(win.selFrom, win.selTo); else setRange(win.from, win.to)
            name()
            open()
        }
        function name() { if (rangeOk) output = rv.exportName(a, b, format) }
        onFormatChanged: name()
        onAChanged: name()
        onBChanged: name()
        function go() {
            if (!rangeOk) { error = "The range is not on the recording"; return }
            error = rv.exportRange(a, b, format, ins, index, time, output)
            if (error === "") { win.say("Exporting to " + output, false); close() }
        }
        // about the size: CSV ~7 characters a value, the others the codes
        readonly property double estimate: {
            if (!rangeOk) return 0
            const n = Math.max(1, ins.length)
            const per = format === "CSV" ? 7 + (index ? 11 / n : 0) + (time ? 24 / n : 0) : (win.info.bits ? win.info.bits / 8 : 4)
            return (b - a) * ins.length * per
        }
        ColumnLayout {
            id: expCol
            x: 20; y: 18
            width: parent.width - 40
            spacing: 12
            Field {
                label: "Range"
                Layout.fillWidth: true
                RowLayout {
                    spacing: 10
                    Field { label: "From"; TextIn { free: true; mono: true; implicitWidth: 170; text: exportDialog.fromText; bad: isNaN(exportDialog.a); onTextChanged: exportDialog.fromText = text } }
                    Field { label: "To"; TextIn { free: true; mono: true; implicitWidth: 170; text: exportDialog.toText; bad: isNaN(exportDialog.b); onTextChanged: exportDialog.toText = text } }
                    Label2 {
                        Layout.alignment: Qt.AlignBottom
                        Layout.bottomMargin: 8
                        kind: exportDialog.rangeOk ? "mono" : "bad"
                        text: exportDialog.rangeOk ? Theme.count(exportDialog.b - exportDialog.a) + " samples/ch · " + win.dur(exportDialog.b - exportDialog.a) : "not on the recording (0 – " + win.fmtT(win.total, 3) + ")"
                    }
                }
                Text {
                    textFormat: Text.StyledText
                    linkColor: Theme.accent
                    color: Theme.muted
                    font.pixelSize: Theme.fsSm
                    text: "The selection, the window shown or the whole recording: " + (win.hasSel ? "<a href=\"s\">selection</a> · " : "") + "<a href=\"w\">window</a> · <a href=\"a\">whole</a>"
                    onLinkActivated: (l) => l === "s" ? exportDialog.setRange(win.selFrom, win.selTo) : l === "w" ? exportDialog.setRange(win.from, win.to) : exportDialog.setRange(0, win.total)
                    HoverHandler { cursorShape: Qt.PointingHandCursor }
                }
            }
            Field {
                label: "Format"
                hint: exportDialog.format === "BIN" ? "BIN keeps the losses and the trigger waits of the range and opens here again" : ""
                Layout.fillWidth: true
                Seg { options: ["CSV", "WAV", "TDMS", "BIN"]; currentIndex: ["CSV", "WAV", "TDMS", "BIN"].indexOf(exportDialog.format); onPicked: (i) => exportDialog.format = ["CSV", "WAV", "TDMS", "BIN"][i] }
            }
            Field {
                label: "Inputs"
                Row {
                    spacing: 16
                    Repeater {
                        model: win.inputs
                        delegate: Check {
                            required property var modelData
                            text: "IN" + modelData
                            checked: !!exportDialog.chosen[modelData]
                            onClicked: { const c = Object.assign({}, exportDialog.chosen); c[modelData] = checked; exportDialog.chosen = c }
                        }
                    }
                }
            }
            Field {
                visible: exportDialog.format === "CSV"
                label: "CSV columns"
                Row {
                    spacing: 16
                    Check { text: "Index"; checked: exportDialog.index; onClicked: exportDialog.index = checked }
                    Check { text: "Time of each sample"; checked: exportDialog.time; onClicked: exportDialog.time = checked }
                }
            }
            Field {
                label: "Into"
                Layout.fillWidth: true
                TextIn { free: true; mono: true; Layout.fillWidth: true; text: exportDialog.output; onTextChanged: exportDialog.output = text }
            }
            Label2 {
                visible: exportDialog.estimate > 0
                kind: exportDialog.estimate > recordings.diskFree * 0.9 ? "warn" : "hint"
                Layout.fillWidth: true
                text: "About " + Theme.bytes(exportDialog.estimate) + " · the new file shows in the recordings"
            }
            Label2 { visible: exportDialog.error !== ""; kind: "bad"; text: exportDialog.error; Layout.fillWidth: true }
        }
        footer: [
            Item { Layout.fillWidth: true },
            Btn { kind: "outline"; text: "Cancel"; onClicked: exportDialog.close() },
            Btn { kind: "primary"; text: "Export"; enabled: exportDialog.rangeOk && exportDialog.ins.length > 0; onClicked: exportDialog.go() }
        ]
    }
    Connections {
        target: rv
        function onExported(ok, output, message) {
            win.say(ok ? "Exported: " + output : "Not exported: " + message, !ok)
            recordings.rescan()
        }
    }
}
