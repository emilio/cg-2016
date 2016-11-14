#pragma once

#include "glm/glm.hpp"
#include "base/gl.h"

struct Material {
  glm::vec4 m_diffuse = glm::vec4(0.5, 0.5, 0.5, 1.0);
  glm::vec4 m_specular = glm::vec4(1.0, 1.0, 1.0, 1.0);
  glm::vec4 m_ambient = glm::vec4(1.0, 1.0, 1.0, 1.0);
  glm::vec4 m_emissive = glm::vec4(1.0, 1.0, 1.0, 1.0);
  float m_shininess = 2;
  float m_shininess_percent = 0.5;
};

struct MaterialUniforms {
  GLint m_diffuse;
  GLint m_specular;
  GLint m_ambient;
  GLint m_emissive;
  GLint m_shininess;
  GLint m_shininess_percent;
};
