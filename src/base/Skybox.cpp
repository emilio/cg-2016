#include "base/Skybox.h"
#include "base/gl.h"
#include "base/ErrorChecker.h"

#include "glm/gtc/type_ptr.hpp"

#include <vector>
#include <SFML/Graphics.hpp>

// Just a cube's set of 36 vertices.
constexpr GLfloat gSkyboxVertices[] = {
    -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
    -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

    1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

// FIXME: Don't hardcode this?
const char* const gSkyboxFaces[] = {
    "res/skybox/faces/right.jpg", "res/skybox/faces/left.jpg",
    "res/skybox/faces/top.jpg",   "res/skybox/faces/bottom.jpg",
    "res/skybox/faces/back.jpg",  "res/skybox/faces/front.jpg",
};

Skybox::Skybox(std::unique_ptr<Program> a_program)
  : m_program(std::move(a_program)) {
  AutoGLErrorChecker checker;
  assert(m_program);

  glGenTextures(1, &m_cubeMapTexture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeMapTexture);

  uint32_t i = 0;
  for (auto& faceFilename : gSkyboxFaces) {
    sf::Image faceImage;

    if (!faceImage.loadFromFile(faceFilename)) {
      ERROR("Couldn't load face: %s", faceFilename);
      assert(!"Couldn't load face");
    }

    auto size = faceImage.getSize();
    const void* pixels = faceImage.getPixelsPtr();

    LOG("Loading %s: %u %u", faceFilename, size.x, size.y);

    auto target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i++;
    glTexImage2D(target, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixels);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);

  glGenBuffers(1, &m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(gSkyboxVertices), gSkyboxVertices,
               GL_STATIC_DRAW);

  // Vertex positions.
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);

  // Set up uniform.
  m_uniforms.uViewProjection =
      glGetUniformLocation(m_program->id(), "uViewProjection");
  m_uniforms.uSkybox = glGetUniformLocation(m_program->id(), "uSkybox");
  glBindVertexArray(0);
}

Skybox::~Skybox() {
  glDeleteVertexArrays(1, &m_vao);
  glDeleteBuffers(1, &m_vbo);
  glDeleteTextures(1, &m_cubeMapTexture);
}

void Skybox::draw(const glm::mat4& a_viewProjection) const {
  AutoGLErrorChecker checker;
  assert(glIsVertexArray(m_vao));

  m_program->use();

  glCullFace(GL_BACK);
  glDepthMask(GL_FALSE);
  glBindVertexArray(m_vao);

  glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeMapTexture);

  glUniformMatrix4fv(m_uniforms.uViewProjection, 1, GL_FALSE,
                     glm::value_ptr(a_viewProjection));

  static_assert((sizeof(gSkyboxVertices) / sizeof(GLfloat)) == 36 * 3, "wat");
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
  glUseProgram(0);
  glDepthMask(GL_TRUE);
  glEnable(GL_CULL_FACE);
}

std::unique_ptr<Skybox> Skybox::create() {
  ShaderSet shaders("res/skybox/common.glsl", "res/skybox/vertex.glsl",
                    "res/skybox/fragment.glsl");
  std::unique_ptr<Program> program = Program::fromShaders(std::move(shaders));

  if (!program)
    return nullptr;

  return std::unique_ptr<Skybox>(new Skybox(std::move(program)));
}
