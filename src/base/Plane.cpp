#include "Plane.h"

#include "geometry/DrawContext.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_vector.hpp"

Plane::Plane() : m_speed(2.0f) {
  addChild(Node::fromFile("res/models/Airbus A310.obj"));
}

void Plane::pitch(float amount) {
  m_orientation = glm::rotate(glm::quat(), amount, left()) * m_orientation;
}

void Plane::yaw(float amount) {
  m_orientation = glm::rotate(glm::quat(), amount, normal()) * m_orientation;
}

void Plane::roll(float amount) {
  m_orientation = glm::rotate(glm::quat(), amount, direction()) * m_orientation;
}

std::unique_ptr<Plane> Plane::create() {
  return std::unique_ptr<Plane>(new Plane());
}

void Plane::computeTransform() {
  glm::mat4 rot(m_orientation);
  glm::mat4 transform = glm::translate(glm::mat4(), position());
  setTransform(transform * rot);
}
