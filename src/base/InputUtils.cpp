#include "base/InputUtils.h"
#include "base/Scene.h"

#include <SFML/Graphics.hpp>

void InputUtils::handleText(Scene& a_scene, sf::Event::TextEvent& a_event, bool&) {
  switch (a_event.unicode) {
    case 'r':
      a_scene.reloadShaders();
      return;
    case 'w':
      a_scene.toggleWireframeMode();
      return;
  }
}
