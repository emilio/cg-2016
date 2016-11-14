#line 1

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vUv;

// FIXME: We may want the computed normal for this vertex in the geometry
// shader? It's kind of stupid since it can compute it itself, but that means
// that the output is somewhat less smooth in the terrain, sigh.
#if !defined(HAS_GEOMETRY_SHADER) && !defined(HAS_TESS_CONTROL_SHADER)
/** The fragment position in world space, passed to the fragment shader. */
out vec3 fPosition;

/** The fragment normal position, given to the fragment shader. */
out vec3 fNormal;
#endif

void main () {
  // FIXME: For the normal calculation to work properly it requires linear
  // scaling transforms.
  //
  // This is not a big deal, because we don't use it, but otherwise we'd need
  // something like:
  //
  // http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
#if !defined(HAS_GEOMETRY_SHADER) && !defined(HAS_TESS_CONTROL_SHADER)
  fPosition = vec3(uModel * vec4(vPosition, 1.0));
  fNormal = normalize(vec3(uModel * vec4(vNormal, 0.0)));
  gl_Position = uViewProjection * uModel * vec4(vPosition, 1.0);
#else
  // The geometry or tesellation shader takes care of the normals, and transformations.
  gl_Position = vec4(vPosition, 1.0);
#endif
}
