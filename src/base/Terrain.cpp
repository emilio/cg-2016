#include "base/Terrain.h"
#include "base/Logging.h"

#include <SFML/Graphics.hpp>

static inline float mapToHeight(uint8_t byte) {
  // Map byte from 255 to 0, to +0.25/-0.25
  float portion = ((float)byte) / 255;
  return (portion - 0.5) / 3;
}

Terrain::Terrain(std::vector<Vertex>&& vertices,
                 std::vector<GLuint>&& indices,
                 Optional<GLuint> texture)
  : Mesh(std::move(vertices), std::move(indices), std::move(texture)) {}

/* static */ std::unique_ptr<Terrain> Terrain::create() {
  sf::Image heightMap;
  sf::Image texture;

  // FIXME: Stop hardcoding, the usual stuff.
  if (!heightMap.loadFromFile("res/terrain/heightmap.png")) {
    ERROR("Error loading heightmap");
    return nullptr;
  }

  // if (!texture.loadFromFile("res/terrain/texture.png")) {
  //   ERROR("Error loading terrain texture");
  //   return nullptr;
  // }

  auto size = heightMap.getSize();
  auto textureSize = texture.getSize();
  std::vector<Vertex> vertices;
  vertices.reserve(size.x * size.y);

  LOG("Loading terrain from file: %ux%u pixels", size.x, size.y);

  for (uint32_t x = 0; x < size.x; ++x) {
    for (uint32_t y = 0; y < size.y; ++y) {
      float posX = ((float)x) / size.x;
      float posY = ((float)y) / size.y;

      auto pixel = heightMap.getPixel(x, y);
      float height = mapToHeight(pixel.r);
      // LOG("%u %u %u %u", pixel.r, pixel.g, pixel.b, pixel.a);

      Vertex vertex;
      vertex.m_position = glm::vec3(posX - 0.5, height, posY - 0.5);
      // FIXME: This is a smell, we're interpolating manually here when we
      // probably shouldn't.
      vertex.m_uv = glm::vec2(posX * textureSize.x, posY * textureSize.y);
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
      indices.push_back(i + size.x + 1);
      indices.push_back(i + 1);

      indices.push_back(i);
      indices.push_back(i + size.x);
      indices.push_back(i + size.x + 1);
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

  // FIXME: Texture!
  auto terrain = std::unique_ptr<Terrain>(
      new Terrain(std::move(vertices), std::move(indices), None));

  // TODO: Add collision detection boxes, shouldn't be hard.
  terrain->scale(TERRAIN_DIMENSIONS);

  return terrain;
}
