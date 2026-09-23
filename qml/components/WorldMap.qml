import QtQuick 2.15
import QtQuick.Controls 2.15
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

    function creatureAt(position) {
        for (let index = creatureRepeater.count - 1; index >= 0; --index) {
            const creature = creatureRepeater.itemAt(index)

            if (creature !== null
                    && position.x >= creature.x
                    && position.x <= creature.x + creature.width
                    && position.y >= creature.y
                    && position.y <= creature.y + creature.height) {
                return creature
            }
        }

        return null
    }

    function selectAt(position) {
        const creature = creatureAt(position)
        selectedCreatureIds = creature === null ? [] : [creature.creatureId]
    }

    function selectInRectangle(left, top, right, bottom) {
        const selectedIds = []

        for (let index = 0; index < creatureRepeater.count; ++index) {
            const creature = creatureRepeater.itemAt(index)

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

    Repeater {
        id: creatureRepeater

        model: root.creatureModel

        delegate: Item {
            id: creatureDelegate

            required property var creatureId
            required property string creatureTypeIdentifier
            required property int column
            required property int row
            required property string creatureName
            required property string creatureGroup
            required property color creatureColor
            required property color markerColor
            required property int health
            required property int maximumHealth
            required property int attack
            required property int attackRange
            required property int visionRange
            required property real movementSpeed
            required property string creatureDirection
            required property string creatureState
            required property int alertRevision
            required property int damageAmount
            required property int damageRevision
            required property int attackRevision
            required property int walkCommandRevision
            property bool alertVisible: false
            property bool goVisible: false
            readonly property bool hasSprite:
                creatureTypeIdentifier === "minotaur"
            readonly property bool spriteWalking:
                xMovementAnimation.running || yMovementAnimation.running
            readonly property string spriteAnimationName:
                creatureDirection + (spriteWalking ? "_walk" : "_idle")
            readonly property real effectiveMovementSpeed:
                creatureState === "idle"
                    ? movementSpeed * 0.5
                    : movementSpeed
            readonly property int movementDuration: Math.max(
                1,
                Math.round(1000 / Math.max(effectiveMovementSpeed, 0.01))
            )
            readonly property real healthRatio: Math.max(
                0,
                Math.min(1, health / Math.max(maximumHealth, 1))
            )
            readonly property color healthColor: healthRatio > 0.8
                ? "#46c95b"
                : healthRatio >= 0.3 ? "#e2c94f" : "#d94a45"

            x: column * root.tileSize + 2
            y: row * root.tileSize + 2
            width: root.tileSize - 4
            height: root.tileSize - 4
            z: 2

            Rectangle {
                id: creatureBody

                anchors.fill: parent
                radius: width / 2
                color: creatureDelegate.hasSprite
                    ? "transparent"
                    : creatureColor
                border.width: root.isCreatureSelected(creatureId)
                    ? 2
                    : creatureDelegate.hasSprite ? 0 : 1
                border.color: root.isCreatureSelected(creatureId)
                    ? "#f4df5a"
                    : Qt.darker(creatureColor, 1.5)

                transform: Translate {
                    id: creatureAttackTranslation
                }

                Rectangle {
                    anchors.centerIn: parent
                    width: parent.width * 0.35
                    height: width
                    radius: width / 2
                    color: markerColor
                    visible: !creatureDelegate.hasSprite && markerColor.a > 0
                }

                GameSpriteSequence {
                    id: creatureSprite

                    anchors.centerIn: parent
                    width: 32
                    height: 32
                    defaultSource: creatureDelegate.hasSprite
                        ? "qrc:/assets/creatures/minotaur/+hd2/minotaur.png"
                        : ""
                    interpolate: false
                    running: creatureDelegate.hasSprite
                        && creatureDelegate.spriteWalking
                    visible: creatureDelegate.hasSprite

                    GameSprite {
                        name: "north_idle"
                        frameX: 0
                        frameY: 0
                        frameWidth: 32
                        frameHeight: 32
                        frameCount: 1
                        frameDuration: 1000
                    }

                    GameSprite {
                        name: "north_walk"
                        frameX: 32
                        frameY: 0
                        frameWidth: 32
                        frameHeight: 32
                        frameCount: 2
                        frameDuration: Math.max(
                            100,
                            creatureDelegate.movementDuration / 2
                        )
                    }

                    GameSprite {
                        name: "south_idle"
                        frameX: 0
                        frameY: 32
                        frameWidth: 32
                        frameHeight: 32
                        frameCount: 1
                        frameDuration: 1000
                    }

                    GameSprite {
                        name: "south_walk"
                        frameX: 32
                        frameY: 32
                        frameWidth: 32
                        frameHeight: 32
                        frameCount: 2
                        frameDuration: Math.max(
                            100,
                            creatureDelegate.movementDuration / 2
                        )
                    }

                    GameSprite {
                        name: "west_idle"
                        frameX: 0
                        frameY: 64
                        frameWidth: 32
                        frameHeight: 32
                        frameCount: 1
                        frameDuration: 1000
                    }

                    GameSprite {
                        name: "west_walk"
                        frameX: 32
                        frameY: 64
                        frameWidth: 32
                        frameHeight: 32
                        frameCount: 2
                        frameDuration: Math.max(
                            100,
                            creatureDelegate.movementDuration / 2
                        )
                    }

                    GameSprite {
                        name: "east_idle"
                        frameX: 0
                        frameY: 96
                        frameWidth: 32
                        frameHeight: 32
                        frameCount: 1
                        frameDuration: 1000
                    }

                    GameSprite {
                        name: "east_walk"
                        frameX: 32
                        frameY: 96
                        frameWidth: 32
                        frameHeight: 32
                        frameCount: 2
                        frameDuration: Math.max(
                            100,
                            creatureDelegate.movementDuration / 2
                        )
                    }
                }
            }

            onSpriteAnimationNameChanged: {
                if (hasSprite) {
                    creatureSprite.jumpTo(spriteAnimationName)
                }
            }

            Component.onCompleted: {
                if (hasSprite) {
                    creatureSprite.jumpTo(spriteAnimationName)
                }
            }

            SequentialAnimation {
                id: creatureAttackAnimation

                NumberAnimation {
                    target: creatureAttackTranslation
                    property: "y"
                    to: -3
                    duration: 80
                    easing.type: Easing.OutCubic
                }

                NumberAnimation {
                    target: creatureAttackTranslation
                    property: "y"
                    to: 0
                    duration: 110
                    easing.type: Easing.InCubic
                }
            }

            Text {
                id: creatureDamageText

                anchors.horizontalCenter: parent.horizontalCenter
                color: "#f04444"
                font.bold: true
                font.pixelSize: 10
                opacity: 0
                style: Text.Outline
                styleColor: "#000000"
                text: damageAmount > 0 ? "-" + damageAmount : ""
                z: 14
            }

            ParallelAnimation {
                id: creatureDamageAnimation

                NumberAnimation {
                    target: creatureDamageText
                    property: "y"
                    from: creatureDelegate.height / 2
                        - creatureDamageText.height / 2
                    to: -root.tileSize
                    duration: 900
                    easing.type: Easing.OutCubic
                }

                NumberAnimation {
                    target: creatureDamageText
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 900
                }
            }

            Timer {
                id: creatureAlertTimer

                interval: 750

                onTriggered: creatureDelegate.alertVisible = false
            }

            Timer {
                id: creatureGoTimer

                interval: 600

                onTriggered: creatureDelegate.goVisible = false
            }

            onAlertRevisionChanged: {
                if (alertRevision > 0) {
                    alertVisible = true
                    creatureAlertTimer.restart()
                }
            }

            onWalkCommandRevisionChanged: {
                if (walkCommandRevision > 0) {
                    goVisible = true
                    creatureGoTimer.restart()
                }
            }

            onDamageRevisionChanged: {
                if (damageRevision > 0) {
                    creatureDamageAnimation.restart()
                }
            }

            onAttackRevisionChanged: {
                if (attackRevision > 0) {
                    creatureAttackAnimation.restart()
                }
            }

            Item {
                id: creatureIdentity

                readonly property bool placeAbove:
                    creatureDelegate.y >= height + 2

                anchors.horizontalCenter: parent.horizontalCenter
                y: placeAbove ? -height - 2 : parent.height + 2
                width: 56
                height: 15
                z: 10

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    width: Math.ceil(creatureStatusText.implicitWidth) + 4
                    height: 10
                    radius: 2
                    color: creatureHover.hovered ? "#b8000000" : "transparent"
                    visible: creatureStatusText.text.length > 0

                    Text {
                        id: creatureStatusText

                        anchors.centerIn: parent
                        color: creatureHover.hovered
                            ? creatureDelegate.healthColor
                            : creatureDelegate.alertVisible
                                ? "#ffdc4f"
                                : "#f1f4f2"
                        font.pixelSize: 8
                        font.weight: Font.DemiBold
                        font.letterSpacing: -0.2
                        renderType: Text.QtRendering
                        style: Text.Outline
                        styleColor: "#000000"
                        text: creatureHover.hovered
                            ? creatureName
                            : creatureDelegate.alertVisible
                                ? "!"
                                : creatureDelegate.goVisible
                                    ? qsTr("Go")
                                    : creatureState === "idle"
                                        ? [".", "..", "..."][root.idleDotCount - 1]
                                        : ""
                    }
                }

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
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
                        width: (parent.width - 2) * creatureDelegate.healthRatio
                        height: parent.height - 2
                        radius: height / 2
                        color: creatureDelegate.healthColor
                    }
                }
            }

            Rectangle {
                anchors.centerIn: parent
                width: visionRange * 2 * root.tileSize
                height: width
                radius: width / 2
                color: "transparent"
                border.width: 1
                border.color: "#33e8d878"
                visible: root.visionRangesVisible
                z: -1
            }

            HoverHandler {
                id: creatureHover
            }

            Rectangle {
                id: creatureDetails

                readonly property bool fitsLeft:
                    creatureDelegate.x >= width + 4
                readonly property bool fitsRight:
                    root.width - creatureDelegate.x - creatureDelegate.width >= width + 4
                readonly property bool fitsAbove:
                    creatureDelegate.y >= height + 4
                readonly property bool fitsBelow:
                    root.height - creatureDelegate.y - creatureDelegate.height >= height + 4
                readonly property bool placeLeft:
                    fitsLeft || (!fitsRight && creatureDelegate.x > root.width / 2)
                readonly property bool placeAbove:
                    fitsAbove || (!fitsBelow && creatureDelegate.y > root.height / 2)

                x: placeLeft ? -width - 4 : parent.width + 4
                y: placeAbove ? -height - 4 : parent.height + 4
                width: creatureDetailsText.implicitWidth + 8
                height: creatureDetailsText.implicitHeight + 6
                radius: 2
                color: "#e6111814"
                border.width: 1
                border.color: "#758579"
                visible: opacity > 0
                opacity: creatureHover.hovered ? 1 : 0
                z: 10

                Text {
                    id: creatureDetailsText

                    anchors.centerIn: parent
                    color: "#f1f4f2"
                    font.pixelSize: 8
                    text: qsTr(
                        "ATK: %1  Range: %2\nVision: %3  Speed: %4"
                    ).arg(attack)
                        .arg(attackRange)
                        .arg(visionRange)
                        .arg(movementSpeed)
                }

                Behavior on opacity {
                    NumberAnimation {
                        duration: 120
                    }
                }
            }

            Behavior on x {
                NumberAnimation {
                    id: xMovementAnimation

                    duration: creatureDelegate.movementDuration
                    easing.type: Easing.Linear
                }
            }

            Behavior on y {
                NumberAnimation {
                    id: yMovementAnimation

                    duration: creatureDelegate.movementDuration
                    easing.type: Easing.Linear
                }
            }
        }
    }

    Rectangle {
        x: Math.min(selectionDrag.startX, selectionDrag.currentX)
        y: Math.min(selectionDrag.startY, selectionDrag.currentY)
        width: Math.abs(selectionDrag.currentX - selectionDrag.startX)
        height: Math.abs(selectionDrag.currentY - selectionDrag.startY)
        color: "#304d8fd8"
        border.width: 1
        border.color: "#8eb9ff"
        visible: selectionDrag.active
        z: 20
    }
}
