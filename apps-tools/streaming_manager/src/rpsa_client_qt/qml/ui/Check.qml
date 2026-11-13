import QtQuick
import QtQuick.Controls

// input.cbx with its text
CheckBox {
    id: root
    spacing: 10
    focusPolicy: Qt.StrongFocus
    indicator: Rectangle {
        implicitWidth: 16 + Theme.fsAdd
        implicitHeight: 16 + Theme.fsAdd
        x: root.leftPadding
        y: (root.height - height) / 2
        radius: Theme.radius
        color: root.checked ? Theme.accent : Theme.inputBg
        border.color: root.visualFocus ? Theme.focus : root.checked ? Theme.accent : Theme.muted
        border.width: root.visualFocus ? 2 : 1
        Text {
            anchors.centerIn: parent
            visible: root.checked
            text: "✓"
            color: "#000000"
            font.pixelSize: parent.height - 2
            font.bold: true
        }
    }
    contentItem: Text {
        leftPadding: root.indicator.width + root.spacing
        text: root.text
        color: Theme.text
        font.pixelSize: Theme.fsMd
        verticalAlignment: Text.AlignVCenter
    }
}
