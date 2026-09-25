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
    property var baseModel
    property var baseActionsModel
    property var playerBaseId: 0
    property int playerGold: 0
    property int playerFood: 0
    property bool gameRunning: true
    property real gameTimeScale: 1
    property bool spawnEnabled: true
    property int contextColumn: 0
    property int contextRow: 0
    property var selectedCreatureIds: []
    property var selectedBaseId: 0
    property string playerGroup
    property int idleDotCount: 1
    readonly property var targetedIds: {
        const targetIds = []

        for (let index = 0; index < creatureRepeater.count; ++index) {
            const creature = creatureRepeater.itemAt(index) as CreatureView

            if (creature !== null
                    && creature.selected
                    && creature.targetId > 0
                    && targetIds.indexOf(creature.targetId) === -1) {
                targetIds.push(creature.targetId)
            }
        }

        return targetIds
    }

    signal baseSpawnRequested(string identifier, int column, int row)
    signal baseActionRequested(string actionKey)
    signal creatureTypeSpawnRequested(
        string identifier,
        int column,
        int row
    )
    signal walkRequested(var creatureId, int column, int row)
    signal attackRequested(var creatureId, var targetId)

    function isOwn(group) {
        return playerGroup.length === 0 || group === playerGroup
    }

    function baseAt(column, row) {
        for (let index = 0; index < baseRepeater.count; ++index) {
            const base = baseRepeater.itemAt(index) as BaseView

            if (base !== null
                    && column >= base.column
                    && column < base.column + base.baseSize
                    && row >= base.row
                    && row < base.row + base.baseSize) {
                return base
            }
        }

        return null
    }

    function hasBase(identifier) {
        for (let index = 0; index < baseRepeater.count; ++index) {
            const base = baseRepeater.itemAt(index) as BaseView

            if (base !== null && base.baseTypeIdentifier === identifier) {
                return true
            }
        }

        return false
    }

    function isCreatureSelected(creatureId) {
        return selectedCreatureIds.indexOf(creatureId) !== -1
    }

    // Front-most creature whose drawn graphic contains the point.
    function creatureAt(position, own) {
        let frontCreature = null

        for (let index = 0; index < creatureRepeater.count; ++index) {
            const creature = creatureRepeater.itemAt(index) as CreatureView

            if (creature === null || root.isOwn(creature.creatureGroup) !== own) {
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
        const creature = creatureAt(position, true)
        const base = creature === null
            ? baseAt(Math.floor(position.x / tileSize), Math.floor(position.y / tileSize))
            : null

        selectedCreatureIds = creature === null ? [] : [creature.creatureId]
        selectedBaseId = base !== null && isOwn(base.baseGroup) ? base.baseId : 0
    }

    function selectInRectangle(left, top, right, bottom) {
        const selectedIds = []

        for (let index = 0; index < creatureRepeater.count; ++index) {
            const creature = creatureRepeater.itemAt(index) as CreatureView

            if (creature !== null
                    && root.isOwn(creature.creatureGroup)
                    && creature.x + creature.width >= left
                    && creature.x <= right
                    && creature.y + creature.height >= top
                    && creature.y <= bottom) {
                selectedIds.push(creature.creatureId)
            }
        }

        selectedCreatureIds = selectedIds
        selectedBaseId = 0
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
        interval: Math.max(1, 400 / root.gameTimeScale)
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
                const enemy = root.creatureAt(eventPoint.position, false)
                const base = enemy === null ? root.baseAt(column, row) : null
                const targetId = enemy !== null
                    ? enemy.creatureId
                    : base !== null && !root.isOwn(base.baseGroup) ? base.baseId : 0

                moveTargetMarker.x = column * root.tileSize
                moveTargetMarker.y = row * root.tileSize
                moveTargetMarker.attack = targetId !== 0
                moveTargetAnimation.restart()

                for (const creatureId of root.selectedCreatureIds) {
                    if (targetId !== 0) {
                        root.attackRequested(creatureId, targetId)
                    } else {
                        root.walkRequested(creatureId, column, row)
                    }
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

        property bool attack: false

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
            color: moveTargetMarker.attack ? "#ff4a3d" : "#f4df5a"
        }

        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.75
            height: 2
            radius: 1
            rotation: -45
            color: moveTargetMarker.attack ? "#ff4a3d" : "#f4df5a"
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
            text: qsTr("Spawn Minotaur Base")
            enabled: root.spawnEnabled && !root.hasBase("minotaur_base")

            onTriggered: root.baseSpawnRequested(
                "minotaur_base",
                root.contextColumn,
                root.contextRow
            )
        }

        MenuItem {
            text: qsTr("Spawn Orc Base")
            enabled: root.spawnEnabled && !root.hasBase("orc_base")

            onTriggered: root.baseSpawnRequested(
                "orc_base",
                root.contextColumn,
                root.contextRow
            )
        }

        MenuItem {
            text: qsTr("Spawn Dwarf Base")
            enabled: root.spawnEnabled && !root.hasBase("dwarf_base")

            onTriggered: root.baseSpawnRequested(
                "dwarf_base",
                root.contextColumn,
                root.contextRow
            )
        }

        MenuSeparator {}

        MenuItem {
            text: qsTr("Spawn Deer")
            enabled: root.spawnEnabled

            onTriggered: root.creatureTypeSpawnRequested(
                "deer",
                root.contextColumn,
                root.contextRow
            )
        }

        MenuItem {
            text: qsTr("Spawn Wolf")
            enabled: root.spawnEnabled

            onTriggered: root.creatureTypeSpawnRequested(
                "wolf",
                root.contextColumn,
                root.contextRow
            )
        }

        MenuItem {
            text: qsTr("Spawn Troll")
            enabled: root.spawnEnabled

            onTriggered: root.creatureTypeSpawnRequested(
                "troll",
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

    // Creatures and bases are depth-sorted together inside this layer.
    Item {
        id: creatureLayer

        anchors.fill: parent
        z: 2

        Repeater {
            id: baseRepeater

            model: root.baseModel

            delegate: BaseView {
                tileSize: root.tileSize
                rangeVisible: root.visionRangesVisible
                overlayParent: creatureOverlayLayer
                isPlayerBase: baseId === root.playerBaseId
                actionsModel: root.baseActionsModel
                gold: root.playerGold
                food: root.playerFood
                gameRunning: root.gameRunning
                gameTimeScale: root.gameTimeScale
                mapScale: root.scale
                selected: baseId === root.selectedBaseId
                targeted: root.targetedIds.indexOf(baseId) !== -1

                onActionRequested: function(actionKey) {
                    root.baseActionRequested(actionKey)
                }
            }
        }

        Repeater {
            id: creatureRepeater

            model: root.creatureModel

            delegate: CreatureView {
                tileSize: root.tileSize
                selected: root.isCreatureSelected(creatureId)
                targeted: root.targetedIds.indexOf(creatureId) !== -1
                visionRangeVisible: root.visionRangesVisible
                idleDotCount: root.idleDotCount
                gameTimeScale: root.gameTimeScale
                mapScale: root.scale
                areaWidth: root.width
                areaHeight: root.height
                overlayParent: creatureOverlayLayer
            }
        }
    }

    Item {
        id: effectLayer

        anchors.fill: parent
        z: 2.5
    }

    EntityManager {
        id: entityManager

        entityContainer: effectLayer
    }

    Component {
        id: areaExplosionComponent

        AreaExplosion {}
    }

    function showAreaAttack(column, row, radius, color) {
        entityManager.createEntityFromComponentWithProperties(areaExplosionComponent, {
            x: (column - radius) * root.tileSize,
            y: (row - radius) * root.tileSize,
            tileSize: root.tileSize,
            radius: radius,
            color: color,
            gameTimeScale: root.gameTimeScale
        })
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
