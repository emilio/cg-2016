layout(triangles, equal_spacing, ccw) in;

#if !defined(HAS_GEOMETRY_SHADER)
out vec3 fNormal;
out vec3 fPosition;
#endif

// Let the geometry shader calculate the normal.
void main() {
  vec4 position = (gl_TessCoord.x * gl_in[0].gl_Position) +
                  (gl_TessCoord.y * gl_in[1].gl_Position) +
                  (gl_TessCoord.z * gl_in[2].gl_Position);

#if !defined(HAS_GEOMETRY_SHADER)
  vec3 A = vec3(gl_in[1].gl_Position - gl_in[0].gl_Position);
  vec3 B = vec3(gl_in[2].gl_Position - gl_in[0].gl_Position);
  vec3 normal = normalize(cross(A, B));
  fNormal = normalize(vec3(uModel * vec4(normal, 1.0)));
  fPosition = vec3(uModel * position);
  gl_Position = uViewProjection * uModel * position;
#else
  // Let it handle the boring stuff, and do other stuff if I eventually want
  // that.
  gl_Position = position;
#endif
}
