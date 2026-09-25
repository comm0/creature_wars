import QtQuick
import Felgo 4.0
import "scenes"

GameWindow {
    id: gameWindow

    required property var gameBackend

    screenWidth: 1280
    screenHeight: 720
    storeWindowGeometry: false
    activeScene: menuScene
    state: "menu"

    MenuScene {
        id: menuScene

        onStartRequested: function(baseIdentifier) {
            gameScene.resetGameTimer()
            gameWindow.gameBackend.startMatch(baseIdentifier)
            gameWindow.state = "game"
        }
    }

    GameScene {
        id: gameScene

        gameBackend: gameWindow.gameBackend
    }

    states: [
        State {
            name: "menu"

            PropertyChanges {
                menuScene.opacity: 1
                gameWindow.activeScene: menuScene
            }
        },
        State {
            name: "game"

            PropertyChanges {
                gameScene.opacity: 1
                gameWindow.activeScene: gameScene
            }
        }
    ]
}
