#pragma once

#include "base/gl.h"
#include "base/Logging.h"
#include "base/ErrorChecker.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "geometry/Node.h"
#include "geometry/Vertex.h"

#include "tools/Optional.h"

#include <vector>
#include <memory>

#define UNINITIALIZED ((GLuint)-1)

class Mesh final : public Node {
  std::vector<Vertex> m_vertices;
  std::vector<GLuint> m_indices;

  // The texture we're using.
  Optional<GLuint> m_texture;

  // The vertex array object.
  GLuint m_vao;

  // The buffer object for the vertices.
  GLuint m_vbo;

  // The buffer object for the indices.
  GLuint m_ebo;

public:
  // No copy semantics, just move.
  Mesh(const Mesh& aOther) = delete;

  Mesh(Mesh&& aOther) {
    m_vertices.swap(aOther.m_vertices);
    m_indices.swap(aOther.m_indices);

    m_vao = aOther.m_vao;
    m_vbo = aOther.m_vbo;
    m_ebo = aOther.m_ebo;
    m_transform = aOther.m_transform;

    m_texture = std::move(aOther.m_texture);

    aOther.m_vao = aOther.m_vbo = aOther.m_ebo = UNINITIALIZED;
  }

  virtual ~Mesh();

  Mesh(std::vector<Vertex>&& a_vertices,
       std::vector<GLuint>&& a_indices,
       Optional<GLuint>&& a_texture);

  virtual void draw(DrawContext&) const override;
};
