#include "main/PhysicsState.h"

#include "geometry/Node.h"
#include "base/Logging.h"
#include "base/Scene.h"
#include "base/Plane.h"

#include <cmath>

static glm::vec3 interpolateVectors(const glm::vec3& one,
                                    const glm::vec3& other,
                                    float amount) {
  return one + (other - one) * amount;
}


glm::vec3 PhysicsState::normal() const {
  return m_orientation * Y_AXIS;
}

PhysicsState::PhysicsState(Plane& a_node)
  : m_lastPhysics(Clock::now()), m_plane(a_node) {}

void PhysicsState::tick(Scene& scene) {
  const float INTERPOLATION_FACTOR = 0.1f;

  TimePoint now = Clock::now();
  Milliseconds diff = now - m_lastPhysics;

  m_plane.advance(diff);

  auto newPlanePos = m_plane.position();

  // Now we have to build a rotation matrix so it points to `m_direction`, and a
  // translation one so the object is in m_planePosition.
  m_plane.computeTransform();

  auto oldCameraPos = scene.m_cameraPosition;
  auto targetCameraPos =
      m_plane.position() - CAM_PLANE_DISTANCE * m_plane.direction();

  m_orientation = glm::slerp(m_orientation, m_plane.orientation(), INTERPOLATION_FACTOR);

  scene.m_cameraPosition = interpolateVectors(oldCameraPos, targetCameraPos, INTERPOLATION_FACTOR);

  scene.recomputeView(newPlanePos, normal());

  m_lastPhysics = now;
}

void PhysicsState::speedUp(float amount) {
  m_plane.speedUp(amount);
}

void PhysicsState::rotate(Scene&, Direction dir, float amount) {
  bool isTopDown = dir == Direction::Top || dir == Direction::Down;

  if (dir == Direction::Top || dir == Direction::Left)
    amount *= -1.0;

  if (isTopDown)
    m_plane.pitch(amount);
  else
    m_plane.roll(amount);
}
