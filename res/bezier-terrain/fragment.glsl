#line 1

#if defined(FOR_SHADOW_MAP)

void main() {
}

#else

in vec4 fPosition;
in vec2 fUv;

out vec4 oFragColor;

float getShadow() {
  vec4 posLightSpace = uShadowMapViewProjection * uModel * fPosition;
  vec3 properCoords = posLightSpace.xyz / posLightSpace.w;
  vec2 uv = properCoords.xy / 2.0 + vec2(0.5, 0.5);
  float depth = texture(uShadowMap, uv).r;
  if (depth < gl_FragCoord.z)
    return 1. - (gl_FragCoord.z - depth) / gl_FragCoord.z;
  return 1.0;
}

// TODO: Lighting? Probably could calculate a normal using a simple
// pass-through shader.
void main() {
  // Multiplication per uDimension is to keep aspect ratio of the original
  // image, though we shouldn't keep the hard-coded number here.
  vec4 color = texture2D(uCover, fUv * uDimension / 100.0);
  oFragColor = color * getShadow();
}

#endif
