import QtQuick
import QtQuick.Dialogs
import QtQuick.Layouts
import "../ui"

// Board: the settings as a whole, what the board is, the network
ColumnLayout {
    id: root
    property var b
    spacing: 18

    readonly property var caps: b ? b.caps : ({})
    property string msg: ""
    property bool msgBad: false

    ColumnLayout {
        spacing: 4
        Label2 { text: "Board"; kind: "title" }
        Label2 { kind: "sub"; Layout.fillWidth: true; text: "The settings of the board as a whole, what it has and the network." }
    }

    GridLayout {
        Layout.fillWidth: true
        columns: width > 700 ? 2 : 1
        columnSpacing: 16
        rowSpacing: 16

        Card {
            title: "Settings"
            subtitle: "Changes take effect at once and are saved on the board as they change."
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: 10
            Flow {
                Layout.fillWidth: true
                spacing: 10
                Btn {
                    text: "Defaults"
                    enabled: b && b.isConfigConnected
                    onClicked: { b.saveDefaults(); root.msg = "The default settings are sent: the streams stop."; root.msgBad = false }
                }
                Btn { kind: "outline"; text: "Export JSON…"; enabled: b && b.settingsLoaded; onClicked: saveDlg.open() }
                Btn { kind: "outline"; text: "Import JSON…"; enabled: b && b.isConfigConnected; onClicked: openDlg.open() }
            }
            Label2 { visible: root.msg !== ""; kind: root.msgBad ? "bad" : "hint"; text: root.msg; Layout.fillWidth: true }
        }

        Card {
            title: "This board"
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: 10
            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 16
                rowSpacing: 10
                Kv { label: "Model"; value: b ? b.model : ""; Layout.fillWidth: true; Layout.preferredWidth: 10 }
                Kv { label: "MAC"; value: root.caps.mac || "-"; mono: true; Layout.fillWidth: true; Layout.preferredWidth: 10 }
                Kv { label: "IP"; value: b ? b.ip : ""; mono: true; Layout.fillWidth: true }
                Kv { label: "Clock"; value: b && b.isMaster ? "Master" : "Slave (daisy chain)"; Layout.fillWidth: true }
                Kv { label: "ADC"; value: (root.caps.adcChannels || 0) + " ch · " + Theme.rate(root.caps.adcRate || 0).replace("S/s", "Hz") + " · " + (root.caps.adcBits || 0) + " bit"; Layout.fillWidth: true }
                Kv { label: "DAC"; value: root.caps.dac ? root.caps.dacChannels + " ch · " + Theme.rate(root.caps.dacRate || 0).replace("S/s", "Hz") : "none"; Layout.fillWidth: true }
                Kv { label: "GPIO streaming"; value: root.caps.gpio ? "yes" : "no"; Layout.fillWidth: true }
                Kv { label: "DMA memory"; value: root.caps.memorySize ? Theme.bytes(root.caps.memorySize) : "-"; Layout.fillWidth: true }
            }
            Label2 {
                kind: "hint"
                Layout.fillWidth: true
                text: root.caps.fromBoard ? "As the board tells it." : "From the model: the streaming application of the board is older and does not tell what it has."
            }
        }

        Card {
            title: "Network"
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: 10
            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 16
                rowSpacing: 10
                Kv { label: "Settings"; value: "18901"; Layout.fillWidth: true; Layout.preferredWidth: 10 }
                Kv { label: "Board discovery (UDP)"; value: "18902"; Layout.fillWidth: true; Layout.preferredWidth: 10 }
                Kv { label: "ADC data"; value: "18900"; Layout.fillWidth: true }
                Kv { label: "DAC data"; value: "18903"; Layout.fillWidth: true }
                Kv { label: "GPIO input data"; value: "18904"; Layout.fillWidth: true }
                Kv { label: "GPIO output data"; value: "18905"; Layout.fillWidth: true }
            }
            Label2 { kind: "hint"; Layout.fillWidth: true; text: "One client at a time: a new client (this one, another computer or the web page) takes the board over and the streams of the old one stop." }
        }
    }

    FileDialog {
        id: saveDlg
        fileMode: FileDialog.SaveFile
        nameFilters: ["JSON (*.json)"]
        defaultSuffix: "json"
        currentFile: app.pathToUrl(b ? b.recordDir() + "/" + b.name + "_settings.json" : "")
        onAccepted: {
            const ok = ui_controller.writeTextFile(selectedFile, b.settingsJson())
            root.msgBad = !ok
            root.msg = ok ? "Saved to " + app.urlToPath(selectedFile) : "Not saved: " + ui_controller.lastError()
        }
    }
    FileDialog {
        id: openDlg
        fileMode: FileDialog.OpenFile
        nameFilters: ["JSON (*.json)", "All files (*)"]
        onAccepted: {
            const text = ui_controller.readTextFile(selectedFile)
            const ok = text !== "" && b.loadSettingsJson(text)
            root.msgBad = !ok
            root.msg = ok ? "Sent to the board: the streams stop and take the new settings." : text === "" ? "Not read: " + ui_controller.lastError() : "Refused: not a settings file of the streaming application."
        }
    }
}
