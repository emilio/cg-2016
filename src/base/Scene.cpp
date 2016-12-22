#include "base/Scene.h"
#include "base/Skybox.h"
#include "base/gl.h"
#include "base/Skybox.h"
#include "base/Terrain.h"
#include "base/DynTerrain.h"
#include "base/BezierTerrain.h"

#include "geometry/DrawContext.h"

#include "glm/matrix.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

void SceneUniforms::findInProgram(GLuint a_programId) {
#define FIND(u) u = glGetUniformLocation(a_programId, #u);

  FIND(uViewProjection)
  FIND(uModel)
  FIND(uUsesTexture)
  FIND(uTexture)
  FIND(uShadowMap)
  FIND(uMaterial.m_diffuse)
  FIND(uMaterial.m_specular)
  FIND(uMaterial.m_ambient)
  FIND(uMaterial.m_emissive)
  FIND(uMaterial.m_shininess)
  FIND(uMaterial.m_shininess_percent)
  FIND(uFrame)
  FIND(uAmbientLightColor)
  FIND(uAmbientLightStrength)
  FIND(uLightSourcePosition)
  FIND(uLightSourceColor)
  FIND(uCameraPosition)
  FIND(uDrawingForShadowMap)
  FIND(uShadowMapViewProjection)

#undef FIND
}

