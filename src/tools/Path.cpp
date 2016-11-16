#include "Path.h"

void Path::pop() {
  auto pos = m_inner.rfind(DELIMITER);
  if (pos != std::string::npos)
    m_inner.erase(pos);
}

void Path::push(const std::string& path) {
  if (m_inner.empty()) {
    m_inner = path;
    return;
  }

  if (m_inner[m_inner.length() - 1] != DELIMITER)
    m_inner.append(1, DELIMITER);

  m_inner.append(path);
}
