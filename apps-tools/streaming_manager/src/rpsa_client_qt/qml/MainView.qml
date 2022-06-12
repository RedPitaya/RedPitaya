import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.5
import "./components"
import "./base_components"

Item {

    Column {
        anchors.fill: parent
        Item {
            width: parent.width
            height: parent.height - 50
            ListView {
                anchors.fill: parent
                model: ui_controller.getBoardsModel()
                delegate: Item {
                    width: parent.width
                    height: 300 * mainVisibleRootWindowId.scaleFactor
                    Rectangle {
                        color: "transparent"
                        border.width: 1
                        anchors.fill: parent
                    }
                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: 5 * mainVisibleRootWindowId.scaleFactor
                        color: baseRedColor
                    }
                    Item {
                        anchors.fill: parent
                        anchors.leftMargin: 5 * mainVisibleRootWindowId.scaleFactor
                        RPCellHeader {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: parent.top
                            height: 40 * mainVisibleRootWindowId.scaleFactor
                            isMaster: board.isMaster
                            onLine:board.isOnline
                            ipAddress:board.ip
                            isConfigConnected: board.isConfigConnected
                            isADCStarted: board.isADCStarted
                        }

                        RPCell {
                            anchors.topMargin: 40 * mainVisibleRootWindowId.scaleFactor
                            anchors.fill: parent
                        }
                    }
                }
            }
        }

        Item {
            width: parent.width
            height: 50
            Item {
                anchors.fill: parent
                anchors.topMargin: 10
                anchors.bottomMargin: 10
                Row {
                    anchors.fill: parent
                    Item {
                        height: parent.height
                        width: 150 * mainVisibleRootWindowId.scaleFactor
                        BaseButton {
                            anchors.fill: parent
                            text: "START ALL BOARDS"
                            fontFamily: applicationFont.name
                            radius: 5 * mainVisibleRootWindowId.scaleFactor
                            textSize: 15 * mainVisibleRootWindowId.scaleFactor
                            borderColor: baseRedSwitchColor
                            hoverColor: baseRedSwitchColor
                            textColor: baseTextColor

                            onClickButton: function(){
                                ui_controller.runAll()
                            }

                        }
                    }
                    Item {
                        width: 12 * mainVisibleRootWindowId.scaleFactor
                        height: parent.height
                    }
                    Item {
                        height: parent.height
                        width: 150 * mainVisibleRootWindowId.scaleFactor
                        BaseButton {
                            anchors.fill: parent
                            text: "STOP ALL BOARDS"
                            fontFamily: applicationFont.name
                            radius: 5 * mainVisibleRootWindowId.scaleFactor
                            textSize: 15 * mainVisibleRootWindowId.scaleFactor
                            borderColor: baseRedSwitchColor
                            hoverColor: baseRedSwitchColor
                            textColor: baseTextColor

                            onClickButton: function(){
                                ui_controller.stopAll()
                            }
                        }
                    }
                    Item {
                        width: 12 * mainVisibleRootWindowId.scaleFactor
                        height: parent.height
                    }
                    Item {
                        height: parent.height
                        width: 150 * mainVisibleRootWindowId.scaleFactor
                        BaseButton {
                            anchors.fill: parent
                            text: "OPEN FOLDER"
                            fontFamily: applicationFont.name
                            radius: 5 * mainVisibleRootWindowId.scaleFactor
                            textSize: 15 * mainVisibleRootWindowId.scaleFactor
                            borderColor: baseRedSwitchColor
                            hoverColor: baseRedSwitchColor
                            textColor: baseTextColor

                            onClickButton: function(){
                                ui_controller.openFolder()
                            }
                        }
                    }
                }
            }
        }
    }
}
