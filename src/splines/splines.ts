import BSpline from "Spline";
import HermiteCurve from "HermiteCurve";
import PolyLine from "PolyLine";
import { DisplayMode, Line, LineType, Matrix4D, Point, Point3D } from "Line";

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
              public hermiteTangentY: HTMLInputElement,
              public uIncrement: HTMLInputElement,
              public revolutionTrigger: HTMLElement,
              public revolutionAxis: HTMLSelectElement,
              public rotationAmount: HTMLInputElement,
              public rotationStep: HTMLInputElement) {}
}

class Application {
  lines: Array<Line>;
  dom: ApplicationDOM;
  draggingPoint: boolean;
  gl: WebGLRenderingContext;
  selection: Selection;
  displayMode: DisplayMode;
  cameraDrag: Point;
  lazyRedrawTimeout: number;

  // Rotations around the origin in _radians_.
  cameraRotationX: number;
  cameraRotationY: number;

  constructor(dom: ApplicationDOM) {
    dom.canvas.width = dom.canvas.height = 800;
    this.dom = dom;
    this.lines = [];
    this.draggingPoint = false;
    this.cameraDrag = null;
    this.lazyRedrawTimeout = null;

    this.cameraRotationX = this.cameraRotationY = 0.0;

    this.selection = new Selection(-1, -1);
    this.displayMode = DisplayMode.Lines;
    this.gl = dom.canvas.getContext('webgl');

    if (!this.gl)
      throw "Couldn't create WebGL context!";
  }

