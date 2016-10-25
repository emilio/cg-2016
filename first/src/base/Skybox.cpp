#include "base/Skybox.h"
#include <vector>

std::unique_ptr<Skybox> Skybox::create() {
  std::unique_ptr<Mesh> one_cube = Mesh::fromFile("res/models/cube.obj");

  if (!one_cube)
    return nullptr;

  one_cube->scale(glm::vec3(SKYBOX_WIDTH, SKYBOX_HEIGHT, SKYBOX_DEPTH));

  return std::unique_ptr<Skybox>(new Skybox(std::move(*one_cube)));
}
