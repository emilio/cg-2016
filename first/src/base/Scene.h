#ifndef Scene_h
#define Scene_h

#include <cassert>
#include <memory>
#include <mutex>
#include "geometry/Mesh.h"
#include "base/Program.h"

const glm::vec3 X_AXIS = glm::vec3(1, 0, 0);
const glm::vec3 Y_AXIS = glm::vec3(0, 1, 0);
const glm::vec3 Z_AXIS = glm::vec3(0, 0, 1);

class AutoSceneLocker;
class Skybox;

class Scene {
  friend class AutoSceneLocker;

public:
  Scene();
  ~Scene();

private:
  std::unique_ptr<Program> m_mainProgram;
  std::vector<std::unique_ptr<Mesh>> m_objects;
  GLuint m_frameCount;
  std::unique_ptr<Skybox> m_skybox;
  GLint m_u_frame;
  GLint m_u_transform;
  glm::mat4 m_projection;
  glm::mat4 m_view;
  bool m_shouldPaint;

public:  // FIXME: too lazy.
  glm::vec3 m_cameraPosition;

private:
  glm::vec3 m_dimension;
  std::mutex m_lock;
  bool m_locked;

#ifdef DEBUG
  bool m_wireframeMode;
#endif

  void assertLocked() {
    assert(m_locked);
  }

  void setupUniforms();

public:
  void addObject(std::unique_ptr<Mesh>&& a_object);
  void recomputeView();

  void setupProjection(float width, float height);
  void reloadShaders();

  void toggleWireframeMode();
  void draw();
  bool shouldPaint();
  void stopPainting();
};

class AutoSceneLocker {
public:
  explicit AutoSceneLocker(Scene& a_scene) : m_scene(a_scene) {
    m_scene.m_lock.lock();
#ifdef DEBUG
    m_scene.m_locked = true;
#endif
  }

  ~AutoSceneLocker() {
#ifdef DEBUG
    m_scene.m_locked = false;
#endif
    m_scene.m_lock.unlock();
  }

private:
  Scene& m_scene;
};

#endif
