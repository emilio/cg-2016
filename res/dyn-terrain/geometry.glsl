#line 1

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

out vec3 fPosition;
out vec3 fNormal;
out vec2 fUv;

void emitPos(vec4 a_untransformedPos) {
  fUv = vec2(a_untransformedPos.x, a_untransformedPos.z);
  fPosition = vec3(uModel * a_untransformedPos);
  gl_Position = uViewProjection * uModel * a_untransformedPos;
  EmitVertex();
}

// This is a very simple geometry shader that just takes care of calculating
// normals and passing everything down again.
void main() {
  // Calculate the normal on the fly
  vec3 A = vec3(gl_in[1].gl_Position - gl_in[0].gl_Position);
  vec3 B = vec3(gl_in[2].gl_Position - gl_in[0].gl_Position);

	// FIXME: Same problem than in the vertex shader, no proper normal matrix.
  vec3 normal = normalize(cross(A, B));
  fNormal = normalize(vec3(uModel * vec4(normal, 0.0)));

  emitPos(gl_in[0].gl_Position);
  emitPos(gl_in[1].gl_Position);
  emitPos(gl_in[2].gl_Position);

  EndPrimitive();
}
