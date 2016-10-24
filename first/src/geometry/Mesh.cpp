
#include "geometry/Mesh.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"

#include <memory>
#include <fstream>

Mesh::Mesh(std::vector<Vertex>&& a_vertices, std::vector<GLuint>&& a_indices)
  : m_vertices(a_vertices)
  , m_indices(a_indices)
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

/* static */ std::unique_ptr<Mesh> Mesh::fromFile(const char* a_modelPath) {
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(a_modelPath, 0);

  if (!scene)
    return nullptr;

  if (!scene->HasMeshes())
    return nullptr;

  // We only import the first one, dangit. If we need to import more we need to
  // change this to return a node. Which isn't that bad, but at that point
  // you're already duplicating a lot of work.
  if (scene->mNumMeshes > 1)
    return nullptr;

  const aiMesh* mesh = scene->mMeshes[0];

  assert(mesh->HasFaces());
  assert(mesh->HasPositions());

  std::vector<Vertex> vertices;
  vertices.reserve(mesh->mNumVertices);

  for (size_t i = 0; i < mesh->mNumVertices; ++i) {
    Vertex vertex;
    vertex.m_position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y,
                                  mesh->mVertices[i].z);

    if (mesh->HasNormals()) {
      vertex.m_normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y,
                                  mesh->mNormals[i].z);
    }

    // FIXME: Texture coords, I guess.

    vertices.push_back(vertex);
  }

  // We assume these are triangulated.
  std::vector<GLuint> indices;
  indices.reserve(mesh->mNumFaces * 3);
  for (size_t i = 0; i < mesh->mNumFaces; ++i) {
    const aiFace& face = mesh->mFaces[i];
    assert(face.mNumIndices == 3);
    indices.push_back(face.mIndices[0]);
    indices.push_back(face.mIndices[1]);
    indices.push_back(face.mIndices[2]);
  }

  // NB: The importer destructor frees the stuff.
  if (!mesh->HasNormals()) {
    for (size_t i = 0; i < indices.size(); i += 3) {
      GLuint i_1 = indices[i];
      GLuint i_2 = indices[i + 1];
      GLuint i_3 = indices[i + 2];
      vertices[i_1].m_normal = vertices[i_2].m_normal = vertices[i_3].m_normal =
          glm::normalize(
              glm::cross(vertices[i_2].m_position - vertices[i_1].m_position,
                         vertices[i_3].m_position - vertices[i_1].m_position));
    }
  }

  return std::make_unique<Mesh>(std::move(vertices), std::move(indices));
}

void Mesh::draw() {
  AutoGLErrorChecker checker;
  assert(glIsVertexArray(m_vao));

  LOG("Draw: %d, %zu", m_vao, m_indices.size());

  glBindVertexArray(m_vao);
  glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);

  glBindVertexArray(0);
}
