import QtQuick 2.11
import QtQuick.Controls 2.4

Item {
    id: root
    width: 68

    property var mapObj

    Column {
        anchors.right: parent.right
        anchors.top: parent.top
        spacing: 2

        Repeater {
            model: root.mapObj ? root.mapObj.supportedMapTypes : []

            delegate: Button {
                width: 64
                height: 24
                text: modelData.name
                font.pixelSize: 10
                onClicked: root.mapObj.activeMapType = modelData
            }
        }
    }
}
