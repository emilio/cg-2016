
#include "geometry/Mesh.h"
#include "loader/BasicObjLoader.h"
#include <memory>
#include <fstream>

/* static */ std::unique_ptr<Mesh>
Mesh::fromFile(const char* a_modelPath)
{
  std::ifstream in(a_modelPath);

  if (!in) {
    return nullptr;
  }

  std::vector<Vertex> a_vertices;
  std::vector<GLuint> a_indices;
  bool success = loader::BasicObjLoader::Load(in, a_vertices, a_indices);
  if (!success) {
    return nullptr;
  }

  return std::make_unique<Mesh>(std::move(a_vertices), std::move(a_indices));
}

void Mesh::draw() {
  AutoGLErrorChecker checker;
  assert(glIsVertexArray(m_vao));

  LOG("Draw: %d, %zu", m_vao, m_indices.size());

  glBindVertexArray(m_vao);
  glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);

  glBindVertexArray(0);
}
