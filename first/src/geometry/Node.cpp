#include <vector>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"

#include "base/Logging.h"
#include "base/gl.h"

#include "geometry/Node.h"
#include "geometry/Mesh.h"
#include "geometry/Vertex.h"

/* static */ std::unique_ptr<Node> Node::fromFile(const char* a_modelPath) {
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(a_modelPath, 0);

  if (!scene) {
    LOG("Assimp failed to read the scene");
    return nullptr;
  }

  if (!scene->HasMeshes()) {
    LOG("Scene with no meshes");
    return nullptr;
  }

  // We only import the first one, dangit. If we need to import more we need to
  // change this to return a node. Which isn't that bad, but at that point
  // you're already duplicating a lot of work.
  if (scene->mNumMeshes > 1)
    WARN("Scene has %d meshes, using only the first", scene->mNumMeshes);

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

  return std::make_unique<Mesh>(std::move(vertices), std::move(indices), None);
}
