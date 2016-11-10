#pragma once

#include <chrono>

#include "glm/glm.hpp"

class Node;
class Scene;

struct PhysicsState {
  using Clock = std::chrono::system_clock;
  using TimePoint = std::chrono::time_point<Clock>;
  using Milliseconds = std::chrono::duration<float, std::ratio<1, 1000>>;

  TimePoint m_lastPhysics;
  float m_planeSpeed;
  glm::vec3 m_planeDirection;

  PhysicsState()
    : m_lastPhysics(Clock::now())
    , m_planeSpeed(5.0f)
    , m_planeDirection(0.0, 0.0, -1.0) {}


  void tick(Node& plane, Scene&);
};
