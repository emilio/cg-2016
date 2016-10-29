#pragma once

#include "base/gl.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "geometry/Node.h"

#include <stack>

class DrawContext final {
  struct NodeInfo {
    glm::vec3 m_color;
    glm::mat4 m_transform;
  };

  std::stack<NodeInfo> m_stack;

  GLuint m_transformUniform;
  GLuint m_colorUniform;

public:
  explicit DrawContext(GLuint a_transformUniform,
                       GLuint a_colorUniform,
                       glm::mat4 a_initialTransform)
    : m_transformUniform(a_transformUniform), m_colorUniform(a_colorUniform) {
    m_stack.push(NodeInfo{glm::vec3(0.0, 0.0, 0.0), a_initialTransform});
  }

  void push(const Node& a_node) {
    assert(!m_stack.empty());

    m_stack.push(NodeInfo{a_node.color(),
                          m_stack.top().m_transform * a_node.transform()});

    glUniformMatrix4fv(m_transformUniform, 1, GL_FALSE,
                       glm::value_ptr(m_stack.top().m_transform));
    glUniform3fv(m_colorUniform, 1, glm::value_ptr(m_stack.top().m_color));
  }

  void pop() {
    assert(!m_stack.empty());
    m_stack.pop();
  }

#ifdef DEBUG
  ~DrawContext() {
    assert(m_stack.size() == 1 && "Unbalanced!");
  }
#endif
};
