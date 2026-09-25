pragma ComponentBehavior: Bound

import QtQuick
import Felgo 4.0

EntityBase {
    id: resourceEffect

    required property url imageSource
    required property int amount
    required property real startX
    required property real startY
    required property real targetX
    required property real targetY
    property int delay: 0

    entityType: "resourceFly"
    x: startX - width / 2
    y: startY - height / 2
    width: 34
    height: 14
    z: 1

    Rectangle {
        anchors.fill: parent
        radius: 4
        color: "#d9161d19"
        border.width: 1
        border.color: "#805d7163"
    }

    Row {
        anchors.centerIn: parent
        spacing: 2

        MultiResolutionImage {
            anchors.verticalCenter: parent.verticalCenter
            width: 10
            height: 10
            source: resourceEffect.imageSource
            smooth: false
        }

        AppText {
            anchors.verticalCenter: parent.verticalCenter
            color: "#f1f4f2"
            font.pixelSize: 8
            font.weight: Font.Bold
            style: Text.Outline
            styleColor: "#000000"
            text: "+" + resourceEffect.amount
        }
    }

    SequentialAnimation {
        running: true

        PauseAnimation {
            duration: resourceEffect.delay
        }

        ParallelAnimation {
            NumberAnimation {
                target: resourceEffect
                property: "x"
                to: resourceEffect.targetX - resourceEffect.width / 2
                duration: 900
                easing.type: Easing.InOutCubic
            }

            NumberAnimation {
                target: resourceEffect
                property: "y"
                to: resourceEffect.targetY - resourceEffect.height / 2
                duration: 900
                easing.type: Easing.InOutCubic
            }

            SequentialAnimation {
                PauseAnimation {
                    duration: 620
                }

                NumberAnimation {
                    target: resourceEffect
                    property: "opacity"
                    to: 0
                    duration: 280
                }
            }
        }

        onFinished: resourceEffect.removeEntity()
    }
}
