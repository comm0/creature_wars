import QtQuick
import Felgo 4.0

Rectangle {
    id: raceCard

    required property string raceName
    required property string creatureIdentifier
    property bool selected: false

    signal clicked()

    width: 96
    height: 112
    radius: 4
    color: raceHover.hovered ? "#223029" : "#1a241e"
    border.width: selected ? 2 : 1
    border.color: selected ? "#f4df5a" : "#405348"

    TexturePackerAnimatedSprite {
        id: raceSprite

        anchors.horizontalCenter: parent.horizontalCenter
        y: 12
        width: 64
        height: 64
        source: "qrc:/assets/creatures/"
            + raceCard.creatureIdentifier
            + "/"
            + raceCard.creatureIdentifier
            + ".json"
        frameNames: ["south_0.png"]
    }

    OutlineEffect {
        source: raceSprite
        visible: raceCard.selected
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        color: raceCard.selected ? "#f4df5a" : "#f1f4f2"
        font.pixelSize: 11
        font.weight: Font.DemiBold
        text: raceCard.raceName
    }

    HoverHandler {
        id: raceHover
    }

    TapHandler {
        onTapped: raceCard.clicked()
    }
}
