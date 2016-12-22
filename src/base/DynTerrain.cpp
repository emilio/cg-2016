#include "DynTerrain.h"
#include "base/Logging.h"
#include "base/Scene.h"
#include "base/Terrain.h"
#include "geometry/DrawContext.h"

#include <vector>
#include <SFML/Graphics.hpp>

// TODO: Share this among terrains, possibly converting this to a EBO.
std::vector<glm::vec2> makePlane(uint32_t width, uint32_t height) {
  std::vector<glm::vec2> ret;
  ret.reserve(width * height * 4);

  float frac_x = 1.0f / width;
  float frac_y = 1.0f / height;

#define AT(x, y) (glm::vec2((frac_x * (x)-0.5), (frac_y * (y)-0.5)))

  // TODO: Could we send quads and just split those?
  for (size_t x = 0; x < width; ++x) {
    for (size_t y = 0; y < height; ++y) {
      ret.push_back(AT(x, y + 1));
      ret.push_back(AT(x + 1, y));
      ret.push_back(AT(x, y));
      ret.push_back(AT(x + 1, y + 1));
      ret.push_back(AT(x + 1, y));
      ret.push_back(AT(x, y + 1));
    }
  }

#undef AT

  return ret;
}

DynTerrain::DynTerrain(std::unique_ptr<Program> a_program,
                       sf::Image&& a_image,
                       GLuint a_cover,
                       GLuint a_heightmap,
                       std::vector<glm::vec2> a_vertices)
  : m_program(std::move(a_program))
  , m_coverTexture(a_cover)
  , m_heightmapTexture(a_heightmap)
  , m_heightmap(std::move(a_image))
  , m_vertices(std::move(a_vertices)) {
  AutoGLErrorChecker checker;
  glGenVertexArrays(1, &m_vao);

  glBindVertexArray(m_vao);
  glGenBuffers(1, &m_vbo);

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * m_vertices.size(),
               m_vertices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);

  glBindVertexArray(0);

  queryUniforms();
}

// FIXME: This should live in a common place to avoid all the duplicated code.
GLuint DynTerrain::textureFromImage(const sf::Image& image, bool a_mipmaps) {
  GLuint ret;

  auto size = image.getSize();

  AutoGLErrorChecker checker;
  glGenTextures(1, &ret);
  glBindTexture(GL_TEXTURE_2D, ret);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, image.getPixelsPtr());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  a_mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  if (a_mipmaps) {
    AutoGLErrorChecker checker;

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.0f);
    glGenerateMipmap(GL_TEXTURE_2D);

    float maxAnisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    float val = std::max(4.0f, maxAnisotropy);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, val);
  }

  glBindTexture(GL_TEXTURE_2D, 0);

  return ret;
}

std::unique_ptr<DynTerrain> DynTerrain::create() {
  ShaderSet shaders("res/dyn-terrain/common.glsl",
                    "res/dyn-terrain/vertex.glsl",
                    "res/dyn-terrain/fragment.glsl");
  shaders.m_geometry = "res/dyn-terrain/geometry.glsl";
  shaders.m_tessellation_control = "res/dyn-terrain/tess-control.glsl";
  shaders.m_tessellation_evaluation = "res/dyn-terrain/tess-eval.glsl";

  auto program = Program::fromShaders(shaders);
  if (!program) {
    ERROR("Failed to create DynTerrain program");
    return nullptr;
  }
  sf::Image heightMapImporter;
  sf::Image coverImporter;

  if (!heightMapImporter.loadFromFile("res/terrain/heightmap.png")) {
    // if (!heightMapImporter.loadFromFile("res/terrain/maribor.png")) {
    ERROR("Error loading heightmap");
    return nullptr;
  }

  if (!coverImporter.loadFromFile("res/terrain/cover.png")) {
    ERROR("Error loading cover");
    return nullptr;
  }

  GLuint cover = textureFromImage(coverImporter, true);
  GLuint heightmap = textureFromImage(heightMapImporter, false);

  auto ret = std::unique_ptr<DynTerrain>(
      new DynTerrain(std::move(program), std::move(heightMapImporter), cover,
        heightmap, makePlane(TERRAIN_DIMENSIONS, TERRAIN_DIMENSIONS)));

  ret->scale(TERRAIN_DIMENSIONS);
  return ret;
}

DynTerrain::~DynTerrain() {
  glDeleteTextures(1, &m_coverTexture);
  glDeleteTextures(1, &m_heightmapTexture);

  glDeleteBuffers(1, &m_vbo);
  glDeleteVertexArrays(1, &m_vao);
}

void DynTerrain::queryUniforms() {
#define QUERY(u)                                                               \
  do {                                                                         \
    m_uniforms.u = glGetUniformLocation(m_program->id(), #u);                  \
    /* assert(m_uniforms.u != -1); */                                          \
  } while (0)

  QUERY(uCameraPosition);
  QUERY(uLightSourcePosition);
  QUERY(uViewProjection);
  QUERY(uShadowMapViewProjection);
  QUERY(uModel);
  QUERY(uCover);
  QUERY(uHeightMap);
  QUERY(uShadowMap);
  QUERY(uDimension);
}

void DynTerrain::drawTerrain(const Scene& scene) const {
  m_program->use();
  glDisable(GL_CULL_FACE);
  // glCullFace(GL_BACK);
  glBindVertexArray(m_vao);

  glm::mat4 viewProjection = scene.viewProjection();
  glUniform3fv(m_uniforms.uCameraPosition, 1,
               glm::value_ptr(scene.cameraPosition()));
  glUniform3fv(m_uniforms.uLightSourcePosition, 1,
               glm::value_ptr(scene.lightSourcePosition()));
  glUniformMatrix4fv(m_uniforms.uViewProjection, 1, GL_FALSE,
                     glm::value_ptr(viewProjection));
  glUniformMatrix4fv(m_uniforms.uShadowMapViewProjection, 1, GL_FALSE,
                     glm::value_ptr(scene.shadowMapViewProjection()));
  glUniformMatrix4fv(m_uniforms.uModel, 1, GL_FALSE,
                     glm::value_ptr(transform()));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_coverTexture);

  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, m_heightmapTexture);

  if (scene.shadowMap()) {
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, *scene.shadowMap());
  }

  // These should be constant.
  glUniform1i(m_uniforms.uCover, 0);
  glUniform1i(m_uniforms.uHeightMap, 1);
  glUniform1i(m_uniforms.uShadowMap, 2);
  glUniform1f(m_uniforms.uDimension, TERRAIN_DIMENSIONS);

  if (m_program->tessControlShader()) {
    glPatchParameteri(GL_PATCH_VERTICES, 3);
    glDrawArrays(GL_PATCHES, 0, m_vertices.size());
  } else {
    glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());
  }

  glBindVertexArray(0);
  glUseProgram(0);
  glEnable(GL_CULL_FACE);
}

float DynTerrain::heightAt(float x, float y) const {
  assert(x < TERRAIN_DIMENSIONS);
  assert(y < TERRAIN_DIMENSIONS);
  float x_ = x / TERRAIN_DIMENSIONS;
  float y_ = y / TERRAIN_DIMENSIONS;
  auto size = m_heightmap.getSize();

  // This is kind of fiddly, because we're duplicating the stuff that is in the
  // fragment shader, but oh well.
  float v = m_heightmap.getPixel(x_ * size.x, y_ * size.y).g /  255.0f;
  return (v - 0.5) / 3.0 * TERRAIN_DIMENSIONS;
}
