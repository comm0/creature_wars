import QtQuick 2.15
import Felgo 4.0

// One creature standing on a single map tile. The item itself covers exactly
// that tile (used for hit testing), the sprite may extend beyond it.
Item {
    id: creatureView

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

    property int tileSize: 16
    property bool selected: false
    property bool visionRangeVisible: false
    property int idleDotCount: 1
    property real areaWidth: 0
    property real areaHeight: 0
    // Layer drawn above all creatures; names, health bars, damage numbers
    // and tooltips live there so neighbouring sprites never cover them.
    // It must share the coordinate system of this item's parent.
    property Item overlayParent: parent

    property bool alertVisible: false
    property bool goVisible: false
    readonly property bool hasSprite:
        creatureTypeIdentifier === "minotaur"
    readonly property int spriteSize: 32
    // Tibia-style anchoring: the sprite's bottom-right corner sits on the
    // tile's bottom-right corner, bigger sprites grow up and to the left.
    readonly property real visualLeft: hasSprite ? tileSize - spriteSize : 0
    readonly property real visualTop: hasSprite ? tileSize - spriteSize : 0
    readonly property real visualWidth: hasSprite ? spriteSize : tileSize
    readonly property real visualHeight: hasSprite ? spriteSize : tileSize
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

    x: column * tileSize
    y: row * tileSize
    width: tileSize
    height: tileSize
    // Painter's order: further south is drawn on top, east wins within a row.
    z: y + x / Math.max(areaWidth, 1)

    Item {
        id: creatureVisual

        anchors.fill: parent

        transform: Translate {
            id: creatureAttackTranslation
        }

        Rectangle {
            id: creatureBody

            anchors.fill: parent
            anchors.margins: 2
            radius: width / 2
            color: creatureView.hasSprite
                ? "transparent"
                : creatureView.creatureColor
            border.width: creatureView.hasSprite ? 0 : 1
            border.color: Qt.darker(creatureView.creatureColor, 1.5)

            Rectangle {
                anchors.centerIn: parent
                width: parent.width * 0.35
                height: width
                radius: width / 2
                color: creatureView.markerColor
                visible: !creatureView.hasSprite
                    && creatureView.markerColor.a > 0
            }
        }

        GameSpriteSequence {
            id: creatureSprite

            x: creatureView.visualLeft
            y: creatureView.visualTop
            width: creatureView.spriteSize
            height: creatureView.spriteSize
            defaultSource: creatureView.hasSprite
                ? "qrc:/assets/creatures/minotaur/+hd2/minotaur.png"
                : ""
            interpolate: false
            running: creatureView.hasSprite
                && creatureView.spriteWalking
            visible: creatureView.hasSprite

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
                    creatureView.movementDuration / 2
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
                    creatureView.movementDuration / 2
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
                    creatureView.movementDuration / 2
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
                    creatureView.movementDuration / 2
                )
            }
        }

        OutlineEffect {
            source: creatureView.hasSprite ? creatureSprite : creatureBody
            color: "#f4df5a"
            visible: creatureView.selected
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

    ParallelAnimation {
        id: creatureDamageAnimation

        NumberAnimation {
            target: creatureDamageText
            property: "y"
            from: creatureView.visualTop
                + creatureView.visualHeight / 2
                - creatureDamageText.height / 2
            to: creatureView.visualTop - creatureView.tileSize
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

        onTriggered: creatureView.alertVisible = false
    }

    Timer {
        id: creatureGoTimer

        interval: 600

        onTriggered: creatureView.goVisible = false
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

    Rectangle {
        anchors.centerIn: parent
        width: creatureView.visionRange * 2 * creatureView.tileSize
        height: width
        radius: width / 2
        color: "transparent"
        border.width: 1
        border.color: "#33e8d878"
        visible: creatureView.visionRangeVisible
        z: -1
    }

    HoverHandler {
        id: creatureHover
    }

    Behavior on x {
        NumberAnimation {
            id: xMovementAnimation

            duration: creatureView.movementDuration
            easing.type: Easing.Linear
        }
    }

    Behavior on y {
        NumberAnimation {
            id: yMovementAnimation

            duration: creatureView.movementDuration
            easing.type: Easing.Linear
        }
    }

    // Reparented into overlayParent; follows this creature's tile.
    Item {
        id: creatureOverlay

        parent: creatureView.overlayParent
        x: creatureView.x
        y: creatureView.y
        width: creatureView.width
        height: creatureView.height
        z: creatureHover.hovered ? 1 : 0

        Text {
            id: creatureDamageText

            x: creatureView.visualLeft
                + creatureView.visualWidth / 2
                - width / 2
            color: "#f04444"
            font.bold: true
            font.pixelSize: 10
            opacity: 0
            style: Text.Outline
            styleColor: "#000000"
            text: creatureView.damageAmount > 0
                ? "-" + creatureView.damageAmount
                : ""
            z: 1
        }

        Item {
            id: creatureIdentity

            readonly property bool placeAbove:
                creatureView.y + creatureView.visualTop >= height + 2

            x: creatureView.visualLeft
                + creatureView.visualWidth / 2
                - width / 2
            y: placeAbove
                ? creatureView.visualTop - height - 2
                : creatureView.height + 2
            width: 56
            height: 15

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
                        ? creatureHealthBar.fillColor
                        : creatureView.alertVisible
                            ? "#ffdc4f"
                            : "#f1f4f2"
                    font.pixelSize: 8
                    font.weight: Font.DemiBold
                    font.letterSpacing: -0.2
                    renderType: Text.QtRendering
                    style: Text.Outline
                    styleColor: "#000000"
                    text: creatureHover.hovered
                        ? creatureView.creatureName
                        : creatureView.alertVisible
                            ? "!"
                            : creatureView.goVisible
                                ? qsTr("Go")
                                : creatureView.creatureState === "idle"
                                    ? [".", "..", "..."][creatureView.idleDotCount - 1]
                                    : ""
                }
            }

            HealthBar {
                id: creatureHealthBar

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                health: creatureView.health
                maximumHealth: creatureView.maximumHealth
            }
        }

        CreatureTooltip {
            shown: creatureHover.hovered
            areaWidth: creatureView.areaWidth
            areaHeight: creatureView.areaHeight
            attack: creatureView.attack
            attackRange: creatureView.attackRange
            visionRange: creatureView.visionRange
            movementSpeed: creatureView.movementSpeed
            z: 2
        }
    }
}
