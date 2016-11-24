layout(quads) in;

out vec2 fUv;

void main() {
	float u = gl_TessCoord.x;
  float v = gl_TessCoord.y;
  vec4 a = mix(gl_in[0].gl_Position, gl_in[3].gl_Position, u);
  vec4 b = mix(gl_in[12].gl_Position, gl_in[15].gl_Position, u);
  gl_Position = mix(a, b, v);
  gl_Position = uViewProjection * uModel * gl_Position;
}
