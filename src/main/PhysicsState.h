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
  float m_planeSpeed;
  glm::vec3 m_planeDirection;
  glm::vec3 m_planeNormal;
  Node& m_plane;

  glm::vec3 planePosition() const;

public:
  enum Direction {
    Top,
    Down,
    Right,
    Left,
  };

  PhysicsState(Node& a_node)
    : m_lastPhysics(Clock::now())
    , m_planeSpeed(2.0f)
    , m_planeDirection(0.0, 0.0, -1.0)
    , m_planeNormal(0.0, 1.0, 0.0)
    , m_plane(a_node) {}

  void tick(Scene&);

  void speedUp(float amount) {
    m_planeSpeed += amount;
  }

  void rotate(Direction, float amount);
};
