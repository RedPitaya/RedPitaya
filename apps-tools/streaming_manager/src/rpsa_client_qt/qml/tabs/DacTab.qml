import QtQuick
import QtQuick.Dialogs
import QtQuick.Layouts
import "../ui"

// DAC (streaming_improvements 4.17): OUT1, OUT2 from a file of this computer (sent to the board
// while it plays) or from a file of the SD card of the board. As the DAC section of the web page
// of the board; the file of this computer and its repeat are kept on this computer (pcValue).
ColumnLayout {
    id: root
    property var b
    spacing: 18

    readonly property int rev: b ? b.settingsRev : 0
    readonly property int pcRev: b ? b.pcRev : 0
    function v(key) { return rev >= 0 && b ? b.value(key) : "" }
    function pv(key) { return pcRev >= 0 && b ? b.pcValue(key) : "" }
    function set(key, value) { say(key, ""); b.setValue(key, String(value)) }
    property var msgs: ({})
    onBChanged: msgs = ({})
    function say(key, text) {
        const m = Object.assign({}, msgs)
        if (text) m[key] = text
        else delete m[key]
        msgs = m
    }
    Connections {
        target: root.b
        function onSettingRefused(key, value) { root.say(key, "The board did not take " + value + ": it keeps " + root.v(key)) }
    }

    readonly property var caps: b ? b.caps : ({})
    readonly property int n: Math.max(1, Math.min(2, caps.dacChannels || 2))
    readonly property bool sd: v("dac_pass_mode") === "DAC_FILE"
    readonly property var file: b ? b.dacFile : ({})
    readonly property double rate: Number(v("dac_rate")) || 0
    // WAV, TDMS or BIN by the extension, as the web page takes it
    function typeOf(name) {
        const e = String(name).split(".").pop().toLowerCase()
        return e === "tdms" ? "TDMS" : e === "bin" ? "BIN" : e === "wav" ? "WAV" : ""
    }
    function dur(sec) {
        if (!(sec >= 0) || !isFinite(sec)) return "-"
        return sec < 1 ? (sec * 1000).toFixed(1) + " ms" : sec < 90 ? sec.toFixed(1) + " s" : sec < 5400 ? Math.round(sec / 60) + " min" : (sec / 3600).toFixed(1) + " h"
    }
    readonly property string fileText: !file.path ? "WAV (PCM 8 or 16 bit), TDMS or a BIN recording of the ADC"
        : file.reading ? "Reading the file..."
        : !file.ok ? (file.error || "")
        : [file.format,
           (file.channels === 3 ? "2 channels" : "1 channel (" + (file.channels === 2 ? "OUT2" : "OUT1") + ")"),
           file.bits + " bit",
           Theme.count(file.samples) + " samples",
           Theme.bytes(file.size || 0),
           root.rate > 0 ? dur(file.samples / root.rate) + " at " + Theme.rate(root.rate) : ""]
          .filter(x => x !== "").join(" · ")

    ColumnLayout {
        spacing: 4
        Label2 { text: "DAC"; kind: "title" }
        Label2 { kind: "sub"; Layout.fillWidth: true; text: (root.n === 1 ? "OUT1" : "OUT1, OUT2") + " from a file on this computer, or from a file on the SD card of the board." }
    }

    Card {
        Layout.fillWidth: true
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            Label2 { text: "Source"; kind: "cardTitle"; Layout.fillWidth: true }
            Seg {
                options: ["File on this PC", "File on SD card"]
                currentIndex: root.sd ? 1 : 0
                onPicked: (i) => root.set("dac_pass_mode", i === 1 ? "DAC_FILE" : "DAC_NET")
            }
        }

        // a file of this computer: kept here, not on the board
        GridLayout {
            visible: !root.sd
            Layout.fillWidth: true
            columns: width > 760 ? 4 : 2
            columnSpacing: 16
            rowSpacing: 12
            Field {
                label: "File"
                hint: root.fileText
                hintBad: !!root.file.path && !root.file.reading && !root.file.ok
                Layout.fillWidth: true
                Layout.columnSpan: 2
                Layout.preferredWidth: 20
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    TextIn {
                        objectName: "field:dacFile"
                        Layout.fillWidth: true
                        mono: true
                        placeholderText: "a WAV, TDMS or BIN file of this computer"
                        value: String(root.pv("dacFile"))
                        onCommitted: (t) => b.setPcValue("dacFile", t.trim())
                    }
                    Btn { kind: "outline"; text: "Browse…"; onClicked: fileDlg.open() }
                }
            }
            Field {
                label: "Repeat"
                Layout.fillWidth: true
                Layout.preferredWidth: 10
                Select {
                    Layout.fillWidth: true
                    options: [["once", "once"], ["times", "N times"], ["inf", "until the stop"]]
                    value: String(root.pv("dacRepeat"))
                    onPicked: (x) => b.setPcValue("dacRepeat", x)
                }
            }
            Field {
                label: "Times"
                Layout.fillWidth: true
                Layout.preferredWidth: 10
                opacity: root.pv("dacRepeat") === "times" ? 1 : 0.45
                TextIn {
                    objectName: "field:dacTimes"
                    Layout.fillWidth: true
                    mono: true
                    enabled: root.pv("dacRepeat") === "times"
                    value: String(root.pv("dacTimes"))
                    validator: IntValidator { bottom: 1; top: 1000000000 }
                    onCommitted: (t) => b.setPcValue("dacTimes", Math.max(1, Number(t) || 1))
                }
            }
        }
        Label2 {
            visible: !root.sd
            Layout.fillWidth: true
            kind: "hint"
            text: "The file is read on this computer and sent to the board (port 18903) while it plays; the file and the repeat are kept on this computer. A file that fits one block (block_size, Memory tab) goes once: the board repeats it."
        }

        // a file of the SD card of the board
        GridLayout {
            visible: root.sd
            Layout.fillWidth: true
            columns: width > 760 ? 4 : 2
            columnSpacing: 16
            rowSpacing: 12
            Field {
                label: "File on the SD card"
                hint: root.msgs["file_sd"] || "A name in /home/redpitaya/streaming_files/dac (upload files on the web page of the board)"
                hintBad: !!root.msgs["file_sd"]
                Layout.fillWidth: true
                Layout.columnSpan: 2
                Layout.preferredWidth: 20
                TextIn {
                    objectName: "field:file_sd"
                    Layout.fillWidth: true
                    mono: true
                    placeholderText: "signal.wav"
                    value: root.v("file_sd")
                    onCommitted: (t) => {
                        const name = t.trim()
                        root.set("file_sd", name)
                        const type = root.typeOf(name)
                        if (type !== "" && type !== root.v("file_type_sd")) root.set("file_type_sd", type)
                    }
                }
            }
            Field {
                label: "Type"
                Layout.fillWidth: true
                Layout.preferredWidth: 10
                Select {
                    Layout.fillWidth: true
                    options: [["WAV", "WAV"], ["TDMS", "TDMS"], ["BIN", "BIN"]]
                    value: root.v("file_type_sd") || "WAV"
                    onPicked: (x) => root.set("file_type_sd", x)
                }
            }
            Item { Layout.fillWidth: true; Layout.preferredWidth: 10; visible: parent.columns === 4 }
            Field {
                label: "Repeat"
                Layout.fillWidth: true
                Layout.preferredWidth: 10
                Select {
                    Layout.fillWidth: true
                    options: [["DAC_REP_OFF", "once"], ["DAC_REP_ON", "N times"], ["DAC_REP_INF", "until the stop"]]
                    value: root.v("repeat") || "DAC_REP_OFF"
                    onPicked: (x) => root.set("repeat", x)
                }
            }
            Field {
                label: "Times"
                hint: root.msgs["repeatCount"] || ""
                hintBad: !!root.msgs["repeatCount"]
                Layout.fillWidth: true
                Layout.preferredWidth: 10
                opacity: root.v("repeat") === "DAC_REP_ON" ? 1 : 0.45
                TextIn {
                    objectName: "field:repeatCount"
                    Layout.fillWidth: true
                    mono: true
                    enabled: root.v("repeat") === "DAC_REP_ON"
                    value: root.v("repeatCount")
                    validator: IntValidator { bottom: 1; top: 2000000000 }
                    onCommitted: (t) => root.set("repeatCount", Math.max(1, Number(t) || 1))
                }
            }
        }
    }

    Card {
        title: "Output"
        Layout.fillWidth: true
        GridLayout {
            Layout.fillWidth: true
            columns: width > 760 ? 4 : 2
            columnSpacing: 16
            rowSpacing: 12
            Field {
                label: "Sample rate (S/s)"
                hint: root.msgs["dac_rate"] || ((!root.sd && root.file.fileRate > 0 ? "the file: " + Theme.rate(root.file.fileRate) + " · " : "")
                      + Theme.rate(root.caps.dacRate || 125000000) + " at most")
                hintBad: !!root.msgs["dac_rate"]
                Layout.fillWidth: true
                Layout.preferredWidth: 10
                TextIn {
                    objectName: "field:dac_rate"
                    Layout.fillWidth: true
                    mono: true
                    value: root.v("dac_rate")
                    validator: IntValidator { bottom: 1; top: 2000000000 }
                    onCommitted: (t) => {
                        const r = Number(t) || 0
                        const max = root.caps.dacRate || 125000000
                        if (r < 1 || r > max) { root.say("dac_rate", "From 1 to " + max + " S/s"); return }
                        root.set("dac_rate", Math.round(r))
                    }
                }
                Btn {
                    visible: !root.sd && root.file.fileRate > 0 && root.file.fileRate !== root.rate
                    kind: "outline"; compact: true
                    text: "Use the rate of the file"
                    onClicked: root.set("dac_rate", Math.round(root.file.fileRate))
                }
            }
            Repeater {
                model: root.caps.dacGain ? root.n : 0
                delegate: Field {
                    required property int index
                    label: "OUT" + (index + 1) + " gain"
                    Layout.fillWidth: true
                    Layout.preferredWidth: 10
                    Select {
                        Layout.fillWidth: true
                        options: [["X1", "x1"], ["X5", "x5"]]
                        value: root.v("channel_gain_" + (index + 1)) || "X1"
                        onPicked: (x) => root.set("channel_gain_" + (index + 1), x)
                    }
                }
            }
            // the columns stay those of the other cards on a board of one gain
            Repeater {
                model: root.caps.dacGain ? 0 : root.n
                delegate: Item { Layout.fillWidth: true; Layout.preferredWidth: 10 }
            }
            Field {
                label: " "
                Layout.fillWidth: true
                Layout.preferredWidth: 10
                Flip {
                    label: "Play the gaps of a recording as 0"
                    Layout.fillWidth: true
                    checked: root.v("dac_play_gaps") === "ON"
                    onClicked: root.set("dac_play_gaps", checked ? "ON" : "OFF")
                }
            }
        }
        Label2 {
            Layout.fillWidth: true
            kind: "hint"
            text: "The gaps: a BIN recording of the ADC plays the samples it lost or waited for a trigger as 0, in its own time; without them the samples follow each other."
                  + (root.caps.dacGain ? "" : " The outputs of this board have one gain (x1).")
        }
    }

    Card {
        Layout.fillWidth: true
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            Label2 { text: "Trigger"; kind: "cardTitle"; Layout.fillWidth: true }
            Label2 { text: "Sources: Triggers tab"; kind: "hint" }
        }
        GridLayout {
            Layout.fillWidth: true
            columns: width > 760 ? 1 + root.n : 1
            columnSpacing: 16
            rowSpacing: 12
            Field {
                label: "Start and stop together with"
                Layout.fillWidth: true
                Layout.preferredWidth: 10
                Select {
                    Layout.fillWidth: true
                    options: [["OWN", "itself"], ["OSC1", "ADC"]].concat(root.caps.gpio ? [["LA", "GPIO input"]] : [])
                    value: root.v("dac_event_source") || "OWN"
                    onPicked: (x) => root.set("dac_event_source", x)
                }
            }
            Repeater {
                model: root.n
                delegate: Field {
                    required property int index
                    label: "OUT" + (index + 1) + " waits for"
                    Layout.fillWidth: true
                    Layout.preferredWidth: 10
                    Select {
                        Layout.fillWidth: true
                        options: [["OFF", "nothing (at the start)"], ["ON", "a trigger"]]
                        value: root.v("dac_trigger_mode_" + (index + 1)) || "OFF"
                        onPicked: (x) => root.set("dac_trigger_mode_" + (index + 1), x)
                    }
                }
            }
        }
        Label2 {
            readonly property bool waits: root.v("dac_trigger_mode_1") === "ON" || (root.n > 1 && root.v("dac_trigger_mode_2") === "ON")
            Layout.fillWidth: true
            kind: waits && !(Number(root.v("dac_trigger_sources")) > 0) ? "warn" : "hint"
            text: waits && !(Number(root.v("dac_trigger_sources")) > 0)
                  ? "No source: only the software trigger starts the waiting outputs. The sources are set in the Triggers tab."
                  : "What starts a waiting output (the levels of the inputs, GPIO, the external input) is set in the Triggers tab."
        }
        Label2 {
            visible: Object.keys(root.msgs).some(k => k !== "file_sd" && k !== "repeatCount" && k !== "dac_rate")
            Layout.fillWidth: true
            kind: "bad"
            text: Object.keys(root.msgs).filter(k => k !== "file_sd" && k !== "repeatCount" && k !== "dac_rate").map(k => root.msgs[k]).join(" · ")
        }
    }

    FileDialog {
        id: fileDlg
        fileMode: FileDialog.OpenFile
        nameFilters: ["DAC files (*.wav *.tdms *.bin)", "WAV (*.wav)", "TDMS (*.tdms)", "BIN (*.bin)", "All files (*)"]
        currentFolder: String(root.pv("dacFile")) !== "" ? app.pathToUrl(String(root.pv("dacFile")).replace(/[\\/][^\\/]*$/, "")) : ""
        onAccepted: b.setPcValue("dacFile", app.urlToPath(selectedFile))
    }
}
