import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "ui"

// Save the settings of a board as a profile (streaming_improvements 4.9): its name, the groups
// it holds. Opened from the Board tab of a board, or from the profiles (Save again from a board:
// the board is chosen here).
Dialog2 {
    id: root
    property var board: null
    property string name: ""
    property var chosen: ({})
    property string error: ""
    // the board is chosen here
    property bool pick: false
    signal saved(string name)

    // the boards whose settings are read
    property var ready: []
    function readBoards() {
        const r = []
        for (let i = 0; i < boards.count; i++) {
            const b = boards.board(i)
            if (b && b.isConfigConnected && b.settingsLoaded) r.push(b)
        }
        ready = r
    }
    // g: the groups to tick (Save again: those of the profile)
    function ask(b, n, g) {
        readBoards()
        pick = !b
        board = b || (ready.length ? ready[0] : null)
        name = n || ""
        error = ""
        const c = {}
        for (const x of (g && g.length ? g : ["inputs", "trigger", "destination"])) c[x] = true
        chosen = c
        open()
    }
    // what the board has now, a group a row
    readonly property var groups: { if (!board) return []; board.settingsRev; return profiles.groupsOf(board) }
    readonly property bool replaces: profiles.idOf(name) !== ""

    width: Math.min(640, Overlay.overlay ? Overlay.overlay.width - 48 : 640)
    height: Math.min(saveCol.implicitHeight + 40 + chromeHeight, maxHeight)
    title: "Save as a profile"
    subtitle: board ? "the settings of " + board.name + " · " + board.model + " as they are now" : "no board with its settings read"

    ColumnLayout {
        id: saveCol
        x: 20; y: 18
        width: parent.width - 40
        spacing: 12
        Field {
            visible: root.pick
            label: "From the board"
            Layout.fillWidth: true
            Select {
                implicitWidth: 320
                options: root.ready.map(b => [b.ip, b.name + " · " + b.model])
                value: root.board ? root.board.ip : ""
                onPicked: (ip) => root.board = boards.boardByIp(ip)
            }
        }
        Field {
            label: "Name"
            Layout.fillWidth: true
            hint: root.replaces ? "A profile of this name is there: it is replaced" : ""
            hintWarn: root.replaces
            TextIn { free: true; Layout.fillWidth: true; text: root.name; onTextChanged: root.name = text; onAccepted: root.go() }
        }
        Field {
            label: "What it holds"
            Layout.fillWidth: true
            Repeater {
                model: root.groups
                delegate: RowLayout {
                    required property var modelData
                    Layout.fillWidth: true
                    spacing: 0
                    Check {
                        Layout.alignment: Qt.AlignTop
                        text: modelData.label
                        checked: !!root.chosen[modelData.group]
                        onClicked: { const c = Object.assign({}, root.chosen); c[modelData.group] = checked; root.chosen = c }
                    }
                    Label2 { kind: "hint"; text: modelData.summary; Layout.fillWidth: true; Layout.leftMargin: 12; Layout.topMargin: 2 }
                }
            }
        }
        Label2 {
            kind: "hint"
            Layout.fillWidth: true
            text: "Not kept: the files on the SD card (they are on this board), IP and name. A group left out stays as it is on a board the profile is applied to."
        }
        Label2 { visible: root.error !== ""; kind: "bad"; text: root.error; Layout.fillWidth: true }
    }
    function go() {
        const g = root.groups.map(x => x.group).filter(x => root.chosen[x])
        error = profiles.save(board, name, g)
        if (error === "") { saved(name.trim()); close() }
    }
    footer: [
        Item { Layout.fillWidth: true },
        Btn { kind: "outline"; text: "Cancel"; onClicked: root.close() },
        Btn { kind: "primary"; text: "Save"; enabled: !!root.board && root.name.trim() !== ""; onClicked: root.go() }
    ]
}
