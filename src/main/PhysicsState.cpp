#include "main/PhysicsState.h"

#include "geometry/Node.h"
#include "base/Scene.h"

void PhysicsState::tick(Node& plane, Scene& scene) {
  TimePoint now = Clock::now();
  Milliseconds diff = now - m_lastPhysics;

  float distance = m_planeSpeed * diff.count() / 1000.0f;

  // TODO: collision detection, changing direction...
  // all the good stuff.
  plane.translate(distance, m_planeDirection);

  // Compute the plane position in world space, that's where we're looking.
  glm::vec3 planePosition =
      glm::vec3(plane.transform() * glm::vec4(0, 0, 0, 1.0));

  scene.m_cameraPosition = planePosition - 5.0f * m_planeDirection;

  // TODO: Compute the `up` axis correctly.
  scene.recomputeView(planePosition, Y_AXIS);

  m_lastPhysics = now;
}
