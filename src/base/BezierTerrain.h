#pragma once

#include "geometry/Node.h"
#include "base/gl.h"
#include "ITerrain.h"

#include <vector>

class Program;
class Scene;

class BezierTerrain final : public Node,
                            public ITerrain
{
  std::unique_ptr<Program> m_program;

  GLuint m_coverTexture;

  // We don't bother to keep the vertices in memory, maybe we should?
  // std::vector<glm::vec3> m_vertices;
  // std::vector<GLuint> m_indices;
  size_t m_indicesCount;

  struct {
    GLint uLodEnabled;
    GLint uLodLevel;
    GLint uCameraPosition;
    GLint uViewProjection;
    GLint uModel;
    GLint uCover;
    GLint uShadowMap;
    GLint uDimension;
    GLint uDrawingForShadowMap;
    GLint uShadowMapViewProjection;
  } m_uniforms;

  GLuint m_vao;
  GLuint m_ebo;
  GLuint m_vbo;

  GLuint m_shadowMapFB;
  GLuint m_shadowMapTexture;

  BezierTerrain(std::unique_ptr<Program>,
                GLuint,
                const std::vector<glm::vec3>&,
                const std::vector<GLuint>&);

  void queryUniforms();

public:
  virtual void drawTerrain(const Scene&) const override;
  void drawTerrainInternal(const Scene&, const glm::mat4& viewProjection, const
                           glm::vec3& cameraPos, bool forShadowMap) const;

  virtual ~BezierTerrain();

  static std::unique_ptr<BezierTerrain> create();

  virtual void recomputeShadowMap(const Scene&) override;
  virtual Optional<GLuint> shadowMapFBO() const override;

  virtual void draw(DrawContext&) const override {
    assert(false && "call drawTerrain instead!");
  }
};
