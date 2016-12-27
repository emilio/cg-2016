import { Point, Line, LineType } from "Line";

let PROGRAM_CACHE: WebGLProgram = null;

class PolyLine implements Line {
  controlPoints: Array<Point>;
  constructor(controlPoints: Array<Point> = []) {
    this.controlPoints = controlPoints;
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
    console.log(this.controlPoints, ret);
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
    if (kind == gl.POINTS && isSelected) {
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

  addControlPoint(p: Point) {
    this.controlPoints.push(p);
    this.setDirty();
  }

  setDirty() {
    // TODO: When we cache stuff we will want to properly implement this.
  }

  getType() : LineType {
    return LineType.PolyLine;
  }
}

export default PolyLine;
