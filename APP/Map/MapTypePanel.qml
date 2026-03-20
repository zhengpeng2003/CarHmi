import QtQuick 2.11
import QtQuick.Controls 2.4

Item {
    id: root
    width: 76

    property var mapObj

    Column {
        anchors.right: parent.right
        anchors.top: parent.top
        spacing: 4

        Repeater {
            model: root.mapObj ? root.mapObj.supportedMapTypes : []

            delegate: Button {
                width: 72
                height: 28
                text: modelData.name
                onClicked: root.mapObj.activeMapType = modelData
            }
        }
    }

}
