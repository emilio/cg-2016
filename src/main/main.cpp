#include <cstddef>
#include <cstdio>
#include <cassert>
#include <memory>
#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "base/gl.h"
#include "base/DebuggingUtils.h"
#include "base/InputUtils.h"
#include "base/Logging.h"
#include "base/Platform.h"
#include "base/Program.h"
#include "base/Scene.h"
#include "base/Plane.h"
#include "main/PhysicsState.h"

#include "geometry/Mesh.h"

#include "glm/matrix.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <SFML/Graphics.hpp>

void handleKey(Scene& scene,
               sf::Event::KeyEvent& a_event,
               PhysicsState& a_state,
               bool& a_shouldClose) {
  constexpr const float PLANE_ROTATION = glm::radians(1.0f);
  constexpr const float SPEED_DELTA = 0.2f;

  switch (a_event.code) {
    case sf::Keyboard::Escape:
      a_shouldClose = true;
      break;
    case sf::Keyboard::PageUp:
      a_state.speedUp(SPEED_DELTA);
      break;
    case sf::Keyboard::PageDown:
      a_state.speedUp(-SPEED_DELTA);
      break;
    case sf::Keyboard::Up:
      a_state.rotate(scene, PhysicsState::Top, PLANE_ROTATION);
      break;
    case sf::Keyboard::Down:
      a_state.rotate(scene, PhysicsState::Down, PLANE_ROTATION);
      break;
    case sf::Keyboard::Right:
      a_state.rotate(scene, PhysicsState::Right, PLANE_ROTATION);
      break;
    case sf::Keyboard::Left:
      a_state.rotate(scene, PhysicsState::Left, PLANE_ROTATION);
      break;
    default:
      LOG("Unhandled special key %d", a_event.code);
      return;
  }
}

// The renderer loop, executed in a second thread.
void renderer(std::shared_ptr<sf::Window> window,
              std::condition_variable* condvar,
              std::shared_ptr<Scene>* out_scene,
              Plane** out_plane) {
  window->setActive(true);

  // Basic debugging setup.
  DebuggingUtils::dumpRenderingInfo();

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  ShaderSet shaders("res/common.glsl", "res/vertex.glsl", "res/fragment.glsl");
  // auto scene = std::make_shared<Scene>(std::move(shaders),
  // Scene::DynTerrain);
  auto scene =
      std::make_shared<Scene>(std::move(shaders), Scene::BezierTerrain);
  *out_scene = scene;

  {
    AutoSceneLocker lock(*scene);

    auto size = window->getSize();
    scene->resize(size.x, size.y);

    auto plane = Plane::create();
    // plane->setColor(glm::vec3(0.0, 1.0, 0.0));

    // Yup, for now our plane is going to be a cube, awesome, isn't it?
    *out_plane = plane.get();
    scene->addObject(std::move(plane));

    // auto secondCube = Mesh::fromFile("res/models/cube.obj");
    // secondCube->translate(glm::vec3(3.0, 0.0, 4.0));
    // secondCube->setColor(glm::vec3(1.0, 1.0, 1.0));
    // scene->addObject(std::move(secondCube));

    // auto suzanne = Mesh::fromFile("res/models/suzanne.obj");
    // suzanne->scale(glm::vec3(0.5, 0.5, 0.5));
    // scene->addObject(std::move(suzanne));
    // scene->addObject(Mesh::fromFile("res/models/QuestionBlock.obj"));
    // scene->addObject(Mesh::fromFile("res/models/Airbus A310.obj"));
  }

  // Scene is ready, ready to handle events!
  condvar->notify_all();

  const size_t HALF_A_FRAME_MS = 62;
  while (true) {
    {
      AutoSceneLocker lock(*scene);

      if (!scene->shouldPaint())
        break;

      scene->draw();
    }
    window->display();
    std::this_thread::sleep_for(std::chrono::milliseconds(HALF_A_FRAME_MS));
  }
}

int main(int, char**) {
  const size_t INITIAL_WIDTH = 2000;
  const size_t INITIAL_HEIGHT = 2000;
  const char* TITLE = "OpenGL";  // FIXME: Think of a good title

  Platform::init();

  // TODO: Request OpenGL 3.1, should be trivial, but...
  sf::ContextSettings settings;
  settings.majorVersion = 4;
  settings.minorVersion = 0;
  settings.depthBits = 32;
  settings.attributeFlags = sf::ContextSettings::Core;

  sf::VideoMode vm(INITIAL_WIDTH, INITIAL_HEIGHT);
  auto window =
      std::make_shared<sf::Window>(vm, TITLE, sf::Style::Default, settings);
  window->setVerticalSyncEnabled(true);

  std::shared_ptr<Scene> scene(nullptr);

  Plane* plane;

  // Start the renderer and wait for the scene to be ready.
  std::unique_ptr<std::thread> rendererThread;
  {
    window->setActive(false);
    std::condition_variable condvar;
    std::mutex condvar_mutex;
    std::unique_lock<std::mutex> lock(condvar_mutex);
    rendererThread = std::make_unique<std::thread>(renderer, window, &condvar,
                                                   &scene, &plane);
    assert(rendererThread);
    condvar.wait(lock);
    assert(scene);
    assert(plane);
  }

  PhysicsState physicsState(*plane);
  {
    AutoSceneLocker lock(*scene);
    scene->setPhysicsCallback([&](Scene& scene) { physicsState.tick(scene); });
    scene->setLightSourcePosition(glm::vec3(10.0f, 40.0f, 10.0f));
  }

  bool shouldClose = false;
  sf::Event event;
  while (!shouldClose && window->waitEvent(event)) {
    AutoSceneLocker lock(*scene);
    switch (event.type) {
      case sf::Event::Closed:
        shouldClose = true;
        break;
      case sf::Event::Resized:
        scene->setPendingResize(event.size.width, event.size.height);
        break;
      case sf::Event::KeyPressed:
        handleKey(*scene, event.key, physicsState, shouldClose);
        break;
      case sf::Event::TextEntered:
        InputUtils::handleText(*scene, event.text, shouldClose);
        break;
      default:
        std::cerr << "[event] Unhandled " << event.type << "\n";
        break;
    }
  }

  {
    AutoSceneLocker lock(*scene);
    scene->stopPainting();
  }

  rendererThread->join();

  Platform::shutDown();
  return 0;
}
