import { ContainmentResult, Line, Point, LineType } from "Line";
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
  dirty: boolean;

  constructor() {
    // TODO: Compute the order from the knot vector length et al.
    this.order = 4; // Cubic for now.
    this.knots = [0, 1, 2, 3, 4]; // We need n + 1 more here.
    this.controlPoints = [];
    this.weights = [];
    this.evaluated = new PolyLine();
    this.dirty = false;
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
        if (u >= this.knots[i] && u < this.knots[i + 1])
          r = 1.0;
        console.log("bas(", i, ", ", n, ", ", u, "): ", r);
        return r;
      };
    }
    // General case: N_{i, n}.
    return u => {
      // TODO: This should probably handle division by zero.
      // f_{i, n} = (u - k_i) / (k_{i + n} - k_i)
      let f_i_n = (u - this.knots[i]) / (this.knots[i + n] - this.knots[i]);

      // g_{i, n} = (k_{i+n} - u) / (k_{i+n} - k_i)
      // NB: Here we calculate g_{i + 1, n}
      let g_iplusone_n = (this.knots[i + 1 + n] - u) / (this.knots[i + 1 + n] - this.knots[i + 1]);

      let n_i_nminusone = this.calculateBasisFunction(i, n - 1)(u);
      let n_iplusone_nminusone = this.calculateBasisFunction(i + 1, n - 1)(u);

      console.log("bas(", i, ", ", n, ", ", u, "): ",
                  f_i_n, " * ", n_i_nminusone, " + ",
                  g_iplusone_n, " * ", n_iplusone_nminusone);
      return f_i_n * n_i_nminusone + g_iplusone_n * n_iplusone_nminusone;
    };
  }

  degree() : number {
    return this.order - 1;
  }

  evaluateAt(u: number) : Point {
    let span = this.getKnotSpan(u);
    let basis = this.getBasisFunctions(u);
    console.log(span, basis);
    let p = new Point(0.0, 0.0);
    let degree = this.degree();
    for (let i = 0; i < degree; ++i)
      p = Point.add(p, Point.mul(this.controlPoints[span - degree + i], basis[i]));
    return p;
  }

  reevaluate() {
    const POINTS: number = 1.00 / 0.05;
    this.evaluated = new PolyLine();

    let min = this.knots[0];
    let max = this.knots[this.knots.length - 1];
    let stepSize = (max - min) / POINTS;

    for (let u = 0.0; u < max; u += stepSize) {
      // this.evaluated.controlPoints.push(this.evaluateAt(u));
      this.evaluated.controlPoints.push(Point.mul(this.evaluateAt(u), 800));
    }

    console.log("Evaluated: ", this.evaluated.controlPoints);
    this.dirty = false;
  }

  evaluatedLine() : PolyLine {
    if (this.dirty)
      this.reevaluate();
    return this.evaluated;
  }

  draw(gl: WebGLRenderingContext,
       isSelected: boolean,
       selectedPointIndex: number) {
    // We evaluate a BSpline as a simple polyline, then just paint that. It's
    // possible we could use the GPU to evaluate bezier curves with geometry
    // shaders, but oh well.
    this.evaluatedLine().drawLine(gl, isSelected, selectedPointIndex);
    // Then we draw the control points using the same code as for the polyline.
    let l = new PolyLine(this.controlPoints);
    l.drawPoints(gl, isSelected, selectedPointIndex);
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

  isDirty() : boolean {
    return this.dirty;
  }

  getKnotSpan(u: number) : number {
    let n = this.controlPoints.length - 1;
    let low = this.degree();
    let hi = this.controlPoints.length;
    let mid = Math.floor((low + hi) / 2);

    if (u === this.knots[hi])
      return n;


    while ((u < this.knots[mid] || u >= this.knots[mid + 1])) {
      if (hi == low && hi == mid)
        break;

      if (u < this.knots[mid])
        hi = mid;
      else
        low = mid;

      mid = Math.floor((low + hi) / 2);
    }

    return mid;
  }

  /**
   * Get a vector with `this.controlPoints.length` numbers corresponding to the
   * evaluation of the basis functions at point `u`.
   */
  getBasisFunctions(u: number) : Array<number> {
    let knotIndex = this.getKnotSpan(u);
    let basis = new Array(this.controlPoints.length);
    basis[0] = 1;

    let left = new Array(this.controlPoints.length);
    let right = new Array(this.controlPoints.length);
    for (let j = 1; j <= this.degree(); ++j) {
      left[j] = u - this.knots[knotIndex + 1 - j];
      right[j] = this.knots[knotIndex + j] - u;
      let saved = 0.0;
      for (let r = 0; r < j; ++r) {
        let temp = basis[r] / (right[r + 1] + left[j - r]);
        basis[r] = saved + right[r + 1] * temp;
        saved = left[j - r] * temp;
      }
      basis[j] = saved;
    }
    return basis;
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
    this.dirty = true;
  }

  getType() : LineType {
    return LineType.BSpline;
  }
}

export default BSpline;
