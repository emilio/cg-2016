#pragma once

#ifdef DEBUG
#define LOG_INTERNAL(type, msg, ...)                                           \
  fprintf(stderr, "[" type "] " msg "\n", ##__VA_ARGS__);
#else
#define LOG_INTERNAL(...)
#endif

#define LOG(msg, ...) LOG_INTERNAL("debug", msg, ##__VA_ARGS__)
#define WARN(msg, ...) LOG_INTERNAL("warn", msg, ##__VA_ARGS__)
#define ERROR(msg, ...) LOG_INTERNAL("error", msg, ##__VA_ARGS__)
