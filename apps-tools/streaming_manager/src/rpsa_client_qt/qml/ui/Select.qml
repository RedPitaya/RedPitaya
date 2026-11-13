import QtQuick
import QtQuick.Controls

// .field select: options are [value, text] pairs; picked(value) when the user chooses one
ComboBox {
    id: root
    property var options: []
    property var value
    signal picked(var value)

    model: options.map(o => o[1])
    currentIndex: {
        for (let i = 0; i < options.length; i++)
            if (String(options[i][0]) === String(value)) return i
        return -1
    }
    displayText: currentIndex >= 0 ? options[currentIndex][1] : (value === undefined ? "" : String(value))
    onActivated: (i) => { if (String(options[i][0]) !== String(value)) picked(options[i][0]) }

    implicitHeight: Theme.ctrlH
    implicitWidth: 180
    font.pixelSize: Theme.fsBase
    hoverEnabled: true

    contentItem: Text {
        leftPadding: 10
        rightPadding: 28
        text: root.displayText
        color: Theme.value
        font: root.font
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    indicator: Canvas {
        x: root.width - width - 10
        y: (root.height - height) / 2
        width: 10; height: 6
        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            ctx.strokeStyle = Theme.text
            ctx.lineWidth = 1.5
            ctx.beginPath(); ctx.moveTo(1, 1); ctx.lineTo(5, 5); ctx.lineTo(9, 1); ctx.stroke()
        }
    }
    background: Rectangle {
        radius: Theme.radius
        color: Theme.inputBg
        border.width: root.visualFocus ? 2 : 1
        border.color: root.visualFocus ? Theme.accent : Theme.inputBorder
    }
    // the options as the browser draws them on the web page: light
    delegate: ItemDelegate {
        required property int index
        required property var modelData
        width: root.width
        height: Theme.ctrlH - 4
        highlighted: root.highlightedIndex === index
        contentItem: Text {
            text: modelData
            color: "#343433"
            font.pixelSize: Theme.fsBase
            font.bold: index === root.currentIndex
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        background: Rectangle { color: highlighted ? "#cfe8f0" : "#eeeeee" }
    }
    popup: Popup {
        y: root.height
        width: root.width
        implicitHeight: Math.min(contentItem.implicitHeight + 2, 360)
        padding: 1
        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator {}
        }
        background: Rectangle { color: "#eeeeee"; border.color: "#999999"; radius: 2 }
    }
}
