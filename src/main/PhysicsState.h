#pragma once

#include <chrono>

#include "glm/glm.hpp"

class Node;
class Scene;

class PhysicsState {
  using Clock = std::chrono::system_clock;
  using TimePoint = std::chrono::time_point<Clock>;
  using Milliseconds = std::chrono::duration<float, std::ratio<1, 1000>>;

  TimePoint m_lastPhysics;
  Node& m_plane;

  // We track the plane position and such manually.
  float m_planeSpeed;
  glm::vec3 m_direction;
  glm::mat4 m_planeInitialTransform;
  glm::vec3 m_planePosition;

public:
  enum Direction {
    Top,
    Down,
    Right,
    Left,
  };

  PhysicsState(Node& a_node);

  void tick(Scene&);

  void speedUp(float amount) {
    m_planeSpeed += amount;
  }

  void rotate(Scene&, Direction, float amount);
};
