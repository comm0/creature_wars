pragma ComponentBehavior: Bound

import QtQuick
import Felgo 4.0

Item {
    id: baseView

    required property var baseId
    required property string baseTypeIdentifier
    required property int column
    required property int row
    required property int baseSize
    required property int baseLevel
    required property string baseName
    required property string baseGroup
    required property color baseColor
    required property int health
    required property int maximumHealth
    required property int attack
    required property int baseRange
    required property int damageAmount
    required property int damageRevision
    required property int attackRevision
    required property int attackTargetColumn
    required property int attackTargetRow
    required property bool removing

    property int tileSize: 16
    property bool rangeVisible: false
    property Item overlayParent: parent
    property bool isPlayerBase: false
    property var actionsModel
    property int gold: 0
    property int food: 0
    property bool gameRunning: true
    property real gameTimeScale: 1
    property real mapScale: 1
    property bool selected: false
    property bool targeted: false
    property bool appeared: false

    readonly property int artLevel: Math.max(1, Math.min(baseLevel, 4))
    readonly property int artHeight: [12, 22, 28, 34][artLevel - 1]
    readonly property real identityScale: 1 / Math.max(mapScale, 0.01)
    readonly property bool menuOpen: !removing && isPlayerBase
        && (selected || baseHover.hovered || radialMenu.hovered || menuCloseTimer.running)

    signal actionRequested(string actionKey)

    x: column * tileSize
    y: row * tileSize
    width: baseSize * tileSize
    height: baseSize * tileSize
    enabled: !removing
    opacity: removing ? 0 : (appeared ? 1 : 0)
    z: column + baseSize - 1 + row + row / 1000

    Component.onCompleted: Qt.callLater(function() {
        baseView.appeared = true
    })

    Behavior on opacity {
        NumberAnimation {
            duration: 300
        }
    }

    MultiResolutionImage {
        id: baseImage

        x: -baseView.artHeight
        y: -baseView.artHeight
        width: baseView.width + baseView.artHeight
        height: baseView.height + baseView.artHeight
        source: "qrc:/assets/structures/base/base_" + baseView.artLevel + ".png"
        smooth: false
        layer.enabled: true
        layer.effect: TintEffect {
            color: baseView.baseColor
        }
    }

    OutlineEffect {
        id: baseSelectionOutline

        source: baseImage
        color: baseView.targeted ? "#ff4a3d" : "#f4df5a"
        visible: baseView.selected || baseView.targeted
    }

    Rectangle {
        anchors.centerIn: parent
        width: parent.width + 2 * baseView.baseRange * baseView.tileSize
        height: width
        color: "transparent"
        border.width: 1
        border.color: "#44e86a5a"
        visible: baseView.rangeVisible
        z: -1
    }

    Item {
        id: baseOverlay

        parent: baseView.overlayParent
        x: baseView.x
        y: baseView.y
        width: baseView.width
        height: baseView.height
        opacity: baseView.opacity
        z: baseHover.hovered || baseView.menuOpen ? 1 : 0

        TargetReticle {
            x: -baseView.artHeight - 2
            y: -baseView.artHeight - 2
            width: baseView.width + baseView.artHeight + 4
            height: baseView.height + baseView.artHeight + 4
            cornerLength: 8
            visible: baseView.targeted
        }

        Item {
            id: baseIdentity

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.horizontalCenterOffset: -baseView.artHeight / 2
            y: Math.max(
                -baseView.artHeight - height - 2,
                -baseView.y - height + height * baseView.identityScale
            )
            width: 72
            height: 16
            scale: baseView.identityScale
            transformOrigin: Item.Bottom

            AppText {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                color: baseHealthBar.fillColor
                font.pixelSize: 8
                font.weight: Font.DemiBold
                style: Text.Outline
                styleColor: "#000000"
                renderType: Text.CurveRendering
                renderTypeQuality: Text.VeryHighRenderTypeQuality
                text: qsTr("%1  HP %2/%3")
                    .arg(baseView.baseName)
                    .arg(baseView.health)
                    .arg(baseView.maximumHealth)
                visible: baseHover.hovered
            }

            HealthBar {
                id: baseHealthBar

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                width: (baseView.width - 8) * 1.3
                health: baseView.health
                maximumHealth: baseView.maximumHealth
            }
        }

        Text {
            id: baseDamageText

            anchors.horizontalCenter: parent.horizontalCenter
            color: "#f04444"
            font.bold: true
            font.pixelSize: 10
            opacity: 0
            style: Text.Outline
            styleColor: "#000000"
            text: baseView.damageAmount > 0 ? "-" + baseView.damageAmount : ""
        }

        Rectangle {
            id: baseProjectile

            width: 4
            height: 4
            radius: 2
            color: Qt.lighter(baseView.baseColor, 1.8)
            border.width: 1
            border.color: "#000000"
            visible: baseProjectileAnimation.running
        }

        Item {
            anchors.fill: parent

            HoverHandler {
                id: baseHover

                blocking: true
            }
        }

        RadialMenu {
            id: radialMenu

            anchors.centerIn: parent
            anchors.verticalCenterOffset: Math.min(
                0,
                baseView.overlayParent.height
                    - (baseView.y + baseView.height / 2)
                    - radius
                    - actionSize / 2
                    - 2
            )
            radius: baseView.width / 2 + 44
            actionSize: 44
            visible: baseView.menuOpen
            enabled: visible
            actionsModel: baseView.isPlayerBase ? baseView.actionsModel : null
            gold: baseView.gold
            food: baseView.food
            running: baseView.gameRunning
            timeScale: baseView.gameTimeScale

            onActionActivated: function(actionKey) {
                baseView.actionRequested(actionKey)
            }
        }
    }

    Timer {
        id: menuCloseTimer

        interval: 300
    }

    Connections {
        target: baseHover

        function onHoveredChanged() {
            if (!baseHover.hovered) {
                menuCloseTimer.restart()
            }
        }
    }

    Connections {
        target: radialMenu

        function onHoveredChanged() {
            if (!radialMenu.hovered) {
                menuCloseTimer.restart()
            }
        }
    }

    ParallelAnimation {
        id: baseDamageAnimation

        NumberAnimation {
            target: baseDamageText
            property: "y"
            from: baseView.height / 2 - baseDamageText.height / 2
            to: -baseView.tileSize
            duration: Math.max(1, 900 / baseView.gameTimeScale)
            easing.type: Easing.OutCubic
        }

        NumberAnimation {
            target: baseDamageText
            property: "opacity"
            from: 1
            to: 0
            duration: Math.max(1, 900 / baseView.gameTimeScale)
        }
    }

    ParallelAnimation {
        id: baseProjectileAnimation

        NumberAnimation {
            target: baseProjectile
            property: "x"
            from: baseView.width / 2 - baseProjectile.width / 2
            to: (baseView.attackTargetColumn + 0.5) * baseView.tileSize
                - baseView.x
                - baseProjectile.width / 2
            duration: Math.max(1, 180 / baseView.gameTimeScale)
        }

        NumberAnimation {
            target: baseProjectile
            property: "y"
            from: baseView.height / 2 - baseProjectile.height / 2
            to: (baseView.attackTargetRow + 0.5) * baseView.tileSize
                - baseView.y
                - baseProjectile.height / 2
            duration: Math.max(1, 180 / baseView.gameTimeScale)
        }
    }

    onDamageRevisionChanged: {
        if (damageRevision > 0) {
            baseDamageAnimation.restart()
        }
    }

    onAttackRevisionChanged: {
        if (attackRevision > 0) {
            Qt.callLater(baseProjectileAnimation.restart)
        }
    }
}
