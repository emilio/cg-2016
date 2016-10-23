#version 330

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vUv;

uniform mat4 uTransform;
uniform float uFrame;

out vec4 fColor;

void main () {
  fColor = vec4(vPosition, 1.0);
  gl_Position = uTransform * vec4(vPosition, 1.0);
}
