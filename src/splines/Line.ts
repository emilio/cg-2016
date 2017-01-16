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

class Point3D {
  constructor(public x: number, public y: number, public z: number) {}

  static mul(one: Point3D, n: number) : Point3D {
    return new Point3D(one.x * n, one.y * n, one.z * n);
  }

  static substract(one: Point3D, other: Point3D) : Point3D {
    return new Point3D(one.x - other.x, one.y - other.y, one.z - other.z);
  }

  static cross(one: Point3D, other: Point3D) : Point3D {
    return new Point3D(one.y * other.z - other.y * one.z,
                       one.z * other.x - other.z * one.x,
                       one.x * other.y - other.x * one.y);
  }

  length() : number {
    return Math.sqrt(this.x * this.x + this.y * this.y + this.z * this.z);
  }

  normalize() : Point3D {
    const len = this.length();
    return new Point3D(this.x / len,
                       this.y / len,
                       this.z / len);
  }
}

enum LineType {
  BSpline,
  Hermite,
  PolyLine,
}

enum DisplayMode {
  Lines,
  Revolution,
};

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
   * Evaluates this line and draws it as a revolution surface around the axis.
   */
  drawRevolutionSurface(gl: WebGLRenderingContext,
                        axis: Point3D);

  /**
   * Returns whether point p is one of the points in this line or touches it.
   */
  contains(p: Point) : ContainmentResult;

  addControlPoint(p: Point);
  removeControlPointAt(i: number);
  setDirty();

  getType() : LineType;
};

export { ContainmentResult, DisplayMode, Point, Point3D, Line, LineType };
