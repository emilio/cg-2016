#pragma once

#include <SFML/Graphics.hpp>

class Scene;

class InputUtils final {
public:
  static void handleText(Scene& a_scene, const sf::Event::TextEntered& a_event, bool&);
};
