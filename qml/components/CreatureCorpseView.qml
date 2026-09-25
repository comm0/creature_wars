import QtQuick
import Felgo 4.0

Item {
    id: corpseView

    required property string creatureTypeIdentifier
    required property int column
    required property int row
    required property bool removing

    property int tileSize: 16
    property int fadeDuration: 1500
    property bool appeared: false

    readonly property int artSize: 32

    x: column * tileSize
    y: row * tileSize
    width: tileSize
    height: tileSize
    opacity: removing ? 0 : (appeared ? 1 : 0)
    z: (x + y) / tileSize + y / tileSize / 1000

    Component.onCompleted: Qt.callLater(function() {
        corpseView.appeared = true
    })

    Behavior on opacity {
        NumberAnimation {
            duration: corpseView.fadeDuration
        }
    }

    MultiResolutionImage {
        x: parent.width - width
        y: parent.height - height
        width: corpseView.artSize
        height: corpseView.artSize
        source: "qrc:/assets/creatures/"
            + corpseView.creatureTypeIdentifier
            + "/corpse.png"
        smooth: false
    }

}
