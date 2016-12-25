import { Line, Point, LineType } from "Line";

class BSpline implements Line {
  controlPoints: Array<Point>;
  weights: Array<Point>;
  constructor(public degree: number) {
    this.controlPoints = new Array();
    this.weights = new Array();
  }

  draw(gl: WebGLRenderingContext) {
    // TODO
  }

  getType() : LineType {
    return LineType.BSpline;
  }
}

export default BSpline;
