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
        : "Stopped"
    readonly property string stateKind: !online || link === "refused" || link === "unreachable" ? "red"
        : link === "taken" ? "amber"
        : !connected ? "grey"
        : st === "streaming" || st === "sd" ? "ok"
        : st === "starting" || st === "armed" ? "amber"
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
}
