pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: radialMenu

    property var actionsModel
    property real radius: 40
    property real actionSize: 44
    property int gold: 0
    property int food: 0
    property bool running: true
    property real timeScale: 1
    readonly property alias hovered: menuHover.hovered

    signal actionActivated(string actionKey)

    width: 2 * radius + actionSize
    height: width

    HoverHandler {
        id: menuHover

        blocking: true
    }

    Repeater {
        id: circleRepeater

        model: radialMenu.actionsModel

        delegate: ActionCircle {
            required property int index

            readonly property real angle:
                (-90 + index * 360 / Math.max(1, circleRepeater.count)) * Math.PI / 180

            x: radialMenu.width / 2 + radialMenu.radius * Math.cos(angle) - width / 2
            y: radialMenu.height / 2 + radialMenu.radius * Math.sin(angle) - height / 2
            width: radialMenu.actionSize
            height: radialMenu.actionSize
            gold: radialMenu.gold
            food: radialMenu.food
            running: radialMenu.running
            timeScale: radialMenu.timeScale

            onActivated: function(actionKey) {
                radialMenu.actionActivated(actionKey)
            }
        }
    }
}
