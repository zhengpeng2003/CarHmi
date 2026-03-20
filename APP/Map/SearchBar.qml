import QtQuick 2.11
import QtQuick.Controls 2.4

Rectangle {
    width: 200
    height: 30
    anchors.left: parent.left
    anchors.top: parent.top
    anchors.margins: 5
    color: "#66000000"

    signal search(string text)
    property string inputText: ""

    Text {
        anchors.centerIn: parent
        text: inputText
        color: "white"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: keyboard.visible = true
    }

    Keyboard {
        id: keyboard
        anchors.bottom: parent.bottom
        visible: false
        onKeyPressed: inputText += key
    }

    MouseArea {
        anchors.right: parent.right
        width: 40
        height: parent.height
        onClicked: search(inputText)
    }
}
