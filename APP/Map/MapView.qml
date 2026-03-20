import QtQuick 2.11
import QtPositioning 5.11
import QtLocation 5.11

Item {
    id: root
    anchors.fill: parent

    Plugin {
        id: mapPlugin
        name: "amap"
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        zoomLevel: 15
        center: QtPositioning.coordinate(gpsManager.latitude,gpsManager.longitude)

        MapQuickItem {
            id: searchMarker
            visible: false
            anchorPoint.x: 10
            anchorPoint.y: 20
            coordinate: QtPositioning.coordinate(gpsManager.latitude,
                                                 gpsManager.longitude)

            sourceItem: Column {
                spacing: 2

                Rectangle {
                    width: 20
                    height: 20
                    radius: 10
                    color: "#e74c3c"
                    border.width: 2
                    border.color: "white"
                }

                Rectangle {
                    width: 4
                    height: 8
                    radius: 2
                    color: "#e74c3c"
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }

    property alias mapObj: map

    Connections {
        target: gpsManager
        onPositionChanged: {
            map.center = QtPositioning.coordinate(lat, lng)
        }
        onSearchLocationFound: {
            map.center = QtPositioning.coordinate(lat, lng)
            searchMarker.coordinate = QtPositioning.coordinate(lat, lng)
            searchMarker.visible = true
        }
        onSearchFailed: {
            searchMarker.visible = false
        }
    }
}
