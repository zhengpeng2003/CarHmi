import QtQuick 2.11
import QtQuick.Controls 2.2   // 使用 2.2 以获得更好的兼容性

Rectangle {
    id: searchBarRoot
    height: content.implicitHeight + 8
    color: "#66000000"
    radius: 4

    property string message: ""
    property var resultsModel: []
    property var historyModel: []

    property alias inputField: inputField
    property alias inputFieldText: inputField.text
    signal inputFieldFocusChanged(bool focused)

    signal searchRequested(string keyword)
    signal resultSelected(int index)
    signal retryRequested()
    signal historySelected(string keyword)
    signal clearHistoryRequested()
    signal resumeFollowRequested()

    function getInputText() { return inputField.text }
    function setInputText(text) {
        inputField.text = text
        inputField.cursorPosition = text.length
    }
    function setKeyword(value) {
        inputField.text = value
        inputField.forceActiveFocus()
    }
    function clearSearchUi() {
        resultsModel = []
        inputField.text = ""
    }

    Column {
        id: content
        anchors.fill: parent
        anchors.margins: 4
        spacing: 4

        Row {
            id: buttonRow
            width: parent.width
            height: 30
            spacing: 4

            TextField {
                id: inputField
                // 计算宽度：总宽减去所有按钮宽度和间距
                width: parent.width
                       - searchButton.width
                       - retryButton.width
                       - followButton.width
                       - exitButton.width
                       - (4 * parent.spacing)  // 4个间隙
                height: parent.height
                placeholderText: "输入地点关键字"
                font.pixelSize: 12

                onActiveFocusChanged: {
                    console.log("InputField focus changed:", activeFocus)
                    searchBarRoot.inputFieldFocusChanged(activeFocus)
                }
                onAccepted: searchBarRoot.searchRequested(text)
            }

            Button {
                id: searchButton
                width: 52
                height: parent.height
                text: "搜索"
                font.pixelSize: 11
                onClicked: searchBarRoot.searchRequested(inputField.text)
            }

            Button {
                id: retryButton
                width: 52
                height: parent.height
                text: "重试"
                font.pixelSize: 11
                onClicked: searchBarRoot.retryRequested()
            }

            Button {
                id: followButton
                width: 52
                height: parent.height
                text: "跟随"
                font.pixelSize: 11
                onClicked: searchBarRoot.resumeFollowRequested()
            }

            Button {
                id: exitButton
                width: 52
                height: parent.height
                text: "退出"
                font.pixelSize: 11
                onClicked: {
                    if (typeof appQuitter !== "undefined" && appQuitter)
                        appQuitter.quitApp()
                    else
                        console.warn("appQuitter not available")
                }
            }
        }

        Text {
            width: parent.width
            text: searchBarRoot.message
            color: "#ffe08a"
            font.pixelSize: 11
            wrapMode: Text.Wrap
        }

        Rectangle {
            width: parent.width
            visible: historyRepeater.count > 0
            height: visible ? 24 : 0
            color: "transparent"
            Row {
                anchors.fill: parent
                spacing: 4
                Text {
                    text: "历史"
                    color: "white"
                    width: 24
                    font.pixelSize: 10
                    verticalAlignment: Text.AlignVCenter
                }
                Repeater {
                    id: historyRepeater
                    model: searchBarRoot.historyModel
                    delegate: Button {
                        height: 20
                        text: modelData
                        font.pixelSize: 10
                        onClicked: searchBarRoot.historySelected(modelData)
                    }
                }
                Button {
                    visible: historyRepeater.count > 0
                    height: 20
                    text: "清空"
                    font.pixelSize: 10
                    onClicked: searchBarRoot.clearHistoryRequested()
                }
            }
        }

        Rectangle {
            width: parent.width
            visible: resultsView.count > 0
            height: visible ? Math.min(resultsView.contentHeight + 8, 100) : 0
            radius: 4
            color: "#1b2631"
            border.color: "#4f5b66"

            ListView {
                id: resultsView
                anchors.fill: parent
                anchors.margins: 4
                model: searchBarRoot.resultsModel
                clip: true
                delegate: Rectangle {
                    width: ListView.view.width
                    height: 28
                    color: index % 2 === 0 ? "#243447" : "#1f2d3a"

                    Row {
                        anchors.fill: parent
                        anchors.margins: 4
                        spacing: 6
                        Text {
                            width: parent.width - chooseButton.width - 8
                            color: "white"
                            elide: Text.ElideRight
                            font.pixelSize: 10
                            text: (modelData.title || "") + ((modelData.district || "") ? (" (" + modelData.district + ")") : "")
                            verticalAlignment: Text.AlignVCenter
                        }
                        Button {
                            id: chooseButton
                            width: 40
                            height: 20
                            text: "定位"
                            font.pixelSize: 9
                            onClicked: {
                                searchBarRoot.resultSelected(index)
                                searchBarRoot.clearSearchUi()
                            }
                        }
                    }
                }
            }
        }
    }
}
