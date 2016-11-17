#line 1

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

#if defined(HAS_TESS_CONTROL_SHADER)
in vec2 fUvFromTess[3];
#define UV fUvFromTess
#else
in vec2 fUvFromVertex[3];
#define UV fUvFromVertex
#endif

out vec3 fNormal;
out vec2 fUv;
out vec3 fPosition;

void emitPos(vec4 a_untransformedPos, vec2 a_Uv) {
  fUv = a_Uv;
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

  // FIXME: Same problem than in the fragment shader, no proper normal matrix.
  vec3 normal = normalize(cross(A, B));
  fNormal = normalize(vec3(uModel * vec4(normal, 0.0)));

  emitPos(gl_in[0].gl_Position, UV[0]);
  emitPos(gl_in[1].gl_Position, UV[1]);
  emitPos(gl_in[2].gl_Position, UV[2]);

  EndPrimitive();
}
