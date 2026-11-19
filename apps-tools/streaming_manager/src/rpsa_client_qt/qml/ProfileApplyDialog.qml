import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "ui"

// Apply a profile to boards (streaming_improvements 4.9): the boards chosen, what changes on each
// first (another model: amber notes), then the settings sent; a board takes them as Import JSON.
Dialog2 {
    id: root
    property string pid: ""
    // ip -> true
    property var picks: ({})
    // opened from a board: the profile is chosen here
    property bool pickProfile: false
    property string result: ""
    property bool resultBad: false
    // the boards (again when one comes or goes)
    property var boardList: []
    function readBoards() {
        const r = []
        for (let i = 0; i < boards.count; i++) if (boards.board(i)) r.push(boards.board(i))
        boardList = r
    }
    // what changes: read again now and then (settings that came), taken only when it differs, so
    // the lists keep their place
    property var pv: []
    property string pvKey: ""
    function refresh() {
        const n = pid !== "" ? profiles.preview(pid, ips) : []
        const k = JSON.stringify(n)
        if (k !== pvKey) { pvKey = k; pv = n }
    }
    onPidChanged: refresh()
    onIpsChanged: refresh()

    function ask(id, board) {
        readBoards()
        pickProfile = !id
        pid = id || (profiles.list.length ? profiles.list[0].id : "")
        const p = {}
        if (board) p[board.ip] = true
        else for (const b of boardList) if (b.isConfigConnected && b.settingsLoaded) p[b.ip] = true
        picks = p
        result = ""
        refresh()
        open()
    }

    readonly property var prof: profiles.list.find(p => p.id === pid) || ({})
    readonly property var ips: Object.keys(picks).filter(ip => picks[ip])
    readonly property var changing: pv.filter(b => b.ready && b.changes.length > 0)
    readonly property int stopping: changing.filter(b => b.streaming).length
    readonly property int same: pv.filter(b => b.ready && b.changes.length === 0).length
    Timer { interval: 1500; repeat: true; running: root.opened; onTriggered: root.refresh() }
    Connections { target: boards; function onCountChanged() { root.readBoards(); root.refresh() } }

    function pickAll(f) {
        const p = {}
        for (const b of boardList) if (b.isConfigConnected && b.settingsLoaded && f(b)) p[b.ip] = true
        picks = p
    }
    function go() {
        const e = profiles.apply(pid, changing.map(b => b.ip))
        const n = changing.length
        result = e !== "" ? e : "Sent to " + n + (n === 1 ? " board" : " boards") + ": they take the settings"
        resultBad = e !== ""
        refresh()
    }

    width: Math.min(1180, Overlay.overlay ? Overlay.overlay.width - 48 : 1180)
    height: Math.min(780, maxHeight)
    title: "Apply “" + (prof.name || "") + "”"
    subtitle: (prof.groups || "") + "; the other settings of a board stay as they are"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12
        Field {
            visible: root.pickProfile
            label: "Profile"
            Select {
                implicitWidth: 360
                options: profiles.list.map(p => [p.id, p.name])
                value: root.pid
                onPicked: (id) => { root.pid = id; root.result = "" }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            // the boards
            ColumnLayout {
                Layout.preferredWidth: 360
                Layout.maximumWidth: 360
                Layout.fillWidth: false
                Layout.fillHeight: true
                spacing: 8
                Label2 { text: "Boards"; kind: "label" }
                Text {
                    textFormat: Text.StyledText
                    linkColor: Theme.accent
                    color: Theme.muted
                    font.pixelSize: Theme.fsSm
                    text: "<a href=\"a\">all</a> · <a href=\"m\">of this model</a> · <a href=\"n\">none</a>"
                    onLinkActivated: (l) => { root.result = ""; l === "a" ? root.pickAll(b => true) : l === "m" ? root.pickAll(b => b.model === root.prof.model) : (root.picks = ({})) }
                    HoverHandler { cursorShape: Qt.PointingHandCursor }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: Theme.card
                    border.color: Theme.border
                    radius: Theme.radius
                    ListView {
                        id: boardsView
                        anchors.fill: parent
                        anchors.margins: 1
                        clip: true
                        model: root.boardList
                        ScrollBar.vertical: ScrollBar {}
                        delegate: Item {
                            id: brow
                            required property var modelData
                            readonly property bool usable: modelData.isConfigConnected && modelData.settingsLoaded
                            width: boardsView.width
                            height: 48 + Theme.fsAdd
                            opacity: usable ? 1 : 0.5
                            Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.borderSoft }
                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                spacing: 4
                                Check {
                                    enabled: brow.usable
                                    checked: brow.usable && !!root.picks[brow.modelData.ip]
                                    onClicked: { const p = Object.assign({}, root.picks); p[brow.modelData.ip] = checked; root.picks = p; root.result = "" }
                                    Accessible.name: brow.modelData.name
                                }
                                ColumnLayout {
                                    spacing: 1
                                    Layout.fillWidth: true
                                    Text { text: brow.modelData.name; color: Theme.value; font.pixelSize: Theme.fsBase; elide: Text.ElideRight; Layout.fillWidth: true }
                                    Text {
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                        textFormat: Text.StyledText
                                        color: Theme.muted
                                        font.pixelSize: Theme.fsSm
                                        text: brow.modelData.model + " · " + (!brow.modelData.isConfigConnected ? "not connected" : !brow.modelData.settingsLoaded ? "settings not read yet"
                                            : brow.modelData.isADCStarted ? "<font color=\"" + Theme.okText + "\">streaming</font>" : "idle")
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // what changes
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8
                Label2 { text: "What changes"; kind: "label" }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: Theme.card
                    border.color: Theme.border
                    radius: Theme.radius
                    Label2 { anchors.centerIn: parent; visible: root.pv.length === 0; kind: "hint"; text: "Tick the boards to apply it to" }
                    Flickable {
                        anchors.fill: parent
                        anchors.margins: 1
                        clip: true
                        contentHeight: changesCol.implicitHeight + 8
                        ScrollBar.vertical: ScrollBar {}
                        ColumnLayout {
                            id: changesCol
                            x: 14
                            width: parent.width - 28
                            spacing: 0
                            Repeater {
                                model: root.pv
                                delegate: ColumnLayout {
                                    id: one
                                    required property var modelData
                                    required property int index
                                    Layout.fillWidth: true
                                    Layout.topMargin: 10
                                    spacing: 5
                                    readonly property int n: modelData.ready ? modelData.changes.length : 0
                                    RowLayout {
                                        spacing: 10
                                        Text { text: one.modelData.name; color: Theme.value; font.bold: true; font.pixelSize: Theme.fsBase; Layout.preferredWidth: 120; elide: Text.ElideRight }
                                        Text {
                                            textFormat: Text.StyledText
                                            font.pixelSize: Theme.fsMd
                                            color: Theme.text
                                            text: !one.modelData.ready ? "<font color=\"" + Theme.dangerText + "\">not connected: nothing is sent</font>"
                                                : one.n === 0 ? "<font color=\"" + Theme.okText + "\">nothing: it has these settings</font>"
                                                : one.n + (one.n === 1 ? " change" : " changes") + (one.modelData.streaming ? " · <font color=\"" + Theme.amberText + "\">its stream stops</font>" : "")
                                                  + (one.modelData.notes.length ? " · <font color=\"" + Theme.amberText + "\">" + one.modelData.notes.length + (one.modelData.notes.length === 1 ? " note" : " notes") + ": " + one.modelData.model + "</font>" : "")
                                        }
                                    }
                                    GridLayout {
                                        visible: one.n > 0
                                        Layout.leftMargin: 130
                                        Layout.fillWidth: true
                                        columns: 4
                                        columnSpacing: 8
                                        rowSpacing: 3
                                        Repeater {
                                            model: one.modelData.ready ? one.modelData.changes : []
                                            delegate: Item {
                                                required property var modelData
                                                Layout.columnSpan: 4
                                                Layout.fillWidth: true
                                                implicitHeight: lbl.implicitHeight + 2
                                                Label2 { id: lbl; text: modelData.label; kind: "hint"; width: 140; elide: Text.ElideRight; wrapMode: Text.NoWrap }
                                                Text { x: 150; width: 200; text: modelData.from; color: Theme.text; font.family: Theme.mono; font.pixelSize: Theme.fsMd; elide: Text.ElideRight }
                                                Text { x: 356; text: "→"; color: Theme.muted; font.pixelSize: Theme.fsMd }
                                                Text { x: 380; width: parent.width - 380; text: modelData.to; color: Theme.value; font.family: Theme.mono; font.pixelSize: Theme.fsMd; elide: Text.ElideRight }
                                            }
                                        }
                                    }
                                    Repeater {
                                        model: one.modelData.ready ? one.modelData.notes : []
                                        delegate: Label2 {
                                            required property string modelData
                                            Layout.leftMargin: 130
                                            Layout.fillWidth: true
                                            kind: "warn"
                                            text: modelData
                                        }
                                    }
                                    Rectangle { Layout.fillWidth: true; Layout.topMargin: 5; height: 1; color: Theme.borderSoft; visible: one.index < root.pv.length - 1 }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    footer: [
        Label2 {
            Layout.fillWidth: true
            kind: root.result !== "" ? (root.resultBad ? "bad" : "hint") : root.stopping > 0 ? "warn" : "hint"
            wrapMode: Text.NoWrap
            elide: Text.ElideRight
            text: root.result !== "" ? root.result
                : (root.stopping > 0 ? root.stopping + (root.stopping === 1 ? " board stops" : " boards stop") + " streaming · " : "")
                  + root.changing.length + (root.changing.length === 1 ? " board changes" : " boards change")
                  + (root.same > 0 ? ", " + root.same + (root.same === 1 ? " has" : " have") + " nothing to change" : "")
        },
        Btn { kind: "outline"; text: root.result !== "" ? "Close" : "Cancel"; onClicked: root.close() },
        Btn {
            kind: "primary"
            text: "Apply to " + root.changing.length + (root.changing.length === 1 ? " board" : " boards")
            enabled: root.changing.length > 0
            onClicked: root.go()
        }
    ]
}
