#include "base/Logging.h"
#include "base/ErrorChecker.h"
#include "base/DebuggingUtils.h"

void DebuggingUtils::dumpRenderingInfo() {
  AutoGLErrorChecker checker;
  GLint paramValue;

  LOG("Renderer: %s", glGetString(GL_RENDERER));
  LOG("OpenGL version: %s", glGetString(GL_VERSION));

#define LOG_PARAM(name_)                                                       \
  glGetIntegerv(name_, &paramValue);                                           \
  LOG(#name_ ": %d", paramValue);

  LOG_PARAM(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
  LOG_PARAM(GL_MAX_CUBE_MAP_TEXTURE_SIZE)
  LOG_PARAM(GL_MAX_DRAW_BUFFERS)
  LOG_PARAM(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS)
  LOG_PARAM(GL_MAX_TEXTURE_IMAGE_UNITS)
  LOG_PARAM(GL_MAX_TEXTURE_SIZE)
  LOG_PARAM(GL_MAX_VARYING_FLOATS)
  LOG_PARAM(GL_MAX_VERTEX_ATTRIBS)
  LOG_PARAM(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS)
  LOG_PARAM(GL_MAX_VERTEX_UNIFORM_COMPONENTS)
  LOG_PARAM(GL_MAX_VIEWPORT_DIMS)
  LOG_PARAM(GL_STEREO)

#undef LOG_PARAM
}
