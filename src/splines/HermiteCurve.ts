import { ContainmentResult, Point, Line, LineType } from "Line";
import PolyLine from "PolyLine";

const EVALUATION_DELTA: number = 0.05;

class HermiteCurve implements Line {
  controlPoints: Array<Point>;

  // Each control point has one tangent vector, that represents the direction
  // and speed of how the line goes through the points.
  //
  // TODO: These should actually be of type |Vector|, but we reuse |Point|
  // because we're on a rush.
  tangents: Array<Point>;
  evaluated: PolyLine;
  dirty: boolean;
  constructor() {
    this.controlPoints = new Array();
    this.tangents = new Array();
    this.evaluated = new PolyLine();
    this.dirty = false;
  }

  evaluateSegment(i: number, evaluatedControlPoints: Array<Point>) {
    let p1 = this.controlPoints[i - 1];
    let t1 = this.tangents[i - 1];
    let p2 = this.controlPoints[i];
    let t2 = this.tangents[i];

    // Now we just apply the basis functions like this for those
    // points/vectors:
    //
    // h1(s) =  2s^3 - 3s^2 + 1
    // h2(s) = -2s^3 + 3s^2
    // h3(s) =   s^3 - 2s^2 + s
    // h4(s) =   s^3 -  s^2
    evaluatedControlPoints.push(p1);
    for (let s = 0.0; s < 1.0; s += EVALUATION_DELTA) {
      let s2 = s * s;
      let s3 = s * s * s;

      let h1 = 2 * s3 - 3 * s2 + 1;
      let h2 = - 2 * s3 + 3 * s2;
      let h3 = s3 - 2 * s2 + s;
      let h4 = s3 - s2;

      evaluatedControlPoints.push(new Point(
        h1 * p1.x + h2 * p2.x + h3 * t1.x + h4 * t2.x,
        h1 * p1.y + h2 * p2.y + h3 * t1.y + h4 * t2.y,
      ));
    }
    evaluatedControlPoints.push(p2);
  }

  removeControlPointAt(i: number) {
    this.controlPoints.splice(i, 1);
    this.tangents.splice(i, 1);
    this.setDirty();
  }

  reevaluate() {
    this.evaluated = new PolyLine();
    if (this.controlPoints.length == 0)
      return;
    for (let i = 1; i < this.controlPoints.length; ++i)
      this.evaluateSegment(i, this.evaluated.controlPoints);
  }

  evaluatedLine() : PolyLine {
    if (this.dirty)
      this.reevaluate();
    this.dirty = false;
    return this.evaluated;
  }

  addControlPoint(p: Point) {
    this.controlPoints.push(p);
    // This will make it seem like a polyline.
    this.tangents.push(new Point(0, 0));
    this.setDirty();
  }

  draw(gl: WebGLRenderingContext,
       isSelected: boolean,
       selectedPointIndex: number) {
    this.evaluatedLine().drawLine(gl, isSelected, selectedPointIndex);
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

  getType() : LineType {
    return LineType.Hermite;
  }

  setDirty() {
    this.dirty = true;
  }

  isDirty() : boolean {
    return this.dirty;
  }
}

export default HermiteCurve;
