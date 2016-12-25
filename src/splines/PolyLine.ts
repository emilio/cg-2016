import { Point, Line, LineType } from "Line";

let PROGRAM_CACHE: WebGLProgram = null;

class PolyLine implements Line {
  controlPoints: Array<Point>;
  constructor() {
    this.controlPoints = new Array();
  }

  /**
   * This is somewhat expensive, and can of course be optimized keeping the
   * control points as a plan Float32Array.
   */
  pointsAsArrayBuffer() : Float32Array {
    let ret = new Float32Array(this.controlPoints.length * 4);
    let floats = [];
    let j = 0;
    for (let i = 1; i < this.controlPoints.length; ++i) {
      ret[j++] = this.controlPoints[i - 1].x;
      ret[j++] = this.controlPoints[i - 1].y;
      ret[j++] = this.controlPoints[i].x;
      ret[j++] = this.controlPoints[i].y;
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
      "  gl_Position = vec4(x, y, 0.0, 1.0);",
      "}",
    ].join("\n"));
    gl.compileShader(vertex);
    console.log("Vertex: ", gl.getShaderInfoLog(vertex));
    let fragment = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragment, [
      "precision mediump float;",
      "void main() {",
      "  gl_FragColor = vec4(0.5, 0.0, 0.0, 1.0);",
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

  draw(gl: WebGLRenderingContext) {
    let buff = gl.createBuffer();
    let arr = this.pointsAsArrayBuffer();
    let program = this.ensureProgram(gl);
    let vPointLocation = gl.getAttribLocation(program, "vPoint");
    console.log("Point: ", vPointLocation);

    gl.useProgram(program);
    gl.bindBuffer(gl.ARRAY_BUFFER, buff);
    gl.bufferData(gl.ARRAY_BUFFER, arr, gl.STATIC_DRAW);
    gl.enableVertexAttribArray(vPointLocation);
    gl.vertexAttribPointer(vPointLocation, 2, gl.FLOAT, false, 0, 0);
    gl.drawArrays(gl.LINES, 0, arr.length / 2);

    gl.useProgram(null);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.deleteBuffer(buff);
  }

  getType() : LineType {
    return LineType.PolyLine;
  }
}

export default PolyLine;
