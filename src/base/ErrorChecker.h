#pragma once

#include "base/gl.h"
#include <cassert>
#include <cstdio>

#ifdef GL_DEBUG
static const char* GLErrorToString(GLuint error) {
  switch (error) {
    case GL_INVALID_VALUE:
      return "GL_INVALID_VALUE";
    case GL_INVALID_ENUM:
      return "GL_INVALID_ENUM";
    case GL_INVALID_OPERATION:
      return "GL_INVALID_OPERATION";
    case GL_STACK_OVERFLOW:
      return "GL_STACK_OVERFLOW";
    case GL_STACK_UNDERFLOW:
      return "GL_STACK_UNDERFLOW";
    case GL_OUT_OF_MEMORY:
      return "GL_OUT_OF_MEMORY";
    default:
      assert(!"Unexpected GL error code");
  }
  return nullptr;
}
#endif

/**
 * Simple RAAI class to ensure there are no errors in debug builds.
 */
class AutoGLErrorChecker {
public:
  AutoGLErrorChecker() {
#ifdef GL_DEBUG
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
      fprintf(stderr, "ERROR: %s\n", GLErrorToString(error));
      assert(!"Unexpected GL error (on entrance)!");
    }
#endif
  }
  ~AutoGLErrorChecker() {
#ifdef GL_DEBUG
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
      fprintf(stderr, "ERROR: %s\n", GLErrorToString(error));
      assert(!"Unexpected GL error!");
    }
#endif
  }
};
