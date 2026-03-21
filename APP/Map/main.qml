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
        width: 320
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 8
        message: gpsManager.searchMessage + (gpsManager.environmentMessage === "" ? "" : ("\n环境: " + gpsManager.environmentMessage))
        resultsModel: gpsManager.searchResults
        historyModel: gpsManager.searchHistory
        onSearchRequested: gpsManager.searchPlace(keyword)
        onResultSelected: gpsManager.confirmSearchResult(index)
        onRetryRequested: gpsManager.retryLastSearch()
        onHistorySelected: {
            setKeyword(keyword)
            gpsManager.searchPlace(keyword)
        }
        onClearHistoryRequested: gpsManager.clearSearchHistory()
        onResumeFollowRequested: gpsManager.resumeGpsFollow()
    }

    MapTypePanel {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.topMargin: 8
        anchors.rightMargin: 8
        anchors.bottomMargin: 8
        mapObj: mapView.mapObj
    }

    Rectangle {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 8
        radius: 4
        color: "#66000000"
        width: 96
        height: 24

        Text {
            anchors.centerIn: parent
            color: "white"
            text: gpsManager.mapState === 0 ? "GPS 跟随" : (gpsManager.mapState === 1 ? "搜索定位" : "手动浏览")
            font.pixelSize: 12
        }
    }
}
