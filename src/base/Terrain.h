#pragma once

#include "base/Program.h"
#include "base/ITerrain.h"
#include "geometry/Mesh.h"

#include <SFML/Graphics.hpp>

const size_t TERRAIN_DIMENSIONS = 100;

class Terrain final : public ITerrain, public Mesh {
  Terrain(std::vector<Vertex>&& vertices,
          std::vector<GLuint>&& indices,
          sf::Image&& heightMap,
          Material material,
          Optional<GLuint> texture);

  sf::Image m_heightMap;

public:
  virtual ~Terrain() {}
  static std::unique_ptr<Terrain> create();

  virtual bool hasCustomProgram() const override { return false; }
  virtual bool wantsShadowMap() const override { return true; }
  virtual void drawTerrain(const Scene&) const override;
  virtual void recomputeShadowMap(const Scene&) override {}
  virtual float heightAt(float x, float y) const override;
};
