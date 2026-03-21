import QtQuick 2.11
import QtQuick.Controls 2.4

Item {
    id: root
    property alias text: field.text
    property alias placeholderText: field.placeholderText
    property alias inputMethodHints: field.inputMethodHints
    signal accepted(string text)

    Rectangle {
        anchors.fill: parent
        radius: 4
        color: "#22313f"
        border.width: 1
        border.color: field.activeFocus ? "#4fc3f7" : "#95a5a6"
    }

    TextField {
        id: field
        anchors.fill: parent
        anchors.margins: 1
        color: "white"
        selectByMouse: true
        focus: false
        padding: 8
        background: null
        inputMethodHints: Qt.ImhNoPredictiveText
        onAccepted: root.accepted(text)
    }
}
