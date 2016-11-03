#pragma once

#include <cassert>
#include <memory>
#include "base/gl.h"
#include "base/ErrorChecker.h"
#include "tools/Optional.h"

struct ShaderSet {
  std::string m_commonHeader;
  std::string m_vertex;
  std::string m_fragment;
  std::string m_geometry;

  ShaderSet(std::string a_common, std::string a_vertex, std::string a_fragment, std::string a_geometry)
    : m_commonHeader(std::move(a_common))
    , m_vertex(std::move(a_vertex))
    , m_fragment(std::move(a_fragment))
    , m_geometry(std::move(a_geometry)) {}

  ShaderSet(std::string a_common, std::string a_vertex, std::string a_fragment)
    : ShaderSet(std::move(a_common), std::move(a_vertex), std::move(a_fragment), "") {}

  ShaderSet(std::string a_vertex, std::string a_fragment)
    : ShaderSet("", std::move(a_vertex), std::move(a_fragment)) {}
};


enum class ShaderKind {
  Vertex = GL_VERTEX_SHADER,
  Fragment = GL_FRAGMENT_SHADER,
  Geometry = GL_GEOMETRY_SHADER,
};

class Program;

class Shader {
  friend class Program;

  // FIXME: This loses the point :(
  friend class Optional<Shader>;

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
  Optional<Shader> m_geometryShader;

  Program(GLuint a_id,
          GLuint a_vertexShaderId,
          GLuint a_fragmentShaderId,
          Optional<GLuint> a_geometryShaderId)
    : m_id(a_id)
    , m_vertexShader(a_vertexShaderId, ShaderKind::Vertex)
    , m_fragmentShader(a_fragmentShaderId, ShaderKind::Fragment)
    , m_geometryShader(None) {
    if (a_geometryShaderId.isSome())
      m_geometryShader.set(*a_geometryShaderId, ShaderKind::Geometry);

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

  const Shader& vertexShader() const {
    return m_vertexShader;
  }

  Shader& fragmentShader() {
    return m_fragmentShader;
  }

  const Shader& fragmentShader() const {
    return m_fragmentShader;
  }

  Shader* geometryShader() {
    if (m_geometryShader)
      return &*m_geometryShader;
    return nullptr;
  }

  const Shader* geometryShader() const {
    if (m_geometryShader)
      return &*m_geometryShader;
    return nullptr;
  }

  static std::unique_ptr<Program> fromShaders(const ShaderSet&);

  ~Program() {
    glDeleteProgram(m_id);
  }
};
