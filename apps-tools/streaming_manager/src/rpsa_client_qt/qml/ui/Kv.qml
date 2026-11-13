import QtQuick
import QtQuick.Layouts

// .kv / .scell: a small label over its value
ColumnLayout {
    property string label: ""
    property string value: ""
    property bool mono: false
    property color valueColor: Theme.value
    spacing: 1
    Label2 { text: parent.label; kind: "hint"; wrapMode: Text.NoWrap; elide: Text.ElideRight; Layout.fillWidth: true; font.pixelSize: Theme.fsXs }
    Text {
        text: parent.value
        color: parent.valueColor
        font.pixelSize: Theme.fsMd
        font.bold: !parent.mono
        font.family: parent.mono ? Theme.mono : Qt.application.font.family
        elide: Text.ElideMiddle
        Layout.fillWidth: true
    }
}
