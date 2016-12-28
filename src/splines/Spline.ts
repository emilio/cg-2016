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
    this.knots = [0, 0, 0, 0, 0]; // We need n + 1 more here.
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
      return t => {
        if (t >= this.knots[i] && t < this.knots[i + 1])
          return 1;
        return 0;
      };
    }
    // General case: N_{i, k}.
    return t => {
      // This is a bit less straight-forward than the formulae, to avoid
      // division by zero.
      let firstDivisor = this.knots[i + n - 1] - this.knots[i];
      let firstPart = 0.0;
      if (firstDivisor != 0.0)
        firstPart = (t - this.knots[i]) / firstDivisor * this.calculateBasisFunction(i, n - 1)(t);

      let secondDivisor = this.knots[i + n] - this.knots[i + 1];
      let secondPart = 0.0;
      if (secondDivisor != 0.0)
        secondPart = (this.knots[i + n] - t) / secondDivisor * this.calculateBasisFunction(i + 1, n - 1)(t);
      return firstPart + secondPart;
    };
  }

  reevaluate() {
    this.evaluated = new PolyLine();
    let evaluateAt = t => {
      // http://www.cl.cam.ac.uk/teaching/2000/AGraphHCI/SMEG/node4.html#eqn:BasicBspline
      let v = new Point(0, 0);
      for (let i = 0; i < this.controlPoints.length; ++i) {
        let basis = this.calculateBasisFunction(i, this.order - 1)(t);
        // TODO(emilio): We probably need to un-normalize afterwards (weight is
        // always 1 for now so it doesn't matter).
        console.log(t, basis);
        v = Point.add(v, Point.mul(Point.mul(this.controlPoints[i], this.weights[i]), basis));
      }
      return v;
    };

    const EVALUATION_DELTA: number = 0.05;
    for (let s = 0.0; s <= 1.0; s+= EVALUATION_DELTA)
      this.evaluated.controlPoints.push(evaluateAt(s));
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
    return this.evaluatedLine().contains(p);
  }

  isDirty() : boolean {
    return this.dirty;
  }

  addControlPoint(p: Point) {
    this.controlPoints.push(p);
    this.weights.push(1.0);
    this.knots.push(this.controlPoints.length);
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
