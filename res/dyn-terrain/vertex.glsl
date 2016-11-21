#line 1

layout (location = 0) in vec2 vPosition;

void main () {
  gl_Position = vec4(vPosition.x, 0.0, vPosition.y, 1.0);
}
