#pragma once

#include "base/Logging.h"

#define ASSERT(t)                                                              \
  do {                                                                         \
    if (!(t)) {                                                                \
      ERROR("Assertion failure: " #t "\n");                                    \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

#define ASSERT_EQ(t, expected) ASSERT((t) == (expected))
#define ASSERT_NEQ(t, expected) ASSERT((t) != (expected))
