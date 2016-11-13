#pragma once

#include <chrono>

#include "glm/glm.hpp"

class Plane;
class Scene;

const float CAM_PLANE_DISTANCE = 5.0f;

class PhysicsState {
  using Clock = std::chrono::system_clock;
  using TimePoint = std::chrono::time_point<Clock>;
  using Milliseconds = std::chrono::duration<float, std::ratio<1, 1000>>;

  TimePoint m_lastPhysics;
  Plane& m_plane;

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
