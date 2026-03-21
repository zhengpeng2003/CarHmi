import QtQuick 2.11
import QtQuick.Controls 2.4

Rectangle {
    id: root
    height: 144
    color: "#d8dde3"
    border.color: "#7f8c8d"
    border.width: 1
    radius: 6

    property bool englishMode: true
    property string hintText: englishMode ? "EN 输入" : "中文拼音输入"

    signal keyPressed(string keyValue)
    signal backspacePressed()
    signal clearPressed()
    signal enterPressed()
    signal hideRequested()
    signal languageChanged(bool englishMode)

    function keyLabels(row) {
        if (englishMode) {
            if (row === 0)
                return ["1", "2", "3", "4", "5", "6", "7", "8", "9", "0"]
            if (row === 1)
                return ["q", "w", "e", "r", "t", "y", "u", "i", "o", "p"]
            if (row === 2)
                return ["a", "s", "d", "f", "g", "h", "j", "k", "l"]
            return ["z", "x", "c", "v", "b", "n", "m"]
        }

        if (row === 0)
            return ["中", "国", "北", "京", "上", "海", "深", "圳", "广", "州"]
        if (row === 1)
            return ["a", "o", "e", "i", "u", "n", "g", "h", "r", "s"]
        if (row === 2)
            return ["b", "p", "m", "f", "d", "t", "l", "y", "w"]
        return ["j", "q", "x", "z", "c", "k", "l"]
    }

    Column {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 4

        Row {
            width: parent.width
            spacing: 8

            Text {
                text: root.hintText + "｜点击输入框呼出键盘，拖地图时不干扰输入"
                color: "#2c3e50"
                font.pixelSize: 12
                width: parent.width - langButton.width - 12
                wrapMode: Text.Wrap
            }

            Button {
                id: langButton
                width: 64
                height: 24
                text: root.englishMode ? "EN/中" : "中/EN"
                onClicked: {
                    root.englishMode = !root.englishMode
                    root.languageChanged(root.englishMode)
                }
            }
        }

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
                        onClicked: root.keyPressed(text)
                    }
                }
            }
        }

        Row {
            spacing: 4
            anchors.horizontalCenter: parent.horizontalCenter

            Button {
                width: 64
                height: 26
                text: "空格"
                onClicked: root.keyPressed(" ")
            }

            Button {
                width: 64
                height: 26
                text: "退格"
                onClicked: root.backspacePressed()
            }

            Button {
                width: 64
                height: 26
                text: "清空"
                onClicked: root.clearPressed()
            }

            Button {
                width: 64
                height: 26
                text: "搜索"
                onClicked: root.enterPressed()
            }

            Button {
                width: 64
                height: 26
                text: "隐藏"
                onClicked: root.hideRequested()
            }
        }
    }
}
