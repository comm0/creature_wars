import QtQuick

// Placed as a child of a creature item; picks the side that fits inside the
// area of size areaWidth x areaHeight in which the creature item is positioned.
Rectangle {
    id: tooltip

    property bool shown: false
    property real areaWidth: 0
    property real areaHeight: 0
    property int attack: 0
    property int attackRange: 0
    property int visionRange: 0
    property real movementSpeed: 0

    readonly property bool fitsLeft:
        parent.x >= width + 4
    readonly property bool fitsRight:
        areaWidth - parent.x - parent.width >= width + 4
    readonly property bool fitsAbove:
        parent.y >= height + 4
    readonly property bool fitsBelow:
        areaHeight - parent.y - parent.height >= height + 4
    readonly property bool placeLeft:
        fitsLeft || (!fitsRight && parent.x > areaWidth / 2)
    readonly property bool placeAbove:
        fitsAbove || (!fitsBelow && parent.y > areaHeight / 2)

    x: placeLeft ? -width - 4 : parent.width + 4
    y: placeAbove ? -height - 4 : parent.height + 4
    width: tooltipText.implicitWidth + 8
    height: tooltipText.implicitHeight + 6
    radius: 2
    color: "#e6111814"
    border.width: 1
    border.color: "#758579"
    visible: opacity > 0
    opacity: shown ? 1 : 0
    z: 10

    Text {
        id: tooltipText

        anchors.centerIn: parent
        color: "#f1f4f2"
        font.pixelSize: 8
        text: qsTr(
            "ATK: %1  Range: %2\nVision: %3  Speed: %4"
        ).arg(tooltip.attack)
            .arg(tooltip.attackRange)
            .arg(tooltip.visionRange)
            .arg(tooltip.movementSpeed)
    }

    Behavior on opacity {
        NumberAnimation {
            duration: 120
        }
    }
}
