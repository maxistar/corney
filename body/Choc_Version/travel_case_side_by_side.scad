use <twokeyboards_side_by_side.scad>
use <board_assembled_touchpad.scad>
use <board_assembled.scad>
use <body_thin.scad>
use <_auxiliary.scad>




module internalShape() {
  //difference() {
  hull() {
  twokeyboards();
}

  /*
    hull() {
      rotate(-getBoardTiltingAngle()) {
        panelbuttonsmoved(simplified=true);
        // sensor
      }
    } */

    //rotate(-getBoardTiltingAngle()) {
    //  linear_extrude(100, center=true) {
    //    bodyProjectionNormalizedTouchpad();
    //  }
    //}
    /*
    difference() {
      cube([300, 300, 80], center=true);
      rotate(-getBoardTiltingAngle()) {
        linear_extrude(100, center=true) {
          bodyProjectionNormalizedTouchpad();
        }
      }
    }*/
  //}
}

/*
minkowski() {
  internalShape();
  sphere(0.1);
} */

module internalShapeRendered() {
  import("internalshape.stl", $fn=3);
}


difference() {
  minkowski() {
    internalShape();
    sphere(1);
  }

  minkowski() {
    internalShape();
    sphere(0.1);
  }

  translate([0, 0, -100])
    cube([300, 300, 200], center=true);
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
