#include "base/Platform.h"
#include "base/gl.h"

#include <string>

// FIXME: Detect if SFML will use EGL or not, if it will this is useless.
#ifdef OS_LINUX
#include <X11/Xlib.h>
#endif

void Platform::init() {
#ifdef OS_LINUX
  XInitThreads();
#endif
}

void Platform::shutDown() {}

int Platform::getGLVersion() {
  static int sVersion = -2;

  if (sVersion == -2) {
    GLint glVersion = -1;
    glGetIntegerv(GL_MAJOR_VERSION, &glVersion);
    sVersion = glVersion;
  }

  return sVersion;
}
const std::string& Platform::getGLSLVersionAsString() {
  static std::string* sGLSLVersion = nullptr;

  if (!sGLSLVersion) {
    sGLSLVersion = new std::string((getGLVersion() == 4) ? "400" : "330");
  }

  return *sGLSLVersion;
}
