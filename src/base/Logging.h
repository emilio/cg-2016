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

#define LOG_MATRIX(name_, var_)                                                \
  LOG(name_ ":\n %f %f %f %f\n %f %f %f %f\n %f %f %f %f\n %f %f %f %f",       \
      var_[0][0], var_[0][1], var_[0][2], var_[0][3], var_[1][0], var_[1][1],  \
      var_[1][2], var_[1][3], var_[2][0], var_[2][1], var_[2][2], var_[2][3],  \
      var_[3][0], var_[3][1], var_[3][2], var_[3][3]);
