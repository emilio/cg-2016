layout(triangles, equal_spacing, ccw) in;

out vec2 fUv;

void main() {
  gl_Position = gl_TessCoord.x * gl_in[0].gl_Position +
                gl_TessCoord.y * gl_in[1].gl_Position +
                gl_TessCoord.z * gl_in[2].gl_Position;
  gl_Position.y = getHeight(vec2(gl_Position.x, gl_Position.z));
  fUv = vec2(gl_Position.x, gl_Position.z);

  gl_Position = uViewProjection * uModel * gl_Position;
}
