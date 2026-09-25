import QtQuick

// Multiplies grayscale art by `color`; use as layer.effect.
ShaderEffect {
    property color color: "white"
    readonly property color tintColor: color

    fragmentShader: "qrc:/shaders/tint.frag.qsb"
}
