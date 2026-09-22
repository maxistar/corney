use <twokeyboards_side_by_side.scad>
use <board_assembled_touchpad.scad>
use <board_assembled.scad>
use <body_thin.scad>
use <_auxiliary.scad>

/*
hull() {
  twokeyboards();
}
*/

module internalShape() {
  intersection() {
    hull() {
      rotate(-getBoardTiltingAngle()) {
        panelbuttonsmoved(simplified=true);
        // sensor
      }
    }

    rotate(-getBoardTiltingAngle()) {
      linear_extrude(100, center=true) {
        bodyProjectionNormalizedTouchpad();
      }
    }
  }
}

/*
minkowski() {
  internalShape();
  sphere(0.1);
} */

module internalShapeRendered() {
  import("internalshape.stl", $fn=3);
}

minkowski() {
  internalShape();
  sphere(5);
}

/*
difference() {
  minkowski() {
    internalShapeRendered();
    sphere(5);
  }

  //minkowski() {
  //  internalShapeRendered();
  //  sphere(0.1);
  //}

  //translate([0, 0, -10 + 2])
  //  cube([300, 300, 20], center=true);

} */
