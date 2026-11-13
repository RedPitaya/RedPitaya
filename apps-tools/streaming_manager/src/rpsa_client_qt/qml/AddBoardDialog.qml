import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "ui"

// Add a board by its address: one that the broadcast does not reach
Dialog2 {
    id: root
    width: Math.min(560, Overlay.overlay ? Overlay.overlay.width - 48 : 560)
    height: Math.min(col.implicitHeight + 40 + chromeHeight, maxHeight)
    title: "Add board by IP"
    property string error: ""

    onOpened: { error = ""; input.text = ""; input.forceActiveFocus() }

    function add() {
        error = app.addManualHost(input.text)
        if (error === "") close()
    }

    ColumnLayout {
        id: col
        x: 20; y: 18
        width: parent.width - 40
        spacing: 12
        Label2 {
            kind: "sub"
            Layout.fillWidth: true
            text: "A board in another subnet does not answer the broadcast search. Its address is kept on this computer and the board shows up on the dashboard."
        }
        Field {
            label: "IP address or host name"
            hint: root.error
            hintBad: root.error !== ""
            Layout.fillWidth: true
            TextIn {
                id: input
                free: true
                Layout.fillWidth: true
                mono: true
                placeholderText: "192.168.1.120"
                onAccepted: root.add()
            }
        }
    }

    footer: [
        Item { Layout.fillWidth: true },
        Btn { kind: "outline"; text: "Cancel"; onClicked: root.close() },
        Btn { kind: "primary"; text: "Add"; onClicked: root.add() }
    ]
}
