pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Effects
import QtQuick.Shapes
import Felgo 4.0
import "CreatureSprites.js" as CreatureSprites

Item {
    id: actionCircle

    required property string actionKey
    required property string actionKind
    required property string actionName
    required property string subject
    required property color subjectColor
    required property int actionLevel
    required property int goldCost
    required property int foodCost
    required property int duration
    required property int queued
    required property int remaining
    required property int revision

    property int gold: 0
    property int food: 0
    property bool running: true
    property real timeScale: 1
    property real progress: 0

    readonly property bool isUnit: actionKind === "spawn" || actionKind === "training"
    readonly property bool inProgress: remaining > 0
    readonly property bool affordable: gold >= goldCost && food >= foodCost
    readonly property bool clickable: affordable
        && (actionKind === "spawn" ? queued < 9 : queued === 0)
    readonly property int secondsLeft: Math.ceil((1 - progress) * duration / 1000)
    readonly property real sizeScale: width / 44
    readonly property var researchIcons: ({
        "base_regeneration": IconType.heart,
        "reinforced_walls": IconType.shield,
        "healing_aura": IconType.medkit,
        "heavy_shots": IconType.fire,
        "multi_shot": IconType.bolt,
        "long_range": IconType.crosshairs
    })
    readonly property var researchColors: ({
        "base_regeneration": "#ef5350",
        "reinforced_walls": "#4f9ee8",
        "healing_aura": "#63d6c5",
        "heavy_shots": "#ff8f3d",
        "multi_shot": "#ffd54f",
        "long_range": "#55b9e8"
    })

    signal activated(string actionKey)

    width: 44
    height: 44
    z: circleHover.hovered ? 2 : 1

    function restartProgress() {
        progressAnimation.stop()

        if (!inProgress || duration <= 0) {
            progress = 0
            return
        }

        progress = Math.max(0, 1 - remaining / duration)
        progressAnimation.duration = Math.max(1, remaining / timeScale)
        progressAnimation.restart()
    }

    function rescaleProgress() {
        if (!inProgress || duration <= 0) {
            return
        }

        const currentRemaining = Math.max(0, (1 - progress) * duration)
        progressAnimation.stop()
        progressAnimation.duration = Math.max(1, currentRemaining / timeScale)
        progressAnimation.restart()
    }

    onRevisionChanged: restartProgress()
    onTimeScaleChanged: rescaleProgress()
    Component.onCompleted: restartProgress()

    NumberAnimation {
        id: progressAnimation

        target: actionCircle
        property: "progress"
        to: 1
        paused: running && !actionCircle.running
    }

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: "#e6121a15"
        border.width: 2 * actionCircle.sizeScale
        border.color: circleHover.hovered && actionCircle.clickable ? "#c9d4cb" : "#405348"
    }

    Item {
        id: circleContent

        anchors.fill: parent
        anchors.margins: 4 * actionCircle.sizeScale
        visible: false
        layer.enabled: true

        TexturePackerAnimatedSprite {
            anchors.centerIn: parent
            width: 32 * actionCircle.sizeScale
            height: width
            smooth: false
            visible: actionCircle.isUnit
            source: actionCircle.isUnit
                ? CreatureSprites.sheetSource(actionCircle.subject)
                : ""
            frameNames: ["south_0.png"]
            layer.enabled: CreatureSprites.isPlaceholder(actionCircle.subject)
            layer.effect: TintEffect {
                color: actionCircle.subjectColor
            }
        }

        AppIcon {
            anchors.centerIn: parent
            size: 22 * actionCircle.sizeScale
            visible: actionCircle.actionKind === "research"
            iconType: actionCircle.researchIcons[actionCircle.subject] ?? IconType.plus
            color: actionCircle.researchColors[actionCircle.subject] ?? "#e8eee9"
        }

        AppIcon {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -1 * actionCircle.sizeScale
            visible: actionCircle.actionKind === "upgrade"
            size: 26 * actionCircle.sizeScale
            iconType: IconType.arrowup
            color: "#f2c438"
        }

        AppIcon {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: 4 * actionCircle.sizeScale
            anchors.bottomMargin: 4 * actionCircle.sizeScale
            visible: actionCircle.actionKind === "research"
            size: 11 * actionCircle.sizeScale
            iconType: IconType.plus
            color: "#62d66f"
        }
    }

    Rectangle {
        id: circleMask

        anchors.fill: circleContent
        radius: width / 2
        visible: false
        layer.enabled: true
    }

    MultiEffect {
        anchors.fill: circleContent
        source: circleContent
        maskEnabled: true
        maskSource: circleMask
        saturation: actionCircle.actionKind === "training"
            || !actionCircle.clickable ? -1 : 0
        opacity: actionCircle.actionKind === "training"
            ? 0.38
            : actionCircle.clickable ? 1 : 0.55
    }

    Shape {
        anchors.fill: parent
        visible: actionCircle.inProgress
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            strokeColor: "#9fe07a"
            strokeWidth: 3 * actionCircle.sizeScale
            fillColor: "transparent"
            capStyle: ShapePath.FlatCap

            PathAngleArc {
                centerX: actionCircle.width / 2
                centerY: actionCircle.height / 2
                radiusX: actionCircle.width / 2 - 2 * actionCircle.sizeScale
                radiusY: actionCircle.height / 2 - 2 * actionCircle.sizeScale
                startAngle: -90
                sweepAngle: 360 * actionCircle.progress
            }
        }
    }

    AppText {
        anchors.centerIn: parent
        visible: actionCircle.inProgress
        color: "#ffffff"
        font.pixelSize: 14 * actionCircle.sizeScale
        font.weight: Font.DemiBold
        style: Text.Outline
        styleColor: "#000000"
        text: actionCircle.secondsLeft
    }

    AppIcon {
        anchors.centerIn: parent
        size: 24 * actionCircle.sizeScale
        visible: actionCircle.actionKind === "training"
        iconType: IconType.graduationcap
        color: actionCircle.clickable ? "#f2c438" : "#7b847e"
    }

    Rectangle {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 2 * actionCircle.sizeScale
        width: levelText.implicitWidth + 6 * actionCircle.sizeScale
        height: 14 * actionCircle.sizeScale
        radius: 3 * actionCircle.sizeScale
        color: actionCircle.clickable ? "#2f6b3a" : "#555d58"
        visible: actionCircle.actionKind === "research" || actionCircle.actionKind === "upgrade"

        AppText {
            id: levelText

            anchors.centerIn: parent
            color: "#e8eee9"
            font.pixelSize: 10 * actionCircle.sizeScale
            text: actionCircle.actionLevel
        }
    }

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 2 * actionCircle.sizeScale
        width: 16 * actionCircle.sizeScale
        height: 16 * actionCircle.sizeScale
        radius: width / 2
        color: "#d94a45"
        visible: actionCircle.actionKind === "spawn" && actionCircle.queued > 0

        AppText {
            anchors.centerIn: parent
            color: "#ffffff"
            font.pixelSize: 10 * actionCircle.sizeScale
            font.weight: Font.DemiBold
            text: actionCircle.queued
        }
    }

    HoverHandler {
        id: circleHover
    }

    TapHandler {
        onTapped: {
            if (actionCircle.clickable) {
                actionCircle.activated(actionCircle.actionKey)
            }
        }
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.top
        anchors.bottomMargin: 3
        width: tooltipColumn.implicitWidth + 8
        height: tooltipColumn.implicitHeight + 6
        radius: 2
        color: "#f0111814"
        border.width: 1
        border.color: "#758579"
        visible: circleHover.hovered

        Column {
            id: tooltipColumn

            anchors.centerIn: parent
            spacing: 1

            AppText {
                color: "#f1f4f2"
                font.pixelSize: 8
                font.weight: Font.DemiBold
                text: actionCircle.actionKind === "research"
                    ? qsTr("%1 %2").arg(actionCircle.actionName).arg(actionCircle.actionLevel)
                    : actionCircle.actionName
            }

            Row {
                spacing: 4

                HudStat {
                    imageSource: "qrc:/assets/ui/gold.png"
                    textColor: actionCircle.gold >= actionCircle.goldCost ? "#f2c438" : "#d94a45"
                    text: actionCircle.goldCost
                    visible: actionCircle.goldCost > 0
                }

                HudStat {
                    imageSource: "qrc:/assets/ui/food.png"
                    textColor: actionCircle.food >= actionCircle.foodCost ? "#e0955a" : "#d94a45"
                    text: actionCircle.foodCost
                    visible: actionCircle.foodCost > 0
                }

                HudStat {
                    iconType: IconType.clocko
                    text: qsTr("%1 s").arg(actionCircle.duration / 1000)
                }
            }
        }
    }
}
