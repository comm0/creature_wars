pragma ComponentBehavior: Bound

import QtQuick
import Felgo 4.0
import "../components"

Scene {
    id: menuScene

    property string selectedBase: ""

    signal startRequested(string baseIdentifier)

    width: 640
    height: 360
    scaleMode: "letterbox"
    opacity: 0
    visible: opacity > 0
    enabled: visible

    Behavior on opacity {
        NumberAnimation {
            duration: 250
        }
    }

    Rectangle {
        anchors.fill: menuScene.gameWindowAnchorItem
        color: "#111814"
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 36
        color: "#f1f4f2"
        font.pixelSize: 32
        font.bold: true
        style: Text.Outline
        styleColor: "#000000"
        text: qsTr("Creature Wars")
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 92
        color: "#a9b8ad"
        font.pixelSize: 12
        text: qsTr("Choose your race")
    }

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 118
        spacing: 16

        Repeater {
            model: [
                {
                    baseIdentifier: "minotaur_base",
                    creatureIdentifier: "minotaur",
                    raceName: qsTr("Minotaurs")
                },
                {
                    baseIdentifier: "orc_base",
                    creatureIdentifier: "orc",
                    raceName: qsTr("Orcs")
                },
                {
                    baseIdentifier: "dwarf_base",
                    creatureIdentifier: "dwarf",
                    raceName: qsTr("Dwarves")
                }
            ]

            delegate: RaceCard {
                required property var modelData

                raceName: modelData.raceName
                creatureIdentifier: modelData.creatureIdentifier
                selected: menuScene.selectedBase === modelData.baseIdentifier

                onClicked: menuScene.selectedBase = modelData.baseIdentifier
            }
        }
    }

    AppButton {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 262
        text: qsTr("Start")
        enabled: menuScene.selectedBase !== ""
        textSize: 14
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
        disabledColor: "#2a332d"
        textColor: "#f1f4f2"
        textColorDisabled: "#6b776f"

        onClicked: menuScene.startRequested(menuScene.selectedBase)
    }
}
