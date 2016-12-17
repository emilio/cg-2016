#pragma once

#include "geometry/Node.h"
#include "base/gl.h"
#include "ITerrain.h"

#include <vector>

class Program;
class Scene;

struct BezierTerrainUniformsForShadowMap {
  GLint uModel;
  GLint uShadowMapViewProjection;

  void query(const Program&);
};

struct BezierTerrainUniforms : public BezierTerrainUniformsForShadowMap {
  GLint uCameraPosition;
  GLint uLodEnabled;
  GLint uLodLevel;
  GLint uCover;
  GLint uShadowMap;
  GLint uDimension;
  GLint uViewProjection;

  void query(const Program&);
  void update(const Scene&) const;
};

class BezierTerrain final : public Node, public ITerrain {
  std::unique_ptr<Program> m_program;
  BezierTerrainUniforms m_uniforms;
  std::unique_ptr<Program> m_programForShadowMap;
  BezierTerrainUniformsForShadowMap m_uniformsForShadowMap;

  GLuint m_coverTexture;

  // We don't bother to keep the vertices in memory, maybe we should?
  // std::vector<glm::vec3> m_vertices;
  // std::vector<GLuint> m_indices;
  size_t m_indicesCount;

  GLuint m_vao;
  GLuint m_ebo;
  GLuint m_vbo;

  GLuint m_shadowMapFB;
  GLuint m_shadowMapTexture;

  BezierTerrain(std::unique_ptr<Program>,
                std::unique_ptr<Program>,
                GLuint,
                const std::vector<glm::vec3>&,
                const std::vector<GLuint>&);

  void queryUniforms();

public:
  virtual void drawTerrain(const Scene&) const override;
  void drawTerrainInternal(const Scene&, bool forShadowMap) const;

  virtual ~BezierTerrain();

  static std::unique_ptr<BezierTerrain> create();

  virtual void recomputeShadowMap(const Scene&) override;
  virtual Optional<GLuint> shadowMapFBO() const override;

  virtual void draw(DrawContext&) const override {
    assert(false && "call drawTerrain instead!");
  }
};
