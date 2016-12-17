#version 400

/** Same meaning as the ones in ../common.glsl. */
uniform mat4 uViewProjection;
uniform mat4 uModel;
uniform vec3 uCameraPosition;

/** The texture for UV mapping */
uniform sampler2D uCover;

/** The shadow map (when we're not rendering _for_ a shadow map). */
uniform sampler2D uShadowMap;

uniform float uDimension;

/** Whether dynamic tessellation is enabled */
uniform bool uLodEnabled;

/** The level of detail hard-coded if uLodEnabled is false. */
uniform float uLodLevel;

/** Whether we're doing a shadow map pass */
uniform bool uDrawingForShadowMap;

/**
 * The transformation to convert to light space, that should only be used when
 * _not_ rendering to a shadow map.
 */
uniform mat4 uShadowMapViewProjection;
