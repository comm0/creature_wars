import QtQuick 2.15

// Draws a solid outline around the opaque pixels of `source` (its silhouette,
// not its bounding box). Place it as a sibling of `source`: by default it
// copies the source's geometry and paints outside of it by `thickness`.
// Only renders (and captures the source) while visible.
Item {
    id: outline

    property Item source
    property color color: "#f4df5a"
    // Outline width in the source's own units.
    property real thickness: 1
    // Capture resolution per source unit; raise it for smooth, scaled content.
    property real textureScale: 4
    property real alphaThreshold: 0.5

    x: source ? source.x : 0
    y: source ? source.y : 0
    width: source ? source.width : 0
    height: source ? source.height : 0

    ShaderEffectSource {
        id: outlineSource

        readonly property real paddedWidth: outline.width + 2 * outline.thickness
        readonly property real paddedHeight: outline.height + 2 * outline.thickness

        sourceItem: outline.visible ? outline.source : null
        sourceRect: Qt.rect(
            -outline.thickness,
            -outline.thickness,
            paddedWidth,
            paddedHeight
        )
        textureSize: Qt.size(
            Math.max(1, Math.ceil(paddedWidth * outline.textureScale)),
            Math.max(1, Math.ceil(paddedHeight * outline.textureScale))
        )
        live: outline.visible
        hideSource: false
        smooth: false
        visible: false
    }

    ShaderEffect {
        readonly property var source: outlineSource
        readonly property color outlineColor: outline.color
        readonly property point texelStep: Qt.point(
            outline.thickness / Math.max(outlineSource.paddedWidth, 1),
            outline.thickness / Math.max(outlineSource.paddedHeight, 1)
        )
        readonly property real alphaThreshold: outline.alphaThreshold

        anchors.fill: parent
        anchors.margins: -outline.thickness
        fragmentShader: "qrc:/shaders/outline.frag.qsb"
    }
}
