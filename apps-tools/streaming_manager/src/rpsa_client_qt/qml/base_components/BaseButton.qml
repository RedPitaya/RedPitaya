import QtQuick 2.12
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.12

import QtQuick.Window 2.12

Rectangle {
    id: rootBaseButton

    property string text: "text"
    property bool enabledButton: true
    property color textColor: "#0c8846"
    property color textDisableColor: "#0c8846"
    property color backColor: "white"
    property color hoverColor: "#5cb259"
    property color disableColor: "#a0000000"
    property double textSpacing: 3
    property double buttonRadius: height / 2.0
    property double textSize: 15
    property string fontFamily: Style.applicationFont.name
    property color borderColor: "#30000000"
    property bool borderVisible: true
    property real borderSize: 1
    property real textMargin: 0
    property bool buttonPress: false
    property int tagId: Math.floor(Math.random() * 1000000)

    color: enabledButton ? (buttonPress ? hoverColor : backColor) : disableColor

    radius: buttonRadius
    border.color: borderColor
    border.width: borderVisible ? borderSize : 0

    signal clickButton

    function onClicked() {
        if (enabledButton)
            clickButton()
    }

    Behavior on color {
        ColorAnimation {
            duration: 100
        }
    }

    Text {
        id: textLabel
        font.pixelSize: textSize
        minimumPixelSize: 3
        font.family: fontFamily
        color: enabledButton ? rootBaseButton.textColor : textDisableColor
        text: rootBaseButton.text
        anchors.fill: parent
        anchors.margins: textMargin
        fontSizeMode: Text.Fit
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onPressed: {
            if (enabledButton) {
                buttonPress = false
            }
        }

        onReleased: {
            if (enabledButton) {
                buttonPress = true
                rootBaseButton.onClicked()
            }
        }

        onEntered: {
            if (enabledButton) {
                buttonPress = true
            }
        }

        onExited: {
            if (enabledButton) {
                buttonPress = false
            }
        }
    }
}
