#line 1

layout (location = 0) in vec2 vPosition;

void main () {
#if defined(HAS_TESS_CONTROL_SHADER)
  gl_Position = vec4(vPosition.x, 0.0, vPosition.y, 1.0);
#else
  float h = texture2D(uHeightMap, vPosition).r;
  gl_Position = uViewProjection * uModel * vec4(vPosition.x, h, vPosition.y, 1.0);
#endif
}
