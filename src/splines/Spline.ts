import { ContainmentResult, Line, Point, LineType } from "Line";
import PolyLine from "PolyLine";

/**
 * A non-uniform Rational B-spline curve.
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

  reevaluate() {
    this.evaluated = new PolyLine();
    let evaluateAt = t => {
      // http://www.cl.cam.ac.uk/teaching/2000/AGraphHCI/SMEG/node4.html#eqn:BasicBspline
      let v = new Point(0, 0);
      let basisSum = 0.0;
      for (let i = 0; i < this.controlPoints.length; ++i) {
        let basis = this.calculateBasisFunction(i, this.degree())(t);
        console.log("bas(", i, ", ", this.degree(), ", ", t, "): ", basis);
        basisSum += basis;
        // TODO(emilio): We probably need to un-normalize afterwards (weight is
        // always 1 for now so it doesn't matter).
        // console.log(t, basis, this.controlPoints[i]);
        v = Point.add(v, Point.mul(Point.mul(this.controlPoints[i], this.weights[i]), basis));
      }

      console.log("Sum: ", basisSum);

      return v;
    };

    const EVALUATION_DELTA: number = 0.05;
    for (let t = 0.0; t <= 1.0; t += EVALUATION_DELTA) {
      let s = t * this.knots[this.knots.length - 1];
      this.evaluated.controlPoints.push(evaluateAt(s));
      // break;
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
