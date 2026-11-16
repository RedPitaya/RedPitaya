import QtQuick
import QtQuick.Layouts
import "../ui"

// The ADC trigger: as the Trigger card of the ADC page of the board
Card {
    id: root
    property var b

    readonly property int rev: b ? b.settingsRev : 0
    function v(key) { return rev >= 0 && b ? b.value(key) : "" }
    function set(key, value) { b.setValue(key, String(value)) }

    readonly property var caps: b ? b.caps : ({})
    readonly property int n: Math.max(1, Math.min(4, caps.adcChannels || 2))
    readonly property bool on: v("adc_trigger_mode") === "ON"
    readonly property int mask: Number(v("adc_trigger_sources")) || 0
    readonly property var trig: b ? b.trigger : ({})
    readonly property bool running: b ? (b.state === "streaming" || b.state === "armed") : false

    // trigger source bits (stream_app 0x8): DAC A/B, the level of IN1/IN2 (IN3/IN4 of the
    // 4-input board take the DAC bits), the GPIO pattern, the external input
    readonly property var src: ({ dacA: 1, dacB: 2, gpio: 16, ext: 32 })
    readonly property var inBits: [4, 8, 1, 2]
    readonly property int inMask: {
        let m = 0
        for (let i = 0; i < n; i++) m |= inBits[i]
        return m
    }
    readonly property bool levelUsed: (mask & inMask) !== 0
    function toggle(bit, checked) { set("adc_trigger_sources", checked ? (mask | bit) : (mask & ~bit)) }

    // the block mode: a number of blocks after each trigger
    readonly property int blocks: Number(v("adc_trigger_blocks")) || 0
    readonly property int blockSamples: Math.floor((Number(v("block_size")) || 65536) / (v("resolution") === "BIT_8" ? 1 : 2))
    readonly property double per: (caps.adcRate || 125000000) / Math.max(1, Number(v("adc_decimation")) || 1)

    RowLayout {
        Layout.fillWidth: true
        spacing: 12
        Label2 { text: "Trigger"; kind: "cardTitle"; Layout.fillWidth: true }
        Dot {
            visible: root.on && root.running
            color: root.trig.armed ? Theme.accent : Theme.muted
        }
        Label2 {
            visible: root.on && root.running
            kind: "hint"
            text: !root.trig.known ? "asking the board" : root.trig.armed ? "Armed" : "Not armed"
        }
        Btn {
            visible: root.on
            enabled: root.running
            compact: true
            kind: "outline"
            text: "Software trigger"
            iconSource: "qrc:/img/icons/bolt.svg"
            onClicked: b.softwareTrigger()
        }
        Flip {
            label: "Wait for a trigger"
            checked: root.on
            onClicked: root.set("adc_trigger_mode", checked ? "ON" : "OFF")
        }
    }

    Label2 {
        visible: !root.on
        kind: "hint"
        Layout.fillWidth: true
        text: "Off: the ADC streams from Start to Stop."
    }

    // the sources
    ColumnLayout {
        visible: root.on
        Layout.fillWidth: true
        spacing: 8
        Label2 { text: "Start on any of"; kind: "label" }
        GridLayout {
            Layout.fillWidth: true
            columns: root.width > 700 ? 3 : 1
            columnSpacing: 24
            rowSpacing: 6
            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                spacing: 4
                Label2 { text: "Inputs"; kind: "hint" }
                Repeater {
                    model: root.n
                    delegate: Check {
                        required property int index
                        text: "IN" + (index + 1) + " level"
                        checked: (root.mask & root.inBits[index]) !== 0
                        onClicked: root.toggle(root.inBits[index], checked)
                    }
                }
            }
            ColumnLayout {
                visible: !!root.caps.dac || !!root.caps.gpio
                Layout.alignment: Qt.AlignTop
                spacing: 4
                Label2 { text: "Other streams"; kind: "hint" }
                Check {
                    visible: !!root.caps.dac
                    enabled: !!root.caps.dacTriggerOutputs
                    text: "DAC A (OUT1 starts)"
                    checked: (root.mask & root.src.dacA) !== 0
                    onClicked: root.toggle(root.src.dacA, checked)
                }
                Check {
                    visible: !!root.caps.dac
                    enabled: !!root.caps.dacTriggerOutputs
                    text: "DAC B (OUT2 starts)"
                    checked: (root.mask & root.src.dacB) !== 0
                    onClicked: root.toggle(root.src.dacB, checked)
                }
                Label2 {
                    visible: !!root.caps.dac && !root.caps.dacTriggerOutputs
                    kind: "hint"
                    text: "DAC triggers need a newer FPGA"
                }
                Check {
                    visible: !!root.caps.gpio
                    text: "GPIO pattern"
                    checked: (root.mask & root.src.gpio) !== 0
                    onClicked: root.toggle(root.src.gpio, checked)
                }
            }
            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                spacing: 4
                Label2 { text: "Board"; kind: "hint" }
                Check {
                    text: "External (EXT TRIG input)"
                    checked: (root.mask & root.src.ext) !== 0
                    onClicked: root.toggle(root.src.ext, checked)
                }
            }
        }
        Label2 {
            visible: root.mask === 0
            kind: "bad"
            Layout.fillWidth: true
            text: "No source is chosen: the ADC waits for ever, only the software trigger starts it."
        }
    }

    // the level window of the inputs, in volts
    GridLayout {
        visible: root.on && root.levelUsed
        Layout.fillWidth: true
        columns: Math.max(1, Math.floor(width / 200))
        columnSpacing: 16
        rowSpacing: 12
        Field {
            label: "Level, V"
            Layout.fillWidth: true
            TextIn {
                Layout.fillWidth: true
                mono: true
                value: String(+(Number(root.v("adc_trigger_level_v")) || 0).toFixed(4))
                validator: DoubleValidator { bottom: -20; top: 20; notation: DoubleValidator.StandardNotation }
                onCommitted: (t) => {
                    root.set("adc_trigger_level_unit", "VOLT")
                    root.set("adc_trigger_level_v", t === "" ? "0" : t.replace(",", "."))
                }
            }
        }
        Field {
            label: "Hysteresis, V"
            Layout.fillWidth: true
            TextIn {
                Layout.fillWidth: true
                mono: true
                value: String(+(Number(root.v("adc_trigger_hysteresis_v")) || 0).toFixed(4))
                validator: DoubleValidator { bottom: 0; top: 20; notation: DoubleValidator.StandardNotation }
                onCommitted: (t) => {
                    root.set("adc_trigger_level_unit", "VOLT")
                    root.set("adc_trigger_hysteresis_v", t === "" ? "0" : t.replace(",", "."))
                }
            }
        }
        Field {
            label: "Edge"
            Layout.fillWidth: true
            Seg {
                options: ["Rising", "Falling"]
                currentIndex: root.v("adc_trigger_edge") === "FALLING" ? 1 : 0
                onPicked: (i) => root.set("adc_trigger_edge", i === 1 ? "FALLING" : "RISING")
            }
        }
    }

    // after a trigger: continuous or a number of blocks
    ColumnLayout {
        visible: root.on && !!root.caps.adcBlockMode
        Layout.fillWidth: true
        spacing: 8
        RowLayout {
            Layout.fillWidth: true
            Label2 { text: "After a trigger"; kind: "label"; Layout.fillWidth: true }
            Seg {
                options: ["Continuous", "Blocks per trigger"]
                currentIndex: root.blocks > 0 ? 1 : 0
                onPicked: (i) => root.set("adc_trigger_blocks", i === 1 ? (root.blocks > 0 ? root.blocks : 10) : 0)
            }
        }
        GridLayout {
            visible: root.blocks > 0
            Layout.fillWidth: true
            columns: Math.max(1, Math.floor(width / 200))
            columnSpacing: 16
            rowSpacing: 12
            Field {
                label: "Blocks per trigger"
                hint: "1 .. 65535"
                Layout.fillWidth: true
                TextIn {
                    Layout.fillWidth: true
                    mono: true
                    value: String(root.blocks)
                    validator: IntValidator { bottom: 1; top: 65535 }
                    onCommitted: (t) => root.set("adc_trigger_blocks", t === "" ? "1" : t)
                }
            }
            Field {
                label: "Series"
                hint: (root.blockSamples * root.blocks / root.per * 1000).toFixed(1) + " ms"
                Layout.fillWidth: true
                Label2 { kind: "value"; text: (root.blockSamples * root.blocks).toLocaleString(Qt.locale("en_US"), "f", 0) + " samples" }
            }
        }
    }

    // start and stop together with another stream
    Field {
        visible: root.on
        label: "Start and stop together with"
        Layout.fillWidth: true
        Select {
            Layout.preferredWidth: 280
            options: [["OWN", "None (own Start, Stop)"]]
                     .concat(root.caps.dac ? [["GEN_A", "DAC"]] : [])
                     .concat(root.caps.gpio ? [["LA", "GPIO input"]] : [])
            value: root.v("adc_event_source") || "OWN"
            onPicked: (x) => root.set("adc_event_source", x)
        }
    }

    Label2 {
        visible: root.on && root.running && root.trig.known
        kind: "hint"
        Layout.fillWidth: true
        text: "Armed: " + (root.trig.armed ? "yes" : "no") + " · triggers seen: " + Number(root.trig.count || 0).toLocaleString(Qt.locale("en_US"), "f", 0)
    }
}