  cameraPosition() : Point3D {
    const CAM_DISTANCE: number = 400;

    let rotX = this.cameraRotationX * Math.PI / 180;
    let rotY = this.cameraRotationY * Math.PI / 180;

    let x = Matrix4D.rotation(rotX, Point3D.XAxis());
    let y = Matrix4D.rotation(rotY, Point3D.YAxis());

    let p = new Point3D(0, 0, -1);

    p = Matrix4D.transformPoint(p, Matrix4D.mul(y, x));

    return Point3D.add(Point3D.mul(p, CAM_DISTANCE), new Point3D(400, 400, 0));
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
        line = new BSpline();
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

  lazyRedraw() {
    const LAZY_REDRAW_TIMEOUT: number = 500;

    if (this.lazyRedrawTimeout)
      clearTimeout(this.lazyRedrawTimeout);

    this.lazyRedrawTimeout = setTimeout(() => {
      this.redraw();
    }, LAZY_REDRAW_TIMEOUT);
  }

  // This does a full redraw of the scene, without any kind of caching.
  //
  // We can do way better than this, but since this isn't run at an animation
  // speed, it's probably not worth.
  //
  // We may want to redraw when the cursor is moving, in that case we may need
  // some perf work.
  redraw() {
    if (this.lazyRedrawTimeout)
      clearTimeout(this.lazyRedrawTimeout);
    this.lazyRedrawTimeout = null;

    this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);
    let uIncrement = Math.min(Math.max(this.dom.uIncrement.valueAsNumber, 0.00001), 1.0);

    for (let i = 0; i < this.lines.length; ++i) {
      let isSelected = this.selection.lineIndex == i;
      if (this.displayMode == DisplayMode.Lines) {
        this.lines[i].draw(this.gl, isSelected,
                           isSelected ? this.selection.pointIndex : -1,
                           uIncrement);
      } else {
        let camera = this.cameraPosition();
        let proj = Matrix4D.ortho(this.gl.canvas.width, -10000, 10000);
        let axisToRotate = Point3D.YAxis();
        let DOMAxis = this.dom.revolutionAxis.options[this.dom.revolutionAxis.selectedIndex].value;
        console.log(DOMAxis);
        switch (DOMAxis) {
          case "x":
            axisToRotate = Point3D.XAxis();
            break;
          case "z":
            axisToRotate = Point3D.ZAxis();
            break;
        }

        let up = Point3D.YAxis();
        let view = Matrix4D.lookAt(camera, new Point3D(400.0, 400.0, 0), up);
        console.log(view);
        console.log(proj);
        let viewProj = Matrix4D.mul(proj, view);
        console.log(viewProj);
        let rotationStep = Math.min(Math.max(this.dom.rotationStep.valueAsNumber, 0.00001), 360);

        this.lines[i]
          .drawRevolutionSurface(this.gl, axisToRotate, viewProj,
                                 uIncrement,
                                 this.dom.rotationAmount.valueAsNumber,
                                 rotationStep);
      }
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

    this.gl.enable(this.gl.DEPTH_TEST);
    this.gl.depthFunc(this.gl.LEQUAL);

    // NB: We rely on the canvas being at the top left of the page, otherwise
    // we'd need to do more expensive operations here.
    //
    // 200ms to avoid triggering a click that destroys our state right before a
    // dblclick.
    const CLICK_GRACE_PERIOD: number = 200;

    this.dom.canvas.addEventListener('mousedown', e => {
      if (this.displayMode == DisplayMode.Revolution) {
        this.cameraDrag = new Point(e.clientX, e.clientY);
        return;
      }
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
        this.draggingPoint = true;
    });

    this.dom.canvas.addEventListener('mousemove', e => {
      const CAMERA_DRAG_PER_PX: number = 0.1;
      if (this.displayMode == DisplayMode.Revolution) {
        if (!this.cameraDrag)
          return;
        let thisPoint = new Point(e.clientX, e.clientY);
        let diff = Point.substract(thisPoint, this.cameraDrag);
        this.cameraRotationX += diff.y * CAMERA_DRAG_PER_PX;
        this.cameraRotationY += diff.x * CAMERA_DRAG_PER_PX;
        this.cameraDrag = thisPoint;
        this.lazyRedraw();
        return;
      }
      // TODO: Throttle this, or make drawing really fast!
      if (!this.draggingPoint || this.selection.pointIndex === -1)
        return;
      let line = this.lines[this.selection.lineIndex];
      let p = line.controlPoints[this.selection.pointIndex];
      p.x = e.clientX;
      p.y = e.clientY;

      line.setDirty();
      this.redraw();
    });

    this.dom.canvas.addEventListener('mouseup', e => {
      if (this.displayMode == DisplayMode.Revolution) {
        this.cameraDrag = null;
        return;
      }
      let wasDragging = this.draggingPoint;
      this.draggingPoint = false;
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
      if (this.displayMode == DisplayMode.Revolution)
        return;

      if (e.keyCode === 46) { // Del
        if (this.selection.lineIndex === -1)
          return;

        let line = this.lines[this.selection.lineIndex];

        // If there's no point selected, or it's the last control point, delete
        // the line.
        if (this.selection.pointIndex === -1 || line.controlPoints.length === 1) {
          // TODO: Tear down cached buffers and stuff? They go away
          // automatically on GC.
          this.lines.splice(this.selection.lineIndex, 1);
          this.selection.lineIndex = -1;
        } else {
          line.removeControlPointAt(this.selection.pointIndex);
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
      if (this.displayMode == DisplayMode.Lines) {
        let p = new Point(e.clientX, e.clientY);
        this.setSelection(
          this.addPointToCurrentLineOrCreate(p, this.currentLineType()));
      }
      this.displayMode = DisplayMode.Lines;
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

    [
      [ 'uIncrement', false ],
      [ 'rotationAmount', true ],
      [ 'rotationStep', true ],
    ].forEach(entry => {
      // FIXME(emilio): https://github.com/Microsoft/TypeScript/issues/13578
      let elem = this.dom[<string>entry[0]];
      let onlyOnRevolutionMode = entry[1];

      elem.addEventListener('input', e => {
        if (onlyOnRevolutionMode && this.displayMode != DisplayMode.Revolution)
          return;
        this.lazyRedraw();
      });
    });

    this.gl.viewport(0, 0, this.dom.canvas.width, this.dom.canvas.height);
    this.gl.clearColor(0.5, 0.5, 0.5, 1.0);
    this.gl.clear(this.gl.COLOR_BUFFER_BIT);

    this.dom.revolutionTrigger.addEventListener('click', e => {
      this.displayMode = DisplayMode.Revolution;
      this.redraw();
      e.preventDefault();
    });

    this.dom.revolutionAxis.addEventListener('change', e => {
      if (this.displayMode != DisplayMode.Revolution)
        return;
      this.redraw();
    });

    // Testing...
    //
    // This should produce a circle (taken from
    // https://en.wikipedia.org/wiki/Non-uniform_rational_B-spline#Example:_a_circle),
    // so let's see.
    //
    // let halfSquareOfTwo = Math.sqrt(2) / 2;
    // let spline = new BSpline();
    // spline.knots = [0, 0, 0, Math.PI / 2, Math.PI / 2, Math.PI, Math.PI,  3 * Math.PI / 2, 3 * Math.PI / 2, 2 * Math.PI, 2 * Math.PI, 2 * Math.PI]
    // spline.controlPoints = [
    //   new Point(1, 0),
    //   new Point(1, 1),
    //   new Point(0, 1),
    //   new Point(-1, 1),
    //   new Point(-1, 0),
    //   new Point(-1, -1),
    //   new Point(0, -1),
    //   new Point(1, -1),
    //   new Point(1, 0),
    // ];
    // spline.weights = [1, halfSquareOfTwo, 1, halfSquareOfTwo, 1, halfSquareOfTwo, 1, halfSquareOfTwo, 1];
    // spline.setDirty();
    // this.lines.push(spline);
    // this.redraw();
  }
};

export {Application, ApplicationDOM};
