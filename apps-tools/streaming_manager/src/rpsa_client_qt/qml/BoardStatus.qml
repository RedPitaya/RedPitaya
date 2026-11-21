import QtQuick
import "ui"

// What the dashboard says of a board, from its state and its settings. Not drawn.
QtObject {
    id: root
    property var b

    readonly property int rev: b ? b.settingsRev : 0
    function v(key) { return rev >= 0 && b ? b.value(key) : "" }

    readonly property bool online: b ? b.isOnline : false
    readonly property bool connected: b ? b.isConfigConnected : false
    readonly property string link: b ? b.link : "connecting"
    readonly property string st: b ? b.state : "stopped"
    readonly property bool running: st !== "stopped"
    // the last error of the streams (streaming_improvements 2.2): text, time, count
    readonly property var err: b ? b.error : ({})
    readonly property bool hasError: !!err.text

    readonly property string stateText: !online && link !== "refused" ? "Offline"
        : link === "taken" ? "Taken by another client"
        : link === "refused" ? "No streaming app"
        : link === "unreachable" ? "No answer"
        : !connected ? "Connecting"
        : st === "starting" ? "Starting"
        : st === "streaming" ? "Streaming"
        : st === "sd" ? "Writing to SD"
        : st === "armed" ? "Waiting for trigger"
        : st === "stopping" ? "Stopping"
        : dacSt === "playing" ? "DAC playing"
        : ginSt === "recording" || ginSt === "sd" || goutSt === "playing" ? "GPIO running"
        : hasError ? "Error"
        : "Stopped"
    readonly property string stateKind: !online || link === "refused" || link === "unreachable" ? "red"
        : link === "taken" ? "amber"
        : !connected ? "grey"
        : st === "streaming" || st === "sd" ? "ok"
        : st === "starting" || st === "armed" ? "amber"
        : dacSt === "playing" ? "ok"
        : ginSt === "recording" || ginSt === "sd" || goutSt === "playing" ? "ok"
        : hasError && !running ? "red"
        : "grey"

    // what the preview says when there is nothing to show
    readonly property string linkText: link === "taken" ? "Another client took the board (it serves one at a time): Reconnect takes it back"
        : link === "refused" ? "The streaming application is not running on the board: start it in its web interface"
        : link === "unreachable" ? "No answer from the board: off, booting or not reachable"
        : !connected ? "Connecting to the board" : ""

    readonly property var caps: b ? b.caps : ({})
    readonly property int adcCount: caps.adcChannels || 2

    readonly property var inputsOn: {
        const r = []
        for (let i = 1; i <= adcCount; i++)
            if (v("channel_state_" + i) === "ON") r.push("IN" + i)
        return r
    }
    // the preview axes (streaming_improvements 4.1): volts of the full range of IN1..IN4 and the
    // inputs on. 1:1 1 V, 1:20 20 V; a board of one range its own (122-16: 0.25 V)
    readonly property var fullScales: {
        const r = []
        const fixed = parseFloat(String(caps.fixedRange || "").replace("±", "")) || 1
        for (let i = 1; i <= 4; i++)
            r.push(caps.adcRanges ? (v("channel_attenuator_" + i) === "A_1_20" ? 20 : 1) : fixed)
        return r
    }
    readonly property var inputsMask: [1, 2, 3, 4].map(i => v("channel_state_" + i) === "ON")
    readonly property int decimation: Math.max(1, Number(v("adc_decimation")) || 1)
    readonly property double perChannel: (caps.adcRate || 125000000) / decimation
    readonly property int sampleBytes: v("resolution") === "BIT_8" ? 1 : 2
    readonly property int blockSamples: Math.floor((Number(v("block_size")) || 65536) / sampleBytes)
    readonly property bool toSd: v("adc_pass_mode") === "FILE"
    readonly property string format: (v("format_sd") || "WAV")

    readonly property string adcText: !b || !b.settingsLoaded ? "settings not read yet"
        : (inputsOn.length ? inputsOn.join(" ") : "no input on") + " · " + sampleBytes * 8 + " bit · dec " + decimation
          + " · " + Theme.rate(perChannel) + " → " + (toSd ? format + " on the SD card" : format + " on this PC")
    readonly property bool trigOn: v("adc_trigger_mode") === "ON"
    readonly property int trigSources: Number(v("adc_trigger_sources")) || 0
    // waiting for a trigger with no source: only the software trigger starts the ADC
    readonly property bool trigNoSource: trigOn && trigSources === 0
    readonly property string trigText: !b || !b.settingsLoaded ? "-" : trigNoSource ? "Waits, no source chosen" : trigOn ? "Waits for a trigger" : "Off (continuous)"

    // the DAC (streaming_improvements 4.17): CBoard::dacState
    readonly property bool hasDac: !!caps.dac
    readonly property var dac: b ? b.dacState : ({})
    readonly property string dacSt: dac.state || "stopped"
    readonly property bool dacRunning: dacSt === "connecting" || dacSt === "playing" || dacSt === "stopping"
    readonly property bool dacSd: v("dac_pass_mode") === "DAC_FILE"
    readonly property string dacPcFile: b && b.pcRev >= 0 ? String(b.pcValue("dacFile") || "") : ""
    readonly property string dacPcRepeat: b && b.pcRev >= 0 ? String(b.pcValue("dacRepeat") || "once") : "once"
    // a source to play is chosen: a file of the SD card or of this computer
    readonly property bool dacReady: dacSd ? v("file_sd") !== "" : dacPcFile !== ""
    readonly property string dacOuts: (caps.dacChannels || 2) > 1 ? "OUT1 OUT2" : "OUT1"
    readonly property string dacRepeatText: dacSd
        ? (v("repeat") === "DAC_REP_INF" ? "until the stop" : v("repeat") === "DAC_REP_ON" ? v("repeatCount") + " times" : "once")
        : (dacPcRepeat === "inf" ? "until the stop" : dacPcRepeat === "times" ? (b && b.pcRev >= 0 ? b.pcValue("dacTimes") : 1) + " times" : "once")
    readonly property string dacKind: dacSt === "playing" ? "ok" : dacSt === "connecting" || dacSt === "stopping" ? "amber" : dacSt === "error" || dacSt === "board" ? "red" : "grey"
    // what it does now
    readonly property string dacLine: dacSt === "connecting" ? "Starting · " + (dac.file || "")
        : dacSt === "stopping" ? "Stopping"
        : dacSt === "playing" ? (dac.source === "sd" ? "Playing from SD: " + dac.file
            : "Playing " + dac.file + (dac.onePack ? " · the board repeats it" : dac.passes === 1 ? "" : dac.passes > 0 ? " · pass " + dac.pass + " of " + dac.passes : " · pass " + dac.pass))
        : dacSt === "board" ? "Stopped by the board" + (dac.text ? ": " + dac.text : "")
        : dacSt === "error" ? "Error: " + (dac.text || "")
        : "Stopped" + (dac.text ? " · " + dac.text : "")
    // where from
    readonly property string dacFrom: dacRunning && dac.source === "pc" ? dacOuts + " · from this PC · " + Theme.bytes(dac.rate || 0) + "/s"
        : dacRunning ? dacOuts + " · from the SD card · " + dacRepeatText
        : !dacReady ? (dacSd ? "No file of the SD card chosen (Settings, DAC)" : "No file chosen (Settings, DAC)")
        : (dacSd ? "from SD: " + v("file_sd") : dacPcFile.replace(/^.*[\\/]/, "") + " from this PC") + " · " + dacRepeatText

    // the GPIO (streaming_improvements 4.17): CBoard::gpioState; Off of a direction is kept on
    // this computer (Start all of the board and its chip leave it out)
    readonly property bool hasGpio: !!caps.gpio
    readonly property var gs: b ? b.gpioState : ({})
    readonly property var gin: gs.in || ({})
    readonly property var gout: gs.out || ({})
    readonly property string ginSt: gin.state || "stopped"
    readonly property string goutSt: gout.state || "stopped"
    readonly property bool ginRunning: ["connecting", "recording", "sd", "stopping"].indexOf(ginSt) >= 0
    readonly property bool goutRunning: ["connecting", "playing", "stopping"].indexOf(goutSt) >= 0
    readonly property bool ginOff: b && b.pcRev >= 0 ? String(b.pcValue("gpioInOff")) === "true" : false
    readonly property bool goutOff: b && b.pcRev >= 0 ? String(b.pcValue("gpioOutOff")) === "true" : false
    readonly property bool ginSd: v("gpio_in_pass_mode") === "FILE"
    readonly property bool goutSd: v("gpio_out_pass_mode") === "FILE"
    readonly property string goutPcFile: b && b.pcRev >= 0 ? String(b.pcValue("gpioOutFile") || "") : ""
    readonly property bool goutReady: goutSd ? v("gpio_out_file_sd") !== "" : goutPcFile !== ""
    readonly property double gpioRate: (caps.adcRate === 62500000 ? 125000000 : (caps.adcRate || 125000000)) / Math.max(1, Number(v("gpio_decimation")) || 1)
    function gpioKind(st) { return st === "recording" || st === "sd" || st === "playing" ? "ok" : st === "connecting" || st === "stopping" ? "amber" : st === "error" || st === "board" ? "red" : "grey" }
    function gpioDur(sec) { sec = Math.round(sec || 0); return sec < 90 ? sec + " s" : sec < 5400 ? Math.round(sec / 60) + " min" : (sec / 3600).toFixed(1) + " h" }
    readonly property string ginKind: ginRunning && gin.armed ? "amber" : gpioKind(ginSt)
    readonly property string goutKind: goutRunning && gout.armed ? "amber" : gpioKind(goutSt)
    readonly property string ginLimitText: !gin.limitKind || gin.limitKind === "none" ? "" : " · " + Math.round((gin.frac || 0) * 100) + "% of the limit"
    readonly property string ginLine: ginSt === "connecting" ? "Starting"
        : ginSt === "stopping" ? "Stopping"
        : ginRunning && gin.armed ? "Waiting for a trigger"
        : ginSt === "recording" ? (gin.packs > 0 ? "Recording on this PC · " + Theme.count(gin.samples || 0) + " samples" + ginLimitText : "Waiting for data")
        : ginSt === "sd" ? "Writing to the SD card" + (gin.known ? " · " + Theme.count(gin.samples || 0) + " samples" + ginLimitText : "")
        : ginSt === "board" ? "Stopped by the board" + (gin.text ? ": " + gin.text : "")
        : ginSt === "error" ? "Error: " + (gin.text || "")
        : ginOff ? "Off (Settings, GPIO)"
        : "Stopped" + (gin.text ? " · " + gin.text : "")
    readonly property string ginFormat: ginSd ? (v("gpio_in_format_sd") || "BIN") : (b && b.pcRev >= 0 ? String(b.pcValue("gpioInFormat") || "BIN") : "BIN")
    readonly property string ginLimitSet: {
        if (ginSd) {
            const t = Number(v("gpio_in_limit_time_sd")), n = Number(v("gpio_in_samples_limit_sd")), z = Number(v("gpio_in_limit_size_sd"))
            return t > 0 ? "stop after " + gpioDur(t) : z > 0 ? "stop after " + Theme.bytes(z) : n > 0 ? "stop after " + Theme.count(n) + " samples" : "until the stop"
        }
        const k = b && b.pcRev >= 0 ? String(b.pcValue("gpioInLimitKind")) : "none"
        const x = b && b.pcRev >= 0 ? Number(b.pcValue("gpioInLimit")) : 0
        return !(x > 0) || k === "none" ? "until the stop" : k === "time" ? "stop after " + gpioDur(x) : "stop after " + Theme.count(x) + " samples"
    }
    readonly property string ginSub: ginRunning && ginSt !== "connecting" ? Theme.bytes(gin.bytes || 0) + (gin.lost > 0 ? " · lost " + Theme.count(gin.lost) : "") + " · " + (gin.file || "")
        : (ginSd ? "→ SD card · " : "→ this PC · ") + ginFormat + " · " + Theme.rate(gpioRate) + " · " + ginLimitSet
    readonly property string goutRepeatText: goutSd
        ? (v("gpio_out_repeat") === "GPIO_REP_INF" ? "until the stop" : v("gpio_out_repeat") === "GPIO_REP_ON" ? v("gpio_out_repeat_count") + " times" : "once")
        : (b && b.pcRev >= 0 ? (b.pcValue("gpioOutRepeat") === "inf" ? "until the stop" : b.pcValue("gpioOutRepeat") === "times" ? b.pcValue("gpioOutTimes") + " times" : "once") : "once")
    readonly property string goutLine: goutSt === "connecting" ? "Starting · " + (gout.file || "")
        : goutSt === "stopping" ? "Stopping"
        : goutRunning && gout.armed ? "Waiting for a trigger"
        : goutSt === "playing" ? (gout.source === "sd" ? "Playing from SD: " + gout.file : gout.whole ? "The whole file is sent, the board plays the rest" : "Playing " + gout.file)
        : goutSt === "board" ? "Stopped by the board" + (gout.text ? ": " + gout.text : "")
        : goutSt === "error" ? "Error: " + (gout.text || "")
        : goutOff ? "Off (Settings, GPIO)"
        : "Stopped" + (gout.text ? " · " + gout.text : "")
    readonly property string goutSub: goutRunning && gout.source === "pc" ? Theme.count(gout.words || 0) + " words · " + Theme.bytes(gout.rate || 0) + "/s · " + goutRepeatText
        : goutRunning ? "from the SD card · " + goutRepeatText
        : !goutReady ? (goutSd ? "No file of the SD card chosen (Settings, GPIO)" : "No file chosen (Settings, GPIO)")
        : (goutSd ? "from SD: " + v("gpio_out_file_sd") : goutPcFile.replace(/^.*[\\/]/, "") + " from this PC") + " · " + goutRepeatText
    // a word or two of each direction for the list
    function gpioShort(st, d, off) {
        return st === "recording" ? (d.armed ? "armed" : "recording") : st === "sd" ? (d.armed ? "armed" : "to SD") : st === "playing" ? (d.armed ? "armed" : "playing")
            : st === "connecting" ? "starting" : st === "stopping" ? "stopping" : st === "error" ? "error" : st === "board" ? "stopped" : off ? "off" : "stopped"
    }
    readonly property string gpioShortText: "GPIO in " + gpioShort(ginSt, gin, ginOff) + " · out " + gpioShort(goutSt, gout, goutOff)
}
