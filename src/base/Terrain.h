#ifndef Terrain_h
#define Terrain_h

#include "base/Program.h"
#include "geometry/Mesh.h"

const size_t TERRAIN_WIDTH = 2000;
const size_t TERRAIN_DEPTH = 2000;
const size_t TERRAIN_HEIGHT_VARIANCE = 10;

class Terrain final : public Mesh {
  Terrain(std::vector<Vertex>&& vertices,
          std::vector<GLuint>&& indices,
          Optional<GLuint> texture);

public:
  static std::unique_ptr<Terrain> create();
};

#endif
