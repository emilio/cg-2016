#include "main/PhysicsState.h"

#include "geometry/Node.h"
#include "base/Scene.h"

// Computes the plane position in world space.
glm::vec3 PhysicsState::planePosition() const {
  return glm::vec3(m_plane.transform() * glm::vec4(0, 0, 0, 1.0));

}

void PhysicsState::tick(Scene& scene) {
  TimePoint now = Clock::now();
  Milliseconds diff = now - m_lastPhysics;

  float distance = m_planeSpeed * diff.count() / 1000.0f;

  // TODO: collision detection, changing direction...
  // all the good stuff.
  m_plane.translate(distance, m_planeDirection);

  glm::vec3 planePosition = this->planePosition();

  scene.m_cameraPosition = planePosition - 5.0f * m_planeDirection;

  // TODO: Compute the `up` axis correctly.
  scene.recomputeView(planePosition, m_planeNormal);

  m_lastPhysics = now;
}

void PhysicsState::rotate(Direction dir, float amount) {
  bool isTopDown = false;
  float multiplier = 1.0;

  if (dir == Direction::Top || dir == Direction::Down)
    isTopDown = true;

  if (dir == Direction::Down || dir == Direction::Right)
    multiplier = -1.0;

  // FIXME: We probably have to account for planeNormal here too if we want to
  // support inclination and stuff, sigh. I think the cross product of it and
  // the axis will do.
  //
  // Also, this is wrong by itself.
  const auto& axis = isTopDown ? X_AXIS : Y_AXIS;

  m_plane.rotate(amount * multiplier, axis);

  glm::mat4 rot = glm::rotate(glm::mat4(), amount * multiplier, axis);
  m_planeDirection = rot * glm::vec4(m_planeDirection, 0.0);
  if (isTopDown)
    m_planeNormal = rot * glm::vec4(m_planeNormal, 0.0);
}
