import QtQuick 2.11
import QtQuick.Controls 2.4

Rectangle {
    id: root
    height: 54
    color: "#66000000"
    radius: 4

    property string text: ""
    property string message: ""

    signal requestKeyboard()
    signal searchRequested(string keyword)

    Column {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 4

        Row {
            width: parent.width
            height: 26
            spacing: 4

            TextField {
                id: textField
                width: parent.width - searchButton.width - 4
                height: parent.height
                readOnly: true
                text: root.text
                placeholderText: "输入城市拼音"
                color: "white"
                selectByMouse: false

                background: Rectangle {
                    radius: 2
                    color: "#22313f"
                    border.width: 1
                    border.color: "#95a5a6"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.requestKeyboard()
                }
            }

            Button {
                id: searchButton
                width: 56
                height: parent.height
                text: "搜索"
                onClicked: root.searchRequested(root.text)
            }
        }

        Text {
            width: parent.width
            height: 16
            text: root.message
            color: "#ffe08a"
            font.pixelSize: 12
            elide: Text.ElideRight
        }
    }
}
