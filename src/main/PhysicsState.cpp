#include "main/PhysicsState.h"

#include "geometry/Node.h"
#include "base/Logging.h"
#include "base/Scene.h"

#include <cmath>

PhysicsState::PhysicsState(Node& a_node)
  : m_lastPhysics(Clock::now())
  , m_plane(a_node)
  , m_planeSpeed(2.0f)
  , m_direction(0.0, 0.0, -1.0)
  , m_planeInitialTransform(m_plane.transform())
  , m_planePosition(m_planeInitialTransform * glm::vec4(0, 0, 0, 1.0)) {}

void PhysicsState::tick(Scene& scene) {
  TimePoint now = Clock::now();
  Milliseconds diff = now - m_lastPhysics;

  float distance = m_planeSpeed * diff.count() / 1000.0f;

  m_planePosition = m_planePosition + distance * m_direction;

  auto planeTransform = glm::translate(glm::mat4(), m_planePosition);
  // {
  //   // TODO: We could cache this if the direction hasn't changed.
  //   auto angleXY = std::atan2(m_direction.y, m_direction.x);
  //   planeTransform = glm::rotate(planeTransform, angleXY, Z_AXIS);

  //   auto angleZY = std::atan2(m_direction.y, m_direction.z);
  //   planeTransform = glm::rotate(planeTransform, angleZY, X_AXIS);

  //   auto angleZX = std::atan2(m_direction.z, m_direction.x);
  //   planeTransform = glm::rotate(planeTransform, angleZX, Y_AXIS);
  // }

  m_plane.setTransform(planeTransform);

  scene.m_cameraPosition = m_planePosition - 4.0f * m_direction;
  scene.recomputeView(m_planePosition, Y_AXIS);

  m_lastPhysics = now;
}

void PhysicsState::rotate(Scene&, Direction dir, float amount) {
  float multiplier = 1.0;
  bool isTopDown = dir == Direction::Top || dir == Direction::Down;

  if (dir == Direction::Down || dir == Direction::Right)
    multiplier = -1.0;

  glm::vec3 axis = isTopDown ?  X_AXIS : Y_AXIS;
  auto rot = glm::rotate(glm::mat4(), amount * multiplier, axis);
  m_direction = rot * glm::vec4(m_direction, 0.0);
}
