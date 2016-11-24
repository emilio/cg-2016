in vec3 fPosition;
in vec2 fUv;

out vec4 oFragColor;

// TODO: Lighting? Probably could calculate a normal using a simple
// pass-through shader.
void main() {
  // Multiplication per uDimension is to keep aspect ratio of the original
  // image, though we shouldn't keep the hard-coded number here.
  vec4 color =  texture2D(uCover, fUv * uDimension / 100.0);
  oFragColor = color;
}
