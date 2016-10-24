#include "loader/BasicObjLoader.h"
#include "base/Logging.h"
#include "glm/gtc/type_ptr.hpp"

namespace loader {

// Comment those to get proper debugging output.
//
// It's too slow to keep it always.
#undef LOG
#define LOG(...)

template <typename T, size_t N, typename Conversion>
static inline bool ReadComponents(T* a_out,
                                  Conversion aConversionFn,
                                  std::string& a_line,
                                  size_t a_current_index) {
  uint32_t current_vertex_component = 0;

  do {
    if (current_vertex_component > N - 1) {
      if (a_current_index == a_line.length() - 1) {
        return true;
      }
      ERROR(
          "Invalid vertex: %s, got %u components, currently at position %zu of "
          "%zu",
          a_line.c_str(), current_vertex_component, a_current_index,
          a_line.length());
      return false;
    }

    assert(a_line.length() > a_current_index);
    a_out[current_vertex_component] =
        aConversionFn(&a_line[a_current_index + 1]);
    current_vertex_component++;
  } while ((a_current_index = a_line.find(" ", a_current_index + 1)) !=
           std::string::npos);

  return current_vertex_component == N;
}

/* static */ bool BasicObjLoader::load(std::istream& a_inStream,
                                       std::vector<Vertex>& a_vertices,
                                       std::vector<GLuint>& a_indices) {
  a_vertices.clear();
  a_indices.clear();

  std::string line;
  while (std::getline(a_inStream, line)) {
    if (line.empty() || line[0] == '#') {
      continue;
    }

    if (line.length() <= 3 || line[1] != ' ') {
      LOG("Ignoring too-short or invalid line: %s", line.c_str());
      continue;
    }

    LOG("scanning line: %s", line.c_str());
    switch (line[0]) {
      case 'v': {
        glm::vec3 vertex;
        if (!ReadComponents<float, 3>(glm::value_ptr(vertex), std::atof, line,
                                      1)) {
          ERROR("Invalid line: %s", line.c_str());
          return false;
        }
        LOG("Scanned %s, got (%f, %f, %f)", line.c_str(), vertex[0], vertex[1],
            vertex[2]);
        a_vertices.push_back({vertex, glm::vec3(), glm::vec2()});
        break;
      }
      case 'f': {
        GLuint indices[3];
        if (!ReadComponents<GLuint, 3>(indices, std::atoll, line, 1)) {
          ERROR("Invalid line: %s", line.c_str());
          return false;
        }

        LOG("Scanned %s, got (%u, %u, %u)", line.c_str(), indices[0],
            indices[1], indices[2]);

        for (auto& index : indices) {
          if (index == 0) {
            ERROR("Found 0-based index in object file");
            return false;
          }

          index--;

          if (index >= a_vertices.size()) {
            ERROR("Invalid index %u (had %u vertices)", index,
                  (unsigned int)a_vertices.size());
            return false;
          }
        }
        a_indices.insert(a_indices.end(), std::begin(indices),
                         std::end(indices));
        break;
      }
    }
  }

  assert(a_indices.size() % 3 == 0);

  for (size_t i = 0; i < a_indices.size(); i += 3) {
    GLuint i_1 = a_indices[i];
    GLuint i_2 = a_indices[i + 1];
    GLuint i_3 = a_indices[i + 2];
    a_vertices[i_1].m_normal = a_vertices[i_2].m_normal =
        a_vertices[i_3].m_normal = glm::normalize(glm::cross(
            a_vertices[i_2].m_position - a_vertices[i_1].m_position,
            a_vertices[i_3].m_position - a_vertices[i_1].m_position));
  }

  return true;
}

}  // namespace loader
