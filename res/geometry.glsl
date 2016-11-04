#line 1

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
// layout(triangles) out;

out vec3 fNormal;
out vec3 fPosition;

void emitPos(vec4 a_untransformedPos) {
  fPosition = vec3(uModel * a_untransformedPos);
  gl_Position = uViewProjection * uModel * a_untransformedPos;
  EmitVertex();
}

void main() {
  // Calculate the normal on the fly
  //
  // NOTE: This is sort of different from the output we'd get with a normal
  // mesh with precomputed normals, because of how we compute them.
  //
  // In particular, this is noticeable in the terrain (try removing the
  // geometry shader), because we override the normal stored for a vertex with
  // the normal for the *last* face we've computed for it, which in this case
  // Makes for a really smooth effect.
  vec3 A = vec3(gl_in[1].gl_Position - gl_in[0].gl_Position);
  vec3 B = vec3(gl_in[2].gl_Position - gl_in[0].gl_Position);

  // FIXME: Same problem than in the fragment shader, no proper normal matrix.
  vec3 normal = normalize(cross(A, B));

  // fNormal = normalize(vec3(uModel * normalize(normal)));
  fNormal = normalize(vec3(uModel * vec4(normal, 1.0)));
  emitPos(gl_in[0].gl_Position);
  emitPos(gl_in[1].gl_Position);
  emitPos(gl_in[2].gl_Position);

  EndPrimitive();
}
