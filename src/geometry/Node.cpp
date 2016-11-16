#include <vector>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"

#include "base/Logging.h"
#include "base/gl.h"
#include "tools/ArrayView.h"

#include "geometry/Node.h"
#include "geometry/Mesh.h"
#include "geometry/Vertex.h"
#include "geometry/DrawContext.h"

void Node::draw(DrawContext& context) const {
  if (m_children.empty())
    return;

  context.push(*this);
  for (auto& child : m_children)
    child->draw(context);
  context.pop();
}

static std::unique_ptr<Node> meshFromAi(const aiScene& scene,
                                        const aiMesh& mesh) {
  assert(mesh.HasFaces());
  assert(mesh.HasPositions());

  std::vector<Vertex> vertices;
  vertices.reserve(mesh.mNumVertices);

  for (size_t i = 0; i < mesh.mNumVertices; ++i) {
    Vertex vertex;
    vertex.m_position = glm::vec3(mesh.mVertices[i].x, mesh.mVertices[i].y,
                                  mesh.mVertices[i].z);

    if (mesh.HasNormals()) {
      vertex.m_normal =
          glm::vec3(mesh.mNormals[i].x, mesh.mNormals[i].y, mesh.mNormals[i].z);
    }

    // TODO(emilio): uv coords, textures, ...
    vertices.push_back(vertex);
  }

  // We only do diffuse textures for now.
  const aiMaterial& ai_material = *scene.mMaterials[mesh.mMaterialIndex];
  Material material;

  aiColor4D color;
  if (!ai_material.Get(AI_MATKEY_COLOR_DIFFUSE, color))
    material.m_diffuse = glm::vec4(color.r, color.g, color.b, color.a);

  if (!ai_material.Get(AI_MATKEY_COLOR_SPECULAR, color))
    material.m_specular = glm::vec4(color.r, color.g, color.b, color.a);
  else
    material.m_specular = material.m_diffuse;

  if (!ai_material.Get(AI_MATKEY_COLOR_AMBIENT, color))
    material.m_ambient = glm::vec4(color.r, color.g, color.b, color.a);
  else
    material.m_ambient = material.m_diffuse;

  if (!ai_material.Get(AI_MATKEY_COLOR_EMISSIVE, color))
    material.m_emissive = glm::vec4(color.r, color.g, color.b, color.a);

  float floatVal;
  if (!ai_material.Get(AI_MATKEY_SHININESS, floatVal))
    material.m_shininess = floatVal;

  if (!ai_material.Get(AI_MATKEY_SHININESS_STRENGTH, floatVal))
    material.m_shininess_percent = floatVal;

  uint32_t count = ai_material.GetTextureCount(aiTextureType_DIFFUSE);
  if (count)
    LOG("Missing textures, TODO");

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

  return std::make_unique<Mesh>(std::move(vertices), std::move(indices),
                                material, None);
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

  // Not worth to add an extra layer of indirection in the simple case.
  if (scene->mNumMeshes == 1)
    return meshFromAi(*scene, *scene->mMeshes[0]);

  auto ret = std::make_unique<Node>();

  for (size_t i = 0; i < scene->mNumMeshes; ++i) {
    assert(scene->mMeshes[i]);
    ret->addChild(meshFromAi(*scene, *scene->mMeshes[i]));
  }

  return ret;
}
