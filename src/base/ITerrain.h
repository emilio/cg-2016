#pragma once

#include "glm/glm.hpp"

class Scene;

// Can't believe I'm doing this.
class ITerrain {
public:
  virtual void drawTerrain(Scene&,
                           const glm::mat4& viewProjection,
                           const glm::vec3& cameraPos) const = 0;
  virtual ~ITerrain() {}
};
