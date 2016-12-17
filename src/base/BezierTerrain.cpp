#include "BezierTerrain.h"
#include "DynTerrain.h"
#include "Terrain.h"

#include "base/gl.h"
#include "base/Logging.h"
#include "base/Program.h"
#include "base/ErrorChecker.h"
#include "base/Scene.h"
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
  static_assert(size > 4 && ((size - 4) % 3) == 0, "Improper plane size");

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

  glGenTextures(1, &m_shadowMapTexture);

  // Create the proper depth map and attach it to our shadowmap framebuffer.
  glBindTexture(GL_TEXTURE_2D, m_shadowMapTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
               SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glGenFramebuffers(1, &m_shadowMapFB);
  glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFB);
  glReadBuffer(GL_NONE);
  glDrawBuffer(GL_NONE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMapTexture, 0);

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  queryUniforms();
}

BezierTerrain::~BezierTerrain() {
  AutoGLErrorChecker checker;
  glDeleteTextures(1, &m_coverTexture);
  glDeleteFramebuffers(1, &m_shadowMapFB);
  glDeleteTextures(1, &m_shadowMapTexture);
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
  QUERY(uShadowMap);
  QUERY(uDimension);
  QUERY(uLodEnabled);
  QUERY(uLodLevel);
  QUERY(uDrawingForShadowMap);
  QUERY(uShadowMapViewProjection);
}

void BezierTerrain::drawTerrain(const Scene& scene) const {
  assert(scene.shadowMap());
  drawTerrainInternal(scene, scene.viewProjection(), scene.cameraPosition(), false);
}

void BezierTerrain::drawTerrainInternal(const Scene& scene,
                                        const glm::mat4& viewProjection,
                                        const glm::vec3& cameraPos,
                                        bool forShadowMap) const {
  AutoGLErrorChecker checker;
  m_program->use();
  glBindVertexArray(m_vao);
  glUniform3fv(m_uniforms.uCameraPosition, 1, glm::value_ptr(cameraPos));
  glUniform1i(m_uniforms.uDrawingForShadowMap, forShadowMap);
  glUniformMatrix4fv(m_uniforms.uViewProjection, 1, GL_FALSE,
                     glm::value_ptr(viewProjection));
  glUniformMatrix4fv(m_uniforms.uModel, 1, GL_FALSE,
                     glm::value_ptr(transform()));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_coverTexture);
  if (!forShadowMap) {
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, *scene.shadowMap());
    glUniformMatrix4fv(m_uniforms.uShadowMapViewProjection, 1, GL_FALSE,
        glm::value_ptr(scene.shadowMapViewProjection()));
  }

  glUniform1i(m_uniforms.uLodEnabled, scene.dynamicTessellationEnabled());
  glUniform1f(m_uniforms.uLodLevel, scene.tessLevel());

  // These should be constant.
  glUniform1i(m_uniforms.uCover, 0);
  glUniform1i(m_uniforms.uShadowMap, 1);
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
  makePlane<139>(heightMap, vertices, indices);

  auto terrain = std::unique_ptr<BezierTerrain>(
      new BezierTerrain(std::move(program), coverTexture, vertices, indices));

  terrain->scale(TERRAIN_DIMENSIONS);

  return terrain;
}

// We compute a shadow map at the max resolution once.
void BezierTerrain::recomputeShadowMap(const Scene& scene) {
  AutoGLErrorChecker checker;

  glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFB);
  glClear(GL_DEPTH_BUFFER_BIT);

  drawTerrainInternal(scene,
      scene.shadowMapViewProjection(), scene.lightSourcePosition(), true);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Optional<GLuint> BezierTerrain::shadowMapFBO() const {
  return Some(m_shadowMapFB);
}
