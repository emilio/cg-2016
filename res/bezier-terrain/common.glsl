#version 400

/** Same meaning as the ones in ../common.glsl. */
uniform mat4 uViewProjection;
uniform mat4 uModel;
uniform vec3 uCameraPosition;

/** The texture for UV mapping */
uniform sampler2D uCover;

uniform float uDimension;

/** Whether dynamic tessellation is enabled */
uniform bool uLodEnabled;

/** The level of detail hard-coded if uLodEnabled is false. */
uniform float uLodLevel;
