pragma ComponentBehavior: Bound

import QtQuick
import Felgo 4.0

EntityBase {
    id: explosion

    property int tileSize: 16
    property int radius: 1
    property color color: "white"
    property real gameTimeScale: 1

    entityType: "areaExplosion"
    width: (2 * radius + 1) * tileSize
    height: width

    Grid {
        anchors.fill: parent
        columns: 2 * explosion.radius + 1

        Repeater {
            model: (2 * explosion.radius + 1) * (2 * explosion.radius + 1)

            delegate: Rectangle {
                width: explosion.tileSize
                height: explosion.tileSize
                color: Qt.rgba(explosion.color.r, explosion.color.g, explosion.color.b, 0.45)
                border.width: 1
                border.color: explosion.color
            }
        }
    }

    SequentialAnimation {
        running: true

        NumberAnimation {
            target: explosion
            property: "opacity"
            from: 1
            to: 0
            duration: Math.max(1, 450 / explosion.gameTimeScale)
            easing.type: Easing.InQuad
        }

        ScriptAction {
            script: explosion.removeEntity()
        }
    }
}
