import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "ui"

// The boards: a card or a row each, what they stream and how much.
Item {
    id: root
    signal openBoard(var board, string tab)
    signal openApp()
    signal addBoard()
    // the profiles of the settings (streaming_improvements 4.9)
    signal openProfiles()
    // Start of a board (null: Start all): main checks the space on the disk first; all: the
    // other streams of the board too (Start all of this board)
    signal askStart(var board, bool all)

    readonly property var sum: boards.summary
    readonly property bool single: boards.count === 1
    readonly property bool isList: app.view === "list"
    // the boards or the recordings on this computer (streaming_improvements 4.7)
    property string page: "boards"
    function recordingsScript(what) { recPage.scriptOpen(what) }
    function viewerScript(p) { if (recPage.lastViewer) recPage.lastViewer.script(p) }
    function closeViewers() { recPage.closeViewers() }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // application bar
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: bar.implicitHeight + 16
            color: Theme.card
            Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.borderStrong }
            RowLayout {
                id: bar
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 22
                Image { source: "qrc:/img/logo.png"; sourceSize.width: 220; Layout.preferredWidth: 110; Layout.preferredHeight: 34; fillMode: Image.PreserveAspectFit }
                Text { text: "Streaming client"; color: Theme.title; font.pixelSize: Theme.fsXl; visible: root.width > 900 }
                Seg {
                    options: ["Boards", "Recordings"]
                    currentIndex: root.page === "recordings" ? 1 : 0
                    onPicked: (i) => root.page = i === 1 ? "recordings" : "boards"
                }
                RowLayout {
                    spacing: 8
                    Layout.fillWidth: true
                    Dot { color: app.discovery ? Theme.ok : Theme.dim }
                    Label2 {
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        kind: "hint"
                        text: (app.discovery ? "Broadcast search on (UDP 18902) · " + root.sum.broadcast + " found"
                                             : "Broadcast search off")
                              + (root.sum.manual ? ", " + root.sum.manual + " added by IP" : "")
                    }
                }
                Btn { kind: "outline"; text: "Profiles"; onClicked: root.openProfiles() }
                Btn { kind: "outline"; text: "Add board by IP"; iconSource: "qrc:/img/icons/plus.svg"; onClicked: root.addBoard() }
                Btn { kind: "outline"; text: "Application settings"; iconSource: "qrc:/img/icons/gear.svg"; onClicked: root.openApp() }
            }
        }

        Recordings {
            id: recPage
            visible: root.page === "recordings"
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        ScrollView {
            id: scroll
            visible: root.page === "boards"
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: availableWidth
            clip: true

            ColumnLayout {
                width: scroll.availableWidth
                spacing: 16

                Item { implicitHeight: 2 }

                // toolbar: the buttons go under the title when the window is narrow
                GridLayout {
                    Layout.leftMargin: 20
                    Layout.rightMargin: 20
                    Layout.fillWidth: true
                    columns: scroll.availableWidth < 980 ? 1 : 2
                    columnSpacing: 12
                    rowSpacing: 12
                    ColumnLayout {
                        spacing: 4
                        Layout.fillWidth: true
                        Label2 { text: "Boards"; kind: "title" }
                        Label2 {
                            kind: "sub"
                            Layout.fillWidth: true
                            text: boards.count === 0 ? "No board yet"
                                : root.single ? "1 board · " + (root.sum.running ? "running" : "stopped")
                                : boards.count + " boards · " + root.sum.online + " online · " + root.sum.running + " running"
                                  + (root.sum.master ? " · " + root.sum.master + " is the clock master" : "")
                                  + (root.sum.errors ? " · <font color=\"" + Theme.dangerText + "\">" + root.sum.errors + (root.sum.errors === 1 ? " error" : " errors") + "</font>" : "")
                            textFormat: Text.StyledText
                        }
                    }
                    RowLayout {
                    spacing: 12
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    Seg {
                        options: ["Cards", "List"]
                        currentIndex: root.isList ? 1 : 0
                        onPicked: (i) => app.view = i === 1 ? "list" : "cards"
                        visible: boards.count > 0
                    }
                    Btn {
                        kind: "primary"; text: boards.starting ? "Starting…" : "Start all"; iconSource: "qrc:/img/icons/play.svg"
                        enabled: !boards.starting && root.sum.online > root.sum.running
                        onClicked: root.askStart(null, false)
                    }
                    Btn { kind: "danger"; text: "Stop all"; iconSource: "qrc:/img/icons/stop.svg"; enabled: root.sum.running > 0 || root.sum.others > 0 || boards.starting; onClicked: boards.stopAll() }
                    Btn { kind: "outline"; text: "Trigger all"; iconSource: "qrc:/img/icons/bolt.svg"; enabled: root.sum.running > 0; onClicked: boards.triggerAll() }
                    }
                }

                // totals
                GridLayout {
                    Layout.leftMargin: 20
                    Layout.rightMargin: 20
                    columns: Math.max(1, Math.min(4, Math.floor((scroll.availableWidth - 40) / 220)))
                    columnSpacing: 12
                    rowSpacing: 12
                    SumTile { label: "Streaming now"; value: Theme.bytes(root.sum.speed) + "/s" }
                    SumTile { label: "Received (this run)"; value: Theme.bytes(root.sum.received) }
                    SumTile { label: "Lost samples"; value: Theme.count(root.sum.lost); bad: root.sum.lost > 0 }
                    SumTile {
                        label: "Recordings folder"
                        value: app.recordDir
                        mono: true
                        Btn { kind: "outline"; text: "Open"; compact: true; onClicked: ui_controller.openFolder(app.recordDirFor("")) }
                    }
                }

                // no board
                Card {
                    visible: boards.count === 0
                    Layout.leftMargin: 20
                    Layout.rightMargin: 20
                    Layout.fillWidth: true
                    title: "Looking for boards"
                    subtitle: "Boards with the streaming application running answer the broadcast search (UDP 18902) and show up here by themselves. A board in another subnet does not answer: add it by IP."
                    Btn { kind: "primary"; text: "Add board by IP"; iconSource: "qrc:/img/icons/plus.svg"; onClicked: root.addBoard() }
                }

                // cards
                GridLayout {
                    id: cardsGrid
                    visible: !root.isList && boards.count > 0
                    Layout.leftMargin: 20
                    Layout.rightMargin: 20
                    Layout.fillWidth: true
                    columns: root.single ? 1 : Math.max(1, Math.floor((scroll.availableWidth - 40 + 16) / (420 + 16)))
                    columnSpacing: 16
                    rowSpacing: 16
                    Repeater {
                        model: boards
                        delegate: BoardCard {
                            required property var board
                            b: board
                            big: root.single
                            // the whole card in the window, whatever the text size
                            bigChartHeight: Math.max(160, Math.min(700, scroll.height - cardsGrid.y - restHeight - 12))
                            Layout.fillWidth: true
                            Layout.preferredWidth: 420
                            Layout.alignment: Qt.AlignTop
                            onOpenSettings: (tab) => root.openBoard(board, tab)
                            onStartAsked: (all) => root.askStart(board, all)
                        }
                    }
                }

                // list
                Card {
                    visible: root.isList && boards.count > 0
                    Layout.leftMargin: 20
                    Layout.rightMargin: 20
                    Layout.fillWidth: true
                    padding: 6
                    spacing: 0
                    BoardRow { header: true; Layout.fillWidth: true }
                    Repeater {
                        model: boards
                        delegate: BoardRow {
                            required property var board
                            required property int index
                            b: board
                            last: index === boards.count - 1
                            Layout.fillWidth: true
                            onOpenSettings: (tab) => root.openBoard(board, tab)
                            onStartAsked: root.askStart(board, false)
                        }
                    }
                }

                // one board: more may come
                Rectangle {
                    visible: root.single
                    Layout.leftMargin: 20
                    Layout.rightMargin: 20
                    Layout.fillWidth: true
                    implicitHeight: more.implicitHeight + 36
                    color: "transparent"
                    radius: Theme.radius
                    border.color: Theme.border
                    // a dashed frame
                    Canvas {
                        anchors.fill: parent
                        onPaint: {
                            const ctx = getContext("2d"); ctx.reset()
                            ctx.setLineDash([6, 4]); ctx.strokeStyle = "#999999"; ctx.lineWidth = 1
                            ctx.strokeRect(0.5, 0.5, width - 1, height - 1)
                        }
                    }
                    RowLayout {
                        id: more
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 16
                        Dot { width: 10; height: 10; radius: 5; color: app.discovery ? Theme.ok : Theme.dim }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text { text: "Looking for more boards"; color: Theme.title; font.bold: true; font.pixelSize: Theme.fsBase }
                            Label2 {
                                kind: "hint"
                                Layout.fillWidth: true
                                text: "Boards with the streaming application running answer the broadcast search (UDP 18902) and show up here by themselves. A board in another subnet does not answer: add it by IP."
                            }
                        }
                        Btn { kind: "outline"; text: "Add board by IP"; iconSource: "qrc:/img/icons/plus.svg"; onClicked: root.addBoard() }
                    }
                }

                Item { implicitHeight: 8 }
            }
        }
    }

    component SumTile: Rectangle {
        id: tile
        property string label: ""
        property string value: ""
        property bool mono: false
        property bool bad: false
        default property alias extra: extraRow.data
        Layout.fillWidth: true
        implicitHeight: 58 + Theme.fsAdd
        color: Theme.card
        border.color: Theme.border
        radius: Theme.radius
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 14
            anchors.rightMargin: 14
            spacing: 10
            ColumnLayout {
                spacing: 2
                Layout.fillWidth: true
                Label2 { text: tile.label; kind: "hint" }
                Text {
                    text: tile.value
                    color: tile.bad ? Theme.dangerText : Theme.value
                    font.pixelSize: tile.mono ? Theme.fsMd : Theme.fsLg
                    font.bold: !tile.mono
                    font.family: tile.mono ? Theme.mono : Qt.application.font.family
                    elide: Text.ElideMiddle
                    Layout.fillWidth: true
                }
            }
            Row { id: extraRow; spacing: 8 }
        }
    }
}
