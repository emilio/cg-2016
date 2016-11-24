#version 400

/** Same meaning as the ones in ../common.glsl. */
uniform mat4 uViewProjection;
uniform mat4 uModel;
uniform vec3 uCameraPosition;

/** The texture for UV mapping */
uniform sampler2D uCover;

uniform float uDimension;
