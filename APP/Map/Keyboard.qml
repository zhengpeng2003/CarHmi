import QtQuick 2.11
import QtQuick.Controls 2.4

Rectangle {
    id: root
    height: 100
    color: "#d8dde3"
    border.color: "#7f8c8d"
    border.width: 1

    property string inputText: ""
    signal enterPressed()
    signal textUpdated(string text)  // 改名，避免与属性信号冲突

    function appendKey(keyValue) {
        inputText += keyValue
        textUpdated(inputText)
    }

    function backspace() {
        if (inputText.length > 0) {
            inputText = inputText.slice(0, inputText.length - 1)
            textUpdated(inputText)
        }
    }

    function clearAll() {
        inputText = ""
        textUpdated(inputText)
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
        anchors.margins: 2
        spacing: 2

        Repeater {
            model: 4

            delegate: Row {
                spacing: 2
                anchors.horizontalCenter: parent.horizontalCenter

                Repeater {
                    model: root.keyLabels(index)

                    delegate: Button {
                        width: 40
                        height: 25
                        text: modelData
                        font.pixelSize: 10
                        onClicked: root.appendKey(text)
                    }
                }
            }
        }

        Row {
            spacing: 2
            anchors.horizontalCenter: parent.horizontalCenter

            Button {
                width: 50
                height: 22
                text: "空格"
                font.pixelSize: 10
                onClicked: root.appendKey(" ")
            }

            Button {
                width: 50
                height: 22
                text: "回退"
                font.pixelSize: 10
                onClicked: root.backspace()
            }

            Button {
                width: 50
                height: 22
                text: "清空"
                font.pixelSize: 10
                onClicked: root.clearAll()
            }

            Button {
                width: 50
                height: 22
                text: "确认"
                font.pixelSize: 10
                onClicked: root.enterPressed()
            }

            Button {
                width: 50
                height: 22
                text: "隐藏"
                font.pixelSize: 10
                onClicked: root.visible = false
            }
        }
    }
}
