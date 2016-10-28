#ifndef Terrain_h
#define Terrain_h

#include "geometry/Mesh.h"

const size_t TERRAIN_WIDTH = 2000;
const size_t TERRAIN_DEPTH = 2000;
const size_t TERRAIN_HEIGHT_VARIANCE = 10;

class Terrain {
  Terrain() = delete;

public:
  static std::unique_ptr<Terrain> create();
};

#endif
