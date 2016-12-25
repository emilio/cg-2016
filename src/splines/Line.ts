class Point {
  constructor(public x: number, public y: number) {}
}

enum LineType {
  BSpline,
  Hermite,
  PolyLine,
}

interface Line {
  controlPoints: Array<Point>;

  /**
   * Upload and draw the line into the current GL context.
   *
   * Width and height can be reached via gl.canvas.
   *
   * This is expected to be an atomic operation.
   */
  draw(gl: WebGLRenderingContext);

  getType() : LineType;
};

export { Point, Line, LineType };
