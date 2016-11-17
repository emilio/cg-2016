#include "geometry/Mesh.h"
#include "geometry/DrawContext.h"

Mesh::Mesh(std::vector<Vertex>&& a_vertices,
           std::vector<GLuint>&& a_indices,
           Material a_material,
           Optional<GLuint>&& a_texture)
  : m_vertices(std::move(a_vertices))
  , m_indices(std::move(a_indices))
  , m_material(a_material)
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
  AutoGLErrorChecker checker;
  if (m_texture.isSome())
    glDeleteTextures(1, &*m_texture);

  if (m_vao == UNINITIALIZED) {
    assert(m_vbo == UNINITIALIZED);
    assert(m_ebo == UNINITIALIZED);
    return;
  }

  glDeleteVertexArrays(1, &m_vao);
  glDeleteBuffers(1, &m_vbo);
  glDeleteBuffers(1, &m_ebo);
}

void Mesh::draw(DrawContext& context) const {
  AutoGLErrorChecker checker;
  assert(glIsVertexArray(m_vao));

  // LOG("Draw: %d, %zu", m_vao, m_indices.size());

  // FIXME(emilio): This duplicates code with what node does, we should probably
  // add a beginDraw/endDraw functions.
  //
  // This doesn't matter in practice because the way it works right now meshes
  // don't have children, so this doesn't incur in more gl calls.
  //
  // Though that means that probably the class hierarchy needs to be redesigned?
  // Who knows :-)
  context.push(*this);

  glUniform4fv(context.uniforms().m_material.m_diffuse, 1,
               glm::value_ptr(m_material.m_diffuse));
  glUniform4fv(context.uniforms().m_material.m_ambient, 1,
               glm::value_ptr(m_material.m_ambient));
  glUniform4fv(context.uniforms().m_material.m_emissive, 1,
               glm::value_ptr(m_material.m_emissive));
  glUniform4fv(context.uniforms().m_material.m_specular, 1,
               glm::value_ptr(m_material.m_specular));
  glUniform1f(context.uniforms().m_material.m_shininess,
              m_material.m_shininess);
  glUniform1f(context.uniforms().m_material.m_shininess_percent,
              m_material.m_shininess_percent);
  glUniform1i(context.uniforms().m_usesTexture, m_texture.isSome());

  if (m_texture) {
    // TODO: We should be able to avoid this glUniform1i call and do the
    // glActiveTexture call just once, but anyway.
    glUniform1i(context.uniforms().m_texture, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, *m_texture);
  }

  glBindVertexArray(m_vao);
  if (context.program().tessControlShader()) {
    glPatchParameteri(GL_PATCH_VERTICES, 3);
    glDrawElements(GL_PATCHES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
  } else {
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
  }
  glBindVertexArray(0);

  context.pop();

  Node::draw(context);
}
