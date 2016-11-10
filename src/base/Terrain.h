#pragma once

#include "base/Program.h"
#include "geometry/Mesh.h"

const float TERRAIN_DIMENSIONS = 100.0f;

class Terrain final : public Mesh {
  Terrain(std::vector<Vertex>&& vertices,
          std::vector<GLuint>&& indices,
          Optional<GLuint> texture);

public:
  static std::unique_ptr<Terrain> create();
};
