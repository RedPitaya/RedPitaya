import QtQuick
import QtQuick.Layouts
import "../ui"

// ADC: where the data goes, the inputs and the sampling (as the ADC page of the board)
ColumnLayout {
    id: root
    property var b
    spacing: 18

    readonly property int rev: b ? b.settingsRev : 0
    function v(key) { return rev >= 0 && b ? b.value(key) : "" }
    // a value sent clears what its field said
    function set(key, value) { say(key, ""); b.setValue(key, String(value)) }

    // what a field says under it (streaming_improvements 2.5): {key: {text, kind: bad|warn}},
    // until the next edit of the field
    property var msgs: ({})
    onBChanged: msgs = ({})  // the dialog opened for another board
    function say(key, text, kind) {
        const m = Object.assign({}, msgs)
        if (text) m[key] = { text: text, kind: kind || "bad" }
        else delete m[key]
        msgs = m
    }
    function msg(key) { return msgs[key] ? msgs[key].text : "" }
    function msgBad(key) { return !!msgs[key] && msgs[key].kind === "bad" }
    function msgWarn(key) { return !!msgs[key] && msgs[key].kind === "warn" }
    Connections {
        target: root.b
        function onSettingRefused(key, value) { root.say(key, "The board did not take " + value + ": it keeps " + root.v(key)) }
    }

    readonly property var caps: b ? b.caps : ({})
    readonly property int n: caps.adcChannels || 2
    readonly property bool toSd: v("adc_pass_mode") === "FILE"
    readonly property int dec: Math.max(1, Number(v("adc_decimation")) || 1)
    readonly property double per: (caps.adcRate || 125000000) / dec
    readonly property int bytes: v("resolution") === "BIT_8" ? 1 : 2
    readonly property int on: {
        let c = 0
        for (let i = 1; i <= n; i++) if (v("channel_state_" + i) === "ON") c++
        return c
    }
    readonly property string format: v("format_sd")

    // the decimations the FPGA takes: 1, 2, 4, 8, 16, then every value up to 65536 (as the page)
    function setDecimation(t) {
        const d = Number(t)
        if (t === "" || !Number.isInteger(d) || d < 1 || d > 65536) {
            say("adc_decimation", "From 1 to 65536: not sent, the board keeps " + dec)
            return
        }
        const snap = d === 3 ? 4 : d >= 5 && d <= 7 ? 8 : d >= 9 && d <= 15 ? 16 : d
        set("adc_decimation", snap)
        if (snap !== d)
            say("adc_decimation", d + " is not one the FPGA takes: set to " + snap + ". It takes 1, 2, 4, 8, 16 and every value from 17 to 65536", "warn")
    }

    ColumnLayout {
        spacing: 4
        Label2 { text: "ADC"; kind: "title" }
        Label2 {
            kind: "sub"
            Layout.fillWidth: true
            text: "The inputs " + Array.from({ length: root.n }, (_, i) => "IN" + (i + 1)).join(", ") + " to a file on this computer or on the SD card of the board."
        }
    }

    Card {
        Layout.fillWidth: true
        RowLayout {
            Layout.fillWidth: true
            Label2 { text: "Destination"; kind: "cardTitle"; Layout.fillWidth: true }
            Seg {
                options: ["This PC", "SD card of the board"]
                currentIndex: root.toSd ? 1 : 0
                onPicked: (i) => root.set("adc_pass_mode", i === 1 ? "FILE" : "NET")
            }
        }
        GridLayout {
            Layout.fillWidth: true
            columns: Math.max(1, Math.floor(width / 200))
            columnSpacing: 16
            rowSpacing: 12
            Field {
                label: "Format"
                Layout.fillWidth: true
                Select {
                    Layout.fillWidth: true
                    options: [["WAV", "WAV"], ["TDMS", "TDMS"], ["BIN", "BIN"]]
                    value: root.format
                    onPicked: (x) => root.set("format_sd", x)
                }
            }
            Field {
                label: "Values"
                Layout.fillWidth: true
                Select {
                    Layout.fillWidth: true
                    options: [["RAW", "Raw ADC codes"], ["VOLT", "Volts"]]
                    value: root.v("data_type_sd")
                    onPicked: (x) => root.set("data_type_sd", x)
                }
            }
            Field {
                label: "Samples per channel"
                hint: "0 - until the stop"
                Layout.fillWidth: true
                TextIn {
                    Layout.fillWidth: true
                    mono: true
                    value: root.v("samples_limit_sd")
                    validator: RegularExpressionValidator { regularExpression: /[0-9]{1,19}/ }
                    onCommitted: (t) => root.set("samples_limit_sd", t === "" ? "0" : t)
                }
            }
            Field {
                visible: root.format === "TDMS"
                label: "TDMS time column"
                Layout.fillWidth: true
                Select {
                    Layout.fillWidth: true
                    options: [["OFF", "Off"], ["ON", "On"]]
                    value: root.v("tdms_time_column")
                    onPicked: (x) => root.set("tdms_time_column", x)
                }
            }
        }
        Label2 {
            kind: "hint"
            Layout.fillWidth: true
            text: root.toSd ? "The board writes /home/redpitaya/streaming_files; the preview has no data meanwhile."
                            : "Written to " + (b ? (b.pcRev, b.recordDir()) : "") + " (This PC tab)."
        }
    }

    GridLayout {
        Layout.fillWidth: true
        columns: width > 700 ? 2 : 1
        columnSpacing: 16
        rowSpacing: 16

        Card {
            title: "Channels"
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: 10
            // the table of the inputs
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                Label2 { text: "Input"; kind: "hint"; font.bold: true; Layout.preferredWidth: 48 + 4 * Theme.fsAdd }
                Label2 { text: "On"; kind: "hint"; font.bold: true; Layout.preferredWidth: 62 + 4 * Theme.fsAdd }
                Label2 { text: "Range"; kind: "hint"; font.bold: true; Layout.fillWidth: true; Layout.preferredWidth: 10 }
                Label2 { text: "Coupling"; kind: "hint"; font.bold: true; Layout.fillWidth: true; Layout.preferredWidth: 10 }
            }
            Rectangle { Layout.fillWidth: true; height: 1; color: "#777777" }
            Repeater {
                model: root.n
                delegate: RowLayout {
                    required property int index
                    readonly property int ch: index + 1
                    Layout.fillWidth: true
                    spacing: 12
                    Text { text: "IN" + ch; color: Theme.text; font.pixelSize: Theme.fsMd; Layout.preferredWidth: 48 + 4 * Theme.fsAdd }
                    Flip {
                        small: true
                        checked: root.v("channel_state_" + ch) === "ON"
                        onClicked: root.set("channel_state_" + ch, checked ? "ON" : "OFF")
                        Accessible.name: "IN" + ch + " on"
                    }
                    Select {
                        visible: !!root.caps.adcRanges
                        Layout.fillWidth: true
                        Layout.preferredWidth: 10
                        options: [["A_1_1", "1:1 (±1 V)"], ["A_1_20", "1:20 (±20 V)"]]
                        value: root.v("channel_attenuator_" + ch)
                        onPicked: (x) => root.set("channel_attenuator_" + ch, x)
                    }
                    Label2 { visible: !root.caps.adcRanges; kind: "hint"; text: "one range, " + (root.caps.fixedRange || "±1 V"); Layout.fillWidth: true; Layout.preferredWidth: 10 }
                    Select {
                        visible: !!root.caps.adcACDC
                        Layout.fillWidth: true
                        Layout.preferredWidth: 10
                        options: [["DC", "DC"], ["AC", "AC"]]
                        value: root.v("channel_ac_dc_" + ch)
                        onPicked: (x) => root.set("channel_ac_dc_" + ch, x)
                    }
                    Label2 { visible: !root.caps.adcACDC; kind: "hint"; text: "DC only"; Layout.fillWidth: true; Layout.preferredWidth: 10 }
                }
            }
            Label2 { visible: root.on === 0; kind: "bad"; text: "No input is on: the ADC does not start."; Layout.fillWidth: true }
        }

        Card {
            title: "Sampling"
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: 10
            GridLayout {
                Layout.fillWidth: true
                columns: width > 440 ? 2 : 1
                columnSpacing: 16
                rowSpacing: 12
                Field {
                    label: "Decimation"
                    hint: root.msg("adc_decimation") || Theme.rate(root.per) + " per channel"
                    hintBad: root.msgBad("adc_decimation")
                    hintWarn: root.msgWarn("adc_decimation")
                    Layout.fillWidth: true
                    TextIn {
                        Layout.fillWidth: true
                        mono: true
                        objectName: "field:adc_decimation"
                        bad: root.msgBad("adc_decimation")
                        value: String(root.dec)
                        validator: RegularExpressionValidator { regularExpression: /-?[0-9]{0,9}/ }
                        onCommitted: (t) => root.setDecimation(t)
                    }
                }
                Field {
                    label: "Resolution"
                    Layout.fillWidth: true
                    Select {
                        Layout.fillWidth: true
                        options: [["BIT_16", "16 bit"], ["BIT_8", "8 bit"]]
                        value: root.v("resolution")
                        onPicked: (x) => root.set("resolution", x)
                    }
                }
            }
            Flow {
                Layout.fillWidth: true
                spacing: 20
                Flip {
                    label: "Calibration"
                    checked: root.v("use_calib") === "ON"
                    onClicked: root.set("use_calib", checked ? "ON" : "OFF")
                }
                Flip {
                    label: "Time of each buffer"
                    checked: root.v("adc_capture_time") === "ON"
                    onClicked: root.set("adc_capture_time", checked ? "ON" : "OFF")
                }
            }
            Label2 {
                kind: "hint"
                Layout.fillWidth: true
                text: "Network: " + Theme.bytes(root.per * root.on * root.bytes) + "/s for " + root.on + " channel" + (root.on === 1 ? "" : "s") + " at " + root.bytes * 8 + " bit."
                      + (root.per * root.on * root.bytes > 100e6 && !root.toSd ? " More than 1 GbE carries: data will be lost." : "")
            }
        }
    }

    AdcTrigger {
        b: root.b
        Layout.fillWidth: true
    }
}
