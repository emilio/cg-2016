#line 1

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vUv;

/** The fragment position in world space, passed to the fragment shader. */
out vec3 fPosition;

/** The fragment normal position, given to the fragment shader. */
out vec3 fNormal;

void main () {
  fPosition = vec3(uModel * vec4(vPosition, 1.0));

  // FIXME: For this to work properly it requires linear scaling transforms.
  //
  // This is not a big deal, because we don't use it, but otherwise we'd need
  // something like:
  //
  // http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
  fNormal = normalize(vec3(uModel * vec4(vNormal, 1.0)));
  gl_Position = uViewProjection * uModel * vec4(vPosition, 1.0);
}
