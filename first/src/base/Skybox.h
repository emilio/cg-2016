#pragma once

#include "geometry/Mesh.h"

const size_t SKYBOX_WIDTH = 2000;
const size_t SKYBOX_HEIGHT = 2000;
const size_t SKYBOX_DEPTH = 2000;

class Skybox final : public Node {
  Skybox(std::unique_ptr<Node> a_node) {
    addChild(std::move(a_node));
  }

public:
  static std::unique_ptr<Skybox> create();
};
