#include <cstddef>
#include <cstdio>
#include <cassert>
#include <memory>
#include <iostream>
#include <mutex>
#include <thread>

#include "base/gl.h"
#include "base/Logging.h"
#include "base/Program.h"
#include "base/Scene.h"

#include "geometry/Mesh.h"

#include "glm/matrix.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <SFML/Graphics.hpp>


int ROOT_WINDOW = -1;

void handleText(Scene& a_scene, sf::Event::TextEvent& a_event, bool&) {
  switch (a_event.unicode) {
#ifdef DEBUG
    case 'r':
      a_scene.reloadShaders();
      return;
    case 'w':
      a_scene.toggleWireframeMode();
      return;
#endif
  }
}

void handleKey(Scene& a_scene,
               sf::Event::KeyEvent& a_event,
               bool& a_shouldClose) {
  constexpr const float CAMERA_MOVEMENT = 0.05f;

  float multiplier = 1.0;

  if (a_event.code == sf::Keyboard::Up || a_event.code == sf::Keyboard::Left) {
    multiplier = -1.0f;
  }

  float movement = multiplier * CAMERA_MOVEMENT;

  switch (a_event.code) {
    case sf::Keyboard::Escape:
      a_shouldClose = true;
      return;
    case sf::Keyboard::Up:
    case sf::Keyboard::Down:
      LOG("Up/Down");
      a_scene.m_cameraPosition[2] += movement;
      break;
    case sf::Keyboard::Left:
    case sf::Keyboard::Right:
      LOG("Left/Right");
      a_scene.m_cameraPosition[0] += movement;
      break;
    default:
      LOG("Unhandled special key %d", a_event.code);
      return;
  }

  a_scene.recomputeView();
}

static void
dumpRenderingInfo() {
  AutoGLErrorChecker checker;
  GLint paramValue;

  LOG("Renderer: %s", glGetString(GL_RENDERER));
  LOG("OpenGL version: %s", glGetString(GL_VERSION));

#define LOG_PARAM(name_)                                                       \
  glGetIntegerv(name_, &paramValue);                                           \
  LOG(#name_ ": %d", paramValue);                                              \

  LOG_PARAM(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
  LOG_PARAM(GL_MAX_CUBE_MAP_TEXTURE_SIZE)
  LOG_PARAM(GL_MAX_DRAW_BUFFERS)
  LOG_PARAM(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS)
  LOG_PARAM(GL_MAX_TEXTURE_IMAGE_UNITS)
  LOG_PARAM(GL_MAX_TEXTURE_SIZE)
  LOG_PARAM(GL_MAX_VARYING_FLOATS)
  LOG_PARAM(GL_MAX_VERTEX_ATTRIBS)
  LOG_PARAM(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS)
  LOG_PARAM(GL_MAX_VERTEX_UNIFORM_COMPONENTS)
  LOG_PARAM(GL_MAX_VIEWPORT_DIMS)
  LOG_PARAM(GL_STEREO)
}

// The renderer loop, executed in a second thread.
void renderer(std::shared_ptr<sf::Window> window,
              std::shared_ptr<Scene>* out_scene) {
  window->setActive(true);

  // Basic debugging setup.
  dumpRenderingInfo();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glDepthMask(GL_TRUE);

  auto scene = std::make_shared<Scene>();
  *out_scene = scene;

  {
    AutoSceneLocker lock(*scene);

    auto size = window->getSize();
    scene->setupProjection(size.x, size.y);

    auto firstCube = Mesh::fromFile("res/models/cube.obj");
    assert(firstCube);
    firstCube->translateX(-0.2);
    scene->addObject(std::move(firstCube));

    auto secondCube = Mesh::fromFile("res/models/cube.obj");
    assert(secondCube);
    secondCube->translateX(0.2);
    scene->addObject(std::move(secondCube));

    scene->addObject(Mesh::fromFile("res/models/suzanne.obj"));
  }

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
  const char* TITLE = "OpenGL"; // FIXME: Think of a good title

  // Request OpenGL 3.1
  sf::ContextSettings settings;
  settings.majorVersion = 3;
  settings.minorVersion = 1;
  settings.attributeFlags = sf::ContextSettings::Core;
#ifdef DEBUG
  settings.attributeFlags |= sf::ContextSettings::Debug;
#endif

  sf::VideoMode vm(INITIAL_WIDTH, INITIAL_HEIGHT);
  auto window =
    std::make_shared<sf::Window>(vm, TITLE, sf::Style::Default, settings);
  window->setVerticalSyncEnabled(true);

  std::shared_ptr<Scene> scene(nullptr);
  window->setActive(false);

  std::thread rendererThread(renderer, window, &scene);

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
        assert(!"Got resize!");
        // FIXME: Send this event to the renderer somehow!
        // glViewport(0, 0, event.size.width, event.size.height);
        // scene->setupProjection(event.size.width, event.size.height);
        break;
      case sf::Event::KeyPressed:
        handleKey(*scene, event.key, shouldClose);
        break;
      case sf::Event::TextEntered:
        handleText(*scene, event.text, shouldClose);
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

  rendererThread.join();
  return 0;
}
