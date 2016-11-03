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
                               const char* a_from,
                               const std::string& a_prefix,
                               GLuint& a_shader) {
  std::stringstream buff;
  std::ifstream stream(a_from);
  buff << stream.rdbuf();

  std::string string = buff.str();
  return createShaderFromSource(a_kind, string.c_str(), a_prefix, a_shader);
}

/* static */ std::unique_ptr<Program> Program::fromShaderFiles(
    const char* a_vertexShader,
    const char* a_fragmentShader,
    const char* a_commonPrefix) {
  GLuint vertexShaderId, fragmentShaderId;

  std::string prefix;
  std::stringstream prefixBuff;
  if (a_commonPrefix) {
    std::ifstream stream(a_commonPrefix);
    prefixBuff << stream.rdbuf();
    prefix = prefixBuff.str();
  }

  if (!createShaderOfKind(ShaderKind::Vertex, a_vertexShader, prefix,
                          vertexShaderId)) {
    ERROR("Shader compilation failed: %s", a_vertexShader);
    return nullptr;
  }

  if (!createShaderOfKind(ShaderKind::Fragment, a_fragmentShader, prefix,
                          fragmentShaderId)) {
    ERROR("Shader compilation failed: %s", a_fragmentShader);
    return nullptr;
  }

  GLuint id = glCreateProgram();

  AutoGLErrorChecker checker;

  glAttachShader(id, vertexShaderId);
  glAttachShader(id, fragmentShaderId);

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
      new Program(id, vertexShaderId, fragmentShaderId));
}
