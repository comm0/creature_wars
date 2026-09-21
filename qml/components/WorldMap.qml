import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root

    required property int columns
    required property int rows
    property int tileSize: 16
    property bool gridVisible: true
    property var creatureModel
    property bool spawnEnabled: true
    property int contextColumn: 0
    property int contextRow: 0
    property var selectedCreatureIds: []

    signal spawnRequested(string identifier, int column, int row)

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

    width: columns * tileSize
    height: rows * tileSize
    clip: true

    TapHandler {
        acceptedButtons: Qt.LeftButton

        onTapped: function(eventPoint) {
            root.selectAt(eventPoint.position)
        }
    }

    TapHandler {
        acceptedButtons: Qt.RightButton

        onTapped: function(eventPoint) {
            root.contextColumn = Math.floor(eventPoint.position.x / root.tileSize)
            root.contextRow = Math.floor(eventPoint.position.y / root.tileSize)
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
            required property int attack
            required property int attackRange
            required property int visionRange

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

            Rectangle {
                anchors.centerIn: parent
                width: (visionRange * 2 + 1) * root.tileSize
                height: width
                color: "transparent"
                border.width: 1
                border.color: "#e8d878"
                visible: opacity > 0
                opacity: creatureHover.hovered ? 1 : 0
                z: -1

                Behavior on opacity {
                    NumberAnimation {
                        duration: 150
                    }
                }
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
                width: creatureDetailsText.implicitWidth + 12
                height: creatureDetailsText.implicitHeight + 10
                radius: 3
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
                    font.pixelSize: 10
                    text: qsTr(
                        "Name: %1\nGroup: %2\nHP: %3\nATK: %4\nATK range: %5\nVision range: %6"
                    ).arg(creatureName)
                        .arg(creatureGroup)
                        .arg(health)
                        .arg(attack)
                        .arg(attackRange)
                        .arg(visionRange)
                }

                Behavior on opacity {
                    NumberAnimation {
                        duration: 120
                    }
                }
            }

            Behavior on x {
                NumberAnimation {
                    duration: 700
                    easing.type: Easing.Linear
                }
            }

            Behavior on y {
                NumberAnimation {
                    duration: 700
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
