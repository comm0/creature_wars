import QtQuick
import Felgo 4.0

Rectangle {
    id: hudToggle

    property string iconType
    property alias text: toggleText.text
    property bool checked: false

    signal toggled()

    width: toggleRow.implicitWidth + 10
    height: 16
    radius: 3
    color: toggleHover.hovered ? "#223029" : "transparent"
    border.width: 1
    border.color: checked ? "#6f8f63" : "#2c3a31"

    Row {
        id: toggleRow

        anchors.centerIn: parent
        spacing: 4

        AppIcon {
            anchors.verticalCenter: parent.verticalCenter
            size: 9
            iconType: hudToggle.iconType
            color: hudToggle.checked ? "#9fe07a" : "#6b776f"
        }

        AppText {
            id: toggleText

            anchors.verticalCenter: parent.verticalCenter
            color: hudToggle.checked ? "#e8eee9" : "#8a968d"
            font.pixelSize: 8
        }
    }

    HoverHandler {
        id: toggleHover
    }

    TapHandler {
        onTapped: hudToggle.toggled()
    }
}
