import QtQuick
import QtQuick.Layouts

// .field: a label over its controls and an optional hint under them
ColumnLayout {
    id: root
    property string label: ""
    property string hint: ""
    property bool hintBad: false
    default property alias content: holder.data
    spacing: 6
    // in a row of fields the labels and the inputs line up whatever hints are under them
    Layout.alignment: Qt.AlignTop

    Label2 {
        visible: root.label !== ""
        text: root.label
        kind: "label"
        Layout.fillWidth: true
    }
    ColumnLayout {
        id: holder
        spacing: 6
        Layout.fillWidth: true
    }
    Label2 {
        visible: root.hint !== ""
        text: root.hint
        kind: root.hintBad ? "bad" : "hint"
        Layout.fillWidth: true
    }
}
