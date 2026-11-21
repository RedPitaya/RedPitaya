.pragma library

// The limits and the parts of a recording (streaming_improvements 4.11): the times and the sizes
// as the fields show and take them. Used by the ADC and the GPIO tabs.

function fmtTime(t) { return t >= 3600 && t % 3600 === 0 ? t / 3600 + " h" : t >= 60 && t % 60 === 0 ? t / 60 + " min" : (+t.toFixed(3)) + " s" }

function parseTime(text) {
    const m = /^\s*([0-9]*\.?[0-9]+)\s*(ms|s|sec|m|min|h)?\s*$/i.exec(String(text))
    if (!m) return NaN
    const u = (m[2] || "s").toLowerCase()
    return Number(m[1]) * (u === "ms" ? 0.001 : u === "m" || u === "min" ? 60 : u === "h" ? 3600 : 1)
}

// bytes: the Theme.bytes text of a size that is not a whole MB or GB
function fmtSize(b, bytes) { return b >= 1073741824 && b % 1073741824 === 0 ? b / 1073741824 + " GB" : b >= 1048576 && b % 1048576 === 0 ? b / 1048576 + " MB" : bytes }

function parseSize(text) {
    const m = /^\s*([0-9]*\.?[0-9]+)\s*(b|k|kb|m|mb|g|gb)?\s*$/i.exec(String(text))
    if (!m) return NaN
    const u = (m[2] || "b").toLowerCase()[0]
    return Math.round(Number(m[1]) * (u === "k" ? 1024 : u === "m" ? 1048576 : u === "g" ? 1073741824 : 1))
}

function dur(sec) {
    if (!(sec >= 0) || !isFinite(sec)) return "-"
    if (sec < 90) return Math.round(sec) + " s"
    if (sec < 5400) return Math.round(sec / 60) + " min"
    return Math.floor(sec / 3600) + " h " + Math.round(sec % 3600 / 60) + " min"
}
