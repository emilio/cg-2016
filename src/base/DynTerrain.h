#pragma once

#include "base/Program.h"
#include "base/Program.h"
#include "base/ITerrain.h"
#include "geometry/Node.h"
#include <memory>
#include <vector>
#include <SFML/Graphics.hpp>

/**
 * This is intended to be an alternative terrain representation using quads
 * instead of triangles, that will allow me to use fancy tessellation shaders
 * with bezier interpolation.
 */
class DynTerrain final : public Node, public ITerrain {
  std::unique_ptr<Program> m_program;

  GLuint m_coverTexture;

  // TODO: If we want to eventually do collision detection with this terrain we
  // have to either keep the heightmap in memory, or similar.
  GLuint m_heightmapTexture;

  sf::Image m_heightmap;

  // A vector with the vertices divided in quads. Note that we calculate the
  // height of the terrain dynamically in the vertex shader.
  std::vector<glm::vec2> m_vertices;

  struct {
    GLint uCameraPosition;
    GLint uLightSourcePosition;
    GLint uViewProjection;
    GLint uShadowMapViewProjection;
    GLint uModel;
    GLint uCover;
    GLint uHeightMap;
    GLint uDimension;
    GLint uShadowMap;
  } m_uniforms;

  void queryUniforms();

  DynTerrain(std::unique_ptr<Program>,
             sf::Image&&,
             GLuint,
             GLuint,
             std::vector<glm::vec2>);

  GLuint m_vao;
  // TODO: We could maybe optimize the memory representation of the quads using
  // an ebo, but in practice it doesn't matter that much.
  GLuint m_vbo;

public:
  virtual ~DynTerrain();
  static std::unique_ptr<DynTerrain> create();
  static GLuint textureFromImage(const sf::Image& image, bool a_mipmaps);

  virtual void drawTerrain(const Scene&) const override;
  virtual float heightAt(float x, float y) const override;
  void draw(DrawContext&) const override {
    assert(false && "not implemented! use drawTerrain instead!");
  }
};
