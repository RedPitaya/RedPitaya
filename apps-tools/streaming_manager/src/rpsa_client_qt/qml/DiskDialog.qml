import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "ui"

// Start or Start all whose recordings with a limit do not fit the disk of their folder
// (streaming_improvements 2.4): asked before the start, the recordings stop by themselves at a
// full disk anyway
Dialog2 {
    id: root
    width: Math.min(600, Overlay.overlay ? Overlay.overlay.width - 48 : 600)
    height: Math.min(col.implicitHeight + 40 + chromeHeight, maxHeight)
    title: "Not enough space on this PC"
    // the board of the Start, null for Start all
    property var board: null
    property var shortage: ({})

    function go() {
        if (board)
            board.startStreaming()
        else
            boards.runAll()
    }

    // starts at once when the recordings fit
    function start(b) {
        board = b
        shortage = boards.diskShortage(b ? b.ip : "")
        if (!shortage.need) {
            go()
            return
        }
        open()
    }

    ColumnLayout {
        id: col
        x: 20; y: 18
        width: parent.width - 40
        spacing: 12
        Text {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            textFormat: Text.StyledText
            color: Theme.text
            font.pixelSize: Theme.fsMd
            text: "The recordings of this start need about <b>" + Theme.bytes(root.shortage.need || 0) + "</b>; the disk of their folder ("
                  + (root.shortage.disk || "") + ") has <b>" + Theme.bytes(root.shortage.free || 0) + "</b> free."
        }
        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 16
            rowSpacing: 4
            Repeater {
                model: (root.shortage.boards || []).length * 2
                delegate: Text {
                    required property int index
                    readonly property var row: root.shortage.boards[Math.floor(index / 2)]
                    text: index % 2 ? Theme.bytes(row.need)
                        : row.name + " · " + (row.running ? "already recording, the rest of " : "") + Theme.count(row.samples) + " samples × " + row.inputs + (row.inputs === 1 ? " input" : " inputs")
                    color: index % 2 ? Theme.value : Theme.muted
                    font.pixelSize: Theme.fsSm
                    Layout.fillWidth: index % 2 === 0
                    Layout.alignment: index % 2 ? Qt.AlignRight : Qt.AlignLeft
                }
            }
        }
        Label2 {
            kind: "hint"
            Layout.fillWidth: true
            text: "A recording stops by itself when the disk is full; its file is closed and stays readable."
        }
    }

    footer: [
        Item { Layout.fillWidth: true },
        Btn { kind: "outline"; text: "Cancel"; onClicked: root.close() },
        Btn { kind: "primary"; text: "Start anyway"; onClicked: { root.close(); root.go() } }
    ]
}
