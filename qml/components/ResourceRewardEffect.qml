pragma ComponentBehavior: Bound

import QtQuick
import Felgo 4.0

EntityBase {
    id: rewardEffect

    property int gold: 0
    property int food: 0

    entityType: "resourceReward"
    width: 96
    height: 16

    transform: Translate {
        id: rewardRise
    }

    Rectangle {
        anchors.fill: parent
        radius: 3
        color: "#d9161d19"
        border.width: 1
        border.color: "#805d7163"
    }

    Row {
        anchors.centerIn: parent
        spacing: 8

        Row {
            spacing: 2
            visible: rewardEffect.gold > 0

            MultiResolutionImage {
                anchors.verticalCenter: parent.verticalCenter
                width: 10
                height: 10
                source: "qrc:/assets/ui/gold.png"
                smooth: false
            }

            AppText {
                anchors.verticalCenter: parent.verticalCenter
                color: "#f2c438"
                font.pixelSize: 9
                font.weight: Font.Bold
                text: "+" + rewardEffect.gold
            }
        }

        Row {
            spacing: 2
            visible: rewardEffect.food > 0

            MultiResolutionImage {
                anchors.verticalCenter: parent.verticalCenter
                width: 10
                height: 10
                source: "qrc:/assets/ui/food.png"
                smooth: false
            }

            AppText {
                anchors.verticalCenter: parent.verticalCenter
                color: "#e0955a"
                font.pixelSize: 9
                font.weight: Font.Bold
                text: "+" + rewardEffect.food
            }
        }
    }

    ParallelAnimation {
        running: true

        NumberAnimation {
            target: rewardRise
            property: "y"
            from: 0
            to: -20
            duration: 1400
            easing.type: Easing.OutCubic
        }

        SequentialAnimation {
            PauseAnimation {
                duration: 500
            }

            NumberAnimation {
                target: rewardEffect
                property: "opacity"
                from: 1
                to: 0
                duration: 900
                easing.type: Easing.InQuad
            }
        }

        onFinished: rewardEffect.removeEntity()
    }
}
