import QtQuick 2.15
import Felgo 4.0
import "scenes"

GameWindow {
    id: gameWindow

    required property var gameBackend

    screenWidth: 1280
    screenHeight: 720
    activeScene: gameScene

    GameScene {
        id: gameScene

        gameBackend: gameWindow.gameBackend
    }
}
