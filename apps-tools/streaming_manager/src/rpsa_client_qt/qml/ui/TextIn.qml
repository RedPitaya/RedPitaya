import QtQuick
import QtQuick.Controls

// .field input: committed() once the text is done (Enter or the focus leaves)
TextField {
    id: root
    property bool mono: false
    property bool bad: false
    // the value as the board has it: shown again when the edit is not taken
    property string value: ""
    // a free input (not a value of the board): the text stays as typed
    property bool free: false
    signal committed(string text)

    text: value
    onValueChanged: if (!activeFocus && !free) text = value
    onEditingFinished: { if (free) return; if (text !== value) committed(text); else text = value }
    onActiveFocusChanged: if (!activeFocus && !free) text = value

    implicitHeight: Theme.ctrlH
    implicitWidth: 160
    leftPadding: 10
    rightPadding: 10
    verticalAlignment: TextInput.AlignVCenter
    color: Theme.value
    placeholderTextColor: Theme.dim
    selectionColor: Theme.accent
    selectedTextColor: "#000000"
    font.pixelSize: Theme.fsBase
    font.family: mono ? Theme.mono : Qt.application.font.family
    background: Rectangle {
        radius: Theme.radius
        color: Theme.inputBg
        border.width: root.activeFocus ? 2 : 1
        border.color: root.bad ? Theme.danger : root.activeFocus ? Theme.accent : Theme.inputBorder
    }
}
