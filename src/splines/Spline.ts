import { ContainmentResult, Line, Matrix4D, Point, Point3D, LineType } from "Line";
import PolyLine from "PolyLine";

/**
 * A non-uniform Rational B-spline curve.
 *
 * Part of the code is taken, out of despair, from:
 * https://github.com/OpenNURBS/OpenNURBS/blob/develop/src/geom_defs/gCurve.hpp
 */
class BSpline implements Line {
  /**
   * The order of this B-spline, this is equivalent to the degree + 1, that
   * is, k
   */
  order: number;
  /**
   * The knot vector for this line.
   *
   * This is a vector of `controlPoints.length + order` elements, and needs
   * to be sorted in non-decreasing order.
   *
   * http://www.cl.cam.ac.uk/teaching/2000/AGraphHCI/SMEG/node4.html
   *
   * TODO(emilio): Let the user modify these. For now, let's use k[i] = i.
   */
  knots: Array<number>;
  /** The control points of this curve. */
  controlPoints: Array<Point>;
  /** The weights of each control point. */
  weights: Array<number>;
  evaluated: PolyLine;
  dirty: number;

  constructor() {
    // TODO: Compute the order from the knot vector length et al.
    this.order = 4; // Cubic for now.
    this.knots = [0, 1, 2, 3, 4]; // We need n + 1 more here.
    this.controlPoints = [];
    this.weights = [];
    this.evaluated = new PolyLine();
    this.dirty = -1.0;
  }

  /**
   * Calculates a basis function N_{i, n}, where i is the ith control point,
   * and n is the degree of the basis function.
   *
   * TODO(emilio): mnemonice this algorithm, otherwise it'll likely scale
   * pretty badly.
   */
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

  degree() : number {
    return this.order - 1;
  }

  evaluateAt(u: number) : Point {
    // "Normalize" it.
    let norm = u * this.knots[this.knots.length - 1];

    let num = new Point(0.0, 0.0);
    let denom = 0.0;
    for (let i = 0; i < this.controlPoints.length; ++i) {
      let factor = this.calculateBasisFunction(i, this.degree())(norm);
      factor *= this.weights[i];
      num = Point.add(num, Point.mul(this.controlPoints[i], factor));
      denom += factor;
    }

    return Point.div(num, denom);
  }

  reevaluate(uIncrement: number) {
    this.evaluated = new PolyLine();

    let u = 0.0;
    for (; u <= 1.0; u += uIncrement)
      this.evaluated.controlPoints.push(this.evaluateAt(u));
    if (u > 1.0)
      this.evaluated.controlPoints.push(this.evaluateAt(1.0));
  }

  evaluatedLine(uIncrement: number = 0.05) : PolyLine {
    if (this.dirty !== uIncrement)
      this.reevaluate(uIncrement);
    this.dirty = uIncrement;
    return this.evaluated;
  }

  draw(gl: WebGLRenderingContext,
       isSelected: boolean,
       selectedPointIndex: number,
       uIncrement: number) {
    // We evaluate a BSpline as a simple polyline, then just paint that. It's
    // possible we could use the GPU to evaluate bezier curves with geometry
    // shaders, but oh well.
    this.evaluatedLine(uIncrement).drawLine(gl, isSelected, selectedPointIndex);
    // Then we draw the control points using the same code as for the polyline.
    let l = new PolyLine(this.controlPoints);
    l.drawPoints(gl, isSelected, selectedPointIndex);
  }

  drawRevolutionSurface(gl: WebGLRenderingContext,
                        axis: Point3D,
                        viewProj: Matrix4D,
                        uIncrement: number,
                        rotationAmount: number,
                        rotationStep: number) {
    this.evaluatedLine(uIncrement)
      .drawRevolutionSurface(gl,
                             axis,
                             viewProj,
                             uIncrement,
                             rotationAmount,
                             rotationStep);
  }

  contains(p: Point) : ContainmentResult {
    // This is just a shady trick to reuse the control point
    // check.
    let r = (new PolyLine(this.controlPoints)).contains(p);
    if (r.success && r.selectedPointIndex !== -1)
      return r;

    r = this.evaluatedLine().contains(p);
    r.selectedPointIndex = -1;
    return r;
  }

  addControlPoint(p: Point) {
    this.controlPoints.push(p);
    this.weights.push(1.0);
    this.knots.push(this.knots.length);
    this.setDirty();
  }

  removeControlPointAt(i: number) {
    this.controlPoints.splice(i, 1);
    this.weights.splice(i, 1);
    this.knots.splice(i + this.order + 1, 1);
    for (; i < this.knots.length; ++i)
      this.knots[i] -= 1;
  }

  setDirty() {
    this.dirty = -1.0;
  }

  getType() : LineType {
    return LineType.BSpline;
  }
}

export default BSpline;
