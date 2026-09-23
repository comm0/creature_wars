import QtQuick 2.15

// Rubber-band rectangle spanning two corner points given in parent coordinates.
Rectangle {
    property real startX: 0
    property real startY: 0
    property real currentX: 0
    property real currentY: 0

    x: Math.min(startX, currentX)
    y: Math.min(startY, currentY)
    width: Math.abs(currentX - startX)
    height: Math.abs(currentY - startY)
    color: "#304d8fd8"
    border.width: 1
    border.color: "#8eb9ff"
}
