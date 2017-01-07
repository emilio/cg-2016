#include <fstream>
#include <sstream>

#include "base/Program.h"
#include "base/ErrorChecker.h"
#include "base/Logging.h"

static bool createShaderFromSource(ShaderKind a_kind,
                                   const char* a_source,
                                   const std::string& a_version,
                                   const std::string& a_raw_prefix,
                                   const std::string& a_prefix_from_file,
                                   GLuint& a_shader) {
  AutoGLErrorChecker checker;

  assert(a_source);

  LOG("Shader: %s, #version %s\n%s, %s, %s",
      a_kind == ShaderKind::Vertex ? "Vertex" : "Fragment", a_version.c_str(),
      a_raw_prefix.c_str(), a_prefix_from_file.c_str(), a_source);

  a_shader = glCreateShader(GLenum(a_kind));

  const char* sources[7] = {nullptr};
  size_t size = 0;
  sources[size++] = "#version ";
  sources[size++] = a_version.c_str();
  sources[size++] = "\n";
  if (!a_raw_prefix.empty())
    sources[size++] = a_raw_prefix.c_str();
  if (!a_prefix_from_file.empty())
    sources[size++] = a_prefix_from_file.c_str();
  sources[size++] = a_source;

  glShaderSource(a_shader, size, sources, nullptr);
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
                               const std::string& a_version,
                               const std::string& a_raw_prefix,
                               const std::string& a_prefix_from_file,
                               GLuint& a_shader) {
  std::stringstream buff;
  std::ifstream stream(a_from);
  buff << stream.rdbuf();

  std::string string = buff.str();
  return createShaderFromSource(a_kind, string.c_str(), a_version, a_raw_prefix,
                                a_prefix_from_file, a_shader);
}

/* static */ std::unique_ptr<Program> Program::fromShaders(
    const ShaderSet& a_shaderSet) {
  GLuint vertexShaderId, fragmentShaderId;
  GLuint potentialOptionalShaderId;
  Optional<GLuint> geometryShaderId, tessControlShaderId,
      tessEvaluationShaderId;

  // We need at least one of these.
  if (a_shaderSet.m_vertex.empty() || a_shaderSet.m_fragment.empty())
    return nullptr;

  std::string raw_prefix = a_shaderSet.m_raw_prefix;
  std::string prefix;
  std::stringstream prefixBuff;
  if (!a_shaderSet.m_commonHeader.empty()) {
    std::ifstream stream(a_shaderSet.m_commonHeader);
    prefixBuff << stream.rdbuf();
    prefix = prefixBuff.str();
  }

  // We add a few defines to allow knowing which kind of pipeline we're using.
  if (!a_shaderSet.m_geometry.empty())
    raw_prefix += "#define HAS_GEOMETRY_SHADER\n";

  if (!a_shaderSet.m_tessellation_control.empty())
    raw_prefix += "#define HAS_TESS_CONTROL_SHADER\n";

  if (!a_shaderSet.m_tessellation_evaluation.empty())
    raw_prefix += "#define HAS_TESS_EVAL_SHADER\n";

  if (!createShaderOfKind(ShaderKind::Vertex, a_shaderSet.m_vertex,
                          a_shaderSet.m_version, raw_prefix, prefix,
                          vertexShaderId)) {
    ERROR("Shader compilation failed: %s", a_shaderSet.m_vertex.c_str());
    return nullptr;
  }

  if (!createShaderOfKind(ShaderKind::Fragment, a_shaderSet.m_fragment,
                          a_shaderSet.m_version, raw_prefix, prefix,
                          fragmentShaderId)) {
    ERROR("Shader compilation failed: %s", a_shaderSet.m_fragment.c_str());
    return nullptr;
  }

#define OPTIONAL_SHADER(kind_, member_, var_)                                  \
  do {                                                                         \
    if (!a_shaderSet.member_.empty()) {                                        \
      if (!createShaderOfKind(ShaderKind::kind_, a_shaderSet.member_,          \
                              a_shaderSet.m_version, raw_prefix, prefix,       \
                              potentialOptionalShaderId)) {                    \
        ERROR(#kind_ " shader compilation failed: %s",                         \
              a_shaderSet.member_.c_str());                                    \
        return nullptr;                                                        \
      }                                                                        \
      var_.set(potentialOptionalShaderId);                                     \
    }                                                                          \
  } while (0)

  OPTIONAL_SHADER(Geometry, m_geometry, geometryShaderId);
  OPTIONAL_SHADER(TessControl, m_tessellation_control, tessControlShaderId);
  OPTIONAL_SHADER(TessEvaluation, m_tessellation_evaluation,
                  tessEvaluationShaderId);

  GLuint id = glCreateProgram();
  LOG("Creating program: %u", id);

  AutoGLErrorChecker checker;

  LOG(" * vertex: %u", vertexShaderId);
  glAttachShader(id, vertexShaderId);

  if (tessControlShaderId) {
    LOG(" * tess control: %u", *tessControlShaderId);
    glAttachShader(id, *tessControlShaderId);
  }

  if (tessEvaluationShaderId) {
    LOG(" * tess eval: %u", *tessEvaluationShaderId);
    glAttachShader(id, *tessEvaluationShaderId);
  }

  if (geometryShaderId) {
    LOG(" * geometry: %u", *geometryShaderId);
    glAttachShader(id, *geometryShaderId);
  }

  LOG(" * fragment: %u", fragmentShaderId);
  glAttachShader(id, fragmentShaderId);

  glLinkProgram(id);

  glValidateProgram(id);

  GLint linkSuccess;
  glGetProgramiv(id, GL_LINK_STATUS, &linkSuccess);

  GLint validateSuccess;
  glGetProgramiv(id, GL_VALIDATE_STATUS, &validateSuccess);

  LOG("Program status: link: %d, validate: %d", linkSuccess, validateSuccess);

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
    if (geometryShaderId)
      glDeleteShader(*geometryShaderId);
    if (tessControlShaderId)
      glDeleteShader(*tessControlShaderId);
    if (tessEvaluationShaderId)
      glDeleteShader(*tessEvaluationShaderId);
    glDeleteProgram(id);
    return nullptr;
  }

  // NB: Not using make_unique because constructor is public.
  return std::unique_ptr<Program>(new Program(
      id, vertexShaderId, fragmentShaderId, std::move(geometryShaderId),
      std::move(tessControlShaderId), std::move(tessEvaluationShaderId)));
}
