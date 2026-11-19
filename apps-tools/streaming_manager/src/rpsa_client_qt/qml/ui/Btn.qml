import QtQuick
import QtQuick.Controls

// .btn: kind is default (light), primary (blue), danger (red outline) or outline (grey outline)
Button {
    id: root
    property string kind: "default"
    property string iconSource: ""
    property bool compact: false
    // an outline button that is on (a toggle): filled and bold
    property bool active: false

    implicitHeight: compact ? 24 + Theme.fsAdd : Theme.ctrlH
    implicitWidth: Math.max(implicitHeight, contentItem.implicitWidth + leftPadding + rightPadding)
    leftPadding: text === "" ? 0 : (compact ? 10 : 16)
    rightPadding: leftPadding
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus

    readonly property color fg: kind === "primary" || kind === "default" ? "#000000"
                              : kind === "danger" ? Theme.dangerText : Theme.text

    // the icon and the text in the middle of the button, whatever its size
    contentItem: Item {
        implicitWidth: row.implicitWidth
        implicitHeight: row.implicitHeight
        Row {
            id: row
            spacing: 8
            anchors.centerIn: parent
            Image {
                visible: root.iconSource !== ""
                source: root.iconSource
                width: 16; height: 16
                sourceSize: Qt.size(32, 32)
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                visible: root.text !== ""
                text: root.text
                color: root.enabled ? root.fg : Theme.dim
                font.pixelSize: root.compact ? Theme.fsSm : Theme.fsBase
                font.bold: root.kind !== "outline" || root.active
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    background: Rectangle {
        radius: Theme.radius
        opacity: root.enabled ? 1 : 0.45
        border.width: 1
        border.color: root.visualFocus ? Theme.focus
                    : root.kind === "primary" ? Theme.accent
                    : root.kind === "danger" ? Theme.danger
                    : root.kind === "outline" ? Theme.borderStrong : "#888888"
        color: root.kind === "primary" ? (root.hovered ? Theme.accentHover : Theme.accent)
             : root.kind === "default" ? (root.hovered ? Theme.title : Theme.text)
             : root.kind === "danger" ? (root.hovered ? Theme.redBg : "transparent")
             : root.active ? Theme.hover
             : (root.hovered ? Theme.hover : "transparent")
    }
}
