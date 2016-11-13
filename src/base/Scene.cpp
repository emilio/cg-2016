#include "base/Scene.h"
#include "base/Skybox.h"
#include "base/gl.h"
#include "base/Skybox.h"
#include "base/Terrain.h"

#include "geometry/DrawContext.h"

#include "glm/matrix.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

void SceneUniforms::findInProgram(GLuint a_programId) {
#define FIND(u) u = glGetUniformLocation(a_programId, #u);

  FIND(uViewProjection)
  FIND(uModel)
  FIND(uColor)
  FIND(uFrame)
  FIND(uAmbientLightColor)
  FIND(uAmbientLightStrength)
  FIND(uLightSourcePosition)
  FIND(uLightSourceColor)
  FIND(uCameraPosition)

#undef FIND
}

Scene::Scene(ShaderSet a_shaderSet, TerrainMode a_terrainMode)
  : m_shaderSet(std::move(a_shaderSet))
  , m_frameCount(0)
  , m_skybox(Skybox::create())
  , m_shouldPaint(true)
  , m_cameraPosition(0, 0, 5)
  , m_dimensions(SKYBOX_WIDTH, SKYBOX_HEIGHT, SKYBOX_DEPTH)
  , m_locked(true)
  , m_wireframeMode(false) {
  assert(m_skybox);

  reloadShaders();
  assert(m_mainProgram);

  if (a_terrainMode == Terrain) {
    auto terrain = Terrain::create();
    if (terrain)
      addObject(std::move(terrain));
  }

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

  m_uniforms.findInProgram(m_mainProgram->id());

  // assert(m_u_frame != -1);
  // assert(m_u_transform != -1);
  recomputeView();
}

void Scene::addObject(std::unique_ptr<Node>&& a_object) {
  assertLocked();
  m_objects.push_back(std::move(a_object));
}

void Scene::recomputeView() {
  recomputeView(glm::vec3(0, 0, 0), Y_AXIS);
}

void Scene::recomputeView(const glm::vec3& lookingAt, const glm::vec3& up) {
  assertLocked();
  m_view = glm::lookAt(m_cameraPosition, lookingAt, up);
  // NB: The view projection matrix from the skybox doesn't contain the camera
  // translation.
  m_skyboxView =
      glm::lookAt(glm::vec3(0, 0, 0), lookingAt - m_cameraPosition, up);
}

void Scene::setupProjection(float width, float height) {
  const float NEAR = 0.1f;
  const float FAR = 1000.0f;
  const float FIELD_OF_VIEW = glm::radians(44.0f);

  const float aspectRatio = width / height;

  LOG("Projecting (%fx%f), aspect ratio: %f", width, height, aspectRatio);
  assertLocked();
  m_projection = glm::perspective(FIELD_OF_VIEW, aspectRatio, NEAR, FAR);
}

void Scene::reloadShaders() {
  assertLocked();
  m_mainProgram = Program::fromShaders(m_shaderSet);
  setupUniforms();
}

void Scene::toggleWireframeMode() {
  assertLocked();
  m_wireframeMode = !m_wireframeMode;
}

void Scene::setPendingResize(uint32_t width, uint32_t height) {
  m_pendingResize.set(width, height);
}

void Scene::setPhysicsCallback(PhysicsCallback callback) {
  m_physicsCallback.set(callback);
}

#undef LOG
#define LOG(...)
void Scene::draw() {
  LOG("DisplayScene");
  AutoGLErrorChecker checker;
  assertLocked();

  if (m_pendingResize) {
    auto& resize = *m_pendingResize;
    glViewport(0, 0, resize.x, resize.y);
    setupProjection(resize.x, resize.y);
    m_pendingResize.clear();
  }

  // TODO: Probably we may want to run more/less physics than once per frame,
  // but this is ok for now.
  if (m_physicsCallback)
    (*m_physicsCallback)(*this);

  assert(!m_pendingResize);

  glClearColor(1, 1, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // First draw the Skybox.
  {
    glm::mat4 viewProjection = m_projection * m_skyboxView;
    //
    // TODO: We can make it faster if we draw the skybox last using the depth
    // buffer. Not a big deal for now I guess.
    m_skybox->draw(viewProjection);
  }

  m_mainProgram->use();

  glm::mat4 viewProjection = m_projection * m_view;

  glUniform1f(m_uniforms.uFrame,
              glm::radians(static_cast<float>(m_frameCount++)));
  glUniformMatrix4fv(m_uniforms.uViewProjection, 1, GL_FALSE,
                     glm::value_ptr(viewProjection));

  // TODO: Implement some controls for light position, but meanwhile... why not?
  // glm::vec3 lightPosition = glm::vec3(0., 0., 3.);
  glm::vec3 lightPosition = m_cameraPosition;
  glUniform3fv(m_uniforms.uLightSourcePosition, 1,
               glm::value_ptr(lightPosition));

  glm::vec3 lightColor = glm::vec3(1.0, 0.7, 0.7);
  glUniform3fv(m_uniforms.uLightSourceColor, 1, glm::value_ptr(lightColor));

  glm::vec3 ambientColor = glm::vec3(1.0, 1.0, 1.0);
  glUniform3fv(m_uniforms.uAmbientLightColor, 1, glm::value_ptr(ambientColor));

  // FIXME: Not hardcode this? Maybe make it depend on the frame, or the time...
  float ambientStrength = 0.5f;
  glUniform1f(m_uniforms.uAmbientLightStrength, ambientStrength);

  glUniform3fv(m_uniforms.uCameraPosition, 1, glm::value_ptr(m_cameraPosition));

  LOG("camera: (%f %f %f)", m_cameraPosition[0], m_cameraPosition[1],
      m_cameraPosition[2]);
  LOG_MATRIX("projection", m_projection);
  LOG_MATRIX("view", m_view);
  LOG_MATRIX("viewProjection", viewProjection);

  glPolygonMode(GL_FRONT_AND_BACK, m_wireframeMode ? GL_LINE : GL_FILL);

  DrawContext context(*m_mainProgram,
                      DrawContext::Uniforms{
                          m_uniforms.uModel, m_uniforms.uColor,
                      },
                      glm::mat4());
  // size_t i = 0;
  for (auto& object : m_objects) {
    assert(object);

    // if (i++ % 2 == 0)
    //   object->rotateY(glm::radians(2.5f));
    // else
    //   object->rotateX(glm::radians(4.0f));

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
