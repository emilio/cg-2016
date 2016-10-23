#undef NDEBUG
#ifndef DEBUG
#define DEBUG
#endif

#define RAW(...) #__VA_ARGS__

const char* CUBE_FILE = "\
# testing comments comments\n\
v -0.500000 -0.500000 0.500000\n\
v 0.500000 -0.500000 0.500000\n\
v -0.500000 0.500000 0.500000\n\
v 0.500000 0.500000 0.500000\n\
v -0.500000 0.500000 -0.500000\n\
v 0.500000 0.500000 -0.500000\n\
v -0.500000 -0.500000 -0.500000\n\
v 0.500000 -0.500000 -0.500000\n\
\
f 1 2 3\n\
f 3 2 4\n\
f 3 4 5\n\
f 5 4 6\n\
f 5 6 7\n\
f 7 6 8\n\
f 7 8 1\n\
f 1 8 2\n\
f 2 8 4\n\
f 4 8 6\n\
f 7 1 5\n\
f 5 1 3\n";

#include <cassert>
#include <sstream>

#include "loader/BasicObjLoader.h"

int main() {
  std::istringstream in(CUBE_FILE);

  assert(in);

  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;
  bool success = loader::BasicObjLoader::Load(in, vertices, indices);
  assert(success);
  assert(vertices.size() == 8);
  assert(indices.size() == 36);

#define ASSERT_VERTEX(index_, x_, y_, z_)            \
  assert(vertices[index_].m_position.x == x_);       \
  assert(vertices[index_].m_position.y == y_);       \
  assert(vertices[index_].m_position.z == z_);

  ASSERT_VERTEX(0, -0.5, -0.5, 0.5)
  ASSERT_VERTEX(7, 0.5, -0.5, -0.5)

#define ASSERT_TRIANGLE(index_, a_, b_, c_)          \
  assert(indices[index_ * 3] == a_ - 1);             \
  assert(indices[index_ * 3 + 1] == b_ - 1);         \
  assert(indices[index_ * 3 + 2] == c_ - 1);

  ASSERT_TRIANGLE(0, 1, 2, 3);
  ASSERT_TRIANGLE(1, 3, 2, 4);
  ASSERT_TRIANGLE(11, 5, 1, 3);

  return 0;
}
