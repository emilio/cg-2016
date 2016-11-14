#line 1

out vec4 oFragColor;

in vec3 fPosition;
in vec3 fNormal;

void main() {
  // oFragColor = vec4(fNormal, 1.0);
  // return;

  // First, the ambient light.
  // NOTE: We're multiplying for the diffuse color because otherwise the result
  // is pretty lame. I don't know for sure if it's due to the model or how I
  // read it.
  vec4 ambient =
    uMaterial.m_diffuse * uMaterial.m_ambient * vec4(uAmbientLightColor, 1.0) * uAmbientLightStrength;

  // Then the diffuse light.
  vec3 lightDirection = normalize(uLightSourcePosition - fPosition);
  float diffuseImpact = max(dot(fNormal, lightDirection), 0.0);
  vec4 diffuse = uMaterial.m_diffuse * vec4(uLightSourceColor, 1.0) * diffuseImpact;

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

  oFragColor = diffuse + ambient + specular;
  // oFragColor = vec4(0.5, 0.0, 0.5, 1.0);
  // gl_FragColor = vec4 (0.5, 0.0, 0.5, 1.0);
}
