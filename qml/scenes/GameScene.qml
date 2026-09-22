import QtQuick 2.15
import QtQuick.Controls 2.15
import Felgo 4.0
import "../components"

Scene {
    id: gameScene

    required property var gameBackend

    width: 640
    height: 360
    scaleMode: "letterbox"

    Rectangle {
        anchors.fill: parent
        color: "#111814"
    }

    WorldMap {
        id: worldMap

        x: 0
        y: 0
        columns: gameScene.gameBackend.mapColumnCount
        rows: gameScene.gameBackend.mapRowCount
        tileSize: 16
        creatureModel: gameScene.gameBackend.creaturesModel
        spawnEnabled: gameScene.gameBackend.running
        visionRangesVisible: showRangeCheckBox.checked

        onSpawnRequested: function(identifier, column, row) {
            gameScene.gameBackend.spawnCreature(identifier, column, row)
        }

        onWalkRequested: function(creatureId, column, row) {
            gameScene.gameBackend.walkCreature(creatureId, column, row)
        }
    }

    Rectangle {
        x: 0
        y: 320
        width: parent.width
        height: 40
        color: "#17231c"

        Rectangle {
            width: parent.width
            height: 1
            anchors.top: parent.top
            color: "#405348"
        }

        Rectangle {
            id: heartbeatIndicator

            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            width: 10
            height: 10
            radius: width / 2
            color: "#7ddd98"
            opacity: 0.35
        }

        Row {
            anchors.centerIn: parent
            spacing: 16

            CheckBox {
                text: qsTr("Show grid")
                checked: true
                palette.windowText: "#e8eee9"

                onToggled: worldMap.gridVisible = checked
            }

            CheckBox {
                id: showRangeCheckBox

                text: qsTr("Show range")
                checked: false
                palette.windowText: "#e8eee9"
            }

            CheckBox {
                text: qsTr("Aggressive")
                checked: gameScene.gameBackend.aggressive
                palette.windowText: "#e8eee9"

                onToggled: gameScene.gameBackend.aggressive = checked
            }

            Button {
                text: qsTr("Start")
                enabled: !gameScene.gameBackend.running

                onClicked: gameScene.gameBackend.start()
            }

            Button {
                text: qsTr("Stop")
                enabled: gameScene.gameBackend.running

                onClicked: gameScene.gameBackend.stop()
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
