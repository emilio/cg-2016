#include "geometry/Mesh.h"

Mesh::Mesh(std::vector<Vertex>&& a_vertices,
           std::vector<GLuint>&& a_indices,
           Optional<GLuint>&& a_texture)
  : m_vertices(std::move(a_vertices))
  , m_indices(std::move(a_indices))
  , m_texture(std::move(a_texture))
  , m_vao(UNINITIALIZED)
  , m_vbo(UNINITIALIZED)
  , m_ebo(UNINITIALIZED) {
#ifdef DEBUG
  for (auto index : m_indices) {
    assert(index < m_vertices.size() || !"Index out of bounds");
  }
#endif

  AutoGLErrorChecker checker;
  glGenVertexArrays(1, &m_vao);

  glBindVertexArray(m_vao);

  glGenBuffers(1, &m_vbo);
  glGenBuffers(1, &m_ebo);

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_vertices.size(),
               m_vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * m_indices.size(),
               m_indices.data(), GL_STATIC_DRAW);

#define INT_TO_GLVOID(i) ((GLvoid*)i)

  // Vertex positions.
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        INT_TO_GLVOID(offsetof(Vertex, m_position)));

  // Vertex normals
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        INT_TO_GLVOID(offsetof(Vertex, m_normal)));

  // Vertex uv coordinates.
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        INT_TO_GLVOID(offsetof(Vertex, m_uv)));
  glBindVertexArray(0);
}

Mesh::~Mesh() {
  if (m_vao == UNINITIALIZED) {
    assert(m_vbo == UNINITIALIZED);
    assert(m_ebo == UNINITIALIZED);
    return;
  }

  AutoGLErrorChecker checker;
  glDeleteVertexArrays(1, &m_vao);
  glDeleteBuffers(1, &m_vbo);
  glDeleteBuffers(1, &m_ebo);
}

void Mesh::draw() {
  AutoGLErrorChecker checker;
  assert(glIsVertexArray(m_vao));

  LOG("Draw: %d, %zu", m_vao, m_indices.size());

  glBindVertexArray(m_vao);
  glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);

  glBindVertexArray(0);
  Node::draw();
}
