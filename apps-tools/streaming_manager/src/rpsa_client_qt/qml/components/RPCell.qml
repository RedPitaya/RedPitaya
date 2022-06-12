import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.5
import QtCharts 2.3

import "../base_components"

Row {
    property int consoleChart: 0
    layer.enabled: true
    Item {
        width: parent.width * 0.4
        height: parent.height

        Column {
            anchors.fill: parent
            anchors.margins: baseSpace * mainVisibleRootWindowId.scaleFactor
            Item {
                width: parent.width
                height: 30 * mainVisibleRootWindowId.scaleFactor
                Text {
                    anchors.fill: parent
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    color: baseTextColor
                    font.pixelSize: height * 0.6
                    font.family: applicationFont.name
                    text: qsTr("SETTINGS")
                }
            }

            Item {
                width: parent.width
                height: parent.height - 60 * mainVisibleRootWindowId.scaleFactor

                RPSettings {
                    anchors.fill: parent
                }
            }

            Item {
                width: parent.width
                height: 30 * mainVisibleRootWindowId.scaleFactor
                Row {
                    anchors.fill: parent
                    Item {
                        height: parent.height
                        width: parent.width / 4.0 - 9
                               * mainVisibleRootWindowId.scaleFactor
                        BaseButton {
                            anchors.fill: parent
                            text: "GET SETTINGS"
                            fontFamily: applicationFont.name
                            radius: 5 * mainVisibleRootWindowId.scaleFactor
                            textSize: 15 * mainVisibleRootWindowId.scaleFactor
                            borderColor: baseRedSwitchColor
                            hoverColor: baseRedSwitchColor
                            textColor: baseTextColor

                            onClickButton: function(){
                                board.getConfig()
                            }
                        }
                    }
                    Item {
                        width: 12 * mainVisibleRootWindowId.scaleFactor
                        height: parent.height
                    }
                    Item {
                        height: parent.height
                        width: parent.width / 4.0 - 9
                               * mainVisibleRootWindowId.scaleFactor
                        BaseButton {
                            anchors.fill: parent
                            text: "SEND SETTINGS"
                            fontFamily: applicationFont.name
                            radius: 5 * mainVisibleRootWindowId.scaleFactor
                            textSize: 15 * mainVisibleRootWindowId.scaleFactor
                            borderColor: baseRedSwitchColor
                            hoverColor: baseRedSwitchColor
                            textColor: baseTextColor

                            onClickButton: function(){
                                board.sendConfig()
                            }
                        }
                    }
                    Item {
                        width: 12 * mainVisibleRootWindowId.scaleFactor
                        height: parent.height
                    }
                    Item {
                        height: parent.height
                        width: parent.width / 4.0 - 9
                               * mainVisibleRootWindowId.scaleFactor
                        BaseButton {
                            anchors.fill: parent
                            text: "START STREAMING"
                            fontFamily: applicationFont.name
                            radius: 5 * mainVisibleRootWindowId.scaleFactor
                            textSize: 15 * mainVisibleRootWindowId.scaleFactor
                            borderColor: baseRedSwitchColor
                            hoverColor: baseRedSwitchColor
                            textColor: baseTextColor

                            onClickButton: function(){
                                board.startStreaming()
                            }
                        }
                    }
                    Item {
                        width: 12 * mainVisibleRootWindowId.scaleFactor
                        height: parent.height
                    }
                    Item {
                        height: parent.height
                        width: parent.width / 4.0 - 9
                               * mainVisibleRootWindowId.scaleFactor
                        BaseButton {
                            anchors.fill: parent
                            text: "STOP STREAMING"
                            fontFamily: applicationFont.name
                            radius: 5 * mainVisibleRootWindowId.scaleFactor
                            textSize: 15 * mainVisibleRootWindowId.scaleFactor
                            borderColor: baseRedSwitchColor
                            hoverColor: baseRedSwitchColor
                            textColor: baseTextColor

                            onClickButton: function(){
                                board.stopStreaming()
                            }
                        }
                    }
                }
            }
        }
    }

    Item {
        width: parent.width * 0.6
        height: parent.height
        Column {
            anchors.fill: parent
            anchors.margins: baseSpace * mainVisibleRootWindowId.scaleFactor
            Item {
                width: parent.width
                height: 30 * mainVisibleRootWindowId.scaleFactor

                Row {
                    anchors.fill: parent
                    Item {
                        height: parent.height
                        width: parent.width - 140
                               * mainVisibleRootWindowId.scaleFactor
                        Text {
                            id:fileNameId
                            anchors.fill: parent
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignLeft
                            color: baseTextColor
                            font.pixelSize: height * 0.5
                            font.family: applicationFontBold.name
                            font.bold: true
                            text: ""
                        }

                        Connections{
                            target: board
                            function onUpdateSaveFileName(){
                                fileNameId.text = board.getSaveFileName()
                            }
                        }
                    }
                    Item {
                        height: parent.height
                        width: 140 * mainVisibleRootWindowId.scaleFactor
                        MultiSwitch {
                            anchors.fill: parent
                            anchors.leftMargin: 4 * mainVisibleRootWindowId.scaleFactor
                            anchors.rightMargin: 4 * mainVisibleRootWindowId.scaleFactor                            
                            buttonNames: ["CONSOLE", "SIGNAL"]
                            inactiveTextColor: "#303030"
                            activeTextColor: "#303030"
                            buttonColor: baseRedSwitchColor
                            fontFamaly: applicationFont.name
                            radiusVal: 0
                            maxTextHeight: 14 * mainVisibleRootWindowId.scaleFactor
                            stateIndex:consoleChart

                            onClickIndex: function(i){
                                consoleChart = i
                                board.setChartEnable(i)
                            }
                        }
                    }
                }
            }

            Item {
                width: parent.width
                height: parent.height - 60 * mainVisibleRootWindowId.scaleFactor
                Rectangle {
                    color: "transparent"
                    border.width: 1
                    border.color: "gray"
                    anchors.fill: parent

                    Item{
                        anchors.fill: parent
                        anchors.margins: 3 * mainVisibleRootWindowId.scaleFactor
                        visible: consoleChart == 0
                        ListView{
                            id:conViewId
                            anchors.fill: parent
                            model: board.getConsoleModel()
                            clip:true
                            delegate: Item{
                                width: conViewId.width
                                height: 16 * mainVisibleRootWindowId.scaleFactor
                                Text {
                                    anchors.fill: parent
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignLeft
                                    color: baseTextColor
                                    font.pixelSize: height * 0.8
                                    font.family: applicationFontBold.name
                                    text: con_text
                                }
                            }
                        }

                    }

                    Item{
                        anchors.fill: parent
                        visible: consoleChart == 1
                        ChartView {
                            id:chartView
                            anchors.fill: parent
                            legend.visible: false
                            animationOptions: ChartView.NoAnimation
                            property bool openGL: true
                            onOpenGLChanged: {
                                series("signal 1").useOpenGL = openGL;
                                series("signal 2").useOpenGL = openGL;
                            }

                            ValueAxis {
                                    id: axisY1
                                    min: -1
                                    max: 1

                                    color: "transparent"
                                }
                            ValueAxis {
                                    id: axisX
                                    min: 0
                                    max: 1000
                                    labelsVisible: false
                                    gridVisible:false
                                    color: "transparent"
                                }
                            LineSeries {
                                    id: lineSeries1
                                    axisX: axisX
                                    axisY: axisY1
                                    useOpenGL: chartView.openGL
                                    color: "red"
                                }
                            LineSeries {
                                    id: lineSeries2
                                    axisX: axisX
                                    axisY: axisY1
                                    useOpenGL: chartView.openGL
                                    color: "blue"
                                }

                            Timer {
                                    id: refreshTimer
                                    interval: 1 / 60 * 1000 // 60 Hz
                                    running: consoleChart == 1
                                    repeat: true
                                    onTriggered: {
                                        if (board.getChartNeedUpdate(0)){
                                            lineSeries1.clear();
                                            let points = board.getChartSignal(0);
                                            for(var k in points){
                                                lineSeries1.append(k,points[k])
                                            }
                                        }

                                        if (board.getChartNeedUpdate(1)){
                                            lineSeries2.clear();
                                            let points2 = board.getChartSignal(1);
                                            for(var k2 in points2){
                                                lineSeries2.append(k2,points2[k2])
                                            }                                            
                                        }

                                    }
                                }
                        }


                    }
                }
            }

            Item {
                width: parent.width
                height: 30 * mainVisibleRootWindowId.scaleFactor
                Row {
                    Connections{
                        target:board

                        function onUpdateStatistic(){
                            bytesId.text = board.getRecivedBytes()
                            bwId.text = board.getBandwidth()
                            sampCH1.text = board.getSamplesCH1()
                            sampCH2.text = board.getSamplesCH2()
                            lostId.text = board.getLostCount()
                        }
                    }

                    anchors.fill: parent
                    Item {
                        height: parent.height
                        width: parent.width / 5.0
                        Row {
                            anchors.fill: parent
                            Item {
                                height: parent.height
                                width: parent.width * 0.3
                                Text {
                                    anchors.fill: parent
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                    color: baseTextColor
                                    font.pixelSize: height * 0.6
                                    font.family: applicationFont.name
                                    text: qsTr("Bytes:")
                                }
                            }

                            Item {
                                height: parent.height
                                width: parent.width * 0.7
                                Text {
                                    id:bytesId
                                    anchors.fill: parent
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                    color: baseTextColor
                                    font.pixelSize: height * 0.5
                                    minimumPixelSize: 5
                                    fontSizeMode: Text.Fit
                                    font.family: applicationFontBold.name
                                    text: "0"
                                }
                            }
                        }
                    }
                    Item {
                        height: parent.height
                        width: parent.width / 5.0
                        Row {
                            anchors.fill: parent
                            Item {
                                height: parent.height
                                width: parent.width * 0.3
                                Text {
                                    anchors.fill: parent
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                    color: baseTextColor
                                    font.pixelSize: height * 0.6
                                    font.family: applicationFont.name
                                    text: qsTr("Bandwidth:")
                                }
                            }

                            Item {
                                height: parent.height
                                width: parent.width * 0.7
                                Text {
                                    id:bwId
                                    anchors.fill: parent
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                    color: baseTextColor
                                    font.pixelSize: height * 0.5
                                    minimumPixelSize: 5
                                    fontSizeMode: Text.Fit
                                    font.family: applicationFontBold.name
                                    text: "0"
                                }
                            }
                        }
                    }
                    Item {
                        height: parent.height
                        width: parent.width / 5.0
                        Row {
                            anchors.fill: parent
                            Item {
                                height: parent.height
                                width: parent.width * 0.3
                                Text {                                    
                                    anchors.fill: parent
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                    color: baseTextColor
                                    font.pixelSize: height * 0.6
                                    font.family: applicationFont.name
                                    text: qsTr("CH1:")
                                }
                            }

                            Item {
                                height: parent.height
                                width: parent.width * 0.7
                                Text {
                                    id:sampCH1
                                    anchors.fill: parent
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                    color: baseTextColor
                                    font.pixelSize: height * 0.5
                                    minimumPixelSize: 5
                                    fontSizeMode: Text.Fit
                                    font.family: applicationFontBold.name
                                    text: "0"
                                }
                            }
                        }
                    }
                    Item {
                        height: parent.height
                        width: parent.width / 5.0
                        Row {
                            anchors.fill: parent
                            Item {
                                height: parent.height
                                width: parent.width * 0.3
                                Text {
                                    anchors.fill: parent
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                    color: baseTextColor
                                    font.pixelSize: height * 0.6
                                    font.family: applicationFont.name
                                    text: qsTr("CH2:")
                                }
                            }

                            Item {
                                height: parent.height
                                width: parent.width * 0.7
                                Text {
                                    id:sampCH2
                                    anchors.fill: parent
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                    color: baseTextColor
                                    font.pixelSize: height * 0.5
                                    minimumPixelSize: 5
                                    fontSizeMode: Text.Fit
                                    font.family: applicationFontBold.name
                                    text: "0"
                                }
                            }
                        }
                    }
                    Item {
                        height: parent.height
                        width: parent.width / 5.0
                        Row {
                            anchors.fill: parent
                            Item {
                                height: parent.height
                                width: parent.width * 0.3
                                Text {                                    
                                    anchors.fill: parent
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                    color: baseTextColor
                                    font.pixelSize: height * 0.6
                                    font.family: applicationFont.name
                                    text: qsTr("Lost:")
                                }
                            }

                            Item {
                                height: parent.height
                                width: parent.width * 0.7
                                Text {
                                    id:lostId
                                    anchors.fill: parent
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                    color: baseTextColor
                                    font.pixelSize: height * 0.5
                                    minimumPixelSize: 5
                                    fontSizeMode: Text.Fit
                                    font.family: applicationFontBold.name
                                    text: "0"
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
