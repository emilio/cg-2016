#pragma once

#include "glm/glm.hpp"

// Can't believe I'm doing this.
class ITerrain {
public:
  virtual void drawTerrain(const glm::mat4& viewProjection,
                           const glm::vec3& cameraPos) const = 0;
  virtual ~ITerrain() {}
};
