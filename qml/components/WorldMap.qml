import QtQuick 2.15
import QtQuick.Controls 2.15

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

        delegate: Rectangle {
            id: creatureDelegate

            required property var creatureId
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
            required property string creatureState
            required property int alertRevision
            readonly property int movementDuration: Math.max(
                1,
                Math.round(1000 / Math.max(movementSpeed, 0.01))
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
            radius: width / 2
            color: creatureColor
            border.width: root.isCreatureSelected(creatureId) ? 2 : 1
            border.color: root.isCreatureSelected(creatureId)
                ? "#f4df5a"
                : Qt.darker(creatureColor, 1.5)
            z: 2

            Text {
                id: creatureStateText

                anchors.horizontalCenter: parent.horizontalCenter
                y: creatureIdentity.placeAbove
                    ? -creatureIdentity.height - height - 4
                    : parent.height + creatureIdentity.height + 4
                color: "#f1f4f2"
                font.pixelSize: 9
                style: Text.Outline
                styleColor: "#111814"
                opacity: creatureAlert.opacity > 0 ? 0 : 1
                text: creatureState === "walk"
                    ? qsTr("walk")
                    : [".", "..", "..."][root.idleDotCount - 1]
                z: 11
            }

            Text {
                id: creatureAlert

                anchors.horizontalCenter: parent.horizontalCenter
                y: creatureIdentity.placeAbove
                    ? -creatureIdentity.height - height - 4
                    : parent.height + creatureIdentity.height + 4
                color: "#ffdc4f"
                font.bold: true
                font.pixelSize: 13
                opacity: 0
                style: Text.Outline
                styleColor: "#111814"
                text: "!"
                z: 12
            }

            SequentialAnimation {
                id: creatureAlertAnimation

                PropertyAction {
                    target: creatureAlert
                    property: "opacity"
                    value: 1
                }

                PauseAnimation {
                    duration: 500
                }

                NumberAnimation {
                    target: creatureAlert
                    property: "opacity"
                    to: 0
                    duration: 250
                }
            }

            onAlertRevisionChanged: {
                if (alertRevision > 0) {
                    creatureAlertAnimation.restart()
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
                    width: Math.ceil(creatureNameText.implicitWidth) + 4
                    height: 10
                    radius: 2
                    color: "#b8000000"

                    Text {
                        id: creatureNameText

                        anchors.centerIn: parent
                        color: creatureDelegate.healthColor
                        font.pixelSize: 8
                        font.weight: Font.DemiBold
                        font.letterSpacing: -0.2
                        renderType: Text.QtRendering
                        text: creatureName
                    }
                }

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    width: 28
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
                anchors.centerIn: parent
                width: parent.width * 0.35
                height: width
                radius: width / 2
                color: markerColor
                visible: markerColor.a > 0
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
                    duration: creatureDelegate.movementDuration
                    easing.type: Easing.Linear
                }
            }

            Behavior on y {
                NumberAnimation {
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
