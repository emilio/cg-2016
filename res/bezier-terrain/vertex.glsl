#line 1

layout (location = 0) in vec3 vPosition;

void main () {
  gl_Position = vec4(vPosition, 1.0);
  // gl_Position = vec4(vPosition.x, vPosition.z, 0.0, 1.0);
}
