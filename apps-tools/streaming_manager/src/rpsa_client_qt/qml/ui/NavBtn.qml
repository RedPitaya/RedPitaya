import QtQuick
import QtQuick.Controls

// .nav-b: a section of the dialog
AbstractButton {
    id: root
    property bool current: false
    implicitHeight: 40 + 2 * Theme.fsAdd
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus
    Accessible.name: text
    background: Rectangle {
        radius: Theme.radius
        color: root.current ? Theme.hover : root.hovered ? Theme.hover : "transparent"
        border.width: root.visualFocus ? 2 : 0
        border.color: Theme.focus
    }
    contentItem: Row {
        spacing: 10
        leftPadding: 12
        Dot {
            anchors.verticalCenter: parent.verticalCenter
            color: root.current ? Theme.accent : "transparent"
        }
        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: root.text
            color: root.current ? "#ffffff" : Theme.text
            font.bold: root.current
            font.pixelSize: Theme.fsBase
        }
    }
}
