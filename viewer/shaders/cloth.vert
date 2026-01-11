#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    vec4 worldPos = vec4(aPos, 1.0);
    gl_Position = uProjection * uView * worldPos;
}