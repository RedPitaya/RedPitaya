import QtQuick
import QtQuick.Controls

// input.flipswitch: OFF | ON with a sliding knob, and the text it switches
AbstractButton {
    id: root
    property bool small: false
    property string label: ""
    checkable: true
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus
    Accessible.name: label

    implicitHeight: Math.max(sw.height, txt.implicitHeight)
    implicitWidth: sw.width + (label !== "" ? 10 + txt.implicitWidth : 0)

    contentItem: Item {
        Rectangle {
            id: sw
            width: (root.small ? 62 : 70) + 4 * Theme.fsAdd
            height: (root.small ? 24 : 28) + Theme.fsAdd
            anchors.verticalCenter: parent.verticalCenter
            color: Theme.card
            radius: Theme.radius
            border.width: root.visualFocus ? 2 : 1
            border.color: root.visualFocus ? Theme.accent : Theme.borderStrong
            Rectangle {
                id: knob
                y: 2
                width: parent.width / 2 - 4
                height: parent.height - 4
                x: root.checked ? parent.width / 2 + 2 : 2
                radius: Theme.radius
                color: Theme.text
                border.color: "#888888"
                Behavior on x { NumberAnimation { duration: 200; easing.type: Easing.InQuad } }
                Text {
                    anchors.centerIn: parent
                    text: root.checked ? "ON" : "OFF"
                    font.pixelSize: Theme.fsXs
                    font.bold: true
                    color: "#000000"
                }
            }
        }
        Text {
            id: txt
            visible: root.label !== ""
            text: root.label
            x: sw.width + 10
            width: parent.width - x
            anchors.verticalCenter: parent.verticalCenter
            color: Theme.text
            font.pixelSize: Theme.fsMd
            wrapMode: Text.WordWrap
        }
    }
    background: Item {}
}
