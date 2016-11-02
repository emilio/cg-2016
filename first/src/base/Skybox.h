#pragma once

#include "base/Program.h"
#include "geometry/Mesh.h"

const size_t SKYBOX_WIDTH = 2000;
const size_t SKYBOX_HEIGHT = 2000;
const size_t SKYBOX_DEPTH = 2000;

class Skybox final {
  Skybox(std::unique_ptr<Program> m_program);

  // There's only one Skybox in here, so it's completely fine for it to own the
  // program, and this allows us to bind/unbind it when drawing without too much
  // problem, and keep the logic around.
  std::unique_ptr<Program> m_program;

  GLuint m_cubeMapTexture;
  GLuint m_vbo;
  GLuint m_vao;

  struct {
    GLint uViewProjection;
    GLint uSkybox;
  } m_uniforms;

public:
  ~Skybox();

  void draw(const glm::mat4& a_viewProjection) const;

  // Could be useful if I decide to do reflection of stuff.
  GLuint texture() const { return m_cubeMapTexture; }

  static std::unique_ptr<Skybox> create();
};
