import QtQuick
import Felgo 4.0

Row {
    id: hudStat

    property url imageSource
    property string iconType
    property color iconColor: "#c9d4cb"
    property alias text: statText.text
    property color textColor: "#f1f4f2"
    property real textMinimumWidth: 0

    spacing: 3

    MultiResolutionImage {
        anchors.verticalCenter: parent.verticalCenter
        width: 10
        height: 10
        source: hudStat.imageSource
        smooth: false
        visible: hudStat.imageSource.toString().length > 0
    }

    AppIcon {
        anchors.verticalCenter: parent.verticalCenter
        size: 9
        iconType: hudStat.iconType
        color: hudStat.iconColor
        visible: hudStat.iconType.length > 0
    }

    AppText {
        id: statText

        anchors.verticalCenter: parent.verticalCenter
        width: Math.max(implicitWidth, hudStat.textMinimumWidth)
        color: hudStat.textColor
        font.pixelSize: 9
        font.weight: Font.DemiBold
    }
}
