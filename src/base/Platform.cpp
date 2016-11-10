#include "base/Platform.h"

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
