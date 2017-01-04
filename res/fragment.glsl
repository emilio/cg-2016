#line 1

out vec4 oFragColor;

in vec3 fPosition;
in vec3 fNormal;
in vec2 fUv;

#define PCF_RANGE 2
#define BIAS 0.05

float getShadow() {
  vec4 posLightSpace = uShadowMapViewProjection * vec4(fPosition, 1.0);
  vec3 properCoords = posLightSpace.xyz / posLightSpace.w;
  if (properCoords.z > 1.0)
    return 0.0;
  // The  coordinates are in normalized device coords, convert back to [0, 1]..
  properCoords = properCoords * 0.5 + 0.5;
  vec2 uv = properCoords.xy;

  // percentage-closer filtering.
  // http://http.developer.nvidia.com/GPUGems/gpugems_ch11.html
  float currentDepth = properCoords.z;
  // return currentDepth;

  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
  for(int x = -PCF_RANGE; x <= PCF_RANGE; ++x) {
    for(int y = -PCF_RANGE; y <= PCF_RANGE; ++y) {
      float pcfDepth = texture(uShadowMap, uv + vec2(x, y) * texelSize).r;
      shadow += currentDepth - BIAS > pcfDepth ? 1.0 : 0.0;
    }
  }

  if (PCF_RANGE == 0)
    return shadow;

  shadow /= pow(PCF_RANGE * 2 + 1, 2);
  return shadow;
}

void main() {
  if (uDrawingForShadowMap)
    return;

  vec4 diffuseColor = uMaterial.m_diffuse;

  if (uUsesTexture)
    diffuseColor = texture2D(uTexture, fUv);

  // First, the ambient light.
  // NOTE: We're multiplying for the diffuse color because otherwise the result
  // is pretty lame. I don't know for sure if it's due to the model or how I
  // read it.
  vec4 ambient =
    diffuseColor * uMaterial.m_ambient * vec4(uAmbientLightColor, 1.0) * uAmbientLightStrength;

  // Then the diffuse light.
  vec3 lightDirection = normalize(uLightSourcePosition - fPosition);
  float diffuseImpact = max(dot(fNormal, lightDirection), 0.0);
  vec4 diffuse = diffuseColor * vec4(uLightSourceColor, 1.0) * diffuseImpact;

  // Then the specular strength to finish up the Phong model.
  vec3 viewDirection = normalize(uCameraPosition - fPosition);
  vec3 reflectionDirection = reflect(-lightDirection, fNormal);
  float spec = 0.0;

  if (uMaterial.m_shininess_percent > 0.0)
    spec = uMaterial.m_shininess_percent;
  else
    spec = pow(max(dot(viewDirection, reflectionDirection), 0.0),
               uMaterial.m_shininess);

  vec4 specular = uMaterial.m_specular * spec * vec4(uLightSourceColor, 1.0);
  float shadow = getShadow();
  oFragColor = ambient + diffuse * (1 - shadow) + specular;
}
