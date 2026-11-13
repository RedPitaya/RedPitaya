pragma Singleton
import QtQuick

// The look of the web page of the board (streaming_manager css/style.css).
QtObject {
    // pixels added to every text size: Application settings, Text size
    property int fsAdd: 0

    readonly property int fsXs: 11 + fsAdd
    readonly property int fsSm: 12 + fsAdd
    readonly property int fsMd: 13 + fsAdd
    readonly property int fsBase: 14 + fsAdd
    readonly property int fsLg: 16 + fsAdd
    readonly property int fsXl: 20 + fsAdd
    readonly property int fsTitle: 24 + fsAdd

    readonly property int ctrlH: 32 + 2 * fsAdd
    readonly property int radius: 3

    readonly property color bg: "#343433"
    readonly property color card: "#2b2b2a"
    readonly property color plot: "#232322"
    readonly property color border: Qt.rgba(153 / 255, 153 / 255, 153 / 255, 0.45)
    readonly property color borderSoft: Qt.rgba(153 / 255, 153 / 255, 153 / 255, 0.25)
    readonly property color borderStrong: "#999999"
    readonly property color inputBorder: "#8a8a88"
    readonly property color inputBg: Qt.rgba(0, 0, 0, 0.15)
    readonly property color hover: "#444444"

    readonly property color text: "#cdcccc"
    readonly property color title: "#e2e1e1"
    readonly property color value: "#e8e8e6"
    readonly property color muted: "#a8a8a6"
    readonly property color dim: "#8a8a88"

    readonly property color accent: "#47b2d1"
    readonly property color accentHover: "#69d4d1"
    readonly property color danger: "#e0675d"
    readonly property color dangerText: "#ff8f86"
    readonly property color ok: "#3ba845"
    readonly property color okText: "#8fdc97"
    readonly property color okBg: Qt.rgba(59 / 255, 168 / 255, 69 / 255, 0.18)
    readonly property color amber: "#e8b84a"
    readonly property color amberText: "#f0cd7a"
    readonly property color amberBg: Qt.rgba(232 / 255, 184 / 255, 74 / 255, 0.16)
    readonly property color redDot: "#d9534f"
    readonly property color redBg: Qt.rgba(224 / 255, 103 / 255, 93 / 255, 0.16)
    readonly property color focus: "#ffcc33"

    // the inputs IN1..IN4 in the preview
    readonly property var channelColors: ["#f3ec1a", "#31b44b", "#d08a45", "#d05bcc"]
    // the streams in the memory bar
    readonly property var streamColors: ({ adc: "#47b2d1", dac: "#e8b84a", gin: "#3ba845", gout: "#b47ad1" })

    readonly property string mono: "Menlo, Consolas, Liberation Mono, DejaVu Sans Mono, monospace"

    function bytes(b) {
        if (b >= 1073741824) return (b / 1073741824).toFixed(2) + " GB"
        if (b >= 1048576) return (b / 1048576).toFixed(2) + " MB"
        if (b >= 1024) return (+(b / 1024).toFixed(1)) + " kB"
        return Math.round(b) + " B"
    }
    function rate(r) {
        if (r >= 1e6) return (+(r / 1e6).toFixed(3)) + " MS/s"
        if (r >= 1e3) return (+(r / 1e3).toFixed(2)) + " kS/s"
        return (+r.toFixed(1)) + " S/s"
    }
    function count(n) {
        if (n >= 1e9) return (n / 1e9).toFixed(2) + " G"
        if (n >= 1e6) return (n / 1e6).toFixed(2) + " M"
        if (n >= 1e4) return (n / 1e3).toFixed(1) + " k"
        return String(Math.round(n))
    }
}
