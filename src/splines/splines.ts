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

class ApplicationDOM {
  constructor(public canvas: HTMLCanvasElement,
              public config: HTMLElement,
              public lineControl: HTMLSelectElement,
              public hermiteTangentX: HTMLInputElement,
              public hermiteTangentY: HTMLInputElement) { }
}

class Application {
  lines: Array<Line>;
  dom: ApplicationDOM;
  dragging: boolean;
  gl: WebGLRenderingContext;
  selection: Selection;

  constructor(dom: ApplicationDOM) {
    dom.canvas.width = dom.canvas.height = 800;
    this.dom = dom;
    this.lines = [];
    this.dragging = false;
    this.selection = new Selection(-1, -1);
    this.gl = dom.canvas.getContext('webgl');

    if (!this.gl)
      throw "Couldn't create WebGL context!";
  }

  setSelection(selection: Selection) {
    this.selection = selection;
    let className = "";
    if (selection.lineIndex !== -1) {
      let line = this.lines[selection.lineIndex];
      let lineType;
      switch (line.getType()) {
        case LineType.PolyLine: lineType = "poly"; break;
        case LineType.Hermite:  lineType = "hermite"; break;
        case LineType.BSpline:  lineType = "bspline"; break;
      }
      className += "line-" + lineType;

      if (selection.pointIndex !== -1) {
        className += " point-selected";

        switch (line.getType()) {
          case LineType.Hermite: {
            let tangent = (<HermiteCurve>line).tangents[selection.pointIndex]
            this.dom.hermiteTangentX.valueAsNumber = tangent.x;
            this.dom.hermiteTangentY.valueAsNumber = tangent.y;
            break;
          }
          case LineType.BSpline:
          case LineType.Hermite:
            break;
        }
      }
    }

    this.dom.config.className = className;
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
    switch (this.dom.lineControl.options[this.dom.lineControl.selectedIndex].value) {
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
    // NB: We rely on the canvas being at the top left of the page, otherwise
    // we'd need to do more expensive operations here.
    //
    // 200ms to avoid triggering a click that destroys our state right before a
    // dblclick.
    const CLICK_GRACE_PERIOD: number = 200;

    this.dom.canvas.addEventListener('mousedown', e => {
      if (clickTimeout) {
        clearTimeout(clickTimeout);
        clickTimeout = null;
      }
      if (this.selection.pointIndex === -1)
        return;
      let line = this.lines[this.selection.lineIndex];
      let p = line.controlPoints[this.selection.pointIndex];
      let mouse = new Point(e.clientX, e.clientY);
      if (p.near(mouse, 5))
        this.dragging = true;
    });

    this.dom.canvas.addEventListener('mousemove', e => {
      // TODO: Throttle this, or make drawing really fast!
      if (!this.dragging || this.selection.pointIndex === -1)
        return;
      let line = this.lines[this.selection.lineIndex];
      let p = line.controlPoints[this.selection.pointIndex];
      p.x = e.clientX;
      p.y = e.clientY;

      line.setDirty();
      this.redraw();
    });

    this.dom.canvas.addEventListener('mouseup', e => {
      let wasDragging = this.dragging;
      this.dragging = false;
      if (clickTimeout) {
        clearTimeout(clickTimeout);
        clickTimeout = null;
      }

      if (wasDragging)
        return;

      clickTimeout = setTimeout(() => {
        console.log('clickTimeout');
        let p = new Point(e.clientX, e.clientY);
        let oldSelection = this.selection;
        this.setSelection(this.stateAtPoint(p));
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

    this.dom.canvas.addEventListener('dblclick', e => {
      console.log('dblclick');
      if (clickTimeout)
        clearTimeout(clickTimeout);
      clickTimeout = null;
      let p = new Point(e.clientX, e.clientY);
      this.setSelection(
        this.addPointToCurrentLineOrCreate(p, this.currentLineType()));
      this.redraw();
    });

    [
      ['hermiteTangentX', 'x'],
      ['hermiteTangentY', 'y']
    ].forEach(el => {
      let element = this.dom[el[0]];
      let propToOverride = el[1];
      element.addEventListener('input', e => {
        let newValue = element.valueAsNumber;
        let line = <HermiteCurve>this.lines[this.selection.lineIndex];
        let p = line.tangents[this.selection.pointIndex];
        p[propToOverride] = newValue;
        line.setDirty();
        this.redraw();
      });

      element.addEventListener('keypress', function(e) {
        // Stop reaching the document event listener that would remove lines.
        e.stopPropagation();
      });
    });

    this.gl.viewport(0, 0, this.dom.canvas.width, this.dom.canvas.height);
    this.gl.clearColor(0.5, 0.5, 0.5, 1.0);
    this.gl.clear(this.gl.COLOR_BUFFER_BIT);
  }
};

export {Application, ApplicationDOM};
