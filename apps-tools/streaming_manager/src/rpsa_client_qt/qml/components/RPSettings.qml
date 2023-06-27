import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.5

import "../base_components"

Item {

    Connections {
        target: board
        function onGetNewSettingSignal() {
            saveTypeId.stateIndex = board.getSaveType()
            protocolId.stateIndex = board.getProtocol()
            decId.text = board.getDecimation()
            sampLimitId.text = board.getSampleLimit()
            resolutionId.stateIndex = board.getResolution()
            calibId.stateIndex = board.getCalibration()

            channelsEn1Id.stateIndex = (board.getChannels() & 0x1) > 0
            channelsEn2Id.stateIndex = (board.getChannels() & 0x2) > 0
            channelsEn3Id.stateIndex = (board.getChannels() & 0x4) > 0
            channelsEn4Id.stateIndex = (board.getChannels() & 0x8) > 0


            channelsAtt1Id.stateIndex = (board.getAttenuator() & 0x1) > 0
            channelsAtt2Id.stateIndex = (board.getAttenuator() & 0x2) > 0
            channelsAtt3Id.stateIndex = (board.getAttenuator() & 0x4) > 0
            channelsAtt4Id.stateIndex = (board.getAttenuator() & 0x8) > 0

            channelsCup1Id.stateIndex = (board.getCoupling() & 0x1) > 0
            channelsCup2Id.stateIndex = (board.getCoupling() & 0x2) > 0
            channelsCup3Id.stateIndex = (board.getCoupling() & 0x4) > 0
            channelsCup4Id.stateIndex = (board.getCoupling() & 0x8) > 0

            dataId.stateIndex = board.getDataFormat()
            saveRawVoltId.stateIndex = board.getSaveMode()
        }
    }

    property real settingSize: board.maxChannels === 2 ? 0.6 : 0.5
    property real channelSize: board.maxChannels === 2 ? 0.4 : 0.5
    property real headerTextSize: 0.8

    anchors.fill: parent
    anchors.margins: 5 * mainVisibleRootWindowId.scaleFactor
    Item {
        id: rootSettinsId
        anchors.fill: parent
        Row {
            anchors.fill: parent
            Column {
                height: rootSettinsId.height
                width: rootSettinsId.width * settingSize

                Row {
                    width: parent.width
                    height: parent.height / 4.0
                    Item {
                        height: parent.height
                        width: parent.width / 3.0
                        Item {
                            anchors.fill: parent
                            anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor

                            Column {
                                anchors.fill: parent
                                Item {
                                    width: parent.width
                                    height: parent.height * 0.3
                                    Text {
                                        anchors.fill: parent
                                        verticalAlignment: Text.AlignVCenter
                                        horizontalAlignment: Text.AlignHCenter
                                        color: baseTextColor
                                        font.pixelSize: height * headerTextSize
                                        font.family: applicationFont.name
                                        text: qsTr("MODE")
                                    }
                                }

                                Item {
                                    width: parent.width
                                    height: parent.height * 0.7
                                    MultiSwitch {
                                        id: saveTypeId
                                        anchors.fill: parent
                                        anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                        anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                        stateIndex: board.getSaveType()
                                        buttonNames: ["Network", "SD card"]
                                        inactiveTextColor: baseGrayColor
                                        activeTextColor: "#303030"
                                        buttonColor: baseRedSwitchColor
                                        fontFamaly: applicationFont.name
                                        radiusVal: 0
                                        maxTextHeight: height * 0.6
                                        onClickIndex: function (i) {
                                            board.setSaveType(i)
                                        }
                                    }
                                }
                            }
                        }
                    }
                    Item {
                        height: parent.height
                        width: parent.width / 3.0
                        Item {
                            anchors.fill: parent
                            anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                            Column {
                                anchors.fill: parent
                                Item {
                                    width: parent.width
                                    height: parent.height * 0.3
                                    Text {
                                        anchors.fill: parent
                                        verticalAlignment: Text.AlignVCenter
                                        horizontalAlignment: Text.AlignHCenter
                                        color: baseTextColor
                                        font.pixelSize: height * headerTextSize
                                        font.family: applicationFont.name
                                        text: qsTr("PROTOCOL")
                                    }
                                }

                                Item {
                                    width: parent.width
                                    height: parent.height * 0.7
                                    MultiSwitch {
                                        id: protocolId
                                        anchors.fill: parent
                                        anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                        anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                        stateIndex: board.getProtocol()
                                        buttonNames: ["TCP", "UDP"]
                                        inactiveTextColor: baseGrayColor
                                        activeTextColor: "#303030"
                                        buttonColor: baseRedSwitchColor
                                        fontFamaly: applicationFont.name
                                        radiusVal: 0
                                        maxTextHeight: height * 0.5
                                        onClickIndex: function (i) {
                                            board.setProtocol(i)
                                        }
                                    }
                                }
                            }
                        }
                    }
                    Item {
                        height: parent.height
                        width: parent.width / 3.0
                        Item {
                            anchors.fill: parent
                            anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                            Column {
                                anchors.fill: parent
                                Item {
                                    width: parent.width
                                    height: parent.height * 0.3
                                    Text {
                                        anchors.fill: parent
                                        verticalAlignment: Text.AlignVCenter
                                        horizontalAlignment: Text.AlignHCenter
                                        color: baseTextColor
                                        font.pixelSize: height * headerTextSize
                                        font.family: applicationFont.name
                                        text: qsTr("DECIMATION")
                                    }
                                }

                                Item {
                                    width: parent.width
                                    height: parent.height * 0.7
                                    TextField {
                                        id: decId
                                        anchors.fill: parent
                                        anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                        anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                        color: baseTextColor
                                        background: Rectangle {
                                            color: baseBackGroundColor
                                            border.width: 1
                                            border.color: baseGrayColor
                                        }

                                        text: board.getDecimation()
                                        font.family: applicationFont.name
                                        font.pixelSize: parent.height * 0.5
                                        validator: RegExpValidator {
                                            regExp: /[0-9]{6}/
                                        }
                                        onTextEdited: {
                                            board.setDecimation(parseInt(text))
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Row {
                    width: parent.width
                    height: parent.height / 4.0
                    Item {
                        height: parent.height
                        width: parent.width / 3.0
                        Item {
                            anchors.fill: parent
                            anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                            Column {
                                anchors.fill: parent
                                Item {
                                    width: parent.width
                                    height: parent.height * 0.3
                                    Text {
                                        anchors.fill: parent
                                        verticalAlignment: Text.AlignVCenter
                                        horizontalAlignment: Text.AlignHCenter
                                        color: baseTextColor
                                        font.pixelSize: height * headerTextSize
                                        font.family: applicationFont.name
                                        text: qsTr("LIMIT SAMPLE")
                                    }
                                }

                                Item {
                                    width: parent.width
                                    height: parent.height * 0.7
                                    TextField {
                                        id: sampLimitId
                                        anchors.fill: parent
                                        anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                        anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                        text: board.getSampleLimit()
                                        color: baseTextColor
                                        background: Rectangle {
                                            color: baseBackGroundColor
                                            border.width: 1
                                            border.color: baseGrayColor
                                        }
                                        font.family: applicationFont.name
                                        font.pixelSize: parent.height * 0.5
                                        validator: RegExpValidator {
                                            regExp: /[0-9]{7}/
                                        }
                                        onTextEdited: {
                                            if (text === "") {
                                                board.setSampleLimit(-1)
                                            } else {
                                                board.setSampleLimit(
                                                            parseInt(text))
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    Item {
                        height: parent.height
                        width: parent.width / 3.0
                        Item {
                            anchors.fill: parent
                            anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor

                            Column {
                                anchors.fill: parent
                                Item {
                                    width: parent.width
                                    height: parent.height * 0.3
                                    Text {
                                        anchors.fill: parent
                                        verticalAlignment: Text.AlignVCenter
                                        horizontalAlignment: Text.AlignHCenter
                                        color: baseTextColor
                                        font.pixelSize: height * headerTextSize
                                        font.family: applicationFont.name
                                        text: qsTr("RESOLUTION")
                                    }
                                }

                                Item {
                                    width: parent.width
                                    height: parent.height * 0.7
                                    MultiSwitch {
                                        id: resolutionId
                                        anchors.fill: parent
                                        anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                        anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                        stateIndex: board.getResolution()
                                        buttonNames: ["8 bit", "16 bit"]
                                        inactiveTextColor: baseGrayColor
                                        activeTextColor: "#303030"
                                        buttonColor: baseRedSwitchColor
                                        fontFamaly: applicationFont.name
                                        radiusVal: 0
                                        maxTextHeight: parent.height * 0.5
                                        onClickIndex: function (i) {
                                            board.setResolution(i)
                                        }
                                    }
                                }
                            }
                        }
                    }
                    Item {
                        height: parent.height
                        width: parent.width / 3.0
                        Item {
                            anchors.fill: parent
                            anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor

                            Column {
                                anchors.fill: parent
                                Item {
                                    width: parent.width
                                    height: parent.height * 0.3
                                    Text {
                                        anchors.fill: parent
                                        verticalAlignment: Text.AlignVCenter
                                        horizontalAlignment: Text.AlignHCenter
                                        color: baseTextColor
                                        font.pixelSize: height * headerTextSize
                                        font.family: applicationFont.name
                                        text: qsTr("TYPE")
                                    }
                                }

                                Item {
                                    width: parent.width
                                    height: parent.height * 0.7
                                    MultiSwitch {
                                        id: dataId
                                        anchors.fill: parent
                                        anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                        anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                        stateIndex: board.getDataFormat()
                                        buttonNames: ["WAV", "TDMS", "BIN"]
                                        inactiveTextColor: baseGrayColor
                                        activeTextColor: "#303030"
                                        buttonColor: baseRedSwitchColor
                                        fontFamaly: applicationFont.name
                                        radiusVal: 0
                                        maxTextHeight: parent.height * 0.5
                                        onClickIndex: function (i) {
                                            board.setDataFormat(i)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Row {
                    width: parent.width
                    height: parent.height / 4.0

                    Item {
                        height: parent.height
                        width: parent.width / 3.0
                        Item {
                            anchors.fill: parent
                            anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                            Column {
                                anchors.fill: parent
                                Item {
                                    width: parent.width
                                    height: parent.height * 0.3
                                    Text {
                                        anchors.fill: parent
                                        verticalAlignment: Text.AlignVCenter
                                        horizontalAlignment: Text.AlignHCenter
                                        color: baseTextColor
                                        font.pixelSize: height * headerTextSize
                                        font.family: applicationFont.name
                                        text: qsTr("CALIBRATION")
                                    }
                                }

                                Item {
                                    width: parent.width
                                    height: parent.height * 0.7
                                    MultiSwitch {
                                        id: calibId
                                        anchors.fill: parent
                                        anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                        anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                        stateIndex: board.getCalibration()
                                        buttonNames: ["No", "Yes"]
                                        inactiveTextColor: baseGrayColor
                                        activeTextColor: "#303030"
                                        buttonColor: baseRedSwitchColor
                                        fontFamaly: applicationFont.name
                                        radiusVal: 0
                                        maxTextHeight: parent.height * 0.5
                                        onClickIndex: function (i) {
                                            board.setCalibration(i)
                                        }
                                    }
                                }
                            }
                        }
                    }
                    Item {
                        height: parent.height
                        width: parent.width / 3.0
                        Item {
                            anchors.fill: parent
                            anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                            Column {
                                anchors.fill: parent
                                Item {
                                    width: parent.width
                                    height: parent.height * 0.3
                                    Text {
                                        anchors.fill: parent
                                        verticalAlignment: Text.AlignVCenter
                                        horizontalAlignment: Text.AlignHCenter
                                        color: baseTextColor
                                        font.pixelSize: height * headerTextSize
                                        font.family: applicationFont.name
                                        text: qsTr("SAVE MODE")
                                    }
                                }

                                Item {
                                    width: parent.width
                                    height: parent.height * 0.7
                                    MultiSwitch {
                                        id: saveRawVoltId
                                        anchors.fill: parent
                                        anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                        anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                        stateIndex: board.getSaveMode()
                                        buttonNames: ["RAW", "VOLT"]
                                        inactiveTextColor: baseGrayColor
                                        activeTextColor: "#303030"
                                        buttonColor: baseRedSwitchColor
                                        fontFamaly: applicationFont.name
                                        radiusVal: 0
                                        maxTextHeight: parent.height * 0.5
                                        onClickIndex: function (i) {
                                            board.setSaveMode(i)
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Item {
                        height: parent.height
                        width: parent.width / 3.0
                        Item {
                            anchors.fill: parent
                            anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                            Column {
                                anchors.fill: parent
                                Item {
                                    width: parent.width
                                    height: parent.height * 0.3
                                    Text {
                                        anchors.fill: parent
                                        verticalAlignment: Text.AlignVCenter
                                        horizontalAlignment: Text.AlignHCenter
                                        color: baseTextColor
                                        font.pixelSize: height * headerTextSize
                                        font.family: applicationFont.name
                                        text: qsTr("TEST MODE")
                                    }
                                }

                                Item {
                                    width: parent.width
                                    height: parent.height * 0.7
                                    MultiSwitch {
                                        id: testModeId
                                        anchors.fill: parent
                                        anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                        anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                        stateIndex: board.getTestMode()
                                        buttonNames: ["OFF", "ON"]
                                        inactiveTextColor: baseGrayColor
                                        activeTextColor: "#303030"
                                        buttonColor: baseRedSwitchColor
                                        fontFamaly: applicationFont.name
                                        radiusVal: 0
                                        maxTextHeight: parent.height * 0.5
                                        onClickIndex: function (i) {
                                            board.setTestMode(i)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Item {
                height: rootSettinsId.height
                width: rootSettinsId.width * channelSize
                Column {
                    anchors.fill: parent

                    Row {
                        width: parent.width
                        height: parent.height / 4.0
                        Item {
                            visible: board.maxChannels >= 1
                            height: parent.height
                            width: parent.width / board.maxChannels
                            Item {
                                anchors.fill: parent
                                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                                Column {
                                    anchors.fill: parent
                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.3
                                        Text {
                                            anchors.fill: parent
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment: Text.AlignHCenter
                                            color: baseTextColor
                                            font.pixelSize: height * headerTextSize
                                            font.family: applicationFont.name
                                            text: qsTr("CH1")
                                        }
                                    }

                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.7
                                        MultiSwitch {
                                            id: channelsEn1Id
                                            anchors.fill: parent
                                            anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            stateIndex: (board.getChannels(
                                                            ) & 0x1) > 0
                                            buttonNames: ["Off", "On"]
                                            inactiveTextColor: baseGrayColor
                                            activeTextColor: "#303030"
                                            buttonColor: baseRedSwitchColor
                                            fontFamaly: applicationFont.name
                                            radiusVal: 0
                                            maxTextHeight: parent.height * 0.5
                                            onClickIndex: function (i) {
                                                var curValue = board.getChannels()
                                                board.setChannels(
                                                            (curValue & 0xE) | (i ? 0x1 : 0x0))
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        Item {
                            visible: board.maxChannels >= 2
                            height: parent.height
                            width: parent.width / board.maxChannels
                            Item {
                                anchors.fill: parent
                                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                                Column {
                                    anchors.fill: parent
                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.3
                                        Text {
                                            anchors.fill: parent
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment: Text.AlignHCenter
                                            color: baseTextColor
                                            font.pixelSize: height * headerTextSize
                                            font.family: applicationFont.name
                                            text: qsTr("CH2")
                                        }
                                    }

                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.7
                                        MultiSwitch {
                                            id: channelsEn2Id
                                            anchors.fill: parent
                                            anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            stateIndex: (board.getChannels(
                                                            ) & 0x2) > 0
                                            buttonNames: ["Off", "On"]
                                            inactiveTextColor: baseGrayColor
                                            activeTextColor: "#303030"
                                            buttonColor: baseRedSwitchColor
                                            fontFamaly: applicationFont.name
                                            radiusVal: 0
                                            maxTextHeight: parent.height * 0.5
                                            onClickIndex: function (i) {
                                                var curValue = board.getChannels()
                                                board.setChannels(
                                                            (curValue & 0xD) | (i ? 0x2 : 0x0))
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        Item {
                            visible: board.maxChannels >= 3
                            height: parent.height
                            width: parent.width / board.maxChannels
                            Item {
                                anchors.fill: parent
                                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                                Column {
                                    anchors.fill: parent
                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.3
                                        Text {
                                            anchors.fill: parent
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment: Text.AlignHCenter
                                            color: baseTextColor
                                            font.pixelSize: height * headerTextSize
                                            font.family: applicationFont.name
                                            text: qsTr("CH3")
                                        }
                                    }

                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.7
                                        MultiSwitch {
                                            id: channelsEn3Id
                                            anchors.fill: parent
                                            anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            stateIndex: (board.getChannels(
                                                            ) & 0x4) > 0
                                            buttonNames: ["Off", "On"]
                                            inactiveTextColor: baseGrayColor
                                            activeTextColor: "#303030"
                                            buttonColor: baseRedSwitchColor
                                            fontFamaly: applicationFont.name
                                            radiusVal: 0
                                            maxTextHeight: parent.height * 0.5
                                            onClickIndex: function (i) {
                                                var curValue = board.getChannels()
                                                board.setChannels(
                                                            (curValue & 0xB) | (i ? 0x4 : 0x0))
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        Item {
                            visible: board.maxChannels >= 4
                            height: parent.height
                            width: parent.width / board.maxChannels
                            Item {
                                anchors.fill: parent
                                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                                Column {
                                    anchors.fill: parent
                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.3
                                        Text {
                                            anchors.fill: parent
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment: Text.AlignHCenter
                                            color: baseTextColor
                                            font.pixelSize: height * headerTextSize
                                            font.family: applicationFont.name
                                            text: qsTr("CH4")
                                        }
                                    }

                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.7
                                        MultiSwitch {
                                            id: channelsEn4Id
                                            anchors.fill: parent
                                            anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            stateIndex: (board.getChannels(
                                                            ) & 0x8) > 0
                                            buttonNames: ["Off", "On"]
                                            inactiveTextColor: baseGrayColor
                                            activeTextColor: "#303030"
                                            buttonColor: baseRedSwitchColor
                                            fontFamaly: applicationFont.name
                                            radiusVal: 0
                                            maxTextHeight: parent.height * 0.5
                                            onClickIndex: function (i) {
                                                var curValue = board.getChannels()
                                                board.setChannels(
                                                            (curValue & 0x7) | (i ? 0x8 : 0x0))
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Row {
                        width: parent.width
                        height: parent.height / 4.0
                        visible: board.isACDC
                        Item {
                            visible: board.maxChannels >= 1
                            height: parent.height
                            width: parent.width / board.maxChannels
                            Item {
                                anchors.fill: parent
                                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                                Column {
                                    anchors.fill: parent
                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.3
                                        Text {
                                            anchors.fill: parent
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment: Text.AlignHCenter
                                            color: baseTextColor
                                            font.pixelSize: height * headerTextSize
                                            font.family: applicationFont.name
                                            text: ""
                                        }
                                    }

                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.7
                                        MultiSwitch {
                                            id: channelsAtt1Id
                                            anchors.fill: parent
                                            anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            stateIndex: (board.getAttenuator(
                                                            ) & 0x1) > 0
                                            buttonNames: ["1:1", "1:20"]
                                            inactiveTextColor: baseGrayColor
                                            activeTextColor: "#303030"
                                            buttonColor: baseRedSwitchColor
                                            fontFamaly: applicationFont.name
                                            radiusVal: 0
                                            maxTextHeight: parent.height * 0.5
                                            onClickIndex: function (i) {
                                                var curValue = board.getAttenuator()
                                                board.setAttenuator(
                                                            (curValue & 0xE) | (i ? 0x1 : 0x0))
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        Item {
                            visible: board.maxChannels >= 2
                            height: parent.height
                            width: parent.width / board.maxChannels
                            Item {
                                anchors.fill: parent
                                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                                Column {
                                    anchors.fill: parent
                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.3
                                        Text {
                                            anchors.fill: parent
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment: Text.AlignHCenter
                                            color: baseTextColor
                                            font.pixelSize: height * headerTextSize
                                            font.family: applicationFont.name
                                            text: ""
                                        }
                                    }

                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.7
                                        MultiSwitch {
                                            id: channelsAtt2Id
                                            anchors.fill: parent
                                            anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            stateIndex: (board.getAttenuator(
                                                            ) & 0x2) > 0
                                            buttonNames: ["1:1", "1:20"]
                                            inactiveTextColor: baseGrayColor
                                            activeTextColor: "#303030"
                                            buttonColor: baseRedSwitchColor
                                            fontFamaly: applicationFont.name
                                            radiusVal: 0
                                            maxTextHeight: parent.height * 0.5
                                            onClickIndex: function (i) {
                                                var curValue = board.getAttenuator()
                                                board.setAttenuator(
                                                            (curValue & 0xD) | (i ? 0x2 : 0x0))
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        Item {
                            visible: board.maxChannels >= 3
                            height: parent.height
                            width: parent.width / board.maxChannels
                            Item {
                                anchors.fill: parent
                                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                                Column {
                                    anchors.fill: parent
                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.3
                                        Text {
                                            anchors.fill: parent
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment: Text.AlignHCenter
                                            color: baseTextColor
                                            font.pixelSize: height * headerTextSize
                                            font.family: applicationFont.name
                                            text: ""
                                        }
                                    }

                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.7
                                        MultiSwitch {
                                            id: channelsAtt3Id
                                            anchors.fill: parent
                                            anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            stateIndex: (board.getAttenuator(
                                                            ) & 0x4) > 0
                                            buttonNames: ["1:1", "1:20"]
                                            inactiveTextColor: baseGrayColor
                                            activeTextColor: "#303030"
                                            buttonColor: baseRedSwitchColor
                                            fontFamaly: applicationFont.name
                                            radiusVal: 0
                                            maxTextHeight: parent.height * 0.5
                                            onClickIndex: function (i) {
                                                var curValue = board.getAttenuator()
                                                board.setAttenuator(
                                                            (curValue & 0xB) | (i ? 0x4 : 0x0))
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        Item {
                            visible: board.maxChannels >= 4
                            height: parent.height
                            width: parent.width / board.maxChannels
                            Item {
                                anchors.fill: parent
                                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                                Column {
                                    anchors.fill: parent
                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.3
                                        Text {
                                            anchors.fill: parent
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment: Text.AlignHCenter
                                            color: baseTextColor
                                            font.pixelSize: height * headerTextSize
                                            font.family: applicationFont.name
                                            text: ""
                                        }
                                    }

                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.7
                                        MultiSwitch {
                                            id: channelsAtt4Id
                                            anchors.fill: parent
                                            anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            stateIndex: (board.getAttenuator(
                                                            ) & 0x8) > 0
                                            buttonNames: ["1:1", "1:20"]
                                            inactiveTextColor: baseGrayColor
                                            activeTextColor: "#303030"
                                            buttonColor: baseRedSwitchColor
                                            fontFamaly: applicationFont.name
                                            radiusVal: 0
                                            maxTextHeight: parent.height * 0.5
                                            onClickIndex: function (i) {
                                                var curValue = board.getAttenuator()
                                                board.setAttenuator(
                                                            (curValue & 0x7) | (i ? 0x8 : 0x0))
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Row {
                        width: parent.width
                        height: parent.height / 4.0
                        Item {
                            visible: board.maxChannels >= 1
                            height: parent.height
                            width: parent.width / board.maxChannels
                            Item {
                                anchors.fill: parent
                                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                                Column {
                                    anchors.fill: parent
                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.3
                                        Text {
                                            anchors.fill: parent
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment: Text.AlignHCenter
                                            color: baseTextColor
                                            font.pixelSize: height * headerTextSize
                                            font.family: applicationFont.name
                                            text: ""
                                        }
                                    }

                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.7
                                        MultiSwitch {
                                            id: channelsCup1Id
                                            anchors.fill: parent
                                            anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            stateIndex: (board.getCoupling(
                                                            ) & 0x1) > 0
                                            buttonNames: ["AC", "DC"]
                                            inactiveTextColor: baseGrayColor
                                            activeTextColor: "#303030"
                                            buttonColor: baseRedSwitchColor
                                            fontFamaly: applicationFont.name
                                            radiusVal: 0
                                            maxTextHeight: parent.height * 0.5
                                            onClickIndex: function (i) {
                                                var curValue = board.getCoupling()
                                                board.setCoupling(
                                                            (curValue & 0xE) | (i ? 0x1 : 0x0))
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        Item {
                            visible: board.maxChannels >= 2
                            height: parent.height
                            width: parent.width / board.maxChannels
                            Item {
                                anchors.fill: parent
                                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                                Column {
                                    anchors.fill: parent
                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.3
                                        Text {
                                            anchors.fill: parent
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment: Text.AlignHCenter
                                            color: baseTextColor
                                            font.pixelSize: height * headerTextSize
                                            font.family: applicationFont.name
                                            text: ""
                                        }
                                    }

                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.7
                                        MultiSwitch {
                                            id: channelsCup2Id
                                            anchors.fill: parent
                                            anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            stateIndex: (board.getCoupling(
                                                            ) & 0x2) > 0
                                            buttonNames: ["AC", "DC"]
                                            inactiveTextColor: baseGrayColor
                                            activeTextColor: "#303030"
                                            buttonColor: baseRedSwitchColor
                                            fontFamaly: applicationFont.name
                                            radiusVal: 0
                                            maxTextHeight: parent.height * 0.5
                                            onClickIndex: function (i) {
                                                var curValue = board.getCoupling()
                                                board.setCoupling(
                                                            (curValue & 0xD) | (i ? 0x2 : 0x0))
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        Item {
                            visible: board.maxChannels >= 3
                            height: parent.height
                            width: parent.width / board.maxChannels
                            Item {
                                anchors.fill: parent
                                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                                Column {
                                    anchors.fill: parent
                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.3
                                        Text {
                                            anchors.fill: parent
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment: Text.AlignHCenter
                                            color: baseTextColor
                                            font.pixelSize: height * headerTextSize
                                            font.family: applicationFont.name
                                            text: ""
                                        }
                                    }

                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.7
                                        MultiSwitch {
                                            id: channelsCup3Id
                                            anchors.fill: parent
                                            anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            stateIndex: (board.getCoupling(
                                                            ) & 0x4) > 0
                                            buttonNames: ["AC", "DC"]
                                            inactiveTextColor: baseGrayColor
                                            activeTextColor: "#303030"
                                            buttonColor: baseRedSwitchColor
                                            fontFamaly: applicationFont.name
                                            radiusVal: 0
                                            maxTextHeight: parent.height * 0.5
                                            onClickIndex: function (i) {
                                                var curValue = board.getCoupling()
                                                board.setCoupling(
                                                            (curValue & 0xB) | (i ? 0x4 : 0x0))
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        Item {
                            visible: board.maxChannels >= 4
                            height: parent.height
                            width: parent.width / board.maxChannels
                            Item {
                                anchors.fill: parent
                                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
                                Column {
                                    anchors.fill: parent
                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.3
                                        Text {
                                            anchors.fill: parent
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment: Text.AlignHCenter
                                            color: baseTextColor
                                            font.pixelSize: height * headerTextSize
                                            font.family: applicationFont.name
                                            text: ""
                                        }
                                    }

                                    Item {
                                        width: parent.width
                                        height: parent.height * 0.7
                                        MultiSwitch {
                                            id: channelsCup4Id
                                            anchors.fill: parent
                                            anchors.leftMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
                                            stateIndex: (board.getCoupling(
                                                            ) & 0x8) > 0
                                            buttonNames: ["AC", "DC"]
                                            inactiveTextColor: baseGrayColor
                                            activeTextColor: "#303030"
                                            buttonColor: baseRedSwitchColor
                                            fontFamaly: applicationFont.name
                                            radiusVal: 0
                                            maxTextHeight: parent.height * 0.5
                                            onClickIndex: function (i) {
                                                var curValue = board.getCoupling()
                                                board.setCoupling(
                                                            (curValue & 0x7) | (i ? 0x8 : 0x0))
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}//Column{

//    Row{
//        width: parent.width
//        height: parent.height / 3.0

//    }

//    Row{
//        width: parent.width
//        height: parent.height / 3.0
//        Item{
//            height: parent.height
//            width: parent.width / 4.0
//            Item {
//                anchors.fill: parent
//                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
//                Column{
//                    anchors.fill: parent
//                    Item{
//                        width: parent.width
//                        height: parent.height * 0.3
//                        Text {
//                            anchors.fill: parent
//                            verticalAlignment: Text.AlignVCenter
//                            horizontalAlignment: Text.AlignHCenter
//                            color: baseTextColor
//                            font.pixelSize: height * 0.6
//                            font.family: applicationFont.name
//                            text: qsTr("ATTENUATOR")
//                        }
//                    }

//                    Item{
//                        width: parent.width
//                        height: parent.height * 0.7
//                        MultiSwitch{
//                            id:attenuatorId
//                            anchors.fill: parent
//                            anchors.leftMargin:  10 * mainVisibleRootWindowId.scaleFactor
//                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
//                            stateIndex: board.getAttenuator() - 1
//                            buttonNames: ["1:1","1:20"]
//                            inactiveTextColor: baseGrayColor
//                            activeTextColor: "#303030"
//                            buttonColor: baseRedSwitchColor
//                            fontFamaly: applicationFont.name
//                            radiusVal:0
//                            maxTextHeight: 14 * mainVisibleRootWindowId.scaleFactor
//                            onClickIndex: function(i){
//                                board.setAttenuator(i + 1)
//                            }
//                        }
//                    }
//                }
//            }

//        }
//        Item{
//            height: parent.height
//            width: parent.width / 4.0
//            Item {
//                anchors.fill: parent
//                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
//                Column{
//                    anchors.fill: parent
//                    Item{
//                        width: parent.width
//                        height: parent.height * 0.3
//                        Text {
//                            anchors.fill: parent
//                            verticalAlignment: Text.AlignVCenter
//                            horizontalAlignment: Text.AlignHCenter
//                            color: baseTextColor
//                            font.pixelSize: height * 0.6
//                            font.family: applicationFont.name
//                            text: qsTr("CHANNELS")
//                        }
//                    }

//                    Item{
//                        width: parent.width
//                        height: parent.height * 0.7
//                        MultiSwitch{
//                            id:channelsId
//                            anchors.fill: parent
//                            anchors.leftMargin:  10 * mainVisibleRootWindowId.scaleFactor
//                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
//                            stateIndex: board.getChannels() - 1
//                            buttonNames: ["1 ch","2 ch","Both"]
//                            inactiveTextColor: baseGrayColor
//                            activeTextColor: "#303030"
//                            buttonColor: baseRedSwitchColor
//                            fontFamaly: applicationFont.name
//                            radiusVal:0
//                            maxTextHeight: 14 * mainVisibleRootWindowId.scaleFactor
//                            onClickIndex: function(i){
//                                board.setChannels(i + 1)
//                            }
//                        }
//                    }
//                }
//            }
//        }
//        Item{
//            height: parent.height
//            width: parent.width / 4.0
//            visible: board.getCouplingVisible()
//            Item {
//                anchors.fill: parent
//                anchors.margins: 2 * mainVisibleRootWindowId.scaleFactor
//                Column{
//                    anchors.fill: parent
//                    Item{
//                        width: parent.width
//                        height: parent.height * 0.3
//                        Text {
//                            anchors.fill: parent
//                            verticalAlignment: Text.AlignVCenter
//                            horizontalAlignment: Text.AlignHCenter
//                            color: baseTextColor
//                            font.pixelSize: height * 0.6
//                            font.family: applicationFont.name
//                            text: qsTr("COUPLING")
//                        }
//                    }

//                    Item{
//                        width: parent.width
//                        height: parent.height * 0.7
//                        MultiSwitch{
//                            id:couplingId
//                            anchors.fill: parent
//                            anchors.leftMargin:  10 * mainVisibleRootWindowId.scaleFactor
//                            anchors.rightMargin: 10 * mainVisibleRootWindowId.scaleFactor
//                            stateIndex: board.getCoupling() - 1
//                            buttonNames: ["AC","DC"]
//                            inactiveTextColor: baseGrayColor
//                            activeTextColor: "#303030"
//                            buttonColor: baseRedSwitchColor
//                            fontFamaly: applicationFont.name
//                            radiusVal:0
//                            maxTextHeight: 14 * mainVisibleRootWindowId.scaleFactor
//                            onClickIndex: function(i){
//                                board.setCoupling(i + 1)
//                            }
//                        }
//                    }
//                }
//            }
//        }

//    }
//}

