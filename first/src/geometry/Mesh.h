#pragma once

#include "base/gl.h"
#include "base/Logging.h"
#include "base/ErrorChecker.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "geometry/Vertex.h"

#include <vector>
#include <memory>

#define UNINITIALIZED ((GLuint)-1)

class Mesh {
  std::vector<Vertex> m_vertices;
  std::vector<GLuint> m_indices;

  // The vertex array object.
  GLuint m_vao;

  // The buffer object for the vertices.
  GLuint m_vbo;

  // The buffer object for the indices.
  GLuint m_ebo;

  // The local transform of this object.
  glm::mat4 m_transform;

public:
  // No copy semantics, just move.
  Mesh(const Mesh& aOther) = delete;

  Mesh(Mesh&& aOther) {
    m_vertices.swap(aOther.m_vertices);
    m_indices.swap(aOther.m_indices);

    m_vao = aOther.m_vao;
    m_vbo = aOther.m_vbo;
    m_ebo = aOther.m_ebo;
    m_transform = aOther.m_transform;

    aOther.m_vao = aOther.m_vbo = aOther.m_ebo = UNINITIALIZED;
  }

  ~Mesh();

  Mesh(std::vector<Vertex>&& a_vertices, std::vector<GLuint>&& a_indices);

  void draw();

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

  static std::unique_ptr<Mesh> fromFile(const char* a_modelPath);
};
