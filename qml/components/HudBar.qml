import QtQuick

Rectangle {
    id: hudBar

    property bool borderOnTop: true

    color: "#e6121a15"

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        y: hudBar.borderOnTop ? 0 : parent.height - height
        height: 1
        color: "#405348"
    }
}
