import QtQuick
import Felgo 4.0

Item {
    id: hudResource

    property url imageSource
    property color color: "#f1f4f2"
    property string resourceName
    property int amount: 0
    property int income: 0
    property int incomeInterval: 0
    property int incomeCycle: 0
    property bool running: true
    property real gameTimeScale: 1
    property real progress: 0

    width: resourceStat.width
    height: 14

    function restartProgress() {
        progress = 0
        progressAnimation.restart()
    }

    onIncomeCycleChanged: restartProgress()
    onIncomeIntervalChanged: restartProgress()

    HudStat {
        id: resourceStat

        y: 1
        imageSource: hudResource.imageSource
        textColor: hudResource.color
        textMinimumWidth: widestAmount.width
        text: hudResource.amount
    }

    TextMetrics {
        id: widestAmount

        font.pixelSize: 9
        font.weight: Font.DemiBold
        text: "99999"
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        y: parent.height - 3
        height: 2
        radius: 1
        color: "#2c3a31"
        visible: hudResource.incomeInterval > 0

        Rectangle {
            width: parent.width * hudResource.progress
            height: parent.height
            radius: 1
            color: hudResource.color
        }
    }

    NumberAnimation {
        id: progressAnimation

        target: hudResource
        property: "progress"
        from: 0
        to: 1
        duration: Math.max(
            1,
            hudResource.incomeInterval / hudResource.gameTimeScale
        )
        paused: running && !hudResource.running
    }

    HoverHandler {
        id: resourceHover
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height + 2
        width: tooltipText.implicitWidth + 8
        height: tooltipText.implicitHeight + 4
        radius: 2
        color: "#e6111814"
        border.width: 1
        border.color: "#758579"
        visible: resourceHover.hovered && hudResource.incomeInterval > 0

        AppText {
            id: tooltipText

            anchors.centerIn: parent
            color: "#f1f4f2"
            font.pixelSize: 8
            text: qsTr("+%1 %2 / %3 s")
                .arg(hudResource.income)
                .arg(hudResource.resourceName)
                .arg(hudResource.incomeInterval / 1000)
        }
    }
}
