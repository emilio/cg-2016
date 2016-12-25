import BSpline from "Spline";
import HermiteCurve from "HermiteCurve";
import PolyLine from "PolyLine";
import { Line, LineType, Point } from "Line";

class Application {
  lines: Array<Line>;
  currentLineType: LineType;
  currentLineIndex: number;
  gl: WebGLRenderingContext;
  constructor(public lineControl: HTMLSelectElement, public canvas: HTMLCanvasElement) {
    this.currentLineType = LineType.PolyLine;
    this.lines = new Array();
    canvas.width = canvas.height = 800;
    this.currentLineIndex = -1;
    this.gl = canvas.getContext('webgl');
    if (!this.gl)
      throw "Couldn't create WebGL context!";
  }

  createLine(p: Point) {
    let line: Line;
    switch (this.currentLineType) {
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
    line.controlPoints.push(p);
    this.lines.push(line);
    this.currentLineIndex = this.lines.length - 1;
    this.redraw();
  }

  addPointToCurrentLineOrCreate(p: Point) {
    if (this.currentLineIndex === -1 ||
        this.lines[this.currentLineIndex].getType() != this.currentLineType) {
      return this.createLine(p);
    }

    let currentLine = this.lines[this.currentLineIndex];
    currentLine.controlPoints.push(p);
    this.redraw();
  }

  // This does a full redraw of the scene, without any kind of caching.
  //
  // We can do way better than this, but since this isn't run at an animation
  // speed, it's probably not worth.
  redraw() {
    console.log("Redrawing canvas", this.lines);
    this.gl.clear(this.gl.COLOR_BUFFER_BIT);
    for (let line of this.lines) {
      line.draw(this.gl);
    }
  }

  setup() {
    this.lineControl.addEventListener('change', () => {
      switch (this.lineControl.options[this.lineControl.selectedIndex].value) {
        case "": break;
        case "bspline": this.currentLineType = LineType.BSpline; break;
        case "hermite": this.currentLineType = LineType.Hermite; break;
        case "poly": this.currentLineType = LineType.PolyLine; break;
        default: throw "Unexpected line type";
      }
    });
    this.canvas.addEventListener('click', e => {
      // We rely on the canvas being at the top left of the page, otherwise
      // we'd need to do more expensive operations here.
      console.log("Canvas click at: ", e.clientX, e.clientY);
      this.addPointToCurrentLineOrCreate(new Point(e.clientX, e.clientY));
    });
    this.gl.viewport(0, 0, this.canvas.width, this.canvas.height);
    this.gl.clearColor(0.5, 0.5, 0.5, 1.0);
    this.gl.clear(this.gl.COLOR_BUFFER_BIT);
  }
};

export default Application;
