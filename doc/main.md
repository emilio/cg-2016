---
title: Computer Graphics
subtitle: First assignment
author:
 - "Emilio Cobos Álvarez <emilio.cobos@student.um.si>, <emilio@crisal.io>"
numbersections: true
toc: true
abstract: |
 Description and documentation of the implementation of a terrain renderer.
---

# Introduction

I've implementing a third-person-view application as requested in the
assignment.

On top of it, I've implemented a bunch of other stuff, mostly for personal
interest[^side-joke].

On top of:

 * A flying object, modeled by me[^about-the-model], maneuverable using the
   keyboard over a 3d rugged landscape.

 * A skybox.

 * Other textured models.

 * Phong shading.

I've implemented the following:

 * Two different approaches to rendering the terrain with different level of
   detail depending on the distance (only GL4+, since they require at least
   geometry shaders).

 * Shadow mapping.

I'll describe them below as well.


[^side-joke]: Parts of the consequences of that is that now I'm on a rush to
finish the second assignment, oh well...

[^about-the-model]: Regarding this, I just noticed that the assignment mentions
that we should take screenshots of the modeling process. I totally forgot about
this so any screenshot if present would be on retrospective.

# Building

There are building instructions available in the `README.md` file.

# Documentation

**Disclaimer**: This is the first renderer I've ever created. Thus, the design
isn't really a design per se, but an evolution based on different iterations.

This makes the code contain a few code smells, like:

```cpp
void draw(DrawContext&) const override {
  assert(false && "not implemented! use drawTerrain instead!");
}
```

On the terrain implementations, just because they inherit from `Node` to get the
transform handling correctly.

## Base painting architecture.

The `Scene` class is the main driver and state-holder of the program.

This class mainly contains a list of objects, a terrain, and a bunch of other
state needed to paint itself, like transform matrices, camera position, light
source direction and position, etc.

The scene is shared between the input thread and the renderer thread, and thus
it needs to be locked in order to be accessed.

Just for the same reason as the terrain code smells, the API that it exposes is
not well designed, but it's relatively consistent.

Here's where the entry point to the GL-related drawing code is each frame.

In concrete, the rendering thread (under `src/main/main.cpp`), looks like this:

```cpp
  while (true) {
    {
      AutoSceneLocker lock(*scene);

      if (!scene->shouldPaint())
        break;

      scene->draw();
    }
    window->display();
  }
```

The `draw` function, depending on your GL version and on whether shadow mapping
is enabled does either a 2-pass or 1-pass drawing.

Most of the code in scene is relatively straight-forward. Most of the objects in
the scene use the main shaders (`res/fragment.glsl` and `res/vertex.glsl`), and
the terrains may or may not use a different program compared to the scene.

In any case, the scene pseudo-code (this is actually just simplified code) looks
like this:

```cpp
  processPendingEvents();
  tickApplicationPhysics();
  doFirstShadowMapPassIfNecessary();
  drawSkyBox();
  drawTerrain();
  drawObjects();
```

The `drawObjects` call sets all the necessary GL state (uniforms, culling, etc),
and creates a `DrawContext`, which is effectively a stack of state that goes
down the tree of nodes (most notably transformation matrices and material
information).

Then each `Node` in the tree draws itself, which implies drawing all the
children nodes.

If a `Node` is a `Mesh`, it contains a vertex attribute object, and a pair of
buffers that contain the actual vertex data, uv info, normals, and texturing
data. `Meshes` are the kind of objects that do the actual draw calls.

Note that there could be a _lot_ of potential optimization that could be done
that just isn't. For example, we don't cache textures, nor vertex info among the
same kind of models. Also, we draw the skybox before everything else, which is
quite expensive and could be optimized, etc.

## State and physics handling

As you may see above, the scene may run a physics callback before each frame.
This callback is held alive by the `Scene` class, and is defined like this:

```cpp
  using PhysicsCallback = std::function<void(Scene&)>;
  Optional<PhysicsCallback> m_physicsCallback;
  void setPhysicsCallback(PhysicsCallback);
```

And is used like so:

```
scene->setPhysicsCallback([&](Scene& scene) { physicsState.tick(scene); });
```

The physics state is held in the stack by the main thread, and thus is alive
until the scene stops painting.

Note that there are no data races here, since the scene is locked both during
painting and during event handling.

The `PhysicsState` is the state affected by input events from the user, and
time.

