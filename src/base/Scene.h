#pragma once

#include <cassert>
#include <memory>
#include <mutex>
#include <vector>
#include <string>
#include <functional>

#include "geometry/Node.h"
#include "base/Program.h"

const glm::vec3 X_AXIS = glm::vec3(1, 0, 0);
const glm::vec3 Y_AXIS = glm::vec3(0, 1, 0);
const glm::vec3 Z_AXIS = glm::vec3(0, 0, 1);
const float CAMERA_DISTANCE = 20.0f;

class AutoSceneLocker;
class Skybox;

class SceneUniforms {
  friend class Scene;

  GLint uViewProjection;
  GLint uModel;
  GLint uColor;
  GLint uFrame;
  GLint uAmbientLightColor;
  GLint uAmbientLightStrength;
  GLint uLightSourcePosition;
  GLint uLightSourceColor;
  GLint uCameraPosition;

  void findInProgram(GLuint a_programId);
};

class Scene {
  friend class AutoSceneLocker;

public:
  enum TerrainMode {
    Terrain,
    NoTerrain,
  };

  using PhysicsCallback = std::function<void(Scene&)>;

  Scene(ShaderSet, TerrainMode);
  Scene(ShaderSet a_set) : Scene(std::move(a_set), Terrain){};
  ~Scene();

private:
  ShaderSet m_shaderSet;
  std::unique_ptr<Program> m_mainProgram;
  std::vector<std::unique_ptr<Node>> m_objects;
  GLuint m_frameCount;
  std::unique_ptr<Skybox> m_skybox;
  SceneUniforms m_uniforms;
  glm::mat4 m_projection;
  glm::mat4 m_view;
  glm::mat4 m_skyboxView;
  Optional<glm::u32vec2> m_pendingResize;
  Optional<PhysicsCallback> m_physicsCallback;
  bool m_shouldPaint;

public:  // FIXME: too lazy.
  glm::vec3 m_cameraPosition;

private:
  glm::vec3 m_dimensions;
  std::mutex m_lock;
  bool m_locked;
  bool m_wireframeMode;

  void assertLocked() {
    assert(m_locked);
  }

  void setupUniforms();

public:
  void addObject(std::unique_ptr<Node>&& a_object);
  void recomputeView();
  void recomputeView(const glm::vec3& lookingAt, const glm::vec3& up);

  void setupProjection(float width, float height);
  void reloadShaders();

  void setPendingResize(uint32_t width, uint32_t height);
  void setPhysicsCallback(PhysicsCallback);

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
