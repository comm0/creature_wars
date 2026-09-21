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
            required property int column
            required property int row

            x: column * root.tileSize + 2
            y: row * root.tileSize + 2
            width: root.tileSize - 4
            height: root.tileSize - 4
            radius: width / 2
            color: "#d9a441"
            border.width: 1
            border.color: "#6f4c16"

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
