import QtQuick
import QtQuick.Controls

// .seg: one of a few choices; the chosen one has the light knob
Rectangle {
    id: root
    property var options: []          // strings
    property int currentIndex: 0
    property bool small: false
    signal picked(int index)

    implicitHeight: small ? 24 + Theme.fsAdd : Theme.ctrlH
    implicitWidth: row.implicitWidth + 2
    color: Theme.card
    border.color: Theme.borderStrong
    border.width: 1
    radius: Theme.radius

    Row {
        id: row
        x: 1; y: 1
        height: parent.height - 2
        Repeater {
            model: root.options
            delegate: AbstractButton {
                id: b
                required property int index
                required property string modelData
                height: row.height
                width: label.implicitWidth + (root.small ? 20 : 28)
                hoverEnabled: true
                focusPolicy: Qt.StrongFocus
                onClicked: { if (root.currentIndex !== index) root.picked(index) }
                Accessible.name: modelData
                background: Rectangle {
                    color: b.index === root.currentIndex ? Theme.text : (b.hovered ? Theme.hover : "transparent")
                    radius: 2
                    border.width: b.visualFocus ? 2 : 0
                    border.color: Theme.focus
                    Rectangle {
                        visible: b.index > 0
                        width: 1; height: parent.height
                        color: Theme.borderStrong
                    }
                }
                contentItem: Text {
                    id: label
                    text: b.modelData
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: b.index === root.currentIndex ? "#111111" : Theme.text
                    font.bold: b.index === root.currentIndex
                    font.pixelSize: root.small ? Theme.fsSm : Theme.fsMd
                }
            }
        }
    }
}
