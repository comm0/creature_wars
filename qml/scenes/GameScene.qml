import QtQuick
import Felgo 4.0
import "../components"

Scene {
    id: gameScene

    required property var gameBackend

    width: 640
    height: 360
    scaleMode: "letterbox"
    opacity: 0
    visible: opacity > 0
    enabled: visible

    Behavior on opacity {
        NumberAnimation {
            duration: 250
        }
    }

    Rectangle {
        anchors.fill: gameScene.gameWindowAnchorItem
        color: "#111814"
    }

    WorldMap {
        id: worldMap

        x: 0
        y: topBar.height
        columns: gameScene.gameBackend.mapColumnCount
        rows: gameScene.gameBackend.mapRowCount
        tileSize: 16
        creatureModel: gameScene.gameBackend.creaturesModel
        baseModel: gameScene.gameBackend.basesModel
        spawnEnabled: gameScene.gameBackend.running
        visionRangesVisible: rangeToggle.checked

        onBaseSpawnRequested: function(identifier, column, row) {
            gameScene.gameBackend.spawnBase(identifier, column, row)
        }

        onCreatureSpawnRequested: function(baseId) {
            gameScene.gameBackend.spawnCreatureFromBase(baseId)
        }

        onCreatureTypeSpawnRequested: function(identifier, column, row) {
            gameScene.gameBackend.spawnCreature(identifier, column, row)
        }

        onWalkRequested: function(creatureId, column, row) {
            gameScene.gameBackend.walkCreature(creatureId, column, row)
        }
    }

    HudBar {
        id: topBar

        anchors.left: gameScene.gameWindowAnchorItem.left
        anchors.right: gameScene.gameWindowAnchorItem.right
        y: 0
        height: 16
        borderOnTop: false

        Row {
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6

            HudStat {
                iconType: IconType.shield
                text: gameScene.gameBackend.playerBaseName
            }

            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: levelText.implicitWidth + 6
                height: 10
                radius: 2
                color: "#2f6b3a"

                AppText {
                    id: levelText

                    anchors.centerIn: parent
                    color: "#e8eee9"
                    font.pixelSize: 7
                    font.weight: Font.DemiBold
                    text: qsTr("Lv %1").arg(gameScene.gameBackend.playerBaseLevel)
                }
            }
        }

        Row {
            anchors.right: heartbeatIndicator.left
            anchors.rightMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            spacing: 12

            HudStat {
                imageSource: "qrc:/assets/ui/gold.png"
                textColor: "#f2c438"
                text: gameScene.gameBackend.playerGold
            }

            HudStat {
                imageSource: "qrc:/assets/ui/food.png"
                textColor: "#e0955a"
                text: gameScene.gameBackend.playerFood
            }
        }

        Rectangle {
            id: heartbeatIndicator

            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            width: 6
            height: 6
            radius: width / 2
            color: "#7ddd98"
            opacity: 0.35
        }
    }

    HudBar {
        id: bottomBar

        anchors.left: gameScene.gameWindowAnchorItem.left
        anchors.right: gameScene.gameWindowAnchorItem.right
        y: worldMap.y + worldMap.height
        height: gameScene.height - y

        Row {
            anchors.centerIn: parent
            spacing: 6

            HudToggle {
                iconType: IconType.th
                text: qsTr("Grid")
                checked: worldMap.gridVisible

                onToggled: worldMap.gridVisible = !worldMap.gridVisible
            }

            HudToggle {
                id: rangeToggle

                iconType: IconType.bullseye
                text: qsTr("Range")

                onToggled: checked = !checked
            }

            HudToggle {
                iconType: IconType.flag
                text: qsTr("Aggressive")
                checked: gameScene.gameBackend.aggressive

                onToggled: gameScene.gameBackend.aggressive = !checked
            }

            HudToggle {
                iconType: gameScene.gameBackend.running ? IconType.pause : IconType.play
                text: gameScene.gameBackend.running ? qsTr("Running") : qsTr("Paused")
                checked: gameScene.gameBackend.running

                onToggled: {
                    if (gameScene.gameBackend.running) {
                        gameScene.gameBackend.stop()
                    } else {
                        gameScene.gameBackend.start()
                    }
                }
            }
        }
    }

    SequentialAnimation {
        id: heartbeatAnimation

        NumberAnimation {
            target: heartbeatIndicator
            property: "opacity"
            to: 1
            duration: 120
        }

        NumberAnimation {
            target: heartbeatIndicator
            property: "opacity"
            to: 0.35
            duration: 500
        }
    }

    Connections {
        target: gameScene.gameBackend

        function onHeartbeat() {
            heartbeatAnimation.restart()
        }

        function onCreatureRemoved(creatureId) {
            worldMap.removeSelectedCreature(creatureId)
        }
    }
}
