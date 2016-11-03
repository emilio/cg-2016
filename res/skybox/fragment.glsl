#line 1

out vec4 oFragColor;
in vec3 fPosition;

void main () {
  oFragColor = texture(uSkybox, fPosition);
}
