#include "base/Scene.h"
#include "base/Skybox.h"
#include "base/gl.h"
#include "base/Skybox.h"

#include "glm/matrix.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

Scene::Scene()
  : m_frameCount(0)
  , m_skybox(Skybox::create())
  , m_u_frame(0)
  , m_u_transform(0)
  , m_shouldPaint(true)
  , m_cameraPosition(0, 0, 3)
  , m_dimension(SKYBOX_WIDTH, SKYBOX_HEIGHT, SKYBOX_DEPTH)
  , m_locked(true)
#ifdef DEBUG
  , m_wireframeMode(false)
#endif
{
  assert(m_skybox);
  reloadShaders();
  assert(m_mainProgram);
  LOG("New program: %u", m_mainProgram->id());
  m_mainProgram->use();
  m_locked = false;
}

Scene::~Scene() {
  glUseProgram(0);
}

void Scene::setupUniforms() {
  AutoGLErrorChecker checker;
  assertLocked();

  m_u_frame = glGetUniformLocation(m_mainProgram->id(), "uFrame");
  m_u_transform = glGetUniformLocation(m_mainProgram->id(), "uTransform");
  // assert(m_u_frame != -1);
  // assert(m_u_transform != -1);
  recomputeView();
}

void Scene::addObject(std::unique_ptr<Node>&& a_object) {
  assertLocked();
  m_objects.push_back(std::move(a_object));
}

void Scene::recomputeView() {
  assertLocked();
  // m_view = glm::translate(glm::mat4(), -m_cameraPosition);
  m_view = glm::lookAt(m_cameraPosition, glm::vec3(0, 0, 0), Y_AXIS);
}

void Scene::setupProjection(float width, float height) {
  const float NEAR = 0.1f;
  const float FAR = 100.0f;
  const float FIELD_OF_VIEW = glm::radians(44.0f);

  const float aspectRatio = width / height;

  LOG("Projecting (%fx%f), aspect ratio: %f", width, height, aspectRatio);
  assertLocked();
  m_projection = glm::perspective(FIELD_OF_VIEW, aspectRatio, NEAR, FAR);
}

void Scene::reloadShaders() {
  assertLocked();
  m_mainProgram =
      Program::fromShaderFiles("res/vertex.glsl", "res/fragment.glsl");
  setupUniforms();
}

void Scene::toggleWireframeMode() {
  assertLocked();
  m_wireframeMode = !m_wireframeMode;
}

#define LOG_MATRIX(name_, var_)                                                \
  LOG(name_ ":\n %f %f %f %f\n %f %f %f %f\n %f %f %f %f\n %f %f %f %f",       \
      var_[0][0], var_[0][1], var_[0][2], var_[0][3], var_[1][0], var_[1][1],  \
      var_[1][2], var_[1][3], var_[2][0], var_[2][1], var_[2][2], var_[2][3],  \
      var_[3][0], var_[3][1], var_[3][2], var_[3][3]);

void Scene::draw() {
  LOG("DisplayScene");
  AutoGLErrorChecker checker;
  assertLocked();

  m_mainProgram->use();
  glUniform1f(m_u_frame, glm::radians(static_cast<float>(m_frameCount++)));

  // So as far as I know, m_transform ~== m_model, right?
  // FIXME: rename uTransform, to uMVP, or similar.

  glm::mat4 viewProjection = m_projection * m_view;

  LOG("camera: (%f %f %f)", m_cameraPosition[0], m_cameraPosition[1],
      m_cameraPosition[2]);
  LOG_MATRIX("projection", m_projection);
  LOG_MATRIX("view", m_view);
  LOG_MATRIX("viewProjection", viewProjection);

#ifdef DEBUG
  glPolygonMode(GL_FRONT_AND_BACK, m_wireframeMode ? GL_LINE : GL_FILL);
#endif

  glClearColor(1, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  DrawContext context(m_u_transform, viewProjection);
  m_skybox->draw(context);

  size_t i = 0;
  for (auto& object : m_objects) {
    assert(object);

    if (i++ % 2 == 0) {
      object->rotateY(glm::radians(2.5f));
    } else {
      object->rotateX(glm::radians(4.0f));
    }

    LOG("Object %zu", i);
    object->draw(context);
  }
}

void Scene::stopPainting() {
  assertLocked();
  m_shouldPaint = false;
}

bool Scene::shouldPaint() {
  assertLocked();
  return m_shouldPaint;
}
