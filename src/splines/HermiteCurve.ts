import { Point, Line, LineType } from "Line";

class HermiteCurve implements Line {
  controlPoints: Array<Point>;
  tangents: Array<Point>;
  constructor() {
    this.controlPoints = new Array();
    this.tangents = new Array();
  }

  draw() {
    // TODO.
  }

  getType() : LineType {
    return LineType.Hermite;
  }
}

export default HermiteCurve;
