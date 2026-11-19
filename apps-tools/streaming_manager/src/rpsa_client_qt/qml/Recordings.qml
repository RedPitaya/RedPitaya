import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "ui"

// The recordings on this computer (streaming_improvements 4.7, the manager): the list of the
// folders of the application, of the boards and the ones added; what a recording holds; convert,
// delete, show. The data: recordings (RecordingsManager).
Item {
    id: root

    property string search: ""
    property string boardFilter: ""
    property string formatFilter: ""
    // the recordings ticked, path -> true; the one shown on the right
    property var sel: ({})
    property string current: ""
    property var info: ({})
    property string message: ""
    property bool messageBad: false
    function say(text, bad) { message = text; messageBad = bad }

    Component.onCompleted: recordings.setLive(visible)
    onVisibleChanged: recordings.setLive(visible)
    onCurrentChanged: { info = ({}); if (current !== "") recordings.details(current) }
    Connections {
        target: recordings
        function onDetailsReady(path, d) { if (path === root.current) root.info = d }
        function onConverted(path, ok, output, error) { root.say(ok ? "Converted: " + output : "Not converted: " + error, !ok) }
        function onListChanged() {
            // a recording that went (deleted, moved) is not shown or ticked any more
            const there = {}
            for (const r of recordings.list) there[r.path] = true
            const s = {}
            for (const p in root.sel) if (there[p]) s[p] = true
            root.sel = s
            if (root.current !== "" && !there[root.current]) root.current = ""
            else if (root.current !== "") recordings.details(root.current)
        }
    }

    function dur(sec) {
        if (!(sec >= 0) || !isFinite(sec)) return "-"
        if (sec < 60) return (+sec.toFixed(1)) + " s"
        if (sec < 3600) return Math.floor(sec / 60) + " min " + String(Math.round(sec % 60)).padStart(2, "0") + " s"
        return Math.floor(sec / 3600) + " h " + String(Math.floor(sec % 3600 / 60)).padStart(2, "0") + " min"
    }
    function when(ms) { return Qt.formatDateTime(new Date(ms), "yyyy-MM-dd hh:mm") }
    readonly property var all: recordings.list
    readonly property var boardNames: [...new Set(all.map(r => r.board).filter(b => b))].sort()
    readonly property var shown: all.filter(r => (boardFilter === "" || r.board === boardFilter) && (formatFilter === "" || r.format === formatFilter)
        && (search === "" || (r.name + " " + r.board).toLowerCase().indexOf(search.toLowerCase()) >= 0))
        .sort((a, b) => (b.recording - a.recording) || b.started - a.started || a.name.localeCompare(b.name))
    // the rows: a head for a run of Start all (one time on several boards), then its recordings
    readonly property var rows: {
        const runs = {}
        for (const r of shown) if (r.run) (runs[r.run] = runs[r.run] || []).push(r)
        const out = []
        const done = {}
        for (const r of shown) {
            const group = r.run ? runs[r.run] : null
            const many = group && new Set(group.map(x => x.board)).size > 1
            if (many) {
                if (done[r.run]) continue
                done[r.run] = true
                out.push({ head: true, text: "Start all " + r.run.replace("_", " ").replace(/-(\d\d)-(\d\d)$/, ":$1:$2") + " · " + new Set(group.map(x => x.board)).size + " boards · "
                    + Theme.bytes(group.reduce((a, x) => a + x.size, 0)) })
                for (const x of group) out.push({ head: false, r: x })
            } else {
                out.push({ head: false, r: r })
            }
        }
        return out
    }
    readonly property var selected: all.filter(r => sel[r.path])
    // the newest recording is shown when none is
    onRowsChanged: if (current === "") { const f = rows.find(x => !x.head); if (f) current = f.r.path }
    // for the UI script (tests, screenshots)
    function scriptOpen(what) {
        if (what === "menu") fileMenu.popupFor([current], fileBtn)
        else if (what === "convert") convertDialog.ask([current])
        else if (what === "delete") deleteDialog.ask(targets())
        else if (what === "folders") foldersDialog.open()
        else if (what === "tick") toggle(current)
        else if (what === "convertgo") { const e = recordings.convert([current], { format: "CSV", inputs: [1], index: true }); say(e, e !== "") }
        else if (what === "deletego") { const e = recordings.remove([current]); say(e, e !== "") }
        else if (what === "next") { const f = rows.filter(x => !x.head); const i = f.findIndex(x => x.r.path === current); if (i + 1 < f.length) current = f[i + 1].r.path }
    }
    function toggle(path) { const s = Object.assign({}, sel); if (s[path]) delete s[path]; else s[path] = true; sel = s }
    // what an action is for: the ticked recordings, else the one shown
    function targets() { const t = selected.map(r => r.path); return t.length ? t : current !== "" ? [current] : [] }
    readonly property var cur: all.find(r => r.path === current) || null
    readonly property var job: recordings.job

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        anchors.topMargin: 16
        spacing: 12

        // the head: what there is, the filters
        RowLayout {
            Layout.fillWidth: true
            spacing: 14
            ColumnLayout {
                spacing: 4
                Layout.fillWidth: true
                Label2 { text: "Recordings"; kind: "title" }
                Label2 {
                    kind: "sub"
                    text: root.all.length + (root.all.length === 1 ? " recording · " : " recordings · ") + Theme.bytes(root.all.reduce((a, r) => a + r.size, 0))
                          + " in " + recordings.folders.length + (recordings.folders.length === 1 ? " folder" : " folders") + " · the disk: " + Theme.bytes(recordings.diskFree) + " free"
                }
            }
            Field {
                Layout.fillWidth: false
                label: "Search"
                TextIn { free: true; placeholderText: "name, board"; implicitWidth: 200; onTextChanged: root.search = text.trim() }
            }
            Field {
                Layout.fillWidth: false
                label: "Board"
                Select { implicitWidth: 160; options: [["", "All boards"]].concat(root.boardNames.map(b => [b, b])); value: root.boardFilter; onPicked: (x) => root.boardFilter = x }
            }
            Field {
                Layout.fillWidth: false
                label: "Format"
                Select { implicitWidth: 110; options: [["", "All"], ["BIN", "BIN"], ["WAV", "WAV"], ["TDMS", "TDMS"], ["CSV", "CSV"], ["VCD", "VCD"]]; value: root.formatFilter; onPicked: (x) => root.formatFilter = x }
            }
            Btn { kind: "outline"; text: "Folders…"; Layout.alignment: Qt.AlignBottom; onClicked: foldersDialog.open() }
            Btn { kind: "outline"; text: "⟳"; implicitWidth: implicitHeight; Accessible.name: "Read the folders again"; Layout.alignment: Qt.AlignBottom; onClicked: recordings.rescan() }
        }

        Label2 { visible: root.message !== ""; text: root.message; kind: root.messageBad ? "bad" : "hint"; Layout.fillWidth: true }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            // the list
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: Theme.card
                border.color: Theme.border
                radius: Theme.radius
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 1
                    spacing: 0
                    Row6 { head: true; Layout.fillWidth: true }
                    Rectangle { Layout.fillWidth: true; height: 1; color: Theme.borderStrong }
                    ListView {
                        id: listView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: root.rows
                        ScrollBar.vertical: ScrollBar {}
                        delegate: Item {
                            required property var modelData
                            width: listView.width
                            height: modelData.head ? 28 + Theme.fsAdd : 36 + Theme.fsAdd
                            Rectangle {
                                visible: modelData.head
                                anchors.fill: parent
                                color: Qt.rgba(71 / 255, 178 / 255, 209 / 255, 0.08)
                                Text { x: 12; anchors.verticalCenter: parent.verticalCenter; text: modelData.head ? "▾ " + modelData.text : ""; color: Theme.muted; font.pixelSize: Theme.fsSm }
                            }
                            Row6 {
                                visible: !modelData.head
                                anchors.fill: parent
                                r: modelData.head ? null : modelData.r
                            }
                        }
                        Label2 {
                            anchors.centerIn: parent
                            visible: root.rows.length === 0
                            horizontalAlignment: Text.AlignHCenter
                            kind: "hint"
                            text: root.all.length ? "No recording matches the filters"
                                : "No recordings in these folders\nA board streaming to this PC writes here; recordings from rpsa_client: Folders…"
                        }
                    }
                    Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.margins: 8
                        Layout.leftMargin: 12
                        spacing: 10
                        Label2 {
                            kind: "hint"
                            Layout.fillWidth: true
                            text: root.selected.length ? root.selected.length + " selected · " + Theme.bytes(root.selected.reduce((a, r) => a + r.size, 0)) : "Tick recordings to convert or delete several"
                        }
                        Btn { kind: "outline"; compact: true; text: "Convert…"; enabled: root.selected.length > 0 && root.selected.every(r => r.convertible && !r.recording); onClicked: convertDialog.ask(root.targets()) }
                        Btn { kind: "danger"; compact: true; text: "Delete…"; enabled: root.selected.length > 0 && root.selected.every(r => !r.recording); onClicked: deleteDialog.ask(root.targets()) }
                    }
                }
            }

            // the recording shown
            Rectangle {
                Layout.preferredWidth: 420
                Layout.fillHeight: true
                color: Theme.card
                border.color: Theme.border
                radius: Theme.radius
                Label2 { anchors.centerIn: parent; visible: !root.cur; kind: "hint"; text: "Click a recording" }
                ColumnLayout {
                    visible: !!root.cur
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12
                    ColumnLayout {
                        spacing: 3
                        Layout.fillWidth: true
                        Text { text: root.cur ? root.cur.name : ""; color: Theme.title; font.pixelSize: Theme.fsLg; font.bold: true; font.family: Theme.mono; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true }
                        Label2 { text: root.cur ? root.cur.dir : ""; kind: "hint"; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true }
                    }
                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        columnSpacing: 10
                        rowSpacing: 6
                        component K: Label2 { kind: "hint"; Layout.preferredWidth: 110 }
                        component V: Text { color: Theme.value; font.pixelSize: Theme.fsMd; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                        readonly property var d: root.info
                        readonly property var b: root.cur && root.cur.board ? boards.boardByName(root.cur.board) : null
                        K { text: "Board" }
                        V { text: root.cur ? (root.cur.board || "-") + (parent.b ? " · " + parent.b.model : "") : "" }
                        K { text: "Started" }
                        V { text: root.cur ? (parent.d.startNs ? Qt.formatDateTime(new Date(parent.d.startNs / 1e6), "yyyy-MM-dd hh:mm:ss.zzz") : Qt.formatDateTime(new Date(root.cur.started), "yyyy-MM-dd hh:mm:ss")) : ""; font.family: Theme.mono }
                        K { text: "Length" }
                        V { text: parent.d.seconds !== undefined ? root.dur(parent.d.seconds) + " · " + Theme.count(parent.d.samples) + " samples/ch" : root.cur && root.cur.recording ? "recording" : "-"; font.family: Theme.mono }
                        K { text: "Sample rate" }
                        V { text: parent.d.rate ? Theme.rate(parent.d.rate) + (parent.d.decimation ? " · dec " + parent.d.decimation + (parent.d.adcRate ? " of " + (parent.d.adcRate / 1e6) + " MHz" : "") : "") : "-"; font.family: Theme.mono }
                        K { text: "Inputs" }
                        V { text: parent.d.inputs ? parent.d.inputs.map(i => "IN" + i).join(" ") + (parent.d.bits ? " · " + parent.d.bits + " bit" : "") + (parent.d.values ? " · " + parent.d.values : "") : parent.d.gpio ? "GPIO" : "-" }
                        K { text: "Trigger" }
                        V { text: parent.d.trigger ? (parent.d.trigger === "off" ? "off (continuous)" : parent.d.trigger) : "-" }
                        K { text: "Size" }
                        V { text: root.cur ? Theme.bytes(root.cur.size) + (parent.d.blocks ? " · " + parent.d.blocks + " blocks" : "") : ""; font.family: Theme.mono }
                    }
                    // the losses
                    Rectangle {
                        readonly property double lost: (root.info.lostBoard || 0)
                        readonly property double lostPc: (root.info.lostPcBlocks || 0)
                        visible: !!root.cur && (root.info.lostBoard !== undefined || root.info.lostPcBlocks !== undefined)
                        Layout.fillWidth: true
                        implicitHeight: lostCol.implicitHeight + 16
                        radius: Theme.radius
                        color: lost > 0 || lostPc > 0 ? Theme.redBg : Qt.rgba(0, 0, 0, 0.12)
                        ColumnLayout {
                            id: lostCol
                            x: 10; y: 8
                            width: parent.width - 20
                            spacing: 3
                            Text {
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                                font.pixelSize: Theme.fsMd
                                color: parent.parent.lost > 0 || parent.parent.lostPc > 0 ? Theme.dangerText : Theme.okText
                                text: parent.parent.lost > 0 ? "Lost " + Theme.count(parent.parent.lost) + " samples per channel" + (root.info.samples ? " (" + (100 * parent.parent.lost / (root.info.samples + parent.parent.lost)).toFixed(2) + " %)" : "")
                                          + (root.info.lostPlaces ? " in " + root.info.lostPlaces + (root.info.lostPlaces === 1 ? " place" : " places") : "")
                                    : parent.parent.lostPc > 0 ? "Lost by this PC" : "Nothing lost"
                            }
                            Label2 {
                                Layout.fillWidth: true
                                kind: "hint"
                                visible: parent.parent.lost > 0 || parent.parent.lostPc > 0
                                text: "on the board " + Theme.count(parent.parent.lost) + " · by this PC " + parent.parent.lostPc + " blocks" + (root.info.firstLost !== undefined ? " · first at " + root.dur(root.info.firstLost) : "")
                            }
                        }
                    }
                    // the files of the recording
                    ColumnLayout {
                        spacing: 4
                        Layout.fillWidth: true
                        Label2 { text: "Files of this recording"; kind: "hint" }
                        Repeater {
                            model: root.info.files || []
                            delegate: Text {
                                required property var modelData
                                text: "<a href=\"f\">." + modelData.kind + "</a> · " + Theme.bytes(modelData.size)
                                textFormat: Text.StyledText
                                linkColor: Theme.accent
                                color: Theme.muted
                                font.pixelSize: Theme.fsMd
                                font.family: Theme.mono
                                onLinkActivated: recordings.openFile(modelData.path)
                                HoverHandler { cursorShape: Qt.PointingHandCursor }
                            }
                        }
                    }
                    Item { Layout.fillHeight: true }
                    RowLayout {
                        spacing: 8
                        Btn { kind: "primary"; text: "View"; enabled: false; ToolTip.visible: hovered; ToolTip.text: "The next step (4.7): the recording with its losses marked" }
                        Btn { id: fileBtn; kind: "outline"; text: "File ▾"; active: fileMenu.opened; onClicked: fileMenu.popupFor([root.current], fileBtn) }
                    }
                }
            }
        }
    }

    // a row of the list; head: the column names
    component Row6: Item {
        id: row
        property bool head: false
        property var r: null
        readonly property bool isCur: !!r && r.path === root.current
        readonly property bool converting: !!r && root.job.path === r.path
        implicitHeight: 30
        Rectangle {
            anchors.fill: parent
            visible: !row.head
            color: row.isCur ? "#3a4a52" : rowMouse.containsMouse ? Theme.hover : "transparent"
            Rectangle { visible: row.isCur; width: 3; height: parent.height; color: Theme.accent }
            Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.borderSoft }
        }
        MouseArea {
            id: rowMouse
            anchors.fill: parent
            enabled: !row.head
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onClicked: (e) => {
                root.current = row.r.path
                if (e.button === Qt.RightButton) {
                    const t = root.sel[row.r.path] ? root.targets() : [row.r.path]
                    fileMenu.popupAt(t, rowMouse, e.x, e.y)
                }
            }
        }
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 10
            Item {
                Layout.preferredWidth: 22
                implicitHeight: 20
                Check {
                    visible: !row.head
                    anchors.centerIn: parent
                    padding: 0
                    checked: !!row.r && !!root.sel[row.r.path]
                    onClicked: root.toggle(row.r.path)
                    Accessible.name: row.r ? "Select " + row.r.name : ""
                }
            }
            Cell { t: row.head ? "NAME" : row.r ? row.r.name : ""; w: 3.6; mono: true; strong: true }
            Cell { t: row.head ? "BOARD" : row.r ? row.r.board : ""; w: 1.2 }
            Cell { t: row.head ? "STARTED" : row.r ? root.when(row.r.started) : ""; w: 1.5; mono: true; elide: Text.ElideRight }
            Cell { t: row.head ? "LENGTH" : row.r ? root.dur(row.r.seconds) : ""; w: 1; mono: true }
            Cell { t: row.head ? "INPUTS" : row.r && row.r.inputs ? row.r.inputs.map(i => "IN" + i).join(" ") : "-"; w: 0.9 }
            Item {
                Layout.fillWidth: true
                Layout.preferredWidth: root.job.path || root.all.some(r => r.recording) ? 140 : 10
                implicitHeight: 20
                Text { visible: false; text: "STATE"; color: Theme.muted; font.pixelSize: Theme.fsXs; anchors.verticalCenter: parent.verticalCenter }
                Badge { visible: !row.head && !!row.r && row.r.recording; text: "Recording"; kind: "ok"; anchors.verticalCenter: parent.verticalCenter }
                RowLayout {
                    visible: row.converting
                    anchors.fill: parent
                    spacing: 6
                    Rectangle {
                        Layout.fillWidth: true
                        height: 6; radius: 3; color: Theme.border
                        Rectangle { width: parent.width * (root.job.progress || 0); height: parent.height; radius: 3; color: Theme.accent }
                    }
                    Text { text: Math.round((root.job.progress || 0) * 100) + " %"; color: Theme.value; font.pixelSize: Theme.fsSm; font.family: Theme.mono }
                    Btn { kind: "outline"; compact: true; text: "Stop"; onClicked: recordings.stopConvert() }
                }
            }
            Cell { t: row.head ? "SIZE" : row.r ? Theme.bytes(row.r.size) : ""; w: 0.9; mono: true }
            Cell { t: row.head ? "FORMAT" : row.r ? row.r.format : ""; w: 0.7 }
            Cell { t: row.head ? "LOST" : row.r ? (row.r.lost < 0 ? "-" : Theme.count(row.r.lost)) : ""; w: 0.7; mono: true; bad: !!row.r && row.r.lost > 0 }
        }
    }
    component Cell: Text {
        property string t: ""
        property real w: 1
        property bool mono: false
        property bool strong: false
        property bool bad: false
        // a short value keeps its width; the name gives way
        property bool keep: mono && !strong
        text: t
        Layout.minimumWidth: keep ? implicitWidth : 0
        Layout.fillWidth: true
        Layout.preferredWidth: 100 * w
        elide: Text.ElideMiddle
        color: parent.parent.head ? Theme.muted : bad ? Theme.dangerText : strong ? Theme.value : Theme.text
        font.pixelSize: parent.parent.head ? Theme.fsXs : Theme.fsMd
        font.family: mono && !parent.parent.head ? Theme.mono : Qt.application.font.family
    }

    // File: what is done with a recording (or the ticked ones)
    Popup {
        id: fileMenu
        property var paths: []
        readonly property var recs: root.all.filter(r => paths.indexOf(r.path) >= 0)
        readonly property bool live: recs.some(r => r.recording)
        function popupFor(p, item) { paths = p; parent = item; x = 0; y = -height - 4; open() }
        function popupAt(p, item, px, py) { paths = p; parent = item; x = px; y = py; open() }
        padding: 4
        width: 230
        background: Rectangle { color: Theme.card; border.color: Theme.borderStrong; radius: 4 }
        contentItem: Column {
            spacing: 0
            MenuRow { text: "Convert…"; enabled: !fileMenu.live && fileMenu.recs.length > 0 && fileMenu.recs.every(r => r.convertible); onPicked: convertDialog.ask(fileMenu.paths) }
            MenuRow { text: "Show in folder"; enabled: fileMenu.paths.length === 1; onPicked: recordings.showInFolder(fileMenu.paths[0]) }
            MenuRow { text: "Copy path"; enabled: fileMenu.paths.length === 1; onPicked: recordings.copyPath(fileMenu.paths[0]) }
            MenuRow { text: "Open the report"; enabled: fileMenu.paths.length === 1 && recordings.companions(fileMenu.paths[0]).some(c => c.endsWith(".log.txt")); onPicked: recordings.openFile(fileMenu.paths[0] + ".log.txt") }
            Rectangle { width: parent.width; height: 1; color: Theme.border }
            MenuRow { text: "Delete…"; bad: true; enabled: !fileMenu.live && fileMenu.paths.length > 0; onPicked: deleteDialog.ask(fileMenu.paths) }
        }
    }
    component MenuRow: AbstractButton {
        id: mr
        property bool bad: false
        signal picked()
        width: 222
        height: 30 + Theme.fsAdd
        hoverEnabled: true
        onClicked: { fileMenu.close(); picked() }
        background: Rectangle { color: mr.hovered && mr.enabled ? Theme.hover : "transparent"; radius: 3 }
        contentItem: Text {
            leftPadding: 10
            text: mr.text
            verticalAlignment: Text.AlignVCenter
            color: !mr.enabled ? Theme.dim : mr.bad ? Theme.dangerText : Theme.text
            font.pixelSize: Theme.fsBase
        }
    }

    // Convert: BIN to CSV, WAV, TDMS (a GPIO recording to CSV, VCD)
    Dialog2 {
        id: convertDialog
        property var paths: []
        property string format: "CSV"
        property var inputs: []
        property var chosen: ({})
        property bool index: true
        property bool time: false
        property bool timeNs: false
        property string output: ""
        property string error: ""
        readonly property var d: paths.length === 1 && paths[0] === root.current ? root.info : ({})
        width: Math.min(560, Overlay.overlay ? Overlay.overlay.width - 48 : 560)
        height: Math.min(convCol.implicitHeight + 40 + chromeHeight, maxHeight)
        title: "Convert " + paths.length + (paths.length === 1 ? " recording" : " recordings")
        function ask(p) {
            paths = p
            error = ""
            const ins = (root.current === p[0] && root.info.inputs) ? root.info.inputs : [1, 2]
            inputs = ins
            const c = {}
            for (const i of ins) c[i] = true
            chosen = c
            output = p.length === 1 ? recordings.outputFor(p[0], format) : ""
            open()
        }
        onFormatChanged: if (paths.length === 1) output = recordings.outputFor(paths[0], format)
        // about the size of the output: CSV ~7 characters a value, WAV and TDMS the codes
        readonly property double estimate: {
            if (!d.samples) return 0
            const n = Object.keys(chosen).filter(k => chosen[k]).length
            const per = format === "CSV" ? 7 + (index ? 11 / Math.max(1, n) : 0) + (time || timeNs ? 24 / Math.max(1, n) : 0) : (d.bits ? d.bits / 8 : 2)
            return d.samples * n * per
        }
        ColumnLayout {
            id: convCol
            x: 20; y: 18
            width: parent.width - 40
            spacing: 12
            Field {
                label: "Format"
                Seg { options: ["CSV", "WAV", "TDMS"]; currentIndex: ["CSV", "WAV", "TDMS"].indexOf(convertDialog.format); onPicked: (i) => convertDialog.format = ["CSV", "WAV", "TDMS"][i] }
            }
            Field {
                label: "Inputs"
                Row {
                    spacing: 16
                    Repeater {
                        model: convertDialog.inputs
                        delegate: Check {
                            required property var modelData
                            text: "IN" + modelData
                            checked: !!convertDialog.chosen[modelData]
                            onClicked: { const c = Object.assign({}, convertDialog.chosen); c[modelData] = checked; convertDialog.chosen = c }
                        }
                    }
                }
            }
            Field {
                visible: convertDialog.format === "CSV"
                label: "CSV columns"
                Row {
                    spacing: 16
                    Check { text: "Index"; checked: convertDialog.index; onClicked: convertDialog.index = checked }
                    Check { text: "Time of each sample"; checked: convertDialog.time; onClicked: { convertDialog.time = checked; if (checked) convertDialog.timeNs = false } }
                    Check { text: "Time in ns"; checked: convertDialog.timeNs; onClicked: { convertDialog.timeNs = checked; if (checked) convertDialog.time = false } }
                }
            }
            Field {
                visible: convertDialog.paths.length === 1
                label: "Into"
                Layout.fillWidth: true
                TextIn { free: true; mono: true; Layout.fillWidth: true; text: convertDialog.output; onTextChanged: convertDialog.output = text }
            }
            Label2 {
                visible: convertDialog.paths.length > 1
                kind: "hint"
                Layout.fillWidth: true
                text: "Each beside its recording, with the extension of the format"
            }
            Label2 {
                visible: convertDialog.estimate > 0
                kind: convertDialog.estimate > recordings.diskFree * 0.9 ? "warn" : "hint"
                Layout.fillWidth: true
                text: "The " + convertDialog.format + " takes about " + Theme.bytes(convertDialog.estimate) + "; " + Theme.bytes(recordings.diskFree) + " free"
            }
            Label2 { visible: convertDialog.error !== ""; kind: "bad"; text: convertDialog.error; Layout.fillWidth: true }
        }
        footer: [
            Item { Layout.fillWidth: true },
            Btn { kind: "outline"; text: "Cancel"; onClicked: convertDialog.close() },
            Btn {
                kind: "primary"; text: "Convert"
                enabled: Object.keys(convertDialog.chosen).some(k => convertDialog.chosen[k])
                onClicked: {
                    const ins = Object.keys(convertDialog.chosen).filter(k => convertDialog.chosen[k]).map(Number)
                    convertDialog.error = recordings.convert(convertDialog.paths, { format: convertDialog.format, inputs: ins, index: convertDialog.index,
                        time: convertDialog.time, timeNs: convertDialog.timeNs, output: convertDialog.paths.length === 1 ? convertDialog.output : "" })
                    if (convertDialog.error === "") { root.say("", false); convertDialog.close() }
                }
            }
        ]
    }

    // Delete: the recordings and the files made with them
    Dialog2 {
        id: deleteDialog
        property var paths: []
        readonly property var recs: root.all.filter(r => paths.indexOf(r.path) >= 0)
        width: Math.min(600, Overlay.overlay ? Overlay.overlay.width - 48 : 600)
        height: Math.min(delCol.implicitHeight + 40 + chromeHeight, maxHeight)
        title: "Delete " + paths.length + (paths.length === 1 ? " recording?" : " recordings?")
        function ask(p) { paths = p; open() }
        ColumnLayout {
            id: delCol
            x: 20; y: 18
            width: parent.width - 40
            spacing: 8
            Label2 { text: "These files go, " + Theme.bytes(deleteDialog.recs.reduce((a, r) => a + r.size, 0)) + ":"; Layout.fillWidth: true }
            Repeater {
                model: deleteDialog.paths
                delegate: Text {
                    required property string modelData
                    readonly property var extra: recordings.companions(modelData).map(c => c.slice(modelData.length))
                    text: modelData.split(/[\\/]/).pop() + (extra.length ? " (+ " + extra.join(", ") + ")" : "")
                    color: Theme.muted
                    font.family: Theme.mono
                    font.pixelSize: Theme.fsSm
                    wrapMode: Text.WrapAnywhere
                    Layout.fillWidth: true
                }
            }
            Label2 { kind: "hint"; text: "CSV, WAV and TDMS made from them stay."; Layout.fillWidth: true }
        }
        footer: [
            Item { Layout.fillWidth: true },
            Btn { kind: "outline"; text: "Cancel"; onClicked: deleteDialog.close() },
            Btn {
                kind: "danger"; text: "Delete"
                onClicked: {
                    const e = recordings.remove(deleteDialog.paths)
                    root.say(e, e !== "")
                    deleteDialog.close()
                }
            }
        ]
    }

    // Folders: where the manager looks
    Dialog2 {
        id: foldersDialog
        property string error: ""
        width: Math.min(640, Overlay.overlay ? Overlay.overlay.width - 48 : 640)
        height: Math.min(folCol.implicitHeight + 40 + chromeHeight, maxHeight)
        title: "Folders of the recordings"
        onOpened: error = ""
        ColumnLayout {
            id: folCol
            x: 20; y: 18
            width: parent.width - 40
            spacing: 8
            Repeater {
                model: recordings.folders
                delegate: RowLayout {
                    required property var modelData
                    Layout.fillWidth: true
                    spacing: 10
                    Text { text: modelData.path; color: Theme.value; font.family: Theme.mono; font.pixelSize: Theme.fsMd; elide: Text.ElideMiddle; Layout.fillWidth: true }
                    Label2 {
                        kind: "hint"
                        text: modelData.kind === "app" ? "the folder of the application" : modelData.kind === "board" ? "the folder of " + modelData.board : "added here"
                    }
                    Btn { visible: modelData.kind === "added"; kind: "outline"; compact: true; text: "Remove"; onClicked: recordings.removeFolder(modelData.path) }
                }
            }
            Label2 { visible: foldersDialog.error !== ""; kind: "bad"; text: foldersDialog.error; Layout.fillWidth: true }
        }
        footer: [
            Btn { kind: "outline"; text: "Add a folder…"; onClicked: pickFolder.open() },
            Item { Layout.fillWidth: true },
            Btn { kind: "primary"; text: "Done"; onClicked: foldersDialog.close() }
        ]
    }
    FolderDialog {
        id: pickFolder
        title: "A folder of recordings"
        onAccepted: foldersDialog.error = recordings.addFolder(app.urlToPath(selectedFolder))
    }
}
