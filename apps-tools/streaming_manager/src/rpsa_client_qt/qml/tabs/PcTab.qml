import QtQuick
import QtQuick.Dialogs
import QtQuick.Layouts
import "../ui"

// This PC: where the data of the board goes on this computer (kept here, not on the board)
ColumnLayout {
    id: root
    property var b
    spacing: 18

    readonly property int rev: b ? b.pcRev + b.settingsRev : 0
    function pv(key) { return rev >= 0 && b ? b.pcValue(key) : "" }

    ColumnLayout {
        spacing: 4
        Label2 { text: "This PC"; kind: "title" }
        Label2 { kind: "sub"; Layout.fillWidth: true; text: "Where the data of this board goes on this computer. Kept on this computer, not on the board." }
    }

    GridLayout {
        Layout.fillWidth: true
        columns: width > 760 ? 2 : 1
        columnSpacing: 16
        rowSpacing: 16

        Card {
            title: "Recording on this PC"
            subtitle: "When the ADC destination is This PC. The format, the values and the sample limit are on the ADC tab."
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: 10
            Field {
                label: "Folder"
                hint: root.pv("recordDir") === "" ? "The folder of the application settings: " + (root.rev >= 0 && b ? b.recordDir() : "") : ""
                Layout.fillWidth: true
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    TextIn {
                        Layout.fillWidth: true
                        mono: true
                        placeholderText: "the folder of the application settings"
                        value: root.pv("recordDir")
                        onCommitted: (t) => b.setPcValue("recordDir", t.trim())
                    }
                    Btn { kind: "outline"; text: "Browse…"; onClicked: folderDlg.open() }
                    Btn { kind: "outline"; text: "Open"; onClicked: b.openRecordFolder() }
                }
            }
            Field {
                label: "File name"
                hint: (root.rev >= 0 && b ? b.exampleFileName() : "") + "   ·   {board} {ip} {date} {time}"
                Layout.fillWidth: true
                TextIn {
                    Layout.fillWidth: true
                    mono: true
                    value: root.pv("nameTemplate")
                    onCommitted: (t) => b.setPcValue("nameTemplate", t.trim())
                }
            }
            Flip {
                label: "Open the folder when a recording stops"
                Layout.fillWidth: true
                checked: !!root.pv("openFolderOnStop")
                onClicked: b.setPcValue("openFolderOnStop", checked)
            }
        }

        Card {
            title: "Signal preview and console"
            subtitle: "The preview takes a block of the stream now and then; it never holds the recording back. How often and how many points: Application settings."
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: 10
            Flip {
                label: "Show the signal (else the console)"
                Layout.fillWidth: true
                checked: b ? b.chartEnable : false
                onClicked: b.chartEnable = checked
            }
            Field {
                label: "Console messages"
                Layout.fillWidth: true
                Select {
                    Layout.preferredWidth: 220
                    options: [[0, "Errors and warnings"], [1, "Normal"], [2, "Detailed"]]
                    value: Number(root.pv("consoleLevel"))
                    onPicked: (x) => b.setPcValue("consoleLevel", x)
                }
            }
            Flip {
                label: "Save the console next to the recording"
                Layout.fillWidth: true
                checked: !!root.pv("saveConsole")
                onClicked: b.setPcValue("saveConsole", checked)
            }
        }
    }

    FolderDialog {
        id: folderDlg
        currentFolder: app.pathToUrl(b ? b.recordDir() : "")
        onAccepted: b.setPcValue("recordDir", app.urlToPath(selectedFolder))
    }
}
