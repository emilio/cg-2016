#pragma once

#include <cassert>
#include <memory>
#include "base/gl.h"
#include "base/ErrorChecker.h"

enum class ShaderKind {
  Vertex = GL_VERTEX_SHADER,
  Fragment = GL_FRAGMENT_SHADER,
};

class Program;

class Shader {
  friend class Program;

  Shader(GLuint a_id, ShaderKind a_kind) : m_id(a_id), m_kind(a_kind) {
    assert(glIsShader(a_id));
  }

  Shader(const Shader& other) = delete;
  Shader(const Shader&& other) = delete;

  GLuint m_id;
  ShaderKind m_kind;

public:
  GLuint id() {
    return m_id;
  }
  ShaderKind kind() {
    return m_kind;
  }

  ~Shader() {
    glDeleteShader(m_id);
  }
};

class Program {
  GLuint m_id;
  Shader m_vertexShader;
  Shader m_fragmentShader;

  Program(GLuint a_id, GLuint a_vertexShaderId, GLuint a_fragmentShaderId)
    : m_id(a_id)
    , m_vertexShader(a_vertexShaderId, ShaderKind::Vertex)
    , m_fragmentShader(a_fragmentShaderId, ShaderKind::Fragment) {
    assert(glIsProgram(m_id));
  }

public:
  Program(const Program& other) = delete;
  Program(const Program&& other) = delete;

  void use() {
    AutoGLErrorChecker checker;
    glUseProgram(m_id);
  }

  GLuint id() {
    return m_id;
  }
  Shader& vertexShader() {
    return m_vertexShader;
  }
  Shader& fragmentShader() {
    return m_fragmentShader;
  }

  static std::unique_ptr<Program> fromShaderFiles(const char* a_fragmentShader,
                                                  const char* a_vertexShader);

  ~Program() {
    glDeleteProgram(m_id);
  }
};
