import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// A modal dialog in the look of the application: a header, the content, a footer
Popup {
    id: root
    property string title: ""
    property string subtitle: ""
    default property alias content: body.data
    property alias header: headerExtra.data
    property alias footer: footerRow.data
    property alias bodyItem: body
    // the header and the footer: a dialog that sizes itself to its content adds them
    readonly property real chromeHeight: headRect.height + (footRect.visible ? footRect.height : 0) + 2
    readonly property real maxHeight: Overlay.overlay ? Overlay.overlay.height - 32 : 900

    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape
    anchors.centerIn: Overlay.overlay
    padding: 0
    Overlay.modal: Rectangle { color: Qt.rgba(0, 0, 0, 0.6) }

    background: Rectangle {
        color: Theme.bg
        border.color: Theme.borderStrong
        radius: 4
    }

    contentItem: ColumnLayout {
        spacing: 0
        Rectangle {
            id: headRect
            Layout.fillWidth: true
            implicitHeight: hdr.implicitHeight + 24
            color: Theme.card
            RowLayout {
                id: hdr
                anchors.fill: parent
                anchors.margins: 12
                anchors.leftMargin: 16
                spacing: 14
                Row { id: headerExtra; spacing: 14; Layout.alignment: Qt.AlignVCenter }
                ColumnLayout {
                    spacing: 2
                    Layout.fillWidth: true
                    Text { text: root.title; color: Theme.title; font.pixelSize: Theme.fsXl; elide: Text.ElideRight; Layout.fillWidth: true }
                    Label2 { visible: root.subtitle !== ""; text: root.subtitle; kind: "hint"; Layout.fillWidth: true }
                }
                Btn {
                    kind: "outline"
                    iconSource: "qrc:/img/icons/close.svg"
                    Accessible.name: "Close"
                    implicitWidth: implicitHeight
                    onClicked: root.close()
                }
            }
            Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: "#777777" }
        }
        Item {
            id: body
            Layout.fillWidth: true
            Layout.fillHeight: true
            implicitHeight: childrenRect.height
            implicitWidth: childrenRect.width
        }
        Rectangle {
            id: footRect
            Layout.fillWidth: true
            implicitHeight: footerRow.implicitHeight + 24
            visible: footerRow.children.length > 0
            color: Theme.card
            Rectangle { width: parent.width; height: 1; color: "#777777" }
            RowLayout {
                id: footerRow
                anchors.fill: parent
                anchors.margins: 12
                anchors.leftMargin: 16
                spacing: 10
            }
        }
    }
}
