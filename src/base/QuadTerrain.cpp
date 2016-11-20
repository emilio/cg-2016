#include "QuadTerrain.h"
#include "base/Logging.h"

#include <vector>
#include <SFML/Graphics.hpp>

std::vector<glm::vec2> makePlane(uint32_t width, uint32_t height) {
  std::vector<glm::vec2> ret;
  ret.reserve(width * height * 4);

  float frac_x = 1.0f / width;
  float frac_y = 1.0f / height;

  for (size_t x = 0; x < width; ++x) {
    for (size_t y = 0; y < height; ++y) {
      ret.push_back(glm::vec2(frac_x * x, frac_y * y));
      ret.push_back(glm::vec2((frac_x + 1) * x, frac_y * y));
      ret.push_back(glm::vec2((frac_x + 1) * x, (frac_y + 1) * y));
      ret.push_back(glm::vec2(frac_x * x, (frac_y + 1) * y));
    }
  }

  return ret;
}

QuadTerrain::QuadTerrain(std::unique_ptr<Program> a_program,
                         GLuint a_cover, GLuint a_heightmap,
                         std::vector<glm::vec2> a_quads)
  : m_program(std::move(a_program))
  , m_coverTexture(a_cover)
  , m_heightmapTexture(a_heightmap)
  , m_quads(std::move(a_quads))
{
  AutoGLErrorChecker checker;
  glGenVertexArrays(1, &m_vao);

  glBindVertexArray(m_vao);
  glGenBuffers(1, &m_vbo);

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * m_quads.size(),
               m_quads.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);

  glBindVertexArray(0);
}

// FIXME: This should live in a common place to avoid all the duplicated code.
GLuint textureFromImage(const sf::Image& image) {
  GLuint ret;

  auto size = image.getSize();

  AutoGLErrorChecker checker;
  glGenTextures(1, &ret);
  glBindTexture(GL_TEXTURE_2D, ret);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0,
      GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glBindTexture(GL_TEXTURE_2D, 0);

  return ret;
}

std::unique_ptr<QuadTerrain> QuadTerrain::create() {
  ShaderSet shaders("res/quad-terrain/common.glsl",
                    "res/quad-terrain/vertex.glsl",
                    "res/quad-terrain/fragment.glsl");

  shaders.m_tessellation_control = "res/quad-terrain/tess-control.glsl";
  shaders.m_tessellation_evaluation = "res/quad-terrain/tess-eval.glsl";

  auto program = Program::fromShaders(shaders);
  if (!program) {
    ERROR("Failed to create quad terrain program");
    return nullptr;
  }
  sf::Image heightMapImporter;
  sf::Image coverImporter;

  if (!heightMapImporter.loadFromFile("res/terrain/heightmap.png")) {
    ERROR("Error loading heightmap");
    return nullptr;
  }

  if (!coverImporter.loadFromFile("res/terrain/cover.png")) {
    ERROR("Error loading cover");
    return nullptr;
  }

  GLuint cover = textureFromImage(coverImporter);
  GLuint heightmap = textureFromImage(heightMapImporter);

  return std::unique_ptr<QuadTerrain>(
      new QuadTerrain(std::move(program), cover, heightmap, makePlane(100, 100)));
}

QuadTerrain::~QuadTerrain() {
  glDeleteTextures(1, &m_coverTexture);
  glDeleteTextures(1, &m_heightmapTexture);

  glDeleteBuffers(1, &m_vbo);
  glDeleteVertexArrays(1, &m_vao);
}
