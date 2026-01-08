#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#include "base/DebuggingUtils.h"
#include "base/InputUtils.h"
#include "base/Logging.h"
#include "base/Platform.h"
#include "base/Program.h"
#include "base/Scene.h"
#include "base/gl.h"

#include "geometry/Mesh.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/matrix.hpp"

#include <SFML/Graphics.hpp>

void handleKey(Scene& a_scene,
               const sf::Event::KeyPressed& a_event,
               bool& a_shouldClose) {
  constexpr const float CAMERA_ROTATION = glm::radians(3.f);

  switch (a_event.code) {
    case sf::Keyboard::Key::Escape:
      a_shouldClose = true;
      return;
    case sf::Keyboard::Key::Up:
    case sf::Keyboard::Key::Down: {
      float multiplier = a_event.code == sf::Keyboard::Key::Down ? 1.0 : -1.0;

      // Get closer in the direction to the origin.
      glm::vec3 direction =
          glm::normalize(a_scene.cameraPosition() - glm::vec3(0, 0, 0));

      if (glm::any(glm::isnan(direction)))
        direction = glm::vec3(0.f, 0.f, 1.0);

      a_scene.setCameraPosition(a_scene.cameraPosition() +
                                multiplier * direction * 0.5f);
      break;
    }
    case sf::Keyboard::Key::Left:
    case sf::Keyboard::Key::Right: {
      float multiplier = a_event.code == sf::Keyboard::Key::Right ? 1.0 : -1.0;
      // FIXME: This is probably slow-ish, and pretty crappy, but...
      glm::mat4 mat =
          glm::rotate(glm::mat4(), multiplier * CAMERA_ROTATION, Y_AXIS);
      a_scene.setCameraPosition(mat * glm::vec4(a_scene.cameraPosition(), 1.0));

      break;
    }
    default:
      LOG("Unhandled special key %d", int(a_event.code));
      return;
  }

  a_scene.recomputeView();
}

// The renderer loop, executed in a second thread.
void renderer(std::shared_ptr<sf::Window> window,
              std::condition_variable* condvar,
              std::shared_ptr<Scene>* out_scene) {
  (void)window->setActive(true);

  // Basic debugging setup.
  DebuggingUtils::dumpRenderingInfo();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  glEnable(GL_CULL_FACE);

  ShaderSet shaders("res/common.glsl", "res/vertex.glsl", "res/fragment.glsl");
  shaders.m_geometry = "res/geometry.glsl";
  shaders.m_tessellation_control = "res/tess-control.glsl";
  shaders.m_tessellation_evaluation = "res/tess-eval.glsl";
  auto scene = std::make_shared<Scene>(shaders, Scene::DynTerrain);
  *out_scene = scene;

  {
    AutoSceneLocker lock(*scene);

    auto size = window->getSize();
    scene->resize(size.x, size.y);
    scene->setLightSourcePosition(glm::vec3(0.0f, 10.0f, 5.0f));

    // auto suzanne = Mesh::fromFile("res/models/AirbusA310.obj");
    // auto suzanne = Mesh::fromFile("res/models/QuestionBlock.obj");
    // auto suzanne = Mesh::fromFile("res/models/helicopter/uh60.obj");
    // auto suzanne = Mesh::fromFile("res/models/suzanne.obj");
    // suzanne->scale(0.5f);
    // scene->addObject(std::move(suzanne));
  }

  condvar->notify_all();

  while (true) {
    {
      AutoSceneLocker lock(*scene);

      if (!scene->shouldPaint())
        break;

      scene->draw();
    }
    window->display();
  }
}

int main(int, const char**) {
  const size_t INITIAL_WIDTH = 2000;
  const size_t INITIAL_HEIGHT = 2000;
  const char* TITLE = "OpenGL";  // FIXME: Think of a good title

  Platform::init();

  // Request OpenGL 3.1
  sf::ContextSettings settings;
  settings.majorVersion = 4;
  settings.minorVersion = 0;
  settings.depthBits = 32;
  settings.attributeFlags = sf::ContextSettings::Core;

  sf::VideoMode vm(sf::Vector2u(INITIAL_WIDTH, INITIAL_HEIGHT));
  auto window = std::make_shared<sf::Window>(vm, TITLE, sf::Style::Default,
                                             sf::State::Windowed, settings);
  window->setVerticalSyncEnabled(true);

  std::shared_ptr<Scene> scene(nullptr);
  (void)window->setActive(false);

  std::unique_ptr<std::thread> rendererThread;

  {
    std::condition_variable condvar;
    std::mutex condvar_mutex;
    std::unique_lock<std::mutex> lock(condvar_mutex);
    rendererThread =
        std::make_unique<std::thread>(renderer, window, &condvar, &scene);
    condvar.wait(lock);
    assert(scene);
  }

  bool shouldClose = false;
  while (window->isOpen() && !shouldClose) {
    window->handleEvents([&](const sf::Event::Closed&) { shouldClose = true; },
                         [&](const sf::Event::Resized& event) {
                           AutoSceneLocker lock(*scene);
                           scene->setPendingResize(event.size.x, event.size.y);
                         },
                         [&](const sf::Event::KeyPressed& event) {
                           AutoSceneLocker lock(*scene);
                           handleKey(*scene, event, shouldClose);
                         },
                         [&](const sf::Event::TextEntered& event) {
                           AutoSceneLocker lock(*scene);
                           InputUtils::handleText(*scene, event, shouldClose);
                         });
  }

  {
    AutoSceneLocker lock(*scene);
    scene->stopPainting();
  }

  rendererThread->join();
  Platform::shutDown();
  return 0;
}
