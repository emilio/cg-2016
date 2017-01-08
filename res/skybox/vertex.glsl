#line 1

layout (location = 0) in vec3 vPosition;

out vec3 fPosition;

void main() {
  gl_Position = uViewProjection * vec4(vPosition, 1.0);
  fPosition = vPosition;
}
