#pragma once

#include "gl.h"
#include "glm/glm.hpp"
#include <optional>

class Scene;

// Can't believe I'm doing this.
class ITerrain {
public:
  virtual bool hasCustomProgram() const { return true; }
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
  virtual std::optional<GLuint> shadowMapFBO() const {
    return std::nullopt;
  };

  virtual bool wantsShadowMap() const {
    return false;
  };

  virtual ~ITerrain() {}
};
