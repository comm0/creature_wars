import QtQuick
import Felgo 4.0

Scene {
    id: gameOverScene

    property bool victory: false
    property int durationMs: 0

    signal playAgainRequested()
    signal mainMenuRequested()

    width: 640
    height: 360
    scaleMode: "letterbox"
    opacity: 0
    visible: opacity > 0
    enabled: visible

    Behavior on opacity {
        NumberAnimation {
            duration: 300
        }
    }

    onVisibleChanged: {
        if (visible) {
            titleAnimation.restart()
        }
    }

    function formatDuration(milliseconds) {
        const totalSeconds = Math.floor(milliseconds / 1000)
        const seconds = totalSeconds % 60
        return Math.floor(totalSeconds / 60) + ":" + (seconds < 10 ? "0" : "") + seconds
    }

    Rectangle {
        anchors.fill: gameOverScene.gameWindowAnchorItem
        color: "#cc0b0f0d"
    }

    Text {
        id: title

        anchors.horizontalCenter: parent.horizontalCenter
        y: 96
        color: gameOverScene.victory ? "#f2c438" : "#d94a45"
        font.pixelSize: 40
        font.bold: true
        style: Text.Outline
        styleColor: "#000000"
        text: gameOverScene.victory ? qsTr("Victory") : qsTr("Defeat")
    }

    AppText {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 150
        color: "#a9b8ad"
        font.pixelSize: 12
        text: gameOverScene.victory
            ? qsTr("All enemy bases destroyed in %1").arg(gameOverScene.formatDuration(gameOverScene.durationMs))
            : qsTr("Your base fell after %1").arg(gameOverScene.formatDuration(gameOverScene.durationMs))
    }

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 200
        spacing: 16

        AppButton {
            text: qsTr("Play again")
            textSize: 13
            fontCapitalization: Font.MixedCase
            minimumWidth: 120
            minimumHeight: 0
            horizontalMargin: 0
            verticalMargin: 0
            verticalPadding: 6
            radius: 4
            dropShadow: false
            backgroundColor: "#2f6b3a"
            backgroundColorHovered: "#3a8047"
            backgroundColorPressed: "#255530"
            textColor: "#f1f4f2"

            onClicked: gameOverScene.playAgainRequested()
        }

        AppButton {
            text: qsTr("Main menu")
            textSize: 13
            fontCapitalization: Font.MixedCase
            minimumWidth: 120
            minimumHeight: 0
            horizontalMargin: 0
            verticalMargin: 0
            verticalPadding: 6
            radius: 4
            dropShadow: false
            backgroundColor: "#2a332d"
            backgroundColorHovered: "#36423a"
            backgroundColorPressed: "#222a25"
            textColor: "#f1f4f2"

            onClicked: gameOverScene.mainMenuRequested()
        }
    }

    SequentialAnimation {
        id: titleAnimation

        NumberAnimation {
            target: title
            property: "scale"
            from: 2.2
            to: 0.9
            duration: 260
            easing.type: Easing.OutCubic
        }

        NumberAnimation {
            target: title
            property: "scale"
            to: 1
            duration: 140
            easing.type: Easing.OutBack
        }
    }
}
