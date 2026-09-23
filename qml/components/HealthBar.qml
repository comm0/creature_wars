import QtQuick 2.15

Rectangle {
    id: healthBar

    property int health: 0
    property int maximumHealth: 1
    readonly property real ratio: Math.max(
        0,
        Math.min(1, health / Math.max(maximumHealth, 1))
    )
    readonly property color fillColor: ratio > 0.8
        ? "#46c95b"
        : ratio >= 0.3 ? "#e2c94f" : "#d94a45"

    width: 20
    height: 4
    radius: height / 2
    color: "#cc111511"
    border.width: 1
    border.color: "#cc000000"
    clip: true

    Rectangle {
        x: 1
        y: 1
        width: (parent.width - 2) * healthBar.ratio
        height: parent.height - 2
        radius: height / 2
        color: healthBar.fillColor
    }
}
