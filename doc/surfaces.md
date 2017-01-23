---
title: Computer Graphics
subtitle: Second assignment
author:
 - "Emilio Cobos Álvarez <emilio.cobos@student.um.si>, <emilio@crisal.io>"
numbersections: true
toc: true
colorlinks: true
abstract: |
 Description and documentation of the implementation of a curve evaluation and
 revolution surfaces.
---

# Introduction

The second assignment was about curve evaluation and the implementation of
a simple interface that allowed the user to manipulate a bunch of control
points and curve parameters, as well as generating revolution surfaces from the
curves.

Since I was bored of C++, and I feared getting into a pit of [Wayland
bug-fixing](https://github.com/vberger/wayland-client-rs/issues/75) if I used
Rust, I decided to do this one with WebGL and TypeScript.

The result is not really that much impressive, and I could improve performance
dramatically if I cared enough to cache a bunch of things (like buffers and
evaluated surfaces) in more situations. It does the job though.

# Building

There are building instructions available in the `README.md` file. To build this
you just need a TypeScript compiler that you can install doing:

```console
$ npm install -g typescript
```

It's a web application though, so I've put the compiled version online at
[https://crisal.io/tmp/splines/splines.html](https://crisal.io/tmp/splines/splines.html).

# Documentation

The main idea of how this is implemented is that we have different kind of
lines: `PolyLine`, `HermiteCurve`, and `BSpline`.

The whole (correct) assumption is that every kind of curve can end up being
evaluated as a `PolyLine`.

That is, the internal representation of the Hermite curve has its control
points, and its tangents, but when the time to draw comes, we only need to learn
how to draw a `PolyLine`.

Thus, the drawing code for a `HermiteCurve` looks as follows:

```ts
  draw(gl: WebGLRenderingContext,
       isSelected: boolean,
       selectedPointIndex: number,
       uIncrement: number) {
    this.evaluatedLine(uIncrement).drawLine(gl, isSelected, selectedPointIndex);
    let l = new PolyLine(this.controlPoints);
    l.drawPoints(gl, isSelected, selectedPointIndex);
  }
```

where `evaluatedLine` just returns a `PolyLine`.

## PolyLine drawing code

As we've seen, most of the relevant drawing code ends up deferred to the
`PolyLine` class.

The `PolyLine` drawing code looks as follows:

```ts
draw(gl: WebGLRenderingContext,
     isSelected: boolean,
     selectedPointIndex: number) {
  this.drawLine(gl, isSelected, selectedPointIndex);
  this.drawPoints(gl, isSelected, selectedPointIndex);
}
```

where `drawLine` does the obvious thing (drawing the actual line), and
`drawPoints` also does the obvious thing (drawing the points in another color).

### `PolyLine::drawAs`

The `drawAs` method contains the actual line drawing code, implemented with
WebGL.

The method just submits to the GPU the points of the line _in world space_[^ws],
sets the color uniforms, and calls a GL program with either `gl.LINES` or
`gl.POINTS`.

The GL program looks as follows.

#### Vertex shader

```glsl
precision mediump float;
attribute vec2 vPoint;
void main() {
  float x = vPoint.x / 800.0 * 2.0 - 1.0;
  float y = vPoint.y / 800.0 * (- 2.0) + 1.0;
  gl_PointSize = 10.0;
  gl_Position = vec4(x, y, 0.0, 1.0);
}
```

#### Fragment shader

```glsl
precision mediump float;
uniform vec4 uColor;
void main() {
  gl_FragColor = uColor;
}
```

## Curve evaluation code

The individual curve evaluation code may be one of the most interesting parts of
the applications.

Also not excessively new, but here it goes a simple explanation of how it works.

[^ws]: This is arguably a dubious thing to do, since we have the dimensions of
the canvas hard-coded in the shader in order to transform to clip space. We
should at least ship the dimensions of the canvas to the GPU too via uniforms,
or transform the coordinates to clip space before. Just not done because of lack
of time.

### Hermite curves

The hermite curve evaluation code is very straight-forward.

We go through all the segments, and evaluate each of them individually:

```ts
reevaluate(uIncrement: number) {
  this.evaluated = new PolyLine();
  for (let i = 1; i < this.controlPoints.length; ++i) {
    this.evaluateSegment(i,
                         this.evaluated.controlPoints,
                         uIncrement * this.controlPoints.length);
  }
}
```

This is easy to do since in Hermite curves all of the control points are part of
the curve.

The `evaluateSegment` code is the one that actually does the work, and looks
like this:

```ts
evaluateSegment(i: number, evaluatedControlPoints: Array<Point>, uIncrement: number) {
  let p1 = this.controlPoints[i - 1];
  let t1 = this.tangents[i - 1];
  let p2 = this.controlPoints[i];
  let t2 = this.tangents[i];

  // Now we just apply the basis functions like this for those
  // points/vectors:
  //
  // h1(s) =  2s^3 - 3s^2 + 1
  // h2(s) = -2s^3 + 3s^2
  // h3(s) =   s^3 - 2s^2 + s
  // h4(s) =   s^3 -  s^2
  evaluatedControlPoints.push(p1);
  for (let s = 0.0; s < 1.0; s += uIncrement) {
    let s2 = s * s;
    let s3 = s * s * s;

    let h1 = 2 * s3 - 3 * s2 + 1;
    let h2 = - 2 * s3 + 3 * s2;
    let h3 = s3 - 2 * s2 + s;
    let h4 = s3 - s2;

    evaluatedControlPoints.push(new Point(
      h1 * p1.x + h2 * p2.x + h3 * t1.x + h4 * t2.x,
      h1 * p1.y + h2 * p2.y + h3 * t1.y + h4 * t2.y,
    ));
  }
  evaluatedControlPoints.push(p2);
}
```

As can be seen, it's a straight-forward application of the Hermite polynomials.

### NURBs

The NURBs evaluation code is the more complex code path IMO.

It's totally non-trivial, and I messed it up multiple times refactoring it.

The way it works is simple thanks to JavaScript.

We calculate the basis functions in a very straightforward way. Te function
$N_{i,n}$ is the following:

```ts
calculateBasisFunction(i: number, n: number) {
  // Base case: N_{i, 0}.
  if (n === 0) {
    return u => {
      let r = 0.0;
      if (u >= this.knots[i] && u <= this.knots[i + 1])
        r = 1.0;
      return r;
    };
  }
  // General case: N_{i, n}.
  return u => {
    // f_{i, n} = (u - k_i) / (k_{i + n} - k_i)
    let tmp = this.knots[i + n] - this.knots[i];
    let f_i_n = 0.0;
    if (tmp !== 0)
      f_i_n = (u - this.knots[i]) / tmp;

    // g_{i, n} = (k_{i+n} - u) / (k_{i+n} - k_i)
    // NB: Here we calculate g_{i + 1, n}
    tmp = this.knots[i + n + 1] - this.knots[i + 1];
    let g_iplusone_n = 0.0;
    if (tmp !== 0)
      g_iplusone_n = (this.knots[i + 1 + n] - u) / tmp;

    let left = 0.0;
    if (f_i_n !== 0)
      left = f_i_n * this.calculateBasisFunction(i, n - 1)(u);

    let right = 0.0;
    if (g_iplusone_n !== 0)
      right = g_iplusone_n * this.calculateBasisFunction(i + 1, n - 1)(u);

    return left + right;
  };
}
```

This is a direct interpretation of the equations, and it's just a bit less
legible because it handles division by zero.

## Revolution surface

The way we create revolution surfaces is also very straight-forward, we just
create the surface rotating on the CPU, along with the normals of the faces
we've created (for shading), and ship it over to the CPU.

The shaders are slightly more tricky because we not only have 2D, but 3d, and
because we can move the camera around (so we need a view matrix). Also because
the world space definition is quite bizarre (top left is $(0, 0)$, so the center
in the $X$ and $Y$ axis is the half of the canvas width), yet the center in the
$Z$ axis is defined to be $0$. This was a source of pretty nasty
coordinate-space related bugs, that I was finally able to address.

The point conversion and rotation is implemented in
`revolutionSurfaceAroundAxis` function (in `PolyLine.ts`). I won't paste it here
because it's quite big, but it does the obvious thing.

### Vertex shader

```glsl
precision mediump float;
attribute vec3 vPoint
attribute vec3 vNormal;
varying vec3 fNormal;
varying vec3 fPosition;
uniform mat4 uViewProjection;
void main() {
  // Point already in world space.
  fPosition = vPoint;
  fNormal = vNormal;
  gl_Position = uViewProjection * vec4(vPoint, 1.0);
}
```

### Fragment shader

```glsl
precision mediump float;
varying vec3 fNormal;
varying vec3 fPosition;
uniform vec3 uLightPosition;
void main() {
  const vec4 OBJECT_COLOR = vec4(0.5, 0.5, 0.0, 1.0);
  const vec4 SPEC_COLOR = vec4(1.0, 1.0, 1.0, 1.0);
  const float AMBIENT_LIGHT_STRENGTH = 0.6;

  vec4 ambient = OBJECT_COLOR * AMBIENT_LIGHT_STRENGTH;

  vec3 lightDir = normalize(fPosition - uLightPosition);

  float diffuseImpact = max(dot(fNormal, -lightDir), 0.0);
  vec4 diffuse = diffuseImpact * OBJECT_COLOR;

  vec3 reflectDir = reflect(lightDir, fNormal);
  float spec =
    pow(max(dot(-lightDir, reflectDir), 0.0), 4.0);
  vec4 specular = spec * SPEC_COLOR;
  gl_FragColor = diffuse + specular + ambient;
}
```

# Conclusion

It was a relatively fun program to write. It can of course be optimized _a lot_,
but I had fun doing it.
