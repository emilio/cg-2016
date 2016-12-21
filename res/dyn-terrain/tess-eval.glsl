layout(triangles, equal_spacing) in;

void main() {
  gl_Position = gl_TessCoord.x * gl_in[0].gl_Position +
                gl_TessCoord.y * gl_in[1].gl_Position +
                gl_TessCoord.z * gl_in[2].gl_Position;
  gl_Position.y = getHeight(vec2(gl_Position.x, gl_Position.z));
  // Geometry takes care of this.
  // gl_Position = uViewProjection * uModel * gl_Position;
}
