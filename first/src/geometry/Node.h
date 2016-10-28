#pragma once

#include <list>
#include <memory>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

/**
 * A node is an item in a scene.
 *
 * It can be a mesh or a group of children nodes.
 */
class Node {
  std::list<std::unique_ptr<Node>> m_children;

protected:

  // The local transform of this object.
  glm::mat4 m_transform;

public:
  Node() {}

  virtual void draw() {
    for (auto& child : m_children)
      child->draw();
  }

  void addChild(std::unique_ptr<Node> a_child) {
    m_children.push_back(std::move(a_child));
  }

  const glm::mat4& transform() const {
    return m_transform;
  }

  void translate(const glm::vec3& a_how) {
    m_transform = glm::translate(m_transform, a_how);
  }

  void translateX(float a_howMuch) {
    translate(glm::vec3(a_howMuch, 0, 0));
  }

  void translateY(float a_howMuch) {
    translate(glm::vec3(0, a_howMuch, 0));
  }

  void translateZ(float a_howMuch) {
    translate(glm::vec3(0, 0, a_howMuch));
  }

  void rotate(float a_angleInRadians, const glm::vec3& a_around) {
    m_transform = glm::rotate(m_transform, a_angleInRadians, a_around);
  }

  void rotateX(float a_angleInRadians) {
    rotate(a_angleInRadians, glm::vec3(1, 0, 0));
  }

  void rotateY(float a_angleInRadians) {
    rotate(a_angleInRadians, glm::vec3(0, 1, 0));
  }

  void rotateZ(float a_angleInRadians) {
    rotate(a_angleInRadians, glm::vec3(0, 0, 1));
  }

  void scale(const glm::vec3& a_times) {
    m_transform = glm::scale(m_transform, a_times);
  }

  void scaleX(float a_times) {
    scale(glm::vec3(a_times, 1.0, 1.0));
  }

  void scaleY(float a_times) {
    scale(glm::vec3(1.0, a_times, 1.0));
  }

  void scaleZ(float a_times) {
    scale(glm::vec3(1.0, 1.0, a_times));
  }

  static std::unique_ptr<Node> fromFile(const char* a_modelPath);
};
