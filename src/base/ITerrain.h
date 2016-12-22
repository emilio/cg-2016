#pragma once

#include "glm/glm.hpp"
#include "tools/Optional.h"

class Scene;

// Can't believe I'm doing this.
class ITerrain {
public:
  virtual void drawTerrain(const Scene&) const = 0;
  virtual void recomputeShadowMap(const Scene&){};
  virtual float heightAt(float x, float y) const = 0;
  /**
   * The contract with this function is that the FBO is immutable and only used
   * for reading.
   *
   * It'd be complex to enforce this at the type system level, so we just hope
   * callers act properly.
   */
  virtual Optional<GLuint> shadowMapFBO() const {
    return None;
  };

  virtual bool wantsShadowMap() const {
    return true;
  };

  virtual ~ITerrain() {}
};
