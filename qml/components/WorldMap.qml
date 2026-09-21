import QtQuick 2.15

Item {
    id: root

    required property int columns
    required property int rows
    property int tileSize: 16
    property bool gridVisible: true
    property var creatureModel

    width: columns * tileSize
    height: rows * tileSize
    clip: true

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
        model: root.creatureModel

        delegate: Rectangle {
            id: creatureDelegate

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
            border.width: 1
            border.color: Qt.darker(creatureColor, 1.5)
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
}
