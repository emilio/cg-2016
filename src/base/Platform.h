#pragma once

#include <string>

// A class to add platform-specific bits needed in all the executables.
class Platform final {
public:
  /**
   * Perform platform-specific initialization.
   */
  static void init();
  static void shutDown();
  /**
   * Returns the appropriate GLSL and GL version.
   *
   * The first invocation is not thread-safe, but it doesn't really matter,
   * since we only do GL in one thread.
   */
  static const std::string& getGLSLVersionAsString();
  static int getGLVersion();
};
