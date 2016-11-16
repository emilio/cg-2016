#pragma once

#include <chrono>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

class Plane;
class Scene;

class PhysicsState {
  using Clock = std::chrono::system_clock;
  using TimePoint = std::chrono::time_point<Clock>;
  using Milliseconds = std::chrono::duration<float, std::ratio<1, 1000>>;

  TimePoint m_lastPhysics;
  Plane& m_plane;

  /**
   * This quaternion is used to implement a kind of "follow the object"
   * navigation, where the camera interpolates between the old orientation and
   * the object orientation for a given factor.
   */
  glm::quat m_orientation;

  glm::vec3 normal() const;

public:
  enum Direction {
    Top,
    Down,
    Right,
    Left,
  };

  PhysicsState(Plane& a_node);
  void tick(Scene&);
  void speedUp(float amount);
  void rotate(Scene&, Direction, float amount);
};
