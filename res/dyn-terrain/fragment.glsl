in vec3 fPosition;
in vec3 fNormal;
in vec2 fUv;

out vec4 oFragColor;

#define PCF_RANGE 2

float getShadow() {
  vec4 posLightSpace = uShadowMapViewProjection * vec4(fPosition, 1.0);
  vec3 properCoords = posLightSpace.xyz / posLightSpace.w;
  if (properCoords.z > 1.0)
    return 0.0;
  vec2 uv = properCoords.xy / 2.0 + vec2(0.5, 0.5);

  // percentage-closer filtering
  float currentDepth = gl_FragCoord.z;

  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
  float bias = 0.0005;
  for(int x = -PCF_RANGE; x <= PCF_RANGE; ++x) {
      for(int y = -PCF_RANGE; y <= PCF_RANGE; ++y) {
          float pcfDepth = texture(uShadowMap, uv + vec2(x, y) * texelSize).r;
          shadow += currentDepth > pcfDepth ? 1.0 : 0.0;
      }
  }

  shadow /= pow(PCF_RANGE * 2 + 1, 2);
  return shadow;
}

void main() {
  float shadow = getShadow();

  // TODO: uAmbientLightStrength
  vec2 uv = fUv.xy / 2.0 + vec2(0.5, 0.5);
  // vec2 uv = fUv;
  vec4 diffuseColor = texture2D(uCover, uv);
  oFragColor = diffuseColor;
  vec4 ambient = diffuseColor * vec4(1.0, 1.0, 1.0, 1.0) * 0.5;

  vec3 lightDirection = normalize(uLightSourcePosition - vec3(fPosition));
  float diffuseImpact = max(dot(fNormal, lightDirection), 0.0);
  // TODO: Make an uniform out of it.
  vec4 lightSourceColor = vec4(1.0, 1.0, 1.0, 1.0);
  vec4 diffuse = diffuseColor * lightSourceColor * diffuseImpact;

  oFragColor = ambient + diffuse * (1 - shadow);
}
