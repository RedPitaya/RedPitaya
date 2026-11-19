import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "ui"

// A board on the dashboard: its state, its streams, the preview, the counters and the buttons.
Rectangle {
    id: root
    property var b
    property bool big: false
    // the height of the preview of a lone board: what the window leaves for it
    property real bigChartHeight: 420
    // the card without its plot: the dashboard gives the plot of a lone board what the window has left
    readonly property real restHeight: hdr.implicitHeight + chips.implicitHeight + pvHead.height + 1 + counters.implicitHeight
        + fileRow.implicitHeight + (btnRow.visible ? btnRow.implicitHeight : offRow.implicitHeight) + 6 * col.spacing + 28
        + (errBox.visible ? errBox.implicitHeight + col.spacing : 0)
        + (measBox.visible ? measBox.implicitHeight : 0)
    signal openSettings(string tab)
    // Start pressed: the dashboard checks the space on the disk first
    signal startAsked()

    BoardStatus { id: s; b: root.b }
    readonly property var stats: b ? b.stats : ({})
    // the recording (streaming_improvements 1.5): dest "pc" or "sd" (the board's RecordStatus).
    // The counters of the board's card show the last run to the card only, not one to this PC.
    readonly property var rec: b ? b.record : ({})
    readonly property bool onSd: rec.dest === "sd" && rec.mode === 2
    readonly property bool sdDest: rec.dest === "sd"
    // samples a second (the board's, else the sample rate) and bytes a second of the data
    readonly property double recRate: rec.rate > 0 ? rec.rate : s.perChannel
    // bytes a second of the data as the file gets them (volts are floats, BIN keeps the codes)
    readonly property double dataRate: rec.dataRate || 0
    // what goes to the disk a second: the data, or the disk when it is slower (the measured speed
    // alone overstates a disk the data do not keep busy)
    readonly property double thru: rec.write > 0 && dataRate > 0 ? Math.min(rec.write, dataRate) : dataRate > 0 ? dataRate : (rec.write || 0)
    // before a start to this computer (streaming_improvements 2.4): what the limit takes of the disk
    readonly property bool pcBefore: rec.dest === "pc" && !rec.running && rec.total > 0
    readonly property double need: rec.need || 0
    readonly property bool needShort: pcBefore && need > rec.free
    readonly property string needText: !pcBefore ? ""
        : need > 0 ? "The limit takes about " + Theme.bytes(need) + (needShort ? ", more than is free: the recording stops when the disk is full, at about " + Math.floor(rec.free / need * 100) + "% of the limit" : "")
        : rec.dataRate > 0 ? "Full in about " + dur(rec.free / rec.dataRate) + " at " + Theme.bytes(rec.dataRate) + "/s (the rate of these settings)" : ""
    function dur(sec) { if (!(sec >= 0) || !isFinite(sec)) return "-"; sec = Math.round(sec); return sec < 90 ? sec + " s" : sec < 5400 ? Math.round(sec / 60) + " min" : Math.floor(sec / 3600) + " h " + String(Math.round(sec % 3600 / 60)).padStart(2, "0") + " min" }
    readonly property real recFrac: rec.limit ? Math.min(1, (rec.samples || 0) / rec.limit) : (rec.total ? 1 - rec.free / rec.total : 0)
    readonly property string recText: !rec.dest ? "" : sdDest && !rec.known ? "The board does not report its recording (a streaming server older than 2026-11-18)" : rec.running
        ? (rec.limit ? Math.round(recFrac * 100) + "% of the limit · " + Theme.count(rec.samples) + " of " + Theme.count(rec.limit) + " · about " + dur((rec.limit - rec.samples) / recRate) + " left"
                     : (sdDest ? "SD card" : "PC disk") + " full in about " + dur(rec.free / thru) + " at " + Theme.bytes(thru) + "/s")
          + " · " + Theme.bytes(rec.free) + " free of " + Theme.bytes(rec.total)
        : (sdDest ? "SD card: " : "PC disk: ") + Theme.bytes(rec.free) + " free of " + Theme.bytes(rec.total)

    color: Theme.card
    border.color: s.hasError && !s.running ? Theme.danger : Theme.border
    radius: Theme.radius
    implicitHeight: col.implicitHeight + 28
    opacity: s.online ? 1 : 0.75

    ColumnLayout {
        id: col
        x: 16; y: 14
        width: root.width - 32
        spacing: 12

        // name, model, address, role, state
        RowLayout {
            id: hdr
            spacing: 12
            Layout.fillWidth: true
            Image {
                source: s.running ? "qrc:/img/green_led.png" : "qrc:/img/red_led.png"
                sourceSize: Qt.size(44, 44)
                Layout.preferredWidth: 22; Layout.preferredHeight: 22
                Layout.alignment: Qt.AlignTop
            }
            ColumnLayout {
                spacing: 1
                Layout.fillWidth: true
                Text { text: b ? b.name : ""; color: Theme.title; font.pixelSize: Theme.fsLg; font.bold: true; elide: Text.ElideRight; Layout.fillWidth: true }
                Label2 { kind: "hint"; text: (b ? b.model : "") + " · " + (b ? b.ip : ""); wrapMode: Text.NoWrap; elide: Text.ElideRight; Layout.fillWidth: true }
                Label2 { kind: "hint"; text: b && b.manual ? "Added by IP" : "Found by broadcast"; wrapMode: Text.NoWrap; Layout.fillWidth: true }
            }
            Badge { visible: boards.count > 1 && s.connected; text: b && b.isMaster ? "Master" : "Slave"; kind: "grey"; Layout.alignment: Qt.AlignTop }
            Badge { text: s.stateText; kind: s.stateKind; Layout.alignment: Qt.AlignTop }
        }

        // the last error (streaming_improvements 2.2): until a start that goes or the cross
        Rectangle {
            id: errBox
            visible: s.hasError
            Layout.fillWidth: true
            implicitHeight: errRow.implicitHeight + 16
            color: Theme.redBg
            radius: Theme.radius
            Accessible.role: Accessible.AlertMessage
            Accessible.name: s.err.text || ""
            RowLayout {
                id: errRow
                x: 10; y: 8
                width: parent.width - 20
                spacing: 10
                Image { source: "qrc:/img/icons/warning.svg"; sourceSize: Qt.size(16, 16); Layout.alignment: Qt.AlignTop; Layout.topMargin: 1 }
                ColumnLayout {
                    spacing: 2
                    Layout.fillWidth: true
                    Text { text: s.err.text || ""; color: Theme.dangerText; font.pixelSize: Theme.fsMd; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    Text { text: (s.err.time || "") + (s.err.count > 1 ? " · the last of " + s.err.count + " errors" : ""); color: Theme.muted; font.pixelSize: Theme.fsXs; Layout.fillWidth: true }
                }
                Btn { kind: "outline"; compact: true; text: "Console"; visible: b && b.chartEnable; onClicked: b.chartEnable = false; Layout.alignment: Qt.AlignTop }
                Btn { kind: "outline"; compact: true; iconSource: "qrc:/img/icons/close.svg"; implicitWidth: implicitHeight; Accessible.name: "Dismiss the error"; onClicked: b.dismissError(); Layout.alignment: Qt.AlignTop }
            }
        }

        // the streams
        Flow {
            id: chips
            Layout.fillWidth: true
            spacing: 6
            StreamChip { name: "ADC"; text: s.adcText; active: s.running; amber: s.st === "armed" || s.st === "starting" }
        }

        // preview
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: pvHead.height + (root.big ? root.bigChartHeight : 180) + 1 + (measBox.visible ? measBox.implicitHeight : 0)
            color: Theme.plot
            border.color: Theme.border
            radius: Theme.radius
            clip: true
            RowLayout {
                id: pvHead
                x: 8; y: 0
                width: parent.width - 16
                height: 36 + Theme.fsAdd
                spacing: 10
                Seg {
                    small: true
                    options: ["Signal", "Console"]
                    currentIndex: b && b.chartEnable ? 0 : 1
                    onPicked: (i) => b.chartEnable = i === 0
                }
                Seg {
                    visible: b && b.chartEnable
                    small: true
                    options: ["Full Y", "Fit Y"]
                    currentIndex: b && b.previewFit ? 1 : 0
                    onPicked: (i) => b.previewFit = i === 1
                }
                Select {
                    visible: b && b.chartEnable
                    implicitHeight: 24 + Theme.fsAdd
                    implicitWidth: 150 + 6 * Theme.fsAdd
                    font.pixelSize: Theme.fsSm
                    options: Theme.previewDecimations
                    value: b ? b.previewDecimation : 1
                    onPicked: (x) => b.previewDecimation = Number(x)
                }
                Btn {
                    visible: b && b.chartEnable
                    kind: "outline"; compact: true
                    text: "Measure"; iconSource: "qrc:/img/icons/measure.svg"
                    active: !!(b && b.measure)
                    Accessible.checkable: true
                    Accessible.checked: active
                    onClicked: b.measure = !b.measure
                }
                Item { Layout.fillWidth: true }
                Repeater {
                    model: b && b.chartEnable ? s.adcCount : 0
                    delegate: Row {
                        required property int index
                        spacing: 5
                        opacity: s.inputsOn.indexOf("IN" + (index + 1)) >= 0 ? 1 : 0.35
                        Rectangle { width: 12; height: 3; radius: 2; color: Theme.channelColors[index]; anchors.verticalCenter: parent.verticalCenter }
                        Text { text: "IN" + (index + 1); color: Theme.text; font.pixelSize: Theme.fsXs }
                    }
                }
            }
            Rectangle { y: pvHead.height; width: parent.width; height: 1; color: Theme.borderSoft }
            Item {
                x: 1; y: pvHead.height + 1
                width: parent.width - 2
                height: parent.height - pvHead.height - 2 - (measBox.visible ? measBox.implicitHeight : 0)
                Preview {
                    id: preview
                    anchors.fill: parent
                    visible: b && b.chartEnable
                    board: root.b
                    channels: s.adcCount
                    fit: !!(b && b.previewFit)
                    decimation: b ? b.previewDecimation : 1
                    sampleRate: s.perChannel
                    blockSamples: s.blockSamples
                    fullScales: s.fullScales
                    inputsOn: s.inputsMask
                }
                Rectangle {
                    anchors.centerIn: parent
                    visible: preview.visible && pvMsg.text !== ""
                    width: pvMsg.implicitWidth + 24
                    height: pvMsg.implicitHeight + 12
                    color: Qt.rgba(43 / 255, 43 / 255, 42 / 255, 0.92)
                    border.color: Theme.border
                    radius: Theme.radius
                    Text {
                        id: pvMsg
                        anchors.centerIn: parent
                        color: Theme.text
                        font.pixelSize: Theme.fsSm
                        text: !s.connected ? (s.linkText !== "" ? s.linkText : "The board does not answer")
                            : s.st === "sd" ? "The board writes its SD card: no preview"
                            : s.st === "armed" ? "Waiting for the trigger"
                            : !s.running && !preview.hasData ? "Press Start to see the signal"
                            : s.running && !preview.hasData ? "Waiting for data"
                            : ""
                    }
                }
                ListView {
                    id: con
                    anchors.fill: parent
                    anchors.margins: 8
                    visible: b && !b.chartEnable
                    clip: true
                    model: b ? b.getConsoleModel() : null
                    delegate: Text {
                        required property string con_text
                        required property int con_level
                        width: con.width
                        text: con_text
                        color: con_level === 0 ? Theme.dangerText : con_level === 1 ? Theme.amberText : con_level === 3 ? Theme.muted : Theme.text
                        font.family: Theme.mono
                        font.pixelSize: Theme.fsXs
                        wrapMode: Text.WrapAnywhere
                    }
                    onCountChanged: positionViewAtEnd()
                    ScrollBar.vertical: ScrollBar {}
                }
            }
            // the measurements of the last block (streaming_improvements 4.4)
            Column {
                id: measBox
                visible: !!(b && b.measure && b.chartEnable)
                anchors.bottom: parent.bottom
                x: 12
                width: parent.width - 24
                bottomPadding: 6
                property var meas: []
                Timer {
                    interval: 500
                    running: measBox.visible
                    repeat: true
                    triggeredOnStart: true
                    onTriggered: measBox.meas = cdh.getMeasure(b.ip)
                }
                readonly property var rows: {
                    const r = []
                    for (let c = 0; c < s.adcCount; c++) {
                        const m = meas[c]
                        if (!s.inputsMask[c] || !m || !m.valid) continue
                        const fs = s.fullScales[c] || 1
                        r.push({ c: c, f: m.period > 0 ? s.perChannel / m.period : 0, pp: (m.max - m.min) * fs, min: m.min * fs, max: m.max * fs,
                                 mean: m.mean * fs, rms: m.rms * fs, clipped: m.clipped, samples: m.samples })
                    }
                    return r
                }
                Rectangle { width: parent.width + 24; x: -12; height: 1; color: Theme.borderSoft }
                Item { width: 1; height: 6 }
                GridLayout {
                    width: parent.width
                    columns: 7
                    columnSpacing: 12
                    rowSpacing: 3
                    Repeater {
                        model: ["input", "frequency", "peak-peak", "min", "max", "mean", "RMS"]
                        delegate: Text { required property string modelData; text: modelData; color: Theme.muted; font.pixelSize: Theme.fsXs; Layout.fillWidth: true; Layout.preferredWidth: 10 }
                    }
                    Repeater {
                        model: measBox.rows.length * 7
                        delegate: Text {
                            required property int index
                            readonly property var row: measBox.rows[Math.floor(index / 7)]
                            readonly property int col: index % 7
                            text: !row ? "" : col === 0 ? "IN" + (row.c + 1) : col === 1 ? Theme.freq(row.f) : col === 2 ? Theme.volts(row.pp)
                                : col === 3 ? Theme.volts(row.min) : col === 4 ? Theme.volts(row.max) : col === 5 ? Theme.volts(row.mean) : Theme.volts(row.rms)
                            color: !row ? Theme.value : col === 0 ? ["#f3ec1a", "#5fcf72", "#e0a060", "#e07ddc"][row.c]
                                : row.clipped && col >= 2 && col <= 4 ? Theme.dangerText : col === 1 && !(row.f > 0) ? Theme.muted : Theme.value
                            font.pixelSize: Theme.fsSm
                            font.family: col === 0 ? Qt.application.font.family : Theme.mono
                            font.bold: col === 0
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                            Layout.preferredWidth: 10
                            ToolTip.visible: tipArea.containsMouse && row && row.clipped && col >= 2 && col <= 4
                            ToolTip.text: "The input reached the end of its range: these are of the range, not of the signal"
                            MouseArea { id: tipArea; anchors.fill: parent; hoverEnabled: true; acceptedButtons: Qt.NoButton }
                        }
                    }
                }
                Text {
                    topPadding: 4
                    width: parent.width
                    elide: Text.ElideRight
                    color: Theme.muted
                    font.pixelSize: Theme.fsXs
                    text: measBox.rows.length ? "of the whole last block: " + measBox.rows[0].samples + " samples, " + preview.timeText(measBox.rows[0].samples / s.perChannel) + " · every 0.5 s"
                        : s.running ? "waiting for a block" : "measured while the board streams"
                }
            }
        }

        // counters
        GridLayout {
            id: counters
            Layout.fillWidth: true
            // one row on a wide single card, two otherwise: seven values do not fit a narrow row
            columns: root.big && root.width >= 1150 ? 7 : 4
            columnSpacing: 14
            rowSpacing: 8
            Kv { label: onSd ? "Written on the board" : "Received"; value: onSd ? Theme.bytes(rec.written || 0) : s.online ? Theme.bytes(stats.received || 0) : "-"; Layout.fillWidth: true; Layout.preferredWidth: 10 }
            Kv { label: onSd ? "SD card" : "Speed"; value: onSd ? (rec.running ? Theme.bytes(root.thru) + "/s · " + Math.round(rec.queue * 100) + " %" : Theme.bytes(rec.free) + " free") : s.online ? Theme.bytes(stats.speed || 0) + "/s" : "-"; valueColor: onSd && rec.running && rec.write > 0 && rec.write < root.dataRate ? Theme.dangerText : Theme.value; Layout.fillWidth: true; Layout.preferredWidth: 10 }
            Kv { label: "Samples per channel"; value: onSd ? Theme.count(rec.samples || 0) : s.online ? Theme.count(stats.samples || 0) : "-"; Layout.fillWidth: true; Layout.preferredWidth: 10 }
            Kv { label: "Lost on the board"; value: onSd ? Theme.count(rec.lostBoard || 0) : s.online ? Theme.count(stats.lostBoard || 0) : "-"; valueColor: (onSd ? rec.lostBoard : stats.lostBoard) > 0 ? Theme.dangerText : Theme.value; Layout.fillWidth: true; Layout.preferredWidth: 10 }
            Kv { label: onSd ? "Lost by the SD queue" : "Lost on this PC"; value: onSd ? Theme.count(rec.lostQueue || 0) : s.online ? Theme.count(stats.lostFile || 0) : "-"; valueColor: (onSd ? rec.lostQueue : stats.lostFile) > 0 ? Theme.dangerText : Theme.value; Layout.fillWidth: true; Layout.preferredWidth: 10 }
            // the write speed of the file on this PC and the fill of its queue in memory (half full: red, the data are lost once it is full)
            Kv { label: onSd ? "Time left" : "Disk write"; value: onSd ? (rec.running ? (rec.limit ? dur((rec.limit - rec.samples) / recRate) : "SD full in " + dur(rec.free / root.thru)) : (rec.limit && rec.samples >= rec.limit ? "done" : "stopped")) : s.online && stats.fileRate > 0 ? Theme.bytes(stats.fileRate) + "/s · " + Math.round((stats.fileQueue || 0) * 100) + " %" : "-"; valueColor: !onSd && stats.fileQueue > 0.5 ? Theme.dangerText : Theme.value; Layout.fillWidth: true; Layout.preferredWidth: 10 }
            Kv { label: "Trigger"; value: s.trigText; valueColor: s.trigNoSource ? Theme.dangerText : Theme.value; Layout.fillWidth: true; Layout.preferredWidth: 10 }
        }
        // the progress of the recording: the limit, else the disk or the SD card
        ColumnLayout {
            visible: !!rec.dest && (rec.total > 0 || (sdDest && !rec.known))
            Layout.fillWidth: true
            spacing: 4
            Rectangle {
                Layout.fillWidth: true
                height: 6
                radius: 3
                color: Theme.border
                // before a start to this computer: the disk used, then what the limit takes of it
                Rectangle { width: parent.width * (root.pcBefore ? 1 - rec.free / rec.total : root.recFrac); height: parent.height; radius: 3; color: rec.limit && !root.pcBefore ? Theme.accent : Theme.muted }
                Rectangle {
                    visible: root.pcBefore && root.need > 0
                    x: parent.width * (1 - rec.free / rec.total)
                    width: Math.max(2, parent.width * Math.min(root.need, rec.free) / rec.total)
                    height: parent.height
                    color: root.needShort ? Theme.amber : Theme.accent
                }
            }
            Text { text: root.recText; color: rec.running && rec.write > 0 && rec.write < root.dataRate ? Theme.dangerText : Theme.muted; font.pixelSize: Theme.fsSm; Layout.fillWidth: true; elide: Text.ElideRight }
            Text { visible: root.needText !== ""; text: root.needText; color: root.needShort ? Theme.amberText : Theme.muted; font.pixelSize: Theme.fsSm; Layout.fillWidth: true; wrapMode: Text.WordWrap }
        }
        RowLayout {
            id: fileRow
            Layout.fillWidth: true
            spacing: 8
            Label2 { text: "File"; kind: "hint"; wrapMode: Text.NoWrap }
            Text {
                text: sdDest ? (rec.file || "/home/redpitaya/streaming_files/adc") + " (on the board)" : stats.file || (b ? b.recordDir() : "")
                color: stats.file ? Theme.value : Theme.muted
                font.family: Theme.mono
                font.pixelSize: Theme.fsSm
                elide: Text.ElideMiddle
                Layout.fillWidth: true
            }
        }

        // buttons
        RowLayout {
            id: btnRow
            Layout.fillWidth: true
            spacing: 8
            visible: s.online
            Btn {
                visible: s.connected || s.running
                kind: s.running ? "danger" : "primary"
                text: s.running ? "Stop" : "Start"
                enabled: s.connected && s.st !== "stopping"
                onClicked: s.running ? b.stopStreaming() : root.startAsked()
            }
            Btn {
                visible: !s.connected && !s.running
                kind: s.link === "taken" ? "primary" : "outline"
                text: "Reconnect"
                onClicked: b.reconnect()
            }
            Btn {
                kind: "outline"; text: "Trigger"; iconSource: "qrc:/img/icons/bolt.svg"
                enabled: s.running && s.trigOn
                onClicked: b.softwareTrigger()
            }
            Item { Layout.fillWidth: true }
            Btn {
                kind: "outline"; text: "Settings"; iconSource: "qrc:/img/icons/sliders.svg"
                enabled: s.connected
                onClicked: root.openSettings("adc")
            }
        }
        RowLayout {
            id: offRow
            Layout.fillWidth: true
            spacing: 8
            visible: !s.online
            Btn { kind: "outline"; text: "Reconnect"; onClicked: b.reconnect() }
            Item { Layout.fillWidth: true }
            Btn {
                kind: "outline"; text: "Remove from the list"
                onClicked: app.isManualHost(b.ip) ? app.removeManualHost(b.ip) : boards.removeBoard(b.ip)
            }
        }
    }

    component StreamChip: Rectangle {
        id: chip
        property string name: ""
        property string text: ""
        property bool active: false
        property bool amber: false
        property bool dim: false
        implicitHeight: 24 + Theme.fsAdd
        implicitWidth: Math.min(chipRow.implicitWidth + 18, col.width)
        radius: Theme.radius
        color: Qt.rgba(0, 0, 0, 0.12)
        border.color: Qt.rgba(153 / 255, 153 / 255, 153 / 255, 0.5)
        opacity: dim ? 0.55 : 1
        clip: true
        Row {
            id: chipRow
            x: 9
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6
            Dot { anchors.verticalCenter: parent.verticalCenter; color: chip.active ? (chip.amber ? Theme.amber : Theme.ok) : "#6a6a68" }
            Text { text: chip.name; color: Theme.value; font.bold: true; font.pixelSize: Theme.fsSm }
            Text { text: chip.text; color: Theme.text; font.pixelSize: Theme.fsSm; elide: Text.ElideRight; width: Math.min(implicitWidth, col.width - 80) }
        }
    }
}
