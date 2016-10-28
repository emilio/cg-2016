#pragma once

#include "base/gl.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <stack>

class DrawContext final {
  std::stack<glm::mat4> m_transformStack;
  GLuint m_transformUniform;

public:
  explicit DrawContext(GLuint a_transformUniform, glm::mat4 a_initialTransform)
    : m_transformUniform(a_transformUniform)
  {
    m_transformStack.push(a_initialTransform);
  }

  void pushTransform(glm::mat4 a_transform) {
    assert(!m_transformStack.empty());

    m_transformStack.push(m_transformStack.top() * a_transform);
    glUniformMatrix4fv(m_transformUniform, 1, GL_FALSE,
                       glm::value_ptr(m_transformStack.top()));
  }

  void popTransform() {
    m_transformStack.pop();
  }

#ifdef DEBUG
  ~DrawContext() {
    assert(m_transformStack.size() == 1 && "Unbalanced!");
  }
#endif
};
