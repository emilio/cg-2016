#include "main/PhysicsState.h"

#include "geometry/Node.h"
#include "base/Logging.h"
#include "base/Scene.h"
#include "base/Plane.h"

#include <cmath>

PhysicsState::PhysicsState(Plane& a_node)
  : m_lastPhysics(Clock::now()), m_plane(a_node) {}

void PhysicsState::tick(Scene& scene) {
  TimePoint now = Clock::now();
  Milliseconds diff = now - m_lastPhysics;

  m_plane.advance(diff);

  // Now we have to build a rotation matrix so it points to `m_direction`, and a
  // translation one so the object is in m_planePosition.
  m_plane.computeTransform();

  scene.m_cameraPosition =
      m_plane.position() - CAM_PLANE_DISTANCE * m_plane.direction();

  scene.recomputeView(m_plane.position(), m_plane.normal());

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
