import { ContainmentResult, DisplayMode, Matrix4D, Point, Point3D, Line, LineType } from "Line";

let PROGRAM_CACHE: WebGLProgram = null;
let REVOLUTION_PROGRAM_CACHE: WebGLProgram = null;

class PolyLine implements Line {
  controlPoints: Array<Point>;
  constructor(controlPoints: Array<Point> = []) {
    this.controlPoints = controlPoints;
  }

  removeControlPointAt(i: number) {
    this.controlPoints.splice(i, 1);
    this.setDirty();
  }

  /**
   * This is somewhat expensive, and can of course be optimized keeping the
   * control points as a plan Float32Array.
   */
  pointsAsArrayBuffer() : Float32Array {
    let ret = new Float32Array(Math.max(1, this.controlPoints.length - 1) * 4);
    let j = 0;

    // Specialize this to show a single point when there's just one.
    if (this.controlPoints.length == 1) {
      ret[j++] = this.controlPoints[0].x;
      ret[j++] = this.controlPoints[0].y;
      ret[j++] = this.controlPoints[0].x;
      ret[j++] = this.controlPoints[0].y;
      return ret;
    }

    for (let i = 1; i < this.controlPoints.length; ++i) {
      ret[j++] = this.controlPoints[i - 1].x;
      ret[j++] = this.controlPoints[i - 1].y;
      ret[j++] = this.controlPoints[i].x;
      ret[j++] = this.controlPoints[i].y;
    }
    return ret;
  }

  // Build a rotation matrix for an angle `a` around a normalized axis `axis`.
  rotationMatrix(a: number, axis: Point3D) : Matrix4D {
    return Matrix4D.rotation(a, axis);
  }

  /**
   * Transform a point from world space to UDC centerd and the center of the
   * canvas.
   */
  pointToUDC(gl: WebGLRenderingContext, point: Point3D) : Point3D {
    return new Point3D(point.x / gl.canvas.width * 2.0 - 1.0,
                       point.y / gl.canvas.height * -2.0 + 1.0,
                       point.z / gl.canvas.width * -2.0);
  }

  pointFromUDC(gl: WebGLRenderingContext, point: Point3D) : Point3D {
    return new Point3D((point.x + 1.0) * 0.5 * gl.canvas.width,
                       (point.y - 1.0) * -0.5 * gl.canvas.height,
                        point.z * -0.5 * gl.canvas.width);
  }

  transformPoint(gl: WebGLRenderingContext,
                 p: Point3D,
                 m: Matrix4D) : Point3D {
    return Matrix4D.transformPoint(p, m);
  }

  revolutionSurfaceAroundAxis(gl: WebGLRenderingContext,
                              axis: Point3D,
                              rotationAmount: number,
                              rotationStep: number) : Float32Array {
    if (this.controlPoints.length === 0)
      return new Float32Array([]);

    // The final points of the surface.
    let result = [];

    // The points of the line rotated last time, in world space.
    let previousPoints: Array<Point3D> = [];
    for (let point of this.controlPoints)
      previousPoints.push(this.pointToUDC(gl, new Point3D(point.x, point.y, 0)));

    const rotationStepRadians = rotationStep * Math.PI / 180;

    let rotationMatrix = this.rotationMatrix(rotationStepRadians, axis);

    let addFace = function(p1, p2, p3, normal) {
      // First face.
      result.push(p1);
      result.push(normal);
      result.push(p2);
      result.push(normal);
      result.push(p3);
      result.push(normal);
    };

    // We position the axis in the center of the screen in world position, with z = 0
    for (let angle = rotationStep; angle <= rotationAmount; angle += rotationStep) {
      let thesePoints = new Array(previousPoints.length);
      thesePoints[0] = this.transformPoint(gl, previousPoints[0], rotationMatrix);
      // Push the previous point, then this one, then the next previous point
      // if applicable, then the next of these, to build a square with two
      // triangles.
      for (let i = 1; i < this.controlPoints.length; ++i) {
        thesePoints[i] = this.transformPoint(gl, previousPoints[i], rotationMatrix);

        let normal = Point3D.cross(Point3D.substract(previousPoints[i], previousPoints[i - 1]),
                                   Point3D.substract(thesePoints[i - 1], previousPoints[i - 1]))
          .normalize();

        // First face.
        addFace(previousPoints[i - 1], thesePoints[i - 1], previousPoints[i], normal);

        // Second face.
        addFace(previousPoints[i], thesePoints[i - 1], thesePoints[i], normal);
      }

      previousPoints = thesePoints;
    }

    let ret = new Float32Array(result.length * 3);
    let i = 0;
    for (let p of result) {
      // We ship the point over already in world position.
      if (i % 2 == 0)
        p = this.pointFromUDC(gl, p);
      ret[i++] = p.x;
      ret[i++] = p.y;
      ret[i++] = p.z;
    }

    return ret;
  }

