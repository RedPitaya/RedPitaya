import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "ui"
import "tabs"

// The settings of a board: the tabs of the web page of the board, and This PC.
Dialog2 {
    id: root
    property var b: null
    property string tab: "adc"
    BoardStatus { id: s; b: root.b }

    width: Math.min(1240, Overlay.overlay ? Overlay.overlay.width - 48 : 1240)
    height: Math.min(1100, maxHeight)
    title: (b ? b.name : "") + " settings"
    subtitle: b ? b.model + " · " + b.ip + (s.connected ? " · " + (b.isMaster ? "Master" : "Slave") : "") : ""

    // for the UI script of the tests: the end of a long tab
    function scrollToEnd() { sv.ScrollBar.vertical.position = 1.0 - sv.ScrollBar.vertical.size }

    function openFor(board, t) {
        b = board
        tab = t || "adc"
        open()
    }

    // the board went away: nothing to set
    Connections {
        target: boards
        function onCountChanged() {
            if (root.opened && root.b && !boards.boardByIp(root.b.ip)) root.close()
        }
    }

    header: [
        Image {
            source: s.running ? "qrc:/img/green_led.png" : "qrc:/img/red_led.png"
            sourceSize: Qt.size(44, 44)
            width: 22; height: 22
            anchors.verticalCenter: parent ? parent.verticalCenter : undefined
        }
    ]

    readonly property var tabs: [
        ["adc", "ADC", true],
        ["mem", "Memory", true],
        ["set", "Board", true]
    ]

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // sections
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 210
            color: Theme.card
            Rectangle { anchors.right: parent.right; width: 1; height: parent.height; color: "#777777" }
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                anchors.topMargin: 10
                spacing: 2
                Repeater {
                    model: root.tabs.filter(t => t[2])
                    delegate: NavBtn {
                        required property var modelData
                        text: modelData[1]
                        current: root.tab === modelData[0]
                        Layout.fillWidth: true
                        onClicked: root.tab = modelData[0]
                    }
                }
                Rectangle { Layout.fillWidth: true; Layout.topMargin: 8; Layout.bottomMargin: 8; height: 1; color: Theme.border }
                NavBtn {
                    text: "This PC"
                    current: root.tab === "pc"
                    Layout.fillWidth: true
                    onClicked: root.tab = "pc"
                }
                Item { Layout.fillHeight: true }
                Label2 {
                    kind: "hint"
                    Layout.fillWidth: true
                    Layout.margins: 4
                    text: "The board tabs change the board at once, as its web page does. This PC is kept on this computer."
                }
            }
        }

        ScrollView {
            id: sv
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: availableWidth
            contentHeight: page.height + 42
            clip: true
            Loader {
                id: page
                x: 20
                y: 18
                width: sv.availableWidth - 40
                active: root.b !== null && root.opened
                sourceComponent: root.tab === "mem" ? memTab : root.tab === "set" ? setTab : root.tab === "pc" ? pcTab : adcTab
            }
        }
    }

    footer: [
        Btn { kind: "outline"; text: "Read from the board"; enabled: s.connected; onClicked: b.readSettings() },
        Label2 {
            kind: "hint"
            Layout.fillWidth: true
            text: !s.connected ? "Not connected: the settings cannot change." : s.running ? "Some settings stop the stream when they change (memory, block size)." : ""
        },
        Btn { kind: "primary"; text: "Done"; onClicked: root.close() }
    ]

    Component { id: adcTab; AdcTab { b: root.b } }
    Component { id: memTab; MemoryTab { b: root.b } }
    Component { id: setTab; BoardTab { b: root.b } }
    Component { id: pcTab; PcTab { b: root.b } }
}
