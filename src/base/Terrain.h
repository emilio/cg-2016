#pragma once

#include "base/Program.h"
#include "geometry/Mesh.h"

const size_t TERRAIN_DIMENSIONS = 100;

class Terrain final : public Mesh {
  Terrain(std::vector<Vertex>&& vertices,
          std::vector<GLuint>&& indices,
          Material material,
          Optional<GLuint> texture);

public:
  static std::unique_ptr<Terrain> create();
};
