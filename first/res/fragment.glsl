#version 330

out vec4 oFragColor;
in vec4 fColor;

void main () {
  oFragColor = fColor;
  oFragColor.a = 1.0;
  // oFragColor = vec4(0.5, 0.0, 0.5, 1.0);
  // gl_FragColor = vec4 (0.5, 0.0, 0.5, 1.0);
}
