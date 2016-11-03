#include <fstream>
#include <sstream>

#include "base/Program.h"
#include "base/ErrorChecker.h"
#include "base/Logging.h"

static bool createShaderFromSource(ShaderKind a_kind,
                                   const char* a_source,
                                   const std::string& a_prefix,
                                   GLuint& a_shader) {
  AutoGLErrorChecker checker;

  assert(a_source);

  LOG("Shader: %s, %s, %s",
      a_kind == ShaderKind::Vertex ? "Vertex" : "Fragment", a_prefix.c_str(),
      a_source);

  a_shader = glCreateShader(GLenum(a_kind));
  if (a_prefix.empty()) {
    glShaderSource(a_shader, 1, &a_source, nullptr);
  } else {
    const char* sources[] = {a_prefix.c_str(), a_source};
    glShaderSource(a_shader, 2, sources, nullptr);
  }
  glCompileShader(a_shader);

  GLint success;
  glGetShaderiv(a_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLint logSize = 0;
    glGetShaderiv(a_shader, GL_INFO_LOG_LENGTH, &logSize);

    assert(logSize >= 0);
    if (logSize) {
      std::unique_ptr<char> chars(new char[logSize]);
      glGetShaderInfoLog(a_shader, logSize, nullptr, chars.get());
      ERROR("Shader compilation failed: %s", chars.get());
    }

    glDeleteShader(a_shader);
    a_shader = 0;
    return false;
  }

  return true;
}

static bool createShaderOfKind(ShaderKind a_kind,
                               const std::string& a_from,
                               const std::string& a_prefix,
                               GLuint& a_shader) {
  std::stringstream buff;
  std::ifstream stream(a_from);
  buff << stream.rdbuf();

  std::string string = buff.str();
  return createShaderFromSource(a_kind, string.c_str(), a_prefix, a_shader);
}

/* static */ std::unique_ptr<Program> Program::fromShaders(
    const ShaderSet& a_shaderSet) {
  GLuint vertexShaderId, fragmentShaderId, potentialGeometryShaderId;
  Optional<GLuint> geometryShaderId;

  // We need at least one of these.
  if (a_shaderSet.m_vertex.empty() || a_shaderSet.m_fragment.empty())
    return nullptr;

  std::string prefix;
  std::stringstream prefixBuff;
  if (!a_shaderSet.m_commonHeader.empty()) {
    std::ifstream stream(a_shaderSet.m_commonHeader);
    prefixBuff << stream.rdbuf();
    prefix = prefixBuff.str();
  }

  if (!createShaderOfKind(ShaderKind::Vertex, a_shaderSet.m_vertex, prefix,
                          vertexShaderId)) {
    ERROR("Shader compilation failed: %s", a_shaderSet.m_vertex.c_str());
    return nullptr;
  }

  if (!createShaderOfKind(ShaderKind::Fragment, a_shaderSet.m_fragment, prefix,
                          fragmentShaderId)) {
    ERROR("Shader compilation failed: %s", a_shaderSet.m_fragment.c_str());
    return nullptr;
  }

  if (!a_shaderSet.m_geometry.empty()) {
    if (!createShaderOfKind(ShaderKind::Geometry, a_shaderSet.m_geometry,
                            prefix, potentialGeometryShaderId)) {
      ERROR("Shader compilation failed: %s", a_shaderSet.m_geometry.c_str());
      return nullptr;
    }
    geometryShaderId.set(potentialGeometryShaderId);
  }


  GLuint id = glCreateProgram();

  AutoGLErrorChecker checker;

  glAttachShader(id, vertexShaderId);
  glAttachShader(id, fragmentShaderId);

  if (geometryShaderId)
    glAttachShader(id, *geometryShaderId);

  glLinkProgram(id);

  glValidateProgram(id);

  GLint linkSuccess;
  glGetProgramiv(id, GL_LINK_STATUS, &linkSuccess);

  GLint validateSuccess;
  glGetProgramiv(id, GL_VALIDATE_STATUS, &validateSuccess);

  if (!linkSuccess || !validateSuccess) {
    fprintf(stderr, linkSuccess ? "Program validation failed\n"
                                : "Program failed to link\n");
    GLint logSize;
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logSize);
    assert(logSize >= 0);
    if (logSize) {
      std::unique_ptr<char> chars(new char[logSize]);
      glGetProgramInfoLog(id, logSize, nullptr, chars.get());
      fprintf(stderr, "  Log: %s\n", chars.get());
    }
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);
    glDeleteProgram(id);
    return nullptr;
  }

  // NB: Not using make_unique because constructor is public.
  return std::unique_ptr<Program>(
      new Program(id, vertexShaderId, fragmentShaderId, std::move(geometryShaderId)));
}