  ensureProgram(gl: WebGLRenderingContext) : WebGLProgram {
    if (PROGRAM_CACHE)
      return PROGRAM_CACHE;
    PROGRAM_CACHE = gl.createProgram();
    let vertex = gl.createShader(gl.VERTEX_SHADER);
    // TODO: Pass w/h through uniforms instead of hardcoding.
    gl.shaderSource(vertex, [
      "precision mediump float;",
      "attribute vec2 vPoint;",
      "void main() {",
      "  float x = vPoint.x / 800.0 * 2.0 - 1.0;",
      "  float y = vPoint.y / 800.0 * (- 2.0) + 1.0;",
      "  gl_PointSize = 10.0;",
      "  gl_Position = vec4(x, y, 0.0, 1.0);",
      "}",
    ].join("\n"));
    gl.compileShader(vertex);
    console.log("Vertex: ", gl.getShaderInfoLog(vertex));
    let fragment = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragment, [
      "precision mediump float;",
      "uniform vec4 uColor;",
      "void main() {",
      "  gl_FragColor = uColor;",
      "}",
    ].join("\n"));
    gl.compileShader(fragment);
    console.log("Fragment: ", gl.getShaderInfoLog(fragment));
    gl.attachShader(PROGRAM_CACHE, vertex);
    gl.attachShader(PROGRAM_CACHE, fragment);
    gl.linkProgram(PROGRAM_CACHE);
    console.log("Tried to link program: ", gl.getProgramInfoLog(PROGRAM_CACHE));
    return PROGRAM_CACHE;
  }

  ensureRevolutionProgram(gl: WebGLRenderingContext) : WebGLProgram {
    if (REVOLUTION_PROGRAM_CACHE)
      return REVOLUTION_PROGRAM_CACHE;
    REVOLUTION_PROGRAM_CACHE = gl.createProgram();

    let vertex = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertex, [
      "precision mediump float;",
      "attribute vec3 vPoint;",
      "attribute vec3 vNormal;",
      "varying vec3 fNormal;",
      "varying vec3 fPosition;",
      "uniform mat4 uViewProjection;",
      "void main() {",
      "  // Point already in world space.",
      "  fPosition = vPoint;",
      "  fNormal = vNormal;",
      "  gl_Position = uViewProjection * vec4(vPoint, 1.0);",
      "}",
    ].join("\n"));
    gl.compileShader(vertex);
    console.log("Vertex: ", gl.getShaderInfoLog(vertex));

    let fragment = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragment, [
      "precision mediump float;",
      "varying vec3 fNormal;",
      "varying vec3 fPosition;",
      "uniform vec3 uLightPosition;",
      "void main() {",
      "  const vec4 OBJECT_COLOR = vec4(0.5, 0.5, 0.0, 1.0);",
      "  const vec4 SPEC_COLOR = vec4(1.0, 1.0, 1.0, 1.0);",
      "  const float AMBIENT_LIGHT_STRENGTH = 0.6;",

      "  vec4 ambient = OBJECT_COLOR * AMBIENT_LIGHT_STRENGTH;",

      "  vec3 lightDir = normalize(fPosition - uLightPosition);",

      "  float diffuseImpact = max(dot(fNormal, -lightDir), 0.0);",
      "  vec4 diffuse = diffuseImpact * OBJECT_COLOR;",

      "  vec3 reflectDir = reflect(lightDir, fNormal);",
      "  float spec = ",
      "    pow(max(dot(-lightDir, reflectDir), 0.0), 4.0);",
      "  vec4 specular = spec * SPEC_COLOR;",
      "  gl_FragColor = diffuse + specular + ambient;",
      "}",
    ].join("\n"));
    gl.compileShader(fragment);
    console.log("Fragment: ", gl.getShaderInfoLog(fragment));

    gl.attachShader(REVOLUTION_PROGRAM_CACHE, vertex);
    gl.attachShader(REVOLUTION_PROGRAM_CACHE, fragment);
    gl.linkProgram(REVOLUTION_PROGRAM_CACHE);
    console.log("Tried to link program: ", gl.getProgramInfoLog(REVOLUTION_PROGRAM_CACHE));
    return REVOLUTION_PROGRAM_CACHE;
  }

  draw(gl: WebGLRenderingContext,
       isSelected: boolean,
       selectedPointIndex: number) {
    this.drawLine(gl, isSelected, selectedPointIndex);
    this.drawPoints(gl, isSelected, selectedPointIndex);
  }

  drawAs(gl: WebGLRenderingContext,
         kind: number,
         isSelected: boolean,
         selectedPointIndex: number) {
    // TODO: cache stuff here, this is all very silly.
    let buff = gl.createBuffer();
    gl.lineWidth(5.0);
    let arr = this.pointsAsArrayBuffer();
    let program = this.ensureProgram(gl);
    let vPointLocation = gl.getAttribLocation(program, "vPoint");

    gl.useProgram(program);
    gl.bindBuffer(gl.ARRAY_BUFFER, buff);
    gl.bufferData(gl.ARRAY_BUFFER, arr, gl.STATIC_DRAW);

    gl.enableVertexAttribArray(vPointLocation);
    gl.vertexAttribPointer(vPointLocation, 2, gl.FLOAT, false, 0, 0);

    // TODO(emilio): Implement a custom color for the selected
    // point.
    let color = gl.getUniformLocation(program, "uColor");
    if (kind == gl.POINTS)
      gl.uniform4f(color, 0.0, 0.5, 0.0, 1.0);
    else if (isSelected)
      gl.uniform4f(color, 0.7, 0.0, 0.0, 1.0);
    else
      gl.uniform4f(color, 0.5, 0.0, 0.0, 1.0);

    gl.drawArrays(kind, 0, arr.length / 2);

    // Draw the selected point again with other (greener) color.
    if (kind == gl.POINTS && selectedPointIndex >= 0) {
      gl.uniform4f(color, 0.0, 0.7, 0.0, 1.0);
      gl.drawArrays(kind, Math.max(0, selectedPointIndex * 2 - 1), 1);
    }

    gl.useProgram(null);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.deleteBuffer(buff);
  }

  drawLine(gl: WebGLRenderingContext,
           isSelected: boolean,
           selectedPointIndex: number) {
    this.drawAs(gl, gl.LINES, isSelected, selectedPointIndex);
  }

  drawPoints(gl: WebGLRenderingContext,
             isSelected: boolean,
             selectedPointIndex: number) {
    this.drawAs(gl, gl.POINTS, isSelected, selectedPointIndex);
  }

  drawRevolutionSurface(gl: WebGLRenderingContext,
                        axis: Point3D,
                        viewProjection: Matrix4D,
                        _uIncrement: number,
                        rotationAmount: number,
                        rotationStep: number) {
    console.log(axis);

    let program = this.ensureRevolutionProgram(gl);
    gl.useProgram(program);

    let arr = this.revolutionSurfaceAroundAxis(gl, axis, rotationAmount, rotationStep);

    let vPointLocation = gl.getAttribLocation(program, "vPoint");
    let vNormalLocation = gl.getAttribLocation(program, "vNormal");
    let buff = gl.createBuffer();

    gl.bindBuffer(gl.ARRAY_BUFFER, buff);
    gl.bufferData(gl.ARRAY_BUFFER, arr, gl.STATIC_DRAW);

    gl.uniformMatrix4fv(gl.getUniformLocation(program, "uViewProjection"),
                        false, viewProjection.toFloat32Array());
    gl.uniform3f(gl.getUniformLocation(program, "uLightPosition"),
                 200.0, 200.0, -800.0); // Make it come from the top left.

    // We push six floats each time.
    const SIZE_OF_PACK_BYTES: number = 3 * 4 * 2;

    gl.enableVertexAttribArray(vPointLocation);
    gl.vertexAttribPointer(vPointLocation, 3, gl.FLOAT, false,
                           SIZE_OF_PACK_BYTES, 0);
    gl.enableVertexAttribArray(vNormalLocation);
    gl.vertexAttribPointer(vNormalLocation, 3, gl.FLOAT, false,
                           SIZE_OF_PACK_BYTES, SIZE_OF_PACK_BYTES / 2);

    let pointCount = arr.length / 3 / 2;
    gl.drawArrays(gl.TRIANGLES, 0, pointCount);

    gl.disableVertexAttribArray(vPointLocation);
    gl.disableVertexAttribArray(vNormalLocation);

    gl.useProgram(null);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.deleteBuffer(buff);
  }

  addControlPoint(p: Point) {
    this.controlPoints.push(p);
    this.setDirty();
  }

  setDirty() {
    // TODO: If/when we cache stuff we will want to properly implement this.
  }

  contains(p: Point) : ContainmentResult {
    // First easy pass through the control points to see if p is among them.
    for (let i = 0; i < this.controlPoints.length; ++i)
      if (this.controlPoints[i].near(p, 5))
        return new ContainmentResult(true, i);

    // Now the second pass, doing the math to see if it picks one of the
    // segments.
    for (let i = 1; i < this.controlPoints.length; ++i) {
      let p1 = this.controlPoints[i - 1];
      let p2 = this.controlPoints[i];

      let direction = Point.substract(p2, p1);
      let distance = Point.substract(p1, p);
      let fragment = distance.length() / direction.length();
      if (fragment > 1)
        continue;

      let potentialPoint = Point.add(p1, Point.mul(direction, fragment));
      if (potentialPoint.near(p, 2.5))
        return new ContainmentResult(true);
    }

    // Else don't match.
    return new ContainmentResult(false);
  }

  getType() : LineType {
    return LineType.PolyLine;
  }
}

export default PolyLine;
