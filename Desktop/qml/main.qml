import QtQuick 2.11

Rectangle {
    id: root
    width: 480
    height: 272
    color: "white"

    readonly property int leftWidth: Math.floor(width / 2)
    readonly property int spacingValue: 5
    readonly property int rows: 2
    readonly property int columns: 2
    readonly property int squareW: Math.floor((leftWidth - (columns + 1) * spacingValue) / columns)
    readonly property int squareH: Math.floor((height - (rows + 1) * spacingValue) / rows)
    readonly property int cellSize: Math.min(squareW, squareH)

    Flickable {
        id: pager
        x: 0
        y: 0
        width: root.leftWidth
        height: root.height
        contentWidth: Math.max(1, widgetController.pageCount) * width
        contentHeight: height
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.HorizontalFlick
        interactive: widgetController.pageCount > 1

        property int dragOriginPage: 0

        onMovementStarted: {
            dragOriginPage = widgetController.currentPage
            widgetController.clearSelectionFromQml()
        }

        onMovementEnded: {
            var targetPage = dragOriginPage
            var delta = contentX - dragOriginPage * width

            if (delta > 60 && targetPage < widgetController.pageCount - 1) {
                targetPage += 1
            } else if (delta < -60 && targetPage > 0) {
                targetPage -= 1
            }

            widgetController.pageChanged(targetPage)
            snapAnimation.to = targetPage * width
            snapAnimation.restart()
        }

        Repeater {
            model: Math.max(1, widgetController.pageCount)

            delegate: Item {
                id: pageItem
                property int pageIndex: index
                x: index * pager.width
                y: 0
                width: pager.width
                height: pager.height

                Repeater {
                    model: 4

                    delegate: AppIcon {
                        property int modelIndex: index + (pageItem.pageIndex * 4)
                        property var appData: widgetController.appAt(modelIndex)

                        x: root.spacingValue + (index % 2) * (root.cellSize + root.spacingValue)
                        y: root.spacingValue + Math.floor(index / 2) * (root.cellSize + root.spacingValue)+15
                        width: root.cellSize
                        height: root.cellSize
                        visible: appData.valid === true
                        appName: visible ? appData.name : ""
                        iconSource: visible ? appData.iconPath : ""
                        selected: visible && widgetController.selectedAppId === appData.id
                        onClicked: widgetController.appClicked(appData.id)
                    }
                }
            }
        }

        NumberAnimation {
            id: snapAnimation
            target: pager
            property: "contentX"
            duration: 250
            easing.type: Easing.OutCubic
        }
    }

    Repeater {
        model: Math.max(1, widgetController.pageCount)

        delegate: Rectangle {
            width: 8
            height: 8
            radius: 4
            x: root.leftWidth / 2 - ((Math.max(1, widgetController.pageCount) * 8) + ((Math.max(1, widgetController.pageCount) - 1) * 8)) / 2 + index * 16
            y: root.height - 18
            color: index === widgetController.currentPage ? Qt.rgba(1, 1, 1, 0.82) : Qt.rgba(1, 1, 1, 0.28)
        }
    }

    EnvironmentPanel {
        id: environmentPanel
        x: root.leftWidth + 5
        y: 5
        width: root.width - root.leftWidth - 10
        height: root.height - 10
        temperature: widgetController.temperature
        humidity: widgetController.humidity
        carImageSource: widgetController.carImageSource
    }

    LoadingOverlay {
        anchors.fill: parent
        visible: widgetController.loadingVisible
        appName: widgetController.loadingAppName
    }

    Connections {
        target: widgetController

        onCurrentPageChanged: {
            var targetX = widgetController.currentPage * pager.width
            if (Math.abs(pager.contentX - targetX) > 0.5) {
                snapAnimation.to = targetX
                snapAnimation.restart()
            }
        }
    }
}
