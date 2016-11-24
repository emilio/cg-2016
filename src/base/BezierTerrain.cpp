#include "BezierTerrain.h"
#include "DynTerrain.h"
#include "Terrain.h"

#include "base/gl.h"
#include "base/Logging.h"
#include "base/Program.h"
#include "base/ErrorChecker.h"
#include "glm/gtc/type_ptr.hpp"


#include <SFML/Graphics.hpp>

static inline float mapToHeight(uint8_t byte) {
  // Map byte from 255 to 0, to +0.25/-0.25
  float portion = ((float)byte) / 255;
  return (portion - 0.5) / 3;
}

// We use each nine squares' limits as a control point for a bezier surface:
//
//  +----+----+----+
//  |    |    |    |
//  |    |    |    |
//  +----+----+----+
//  |    |    |    |
//  |    |    |    |
//  +----+----+----+
//  |    |    |    |
//  |    |    |    |
//  +----+----+----+
//
template <size_t size>
static void makePlane(const sf::Image& heightMap,
                      std::vector<glm::vec3>& vertices,
                      std::vector<GLuint>& indices) {
  static_assert(size > 4 && ((size - 4) % 3) == 0);

  vertices.clear();
  vertices.reserve(size * size);

  size_t indicesSize = size / 4 * size / 4 * 16;
  indices.clear();
  indices.reserve(indicesSize);

  auto heightMapSize = heightMap.getSize();

  auto xratio = ((float)heightMapSize.x) / size;
  auto yratio = ((float)heightMapSize.y) / size;

  for (size_t x = 0; x < size; ++x) {
    for (size_t y = 0; y < size; ++y) {
      auto height = mapToHeight(heightMap.getPixel(x * xratio, y * yratio).r);
      glm::vec3 position(((float)x) / size, height, ((float)y) / size);
      vertices.push_back(position - glm::vec3(0.5, 0.0, 0.5));
    }
  }

  auto patchesPerRow = (size - 4) / 3 + 1;
  for (size_t x = 0; x < patchesPerRow; ++x) {
    for (size_t y = 0; y < patchesPerRow; ++y) {
      auto leftCorner = (y * 3) + x * size * 3;
      // For each row...
      for (size_t j = 0; j < 4; ++j) {
        indices.push_back(leftCorner + size * j);
        indices.push_back(leftCorner + size * j + 1);
        indices.push_back(leftCorner + size * j + 2);
        indices.push_back(leftCorner + size * j + 3);
      }
    }
  }
}

BezierTerrain::BezierTerrain(std::unique_ptr<Program> program,
                             GLuint texture,
                             const std::vector<glm::vec3>& vertices,
                             const std::vector<GLuint>& indices)
  : m_program(std::move(program))
  , m_coverTexture(texture)
  , m_indicesCount(indices.size())
{
  AutoGLErrorChecker checker;

  glGenVertexArrays(1, &m_vao);

  glBindVertexArray(m_vao);

  glGenBuffers(1, &m_vbo);
  glGenBuffers(1, &m_ebo);

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(),
               vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(),
               indices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

  glBindVertexArray(0);

  queryUniforms();
}

BezierTerrain::~BezierTerrain() {
  glDeleteTextures(1, &m_coverTexture);
  glDeleteVertexArrays(1, &m_vao);
  glDeleteBuffers(1, &m_vbo);
  glDeleteBuffers(1, &m_ebo);
}

void BezierTerrain::queryUniforms() {
#define QUERY(u)                                                               \
  do {                                                                         \
    m_uniforms.u = glGetUniformLocation(m_program->id(), #u);                  \
    /* assert(m_uniforms.u != -1); */                                                \
  } while (0)

  QUERY(uCameraPosition);
  QUERY(uViewProjection);
  QUERY(uModel);
  QUERY(uCover);
  QUERY(uDimension);
}

void BezierTerrain::drawTerrain(const glm::mat4& viewProjection,
                                const glm::vec3& cameraPos) const {
  m_program->use();
  glBindVertexArray(m_vao);
  glUniform3fv(m_uniforms.uCameraPosition, 1, glm::value_ptr(cameraPos));
  glUniformMatrix4fv(m_uniforms.uViewProjection, 1, GL_FALSE,
                     glm::value_ptr(viewProjection));
  glUniformMatrix4fv(m_uniforms.uModel, 1, GL_FALSE,
                     glm::value_ptr(transform()));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_coverTexture);

  // These should be constant.
  glUniform1i(m_uniforms.uCover, 0);
  glUniform1f(m_uniforms.uDimension, TERRAIN_DIMENSIONS);

  glPatchParameteri(GL_PATCH_VERTICES, 16);
  glDrawElements(GL_PATCHES, m_indicesCount, GL_UNSIGNED_INT, nullptr);

  glBindVertexArray(0);
  glUseProgram(0);
}

std::unique_ptr<BezierTerrain> BezierTerrain::create() {
  ShaderSet shaders("res/bezier-terrain/common.glsl",
                    "res/bezier-terrain/vertex.glsl",
                    "res/bezier-terrain/fragment.glsl");
  shaders.m_tessellation_control = "res/bezier-terrain/tess-control.glsl";
  shaders.m_tessellation_evaluation = "res/bezier-terrain/tess-eval.glsl";

  auto program = Program::fromShaders(shaders);
  if (!program) {
    ERROR("Failed to create quad BezierTerrain program");
    return nullptr;
  }

  sf::Image heightMap;
  if (!heightMap.loadFromFile("res/terrain/heightmap.png")) {
    ERROR("Error loading heightmap");
    return nullptr;
  }

  sf::Image cover;
  if (!cover.loadFromFile("res/terrain/cover.png")) {
    ERROR("Error loading cover");
    return nullptr;
  }

  GLuint coverTexture = DynTerrain::textureFromImage(cover, true);

  std::vector<glm::vec3> vertices;
  std::vector<GLuint> indices;
  makePlane<31>(heightMap, vertices, indices);

  auto terrain = std::unique_ptr<BezierTerrain>(
      new BezierTerrain(std::move(program), coverTexture, vertices, indices));

  terrain->scale(TERRAIN_DIMENSIONS);

  return terrain;
}
