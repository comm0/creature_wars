pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: reticle

    property color color: "#ff4a3d"
    property real cornerLength: 5
    property real lineWidth: 1.5

    Repeater {
        model: 4

        delegate: Item {
            required property int index

            readonly property bool onLeft: index % 2 === 0
            readonly property bool onTop: index < 2

            anchors.fill: parent

            Rectangle {
                x: parent.onLeft ? 0 : reticle.width - reticle.cornerLength
                y: parent.onTop ? 0 : reticle.height - reticle.lineWidth
                width: reticle.cornerLength
                height: reticle.lineWidth
                color: reticle.color
            }

            Rectangle {
                x: parent.onLeft ? 0 : reticle.width - reticle.lineWidth
                y: parent.onTop ? 0 : reticle.height - reticle.cornerLength
                width: reticle.lineWidth
                height: reticle.cornerLength
                color: reticle.color
            }
        }
    }

    SequentialAnimation on scale {
        running: reticle.visible
        loops: Animation.Infinite

        NumberAnimation {
            to: 1.15
            duration: 380
            easing.type: Easing.InOutSine
        }

        NumberAnimation {
            to: 1
            duration: 380
            easing.type: Easing.InOutSine
        }
    }
}
