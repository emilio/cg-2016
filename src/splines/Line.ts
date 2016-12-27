class Point {
  constructor(public x: number, public y: number) {}

  static add(one: Point, other: Point) : Point {
    return new Point(one.x + other.x, one.y + other.y);
  }

  static substract(one: Point, other: Point) : Point {
    return new Point(one.x - other.x, one.y - other.y);
  }

  static mul(one: Point, n: number) : Point {
    return new Point(one.x * n, one.y * n);
  }

  near(other: Point, maxDistance: number) : boolean {
    return Point.substract(this, other).length() <= maxDistance;
  }

  length() : number {
    return Math.sqrt(this.x * this.x + this.y * this.y);
  }
}

enum LineType {
  BSpline,
  Hermite,
  PolyLine,
}

class ContainmentResult {
  constructor(public success: boolean, public selectedPointIndex: number = -1) {}
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
  draw(gl: WebGLRenderingContext,
       isSelected: boolean,
       selectedPointIndex: number);

  /**
   * Returns whether point p is one of the points in this line or touches it.
   */
  contains(p: Point) : ContainmentResult;

  addControlPoint(p: Point);
  removeControlPointAt(i: number);
  setDirty();

  getType() : LineType;
};

export { ContainmentResult, Point, Line, LineType };
