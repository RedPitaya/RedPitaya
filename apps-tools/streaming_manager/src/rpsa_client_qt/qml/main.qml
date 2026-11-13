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

    Dashboard {
        id: dash
        anchors.fill: parent
        onOpenBoard: (board, tab) => boardDialog.openFor(board, tab)
        onOpenApp: appDialog.open()
        onAddBoard: addDialog.open()
    }

    BoardDialog { id: boardDialog }
    AppDialog { id: appDialog; onAddBoard: addDialog.open() }
    AddBoardDialog { id: addDialog }

    // RPSA_QT_UI: steps run at the start, for tests and screenshots, separated by ";":
    // list | cards | app | add | board:<index>:<tab> | set:<index>:<key>=<value> | start:<index> |
    // stop:<index> | wait:<ms> | size:<w>x<h> | console:<index> | reconnect:<index> | startall | stopall
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
                else if (p[0] === "startall") boards.runAll()
                else if (p[0] === "stopall") boards.stopAll()
                else if (p[0] === "console" && boards.board(Number(p[1]))) boards.board(Number(p[1])).chartEnable = false
                else if (p[0] === "reconnect" && boards.board(Number(p[1]))) boards.board(Number(p[1])).reconnect()
                else if (p[0] === "set") {
                    const kv = p.slice(2).join(":").split("=")
                    const b = boards.board(Number(p[1]))
                    if (b) b.setValue(kv[0], kv[1])
                }
                else if (p[0] === "start" && boards.board(Number(p[1]))) boards.board(Number(p[1])).startStreaming()
                else if (p[0] === "stop" && boards.board(Number(p[1]))) boards.board(Number(p[1])).stopStreaming()
                else if (p[0] === "cards") app.view = "cards"
                else if (p[0] === "app") appDialog.open()
                else if (p[0] === "add") addDialog.open()
                else if (p[0] === "board") {
                    const b = boards.board(Number(p[1] || 0))
                    if (b) boardDialog.openFor(b, p[2] || "adc")
                }
            }
        }
    }
}
