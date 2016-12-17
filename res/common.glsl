/** The view-projection transform to use. */
uniform mat4 uViewProjection;

/** The model transform */
uniform mat4 uModel;

/** The texture for UV mapping */
uniform bool uUsesTexture;

/** The texture for UV mapping */
uniform sampler2D uTexture;

/** The shadow map */
uniform sampler2D uShadowMap;

/** The model material */
struct Material {
  vec4 m_diffuse;
  vec4 m_specular;
  vec4 m_ambient;
  vec4 m_emissive;
  float m_shininess;
  float m_shininess_percent;
};

uniform Material uMaterial;

/**
 * The current frame we're in, currently just to do fancy stuff because I'm to
 * lazy to use proper timing and stuff.
 */
uniform float uFrame;

/** The color of the ambient light */
uniform vec3 uAmbientLightColor;

/** The strength of the ambient light, from 0 to 1 */
uniform float uAmbientLightStrength;

/**
 * The position of the light source, in world space.
 *
 * We assume a single light source, because we're pussies.
 */
uniform vec3 uLightSourcePosition;

/** The color of the light source */
uniform vec3 uLightSourceColor;

/** The camera position, in world space */
uniform vec3 uCameraPosition;

/** Whether we're doing a shadow map pass */
uniform bool uDrawingForShadowMap;

/** The matrix to transform to light space */
uniform mat4 uShadowMapViewProjection;
