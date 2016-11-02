#line 1

layout (location = 0) in vec3 vPosition;

out vec4 oFragColor;
out vec3 fPosition;

const float FAR = 100.0;

void main() {
  // Proof of concept, the dimensions would be the map ones.
  mat4 scaleMatrix = mat4(FAR, 0, 0, 0,
                          0, FAR, 0, 0,
                          0, 0, FAR, 0,
                          0, 0,   0, 1.0);
  gl_Position = uViewProjection * scaleMatrix * vec4(vPosition, 1.0);
  fPosition = vPosition;
}
