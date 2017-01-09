#include <vector>

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "base/Logging.h"
#include "base/gl.h"
#include "tools/ArrayView.h"

#include "geometry/Node.h"
#include "geometry/Mesh.h"
#include "geometry/Vertex.h"
#include "geometry/DrawContext.h"

#include "tools/Path.h"
#include <SFML/Graphics.hpp>

void Node::draw(DrawContext& context) const {
  if (m_children.empty())
    return;

  context.push(*this);
  for (auto& child : m_children)
    child->draw(context);
  context.pop();
}

static std::unique_ptr<Node> meshFromAi(const Path& basePath,
                                        const aiScene& scene,
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

    // NB: The uv texture coordinates are loaded afterwards.
    vertices.push_back(vertex);
  }

  // We only do diffuse textures for now.
  const aiMaterial& ai_material = *scene.mMaterials[mesh.mMaterialIndex];
  Material material;

  aiColor4D color;
  if (!ai_material.Get(AI_MATKEY_COLOR_DIFFUSE, color))
    material.m_diffuse = glm::vec4(color.r, color.g, color.b, color.a);

  // Assimp instead of returning a non-AI_SUCCESS value, sets the color to
  // opaque black, which as you might expect, makes things... not great.
  //
  // Actually, I think none of the models I've loaded so far hit the success
  // path, but anyway, worth keeping here as long as later we set the diffuse
  // color appropriately.
  if (!ai_material.Get(AI_MATKEY_COLOR_SPECULAR, color) &&
      color != aiColor4D(0, 0, 0, 1.0))
    material.m_specular = glm::vec4(color.r, color.g, color.b, color.a);

  if (!ai_material.Get(AI_MATKEY_COLOR_AMBIENT, color) &&
      color != aiColor4D(0, 0, 0, 1.0))
    material.m_ambient = glm::vec4(color.r, color.g, color.b, color.a);
  else
    material.m_ambient = material.m_diffuse;

  LOG("ambient: %f %f %f %f",
      material.m_ambient[0],
      material.m_ambient[1],
      material.m_ambient[2],
      material.m_ambient[3])

  if (!ai_material.Get(AI_MATKEY_COLOR_EMISSIVE, color))
    material.m_emissive = glm::vec4(color.r, color.g, color.b, color.a);

  float floatVal;
  if (!ai_material.Get(AI_MATKEY_SHININESS, floatVal))
    material.m_shininess = floatVal;

  if (!ai_material.Get(AI_MATKEY_SHININESS_STRENGTH, floatVal))
    material.m_shininess_percent = floatVal;

  LOG("Shininess: %f, strength: %f",
      material.m_shininess,
      material.m_shininess_percent);

  // TODO: This can load a bunch of textures from the same file, build a cache,
  // and refcount, and all that stuff.
  Optional<GLuint> texture;

  uint32_t count = ai_material.GetTextureCount(aiTextureType_DIFFUSE);
  if (count) {
    LOG("Loading 1/%u textures, TODO", count);
    aiString path;
    // TODO: Right now only load one, be better at this!
    for (uint32_t i = 0; i < 1; ++i) {
      AutoGLErrorChecker checker;
      aiTextureMapping mapping;
      aiReturn ret =
          ai_material.GetTexture(aiTextureType_DIFFUSE, i, &path, &mapping);
      assert(ret == AI_SUCCESS);

      if (mapping != aiTextureMapping_UV) {
        LOG("Texture mapping mode for %s is not uv, skipping", path.C_Str());
        continue;
      }

      assert(mesh.mTextureCoords[0]);
      for (uint32_t i = 0; i < vertices.size(); ++i) {
        vertices[i].m_uv =
            glm::vec2(mesh.mTextureCoords[0][i].x, mesh.mTextureCoords[0][i].y);
      }

      Path texturePath(basePath);
      texturePath.push(path.C_Str());

      LOG(" - %u: %s", i, texturePath.c_str());

      sf::Image textureImporter;
      if (!textureImporter.loadFromFile(texturePath.as_str())) {
        WARN("Loading texture failed: %s", texturePath.c_str());
        continue;
      }

      auto size = textureImporter.getSize();
      texture.set(0);
      glGenTextures(1, &*texture);
      glBindTexture(GL_TEXTURE_2D, *texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, textureImporter.getPixelsPtr());

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
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

  return std::make_unique<Mesh>(std::move(vertices), std::move(indices),
                                material, std::move(texture));
}

/* static */ std::unique_ptr<Node> Node::fromFile(const char* a_modelPath) {
  Assimp::Importer importer;
  // NB: We flip the UV coordinates here instead of somewhere else.
  //
  // We also triangulate faces here if needed, because I'm too lazy to re-export
  // stuff.
  const aiScene* scene =
      importer.ReadFile(a_modelPath, aiProcess_FlipUVs | aiProcess_Triangulate);

  if (!scene) {
    LOG("Assimp failed to read the scene");
    return nullptr;
  }

  if (!scene->HasMeshes()) {
    WARN("Scene with no meshes");
    return nullptr;
  }

  LOG("Materials: %d", scene->mNumMaterials);

  Path basePath(a_modelPath);
  basePath.pop();

  // Not worth to add an extra layer of indirection in the simple case.
  if (scene->mNumMeshes == 1)
    return meshFromAi(basePath, *scene, *scene->mMeshes[0]);

  auto ret = std::make_unique<Node>();

  for (size_t i = 0; i < scene->mNumMeshes; ++i) {
    assert(scene->mMeshes[i]);
    ret->addChild(meshFromAi(basePath, *scene, *scene->mMeshes[i]));
  }

  return ret;
}
