#pragma once

#include <SFML/Graphics.hpp>

class Scene;

class InputUtils final {
public:
  static void handleText(Scene& a_scene, sf::Event::TextEvent& a_event, bool&);
};
