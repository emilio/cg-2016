#line 1

out vec4 oFragColor;

in vec3 fPosition;
in vec3 fNormal;

void main() {
  // First, the ambient light.
  vec3 ambientColor = uAmbientLightColor * uAmbientLightStrength;


  // Then the diffuse light.
  vec3 lightDirection = normalize(uLightSourcePosition - fPosition);
  float diffuseImpact = max(dot(fNormal, lightDirection), 0.0);
  vec3 diffuse = uLightSourceColor * diffuseImpact;

  // Then the specular strength to finish up the Phong model.
  // FIXME: Stop hard-coding strength an exponent?
  float specularStrength = 0.5;
  vec3 viewDirection = normalize(uCameraPosition - fPosition);
  vec3 reflectionDirection = reflect(-lightDirection, fNormal);
  float spec = pow(max(dot(viewDirection, reflectionDirection), 0.0), 4);
  vec3 specular = specularStrength * spec * uLightSourceColor;

  oFragColor = vec4((ambientColor + diffuse + specular) * uColor, 1.0);
  // oFragColor = vec4(0.5, 0.0, 0.5, 1.0);
  // gl_FragColor = vec4 (0.5, 0.0, 0.5, 1.0);
}
