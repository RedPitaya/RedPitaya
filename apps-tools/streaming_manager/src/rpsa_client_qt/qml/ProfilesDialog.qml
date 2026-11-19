import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "ui"

// Profiles of the settings (streaming_improvements 4.9): kept on this computer, saved from a
// board, applied to boards. The data: profiles (ProfilesManager).
Dialog2 {
    id: root
    // Apply to boards…, Save again from a board…: the dialogs of main.qml
    signal applyProfile(string id)
    signal saveAgain(string name, var groups)

    property string current: ""
    property string message: ""
    property bool messageBad: false
    readonly property var all: profiles.list
    // read again when the list changes (renamed, saved again)
    readonly property var d: { root.all; return current !== "" ? profiles.details(current) : ({}) }
    onAllChanged: if (!all.some(p => p.id === current)) current = all.length ? all[0].id : ""
    onOpened: { message = ""; if (current === "" && all.length) current = all[0].id }
    function say(text, bad) { message = text; messageBad = bad }

    width: Math.min(1100, Overlay.overlay ? Overlay.overlay.width - 48 : 1100)
    height: Math.min(720, maxHeight)
    title: "Profiles of settings"
    subtitle: "Kept on this computer. A board takes one when it is applied; its streams stop."

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // the profiles
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
                Row4 { head: true; Layout.fillWidth: true }
                Rectangle { Layout.fillWidth: true; height: 1; color: Theme.borderStrong }
                ListView {
                    id: listView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: root.all
                    ScrollBar.vertical: ScrollBar {}
                    delegate: Row4 {
                        required property var modelData
                        width: listView.width
                        p: modelData
                    }
                    Label2 {
                        anchors.centerIn: parent
                        width: parent.width - 40
                        visible: root.all.length === 0
                        horizontalAlignment: Text.AlignHCenter
                        kind: "hint"
                        text: "No profiles yet.\nA board saves its settings as one: its settings, Board, Save as profile…; or Import JSON… below."
                    }
                }
                Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }
                RowLayout {
                    Layout.fillWidth: true
                    Layout.margins: 8
                    Layout.leftMargin: 12
                    spacing: 10
                    Label2 {
                        Layout.fillWidth: true
                        kind: root.message !== "" && root.messageBad ? "bad" : "hint"
                        text: root.message !== "" ? root.message
                            : root.all.length + (root.all.length === 1 ? " profile" : " profiles") + " · the Board tab of a board saves its settings as one"
                    }
                    Btn { kind: "outline"; compact: true; text: "Import JSON…"; onClicked: importDlg.open() }
                }
            }
        }

        // the profile
        Rectangle {
            Layout.preferredWidth: 400
            Layout.fillHeight: true
            color: Theme.card
            border.color: Theme.border
            radius: Theme.radius
            Label2 { anchors.centerIn: parent; visible: root.current === ""; kind: "hint"; text: "Click a profile" }
            ColumnLayout {
                visible: root.current !== ""
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12
                ColumnLayout {
                    spacing: 3
                    Layout.fillWidth: true
                    Text { text: root.d.name || ""; color: Theme.title; font.pixelSize: Theme.fsLg; font.bold: true; wrapMode: Text.Wrap; Layout.fillWidth: true }
                    Label2 { text: "from " + (root.d.from || "") + (root.d.saved ? " · saved " + Qt.formatDateTime(new Date(root.d.saved), "yyyy-MM-dd hh:mm") : ""); kind: "hint"; Layout.fillWidth: true }
                }
                Flickable {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    contentHeight: groupsCol.implicitHeight
                    ScrollBar.vertical: ScrollBar {}
                    ColumnLayout {
                        id: groupsCol
                        width: parent.width
                        spacing: 12
                        Repeater {
                            model: root.d.groups || []
                            delegate: ColumnLayout {
                                required property var modelData
                                Layout.fillWidth: true
                                spacing: 6
                                Text { text: modelData.label; color: Theme.text; font.bold: true; font.pixelSize: Theme.fsMd }
                                GridLayout {
                                    Layout.fillWidth: true
                                    columns: 2
                                    columnSpacing: 10
                                    rowSpacing: 4
                                    Repeater {
                                        model: modelData.rows
                                        delegate: Item {
                                            required property var modelData
                                            Layout.columnSpan: 2
                                            Layout.fillWidth: true
                                            implicitHeight: Math.max(k.implicitHeight, v.implicitHeight)
                                            Label2 { id: k; text: modelData.k; kind: "hint"; width: 120; wrapMode: Text.NoWrap }
                                            Text { id: v; x: 130; width: parent.width - 130; text: modelData.v; color: Theme.value; font.pixelSize: Theme.fsMd; wrapMode: Text.Wrap }
                                        }
                                    }
                                }
                            }
                        }
                        Label2 {
                            visible: !!root.d.missing
                            Layout.fillWidth: true
                            kind: "hint"
                            text: "Not in this profile: " + (root.d.missing || "") + ". A board keeps its own."
                        }
                    }
                }
                RowLayout {
                    spacing: 8
                    Btn { kind: "primary"; text: "Apply to boards…"; onClicked: root.applyProfile(root.current) }
                    Btn { id: menuBtn; kind: "outline"; text: "Profile ▾"; active: menu.opened; onClicked: menu.open() }
                }
            }
        }
    }

    // a row of the list; head: the column names
    component Row4: Item {
        id: row
        property bool head: false
        property var p: null
        readonly property bool isCur: !!p && p.id === root.current
        implicitHeight: 36 + Theme.fsAdd
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
            onClicked: root.current = row.p.id
            onDoubleClicked: root.applyProfile(row.p.id)
        }
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 10
            Cell { t: row.head ? "NAME" : row.p ? row.p.name : ""; w: 2.4; strong: true }
            Cell { t: row.head ? "FROM" : row.p ? (row.p.model || "a JSON file") : ""; w: 1.6 }
            Cell { t: row.head ? "HOLDS" : row.p ? row.p.groups : ""; w: 1.8 }
            Cell { t: row.head ? "SAVED" : row.p ? Qt.formatDateTime(new Date(row.p.saved), "MM-dd hh:mm") : ""; w: 1; mono: true }
        }
    }
    component Cell: Text {
        property string t: ""
        property real w: 1
        property bool mono: false
        property bool strong: false
        text: t
        Layout.fillWidth: true
        Layout.preferredWidth: 100 * w
        elide: Text.ElideRight
        color: parent.parent.head ? Theme.muted : strong ? Theme.value : Theme.text
        font.pixelSize: parent.parent.head ? Theme.fsXs : Theme.fsMd
        font.bold: strong && !parent.parent.head
        font.family: mono && !parent.parent.head ? Theme.mono : Qt.application.font.family
    }

    // Profile: what is done with the profile shown
    Popup {
        id: menu
        parent: menuBtn
        x: 0
        y: -height - 4
        padding: 4
        width: 240
        background: Rectangle { color: Theme.card; border.color: Theme.borderStrong; radius: 4 }
        contentItem: Column {
            spacing: 0
            MenuRow { text: "Rename…"; onPicked: renameDialog.ask() }
            MenuRow { text: "Save again from a board…"; onPicked: { const p = root.all.find(x => x.id === root.current); root.saveAgain(root.d.name, p ? p.groupList : []) } }
            MenuRow { text: "Duplicate"; onPicked: { const id = profiles.duplicate(root.current); if (id !== "") root.current = id } }
            MenuRow { text: "Export JSON…"; onPicked: exportDlg.open() }
            Rectangle { width: parent.width; height: 1; color: Theme.border }
            MenuRow { text: "Delete…"; bad: true; onPicked: deleteDialog.open() }
        }
    }
    component MenuRow: AbstractButton {
        id: mr
        property bool bad: false
        signal picked()
        width: 232
        height: 30 + Theme.fsAdd
        hoverEnabled: true
        onClicked: { menu.close(); picked() }
        background: Rectangle { color: mr.hovered && mr.enabled ? Theme.hover : "transparent"; radius: 3 }
        contentItem: Text {
            leftPadding: 10
            text: mr.text
            verticalAlignment: Text.AlignVCenter
            color: !mr.enabled ? Theme.dim : mr.bad ? Theme.dangerText : Theme.text
            font.pixelSize: Theme.fsBase
        }
    }

    Dialog2 {
        id: renameDialog
        property string text: ""
        property string error: ""
        function ask() { text = root.d.name || ""; error = ""; open() }
        width: Math.min(480, Overlay.overlay ? Overlay.overlay.width - 48 : 480)
        height: Math.min(renCol.implicitHeight + 40 + chromeHeight, maxHeight)
        title: "Rename the profile"
        ColumnLayout {
            id: renCol
            x: 20; y: 18
            width: parent.width - 40
            spacing: 8
            TextIn { free: true; Layout.fillWidth: true; text: renameDialog.text; onTextChanged: renameDialog.text = text; onAccepted: renameDialog.go() }
            Label2 { visible: renameDialog.error !== ""; kind: "bad"; text: renameDialog.error; Layout.fillWidth: true }
        }
        function go() { error = profiles.rename(root.current, text); if (error === "") close() }
        footer: [
            Item { Layout.fillWidth: true },
            Btn { kind: "outline"; text: "Cancel"; onClicked: renameDialog.close() },
            Btn { kind: "primary"; text: "Rename"; onClicked: renameDialog.go() }
        ]
    }

    Dialog2 {
        id: deleteDialog
        width: Math.min(480, Overlay.overlay ? Overlay.overlay.width - 48 : 480)
        height: Math.min(delCol.implicitHeight + 40 + chromeHeight, maxHeight)
        title: "Delete the profile?"
        ColumnLayout {
            id: delCol
            x: 20; y: 18
            width: parent.width - 40
            Label2 { text: (root.d.name || "") + " goes from this computer. The boards keep their settings."; Layout.fillWidth: true; wrapMode: Text.Wrap }
        }
        footer: [
            Item { Layout.fillWidth: true },
            Btn { kind: "outline"; text: "Cancel"; onClicked: deleteDialog.close() },
            Btn { kind: "danger"; text: "Delete"; onClicked: { profiles.remove(root.current); deleteDialog.close() } }
        ]
    }

    FileDialog {
        id: exportDlg
        fileMode: FileDialog.SaveFile
        nameFilters: ["JSON (*.json)"]
        defaultSuffix: "json"
        // beside the recordings (the folder of the application); the name of the profile
        currentFile: app.recordDir !== "" ? app.pathToUrl(app.recordDir.replace("{board}", "") + "/" + String(root.d.name || "profile").replace(/[\\/:*?"<>|]/g, "_") + ".json") : ""
        onAccepted: {
            const e = profiles.exportJson(root.current, selectedFile)
            root.say(e !== "" ? e : "Saved to " + app.urlToPath(selectedFile), e !== "")
        }
    }
    FileDialog {
        id: importDlg
        fileMode: FileDialog.OpenFile
        nameFilters: ["JSON (*.json)", "All files (*)"]
        onAccepted: {
            const e = profiles.importJson(selectedFile)
            root.say(e !== "" ? e : "Imported from " + app.urlToPath(selectedFile), e !== "")
            if (e === "" && root.all.length) root.current = root.all[root.all.length - 1].id
        }
    }
}
