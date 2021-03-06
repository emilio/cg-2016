#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <thread>

#include "base/gl.h"
#include "base/DebuggingUtils.h"
#include "base/InputUtils.h"
#include "base/Logging.h"
#include "base/Platform.h"
#include "base/Program.h"
#include "base/Scene.h"
#include "base/Plane.h"
#include "base/Terrain.h"
#include "main/PhysicsState.h"

#include "geometry/Mesh.h"

#include "glm/matrix.hpp"
#include "glm/gtc/type_ptr.hpp"
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
              Plane** out_plane,
              int argc,
              const char** argv) {
  window->setActive(true);

  // Basic debugging setup.
  DebuggingUtils::dumpRenderingInfo();

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  ShaderSet shaders("res/common.glsl", "res/vertex.glsl", "res/fragment.glsl");
  // auto scene = std::make_shared<Scene>(std::move(shaders),
  // Scene::DynTerrain);
  auto terrainType = (argc > 1 && !strcmp(argv[1], "--bezier")) ?
    Scene::BezierTerrain : Scene::DynTerrain;
  auto scene = std::make_shared<Scene>(std::move(shaders), terrainType);
  *out_scene = scene;

  {
    AutoSceneLocker lock(*scene);

    auto size = window->getSize();
    scene->resize(size.x, size.y);

    auto plane = Plane::create();
    // plane->setColor(glm::vec3(0.0, 1.0, 0.0));

    *out_plane = plane.get();
    scene->addObject(std::move(plane));

    const size_t kNumTrees = 20;
    std::default_random_engine generator;
    generator.seed(time(nullptr));
    std::uniform_int_distribution<size_t> distribution(0.0f,
                                                       TERRAIN_DIMENSIONS - 1);
    for (size_t i = 0; i < kNumTrees; ++i) {
      auto tree = Mesh::fromFile("res/models/tree/lowpolytree.obj");
      float x = distribution(generator);
      float y = distribution(generator);
      tree->translate(glm::vec3(x - TERRAIN_DIMENSIONS / 2,
                                scene->terrainHeightAt(x, y) + 1.5f,
                                y - TERRAIN_DIMENSIONS / 2));
      scene->addObject(std::move(tree));
    }

    // auto suzanne = Mesh::fromFile("res/models/suzanne.obj");
    // suzanne->scale(glm::vec3(0.5, 0.5, 0.5));
    // scene->addObject(std::move(suzanne));
    auto helicopter = Mesh::fromFile("res/models/helicopter/uh60.obj");
    helicopter->translate(glm::vec3(10.0, 10.0, -10.0));
    helicopter->rotate(glm::radians(270.0f), X_AXIS);
    scene->addObject(std::move(helicopter));
    // scene->addObject(Mesh::fromFile("res/models/Airbus A310.obj"));
  }

  // Scene is ready, ready to handle events!
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

int main(int argc, const char** argv) {
  const size_t INITIAL_WIDTH = 1000;
  const size_t INITIAL_HEIGHT = 1000;
  const char* TITLE = "OpenGL";  // FIXME: Think of a good title

  Platform::init();

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
                                                   &scene, &plane, argc, argv);
    assert(rendererThread);
    condvar.wait(lock);
    assert(scene);
    assert(plane);
  }

  PhysicsState physicsState(*plane);
  {
    AutoSceneLocker lock(*scene);
    scene->setPhysicsCallback([&](Scene& scene) { physicsState.tick(scene); });
    scene->setLightSourcePosition(glm::vec3(00.0f, 20.0f, 30.0f));
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