Scene::Scene(ShaderSet a_shaderSet, TerrainMode a_terrainMode)
  : m_shaderSet(std::move(a_shaderSet))
  , m_frameCount(0)
  , m_skybox(Skybox::create())
  , m_tessLevel(1)
  , m_shouldPaint(true)
  , m_cameraPosition(0, 0, 5)
  , m_dimensions(SKYBOX_WIDTH, SKYBOX_HEIGHT, SKYBOX_DEPTH)
  , m_locked(true)
  , m_wireframeMode(false)
  , m_lodTessellationEnabled(true) {
  assert(m_skybox);

  reloadShaders();
  assert(m_mainProgram);

  switch (a_terrainMode) {
    case Terrain: {
      auto terrain = Terrain::create();
      if (terrain)
        addObject(std::move(terrain));
      break;
    }
    case BezierTerrain:
      m_terrain = BezierTerrain::create();
      assert(m_terrain);
      break;
    case DynTerrain:
      m_terrain = DynTerrain::create();
      assert(m_terrain);
      break;
    case NoTerrain:
      break;
  }

  if (m_terrain && m_terrain->wantsShadowMap()) {
    m_shadowMapFramebufferAndTexture.set(std::make_pair(0, 0));
    glGenFramebuffers(1, &m_shadowMapFramebufferAndTexture->first);
    glGenTextures(1, &m_shadowMapFramebufferAndTexture->second);
    glBindTexture(GL_TEXTURE_2D, m_shadowMapFramebufferAndTexture->second);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH,
                 SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFramebufferAndTexture->first);
    glReadBuffer(GL_NONE);
    glDrawBuffer(GL_NONE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           m_shadowMapFramebufferAndTexture->second, 0);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
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

void Scene::setLightSourcePosition(const glm::vec3& position) {
  m_lightSourcePosition = position;

  m_shadowMapView =
      glm::lookAt(m_lightSourcePosition, glm::vec3(0.0, 0.0, 0.0), Y_AXIS);
  if (m_terrain)
    m_terrain->recomputeShadowMap(*this);
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

void Scene::resize(uint32_t width, uint32_t height) {
  if (m_size.x == width && m_size.y == height)
    return;

  m_pendingResize.set(width, height);
}

void Scene::setupProjection(float width, float height) {
  const float NEAR = 0.1f;
  const float FAR = 100.0f;
  const float FIELD_OF_VIEW = glm::radians(44.0f);

  const float aspectRatio = width / height;

  LOG("Projecting (%fx%f), aspect ratio: %f", width, height, aspectRatio);
  assertLocked();
  m_projection = glm::perspective(FIELD_OF_VIEW, aspectRatio, NEAR, FAR);
  const float SHADOW_PROJ = TERRAIN_DIMENSIONS / 2;
  m_shadowMapProjection = glm::ortho<float>(
      -SHADOW_PROJ, SHADOW_PROJ, -SHADOW_PROJ, SHADOW_PROJ, NEAR, FAR);
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
    m_size = *m_pendingResize;
    glViewport(0, 0, m_size.x, m_size.y);
    setupProjection(m_size.x, m_size.y);
    m_pendingResize.clear();

    if (m_terrain)
      m_terrain->recomputeShadowMap(*this);
  }

#if 0
  if (m_shadowMapFramebufferAndTexture) {
    Optional<GLuint> terrainShadowMap =
        m_terrain ? m_terrain->shadowMapFBO() : None;

    assert(terrainShadowMap);
    // We copy the cached terrain FBO.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, *terrainShadowMap);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
                      m_shadowMapFramebufferAndTexture->first);
    glBlitFramebuffer(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT, 0, 0, SHADOW_WIDTH,
                      SHADOW_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    drawObjects(true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
#endif
  if (m_shadowMapFramebufferAndTexture) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFramebufferAndTexture->first);
    glClear(GL_DEPTH_BUFFER_BIT);
    drawObjects(true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  glPolygonMode(GL_FRONT_AND_BACK, m_wireframeMode ? GL_LINE : GL_FILL);

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

  if (m_terrain)
    m_terrain->drawTerrain(*this);

  drawObjects(false);
}

void Scene::drawObjects(bool forShadowMap) {
  glm::mat4 viewProjection =
      forShadowMap ? shadowMapViewProjection() : this->viewProjection();
  const glm::vec3& cameraPos =
      forShadowMap ? lightSourcePosition() : cameraPosition();

  glCullFace(forShadowMap ? GL_FRONT : GL_BACK);

  m_mainProgram->use();

  // FIXME(emilio): We can avoid most of the traffic here the second time, but
  // oh well.
  glUniform1i(m_uniforms.uDrawingForShadowMap, forShadowMap);

  glUniform1f(m_uniforms.uFrame,
              glm::radians(static_cast<float>(m_frameCount++)));
  glUniformMatrix4fv(m_uniforms.uViewProjection, 1, GL_FALSE,
                     glm::value_ptr(viewProjection));

  glUniform3fv(m_uniforms.uLightSourcePosition, 1,
               glm::value_ptr(lightSourcePosition()));

  glm::vec3 lightColor = glm::vec3(1.0, 1.0, 1.0);
  glUniform3fv(m_uniforms.uLightSourceColor, 1, glm::value_ptr(lightColor));

  glm::vec3 ambientColor = glm::vec3(1.0, 1.0, 1.0);
  glUniform3fv(m_uniforms.uAmbientLightColor, 1, glm::value_ptr(ambientColor));

  // FIXME: Not hardcode this? Maybe make it depend on the frame, or the time...
  float ambientStrength = 0.5;
  glUniform1f(m_uniforms.uAmbientLightStrength, ambientStrength);

  glUniform3fv(m_uniforms.uCameraPosition, 1, glm::value_ptr(cameraPos));

  // Use slot number 1 for the shadow map.
  if (!forShadowMap && m_shadowMapFramebufferAndTexture) {
    glUniformMatrix4fv(m_uniforms.uShadowMapViewProjection, 1, GL_FALSE,
                       glm::value_ptr(shadowMapViewProjection()));
    glUniform1i(m_uniforms.uShadowMap, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_shadowMapFramebufferAndTexture->second);
  }

  LOG("camera: (%f %f %f)", m_cameraPosition[0], m_cameraPosition[1],
      m_cameraPosition[2]);
  LOG_MATRIX("projection", m_projection);
  LOG_MATRIX("view", m_view);
  LOG_MATRIX("viewProjection", viewProjection);

  DrawContext context(*m_mainProgram,
                      DrawContext::Uniforms{
                          m_uniforms.uModel, m_uniforms.uUsesTexture,
                          m_uniforms.uTexture, m_uniforms.uMaterial,
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

float Scene::terrainHeightAt(float x, float y) {
  assertLocked();
  assert(m_terrain);
  return m_terrain->heightAt(x, y);
}
