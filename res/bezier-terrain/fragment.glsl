#line 1

in vec3 fPosition;
in vec2 fUv;

out vec4 oFragColor;

float getShadow() {
  vec4 posLightSpace = uShadowMapViewProjection * uModel * vec4(fPosition, 1.0);
  vec3 properCoords = posLightSpace.xyz / posLightSpace.w;
  vec2 uv = posLightSpace.xy / 2.0 + vec2(0.5, 0.5);
  float depth = texture(uShadowMap, uv).r;
  if (depth < gl_FragCoord.z)
    return 0.5;
  return 1.0;
}

// TODO: Lighting? Probably could calculate a normal using a simple
// pass-through shader.
void main() {
  if (uDrawingForShadowMap) {
    // Uncomment this to see fun in mesa. Will try to see what's going on or if
    // I can reproduce on nvidia.
    // gl_FragDepth = gl_FragCoord.w;
    return;
  }


  // Multiplication per uDimension is to keep aspect ratio of the original
  // image, though we shouldn't keep the hard-coded number here.
  vec4 color = texture2D(uCover, fUv * uDimension / 100.0);
  oFragColor = color * getShadow();
}
