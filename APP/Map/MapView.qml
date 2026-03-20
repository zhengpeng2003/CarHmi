import QtQuick 2.11
import QtPositioning 5.11
import QtLocation 5.11

Item {
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
    }

    property alias mapObj: map

    Connections {
        target: gpsManager
        onPositionChanged: map.center = QtPositioning.coordinate(lat,lng)
    }
}
