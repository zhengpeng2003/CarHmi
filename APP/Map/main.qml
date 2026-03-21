import QtQuick 2.11
import QtQuick.Controls 2.4

Rectangle {
    width: 480
    height: 272
    color: "black"

    property bool keyboardVisible: searchBar.inputFocused

    function showKeyboard() {
        keyboardVisible = true
        Qt.inputMethod.hide()
    }

    function hideKeyboard() {
        keyboardVisible = false
        Qt.inputMethod.hide()
    }

    MapView {
        id: mapView
        anchors.fill: parent
        anchors.bottomMargin: keyboardVisible ? keyboard.height + 8 : 0
        interactionsEnabled: true
    }


    MouseArea {
        anchors.fill: mapView
        visible: keyboardVisible
        z: 10
        onPressed: hideKeyboard()
    }

    SearchBar {
        id: searchBar
        width: 320
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 8
        message: gpsManager.searchMessage + (gpsManager.environmentMessage === "" ? "" : ("\n环境: " + gpsManager.environmentMessage))
        resultsModel: gpsManager.searchResults
        historyModel: gpsManager.searchHistory
        onSearchRequested: {
            gpsManager.searchPlace(keyword)
            showKeyboard()
        }
        onResultSelected: gpsManager.confirmSearchResult(index)
        onRetryRequested: gpsManager.retryLastSearch()
        onHistorySelected: {
            setKeyword(keyword)
            gpsManager.searchPlace(keyword)
            showKeyboard()
        }
        onClearHistoryRequested: gpsManager.clearSearchHistory()
        onResumeFollowRequested: gpsManager.resumeGpsFollow()
        onInputFocusChanged: if (focused) showKeyboard()
        onInputPressed: showKeyboard()
    }

    Keyboard {
        id: keyboard
        visible: keyboardVisible
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        onKeyPressed: {
            searchBar.appendText(keyValue)
            showKeyboard()
        }
        onBackspacePressed: {
            searchBar.backspace()
            showKeyboard()
        }
        onClearPressed: {
            searchBar.clearKeyword()
            showKeyboard()
        }
        onEnterPressed: {
            searchBar.submitSearch()
            showKeyboard()
        }
        onHideRequested: hideKeyboard()
        onLanguageChanged: showKeyboard()
    }

    MapTypePanel {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: keyboardVisible ? keyboard.top : parent.bottom
        anchors.topMargin: 8
        anchors.rightMargin: 8
        anchors.bottomMargin: 8
        mapObj: mapView.mapObj
    }

    Rectangle {
        anchors.left: parent.left
        anchors.bottom: keyboardVisible ? keyboard.top : parent.bottom
        anchors.margins: 8
        radius: 4
        color: "#66000000"
        width: 160
        height: 24

        Text {
            anchors.centerIn: parent
            color: "white"
            text: gpsManager.mapState === 0 ? "GPS 跟随" : (gpsManager.mapState === 1 ? "搜索定位" : "手动浏览")
            font.pixelSize: 12
        }
    }
}
