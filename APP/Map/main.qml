import QtQuick 2.11
import QtQuick.Controls 2.4

Rectangle {
    width: 480
    height: 272
    color: "black"

    MapView {
        id: mapView
        anchors.fill: parent
    }

    SearchBar {
        id: searchBar
        width: 250
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 8
        text: keyboard.inputText
        message: gpsManager.searchMessage

        onRequestKeyboard: keyboard.visible = true
        onSearchRequested: {
            keyboard.visible = false
            gpsManager.searchPlace(keyword)
        }
    }

    MapTypePanel {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.topMargin: 8
        anchors.rightMargin: 8
        anchors.bottomMargin: keyboard.visible ? keyboard.height + 8 : 8
        mapObj: mapView.mapObj
    }

    Keyboard {
        id: keyboard
        width: parent.width
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        visible: false

        onEnterPressed: {
            if (inputText.length > 0) {
                visible = false
                gpsManager.searchPlace(inputText)
            }
        }
    }
}
