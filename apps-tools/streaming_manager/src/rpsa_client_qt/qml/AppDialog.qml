import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "ui"

// The settings of the application: how boards are found, Start all, recordings, preview, look
Dialog2 {
    id: root
    signal addBoard()

    width: Math.min(900, Overlay.overlay ? Overlay.overlay.width - 48 : 900)
    height: Math.min(sv.contentHeight + chromeHeight, maxHeight)
    title: "Application settings"

    ScrollView {
        id: sv
        anchors.fill: parent
        contentWidth: availableWidth
        contentHeight: col.implicitHeight + 36
        clip: true

        ColumnLayout {
            id: col
            x: 20; y: 18
            width: sv.availableWidth - 40
            spacing: 16

            Card {
                title: "Boards"
                Layout.fillWidth: true
                Flip {
                    label: "Find boards on the network by themselves (broadcast, UDP 18902)"
                    Layout.fillWidth: true
                    checked: app.discovery
                    onClicked: app.discovery = checked
                }
                Label2 { kind: "sub"; Layout.fillWidth: true; text: "Boards in another subnet do not answer the search: add them by IP." }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0
                    visible: app.manualHosts.length > 0
                    RowLayout {
                        Layout.fillWidth: true
                        Label2 { text: "Added by IP"; kind: "hint"; font.bold: true; Layout.preferredWidth: 200 }
                        Label2 { text: "State"; kind: "hint"; font.bold: true; Layout.fillWidth: true }
                    }
                    Rectangle { Layout.fillWidth: true; Layout.topMargin: 6; height: 1; color: "#777777" }
                    Repeater {
                        model: app.manualHosts
                        delegate: ColumnLayout {
                            required property string modelData
                            readonly property var brd: (boards.count, boards.boardByIp(modelData))
                            Layout.fillWidth: true
                            spacing: 0
                            RowLayout {
                                Layout.fillWidth: true
                                Layout.topMargin: 6
                                Layout.bottomMargin: 6
                                Text { text: modelData; color: Theme.value; font.family: Theme.mono; font.pixelSize: Theme.fsMd; Layout.preferredWidth: 200; elide: Text.ElideRight }
                                Text {
                                    Layout.fillWidth: true
                                    text: !brd ? "Looking up the name" : brd.isOnline ? "Online · " + brd.name + " · " + brd.model : "No answer"
                                    color: brd && !brd.isOnline ? Theme.dangerText : Theme.text
                                    font.pixelSize: Theme.fsMd
                                    elide: Text.ElideRight
                                }
                                Btn { kind: "outline"; text: "Remove"; compact: true; onClicked: app.removeManualHost(modelData) }
                            }
                            Rectangle { Layout.fillWidth: true; height: 1; color: Theme.borderSoft }
                        }
                    }
                }
                Btn { kind: "primary"; text: "Add board by IP"; iconSource: "qrc:/img/icons/plus.svg"; onClicked: root.addBoard() }
            }

            Card {
                title: "Start all and Stop all"
                subtitle: "The slaves start first and the master last: the clock and the trigger of the master drive the slaves. Stop all stops the master first. A recording of Start all has one time in the names of all its files."
                Layout.fillWidth: true
                Flip {
                    label: "Stop all when a board loses data"
                    Layout.fillWidth: true
                    checked: app.stopAllOnLoss
                    onClicked: app.stopAllOnLoss = checked
                }
            }

            Card {
                title: "Recordings"
                Layout.fillWidth: true
                Field {
                    label: "Folder of the boards"
                    hint: "{board} is the name of the board. A board can have a folder of its own: its settings, This PC."
                    Layout.fillWidth: true
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        TextIn {
                            Layout.fillWidth: true
                            mono: true
                            value: app.recordDir
                            onCommitted: (t) => app.recordDir = t
                        }
                        Btn { kind: "outline"; text: "Browse…"; onClicked: folderDlg.open() }
                    }
                }
            }

            Card {
                title: "Signal preview"
                subtitle: "Every running board gives a block at the same time; the preview never holds the recording back."
                Layout.fillWidth: true
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16
                    Field {
                        label: "Refresh"
                        Layout.preferredWidth: 200
                        Select {
                            Layout.fillWidth: true
                            options: [[1, "1 per second"], [2, "2 per second"], [5, "5 per second"], [10, "10 per second"]]
                            value: app.previewRate
                            onPicked: (x) => app.previewRate = x
                        }
                    }
                    Field {
                        label: "Points"
                        Layout.preferredWidth: 200
                        Select {
                            Layout.fillWidth: true
                            options: [[500, "500"], [1000, "1000"], [2000, "2000"]]
                            value: app.previewPoints
                            onPicked: (x) => app.previewPoints = x
                        }
                    }
                    Item { Layout.fillWidth: true }
                }
            }

            Card {
                title: "Appearance"
                Layout.fillWidth: true
                Field {
                    label: "Text size"
                    Seg {
                        options: ["Normal", "Large", "Larger"]
                        currentIndex: app.textSize / 2
                        onPicked: (i) => app.textSize = i * 2
                    }
                }
            }

            Label2 { kind: "hint"; text: "Red Pitaya streaming client " + ui_controller.version; Layout.fillWidth: true }
        }
    }

    footer: [
        Label2 { kind: "hint"; text: "Kept on this computer."; Layout.fillWidth: true },
        Btn { kind: "primary"; text: "Done"; onClicked: root.close() }
    ]

    FolderDialog {
        id: folderDlg
        currentFolder: app.pathToUrl(app.recordDirFor(""))
        onAccepted: app.recordDir = app.urlToPath(selectedFolder) + "/{board}"
    }
}
