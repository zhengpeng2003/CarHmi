import QtQuick 2.11
import QtQuick.Controls 2.4

Rectangle {
    id: root
    height: 132
    color: "#d8dde3"
    border.color: "#7f8c8d"
    border.width: 1

    property string inputText: ""

    signal enterPressed()

    function appendKey(keyValue) {
        inputText += keyValue
    }

    function backspace() {
        if (inputText.length > 0) {
            inputText = inputText.slice(0, inputText.length - 1)
        }
    }

    function clearAll() {
        inputText = ""
    }

    function keyLabels(row) {
        if (row === 0)
            return ["1", "2", "3", "4", "5", "6", "7", "8", "9", "0"]
        if (row === 1)
            return ["q", "w", "e", "r", "t", "y", "u", "i", "o", "p"]
        if (row === 2)
            return ["a", "s", "d", "f", "g", "h", "j", "k", "l"]
        return ["z", "x", "c", "v", "b", "n", "m"]
    }

    Column {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 4

        Repeater {
            model: 4

            delegate: Row {
                spacing: 4
                anchors.horizontalCenter: parent.horizontalCenter

                Repeater {
                    model: root.keyLabels(index)

                    delegate: Button {
                        width: 42
                        height: 20
                        text: modelData
                        onClicked: root.appendKey(text)
                    }
                }
            }
        }

        Row {
            spacing: 4
            anchors.horizontalCenter: parent.horizontalCenter

            Button {
                width: 72
                height: 26
                text: "Space"
                onClicked: root.appendKey(" ")
            }

            Button {
                width: 72
                height: 26
                text: "Back"
                onClicked: root.backspace()
            }

            Button {
                width: 72
                height: 26
                text: "Clear"
                onClicked: root.clearAll()
            }

            Button {
                width: 72
                height: 26
                text: "Enter"
                onClicked: root.enterPressed()
            }

            Button {
                width: 72
                height: 26
                text: "Hide"
                onClicked: root.visible = false
            }
        }
    }
}
