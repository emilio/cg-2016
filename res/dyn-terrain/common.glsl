/** Same meaning as the ones in ../common.glsl. */
uniform mat4 uViewProjection;
uniform mat4 uModel;
uniform mat4 uShadowMapViewProjection;

uniform vec3 uCameraPosition;
uniform vec3 uLightSourcePosition;

/** The texture for UV mapping */
uniform sampler2D uCover;

/** The heightmap */
uniform sampler2D uHeightMap;

/** The the shadow map with the scene objects */
uniform sampler2D uShadowMap;

uniform float uDimension;

float getHeight(vec2 pos) {
  pos += vec2(0.5, 0.5);
  float v = texture2D(uHeightMap, pos).g;
  return (v - 0.5) / 3.0;
}
