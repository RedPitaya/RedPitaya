import QtQuick
import QtQuick.Layouts
import "../ui"

// Triggers (streaming_improvements 4.17): who starts whom, the external trigger input, the
// streams that start and stop together. As the Triggers section of the web page of the board;
// the levels of the ADC inputs stay in the ADC tab, the GPIO pattern in the GPIO tab.
ColumnLayout {
    id: root
    property var b
    spacing: 18

    readonly property int rev: b ? b.settingsRev : 0
    function v(key) { return rev >= 0 && b ? b.value(key) : "" }
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
    readonly property int n: Math.max(1, Math.min(4, caps.adcChannels || 2))
    readonly property bool hasDac: !!caps.dac
    readonly property bool hasGpio: !!caps.gpio
    readonly property bool dacOut: !!caps.dacTriggerOutputs

    // the source bits (stream_app 0x8): DAC A/B, IN1/IN2 levels (IN3/IN4 of the 4-input board take
    // the DAC bits), the GPIO pattern, the external input
    readonly property var inBits: [4, 8, 1, 2]
    readonly property var cols: {
        let c = []
        for (let i = 0; i < n; i++) c.push(["IN" + (i + 1), inBits[i], "adc"])
        if (hasDac && n <= 2) c.push(["DAC A", 1, "dac"], ["DAC B", 2, "dac"])
        if (hasGpio) c.push(["GPIO", 16, "gpio"])
        c.push(["External", 32, "ext"])
        return c
    }
    // stream, its sources key, its wait key(s), its blocks key ("" none)
    readonly property var rows: {
        let r = [["ADC", "adc_trigger_sources", ["adc_trigger_mode"], "adc_trigger_blocks", "adc"]]
        if (hasDac) r.push(["DAC OUT1, OUT2", "dac_trigger_sources", ["dac_trigger_mode_1", "dac_trigger_mode_2"], "", "dac"])
        if (hasGpio) r.push(["GPIO input", "gpio_trigger_sources", ["gpio_in_trigger_mode"], "gpio_trigger_blocks", "gpio"])
        return r
    }
    function waits(row) { return row[2].some(k => v(k) === "ON") }
    function toggle(key, bit, on) {
        const m = Number(v(key)) || 0
        set(key, on ? (m | bit) : (m & ~bit))
    }
    // a stream does not trigger itself; the DAC bits need the DAC trigger outputs of the FPGA
    function cellUsable(row, col) {
        if (row[4] === col[2] && col[2] !== "adc") return false
        if (col[2] === "dac" && !dacOut) return false
        return true
    }
    readonly property double clock: caps.adcRate || 125000000

    ColumnLayout {
        spacing: 4
        Label2 { text: "Triggers"; kind: "title" }
        Label2 { kind: "sub"; Layout.fillWidth: true; text: "What starts each stream that waits for a trigger, and the external trigger input. Whether a stream waits is set here or in its own tab." }
    }

    Card {
        title: "Sources of each stream"
        subtitle: "A stream that waits starts on any source switched on."
        Layout.fillWidth: true
        GridLayout {
            Layout.fillWidth: true
            columns: root.cols.length + 3
            columnSpacing: 14
            rowSpacing: 8
            // the head
            Label2 { text: ""; kind: "hint" }
            Label2 { text: "waits"; kind: "hint"; Layout.alignment: Qt.AlignHCenter }
            Repeater {
                model: root.cols
                delegate: Label2 { required property var modelData; text: modelData[0]; kind: "hint"; Layout.alignment: Qt.AlignHCenter }
            }
            Label2 { text: "after a trigger"; kind: "hint" }
            // a row a stream
            Repeater {
                model: root.rows.length * (root.cols.length + 3)
                delegate: Item {
                    id: cell
                    required property int index
                    readonly property int cols: root.cols.length + 3
                    readonly property var row: root.rows[Math.floor(index / cols)]
                    readonly property int c: index % cols
                    implicitWidth: lay.implicitWidth
                    implicitHeight: Math.max(28, lay.implicitHeight)
                    Layout.alignment: c === 0 || c === cols - 1 ? Qt.AlignLeft | Qt.AlignVCenter : Qt.AlignHCenter | Qt.AlignVCenter
                    Loader {
                        id: lay
                        anchors.verticalCenter: parent.verticalCenter
                        sourceComponent: cell.c === 0 ? nameC : cell.c === 1 ? waitC : cell.c === cell.cols - 1 ? blocksC : bitC
                        Component { id: nameC; Label2 { text: cell.row[0]; kind: "value" } }
                        Component {
                            id: waitC
                            Flip {
                                small: true
                                checked: root.waits(cell.row)
                                Accessible.name: cell.row[0] + " waits for a trigger"
                                onClicked: { for (const k of cell.row[2]) root.set(k, checked ? "ON" : "OFF") }
                            }
                        }
                        Component {
                            id: bitC
                            Item {
                                readonly property var col: root.cols[cell.c - 2]
                                implicitWidth: ck.visible ? ck.implicitWidth : dash.implicitWidth
                                implicitHeight: 24
                                Check {
                                    id: ck
                                    visible: root.cellUsable(cell.row, parent.col)
                                    anchors.centerIn: parent
                                    padding: 0
                                    checked: ((Number(root.v(cell.row[1])) || 0) & parent.col[1]) !== 0
                                    onClicked: { root.toggle(cell.row[1], parent.col[1], checked); checked = Qt.binding(() => ((Number(root.v(cell.row[1])) || 0) & parent.col[1]) !== 0) }
                                    Accessible.name: cell.row[0] + " from " + parent.col[0]
                                }
                                Label2 { id: dash; visible: !ck.visible; anchors.centerIn: parent; text: "-"; kind: "hint" }
                            }
                        }
                        Component {
                            id: blocksC
                            Item {
                                implicitWidth: 170
                                implicitHeight: 32
                                Label2 { visible: cell.row[3] === ""; anchors.verticalCenter: parent.verticalCenter; text: "-"; kind: "hint" }
                                TextIn {
                                    visible: cell.row[3] !== ""
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: 90
                                    mono: true
                                    value: String(Number(root.v(cell.row[3])) || 0)
                                    validator: IntValidator { bottom: 0; top: 65535 }
                                    onCommitted: (t) => root.set(cell.row[3], t === "" ? "0" : t)
                                    Accessible.name: cell.row[0] + " blocks after a trigger"
                                }
                                Label2 {
                                    visible: cell.row[3] !== ""
                                    x: 98
                                    anchors.verticalCenter: parent.verticalCenter
                                    kind: "hint"
                                    text: (Number(root.v(cell.row[3])) || 0) > 0 ? "blocks" : "continuous"
                                }
                            }
                        }
                    }
                }
            }
        }
        Label2 {
            Layout.fillWidth: true
            kind: "hint"
            text: "The ADC levels stay in the ADC tab (Trigger), the GPIO pattern in the GPIO tab. DAC A, DAC B: when OUT1, OUT2 start"
                  + (root.hasDac && !root.dacOut ? " (needs a newer FPGA with DAC trigger outputs)" : "") + ". The software trigger is a button of the card."
        }
        Label2 {
            visible: Object.keys(root.msgs).length > 0
            Layout.fillWidth: true
            kind: "bad"
            text: Object.values(root.msgs).join(" · ")
        }
    }

    Card {
        visible: !!root.caps.extTrigger || !root.caps.fromBoard
        title: "External trigger input"
        subtitle: "Shared by every stream that takes the external source."
        Layout.fillWidth: true
        GridLayout {
            Layout.fillWidth: true
            columns: Math.max(1, Math.floor(width / 230))
            columnSpacing: 16
            rowSpacing: 12
            Field {
                label: "Input"
                Layout.fillWidth: true
                Select {
                    Layout.fillWidth: true
                    options: [["OFF", "EXT TRIG connector"], ["ON", "DIO0_P line"]]
                    value: root.v("ext_trig_dio0") === "ON" ? "ON" : "OFF"
                    onPicked: (x) => root.set("ext_trig_dio0", x)
                }
            }
            Field {
                label: "Edge"
                Layout.fillWidth: true
                Seg {
                    options: ["Rising", "Falling"]
                    currentIndex: root.v("ext_trig_edge") === "FALLING" ? 1 : 0
                    onPicked: (i) => root.set("ext_trig_edge", i === 1 ? "FALLING" : "RISING")
                }
            }
            Field {
                label: "Edge detector and debounce"
                Layout.fillWidth: true
                Flip {
                    checked: root.v("ext_trig_conditioner") !== "OFF"
                    onClicked: root.set("ext_trig_conditioner", checked ? "ON" : "OFF")
                }
            }
            Field {
                label: "Debounce (clocks)"
                hint: root.msgs["ext_trig_debounce"] || ((Number(root.v("ext_trig_debounce")) || 0) === 0 ? "0 - off" : ((Number(root.v("ext_trig_debounce")) || 0) / root.clock * 1e6).toFixed(2) + " µs at " + (root.clock / 1e6) + " MHz")
                hintBad: !!root.msgs["ext_trig_debounce"]
                Layout.fillWidth: true
                TextIn {
                    Layout.fillWidth: true
                    mono: true
                    value: String(Number(root.v("ext_trig_debounce")) || 0)
                    validator: IntValidator { bottom: 0; top: 65535 }
                    onCommitted: (t) => root.set("ext_trig_debounce", t === "" ? "0" : t)
                }
            }
        }
    }

    Card {
        title: "Start and stop together"
        subtitle: "A stream that follows another one starts, stops and triggers with it."
        Layout.fillWidth: true
        GridLayout {
            Layout.fillWidth: true
            columns: Math.max(1, Math.floor(width / 260))
            columnSpacing: 16
            rowSpacing: 12
            Repeater {
                // the stream, its key, the streams it may follow
                model: [["ADC with", "adc_event_source", "adc"]]
                       .concat(root.hasDac ? [["DAC with", "dac_event_source", "dac"]] : [])
                       .concat(root.hasGpio ? [["GPIO with", "gpio_event_source", "gpio"]] : [])
                delegate: Field {
                    required property var modelData
                    label: modelData[0]
                    Layout.fillWidth: true
                    Select {
                        Layout.fillWidth: true
                        options: [["OWN", "itself (own Start, Stop)"]]
                                 .concat(modelData[2] !== "adc" ? [["OSC1", "ADC"]] : [])
                                 .concat(modelData[2] !== "dac" && root.hasDac ? [["GEN_A", "DAC"]] : [])
                                 .concat(modelData[2] !== "gpio" && root.hasGpio ? [["LA", "GPIO input"]] : [])
                        value: root.v(modelData[1]) || "OWN"
                        onPicked: (x) => root.set(modelData[1], x)
                    }
                }
            }
        }
    }
}
