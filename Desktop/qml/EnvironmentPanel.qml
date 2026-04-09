import QtQuick 2.11

Rectangle {
    id: root

    property real temperature: 0
    property real humidity: 0
    property string carImageSource: ""
    property real carOffsetX: 0

    radius: 15
    color: Qt.rgba(60 / 255, 60 / 255, 60 / 255, 1)

    readonly property real carDisplayWidth: Math.min(200, Math.max(120, width - 40))
    readonly property real carDisplayHeight: Math.min(200, Math.max(120, height - 80))
    readonly property real carBaseX: width / 2 - carImage.width / 2
    readonly property real carBaseY: height / 2 - carImage.height / 2 - 30

    Image {
        id: carImage
        x: root.carBaseX + root.carOffsetX
        y: root.carBaseY
        width: root.carDisplayWidth
        height: root.carDisplayHeight
        source: root.carImageSource
        sourceSize.width: Math.ceil(width)
        sourceSize.height: Math.ceil(height)
        fillMode: Image.PreserveAspectFit
        smooth: true
        mipmap: false
        cache: true
        asynchronous: false
    }

    Text {
        x: 0
        y: root.height / 2 + 20
        width: root.width
        height: 30
        color: "white"
        text: "温度: " + Number(root.temperature).toFixed(1) + "°C"
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 24
        font.bold: true
    }

    Text {
        x: 0
        y: root.height / 2 + 55
        width: root.width
        height: 30
        color: "white"
        text: "湿度: " + Number(root.humidity).toFixed(0) + "%"
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 24
        font.bold: true
    }

    NumberAnimation {
        id: carAnimation
        target: root
        property: "carOffsetX"
        duration: 900
        easing.type: Easing.OutCubic
    }

    MouseArea {
        anchors.fill: carImage
        onClicked: {
            var startOffset = root.width + 20 - root.carBaseX
            carAnimation.stop()
            root.carOffsetX = startOffset
            carAnimation.from = startOffset
            carAnimation.to = 0
            carAnimation.start()
        }
    }
}
