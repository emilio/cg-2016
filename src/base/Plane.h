#pragma once

#include <chrono>

#include "geometry/Node.h"

#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"

class DrawContext;

class Plane final : public Node {
  float m_speed;

  glm::quat m_orientation;
  glm::vec3 m_position;

  Plane();

public:
  using Milliseconds = std::chrono::duration<float, std::ratio<1, 1000>>;

  const glm::vec3 direction() const {
    return m_orientation * glm::vec3(0.0, 0.0, -1.0);
  }

  const glm::vec3& position() const {
    return m_position;
  }

  const glm::vec3 normal() const {
    return m_orientation * glm::vec3(0.0, 1.0, 0.0);
    ;
  }

  const glm::vec3 left() const {
    return glm::cross(direction(), normal());
  }

  void speedUp(float amount) {
    m_speed += amount;
  }

  void advance(const Milliseconds& a_howMany) {
    float distance = m_speed * a_howMany.count() / 1000.0f;
    m_position += distance * direction();
  }

  void pitch(float amount);
  void yaw(float);
  void roll(float);

  void computeTransform();

  static std::unique_ptr<Plane> create();
};
