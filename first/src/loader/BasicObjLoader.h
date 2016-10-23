#pragma once

#include <iostream>
#include <vector>

#include "geometry/Vertex.h"
#include "base/gl.h"

namespace loader {

class BasicObjLoader {
public:
  static bool
  Load(std::istream& a_inStream,
       std::vector<Vertex>& a_vertices,
       std::vector<uint32_t>& a_indices);
};

}
