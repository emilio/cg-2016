#pragma once

#include "base/gl.h"
#include "base/Program.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "geometry/Node.h"

#include <stack>

class DrawContext final {
  struct NodeInfo {
    glm::vec3 m_color;
    glm::mat4 m_transform;
  };

  Program& m_program;

  std::stack<NodeInfo> m_stack;

public:
  struct Uniforms {
    GLint m_transform;
    GLint m_color;
  } m_uniforms;

  explicit DrawContext(Program& a_program,
                       const Uniforms& a_uniforms,
                       glm::mat4 a_initialTransform)
    : m_program(a_program), m_uniforms(a_uniforms) {
    m_stack.push(NodeInfo{glm::vec3(0.5, 0.5, 0.5), a_initialTransform});
  }

  const Uniforms& uniforms() const {
    return m_uniforms;
  }

  void push(const Node& a_node) {
    assert(!m_stack.empty());
    glm::vec3 color = m_stack.top().m_color;
    if (const glm::vec3* nodeColor = a_node.color())
      color = *nodeColor;

    m_stack.push(
        NodeInfo{color, m_stack.top().m_transform * a_node.transform()});

    glUniformMatrix4fv(uniforms().m_transform, 1, GL_FALSE,
                       glm::value_ptr(m_stack.top().m_transform));
    glUniform3fv(uniforms().m_color, 1, glm::value_ptr(m_stack.top().m_color));
  }

  void pop() {
    assert(!m_stack.empty());
    m_stack.pop();
  }

  const Program& program() const {
    return m_program;
  }

#ifdef DEBUG
  ~DrawContext() {
    assert(m_stack.size() == 1 && "Unbalanced!");
  }
#endif
};
