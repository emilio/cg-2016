#pragma once

#include "base/gl.h"
#include "base/Program.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "geometry/Material.h"
#include "geometry/Node.h"

#include <stack>

class DrawContext final {
  const Program& m_program;
  std::stack<glm::mat4> m_stack;

public:
  struct Uniforms {
    GLint m_transform;
    GLint m_usesTexture;
    GLint m_texture;
    MaterialUniforms m_material;
  } m_uniforms;

  explicit DrawContext(const Program& a_program,
                       const Uniforms& a_uniforms,
                       glm::mat4 a_initialTransform)
    : m_program(a_program), m_uniforms(a_uniforms) {
    m_stack.push(a_initialTransform);
  }

  const Uniforms& uniforms() const {
    return m_uniforms;
  }

  void push(const Node& a_node) {
    m_stack.push(m_stack.top() * a_node.transform());

    glUniformMatrix4fv(uniforms().m_transform, 1, GL_FALSE,
                       glm::value_ptr(m_stack.top()));
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
