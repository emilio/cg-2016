#version 330

/** The view-projection transform to use. */
uniform mat4 uViewProjection;

/** The model transform */
uniform mat4 uModel;

/**
 * The plain color of this model, used for lightning.
 *
 * FIXME: Use proper material properties, this sort of sucks.
 */
uniform vec3 uColor;

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

/** The camera position */
uniform vec3 uCameraPosition;
