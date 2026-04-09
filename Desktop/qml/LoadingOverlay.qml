import QtQuick 2.11

Item {
    id: root

    property string appName: ""
    property int frame: 0

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(3 / 255, 10 / 255, 22 / 255, 180 / 255)
    }

    Rectangle {
        width: 250
        height: 136
        anchors.centerIn: parent
        radius: 18
        color: Qt.rgba(20 / 255, 29 / 255, 43 / 255, 238 / 255)
        border.width: 1
        border.color: Qt.rgba(1, 1, 1, 45 / 255)

        Text {
            x: 0
            y: 20
            width: parent.width
            height: 28
            horizontalAlignment: Text.AlignHCenter
            color: "white"
            text: "APP"
            font.pixelSize: 24
            font.bold: true
        }

        Text {
            x: 18
            y: 58
            width: parent.width - 36
            height: 30
            horizontalAlignment: Text.AlignHCenter
            color: "white"
            text: "正在打开 " + root.appName
            font.pixelSize: 21
            font.bold: true
            elide: Text.ElideRight
        }

        Text {
            x: 18
            y: 92
            width: parent.width - 36
            height: 24
            horizontalAlignment: Text.AlignHCenter
            color: Qt.rgba(1, 1, 1, 175 / 255)
            text: "应用初始化中" + ["", ".", "..", "..."][root.frame]
            font.pixelSize: 14
        }
    }

    Timer {
        interval: 180
        repeat: true
        running: root.visible
        onTriggered: root.frame = (root.frame + 1) % 4
    }

    onVisibleChanged: {
        if (visible) {
            frame = 0
        }
    }
}
