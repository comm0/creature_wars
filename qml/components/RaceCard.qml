import QtQuick
import Felgo 4.0

Rectangle {
    id: raceCard

    required property string raceName
    required property string creatureIdentifier
    property bool selected: false
    property string aiDifficulty: "normal"

    signal clicked()
    signal difficultyClicked()

    width: 96
    height: 132
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
        y: 82
        color: raceCard.selected ? "#f4df5a" : "#f1f4f2"
        font.pixelSize: 11
        font.weight: Font.DemiBold
        text: raceCard.raceName
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 7
        width: 78
        height: 20
        radius: 3
        color: raceCard.selected ? "#26332c" : "#30443a"
        border.width: 1
        border.color: raceCard.selected ? "#526159" : "#658672"

        AppText {
            anchors.centerIn: parent
            color: raceCard.selected ? "#8b9990" : "#d9e8de"
            font.pixelSize: 9
            text: raceCard.selected
                ? qsTr("Player")
                : qsTr("AI: %1").arg(raceCard.aiDifficulty)
        }
    }

    HoverHandler {
        id: raceHover
    }

    TapHandler {
        onTapped: function(eventPoint) {
            if (!raceCard.selected
                    && eventPoint.position.y >= raceCard.height - 27) {
                raceCard.difficultyClicked()
            } else {
                raceCard.clicked()
            }
        }
    }
}
