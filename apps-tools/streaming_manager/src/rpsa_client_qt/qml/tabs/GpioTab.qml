import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "../ui"
import "limits.js" as Lim

// GPIO (streaming_improvements 4.17): DIO0..7_P and DIO0..7_N, the input into a file and a file on
// the output, as the GPIO section of the web page of the board. This PC and Off of a direction
// and its file of this computer are kept on this computer (pcValue).
ColumnLayout {
    id: root
    property var b
    spacing: 18

    readonly property int rev: b ? b.settingsRev : 0
    readonly property int pcRev: b ? b.pcRev : 0
    function v(key) { return rev >= 0 && b ? b.value(key) : "" }
    function pv(key) { return pcRev >= 0 && b ? b.pcValue(key) : "" }
    function set(key, value) { say(key, ""); b.setValue(key, String(value)) }
    function setPc(key, value) { say(key, ""); b.setPcValue(key, value) }

    // what a field says under it, until its next edit: {key: {text, kind: bad|warn}}
    property var msgs: ({})
    onBChanged: msgs = ({})
    function say(key, text, kind) {
        const m = Object.assign({}, msgs)
        if (text) m[key] = { text: text, kind: kind || "bad" }
        else delete m[key]
        msgs = m
    }
    function msg(key) { return msgs[key] ? msgs[key].text : "" }
    function msgBad(key) { return !!msgs[key] && msgs[key].kind === "bad" }
    Connections {
        target: root.b
        function onSettingRefused(key, value) { root.say(key, "The board did not take " + value + ": it keeps " + root.v(key)) }
    }

    readonly property var caps: b ? b.caps : ({})
    // the GPIO block runs at the DAC rate on the 65-16 (twice its ADC rate), else at the ADC rate
    readonly property double clock: caps.adcRate === 62500000 ? 125000000 : (caps.adcRate || 125000000)
    readonly property int dec: Math.max(1, Number(v("gpio_decimation")) || 1)
    readonly property double per: clock / dec
    readonly property bool rle: v("gpio_rle") === "ON"
    readonly property bool inOff: String(pv("gpioInOff")) === "true"
    readonly property bool inSd: v("gpio_in_pass_mode") === "FILE"
    readonly property bool outOff: String(pv("gpioOutOff")) === "true"
    readonly property bool outSd: v("gpio_out_pass_mode") === "FILE"
    readonly property var gs: b ? b.gpioState : ({})
    readonly property var gin: gs.in || ({})
    readonly property var gout: gs.out || ({})
    readonly property bool inRunning: ["connecting", "recording", "sd", "stopping"].indexOf(gin.state) >= 0
    readonly property bool outRunning: ["connecting", "playing", "stopping"].indexOf(gout.state) >= 0
    readonly property var file: b ? b.gpioFile : ({})

    // ---- the limits of a recording: the SD card (the keys of the board) or this PC (samples, a time)
    readonly property string inFormat: inSd ? (v("gpio_in_format_sd") || "BIN") : String(pv("gpioInFormat") || "BIN")
    // bytes a second of the file where it is known: BIN without RLE, a word a sample
    readonly property double dataRate: inFormat === "BIN" && !rle ? per * 4 : 0
    readonly property string limKind: inSd
        ? (Number(v("gpio_in_limit_time_sd")) > 0 ? "time" : Number(v("gpio_in_limit_size_sd")) > 0 ? "size" : Number(v("gpio_in_samples_limit_sd")) > 0 ? "samples" : "none")
        : (Number(pv("gpioInLimit")) > 0 ? String(pv("gpioInLimitKind")) : "none")
    readonly property double limSamples: inSd ? Number(v("gpio_in_samples_limit_sd")) : Number(pv("gpioInLimit"))
    readonly property double limTime: inSd ? Number(v("gpio_in_limit_time_sd")) : Number(pv("gpioInLimit"))
    readonly property double limSize: Number(v("gpio_in_limit_size_sd"))
    readonly property string splitKind: Number(v("gpio_in_split_size_sd")) > 0 ? "size" : Number(v("gpio_in_split_time_sd")) > 0 ? "time" : "none"
    readonly property double limitBytes: limKind === "samples" ? limSamples / per * dataRate : limKind === "time" ? limTime * dataRate : limKind === "size" ? limSize : 0
    readonly property int partsAbout: limKind === "none" || splitKind === "none" || !(dataRate > 0 || splitKind === "time")
        ? 0 : Math.max(1, Math.ceil(splitKind === "size" ? limitBytes / Number(v("gpio_in_split_size_sd")) : (limKind === "samples" ? limSamples / per : limKind === "time" ? limTime : limitBytes / dataRate) / Number(v("gpio_in_split_time_sd"))))
    readonly property string limitKey: limKind === "samples" ? "gpio_in_samples_limit_sd" : limKind === "time" ? "gpio_in_limit_time_sd" : "gpio_in_limit_size_sd"
    function setLimitKind(kind) {
        if (!inSd) {
            setPc("gpioInLimitKind", kind)
            setPc("gpioInLimit", kind === "samples" ? Math.round(per * 10) : kind === "time" ? 60 : 0)
            return
        }
        if (kind !== "samples") set("gpio_in_samples_limit_sd", "0")
        if (kind !== "time") set("gpio_in_limit_time_sd", "0")
        if (kind !== "size") set("gpio_in_limit_size_sd", "0")
        if (kind === "samples") set("gpio_in_samples_limit_sd", String(Math.round(per * 10)))
        if (kind === "time") set("gpio_in_limit_time_sd", "600")
        if (kind === "size") set("gpio_in_limit_size_sd", "1073741824")
    }
    function setLimitValue(text) {
        const n = limKind === "samples" ? Number(text) : limKind === "time" ? Lim.parseTime(text) : Lim.parseSize(text)
        const key = inSd ? limitKey : "gpioInLimit"
        if (!(n > 0) || (limKind === "samples" && !Number.isInteger(n))) { say(key, "Not a " + (limKind === "time" ? "time: 90 s, 10 min, 2 h" : limKind === "size" ? "size: 500 MB, 2 GB" : "number of samples")); return }
        if (inSd) set(key, String(n))
        else setPc(key, n)
    }
    function setSplitKind(kind) {
        set("gpio_in_split_size_sd", kind === "size" ? "1073741824" : "0")
        set("gpio_in_split_time_sd", kind === "time" ? "300" : "0")
    }
    function setSplitValue(text) {
        const n = splitKind === "size" ? Lim.parseSize(text) : Lim.parseTime(text)
        const key = splitKind === "size" ? "gpio_in_split_size_sd" : "gpio_in_split_time_sd"
        if (!(n > 0) || (splitKind === "size" && n < 1048576)) { say(key, splitKind === "size" ? "A size of 1 MB or more: 500 MB, 1 GB" : "A time: 30 s, 5 min, 1 h"); return }
        set(key, String(n))
    }
    function aboutBytes(bytes) { return bytes > 0 ? ", about " + Theme.bytes(bytes) : "" }
    readonly property string limitHint: limKind === "samples" ? "= " + Lim.dur(limSamples / per) + aboutBytes(limitBytes)
        : limKind === "time" ? Lim.fmtTime(limTime) + " = " + Theme.count(limTime * per) + " samples" + aboutBytes(limitBytes)
        : limKind === "size" ? (dataRate > 0 ? "about " + Lim.dur(limitBytes / dataRate) + " at " + Theme.bytes(dataRate) + "/s" : "the time depends on how often the lines change")
        : "until the stop" + (inSd ? " or a full SD card" : " or a full disk")
    readonly property string limitText: limKind === "samples" ? String(limSamples) : limKind === "time" ? Lim.fmtTime(limTime) : Lim.fmtSize(limSize, Theme.bytes(limSize))

    // ---- the lines: bit i - P i, bit 8 + i - N i
    readonly property int outMask: (Number(v("gpio_outputs_p")) & 0xFF) | ((Number(v("gpio_outputs_n")) & 0xFF) << 8)
    readonly property int pol: Number(v("gpio_polarity")) & 0xFFFF
    function hex4(x) { return "0x" + (x >>> 0).toString(16).toUpperCase().padStart(4, "0") }
    function setOut(mask) {
        set("gpio_outputs_p", mask & 0xFF)
        set("gpio_outputs_n", (mask >> 8) & 0xFF)
    }
    // the pattern of a line: 0 any, 1 low, 2 high, 3 rising, 4 falling
    readonly property int pm: Number(v("gpio_pattern_mask")) & 0xFFFF
    readonly property int pval: Number(v("gpio_pattern_value")) & 0xFFFF
    readonly property int ppos: Number(v("gpio_pattern_pos_edge")) & 0xFFFF
    readonly property int pneg: Number(v("gpio_pattern_neg_edge")) & 0xFFFF
    function patOf(k) {
        const bit = 1 << k
        if (ppos & bit) return 3
        if (pneg & bit) return 4
        if (pm & bit) return pval & bit ? 2 : 1
        return 0
    }
    function setPattern(pat) {
        let m = 0, val = 0, pos = 0, neg = 0
        pat.forEach((p, k) => {
            const bit = 1 << k
            if (p === 1 || p === 2) m |= bit
            if (p === 2) val |= bit
            if (p === 3) pos |= bit
            if (p === 4) neg |= bit
        })
        set("gpio_pattern_mask", m)
        set("gpio_pattern_value", val)
        set("gpio_pattern_pos_edge", pos)
        set("gpio_pattern_neg_edge", neg)
    }
    function cyclePattern(k) {
        const pats = Array.from({ length: 16 }, (_, i) => patOf(i))
        pats[k] = (pats[k] + 1) % 5
        setPattern(pats)
    }

    readonly property string fileText: !file.path ? "A BIN of GPIO output words or a recording of the GPIO input (as rpsa_client -G --out takes)"
        : file.reading ? "Reading the file..."
        : !file.ok ? (file.error || "")
        : [file.kind === "in" ? "A recording of the GPIO input" : "GPIO output words",
           Theme.count(file.words) + " words",
           Theme.count(file.samples) + " samples",
           file.rle ? "RLE" : "",
           file.kind === "in" && file.clock > 0 ? "recorded at " + Theme.rate(file.clock / Math.max(1, file.decimation)) : "",
           Theme.bytes(file.size || 0)]
          .filter(x => x !== "").join(" · ")

    ColumnLayout {
        spacing: 4
        Label2 { text: "GPIO"; kind: "title" }
        Label2 { kind: "sub"; Layout.fillWidth: true; text: "DIO0..7_P and DIO0..7_N: the input into a file, a file on the output, or both. An output line reads back its own level on the input." }
    }

    GridLayout {
        Layout.fillWidth: true
        // two cards side by side only where the table of the lines fits in half (a larger text makes it wider)
        columns: width > Math.max(900, 2 * (lines.implicitWidth + 36) + columnSpacing) ? 2 : 1
        columnSpacing: 16
        rowSpacing: 16

        // ---- the input
        Card {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: 10
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                Label2 { text: "Input"; kind: "cardTitle"; Layout.fillWidth: true }
                Seg {
                    options: ["This PC", "SD card", "Off"]
                    currentIndex: root.inOff ? 2 : root.inSd ? 1 : 0
                    onPicked: (i) => {
                        root.b.setPcValue("gpioInOff", i === 2)
                        if (i < 2) root.set("gpio_in_pass_mode", i === 1 ? "FILE" : "NET")
                    }
                }
            }
            GridLayout {
                visible: !root.inOff
                Layout.fillWidth: true
                columns: width > 440 ? 2 : 1
                columnSpacing: 16
                rowSpacing: 12
                Field {
                    label: "Format"
                    Layout.fillWidth: true
                    Layout.preferredWidth: 10
                    hint: root.inFormat === "BIN" ? "the words as they come; the viewer and the API read it" : root.inFormat === "CSV" ? "sample,p,n: a row where the lines change" : "value change dump of p[7:0] and n[7:0]"
                    Select {
                        Layout.fillWidth: true
                        options: [["BIN", "BIN"], ["CSV", "CSV"], ["VCD", "VCD"]]
                        value: root.inFormat
                        onPicked: (x) => root.inSd ? root.set("gpio_in_format_sd", x) : root.setPc("gpioInFormat", x)
                    }
                }
                Field {
                    label: "Stop after"
                    Layout.fillWidth: true
                    Layout.preferredWidth: 10
                    hint: root.msg(root.inSd ? root.limitKey : "gpioInLimit") || root.limitHint
                    hintBad: root.msgBad(root.inSd ? root.limitKey : "gpioInLimit")
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        Select {
                            Layout.fillWidth: true
                            options: [["none", "the stop"], ["samples", "samples"], ["time", "a time"]].concat(root.inSd ? [["size", "a size of the data"]] : [])
                            value: root.limKind
                            onPicked: (x) => root.setLimitKind(x)
                        }
                        TextIn {
                            objectName: "field:gpioLimit"
                            visible: root.limKind !== "none"
                            implicitWidth: 110
                            mono: true
                            value: root.limitText
                            onCommitted: (t) => root.setLimitValue(t)
                        }
                    }
                }
                Field {
                    visible: root.inSd
                    label: "Split into parts"
                    Layout.fillWidth: true
                    Layout.preferredWidth: 10
                    Layout.columnSpan: parent.columns
                    hint: root.msg("gpio_in_split_size_sd") || root.msg("gpio_in_split_time_sd") || (
                        root.splitKind === "none" ? "one file" : (root.partsAbout ? "about " + root.partsAbout + " parts: " : "") + "…_0001, …_0002; each a whole file with its report")
                    hintBad: root.msgBad("gpio_in_split_size_sd") || root.msgBad("gpio_in_split_time_sd")
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        Select {
                            Layout.fillWidth: true
                            options: [["none", "off (one file)"], ["size", "every … of data"], ["time", "every … of time"]]
                            value: root.splitKind
                            onPicked: (x) => root.setSplitKind(x)
                        }
                        TextIn {
                            objectName: "field:gpioSplit"
                            visible: root.splitKind !== "none"
                            implicitWidth: 110
                            mono: true
                            value: root.splitKind === "size" ? Lim.fmtSize(Number(root.v("gpio_in_split_size_sd")), Theme.bytes(Number(root.v("gpio_in_split_size_sd")))) : Lim.fmtTime(Number(root.v("gpio_in_split_time_sd")))
                            onCommitted: (t) => root.setSplitValue(t)
                        }
                    }
                }
            }
            Label2 {
                Layout.fillWidth: true
                kind: root.inOff ? "hint" : !root.inSd && root.dataRate > 100e6 ? "warn" : "hint"
                text: root.inOff ? "Off: Start all of this board leaves the input out, and its chip on the card does not start it."
                    : root.inSd ? "The board writes /home/redpitaya/streaming_files/gpio (gpio_<date>_<time>." + root.inFormat.toLowerCase() + "); nothing comes to this computer."
                    : "Written to " + (b ? (b.pcRev, b.recordDir()) : "") + " as <name>_gpio." + root.inFormat.toLowerCase() + " (This PC tab), from port 18904."
                      + (root.dataRate > 100e6 ? " " + Theme.bytes(root.dataRate) + "/s is more than 1 GbE carries: a higher decimation or RLE helps." : "")
            }
        }

        // ---- the output
        Card {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: 10
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                Label2 { text: "Output"; kind: "cardTitle"; Layout.fillWidth: true }
                Seg {
                    options: ["This PC", "SD card", "Off"]
                    currentIndex: root.outOff ? 2 : root.outSd ? 1 : 0
                    onPicked: (i) => {
                        root.b.setPcValue("gpioOutOff", i === 2)
                        if (i < 2) root.set("gpio_out_pass_mode", i === 1 ? "FILE" : "NET")
                    }
                }
            }
            GridLayout {
                visible: !root.outOff
                Layout.fillWidth: true
                columns: width > 440 ? 2 : 1
                columnSpacing: 16
                rowSpacing: 12
                // a file of this computer: kept here
                Field {
                    visible: !root.outSd
                    label: "File"
                    hint: root.fileText
                    hintBad: !!root.file.path && !root.file.reading && !root.file.ok
                    Layout.fillWidth: true
                    Layout.columnSpan: parent.columns
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        TextIn {
                            objectName: "field:gpioOutFile"
                            Layout.fillWidth: true
                            mono: true
                            placeholderText: "a BIN file of this computer"
                            value: String(root.pv("gpioOutFile"))
                            onCommitted: (t) => b.setPcValue("gpioOutFile", t.trim())
                        }
                        Btn { kind: "outline"; text: "Browse…"; onClicked: fileDlg.open() }
                    }
                }
                // a file of the SD card of the board
                Field {
                    visible: root.outSd
                    label: "File on the SD card"
                    hint: root.msg("gpio_out_file_sd") || "A name in /home/redpitaya/streaming_files/gpio (upload files on the web page of the board)"
                    hintBad: root.msgBad("gpio_out_file_sd")
                    Layout.fillWidth: true
                    Layout.columnSpan: parent.columns
                    TextIn {
                        objectName: "field:gpio_out_file_sd"
                        Layout.fillWidth: true
                        mono: true
                        placeholderText: "pattern.bin"
                        value: root.v("gpio_out_file_sd")
                        onCommitted: (t) => root.set("gpio_out_file_sd", t.trim())
                    }
                }
                Field {
                    label: "Repeat"
                    Layout.fillWidth: true
                    Layout.preferredWidth: 10
                    Select {
                        Layout.fillWidth: true
                        options: root.outSd ? [["GPIO_REP_OFF", "once"], ["GPIO_REP_ON", "N times"], ["GPIO_REP_INF", "until the stop"]]
                                            : [["once", "once"], ["times", "N times"], ["inf", "until the stop"]]
                        value: root.outSd ? (root.v("gpio_out_repeat") || "GPIO_REP_OFF") : String(root.pv("gpioOutRepeat"))
                        onPicked: (x) => root.outSd ? root.set("gpio_out_repeat", x) : b.setPcValue("gpioOutRepeat", x)
                    }
                }
                Field {
                    id: timesField
                    readonly property bool on: root.outSd ? root.v("gpio_out_repeat") === "GPIO_REP_ON" : root.pv("gpioOutRepeat") === "times"
                    label: "Times"
                    hint: root.msg("gpio_out_repeat_count")
                    hintBad: root.msgBad("gpio_out_repeat_count")
                    Layout.fillWidth: true
                    Layout.preferredWidth: 10
                    opacity: on ? 1 : 0.45
                    TextIn {
                        objectName: "field:gpioOutTimes"
                        Layout.fillWidth: true
                        mono: true
                        enabled: timesField.on
                        value: root.outSd ? root.v("gpio_out_repeat_count") : String(root.pv("gpioOutTimes"))
                        validator: IntValidator { bottom: 1; top: 2000000000 }
                        onCommitted: (t) => root.outSd ? root.set("gpio_out_repeat_count", Math.max(1, Number(t) || 1)) : b.setPcValue("gpioOutTimes", Math.max(1, Number(t) || 1))
                    }
                }
                Flip {
                    label: "Hold the lines through the gaps"
                    Layout.fillWidth: true
                    Layout.columnSpan: parent.columns
                    checked: root.outSd ? root.v("gpio_out_play_gaps") === "ON" : String(root.pv("gpioOutGaps")) === "true"
                    onClicked: root.outSd ? root.set("gpio_out_play_gaps", checked ? "ON" : "OFF") : b.setPcValue("gpioOutGaps", checked)
                }
            }
            GridLayout {
                Layout.fillWidth: true
                columns: width > 440 ? 2 : 1
                columnSpacing: 16
                rowSpacing: 12
                Field {
                    label: "Step"
                    hint: root.msg("gpio_out_step") || "the read step of the output DMA, 1 - every word. A word plays for a clock (" + Theme.rate(root.clock) + ")" + (root.rle ? ", with RLE its count + 1" : "")
                    hintBad: root.msgBad("gpio_out_step")
                    Layout.fillWidth: true
                    Layout.preferredWidth: 10
                    TextIn {
                        objectName: "field:gpio_out_step"
                        Layout.fillWidth: true
                        mono: true
                        value: root.v("gpio_out_step")
                        validator: IntValidator { bottom: 1; top: 2147483647 }
                        onCommitted: (t) => {
                            const n = Number(t)
                            if (!(n >= 1) || !Number.isInteger(n)) { root.say("gpio_out_step", "1 or more clocks"); return }
                            root.set("gpio_out_step", n)
                        }
                    }
                }
                Field {
                    label: "Output waits for"
                    Layout.fillWidth: true
                    Layout.preferredWidth: 10
                    Select {
                        Layout.fillWidth: true
                        options: [["OFF", "nothing (at the start)"], ["ON", "a trigger"]]
                        value: root.v("gpio_out_trigger_mode") || "OFF"
                        onPicked: (x) => root.set("gpio_out_trigger_mode", x)
                    }
                }
            }
            Label2 {
                Layout.fillWidth: true
                kind: "hint"
                text: (root.outOff ? "Off: Start all of this board leaves the output out, and its chip on the card does not start it. "
                    : root.outSd ? "The board plays its own file. "
                    : "The file is read on this computer and sent to the board (port 18905) while it plays; the file and the repeat are kept on this computer. ")
                    + (root.outOff ? "" : "The gaps: a recording of the input holds the lines through the samples it lost or waited for a trigger, in its own time.")
                    + (root.outRunning ? " Waited for data: " + Theme.count(root.gout.waited || 0) + " clocks" + (root.gout.armed ? " · armed" : "") : "")
            }
        }

        // ---- the lines
        Card {
            title: "Lines"
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: 10
            GridLayout {
                id: lines
                columns: 11
                columnSpacing: 6
                rowSpacing: 8
                Item { Layout.preferredWidth: 1; Layout.preferredHeight: 1 }
                Repeater {
                    model: 8
                    delegate: Text { required property int index; text: index; color: Theme.muted; font.pixelSize: Theme.fsSm; horizontalAlignment: Text.AlignHCenter; Layout.preferredWidth: 20 + Theme.fsAdd }
                }
                Item { Layout.columnSpan: 2; Layout.preferredWidth: 1; Layout.preferredHeight: 1 }
                Repeater {
                    // the label, the setting, its first bit
                    model: [["P driven by output", "out", 0], ["N driven by output", "out", 8], ["P inverted", "pol", 0], ["N inverted", "pol", 8]]
                    delegate: Repeater {
                        id: row
                        required property var modelData
                        readonly property int mask: modelData[1] === "out" ? root.outMask : root.pol
                        function put(m) { if (modelData[1] === "out") root.setOut(m); else root.set("gpio_polarity", m) }
                        model: 11
                        delegate: Item {
                            id: cell
                            required property int index
                            readonly property int bit: row.modelData[2] + index - 1
                            Layout.preferredWidth: index === 0 ? rowLabel.implicitWidth + 8 : index <= 8 ? 20 + Theme.fsAdd : allBtn.implicitWidth
                            Layout.preferredHeight: 24 + Theme.fsAdd
                            Text {
                                id: rowLabel
                                visible: cell.index === 0
                                anchors.verticalCenter: parent.verticalCenter
                                text: row.modelData[0]
                                color: Theme.text
                                font.pixelSize: Theme.fsSm
                            }
                            LineBox {
                                objectName: "line:" + row.modelData[1] + ":" + cell.bit
                                visible: cell.index >= 1 && cell.index <= 8
                                anchors.centerIn: parent
                                on: (row.mask & (1 << cell.bit)) !== 0
                                Accessible.name: "DIO" + (cell.index - 1) + "_" + (row.modelData[2] ? "N" : "P") + (row.modelData[1] === "out" ? " driven by the output" : " inverted")
                                onClicked: row.put(row.mask ^ (1 << cell.bit))
                            }
                            Btn {
                                id: allBtn
                                objectName: "lines:" + row.modelData[1] + ":" + row.modelData[2] + ":" + text
                                visible: cell.index >= 9
                                anchors.verticalCenter: parent.verticalCenter
                                kind: "outline"; compact: true
                                text: cell.index === 9 ? "all" : "none"
                                Accessible.name: text + " of " + row.modelData[0]
                                onClicked: row.put(cell.index === 9 ? row.mask | (0xFF << row.modelData[2]) : row.mask & ~(0xFF << row.modelData[2]))
                            }
                        }
                    }
                }
            }
            Label2 {
                Layout.fillWidth: true
                kind: "hint"
                text: "A line driven by the output is an output; the others read in. Output " + root.hex4(root.outMask) + " · inverted " + root.hex4(root.pol) + " (bits 7:0 P, 15:8 N)."
            }
        }

        // ---- the sampling and the trigger
        Card {
            title: "Sampling and trigger"
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
                    hint: root.msg("gpio_decimation") || Theme.rate(root.per) + " per line" + (root.rle ? ", RLE" : "")
                    hintBad: root.msgBad("gpio_decimation")
                    Layout.fillWidth: true
                    Layout.preferredWidth: 10
                    TextIn {
                        objectName: "field:gpio_decimation"
                        Layout.fillWidth: true
                        mono: true
                        bad: root.msgBad("gpio_decimation")
                        value: String(root.dec)
                        validator: RegularExpressionValidator { regularExpression: /-?[0-9]{0,9}/ }
                        onCommitted: (t) => {
                            const d = Number(t)
                            if (t === "" || !Number.isInteger(d) || d < 1 || d > 131072) { root.say("gpio_decimation", "From 1 to 131072: not sent, the board keeps " + root.dec); return }
                            root.set("gpio_decimation", d)
                        }
                    }
                }
                Field {
                    label: " "
                    Layout.fillWidth: true
                    Layout.preferredWidth: 10
                    Flip {
                        label: "Run-length packing"
                        Layout.fillWidth: true
                        checked: root.rle
                        onClicked: root.set("gpio_rle", checked ? "ON" : "OFF")
                    }
                }
                Field {
                    label: "Input waits for"
                    Layout.fillWidth: true
                    Layout.preferredWidth: 10
                    Select {
                        Layout.fillWidth: true
                        options: [["OFF", "nothing (at the start)"], ["ON", "a trigger"]]
                        value: root.v("gpio_in_trigger_mode") || "OFF"
                        onPicked: (x) => root.set("gpio_in_trigger_mode", x)
                    }
                }
                Field {
                    label: "Start and stop together with"
                    Layout.fillWidth: true
                    Layout.preferredWidth: 10
                    Select {
                        Layout.fillWidth: true
                        options: [["OWN", "itself"], ["OSC1", "ADC"]].concat(root.caps.dac ? [["GEN_A", "DAC"]] : [])
                        value: root.v("gpio_event_source") || "OWN"
                        onPicked: (x) => root.set("gpio_event_source", x)
                    }
                }
            }
            // the pattern trigger: a line x (any), 0, 1, rising, falling
            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                Label2 { text: "Pattern trigger"; kind: "label" }
                Btn { objectName: "pat:clear"; kind: "outline"; compact: true; text: "clear"; enabled: root.pm || root.ppos || root.pneg; onClicked: root.setPattern(Array(16).fill(0)) }
                Label2 {
                    Layout.fillWidth: true
                    kind: "hint"
                    text: root.pm || root.ppos || root.pneg ? "mask " + root.hex4(root.pm) + " value " + root.hex4(root.pval) + " rise " + root.hex4(root.ppos) + " fall " + root.hex4(root.pneg)
                                                           : "no line: the pattern never fires"
                }
            }
            GridLayout {
                columns: 9
                columnSpacing: 8
                rowSpacing: 6
                Repeater {
                    model: 18
                    delegate: Item {
                        id: pc
                        required property int index
                        readonly property int row: Math.floor(index / 9)
                        readonly property int k: row * 8 + index % 9 - 1
                        readonly property int p: index % 9 ? root.patOf(k) : 0
                        implicitWidth: index % 9 ? 26 + Theme.fsAdd : 20
                        implicitHeight: 24 + Theme.fsAdd
                        Text { visible: pc.index % 9 === 0; anchors.verticalCenter: parent.verticalCenter; text: pc.row ? "N" : "P"; color: Theme.muted; font.pixelSize: Theme.fsSm }
                        AbstractButton {
                            id: patBtn
                            objectName: "pat:" + pc.k
                            visible: pc.index % 9 !== 0
                            anchors.fill: parent
                            hoverEnabled: true
                            focusPolicy: Qt.StrongFocus
                            readonly property string what: ["any", "low", "high", "rising edge", "falling edge"][pc.p]
                            Accessible.name: "DIO" + (pc.index % 9 - 1) + "_" + (pc.row ? "N" : "P") + ": " + what
                            ToolTip.visible: hovered
                            ToolTip.delay: 500
                            ToolTip.text: Accessible.name
                            onClicked: root.cyclePattern(pc.k)
                            background: Rectangle {
                                radius: Theme.radius
                                color: pc.p === 0 ? "transparent" : pc.p <= 2 ? Qt.rgba(71 / 255, 178 / 255, 209 / 255, 0.25) : Theme.amberBg
                                border.color: patBtn.visualFocus ? Theme.focus : pc.p === 0 ? Theme.inputBorder : pc.p <= 2 ? Theme.accent : Theme.amber
                                border.width: patBtn.visualFocus ? 2 : 1
                            }
                            contentItem: Item {
                                Text {
                                    visible: pc.p <= 2
                                    anchors.centerIn: parent
                                    text: ["x", "0", "1"][Math.min(pc.p, 2)]
                                    color: pc.p ? Theme.value : Theme.muted
                                    font.family: Theme.mono
                                    font.pixelSize: Theme.fsSm
                                }
                                // an edge: low to high or high to low
                                Canvas {
                                    visible: pc.p >= 3
                                    anchors.centerIn: parent
                                    width: 16; height: 12
                                    readonly property bool up: pc.p === 3
                                    onUpChanged: requestPaint()
                                    onVisibleChanged: requestPaint()
                                    onPaint: {
                                        const c = getContext("2d")
                                        c.reset()
                                        c.strokeStyle = Theme.amberText
                                        c.lineWidth = 1.4
                                        c.beginPath()
                                        c.moveTo(1, up ? 11 : 1); c.lineTo(8, up ? 11 : 1); c.lineTo(8, up ? 1 : 11); c.lineTo(15, up ? 1 : 11)
                                        c.stroke()
                                    }
                                }
                            }
                        }
                    }
                }
            }
            GridLayout {
                Layout.fillWidth: true
                columns: width > 440 ? 2 : 1
                columnSpacing: 16
                rowSpacing: 12
                Field {
                    label: "Buffers per trigger"
                    hint: root.msg("gpio_trigger_blocks") || "0 - everything after it"
                    hintBad: root.msgBad("gpio_trigger_blocks")
                    Layout.fillWidth: true
                    Layout.preferredWidth: 10
                    TextIn {
                        objectName: "field:gpio_trigger_blocks"
                        Layout.fillWidth: true
                        mono: true
                        value: root.v("gpio_trigger_blocks")
                        validator: IntValidator { bottom: 0; top: 65535 }
                        onCommitted: (t) => root.set("gpio_trigger_blocks", Math.max(0, Math.min(65535, Number(t) || 0)))
                    }
                }
                Item { Layout.fillWidth: true; Layout.preferredWidth: 10; visible: parent.columns === 2 }
            }
            Label2 {
                Layout.fillWidth: true
                kind: root.v("gpio_in_trigger_mode") === "ON" && !(Number(root.v("gpio_trigger_sources")) > 0) ? "warn" : "hint"
                text: "Click a line: x (any) → 0 → 1 → rising → falling. The pattern fires every stream with GPIO among its sources; the sources of the input are set in the Triggers tab."
                      + (root.inRunning && root.v("gpio_in_trigger_mode") === "ON" ? " Armed: " + (root.gin.armed ? "yes" : "no") + " · triggers seen: " + Theme.count(root.gin.triggers || 0) : "")
            }
        }
    }

    FileDialog {
        id: fileDlg
        fileMode: FileDialog.OpenFile
        nameFilters: ["GPIO files (*.bin)", "All files (*)"]
        currentFolder: String(root.pv("gpioOutFile")) !== "" ? app.pathToUrl(String(root.pv("gpioOutFile")).replace(/[\\/][^\\/]*$/, "")) : ""
        onAccepted: b.setPcValue("gpioOutFile", app.urlToPath(selectedFile))
    }

    // a line of the matrix: on (filled) or off
    component LineBox: AbstractButton {
        id: box
        property bool on: false
        implicitWidth: 20 + Theme.fsAdd
        implicitHeight: 20 + Theme.fsAdd
        hoverEnabled: true
        focusPolicy: Qt.StrongFocus
        Accessible.role: Accessible.CheckBox
        Accessible.checkable: true
        Accessible.checked: on
        background: Rectangle {
            radius: Theme.radius
            color: box.on ? Theme.accent : box.hovered ? Theme.hover : "transparent"
            border.color: box.visualFocus ? Theme.focus : box.on ? Theme.accent : Theme.inputBorder
            border.width: box.visualFocus ? 2 : 1
        }
    }
}
