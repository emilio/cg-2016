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

  static origin() : Point3D {
    return new Point3D(0, 0, 0);
  }

  static YAxis() : Point3D {
    return new Point3D(0, 1, 0);
  }

  static XAxis() : Point3D {
    return new Point3D(1, 0, 0);
  }

  static ZAxis() : Point3D {
    return new Point3D(0, 0, 1);
  }

  static add(one: Point3D, other: Point3D) : Point3D {
    return new Point3D(one.x + other.x, one.y + other.y, one.z + other.z);
  }

  static mul(one: Point3D, n: number) : Point3D {
    return new Point3D(one.x * n, one.y * n, one.z * n);
  }

  static mid(p1: Point3D, p2: Point3D) : Point3D {
    return Point3D.add(p1, Point3D.mul(Point3D.substract(p2, p1), .5));
  }

  static substract(one: Point3D, other: Point3D) : Point3D {
    return new Point3D(one.x - other.x, one.y - other.y, one.z - other.z);
  }

  static cross(one: Point3D, other: Point3D) : Point3D {
    return new Point3D(one.y * other.z - other.y * one.z,
                       one.z * other.x - other.z * one.x,
                       one.x * other.y - other.x * one.y);
  }

  static dot(one: Point3D, other: Point3D) : number {
    return one.x * other.x + one.y * other.y + one.z * other.z;
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
       selectedPointIndex: number,
       uIncrement: number);

  /**
   * Evaluates this line and draws it as a revolution surface around the axis.
   */
  drawRevolutionSurface(gl: WebGLRenderingContext,
                        axis: Point3D,
                        viewProj: Matrix4D,
                        uIncrement: number,
                        rotationAmount: number,
                        rotationStep: number);

  /**
   * Returns whether point p is one of the points in this line or touches it.
   */
  contains(p: Point) : ContainmentResult;

  addControlPoint(p: Point);
  removeControlPointAt(i: number);
  setDirty();

  getType() : LineType;
};

class Matrix4D {
  constructor(public m11: number, public m12: number, public m13: number, public m14: number,
              public m21: number, public m22: number, public m23: number, public m24: number,
              public m31: number, public m32: number, public m33: number, public m34: number,
              public m41: number, public m42: number, public m43: number, public m44: number)
  {}

  static identity() : Matrix4D {
    return new Matrix4D(1.0, 0.0, 0.0, 0.0,
                        0.0, 1.0, 0.0, 0.0,
                        0.0, 0.0, 1.0, 0.0,
                        0.0, 0.0, 0.0, 1.0);
  }

  static equals(one: Matrix4D, other: Matrix4D) : boolean {
    return one.m11 == other.m11 &&
      one.m12 == other.m12 &&
      one.m13 == other.m13 &&
      one.m14 == other.m14 &&
      one.m21 == other.m21 &&
      one.m22 == other.m22 &&
      one.m23 == other.m23 &&
      one.m24 == other.m24 &&
      one.m31 == other.m31 &&
      one.m32 == other.m32 &&
      one.m33 == other.m33 &&
      one.m34 == other.m34 &&
      one.m41 == other.m41 &&
      one.m42 == other.m42 &&
      one.m43 == other.m43 &&
      one.m44 == other.m44;
  }

  static mul(one: Matrix4D, other: Matrix4D) : Matrix4D {
    return new Matrix4D(one.m11 * other.m11 + one.m12 * other.m21 + one.m13 * other.m31 + one.m14 * other.m41,
                        one.m11 * other.m12 + one.m12 * other.m22 + one.m13 * other.m32 + one.m14 * other.m42,
                        one.m11 * other.m13 + one.m12 * other.m23 + one.m13 * other.m33 + one.m14 * other.m43,
                        one.m11 * other.m14 + one.m12 * other.m24 + one.m13 * other.m34 + one.m14 * other.m44,

                        one.m21 * other.m11 + one.m22 * other.m21 + one.m23 * other.m31 + one.m24 * other.m41,
                        one.m21 * other.m12 + one.m22 * other.m22 + one.m23 * other.m32 + one.m24 * other.m42,
                        one.m21 * other.m13 + one.m22 * other.m23 + one.m23 * other.m33 + one.m24 * other.m43,
                        one.m21 * other.m14 + one.m22 * other.m24 + one.m23 * other.m34 + one.m24 * other.m44,

                        one.m31 * other.m11 + one.m32 * other.m21 + one.m33 * other.m31 + one.m34 * other.m41,
                        one.m31 * other.m12 + one.m32 * other.m22 + one.m33 * other.m32 + one.m34 * other.m42,
                        one.m31 * other.m13 + one.m32 * other.m23 + one.m33 * other.m33 + one.m34 * other.m43,
                        one.m31 * other.m14 + one.m32 * other.m24 + one.m33 * other.m34 + one.m34 * other.m44,

                        one.m41 * other.m11 + one.m42 * other.m21 + one.m43 * other.m31 + one.m44 * other.m41,
                        one.m41 * other.m12 + one.m42 * other.m22 + one.m43 * other.m32 + one.m44 * other.m42,
                        one.m41 * other.m13 + one.m42 * other.m23 + one.m43 * other.m33 + one.m44 * other.m43,
                        one.m41 * other.m14 + one.m42 * other.m24 + one.m43 * other.m34 + one.m44 * other.m44);
  }

