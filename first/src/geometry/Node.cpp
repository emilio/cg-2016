#include <vector>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"

#include "base/Logging.h"
#include "base/gl.h"

#include "geometry/Node.h"
#include "geometry/Mesh.h"
#include "geometry/Vertex.h"

static std::unique_ptr<Node> meshFromAi(const aiMesh& mesh) {
  assert(mesh.HasFaces());
  assert(mesh.HasPositions());

  std::vector<Vertex> vertices;
  vertices.reserve(mesh.mNumVertices);

  for (size_t i = 0; i < mesh.mNumVertices; ++i) {
    Vertex vertex;
    vertex.m_position = glm::vec3(mesh.mVertices[i].x, mesh.mVertices[i].y,
                                  mesh.mVertices[i].z);

    if (mesh.HasNormals()) {
      vertex.m_normal = glm::vec3(mesh.mNormals[i].x, mesh.mNormals[i].y,
                                  mesh.mNormals[i].z);
    }

    // TODO(emilio): uv coords, textures, ...
    vertices.push_back(vertex);
  }

  // We assume these are triangulated.
  std::vector<GLuint> indices;
  indices.reserve(mesh.mNumFaces * 3);
  for (size_t i = 0; i < mesh.mNumFaces; ++i) {
    const aiFace& face = mesh.mFaces[i];
    assert(face.mNumIndices == 3);
    indices.push_back(face.mIndices[0]);
    indices.push_back(face.mIndices[1]);
    indices.push_back(face.mIndices[2]);
  }

  // NB: The importer destructor frees the stuff.
  if (!mesh.HasNormals()) {
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

/* static */ std::unique_ptr<Node> Node::fromFile(const char* a_modelPath) {
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(a_modelPath, 0);

  if (!scene) {
    LOG("Assimp failed to read the scene");
    return nullptr;
  }

  if (!scene->HasMeshes()) {
    WARN("Scene with no meshes");
    return nullptr;
  }

  LOG("Materials: %d", scene->mNumMaterials);
  assert(scene->mNumMaterials == 1);

  // Not worth to add an extra layer of indirection in the simple case.
  if (scene->mNumMeshes == 1)
    return meshFromAi(*scene->mMeshes[0]);

  auto ret = std::make_unique<Node>();

  for (size_t i = 0; i < scene->mNumMeshes; ++i) {
    assert(scene->mMeshes[i]);
    ret->addChild(meshFromAi(*scene->mMeshes[i]));
  }

  return ret;
}
