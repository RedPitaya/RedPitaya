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
    signal openSettings(string tab)

    BoardStatus { id: s; b: root.b }
    readonly property var stats: b ? b.stats : ({})

    color: Theme.card
    border.color: Theme.border
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
            implicitHeight: pvHead.height + (root.big ? root.bigChartHeight : 180) + 1
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
                height: parent.height - pvHead.height - 2
                Preview {
                    id: preview
                    anchors.fill: parent
                    visible: b && b.chartEnable
                    board: root.b
                    channels: s.adcCount
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
        }

        // counters
        GridLayout {
            id: counters
            Layout.fillWidth: true
            columns: root.big ? 6 : 3
            columnSpacing: 14
            rowSpacing: 8
            Kv { label: "Received"; value: s.online ? Theme.bytes(stats.received || 0) : "-"; Layout.fillWidth: true; Layout.preferredWidth: 10 }
            Kv { label: "Speed"; value: s.online ? Theme.bytes(stats.speed || 0) + "/s" : "-"; Layout.fillWidth: true; Layout.preferredWidth: 10 }
            Kv { label: "Samples per channel"; value: s.online ? Theme.count(stats.samples || 0) : "-"; Layout.fillWidth: true; Layout.preferredWidth: 10 }
            Kv { label: "Lost on the board"; value: s.online ? Theme.count(stats.lostBoard || 0) : "-"; valueColor: stats.lostBoard > 0 ? Theme.dangerText : Theme.value; Layout.fillWidth: true; Layout.preferredWidth: 10 }
            Kv { label: "Lost on this PC"; value: s.online ? Theme.count(stats.lostFile || 0) : "-"; valueColor: stats.lostFile > 0 ? Theme.dangerText : Theme.value; Layout.fillWidth: true; Layout.preferredWidth: 10 }
            Kv { label: "Trigger"; value: s.trigText; valueColor: s.trigNoSource ? Theme.dangerText : Theme.value; Layout.fillWidth: true; Layout.preferredWidth: 10 }
        }
        RowLayout {
            id: fileRow
            Layout.fillWidth: true
            spacing: 8
            Label2 { text: "File"; kind: "hint"; wrapMode: Text.NoWrap }
            Text {
                text: stats.file || (b ? b.recordDir() : "")
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
                onClicked: s.running ? b.stopStreaming() : b.startStreaming()
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
