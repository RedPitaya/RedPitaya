import QtQuick
import QtQuick.Layouts
import "../ui"

// Memory: the DMA memory of the board split between the streams (as the Memory page of the board)
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
    readonly property int header: caps.headerSize || 128
    readonly property double total: caps.memorySize || 0
    readonly property double maxBlock: caps.memoryMaxBlock || 8388608
    readonly property int block: Number(v("block_size")) || 65536
    readonly property int blk: block + header
    readonly property var rows: {
        const all = [["adc", "ADC", "adc_size", true], ["dac", "DAC", "dac_size", !!caps.dac],
                     ["gin", "GPIO input", "gpio_in_size", !!caps.gpio], ["gout", "GPIO output", "gpio_out_size", !!caps.gpio]]
        return all.filter(r => r[3]).map(r => {
            const size = Number(v(r[2])) || 0
            const n = Math.floor(size / blk)
            return { k: r[0], name: r[1], key: r[2], size: size, n: n, on: size > 0, ok: size === 0 || n >= 12, color: Theme.streamColors[r[0]] }
        })
    }
    readonly property double used: rows.reduce((a, r) => a + r.size, 0)
    readonly property bool over: total > 0 && used > total
    readonly property int live: Math.max(1, rows.filter(r => r.on).length)
    readonly property var presets: [2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608].filter(x => x + header <= maxBlock)
    property bool custom: presets.indexOf(block) < 0

    function mb(x) { return (x / 1048576).toFixed(2) + " MB" }
    // a multiple of 128 up to 256 MB and the largest DMA block of the board, 12 blocks a stream
    readonly property double blockMax: Math.min(268435456, maxBlock - header)
    function setBlock(t) {
        const x = Number(t)
        if (t === "" || !Number.isInteger(x) || x < 128) {
            say("block_size", "From 128 bytes up: not sent")
            return
        }
        if (x % 128) {
            const lo = Math.floor(x / 128) * 128
            say("block_size", "Not a multiple of 128: " + (lo >= 128 ? lo + " or " : "") + (lo + 128) + "? Not sent")
            return
        }
        if (x > blockMax) {
            say("block_size", "Up to " + Theme.bytes(blockMax) + " (the largest DMA block of the board): not sent")
            return
        }
        if (!fits(x)) {
            say("block_size", "12 blocks per stream do not fit the " + mb(total) + " of the board: not sent")
            return
        }
        say("block_size", "")
        set("block_size", x)
    }
    function fits(x) { return total === 0 || 12 * (x + header) * live <= total }

    ColumnLayout {
        spacing: 4
        Label2 { text: "Memory"; kind: "title" }
        Label2 { kind: "sub"; Layout.fillWidth: true; text: "The DMA memory of the board, split between the streams. A new block size or split stops every stream." }
    }

    Card {
        Layout.fillWidth: true
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            Field {
                label: "Block size"
                Layout.preferredWidth: 280
                Select {
                    Layout.fillWidth: true
                    options: root.presets.map(x => [x, Theme.bytes(x) + (root.fits(x) ? "" : " - 12 blocks per stream do not fit")]).concat([["custom", "Custom…"]])
                    value: root.custom ? "custom" : root.block
                    onPicked: (x) => { if (x === "custom") root.custom = true; else { root.custom = false; root.set("block_size", x) } }
                }
            }
            Field {
                visible: root.custom
                label: "Custom, bytes"
                hint: root.msg("block_size") || "a multiple of 128, up to " + Theme.bytes(root.blockMax) + " · " + Theme.bytes(root.block)
                hintBad: root.msgBad("block_size")
                Layout.preferredWidth: 260
                TextIn {
                    Layout.fillWidth: true
                    mono: true
                    objectName: "field:block_size"
                    bad: root.msgBad("block_size")
                    value: String(root.block)
                    validator: RegularExpressionValidator { regularExpression: /[0-9]{0,10}/ }
                    onCommitted: (t) => root.setBlock(t)
                }
            }
            Item { Layout.fillWidth: true }
            Btn {
                kind: "outline"; text: "Minimum"
                Layout.alignment: Qt.AlignBottom
                onClicked: { for (const r of root.rows) if (r.on) root.set(r.key, 12 * root.blk) }
            }
            Btn {
                kind: "outline"; text: "Split evenly"
                visible: root.total > 0
                Layout.alignment: Qt.AlignBottom
                onClicked: {
                    const each = Math.floor(root.total / root.live / root.blk) * root.blk
                    for (const r of root.rows) if (r.on) root.set(r.key, each)
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: Math.max(1, Math.floor(width / 220))
            columnSpacing: 16
            rowSpacing: 12
            Repeater {
                model: root.rows
                delegate: Field {
                    required property var modelData
                    readonly property var r: modelData
                    Layout.fillWidth: true
                    opacity: r.on ? 1 : 0.45
                    label: r.name + ", bytes"
                    hintBad: !r.ok
                    hint: r.on ? r.n + " blocks · " + root.mb(r.size) + (r.n < 12 ? " · at least 12" : "") + (r.size % root.blk ? " · " + (r.size % root.blk) + " B unused" : "")
                               : "off · no memory, the stream does not start"
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 4
                        Btn { kind: "outline"; text: "−"; implicitWidth: implicitHeight; Accessible.name: r.name + ": one block less"; onClicked: root.set(r.key, Math.max(0, r.size - root.blk)) }
                        TextIn {
                            Layout.fillWidth: true
                            mono: true
                            value: String(r.size)
                            validator: IntValidator { bottom: 0; top: 2147483647 }
                            onCommitted: (t) => root.set(r.key, t === "" ? "0" : t)
                        }
                        Btn { kind: "outline"; text: "+"; implicitWidth: implicitHeight; Accessible.name: r.name + ": one block more"; onClicked: root.set(r.key, r.size + root.blk) }
                    }
                }
            }
        }

        // the bar
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 14
            radius: Theme.radius
            color: Theme.hover
            clip: true
            Row {
                anchors.fill: parent
                readonly property double scale: Math.max(root.total, root.used, 1)
                Repeater {
                    model: root.rows
                    delegate: Rectangle {
                        required property var modelData
                        height: 14
                        width: parent.width * modelData.size / parent.scale
                        color: modelData.color
                    }
                }
            }
        }
        Flow {
            Layout.fillWidth: true
            spacing: 24
            Repeater {
                model: root.rows
                delegate: Row {
                    required property var modelData
                    readonly property var r: modelData
                    spacing: 10
                    Flip {
                        small: true
                        checked: r.on
                        Accessible.name: r.name + " gets memory"
                        anchors.verticalCenter: parent.verticalCenter
                        onClicked: root.set(r.key, checked ? 12 * root.blk : 0)
                    }
                    Dot { color: r.color; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: r.name; color: r.on ? Theme.text : Theme.dim; font.pixelSize: Theme.fsSm; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: root.mb(r.size); color: Theme.value; font.family: Theme.mono; font.pixelSize: Theme.fsSm; anchors.verticalCenter: parent.verticalCenter }
                    Text { visible: root.total > 0; text: (r.size / Math.max(1, root.total) * 100).toFixed(0) + "%"; color: Theme.muted; font.family: Theme.mono; font.pixelSize: Theme.fsSm; anchors.verticalCenter: parent.verticalCenter }
                }
            }
        }
        Label2 {
            kind: root.over ? "bad" : "hint"
            Layout.fillWidth: true
            text: root.total > 0
                ? (root.over ? "Over by " + root.mb(root.used - root.total) + ": " : "") + "used " + root.mb(root.used) + " of " + root.mb(root.total) + ", free " + root.mb(Math.max(0, root.total - root.used)) + ". Each stream needs at least 12 blocks of " + Theme.bytes(root.block) + " + " + root.header + " B."
                : "Used " + root.mb(root.used) + ". The board does not tell how much memory it has (an older streaming application). Each stream needs at least 12 blocks of " + Theme.bytes(root.block) + " + " + root.header + " B."
        }
    }
}
