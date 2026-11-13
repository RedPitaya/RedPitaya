import QtQuick
import QtQuick.Layouts

// .card: a framed block with a column of content
Rectangle {
    id: root
    default property alias content: col.data
    property alias spacing: col.spacing
    property int padding: 18
    property string title: ""
    property string subtitle: ""

    color: Theme.card
    border.color: Theme.border
    border.width: 1
    radius: Theme.radius
    implicitWidth: col.implicitWidth + 2 * padding
    implicitHeight: col.implicitHeight + 2 * padding

    ColumnLayout {
        id: col
        x: root.padding
        y: root.padding
        width: root.width - 2 * root.padding
        spacing: 14

        Label2 {
            visible: root.title !== ""
            text: root.title
            kind: "cardTitle"
            Layout.fillWidth: true
        }
        Label2 {
            visible: root.subtitle !== ""
            text: root.subtitle
            kind: "sub"
            Layout.fillWidth: true
        }
    }
}
