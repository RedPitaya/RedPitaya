import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import QtQuick.Window 2.12
import QtGraphicalEffects 1.12

Item {
    id: rootElement
    width: 600
    height: 50

    property int fontSizeValue: 300
    property int stateIndex: -1
    property var buttonNames: ["TEST"]
    property real buttonTextMargins: 0
    property real subWidth: width / buttonNames.length
    property real buttonMargin: 5
    property double radiusVal: 5.0
    property double radiusValSmall: 5.0
    property color backgraundColor: baseBackGroundColor
    property color buttonColor: baseGrayColor
    property color activeTextColor: "#000000"
    property color inactiveTextColor: baseGrayColor
    property bool  denyAllAnimaton: false
    property bool  enableAnimation: false
    property real  maxTextHeight: 300
    property string fontFamaly: ""

    Timer {
            id: timer
        }

    function delay(delayTime, cb) {
        timer.interval = delayTime;
        timer.repeat = false;
        timer.triggered.connect(cb);
        timer.start();
    }

    onStateIndexChanged: {
        if (enableAnimation === false && !denyAllAnimaton){
            delay(100, function() {
                enableAnimation = true
            });
        }
    }

    signal clickIndex(int index)

    function changeIndex(index) {
        stateIndex = index
        clickIndex(index)
    }

    Rectangle {
        id: backgroundRect
        anchors.fill: parent
        color: backgraundColor
        radius: radiusVal
    }

    TextMetrics {
            id:   t_metrics
            font: backTextId.font
    }

    Row {
        anchors.fill: parent
        spacing: 0
        Repeater {
            model: buttonNames
            Item {
                width: subWidth
                height: rootElement.height
                Text {
                    id:backTextId
                    property real  maxTextH: rootElement.maxTextHeight
                    text: buttonNames[index]
                    anchors.margins: buttonTextMargins + buttonMargin
                    anchors.fill: parent

                    fontSizeMode: Text.Fit
                    font.pixelSize: fontSizeValue
                    font.family: fontFamaly
                    font.weight: Font.Normal
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    color: inactiveTextColor
                    minimumPixelSize: 2
                    font.bold: true
                    lineHeight: 1
                    clip:false
                    onTextChanged: {
                        calcFont()
                    }

                    onWidthChanged: {
                        calcFont()
                    }

                    function calcFont(){
                        if (backTextId.width === 0) return;
                        var currFontSize = 300;
                        var minFontSize  = 300;
                        for(var i = 0 ; i < buttonNames.length ; i++){
                            t_metrics.font.pixelSize = currFontSize;
                            t_metrics.text = buttonNames[i];
                            if (t_metrics.width !== 0){
                                var x = backTextId.width / t_metrics.boundingRect.width;
                                if (minFontSize > currFontSize * x){
                                    minFontSize = currFontSize * x;
                                }

                                var y = backTextId.height / t_metrics.boundingRect.height;
                                if (minFontSize > currFontSize * y){
                                    minFontSize = currFontSize * y;
                                }
                            }
                        }
                        if (maxTextHeight > 0)
                            font.pixelSize = Math.min(maxTextHeight,minFontSize)
                        else
                            font.pixelSize = minFontSize


                    }

                    onMaxTextHChanged: {
                        calcFont()
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        changeIndex(index)
                    }
                }
            }
        }
    }

    Rectangle {
        id: indicatorRect
        z: 2
        width: subWidth - buttonMargin * 2
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.topMargin: buttonMargin
        anchors.bottomMargin: buttonMargin
        x: buttonMargin + stateIndex * subWidth
        radius: radiusValSmall
        color: buttonColor

        Behavior on x {
            enabled: enableAnimation
            NumberAnimation {
                id: bouncebehavior
                easing {
                    type: Easing.InOutQuad
                    amplitude: 2
                    period: 0.2
                }
            }
        }
        Behavior on color {
            enabled: enableAnimation
            ColorAnimation {}
        }

        Text {
            id: buttonLabel
            z: 3
            text: (stateIndex !== -1 && buttonNames[stateIndex] !== undefined) ? buttonNames[stateIndex] : ""
            property real  maxTextH: rootElement.maxTextHeight
            anchors.margins: buttonTextMargins
            anchors.fill: parent
            font.pixelSize: fontSizeValue
            color: activeTextColor
            fontSizeMode: Text.Fit
            font.family: fontFamaly
            font.weight: Font.Normal
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            minimumPixelSize: 2
            font.bold: true
            lineHeight: 1
            clip:false

            onTextChanged: {
                calcFont()
            }

            onWidthChanged: {
                calcFont()
            }

            function calcFont(){
                if (buttonLabel.width === 0) return;
                var currFontSize = 300;
                var minFontSize  = 300;
                for(var i = 0 ; i < buttonNames.length ; i++){
                    t_metrics.font.pixelSize = currFontSize;
                    t_metrics.text = buttonNames[i];
                    if (t_metrics.width !== 0){
                        var x = buttonLabel.width / t_metrics.boundingRect.width;
                        if (minFontSize > currFontSize * x){
                            minFontSize = currFontSize * x;
                        }

                        var y = buttonLabel.height / t_metrics.boundingRect.height;
                        if (minFontSize > currFontSize * y){
                            minFontSize = currFontSize * y;
                        }
                    }
                }
                if (maxTextHeight > 0)
                    font.pixelSize = Math.min(maxTextHeight,minFontSize)
                else
                    font.pixelSize = minFontSize
            }
            Component.onCompleted: {
                calcFont()
            }
            onMaxTextHChanged: {
                calcFont()
            }
        }
    }
}
