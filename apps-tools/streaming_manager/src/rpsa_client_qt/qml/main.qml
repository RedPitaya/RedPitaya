import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.5

ApplicationWindow {
    id: appId
    width: 1980
    height: 1080
    visible: true
    title: qsTr("Red Pitaya X-Streaming")
    visibility : Window.Maximized

    property int nativeWidth: 1980
    property int nativeHeight: 1080
    property double aspectRate: nativeWidth / nativeHeight

    property color  baseRedColor: "#d02321"
    property color  baseRedSwitchColor: "#f5c4b8"
    property color  baseGreenColor: "#10AF10"
    property color  baseTextColor: "#101010"
    property color  basebuttonTextColor: "#101010"

    property double baseSpace: 5

    property FontLoader applicationFontBold:FontLoader {
            source: "qrc:/fonts/BarlowSemiCondensed-Bold.ttf"
    }

    property FontLoader applicationFont:FontLoader {
            source: "qrc:/fonts/BarlowSemiCondensed-Regular.ttf"
    }

    function getWidth() {
        var aspect = appId.width / appId.height
        if (aspect < appId.aspectRate) {
            return width
        } else {
            return height * appId.aspectRate
        }
    }

    function getHeight() {
        var aspect = appId.width / appId.height
        if (aspect < appId.aspectRate) {
            return width / appId.aspectRate
        } else {
            return height
        }
    }


    Item {
            id: mainRootWindowId
            anchors.fill: parent
            Item {
                id: mainVisibleRootWindowId
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
//                height: appId.getHeight()
//                width: appId.getWidth()
                height: parent.height
                width: parent.width
                property double scaleFactor: mainVisibleRootWindowId.height / nativeHeight

                MainView {
                    anchors.fill: parent
                    anchors.margins: 20 * mainVisibleRootWindowId.scaleFactor
                }
            }
    }

}
