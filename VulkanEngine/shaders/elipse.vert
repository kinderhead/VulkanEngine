#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 color;
} ubo;

layout(location = 0) in vec2 inPosition;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragPos;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
    fragColor = ubo.color;
    fragPos = inPosition;
}