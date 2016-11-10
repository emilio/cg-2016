#pragma once

// A class to add platform-specific bits needed in all the executables.
class Platform final {
public:
  /**
   * Perform platform-specific initialization.
   */
  static void init();
  static void shutDown();
};
