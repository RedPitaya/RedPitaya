import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.5

Row {
    property bool isMaster: true
    property bool onLine: true
    property string ipAddress: "127.0.0.1"
    property bool isConfigConnected: false
    property bool isADCStarted:false

    Item {
        width: baseSpace * mainVisibleRootWindowId.scaleFactor
        height: parent.height
    }

    Item {
        width: parent.height
        height: parent.height
        Item {
            anchors.fill: parent
            anchors.margins: baseSpace * mainVisibleRootWindowId.scaleFactor
            Rectangle {
                color: "transparent"
                border.width: 1 * mainVisibleRootWindowId.scaleFactor
                border.color: onLine ? baseGreenColor : baseRedColor
                anchors.fill: parent
                anchors.topMargin: 2 * mainVisibleRootWindowId.scaleFactor
                anchors.leftMargin: 2 * mainVisibleRootWindowId.scaleFactor
                anchors.rightMargin: 2 * mainVisibleRootWindowId.scaleFactor
                radius: 5 * mainVisibleRootWindowId.scaleFactor
            }

            Text {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                color: onLine ? baseGreenColor : baseRedColor
                font.pixelSize: height * 0.6
                font.family: applicationFont.name
                font.bold: true
                text: isMaster ? "M" : "S"
            }
        }
    }

    Item {
        width: baseSpace * mainVisibleRootWindowId.scaleFactor
        height: parent.height
    }

    Item {
        width: parent.width - baseSpace * mainVisibleRootWindowId.scaleFactor * 2 - parent.height * 4
        height: parent.height
        Item {
            anchors.fill: parent
            anchors.margins: baseSpace * mainVisibleRootWindowId.scaleFactor
            Text {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                color: baseTextColor
                font.pixelSize: height * 0.8
                font.family: applicationFontBold.name
                font.bold: true
                text: ipAddress
            }
        }
    }

    Item {
        width: parent.height
        height: parent.height
        Item {
            anchors.fill: parent
            anchors.margins: baseSpace * mainVisibleRootWindowId.scaleFactor
            visible:isADCStarted
            Text {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                color: baseGreenColor
                font.pixelSize: height * 0.6
                font.family: applicationFontBold.name
                text: qsTr("RUN")
            }
        }
    }

    Item {
        width: parent.height
        height: parent.height
        Item {
            anchors.fill: parent
            anchors.margins: baseSpace * mainVisibleRootWindowId.scaleFactor
            Rectangle {
                color: isConfigConnected ? baseGreenColor : baseRedColor
                anchors.fill: parent
                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                radius: height / 2
                ToolTip{
                    id:ttId
                    text: qsTr("Configuration server state");
                }
                MouseArea{
                    anchors.fill: parent

                    onEntered: {
                        ttId.visible = true
                    }

                    onExited: {
                        ttId.visible = false
                    }
                }
            }
        }
    }
}
