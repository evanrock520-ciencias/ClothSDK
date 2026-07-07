#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 uView;
uniform mat4 uProjection;

out vec3 fragPos;
out vec3 fragNormal;

void main() {
    fragPos    = aPos;
    fragNormal = aNormal;

    gl_Position = uProjection * uView * vec4(aPos, 1.0);
}