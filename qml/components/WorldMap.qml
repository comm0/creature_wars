import QtQuick 2.15

Item {
    id: root

    property int columns: 40
    property int rows: 20
    property int tileSize: 16
    property bool gridVisible: true

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
}
