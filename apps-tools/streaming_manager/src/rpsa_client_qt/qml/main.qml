import QtQuick
import QtQuick.Controls
import QtQuick.Window
import "ui"

ApplicationWindow {
    id: win
    // the screen may be smaller than the design size (or scaled): the window fits it
    width: Math.min(1440, Screen.desktopAvailableWidth - 40)
    height: Math.min(900, Screen.desktopAvailableHeight - 60)
    minimumWidth: 720
    minimumHeight: 520
    visible: true
    title: "Red Pitaya streaming client"
    color: Theme.bg

    Binding { target: Theme; property: "fsAdd"; value: app.textSize }

    // the size and the place of the last close, if they still fit a screen of this computer
    Component.onCompleted: {
        const g = app.windowGeometry()
        if (g.width === undefined)
            return
        let fits = false
        for (let i = 0; i < Qt.application.screens.length; i++) {
            const sc = Qt.application.screens[i]
            if (g.x + 40 < sc.virtualX + sc.width && g.x + g.width - 40 > sc.virtualX && g.y >= sc.virtualY - 10 && g.y + 40 < sc.virtualY + sc.height)
                fits = true
        }
        if (!fits)
            return
        width = Math.max(minimumWidth, g.width)
        height = Math.max(minimumHeight, g.height)
        x = g.x
        y = g.y
        if (g.maximized)
            showMaximized()
    }
    onClosing: app.saveWindowGeometry(x, y, width, height, visibility === Window.Maximized)

    Dashboard {
        id: dash
        anchors.fill: parent
        onOpenBoard: (board, tab) => boardDialog.openFor(board, tab)
        onOpenApp: appDialog.open()
        onAddBoard: addDialog.open()
        onAskStart: (board) => diskDialog.start(board)
    }
    DiskDialog { id: diskDialog }

    BoardDialog { id: boardDialog }
    AppDialog { id: appDialog; onAddBoard: addDialog.open() }
    AddBoardDialog { id: addDialog }

    // RPSA_QT_UI: steps run at the start, for tests and screenshots, separated by ";":
    // list | cards | app | add | board:<index>:<tab> | set:<index>:<key>=<value> | start:<index> |
    // stop:<index> | wait:<ms> | size:<w>x<h> | console:<index> | reconnect:<index> | startall | stopall |
    // scroll (the open board dialog to its end) | trigger:<index> (software trigger) |
    // signal:<index> | fit:<index> | fullY:<index> | pdec:<index>:<n> (the preview of a board) |
    // measure:<index>[:off] (its measurements) | spectrum:<index>[:off] | recordings (the page) |
    // rec:menu|convert|delete|folders|tick|convertgo|deletego|next (on the recording shown) | recordings (the page) |
    // rec:menu|convert|delete|folders|tick|convertgo|deletego|next (on the recording shown)
    // type:<key>=<text> (types into the field of that setting in the open board dialog, as the user)
    // <index> may be the IP of the board instead (the order of the boards follows the broadcast)
    function findField(item, key) {
        if (!item) return null
        if (item.objectName === "field:" + key) return item
        const kids = item.children || []
        for (let i = 0; i < kids.length; i++) {
            const f = findField(kids[i], key)
            if (f) return f
        }
        return null
    }
    function scriptBoard(id) { return String(id).indexOf(".") >= 0 ? boards.boardByIp(id) : boards.board(Number(id || 0)) }
    Timer {
        id: script
        property var steps: uiScript === "" ? [] : uiScript.split(";")
        interval: 1500
        running: steps.length > 0
        repeat: true
        onTriggered: {
            interval = 300
            while (steps.length > 0) {
                const p = steps[0].split(":")
                steps = steps.slice(1)
                if (p[0] === "wait") { interval = Number(p[1]); return }
                if (p[0] === "size") { const wh = p[1].split("x"); win.width = Number(wh[0]); win.height = Number(wh[1]) }
                else if (p[0] === "list") app.view = "list"
                else if (p[0] === "recordings") dash.page = "recordings"
                else if (p[0] === "rec") dash.recordingsScript(p[1])
                else if (p[0] === "recordings") dash.page = "recordings"
                else if (p[0] === "startall") diskDialog.start(null)
                else if (p[0] === "stopall") boards.stopAll()
                else if (p[0] === "console" && win.scriptBoard(p[1])) win.scriptBoard(p[1]).chartEnable = false
                else if (p[0] === "signal" && win.scriptBoard(p[1])) win.scriptBoard(p[1]).chartEnable = true
                else if (p[0] === "fit" && win.scriptBoard(p[1])) win.scriptBoard(p[1]).previewFit = true
                else if (p[0] === "pdec" && win.scriptBoard(p[1])) win.scriptBoard(p[1]).previewDecimation = Number(p[2])
                else if (p[0] === "fullY" && win.scriptBoard(p[1])) win.scriptBoard(p[1]).previewFit = false
                else if (p[0] === "measure" && win.scriptBoard(p[1])) win.scriptBoard(p[1]).measure = p[2] !== "off"
                else if (p[0] === "spectrum" && win.scriptBoard(p[1])) win.scriptBoard(p[1]).spectrum = p[2] !== "off"
                else if (p[0] === "reconnect" && win.scriptBoard(p[1])) win.scriptBoard(p[1]).reconnect()
                else if (p[0] === "set") {
                    const kv = p.slice(2).join(":").split("=")
                    const b = win.scriptBoard(p[1])
                    if (b) b.setValue(kv[0], kv[1])
                }
                else if (p[0] === "start" && win.scriptBoard(p[1])) diskDialog.start(win.scriptBoard(p[1]))
                else if (p[0] === "stop" && win.scriptBoard(p[1])) win.scriptBoard(p[1]).stopStreaming()
                else if (p[0] === "scroll") boardDialog.scrollToEnd()
                else if (p[0] === "type") {
                    const kv = p.slice(1).join(":").split("=")
                    const f = win.findField(boardDialog.contentItem, kv[0])
                    if (f) { f.text = kv[1]; f.committed(kv[1]) }
                    else console.warn("type: no field " + kv[0])
                }
                else if (p[0] === "trigger" && win.scriptBoard(p[1])) win.scriptBoard(p[1]).softwareTrigger()
                else if (p[0] === "cards") app.view = "cards"
                else if (p[0] === "app") appDialog.open()
                else if (p[0] === "add") addDialog.open()
                else if (p[0] === "board") {
                    const b = win.scriptBoard(p[1])
                    if (b) boardDialog.openFor(b, p[2] || "adc")
                }
            }
        }
    }
}
