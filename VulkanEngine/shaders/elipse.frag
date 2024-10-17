#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragPos;

layout(location = 0) out vec4 outColor;

void main() {
    if (distance(fragPos, vec2(0.0, 0.0)) > 0.5) {
        discard;
    } else {
        outColor = fragColor;
    }
}
