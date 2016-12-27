import BSpline from "Spline";
import HermiteCurve from "HermiteCurve";
import PolyLine from "PolyLine";
import { Line, LineType, Point } from "Line";

class Application {
  lines: Array<Line>;
  currentLineIndex: number;
  gl: WebGLRenderingContext;
  constructor(public lineControl: HTMLSelectElement, public canvas: HTMLCanvasElement) {
    this.lines = new Array();
    canvas.width = canvas.height = 800;
    this.currentLineIndex = -1;
    this.gl = canvas.getContext('webgl');
    if (!this.gl)
      throw "Couldn't create WebGL context!";
  }

  createLine(p: Point, ty: LineType) {
    let line: Line;
    switch (ty) {
      case LineType.PolyLine:
        line = new PolyLine();
        break;
      case LineType.Hermite:
        line = new HermiteCurve();
        break;
      case LineType.BSpline:
        line = new BSpline(1.0);
        break;
    }
    line.addControlPoint(p);
    this.lines.push(line);
    line.setDirty();
    this.currentLineIndex = this.lines.length - 1;
    this.redraw();
  }

  addPointToCurrentLineOrCreate(p: Point, ty: LineType) {
    if (this.currentLineIndex === -1 ||
        this.lines[this.currentLineIndex].getType() != ty) {
      return this.createLine(p, ty);
    }

    let currentLine = this.lines[this.currentLineIndex];
    currentLine.addControlPoint(p);
    this.redraw();
  }

  currentLineType() : LineType {
    switch (this.lineControl.options[this.lineControl.selectedIndex].value) {
      case "bspline": return LineType.BSpline;
      case "hermite": return LineType.Hermite;
      case "poly": return LineType.PolyLine;
      default: throw "Unexpected line type";
    }
  }

  // This does a full redraw of the scene, without any kind of caching.
  //
  // We can do way better than this, but since this isn't run at an animation
  // speed, it's probably not worth.
  //
  // We may want to redraw when the cursor is moving, in that case we may need
  // some perf work.
  redraw() {
    console.log("Redrawing canvas", this.lines);
    this.gl.clear(this.gl.COLOR_BUFFER_BIT);
    for (let line of this.lines) {
      line.draw(this.gl);
    }
  }

  setup() {
    this.canvas.addEventListener('click', e => {
      // We rely on the canvas being at the top left of the page, otherwise
      // we'd need to do more expensive operations here.
      console.log("Canvas click at: ", e.clientX, e.clientY);
      this.addPointToCurrentLineOrCreate(new Point(e.clientX, e.clientY), this.currentLineType());
    });
    this.gl.viewport(0, 0, this.canvas.width, this.canvas.height);
    this.gl.clearColor(0.5, 0.5, 0.5, 1.0);
    this.gl.clear(this.gl.COLOR_BUFFER_BIT);
  }
};

export default Application;
