#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec4 outlineColor;
    vec2 texelStep;
    float alphaThreshold;
};

layout(binding = 1) uniform sampler2D source;

float covered(vec2 offset_p)
{
    return step(alphaThreshold, texture(source, qt_TexCoord0 + offset_p).a);
}

// Paints outlineColor on transparent pixels that touch an opaque pixel of the
// source, so the outline hugs the silhouette and never covers the source.
void main()
{
    float neighbours = covered(vec2(-texelStep.x, 0.0))
        + covered(vec2(texelStep.x, 0.0))
        + covered(vec2(0.0, -texelStep.y))
        + covered(vec2(0.0, texelStep.y))
        + covered(vec2(-texelStep.x, -texelStep.y))
        + covered(vec2(texelStep.x, -texelStep.y))
        + covered(vec2(-texelStep.x, texelStep.y))
        + covered(vec2(texelStep.x, texelStep.y));
    float edge = min(neighbours, 1.0) * (1.0 - covered(vec2(0.0)));

    fragColor = vec4(outlineColor.rgb * outlineColor.a, outlineColor.a)
        * edge * qt_Opacity;
}
