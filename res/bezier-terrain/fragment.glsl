#line 1

#if defined(FOR_SHADOW_MAP)

void main() {
}

#else

in vec3 fPosition;
in vec3 fNormal;
in vec2 fUv;

out vec4 oFragColor;

float getShadow() {
  vec4 posLightSpace = uShadowMapViewProjection * vec4(fPosition, 1.0);
  vec3 properCoords = posLightSpace.xyz / posLightSpace.w;
  vec2 uv = properCoords.xy / 2.0 + vec2(0.5, 0.5);
  float depth = texture(uShadowMap, uv).r;
  float bias = 0.0005;
  if (depth < gl_FragCoord.z + bias) {
    float d = gl_FragCoord.z + bias - depth;
    // FIXME: Should linearize!
    return 1.0 / d;
  }
  return 0.0;
}

void main() {
  float shadow = getShadow();

  vec4 diffuseColor = texture2D(uCover, fUv);
  // TODO: uAmbientLightStrength
  vec4 ambient = diffuseColor * vec4(1.0, 1.0, 1.0, 1.0) * 0.5;

  vec3 lightDirection = normalize(uLightSourcePosition - vec3(fPosition));
  float diffuseImpact = max(dot(fNormal, lightDirection), 0.0);
  // TODO: Make an uniform out of it.
  vec4 lightSourceColor = vec4(1.0, 1.0, 1.0, 1.0);
  vec4 diffuse = diffuseColor * lightSourceColor * diffuseImpact;

  oFragColor = ambient + diffuse * (1 - shadow);
}

#endif
