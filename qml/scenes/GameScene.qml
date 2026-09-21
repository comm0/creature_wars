import QtQuick 2.15
import QtQuick.Controls 2.15
import Felgo 4.0
import "../components"

Scene {
    id: gameScene

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
        columns: 40
        rows: 20
        tileSize: 16
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

        CheckBox {
            anchors.centerIn: parent
            text: qsTr("Show grid")
            checked: true
            palette.windowText: "#e8eee9"

            onToggled: worldMap.gridVisible = checked
        }
    }
}
