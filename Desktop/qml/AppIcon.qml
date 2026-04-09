import QtQuick 2.11

Rectangle {
    id: root

    property string appName: ""
    property string iconSource: ""
    property bool selected: false

    signal clicked()

    radius: 10
    color: selected ? Qt.rgba(30 / 255, 144 / 255, 1, 90 / 255) : Qt.rgba(0, 0, 0, 60 / 255)
    border.width: selected ? 2 : 1
    border.color: selected ? Qt.rgba(1, 1, 1, 210 / 255) : Qt.rgba(1, 1, 1, 45 / 255)

    Image {
        id: iconImage
        width: 75
        height: 75
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 9
        source: root.iconSource
        sourceSize.width: 75
        sourceSize.height: 75
        fillMode: Image.PreserveAspectFit
        smooth: true
        mipmap: false
    }

    Text {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: "white"
        text: root.appName
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 18
        elide: Text.ElideRight
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }
}
