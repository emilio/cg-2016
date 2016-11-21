#pragma once

#include <string>

class Path final {
  std::string m_inner;

public:
#ifdef OS_WINDOWS
  const char DELIMITER = '\\';
#else
  const char DELIMITER = '/';
#endif

  explicit Path(const std::string& a_string) : m_inner(a_string) {}
  Path(const Path& a_other) : m_inner(a_other.m_inner) {}

  void pop();
  void push(const std::string& fragment);

  const std::string& as_str() const {
    return m_inner;
  }

  const char* c_str() const {
    return m_inner.c_str();
  }
};
