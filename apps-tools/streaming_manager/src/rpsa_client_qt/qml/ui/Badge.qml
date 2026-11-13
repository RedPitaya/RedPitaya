import QtQuick

// .badge: a state with its dot; kind is ok, amber, red or grey
Rectangle {
    id: root
    property string text: ""
    property string kind: "grey"
    implicitHeight: 24 + Theme.fsAdd
    implicitWidth: row.implicitWidth + 20
    radius: Theme.radius
    color: kind === "ok" ? Theme.okBg : kind === "amber" ? Theme.amberBg : kind === "red" ? Theme.redBg : Theme.hover
    Row {
        id: row
        anchors.centerIn: parent
        spacing: 6
        Dot {
            anchors.verticalCenter: parent.verticalCenter
            color: root.kind === "ok" ? Theme.ok : root.kind === "amber" ? Theme.amber : root.kind === "red" ? Theme.redDot : Theme.dim
        }
        Text {
            text: root.text
            color: root.kind === "ok" ? Theme.okText : root.kind === "amber" ? Theme.amberText : root.kind === "red" ? Theme.dangerText : Theme.text
            font.pixelSize: Theme.fsSm
            font.bold: true
        }
    }
}
