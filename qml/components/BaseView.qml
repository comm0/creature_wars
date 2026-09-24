import QtQuick
import Felgo 4.0

Item {
    id: baseView

    required property var baseId
    required property string baseTypeIdentifier
    required property int column
    required property int row
    required property int baseSize
    required property string baseName
    required property string baseGroup
    required property color baseColor
    required property int health
    required property int maximumHealth
    required property int attack
    required property int attackRange
    required property string spawnCreatureName
    required property int damageAmount
    required property int damageRevision
    required property int attackRevision
    required property int attackTargetColumn
    required property int attackTargetRow

    property int tileSize: 16
    property bool rangeVisible: false
    property Item overlayParent: parent

    signal spawnCreatureRequested(var baseId)

    x: column * tileSize
    y: row * tileSize
    width: baseSize * tileSize
    height: baseSize * tileSize

    Rectangle {
        anchors.fill: parent
        anchors.margins: 1
        radius: 2
        color: baseView.baseColor
        border.width: 2
        border.color: Qt.darker(baseView.baseColor, 1.6)

        Rectangle {
            anchors.centerIn: parent
            width: parent.width / 3
            height: width
            radius: 1
            color: Qt.lighter(baseView.baseColor, 1.4)
            border.width: 1
            border.color: Qt.darker(baseView.baseColor, 1.6)
        }
    }

    Rectangle {
        anchors.centerIn: parent
        width: parent.width + 2 * baseView.attackRange * baseView.tileSize
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
        z: baseHover.hovered ? 1 : 0

        Item {
            id: baseIdentity

            anchors.horizontalCenter: parent.horizontalCenter
            y: Math.max(-height - 2, -baseView.y)
            width: 72
            height: 16

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                color: baseHealthBar.fillColor
                font.pixelSize: 8
                font.weight: Font.DemiBold
                style: Text.Outline
                styleColor: "#000000"
                text: baseView.baseName
                visible: baseHover.hovered
            }

            HealthBar {
                id: baseHealthBar

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                width: baseView.width - 8
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
            anchors.centerIn: parent
            width: Math.max(parent.width, spawnButton.width)
            height: parent.height

            HoverHandler {
                id: baseHover

                blocking: true
            }

            AppButton {
                id: spawnButton

                anchors.centerIn: parent
                visible: baseHover.hovered
                text: qsTr("Spawn %1").arg(baseView.spawnCreatureName)
                textSize: 7
                fontCapitalization: Font.MixedCase
                minimumWidth: 0
                minimumHeight: 0
                horizontalMargin: 0
                verticalMargin: 0
                horizontalPadding: 4
                verticalPadding: 2
                radius: 2
                dropShadow: false
                rippleEffect: false
                backgroundColor: "#e6111814"
                backgroundColorHovered: "#e62a3a30"
                backgroundColorPressed: "#e6405348"
                borderColor: "#758579"
                borderWidth: 1
                textColor: "#f1f4f2"

                onClicked: baseView.spawnCreatureRequested(baseView.baseId)
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
            duration: 900
            easing.type: Easing.OutCubic
        }

        NumberAnimation {
            target: baseDamageText
            property: "opacity"
            from: 1
            to: 0
            duration: 900
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
            duration: 180
        }

        NumberAnimation {
            target: baseProjectile
            property: "y"
            from: baseView.height / 2 - baseProjectile.height / 2
            to: (baseView.attackTargetRow + 0.5) * baseView.tileSize
                - baseView.y
                - baseProjectile.height / 2
            duration: 180
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
