import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "ui"

// A board as a row of the list (.srow); header: the column titles
Item {
    id: root
    property var b
    property bool header: false
    property bool last: false
    signal openSettings(string tab)
    signal startAsked()

    BoardStatus { id: s; b: root.b }
    TextMetrics { id: longestState; font.pixelSize: Theme.fsSm; font.bold: true; text: "Taken by another client" }
    readonly property var stats: b ? b.stats : ({})
    // a board writing its SD card has no preview: the progress of its recording takes the place
    readonly property var rec: b ? b.record : ({})
    readonly property bool onSd: rec.dest === "sd" && rec.mode === 2
    readonly property double recRate: rec.rate > 0 ? rec.rate : s.perChannel
    // bytes a second of the data as the file gets them (volts are floats, BIN keeps the codes)
    readonly property double dataRate: rec.dataRate || 0
    readonly property double thru: rec.write > 0 && dataRate > 0 ? Math.min(rec.write, dataRate) : dataRate > 0 ? dataRate : (rec.write || 0)
    readonly property real recFrac: rec.limit ? Math.min(1, (rec.samples || 0) / rec.limit) : (rec.total ? 1 - rec.free / rec.total : 0)
    function dur(sec) { if (!(sec >= 0) || !isFinite(sec)) return "-"; sec = Math.round(sec); return sec < 90 ? sec + " s" : sec < 5400 ? Math.round(sec / 60) + " min" : Math.floor(sec / 3600) + " h" }

    readonly property real rowHeight: header ? 28 + Theme.fsAdd : 56 + Theme.fsAdd
    implicitHeight: rowHeight + (errLine.visible ? errLine.implicitHeight + 8 : 0)

    RowLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: root.rowHeight
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 12

        // board
        RowLayout {
            Layout.preferredWidth: 260 + 8 * Theme.fsAdd; Layout.minimumWidth: 260 + 8 * Theme.fsAdd; Layout.maximumWidth: 260 + 8 * Theme.fsAdd
            spacing: 10
            Image {
                visible: !root.header
                source: s.running ? "qrc:/img/green_led.png" : "qrc:/img/red_led.png"
                sourceSize: Qt.size(44, 44)
                Layout.preferredWidth: 22; Layout.preferredHeight: 22
            }
            ColumnLayout {
                spacing: 1
                Layout.fillWidth: true
                Text { text: root.header ? "BOARD" : (b ? b.name : ""); color: root.header ? Theme.muted : Theme.value; font.pixelSize: root.header ? Theme.fsXs : Theme.fsBase; font.bold: !root.header; elide: Text.ElideRight; Layout.fillWidth: true }
                Label2 { visible: !root.header; kind: "hint"; text: (b ? b.model + " · " + b.ip : ""); wrapMode: Text.NoWrap; elide: Text.ElideRight; Layout.fillWidth: true }
            }
        }
        Item {
            Layout.preferredWidth: 80 + 4 * Theme.fsAdd; Layout.minimumWidth: 80 + 4 * Theme.fsAdd; Layout.maximumWidth: 80 + 4 * Theme.fsAdd
            implicitHeight: 24
            Text { visible: root.header; text: "ROLE"; color: Theme.muted; font.pixelSize: Theme.fsXs; anchors.verticalCenter: parent.verticalCenter }
            Text { visible: !root.header; text: s.connected ? (b.isMaster ? "Master" : "Slave") : "-"; color: Theme.value; font.pixelSize: Theme.fsMd; anchors.verticalCenter: parent.verticalCenter }
        }
        Item {
            // as wide as the longest state, the same in the header and in every row
            Layout.preferredWidth: longestState.width + 44; Layout.minimumWidth: longestState.width + 44; Layout.maximumWidth: longestState.width + 44
            implicitHeight: 24
            Text { visible: root.header; text: "STATE"; color: Theme.muted; font.pixelSize: Theme.fsXs; anchors.verticalCenter: parent.verticalCenter }
            Badge { id: stateBadge; visible: !root.header; text: s.stateText; kind: s.stateKind; anchors.verticalCenter: parent.verticalCenter }
        }
        Item {
            Layout.preferredWidth: 180
            Layout.fillWidth: true
            implicitHeight: 36
            Text { visible: root.header; text: "PREVIEW"; color: Theme.muted; font.pixelSize: Theme.fsXs; anchors.verticalCenter: parent.verticalCenter }
            Rectangle {
                visible: !root.header
                anchors.fill: parent
                color: Theme.plot
                border.color: Theme.borderSoft
                radius: Theme.radius
                Item {
                    visible: root.onSd
                    anchors.fill: parent
                    anchors.margins: 8
                    Rectangle { anchors.top: parent.top; width: parent.width; height: 6; radius: 3; color: Theme.border
                        Rectangle { width: parent.width * root.recFrac; height: parent.height; radius: 3; color: rec.limit ? Theme.accent : Theme.muted } }
                    Text { anchors.bottom: parent.bottom; text: rec.running ? (rec.limit ? Math.round(root.recFrac * 100) + "% · " + root.dur((rec.limit - rec.samples) / root.recRate) + " left" : "SD full in " + root.dur(rec.free / root.thru)) : Theme.bytes(rec.free) + " free on the SD card"; color: Theme.muted; font.pixelSize: Theme.fsXs }
                }
                Preview {
                    visible: !root.onSd
                    id: rowPreview
                    anchors.fill: parent
                    anchors.margins: 1
                    board: root.b
                    grid: false
                    channels: s.adcCount
                    // the list has no console: the signal always
                    active: !root.header && !!b
                    fit: !!(b && b.previewFit)
                    decimation: b ? b.previewDecimation : 1
                    sampleRate: s.perChannel
                    blockSamples: s.blockSamples
                }
                // a click switches the Y range: full or fitted to the signal
                MouseArea {
                    id: pvMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: b.previewFit = !b.previewFit
                    ToolTip.visible: containsMouse
                    ToolTip.delay: 600
                    ToolTip.text: (rowPreview.windowSec > 0 ? rowPreview.timeText(rowPreview.windowSec) + " across · " : "")
                        + "Y " + (b && b.previewFit ? rowPreview.scaleText : "Full")
                        + "\nClick: " + (b && b.previewFit ? "the full range of the inputs" : "fit Y to the signal")
                }
            }
        }
        Cell { title: "SPEED"; value: onSd ? (rec.running ? Theme.bytes(root.thru) + "/s" : "-") : s.online ? Theme.bytes(stats.speed || 0) + "/s" : "-"; Layout.preferredWidth: 100 + 6 * Theme.fsAdd; Layout.minimumWidth: 100 + 6 * Theme.fsAdd; Layout.maximumWidth: 100 + 6 * Theme.fsAdd }
        Cell { title: "RECEIVED"; value: onSd ? Theme.bytes(rec.written || 0) : s.online ? Theme.bytes(stats.received || 0) : "-"; Layout.preferredWidth: 100 + 6 * Theme.fsAdd; Layout.minimumWidth: 100 + 6 * Theme.fsAdd; Layout.maximumWidth: 100 + 6 * Theme.fsAdd }
        Cell { title: "LOST"; value: onSd ? Theme.count((rec.lostBoard || 0) + (rec.lostQueue || 0)) : s.online ? Theme.count((stats.lostBoard || 0) + (stats.lostFile || 0)) : "-"; Layout.preferredWidth: 70 + 4 * Theme.fsAdd; Layout.minimumWidth: 70 + 4 * Theme.fsAdd; Layout.maximumWidth: 70 + 4 * Theme.fsAdd }
        Cell { title: "QUEUE"; value: onSd ? (rec.running ? Math.round(rec.queue * 100) + " %" : "-") : s.online && stats.fileRate > 0 ? Math.round((stats.fileQueue || 0) * 100) + " %" : "-"; valueColor: stats.fileQueue > 0.5 ? Theme.dangerText : Theme.value; tip: stats.fileRate > 0 ? "Disk write " + Theme.bytes(stats.fileRate) + "/s" : ""; Layout.preferredWidth: 60 + 4 * Theme.fsAdd; Layout.minimumWidth: 60 + 4 * Theme.fsAdd; Layout.maximumWidth: 60 + 4 * Theme.fsAdd }
        Row {
            Layout.preferredWidth: 130 + 8 * Theme.fsAdd; Layout.minimumWidth: 130 + 8 * Theme.fsAdd; Layout.maximumWidth: 130 + 8 * Theme.fsAdd
            layoutDirection: Qt.RightToLeft
            spacing: 8
            // takes its place in the header too: the columns line up
            opacity: root.header ? 0 : 1
            enabled: !root.header
            Btn {
                visible: s.online
                kind: "outline"; iconSource: "qrc:/img/icons/sliders.svg"
                implicitWidth: implicitHeight
                Accessible.name: (b ? b.name : "") + " settings"
                enabled: s.connected
                onClicked: root.openSettings("adc")
            }
            Btn {
                visible: s.online && (s.connected || s.running)
                kind: s.running ? "danger" : "primary"
                text: s.running ? "Stop" : "Start"
                implicitWidth: 72
                enabled: s.connected && s.st !== "stopping"
                onClicked: s.running ? b.stopStreaming() : root.startAsked()
            }
            Btn { visible: !s.online || !s.connected; kind: s.link === "taken" ? "primary" : "outline"; text: "Reconnect"; onClicked: b.reconnect() }
        }
    }
    // the last error of the board (streaming_improvements 2.2); the list has no console: the link
    // opens the cards on it
    RowLayout {
        id: errLine
        visible: !root.header && s.hasError
        x: 10 + 32
        y: root.rowHeight - 4
        width: parent.width - 52
        spacing: 10
        Text { text: (s.err.text || "") + " · " + (s.err.time || ""); color: Theme.dangerText; font.pixelSize: Theme.fsSm; elide: Text.ElideRight; Layout.maximumWidth: errLine.width - errLink.implicitWidth - 20 }
        Text {
            id: errLink
            text: "<a href=\"console\">Open the console</a>"
            textFormat: Text.StyledText
            linkColor: Theme.accent
            font.pixelSize: Theme.fsSm
            onLinkActivated: { b.chartEnable = false; app.view = "cards" }
            HoverHandler { cursorShape: Qt.PointingHandCursor }
        }
        Item { Layout.fillWidth: true }
    }
    Rectangle {
        visible: !root.last
        anchors.bottom: parent.bottom
        x: 10; width: parent.width - 20; height: 1
        color: root.header ? "#777777" : Theme.borderSoft
    }

    component Cell: Item {
        id: cell
        property string title: ""
        property string value: ""
        property color valueColor: Theme.value
        property string tip: ""
        implicitHeight: 24
        Text { visible: root.header; text: parent.title; color: Theme.muted; font.pixelSize: Theme.fsXs; anchors.verticalCenter: parent.verticalCenter }
        MouseArea { id: cellMouse; anchors.fill: parent; hoverEnabled: cell.tip !== ""; acceptedButtons: Qt.NoButton }
        ToolTip.visible: cellMouse.containsMouse && cell.tip !== "" && !root.header
        ToolTip.delay: 600
        ToolTip.text: cell.tip
        Text { visible: !root.header; text: parent.value; color: parent.valueColor; font.family: Theme.mono; font.pixelSize: Theme.fsMd; anchors.verticalCenter: parent.verticalCenter; width: parent.width; elide: Text.ElideRight }
    }
}