Its definition (in `src/main/PhysicsState.h`) is pretty straightforward. It
contains a `tick` function (we've already described when it runs), and also the
following state:

```cpp
  TimePoint m_lastPhysics;
  Plane& m_plane;

  /**
   * This quaternion is used to implement a kind of "follow the object"
   * navigation, where the camera interpolates between the old orientation and
   * the object orientation for a given factor.
   */
  glm::quat m_orientation;
```

Mainly:

 * The last time we ticked, for speed calculations.
 * The flying object, which is also a child of the scene[^shared-state].
 * The orientation the camera is going to (the `Plane` object keeps its own
   orientation. Initially, I tried to implement it with transform matrices, but
   had a _lot_ of trouble doing so[^not-that]. Then I read about quaternions,
   and everything was green again.

We use that as follows:

The object can be moved with the arrows[^movement], and also accelerated and
decelerated with the `PgUp` and `PgDn` keys, those events reach through the main
thread to the physics state, that just tunes the state appropriately.

Before painting a frame, we move the object in the appropriate direction, and
use SLERP to follow it with the camera.

The code to change the direction of the plane is straight-forward, thanks to
quaternions! :)

```cpp
void PhysicsState::rotate(Scene&, Direction dir, float amount) {
  bool isTopDown = dir == Direction::Top || dir == Direction::Down;

  if (dir == Direction::Top || dir == Direction::Left)
    amount *= -1.0;

  if (isTopDown)
    m_plane.pitch(amount);
  else
    m_plane.roll(amount);
}

void Plane::pitch(float amount) {
  m_orientation = glm::rotate(glm::quat(), amount, left()) * m_orientation;
}

void Plane::yaw(float amount) {
  m_orientation = glm::rotate(glm::quat(), amount, normal()) * m_orientation;
}

void Plane::roll(float amount) {
  m_orientation = glm::rotate(glm::quat(), amount, direction()) * m_orientation;
}
```

[^shared-state]: Note that the amount of shared state that could cause trouble
started getting worrisome. Other languages like [Rust](https://rust-lang.org)
would make this more manageable. I thought about doing this assignment in Rust
but decided it wasn't too worth it. I was right, and when I tried to do the
second I ended up having to spend a whole day reading the graphics pipeline
source to [find a wayland-related
bug](https://github.com/vberger/wayland-client-rs/issues/75) in one of the Rust
libraries. Now I know about how wayland works and how mesa uses it, but that's
kind of off-topic.

[^not-that]: Not that it couldn't be done (I think at one point I did it
properly), but tracking all the normals and transformation was painful, and
trying to be more clever to implement what right now is a simple SLERP beated
me.

[^movement]: I know, though, that it could be improved, since right now we only
pitch and roll. Allowing yawing could be easy enough, but it's just not done.

## Skybox implementation

The skybox implementation is fairly straight-forward. Mainly we have the 36
vertices that we use to define the triangles that form a giant cube (we don't
use an index buffer for this).

Then, we load six textures (with `GL_TEXTURE_CUBE_MAP`), and create a custom GL
program that doesn't care about shadows, and has that cube map as a uniform.

Drawing is straight-forward, even though for obvious reasons we don't enable the
depth buffer (otherwise everything else will be hidden).

The GLSL program to draw the skybox is really straight-forward.

The vertex shader is only a pass-through shader, that multiplies the position
(which is in the range $[-1, 1]$) by the view projection matrix, and the
fragment shader only does a texture fetch in the cube texture to determine its
final position.

```glsl
// common.glsl
uniform mat4 uViewProjection;
uniform samplerCube uSkybox;

// vertex.glsl
#line 1

layout (location = 0) in vec3 vPosition;

out vec4 oFragColor;
out vec3 fPosition;

void main() {
  gl_Position = uViewProjection * vec4(vPosition, 1.0);
  fPosition = vPosition;
}

// fragment.glsl
#line 1

out vec4 oFragColor;
in vec3 fPosition;

void main () {
  oFragColor = texture(uSkybox, fPosition);
}
```

Worth noting that the skybox _view matrix_ is slightly different than for the
rest of the objects. In particular, the "normal" view matrix contains the
camera translation, while the skybox matrix doesn't. The skybox is always
"looking at" from the origin in the same direction as the camera:

```cpp
void Scene::recomputeView(const glm::vec3& lookingAt, const glm::vec3& up) {
  m_view = glm::lookAt(m_cameraPosition, lookingAt, up);
  m_skyboxView =
      glm::lookAt(glm::vec3(0, 0, 0), lookingAt - m_cameraPosition, up);
}
```

## The terrain

Implementing the terrain wasn't really hard, but there's a lot to tell about the
different terrains.

The general approach for implementing the rugged aspect of the terrain is using
a heightmap, a texture that contains a value for each pixels that represents
more or less height.

Now, that can be implemented in multiple ways, let's go to that.

### "Classic" terrain

The most straight-forward way to implement a terrain, is creating a plane of
pixels, and assigning the correct height to each one of them, then for each
square, generate two triangles.

This is the approach that the `Terrain` class follows. It generates a terrain of
the size of the heightmap, so every single pixel in the heightmap image
represents a "portion" of the terrain.

It pushes a vertex per pixel of the terrain, then uses an index buffer to create
two triangles per each set of four vertices.

This works quite well in practice, but just because the terrain I've used is
really small.

In order to adapt better to different heightmap, we could create a fixed set of
píxels instead.

### Level of detail terrain 1: `DynTerrain`

On OpenGL 4, I experimented with tesellation shaders in order to get cheaply
(without quadtrees and heavy CPU to GPU transfers) a terrain that had more
detail the nearer the camera was.

The first approach I took to do so is implemented in `DynTerrain`.

Basically, we upload the heightmap and a 2d plane to the GPU, then it's the GPU
the one that takes care of tesellating that plane, and getting the appropriate
position for the final vertex.

The code, thus, looks as follows:

1. Generate a 2d plane of $w \times h$ pixels.
2. Upload the heightmap texture, along with that plane, to the GPU.
3. When drawing, decide tesellation level depending on the distance of the
   camera position with respect to the fragment position (incurs one extra
   texture fetch, in `res/dyn-terrain/tess-control.glsl`).
4. Interpolate the triangles normally in the tesellation evaluation shader, and
   leave the final 3d position in `tess-eval.glsl`.
5. A pass-through geometry shader just takes care of computing the appropriate
   normals.
6. The fragment shader just grabs the uv coordinates and colors itself.

This works quite well, though I should be a bit more careful than what I'm right
now about the change of tessellation levels among contiguous fragments, which
seems a bit awful.

That code all lives in the `src/base/DynTerrain.h` and
`src/base/DynTerrain.cpp` files, and in the `res/dyn-terrain` directory.

Nonetheless, there's another approach to be taken.

### LOD terrain 2: `BezierTerrain`

The other approach I took for LOD tessellation of the terrain was not so
straight-forward, and required learning a bit about Bézier surfaces.

The approach is, basically, divide the terrain in patches of 16 control points,
that then you interpolate resolving the cubic Bézier curve equations in order to
get the interpolated points.

That code lives in the `src/base/BezierTerrain.h` and
`src/base/BezierTerrain.cpp` files, and in the `res/bezier-terrain` directory.

You can try it with `./bin/main --bezier` (if you have GL 4), and there are
a few extra controls on top of the normal ones:

 * `<p>` toggles dynamic tessellation (so the effect is seen more easily).
 * `<j>` raises the static tessellation level (not quite static, but anyway,
   raises the tessellation level that is used if dynamic tessellation is not
   enabled.
 * `<k>` decrements the tessellation level used if dynamic tessellation is
   disabled.

I'll send you if I can a video showcasing it.

## Shadow Mapping

I won't extend myself too much on the topic of shadow mapping. It's a very well
known technique to do dynamic shadows.

I implemented it as a two phase rendering, as explained at the beginning of this
writeup.

The only "curious" thing I did is that, instead of re-computing also the shadow
of the terrain all the time, given that the light is static, I just cache it and
blit it over the draw framebuffer before drawing the rest of the shadows of the
scene:

```cpp
if (m_shadowMapFramebufferAndTexture) {
  Optional<GLuint> terrainShadowMap =
      m_terrain ? m_terrain->shadowMapFBO() : None;

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
                    m_shadowMapFramebufferAndTexture->first);
  glClear(GL_DEPTH_BUFFER_BIT);
  if (terrainShadowMap) {
    // We copy the cached terrain FBO.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, *terrainShadowMap);
    glBlitFramebuffer(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT, 0, 0, SHADOW_WIDTH,
                      SHADOW_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
  }
  drawObjects(true);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
```

This happens for all kinds of terrain except the "basic" one, mainly because
that other one reuses the main program and I didn't want to re-compile it, or
reference-count it.

Other curious thing I did is implementing percentage-closer filtering. This
technique is described in [a Nvidia
article](http://http.developer.nvidia.com/GPUGems/gpugems_ch11.html),
and I implemented it the simplest possible way, that is, simply averaging
the eight surrounding pixels in the fragment shader.

There's a lot I could improve for that though.

## Modeling the "Rocket" model

I left this for the end, because it's sincerely the last thing I've done.
I haven't spent too much time with Blender, mostly because I don't enjoy it
a lot.

The model I did was pretty simple, it's indeed a cone at the top, five conic
segments for the middle of the rocket and the inner parts, and a few Bézier
curves I used to do the small red things in the base of the model.

As I wrote above, I completely forgot to do screenshots while doing it, so
I probably re-do the small red parts, that were the most tricky, and send them
over to you by mail.

In general, I [kind of appreciated it in the
end](https://twitter.com/ecbos_/status/817842195696930816).
