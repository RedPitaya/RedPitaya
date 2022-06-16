import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.5

import '../base_components'

Column{

    Connections{
        target: board
        function onGetNewSettingSignal(){
            saveTypeId.stateIndex = board.getSaveType()
            protocolId.stateIndex = board.getProtocol()
            decId.text = board.getDecimation()
            sampLimitId.text = board.getSampleLimit()
            resolutionId.stateIndex = board.getResolution() - 1
            attenuatorId.stateIndex = board.getAttenuator() - 1
            calibId.stateIndex = board.getCalibration()
            channelsId.stateIndex = board.getChannels() - 1
            dataId.stateIndex = board.getDataFormat()
            saveRawVoltId.stateIndex = board.getSaveMode() - 1
            couplingId.stateIndex = board.getCoupling() - 1
        }
    }

    Row{
        width: parent.width
        height: parent.height / 3.0
        Item{
            height: parent.height
            width: parent.width / 4.0
            Item {
                anchors.fill: parent
                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor

                Column{
                    anchors.fill: parent
                    Item{
                        width: parent.width
                        height: parent.height * 0.3
                        Text {
                            anchors.fill: parent
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            color: baseTextColor
                            font.pixelSize: height * 0.6
                            font.family: applicationFont.name
                            text: qsTr("MODE")
                        }
                    }

                    Item{
                        width: parent.width
                        height: parent.height * 0.7
                        MultiSwitch{
                            id:saveTypeId
                            anchors.fill: parent
                            anchors.leftMargin:  10 * mainVisibleRootWindowId.scaleFactor
                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                            stateIndex: board.getSaveType()
                            buttonNames: ["Network","SD card"]
                            inactiveTextColor: baseGrayColor
                            activeTextColor: "#303030"
                            buttonColor: baseRedSwitchColor
                            fontFamaly: applicationFont.name
                            radiusVal:0
                            maxTextHeight: 14 * mainVisibleRootWindowId.scaleFactor
                            onClickIndex: function(i){
                                board.setSaveType(i)
                            }
                        }

                    }
                }
            }
        }
        Item{
            height: parent.height
            width: parent.width / 4.0
            Item {
                anchors.fill: parent
                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                Column{
                    anchors.fill: parent
                    Item{
                        width: parent.width
                        height: parent.height * 0.3
                        Text {
                            anchors.fill: parent
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            color: baseTextColor
                            font.pixelSize: height * 0.6
                            font.family: applicationFont.name
                            text: qsTr("PROTOCOL")
                        }
                    }

                    Item{
                        width: parent.width
                        height: parent.height * 0.7
                        MultiSwitch{
                            id:protocolId
                            anchors.fill: parent
                            anchors.leftMargin:  10 * mainVisibleRootWindowId.scaleFactor
                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                            stateIndex: board.getProtocol()
                            buttonNames: ["TCP","UDP"]
                            inactiveTextColor: baseGrayColor
                            activeTextColor: "#303030"
                            buttonColor: baseRedSwitchColor
                            fontFamaly: applicationFont.name
                            radiusVal:0
                            maxTextHeight: 14 * mainVisibleRootWindowId.scaleFactor
                            onClickIndex: function(i){
                                board.setProtocol(i)
                            }
                        }
                    }
                }
            }
        }
        Item{
            height: parent.height
            width: parent.width / 4.0
            Item {
                anchors.fill: parent
                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                Column{
                    anchors.fill: parent
                    Item{
                        width: parent.width
                        height: parent.height * 0.3
                        Text {
                            anchors.fill: parent
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            color: baseTextColor
                            font.pixelSize: height * 0.6
                            font.family: applicationFont.name
                            text: qsTr("DECIMATION")
                        }
                    }

                    Item{
                        width: parent.width
                        height: parent.height * 0.7
                        TextField{
                            id:decId
                            anchors.fill: parent
                            anchors.leftMargin:  10 * mainVisibleRootWindowId.scaleFactor
                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                            color:baseTextColor
                            background: Rectangle{
                                color:baseBackGroundColor
                                border.width: 1
                                border.color: baseGrayColor
                            }

                            text: board.getDecimation()
                            font.family: applicationFont.name
                            font.pixelSize:  parent.height * 0.5
                            validator : RegExpValidator { regExp : /[0-9]{6}/ }
                            onTextEdited: {
                                board.setDecimation(parseInt(text))
                            }
                        }
                    }
                }
            }
        }
        Item{
            height: parent.height
            width: parent.width / 4.0
            Item {
                anchors.fill: parent
                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                Column{
                    anchors.fill: parent
                    Item{
                        width: parent.width
                        height: parent.height * 0.3
                        Text {
                            anchors.fill: parent
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            color: baseTextColor
                            font.pixelSize: height * 0.6
                            font.family: applicationFont.name
                            text: qsTr("LIMIT SAMPLE")
                        }
                    }

                    Item{
                        width: parent.width
                        height: parent.height * 0.7
                        TextField{
                            id:sampLimitId
                            anchors.fill: parent
                            anchors.leftMargin:  10 * mainVisibleRootWindowId.scaleFactor
                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                            text: board.getSampleLimit()
                            color: baseTextColor
                            background: Rectangle{
                                color:baseBackGroundColor
                                border.width: 1
                                border.color: baseGrayColor
                            }
                            font.family: applicationFont.name
                            font.pixelSize:  parent.height * 0.5
                            validator :RegExpValidator { regExp : /[0-9]{7}/ }
                            onTextEdited: {
                                if (text === ""){
                                    board.setSampleLimit(-1)
                                }else{
                                    board.setSampleLimit(parseInt(text))
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Row{
        width: parent.width
        height: parent.height / 3.0
        Item{
            height: parent.height
            width: parent.width / 4.0
            Item {
                anchors.fill: parent
                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor

                Column{
                    anchors.fill: parent
                    Item{
                        width: parent.width
                        height: parent.height * 0.3
                        Text {
                            anchors.fill: parent
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            color: baseTextColor
                            font.pixelSize: height * 0.6
                            font.family: applicationFont.name
                            text: qsTr("RESOLUTION")
                        }
                    }

                    Item{
                        width: parent.width
                        height: parent.height * 0.7
                        MultiSwitch{
                            id:resolutionId
                            anchors.fill: parent
                            anchors.leftMargin:  10 * mainVisibleRootWindowId.scaleFactor
                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                            stateIndex: board.getResolution() - 1
                            buttonNames: ["8 bit","16 bit"]
                            inactiveTextColor: baseGrayColor
                            activeTextColor: "#303030"
                            buttonColor: baseRedSwitchColor
                            fontFamaly: applicationFont.name
                            radiusVal:0
                            maxTextHeight: 14 * mainVisibleRootWindowId.scaleFactor
                            onClickIndex: function(i){
                                board.setResolution(i + 1)
                            }
                        }

                    }
                }
            }
        }
        Item{
            height: parent.height
            width: parent.width / 4.0
            Item {
                anchors.fill: parent
                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                Column{
                    anchors.fill: parent
                    Item{
                        width: parent.width
                        height: parent.height * 0.3
                        Text {
                            anchors.fill: parent
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            color: baseTextColor
                            font.pixelSize: height * 0.6
                            font.family: applicationFont.name
                            text: qsTr("ATTENUATOR")
                        }
                    }

                    Item{
                        width: parent.width
                        height: parent.height * 0.7
                        MultiSwitch{
                            id:attenuatorId
                            anchors.fill: parent
                            anchors.leftMargin:  10 * mainVisibleRootWindowId.scaleFactor
                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                            stateIndex: board.getAttenuator() - 1
                            buttonNames: ["1:1","1:20"]
                            inactiveTextColor: baseGrayColor
                            activeTextColor: "#303030"
                            buttonColor: baseRedSwitchColor
                            fontFamaly: applicationFont.name
                            radiusVal:0
                            maxTextHeight: 14 * mainVisibleRootWindowId.scaleFactor
                            onClickIndex: function(i){
                                board.setAttenuator(i + 1)
                            }
                        }
                    }
                }
            }
        }
        Item{
            height: parent.height
            width: parent.width / 4.0
            Item {
                anchors.fill: parent
                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                Column{
                    anchors.fill: parent
                    Item{
                        width: parent.width
                        height: parent.height * 0.3
                        Text {
                            anchors.fill: parent
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            color: baseTextColor
                            font.pixelSize: height * 0.6
                            font.family: applicationFont.name
                            text: qsTr("CALIBRATION")
                        }
                    }

                    Item{
                        width: parent.width
                        height: parent.height * 0.7
                        MultiSwitch{
                            id:calibId
                            anchors.fill: parent
                            anchors.leftMargin:  10 * mainVisibleRootWindowId.scaleFactor
                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                            stateIndex: board.getCalibration()
                            buttonNames: ["No","Yes"]
                            inactiveTextColor: baseGrayColor
                            activeTextColor: "#303030"
                            buttonColor: baseRedSwitchColor
                            fontFamaly: applicationFont.name
                            radiusVal:0
                            maxTextHeight: 14 * mainVisibleRootWindowId.scaleFactor
                            onClickIndex: function(i){
                                board.setCalibration(i)
                            }
                        }
                    }
                }
            }
        }
        Item{
            height: parent.height
            width: parent.width / 4.0
            Item {
                anchors.fill: parent
                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                Column{
                    anchors.fill: parent
                    Item{
                        width: parent.width
                        height: parent.height * 0.3
                        Text {
                            anchors.fill: parent
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            color: baseTextColor
                            font.pixelSize: height * 0.6
                            font.family: applicationFont.name
                            text: qsTr("CHANNELS")
                        }
                    }

                    Item{
                        width: parent.width
                        height: parent.height * 0.7
                        MultiSwitch{
                            id:channelsId
                            anchors.fill: parent
                            anchors.leftMargin:  10 * mainVisibleRootWindowId.scaleFactor
                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                            stateIndex: board.getChannels() - 1
                            buttonNames: ["1 ch","2 ch","Both"]
                            inactiveTextColor: baseGrayColor
                            activeTextColor: "#303030"
                            buttonColor: baseRedSwitchColor
                            fontFamaly: applicationFont.name
                            radiusVal:0
                            maxTextHeight: 14 * mainVisibleRootWindowId.scaleFactor
                            onClickIndex: function(i){
                                board.setChannels(i + 1)
                            }
                        }
                    }
                }
            }
        }
    }

    Row{
        width: parent.width
        height: parent.height / 3.0
        Item{
            height: parent.height
            width: parent.width / 4.0
            Item {
                anchors.fill: parent
                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor

                Column{
                    anchors.fill: parent
                    Item{
                        width: parent.width
                        height: parent.height * 0.3
                        Text {
                            anchors.fill: parent
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            color: baseTextColor
                            font.pixelSize: height * 0.6
                            font.family: applicationFont.name
                            text: qsTr("TYPE")
                        }
                    }

                    Item{
                        width: parent.width
                        height: parent.height * 0.7
                        MultiSwitch{
                            id:dataId
                            anchors.fill: parent
                            anchors.leftMargin:  10 * mainVisibleRootWindowId.scaleFactor
                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                            stateIndex: board.getDataFormat()
                            buttonNames: ["WAV","TDMS","BIN"]
                            inactiveTextColor: baseGrayColor
                            activeTextColor: "#303030"
                            buttonColor: baseRedSwitchColor
                            fontFamaly: applicationFont.name
                            radiusVal:0
                            maxTextHeight: 14 * mainVisibleRootWindowId.scaleFactor
                            onClickIndex: function(i){
                                board.setDataFormat(i)
                            }
                        }

                    }
                }
            }
        }
        Item{
            height: parent.height
            width: parent.width / 4.0
            Item {
                anchors.fill: parent
                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                Column{
                    anchors.fill: parent
                    Item{
                        width: parent.width
                        height: parent.height * 0.3
                        Text {
                            anchors.fill: parent
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            color: baseTextColor
                            font.pixelSize: height * 0.6
                            font.family: applicationFont.name
                            text: qsTr("SAVE MODE")
                        }
                    }

                    Item{
                        width: parent.width
                        height: parent.height * 0.7
                        MultiSwitch{
                            id:saveRawVoltId
                            anchors.fill: parent
                            anchors.leftMargin:  10 * mainVisibleRootWindowId.scaleFactor
                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                            stateIndex: board.getSaveMode() - 1
                            buttonNames: ["RAW","VOLT"]
                            inactiveTextColor: baseGrayColor
                            activeTextColor: "#303030"
                            buttonColor: baseRedSwitchColor
                            fontFamaly: applicationFont.name
                            radiusVal:0
                            maxTextHeight: 14 * mainVisibleRootWindowId.scaleFactor
                            onClickIndex: function(i){
                                board.setSaveMode(i + 1)
                            }
                        }
                    }
                }
            }
        }
        Item{
            height: parent.height
            width: parent.width / 4.0
            visible: board.getCouplingVisible()
            Item {
                anchors.fill: parent
                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                Column{
                    anchors.fill: parent
                    Item{
                        width: parent.width
                        height: parent.height * 0.3
                        Text {
                            anchors.fill: parent
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            color: baseTextColor
                            font.pixelSize: height * 0.6
                            font.family: applicationFont.name
                            text: qsTr("COUPLING")
                        }
                    }

                    Item{
                        width: parent.width
                        height: parent.height * 0.7
                        MultiSwitch{
                            id:couplingId
                            anchors.fill: parent
                            anchors.leftMargin:  10 * mainVisibleRootWindowId.scaleFactor
                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                            stateIndex: board.getCoupling() - 1
                            buttonNames: ["AC","DC"]
                            inactiveTextColor: baseGrayColor
                            activeTextColor: "#303030"
                            buttonColor: baseRedSwitchColor
                            fontFamaly: applicationFont.name
                            radiusVal:0
                            maxTextHeight: 14 * mainVisibleRootWindowId.scaleFactor
                            onClickIndex: function(i){
                                board.setCoupling(i + 1)
                            }
                        }
                    }
                }
            }
        }

        Item{
            height: parent.height
            width: parent.width / 4.0
            Item {
                anchors.fill: parent
                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                Column{
                    anchors.fill: parent
                    Item{
                        width: parent.width
                        height: parent.height * 0.3
                        Text {
                            anchors.fill: parent
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            color: baseTextColor
                            font.pixelSize: height * 0.6
                            font.family: applicationFont.name
                            text: qsTr("TEST MODE")
                        }
                    }

                    Item{
                        width: parent.width
                        height: parent.height * 0.7
                        MultiSwitch{
                            id:testModeId
                            anchors.fill: parent
                            anchors.leftMargin:  10 * mainVisibleRootWindowId.scaleFactor
                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                            stateIndex: board.getTestMode()
                            buttonNames: ["OFF","ON"]
                            inactiveTextColor: baseGrayColor
                            activeTextColor: "#303030"
                            buttonColor: baseRedSwitchColor
                            fontFamaly: applicationFont.name
                            radiusVal:0
                            maxTextHeight: 14 * mainVisibleRootWindowId.scaleFactor
                            onClickIndex: function(i){
                                board.setTestMode(i)
                            }
                        }
                    }
                }
            }
        }
    }
}
