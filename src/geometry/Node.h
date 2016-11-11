#pragma once

#include <list>
#include <memory>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class DrawContext;

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

  glm::vec3 m_color;

public:
  Node(glm::vec3 a_color) : m_color(a_color) {}

  Node() : Node(glm::vec3(0.5, 0.5, 0.5)) {}

  virtual ~Node() {}

  virtual void draw(DrawContext& context) const;

  void addChild(std::unique_ptr<Node> a_child) {
    m_children.push_back(std::move(a_child));
  }

  void setColor(const glm::vec3& a_color) {
    m_color = a_color;
  }

  const glm::mat4& transform() const {
    return m_transform;
  }

  void setTransform(const glm::mat4& a_transform) {
    m_transform = a_transform;
  }

  const glm::vec3& color() const {
    return m_color;
  }

  void translate(const glm::vec3& a_how) {
    m_transform = glm::translate(m_transform, a_how);
  }

  void translate(float a_howMuch, const glm::vec3& a_direction) {
    m_transform =
        glm::translate(m_transform, a_howMuch * glm::normalize(a_direction));
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

  void scale(const float a_times) {
    scale(glm::vec3(a_times, a_times, a_times));
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
