pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import Felgo 4.0

Item {
    id: root

    required property int columns
    required property int rows
    property int tileSize: 16
    property bool gridVisible: true
    property bool visionRangesVisible: false
    property var creatureModel
    property bool spawnEnabled: true
    property int contextColumn: 0
    property int contextRow: 0
    property var selectedCreatureIds: []
    property int idleDotCount: 1

    signal spawnRequested(string identifier, int column, int row)
    signal walkRequested(var creatureId, int column, int row)

    function isCreatureSelected(creatureId) {
        return selectedCreatureIds.indexOf(creatureId) !== -1
    }

    // Front-most creature whose drawn graphic contains the point.
    function creatureAt(position) {
        let frontCreature = null

        for (let index = 0; index < creatureRepeater.count; ++index) {
            const creature = creatureRepeater.itemAt(index) as CreatureView

            if (creature === null) {
                continue
            }

            const left = creature.x + creature.visualLeft
            const top = creature.y + creature.visualTop

            if (position.x >= left
                    && position.x <= left + creature.visualWidth
                    && position.y >= top
                    && position.y <= top + creature.visualHeight
                    && (frontCreature === null || creature.z > frontCreature.z)) {
                frontCreature = creature
            }
        }

        return frontCreature
    }

    function selectAt(position) {
        const creature = creatureAt(position)
        selectedCreatureIds = creature === null ? [] : [creature.creatureId]
    }

    function selectInRectangle(left, top, right, bottom) {
        const selectedIds = []

        for (let index = 0; index < creatureRepeater.count; ++index) {
            const creature = creatureRepeater.itemAt(index) as CreatureView

            if (creature !== null
                    && creature.x + creature.width >= left
                    && creature.x <= right
                    && creature.y + creature.height >= top
                    && creature.y <= bottom) {
                selectedIds.push(creature.creatureId)
            }
        }

        selectedCreatureIds = selectedIds
    }

    function removeSelectedCreature(creatureId) {
        const selectedIds = selectedCreatureIds.slice()
        const index = selectedIds.indexOf(creatureId)

        if (index !== -1) {
            selectedIds.splice(index, 1)
            selectedCreatureIds = selectedIds
        }
    }

    width: columns * tileSize
    height: rows * tileSize
    clip: true

    Timer {
        interval: 400
        running: true
        repeat: true

        onTriggered: root.idleDotCount = root.idleDotCount % 3 + 1
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton

        onTapped: function(eventPoint) {
            root.selectAt(eventPoint.position)
        }
    }

    TapHandler {
        acceptedButtons: Qt.RightButton

        onTapped: function(eventPoint) {
            const column = Math.floor(eventPoint.position.x / root.tileSize)
            const row = Math.floor(eventPoint.position.y / root.tileSize)

            if (root.selectedCreatureIds.length > 0) {
                moveTargetMarker.x = column * root.tileSize
                moveTargetMarker.y = row * root.tileSize
                moveTargetAnimation.restart()

                for (const creatureId of root.selectedCreatureIds) {
                    root.walkRequested(creatureId, column, row)
                }

                return
            }

            root.contextColumn = column
            root.contextRow = row
            spawnMenu.popup(eventPoint.position.x, eventPoint.position.y)
        }
    }

    DragHandler {
        id: selectionDrag

        property real startX: 0
        property real startY: 0
        property real currentX: 0
        property real currentY: 0
        property bool selectionStarted: false

        target: null
        acceptedButtons: Qt.LeftButton

        onActiveChanged: {
            if (active) {
                startX = centroid.pressPosition.x
                startY = centroid.pressPosition.y
                currentX = centroid.position.x
                currentY = centroid.position.y
                selectionStarted = true
                return
            }

            if (selectionStarted) {
                root.selectInRectangle(
                    Math.min(startX, currentX),
                    Math.min(startY, currentY),
                    Math.max(startX, currentX),
                    Math.max(startY, currentY)
                )
                selectionStarted = false
            }
        }

        onCentroidChanged: {
            if (active) {
                currentX = centroid.position.x
                currentY = centroid.position.y
            }
        }
    }

    Item {
        id: moveTargetMarker

        width: root.tileSize
        height: root.tileSize
        opacity: 0
        z: 19

        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.75
            height: 2
            radius: 1
            rotation: 45
            color: "#f4df5a"
        }

        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.75
            height: 2
            radius: 1
            rotation: -45
            color: "#f4df5a"
        }
    }

    SequentialAnimation {
        id: moveTargetAnimation

        PropertyAction {
            target: moveTargetMarker
            property: "opacity"
            value: 1
        }

        PropertyAction {
            target: moveTargetMarker
            property: "scale"
            value: 0.55
        }

        ParallelAnimation {
            NumberAnimation {
                target: moveTargetMarker
                property: "opacity"
                to: 0
                duration: 450
            }

            NumberAnimation {
                target: moveTargetMarker
                property: "scale"
                to: 1.35
                duration: 450
                easing.type: Easing.OutCubic
            }
        }
    }

    Menu {
        id: spawnMenu

        MenuItem {
            text: qsTr("Spawn Minotaur")
            enabled: root.spawnEnabled

            onTriggered: root.spawnRequested(
                "minotaur",
                root.contextColumn,
                root.contextRow
            )
        }

        MenuItem {
            text: qsTr("Spawn Orc")
            enabled: root.spawnEnabled

            onTriggered: root.spawnRequested(
                "orc",
                root.contextColumn,
                root.contextRow
            )
        }
    }

    Grid {
        anchors.fill: parent
        columns: root.columns
        rows: root.rows
        spacing: 0

        Repeater {
            model: root.columns * root.rows

            delegate: Rectangle {
                width: root.tileSize
                height: root.tileSize
                color: "#4f824b"
                border.width: root.gridVisible ? 1 : 0
                border.color: "#315a35"
            }
        }
    }

    // Creatures are z-sorted among themselves inside this layer.
    Item {
        id: creatureLayer

        anchors.fill: parent
        z: 2

        Repeater {
            id: creatureRepeater

            model: root.creatureModel

            delegate: CreatureView {
                tileSize: root.tileSize
                selected: root.isCreatureSelected(creatureId)
                visionRangeVisible: root.visionRangesVisible
                idleDotCount: root.idleDotCount
                areaWidth: root.width
                areaHeight: root.height
                overlayParent: creatureOverlayLayer
            }
        }
    }

    Item {
        id: creatureOverlayLayer

        anchors.fill: parent
        z: 3
    }

    SelectionBox {
        startX: selectionDrag.startX
        startY: selectionDrag.startY
        currentX: selectionDrag.currentX
        currentY: selectionDrag.currentY
        visible: selectionDrag.active
        z: 20
    }
}
