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

    property string lastBaseIdentifier
    property string lastMinotaurDifficulty: "normal"
    property string lastOrcDifficulty: "normal"
    property string lastDwarfDifficulty: "normal"

    function startMatch(
        baseIdentifier,
        minotaurDifficulty,
        orcDifficulty,
        dwarfDifficulty
    ) {
        lastBaseIdentifier = baseIdentifier
        lastMinotaurDifficulty = minotaurDifficulty
        lastOrcDifficulty = orcDifficulty
        lastDwarfDifficulty = dwarfDifficulty
        gameScene.resetGameTimer()
        gameBackend.startMatch(
            baseIdentifier,
            minotaurDifficulty,
            orcDifficulty,
            dwarfDifficulty
        )
        state = "game"
    }

    MenuScene {
        id: menuScene

        onStartRequested: function(
            baseIdentifier,
            minotaurDifficulty,
            orcDifficulty,
            dwarfDifficulty
        ) {
            gameWindow.startMatch(
                baseIdentifier,
                minotaurDifficulty,
                orcDifficulty,
                dwarfDifficulty
            )
        }
    }

    GameScene {
        id: gameScene

        gameBackend: gameWindow.gameBackend
    }

    GameOverScene {
        id: gameOverScene

        onPlayAgainRequested: gameWindow.startMatch(
            gameWindow.lastBaseIdentifier,
            gameWindow.lastMinotaurDifficulty,
            gameWindow.lastOrcDifficulty,
            gameWindow.lastDwarfDifficulty
        )
        onMainMenuRequested: gameWindow.state = "menu"
    }

    Connections {
        target: gameWindow.gameBackend

        function onMatchEnded(victory, durationMs) {
            gameOverScene.victory = victory
            gameOverScene.durationMs = durationMs
            gameWindow.state = "gameOver"
        }
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
        },
        State {
            name: "gameOver"

            PropertyChanges {
                gameScene.opacity: 1
                gameOverScene.opacity: 1
                gameWindow.activeScene: gameOverScene
            }
        }
    ]
}
