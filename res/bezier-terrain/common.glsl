uniform mat4 uModel;
uniform mat4 uShadowMapViewProjection;

#if !defined(FOR_SHADOW_MAP)
/** Same meaning as the ones in ../common.glsl. */
uniform mat4 uViewProjection;

uniform vec3 uCameraPosition;

uniform vec3 uLightSourcePosition;

/** The texture for UV mapping */
uniform sampler2D uCover;

/** The shadow map (when we're not rendering _for_ a shadow map). */
uniform sampler2D uShadowMap;

uniform float uDimension;

/** Whether dynamic tessellation is enabled */
uniform bool uLodEnabled;

/** The level of detail hard-coded if uLodEnabled is false. */
uniform float uLodLevel;
#endif
