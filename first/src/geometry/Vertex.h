#pragma once

#include "glm/glm.hpp"

struct Vertex {
  glm::vec3 m_position;
  glm::vec3 m_normal;
  glm::vec2 m_uv;
};

static_assert(sizeof(Vertex) == 8 * sizeof(float), "This should hold");
