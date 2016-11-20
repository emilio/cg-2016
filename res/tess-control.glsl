layout (vertices = 3) out;

in vec2 fUvFromVertex[];
out vec2 fUvFromControl[];

float chooseTessLevel() {
  // We always put the object at the origin so it's a bit pointless to
  // transform it, but we could.
  if (length(uCameraPosition) < 2)
    return 7.0;
  return 2.0;
}

void main() {
  // Pass through the position.
  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

  fUvFromControl[gl_InvocationID] = fUvFromVertex[gl_InvocationID];

  // We map the camera distance from the origin from 0.0 to 20.0.
  // float tessLevel = distance(uCameraPosition, vec3(0.0, 0.0, 0.0)) / 20.0;
  float tessLevel = chooseTessLevel();

  gl_TessLevelInner[0] = tessLevel;
  gl_TessLevelInner[1] = tessLevel;

  gl_TessLevelOuter[0] = tessLevel;
  gl_TessLevelOuter[1] = tessLevel;
  gl_TessLevelOuter[2] = tessLevel;
  gl_TessLevelOuter[3] = tessLevel;
}
