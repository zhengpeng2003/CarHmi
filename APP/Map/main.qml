import QtQuick 2.11
import QtQuick.Controls 2.4

Rectangle {
    width: 480
    height: 272
    color: "black"

    property bool keyboardShowing: false
    property var mapTypeNames: ({
        "road": "街道",
        "terrain": "地形",
        "satellite": "卫星",
        "hybrid": "混合"
    })

    MapView {
        id: mapView
        anchors.fill: parent
    }

    SearchBar {
        id: searchBar
        width: parent.width - 20
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 8
        message: gpsManager.searchMessage + (gpsManager.environmentMessage === "" ? "" : ("\n环境: " + gpsManager.environmentMessage))
        resultsModel: gpsManager.searchResults
        historyModel: gpsManager.searchHistory
        onSearchRequested: gpsManager.searchPlace(keyword)
        onResultSelected: {
            gpsManager.confirmSearchResult(index)
            gpsManager.clearSearchResults()
            setKeyword("")
        }
        onRetryRequested: gpsManager.retryLastSearch()
        onHistorySelected: {
            setKeyword(keyword)
            gpsManager.searchPlace(keyword)
        }
        onClearHistoryRequested: gpsManager.clearSearchHistory()
        onResumeFollowRequested: gpsManager.resumeGpsFollow()
    }

    Column {
        id: zoomButtons
        anchors.left: searchBar.left
        anchors.top: searchBar.bottom
        anchors.topMargin: 8
        spacing: 4

        Button {
            width: 40
            height: 32
            text: "+"
            font.pixelSize: 18
            onClicked: mapView.mapObj.zoomLevel = Math.min(mapView.mapObj.maximumZoomLevel, mapView.mapObj.zoomLevel + 1)
        }

        Button {
            width: 40
            height: 32
            text: "-"
            font.pixelSize: 18
            onClicked: mapView.mapObj.zoomLevel = Math.max(mapView.mapObj.minimumZoomLevel, mapView.mapObj.zoomLevel - 1)
        }
    }

    Column {
        id: mapTypeButtons
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.top: zoomButtons.top
        spacing: 4
        visible: mapView.mapObj ? mapView.mapObj.supportedMapTypes.length > 0 : false

        Repeater {
            model: mapView.mapObj ? mapView.mapObj.supportedMapTypes : []
            delegate: Button {
                width: 56
                height: 32
                text: {
                    var name = modelData.name.toLowerCase()
                    if (name === "road" || name === "street") return "街道"
                    if (name === "terrain") return "地形"
                    if (name === "satellite") return "卫星"
                    if (name === "hybrid") return "混合"
                    return modelData.name
                }
                font.pixelSize: 12
                highlighted: mapView.mapObj.activeMapType === modelData
                onClicked: mapView.mapObj.activeMapType = modelData
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 8
        radius: 4
        color: "#66000000"
        width: 88
        height: 20

        Text {
            anchors.centerIn: parent
            color: "white"
            text: gpsManager.mapState === 0 ? "GPS 跟随" : (gpsManager.mapState === 1 ? "搜索定位" : "手动浏览")
            font.pixelSize: 10
        }
    }

    Keyboard {
        id: floatingKeyboard
        visible: false
        width: parent.width
        height: 140
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        anchors.left: parent.left
        anchors.right: parent.right
        z: 10

        onEnterPressed: {
            gpsManager.searchPlace(searchBar.getInputText())
            visible = false
            keyboardShowing = false
        }

        onTextUpdated: {
            if (visible) {
                searchBar.setInputText(floatingKeyboard.inputText)
            }
        }
    }

    Rectangle {
        id: keyboardBtnArea
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8
        width: 36
        height: 32
        radius: 18
        color: "#4fc3f7"
        visible: true
        z: 11

        Button {
            id: showKeyboardBtn
            anchors.fill: parent
            text: "⌨"
            font.pixelSize: 18
            flat: true
            onClicked: {
                if (!floatingKeyboard.visible) {
                    floatingKeyboard.inputText = searchBar.getInputText()
                    floatingKeyboard.visible = true
                    keyboardShowing = true
                } else {
                    floatingKeyboard.visible = false
                    keyboardShowing = false
                }
            }
        }
    }

    Connections {
        target: searchBar.inputField
        onPressed: {
            if (!keyboardShowing) {
                floatingKeyboard.inputText = searchBar.getInputText()
                floatingKeyboard.visible = true
                keyboardShowing = true
            }
        }
    }

    Connections {
        target: searchBar
        onSearchRequested: {
            floatingKeyboard.visible = false
            keyboardShowing = false
        }
    }
}
