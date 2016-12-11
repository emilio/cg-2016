layout (vertices = 16) out;

// TODO: Be better at this.
float chooseTessLevel() {
  if (!uLodEnabled)
    return uLodLevel;

  vec3 pos = vec3(gl_in[gl_InvocationID].gl_Position);
  pos = vec3(uModel * vec4(pos, 1.0));

  float d = abs(distance(uCameraPosition, pos));
  if (d > 50.0)
    return 1.0;

  if (d > 10.0)
    return 4.0;

  return 7.0;
}

void main() {
  // Pass through the position.
  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

  if (gl_InvocationID == 0) {
    float tessLevel = chooseTessLevel();

    gl_TessLevelInner[0] = tessLevel;
    gl_TessLevelInner[1] = tessLevel;

    gl_TessLevelOuter[0] = tessLevel;
    gl_TessLevelOuter[1] = tessLevel;
    gl_TessLevelOuter[2] = tessLevel;
    gl_TessLevelOuter[3] = tessLevel;
  }
}
