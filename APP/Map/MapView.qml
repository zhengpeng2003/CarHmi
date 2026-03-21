import QtQuick 2.11
import QtPositioning 5.11
import QtLocation 5.11

Item {
    id: root
    anchors.fill: parent
    property alias mapObj: map
    property bool internalCenterUpdate: false

    Plugin {
        id: mapPlugin
        name: "amap"
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        zoomLevel: 15
        center: QtPositioning.coordinate(gpsManager.latitude, gpsManager.longitude)

        gesture.enabled: true
        onCenterChanged: {
            if (!root.internalCenterUpdate) {
                gpsManager.enterManualBrowse()
            }
        }

        MapQuickItem {
            id: searchMarker
            visible: false
            anchorPoint.x: 10
            anchorPoint.y: 20
            coordinate: QtPositioning.coordinate(gpsManager.latitude, gpsManager.longitude)
            sourceItem: Column {
                spacing: 2
                Rectangle { width: 20; height: 20; radius: 10; color: "#e74c3c"; border.width: 2; border.color: "white" }
                Rectangle { width: 4; height: 8; radius: 2; color: "#e74c3c"; anchors.horizontalCenter: parent.horizontalCenter }
            }
        }
    }

    function applyCenter(lat, lng, showMarker) {
        root.internalCenterUpdate = true
        map.center = QtPositioning.coordinate(lat, lng)
        searchMarker.coordinate = QtPositioning.coordinate(lat, lng)
        searchMarker.visible = showMarker
        root.internalCenterUpdate = false
    }

    Connections {
        target: gpsManager
        onCenterRequested: {
            applyCenter(lat, lng, state === 1)
        }
        onSearchFailed: searchMarker.visible = false
    }
}