  static diag(t: number) : Matrix4D {
    return new Matrix4D(t, 0, 0, 0,
                        0, t, 0, 0,
                        0, 0, t, 0,
                        0, 0, 0, t);
  }

  /**
   * An ortographic projection matrix of size S, with near and far coordinates.
   */
  static ortho(size: number, near: number, far: number) : Matrix4D {
    let ret = Matrix4D.diag(1.0);
    let left = -size / 2;
    let right = size / 2;
    let top = -size / 2;
    let bottom = size / 2;
    ret.m11 = 2 / (right - left);
    ret.m22 = 2 / (top - bottom);
    ret.m33 = - 2 / (far - near);

    ret.m14 = - (right + left) / (right - left);
    ret.m24 = - (top + bottom) / (top - bottom);
    ret.m34 = - (far + near) / (far - near);

    return ret;
  }

  static lookAt(eye: Point3D, center: Point3D, up: Point3D) : Matrix4D {
    let f = Point3D.substract(center, eye).normalize();
    let s = Point3D.cross(f, up).normalize();
    let u = Point3D.cross(s, f);

    let ret = Matrix4D.diag(1.0);
    ret.m11 = s.x;
    ret.m12 = s.y;
    ret.m13 = s.z;

    ret.m21 = u.x;
    ret.m22 = u.y;
    ret.m23 = u.z;

    ret.m31 = -f.x;
    ret.m32 = -f.y;
    ret.m33 = -f.z;

    ret.m14 = -Point3D.dot(s, eye);
    ret.m24 = -Point3D.dot(u, eye);
    ret.m34 =  Point3D.dot(f, eye);

    return ret;
  }

  static rotation(a: number, axis: Point3D) : Matrix4D {
    let cos = Math.cos(a);
    let sin = Math.sin(a);

    let oneMinusCos = 1 - cos;
    return new Matrix4D(
      cos + (1 - cos) * axis.x * axis.x,
      (1 - cos) * axis.x * axis.y + sin * axis.z,
      (1 - cos) * axis.x * axis.z - sin * axis.y,
      0,

      (1 - cos) * axis.y * axis.x - sin * axis.z,
      cos + (1 - cos) * axis.y * axis.y,
      (1 - cos) * axis.y * axis.z + sin * axis.x,
      0,

      (1 - cos) * axis.z * axis.x + sin * axis.y,
      (1 - cos) * axis.z * axis.y - sin * axis.x,
      cos + (1 - cos) * axis.z * axis.y - sin * axis.x,
      0,

      0,
      0,
      0,
      1,
    );
  }

  static transformPoint(p: Point3D, m: Matrix4D) : Point3D {
    let x = m.m11 * p.x + m.m12 * p.y + m.m13 * p.z + m.m14;
    let y = m.m21 * p.x + m.m22 * p.y + m.m23 * p.z + m.m24;
    let z = m.m31 * p.x + m.m32 * p.y + m.m33 * p.z + m.m34;
    let w = m.m41 * p.x + m.m42 * p.y + m.m43 * p.z + m.m44;

    return new Point3D(x / w, y / w, z / w);
  }

  // NB: We transpose it here to match opengl's column major order.
  toFloat32Array() : Float32Array {
    let arr = new Float32Array(16);
    let i = 0;
    arr[i++] = this.m11;
    arr[i++] = this.m21;
    arr[i++] = this.m31;
    arr[i++] = this.m41;

    arr[i++] = this.m12;
    arr[i++] = this.m22;
    arr[i++] = this.m32;
    arr[i++] = this.m42;

    arr[i++] = this.m13;
    arr[i++] = this.m23;
    arr[i++] = this.m33;
    arr[i++] = this.m43;

    arr[i++] = this.m14;
    arr[i++] = this.m24;
    arr[i++] = this.m34;
    arr[i++] = this.m44;

    return arr;
  }
};

export { ContainmentResult, DisplayMode, Matrix4D, Point, Point3D, Line, LineType };
