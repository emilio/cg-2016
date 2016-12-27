import { ContainmentResult, Line, Point, LineType } from "Line";
import PolyLine from "PolyLine";

class BSpline implements Line {
  controlPoints: Array<Point>;
  weights: Array<number>;
  evaluated: PolyLine;
  dirty: boolean;
  constructor(public degree: number) {
    this.controlPoints = new Array();
    this.weights = new Array();
    this.evaluated = new PolyLine();
    this.dirty = false;
  }

  reevaluate() {
    this.evaluated = new PolyLine();
    if (this.controlPoints.length == 0)
      return;
    // TODO.
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
    this.weights.push(0);
    this.setDirty();
  }

  removeControlPointAt(i: number) {
    this.controlPoints.splice(i, 1);
    this.weights.splice(i, 1);
  }

  setDirty() {
    this.dirty = true;
  }

  getType() : LineType {
    return LineType.BSpline;
  }
}

export default BSpline;
