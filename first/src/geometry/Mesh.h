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

    aOther.m_vao = aOther.m_vbo = aOther.m_ebo = UNINITIALIZED;
  }

  ~Mesh() {
    if (m_vao == UNINITIALIZED) {
      assert(m_vbo == UNINITIALIZED);
      assert(m_ebo == UNINITIALIZED);
      return;
    }

    AutoGLErrorChecker checker;
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
  }

  Mesh(std::vector<Vertex>&& a_vertices,
       std::vector<GLuint>&& a_indices)
    : m_vertices(a_vertices)
    , m_indices(a_indices)
    , m_vao(UNINITIALIZED)
    , m_vbo(UNINITIALIZED)
    , m_ebo(UNINITIALIZED)
  {
#ifdef DEBUG
    for (auto index : m_indices) {
      assert(index < m_vertices.size() || !"Index out of bounds");
    }
#endif

    AutoGLErrorChecker checker;
    glGenVertexArrays(1, &m_vao);

    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_vertices.size(),
                 m_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * m_indices.size(),
                 m_indices.data(), GL_STATIC_DRAW);

#define INT_TO_GLVOID(i) ((GLvoid*)i)

    // Vertex positions.
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          INT_TO_GLVOID(offsetof(Vertex, m_position)));

    // Vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          INT_TO_GLVOID(offsetof(Vertex, m_normal)));

    // Vertex uv coordinates.
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          INT_TO_GLVOID(offsetof(Vertex, m_uv)));
    glBindVertexArray(0);
  }

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

  static std::unique_ptr<Mesh>
  fromFile(const char* a_modelPath);
};
