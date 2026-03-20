import QtQuick 2.11
import QtQuick.Controls 2.4
import QtPositioning 5.11
import QtLocation 5.11

Rectangle {
    width: 480
    height: 272

    Plugin {
        id: mapPlugin
        name: "amap"
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        zoomLevel: 15

        center: QtPositioning.coordinate(
            gpsManager.latitude,
            gpsManager.longitude
        )

        Connections {
            target: gpsManager
            onPositionChanged: {
                map.center = QtPositioning.coordinate(lat, lng)
            }
        }
    }

    // 搜索框（预留）
    Rectangle {
        width: 180
        height: 28
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 5
        color: "#66000000"

        TextField {
            anchors.fill: parent
            anchors.margins: 2
            placeholderText: "搜索"
        }
    }

    // 右侧地图类型
    Column {
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        spacing: 4

        Repeater {
            model: map.supportedMapTypes

            delegate: Rectangle {
                width: 70
                height: 26
                color: "#66000000"

                Text {
                    anchors.centerIn: parent
                    text: modelData.name
                    color: "white"
                    font.pixelSize: 10
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: map.activeMapType = modelData
                }
            }
        }
    }
}
