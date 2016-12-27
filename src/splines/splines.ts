import BSpline from "Spline";
import HermiteCurve from "HermiteCurve";
import PolyLine from "PolyLine";
import { Line, LineType, Point } from "Line";

class Selection {
  constructor(public lineIndex: number, public pointIndex: number) {}

  equals(other: Selection) : boolean {
    return this.lineIndex == other.lineIndex &&
      this.pointIndex == other.pointIndex;
  }
}

class Application {
  lines: Array<Line>;
  gl: WebGLRenderingContext;
  selection: Selection;
  constructor(public lineControl: HTMLSelectElement, public canvas: HTMLCanvasElement) {
    this.lines = new Array();
    canvas.width = canvas.height = 800;
    this.selection = new Selection(-1, -1);
    this.gl = canvas.getContext('webgl');
    if (!this.gl)
      throw "Couldn't create WebGL context!";
  }

  createLine(p: Point, ty: LineType) : Selection {
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
    return new Selection(this.lines.length - 1, 0);
  }

  addPointToCurrentLineOrCreate(p: Point, ty: LineType) : Selection {
    if (this.selection.lineIndex === -1 ||
        this.lines[this.selection.lineIndex].getType() != ty) {
      return this.createLine(p, ty);
    }

    let currentLine = this.lines[this.selection.lineIndex];
    currentLine.addControlPoint(p);
    return new Selection(this.selection.lineIndex, currentLine.controlPoints.length - 1);
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
    for (let i = 0; i < this.lines.length; ++i) {
      let isSelected = this.selection.lineIndex == i;
      this.lines[i].draw(this.gl, isSelected, isSelected ? this.selection.pointIndex : -1);
    }
  }

  stateAtPoint(p: Point) : Selection {
    // FIXME(emilio): We could try to make this search not suck, but seems
    // harder than what I want to do right now (a quad tree or AABB tree
    // would help I guess).
    for (let i = 0; i < this.lines.length; ++i) {
      // TODO(emilio): Handle overlapping lines, with preference for control
      // points and all that stuff...
      let result = this.lines[i].contains(p);
      if (result.success) {
        console.log(i, result.selectedPointIndex);
        return new Selection(i, result.selectedPointIndex);
      }
    }

    return new Selection(-1, -1);
  }

  setup() {
    var clickTimeout = null;
    // 200ms to avoid triggering a click that destroys our state right before a
    // dblclick.
    const CLICK_GRACE_PERIOD: number = 200;

    // We rely on the canvas being at the top left of the page, otherwise
    // we'd need to do more expensive operations here.
    this.canvas.addEventListener('click', e => {
      console.log('click');
      if (clickTimeout)
        clearTimeout(clickTimeout);
      clickTimeout = setTimeout(() => {
        console.log('clickTimeout');
        let p = new Point(e.clientX, e.clientY);
        let oldSelection = this.selection;
        this.selection = this.stateAtPoint(p);
        if (!oldSelection.equals(this.selection))
          this.redraw();
        clickTimeout = null;
      }, CLICK_GRACE_PERIOD);
    });

    document.addEventListener('keypress', e => {
      if (e.keyCode === 46) { // Del
        if (this.selection.lineIndex === -1)
          return;

        // If there's no point selected, delete the line.
        if (this.selection.pointIndex === -1) {
          // TODO: Tear down cached buffers and stuff? They go away
          // automatically on GC.
          this.lines.splice(this.selection.lineIndex, 1);
          this.selection.lineIndex = -1;
        } else {
          this.lines[this.selection.lineIndex].removeControlPointAt(this.selection.pointIndex);
        }
        this.selection.pointIndex = -1;
        this.redraw();
      }
    });

    this.canvas.addEventListener('dblclick', e => {
      console.log('dblclick');
      if (clickTimeout)
        clearTimeout(clickTimeout);
      clickTimeout = null;
      let p = new Point(e.clientX, e.clientY);
      this.selection =
        this.addPointToCurrentLineOrCreate(p, this.currentLineType());
      this.redraw();
    });

    this.gl.viewport(0, 0, this.canvas.width, this.canvas.height);
    this.gl.clearColor(0.5, 0.5, 0.5, 1.0);
    this.gl.clear(this.gl.COLOR_BUFFER_BIT);
  }
};

export default Application;
