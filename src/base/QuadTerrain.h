#pragma once

#include "base/Program.h"
#include "geometry/Node.h"
#include <memory>
#include <vector>

/**
 * This is intended to be an alternative terrain representation using quads
 * instead of triangles, that will allow me to use fancy tessellation shaders
 * with bezier interpolation.
 */
class QuadTerrain final : public Node {
  std::unique_ptr<Program> m_program;

  GLuint m_coverTexture;

  // TODO: If we want to eventually do collision detection with this terrain we
  // have to either keep the heightmap in memory, or similar.
  GLuint m_heightmapTexture;

  // A vector with the vertices divided in quads. Note that we calculate the
  // height of the terrain dynamically in the vertex shader.
  std::vector<glm::vec2> m_quads;

  QuadTerrain(std::unique_ptr<Program>, GLuint, GLuint, std::vector<glm::vec2>);

  GLuint m_vao;
  // TODO: We could maybe optimize the memory representation of the quads using
  // an ebo, but in practice it doesn't matter that much.
  GLuint m_vbo;

public:
  virtual ~QuadTerrain();
  static std::unique_ptr<QuadTerrain> create();
};
