#include "base/Terrain.h"
#include "base/Logging.h"
#include "tools/Optional.h"

#include <SFML/Graphics.hpp>

static inline float mapToHeight(uint8_t byte) {
  // Map byte from 255 to 0, to +0.25/-0.25
  float portion = ((float)byte) / 255;
  return (portion - 0.5) / 3;
}

Terrain::Terrain(std::vector<Vertex>&& vertices,
                 std::vector<GLuint>&& indices,
                 Material material,
                 Optional<GLuint> texture)
  : Mesh(
        std::move(vertices), std::move(indices), material, std::move(texture)) {
}

/* static */ std::unique_ptr<Terrain> Terrain::create() {
  sf::Image heightMap;
  sf::Image textureImporter;

  // FIXME: Stop hardcoding, the usual stuff.
  if (!heightMap.loadFromFile("res/terrain/heightmap.png")) {
    // if (!heightMap.loadFromFile("res/terrain/maribor.png")) {
    ERROR("Error loading heightmap");
    return nullptr;
  }

  if (!textureImporter.loadFromFile("res/terrain/cover.png")) {
    ERROR("Error loading terrain texture");
    return nullptr;
  }

  auto textureSize = textureImporter.getSize();
  LOG("Loading terrain cover of %dx%d", textureSize.x, textureSize.y);

  GLuint texture;
  {
    AutoGLErrorChecker checker;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSize.x, textureSize.y, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, textureImporter.getPixelsPtr());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  auto size = heightMap.getSize();
  std::vector<Vertex> vertices;
  vertices.reserve(size.x * size.y);

  LOG("Loading terrain from file: %ux%u pixels", size.x, size.y);

  for (uint32_t x = 0; x < size.x; ++x) {
    for (uint32_t y = 0; y < size.y; ++y) {
      float posX = ((float)x) / size.x;
      float posY = ((float)y) / size.y;

      auto pixel = heightMap.getPixel(x, y);
      float height = mapToHeight(pixel.r);

      Vertex vertex;
      vertex.m_position = glm::vec3(posX - 0.5, height, posY - 0.5);
      vertex.m_uv = glm::vec2(posY, posX);
      vertices.push_back(vertex);
    }
  }

  // Two triangles per quad.
  std::vector<GLuint> indices;
  uint32_t triangleCount = (size.x - 1) * (size.y - 1) * 2;
  indices.reserve(triangleCount * 3);

  // Now we generate triangles, two per quad, using something similar to:
  // http://www.3dgep.com/multi-textured-terrain-in-opengl/
  for (uint32_t x = 0; x < size.x - 1; ++x) {
    for (uint32_t y = 0; y < size.y - 1; ++y) {
      uint32_t i = (y * size.x) + x;
      indices.push_back(i);
      indices.push_back(i + 1);
      indices.push_back(i + size.x + 1);

      indices.push_back(i);
      indices.push_back(i + size.x + 1);
      indices.push_back(i + size.x);
    }
  }

  // FIXME: Don't duplicate this code with the importer!
  for (size_t i = 0; i < indices.size(); i += 3) {
    GLuint i_1 = indices[i];
    GLuint i_2 = indices[i + 1];
    GLuint i_3 = indices[i + 2];
    vertices[i_1].m_normal = vertices[i_2].m_normal = vertices[i_3].m_normal =
        glm::normalize(
            glm::cross(vertices[i_2].m_position - vertices[i_1].m_position,
                       vertices[i_3].m_position - vertices[i_1].m_position));
  }

  // FIXME: Nor this!
  Material mat;
  mat.m_diffuse = glm::vec4(140.0, 96.0, 43.0, 255.0f) / glm::vec4(255.0f);
  mat.m_ambient = glm::vec4(1.0, 1.0, 1.0, 1.0);
  mat.m_shininess_percent = 0.1;

  auto terrain = std::unique_ptr<Terrain>(
      new Terrain(std::move(vertices), std::move(indices), mat, Some(texture)));

  // TODO: Add collision detection boxes, shouldn't be hard.
  terrain->scale(TERRAIN_DIMENSIONS);

  return terrain;
}
