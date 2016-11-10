#include <cstddef>
#include <cstdio>
#include <cassert>
#include <memory>
#include <iostream>
#include <mutex>
#include <thread>

#include "base/gl.h"
#include "base/DebuggingUtils.h"
#include "base/InputUtils.h"
#include "base/Logging.h"
#include "base/Platform.h"
#include "base/Program.h"
#include "base/Scene.h"
#include "main/PhysicsState.h"

#include "geometry/Mesh.h"

#include "glm/matrix.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <SFML/Graphics.hpp>
#include <condition_variable>

void handleKey(Scene& a_scene,
               sf::Event::KeyEvent& a_event,
               bool& a_shouldClose) {
  constexpr const float CAMERA_ROTATION = glm::radians(3.f);

  switch (a_event.code) {
    case sf::Keyboard::Escape:
      a_shouldClose = true;
      return;
    case sf::Keyboard::Up:
    case sf::Keyboard::Down:
    case sf::Keyboard::Left:
    case sf::Keyboard::Right:
      break;
    default:
      LOG("Unhandled special key %d", a_event.code);
      return;
  }

  float multiplier = 1.0;
  const glm::vec3* axis = &Y_AXIS;

  if (a_event.code == sf::Keyboard::Up || a_event.code == sf::Keyboard::Left)
    multiplier = -1.0f;

  if (a_event.code == sf::Keyboard::Up || a_event.code == sf::Keyboard::Down)
    axis = &X_AXIS;

  // FIXME: This is probably slow-ish, and pretty crappy, but...
  glm::mat4 mat = glm::rotate(glm::mat4(), multiplier * CAMERA_ROTATION, *axis);

  LOG_MATRIX("mat", mat);

  a_scene.m_cameraPosition = mat * glm::vec4(a_scene.m_cameraPosition, 1.0);
  a_scene.recomputeView();
}

// The renderer loop, executed in a second thread.
void renderer(std::shared_ptr<sf::Window> window,
              std::condition_variable* condvar,
              std::shared_ptr<Scene>* out_scene,
              Node** out_plane) {
  window->setActive(true);

  // Basic debugging setup.
  DebuggingUtils::dumpRenderingInfo();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  // glEnable(GL_CULL_FACE);

  ShaderSet shaders("res/common.glsl", "res/vertex.glsl", "res/fragment.glsl");
  auto scene = std::make_shared<Scene>(std::move(shaders));
  *out_scene = scene;

  {
    AutoSceneLocker lock(*scene);

    auto size = window->getSize();
    scene->setupProjection(size.x, size.y);

    auto firstCube = Node::fromFile("res/models/cube.obj");
    firstCube->setColor(glm::vec3(0.0, 1.0, 0.0));
    // Yup, for now our plane is going to be a cube, awesome, isn't it?
    *out_plane = firstCube.get();
    scene->addObject(std::move(firstCube));

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

  // Request OpenGL 3.1
  sf::ContextSettings settings;
  settings.majorVersion = 3;
  settings.minorVersion = 1;
  settings.depthBits = 32;
  settings.attributeFlags = sf::ContextSettings::Core;

  sf::VideoMode vm(INITIAL_WIDTH, INITIAL_HEIGHT);
  auto window =
      std::make_shared<sf::Window>(vm, TITLE, sf::Style::Default, settings);
  window->setVerticalSyncEnabled(true);

  std::shared_ptr<Scene> scene(nullptr);

  Node* plane;
  PhysicsState physicsState;

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
  }

  {
    AutoSceneLocker lock(*scene);
    scene->setPhysicsCallback([&] (Scene& scene) {
        physicsState.tick(*plane, scene);
    });
  }


  bool shouldClose = false;
  sf::Event event;
  while (!shouldClose && window->waitEvent(event)) {
    // Renderer hasn't still setup the scene.
    //
    // FIXME: This is (technically) potentially thread-unsafe, since pointer
    // stores are not guaranteed to be atomic (even though shared_ptr itself
    // is).
    //
    // But in practice this holds. If it doesn't I'd just use a lock or a
    // condvar, but it seems overkill for this specific case.
    if (!scene)
      continue;

    AutoSceneLocker lock(*scene);
    switch (event.type) {
      case sf::Event::Closed:
        shouldClose = true;
        break;
      case sf::Event::Resized:
        scene->setPendingResize(event.size.width, event.size.height);
        break;
      case sf::Event::KeyPressed:
        handleKey(*scene, event.key, shouldClose);
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
