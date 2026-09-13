use <body_thin.scad>

/**
 * TODO
 * - [x] sensor plate
 *.- reset button
 *.- [x] complete the perimeter
 * - figure out how to fix cover to the body 
 *
 */
$fn = 50;
cube1Rotation = [-15, 0, -30];
cube1Offset = [32, -127.5, 0];

globalMove = [-209.55, -99.5, 0];
sensorRadius = 20;
sensolPosition = [67, 13, 0];
wallThickness = 1.2;

resetButtonPosition = [56.15, -13.7, 0];
resetButtonRadius = 2;

module bodyProjection() {
  import("outline.svg");

  translate(-globalMove) {
    translate(sensolPosition) {
      circle(r=sensorRadius - wallThickness / 2, $fn=50);
    }
  }
}

module cuttingcubes() {
  translate([-56.6, 0, 0]) {
    rotate([0, 10, 0]) {
      cube([200, 200, 200], center=true);
    }
  }

  translate(cube1Offset) {
    rotate(cube1Rotation) {
      cube([200, 200, 200], center=true);
    }
  }
}

module cover() {

  globalMove = [-209.55, -99.5, 0];
  wallThickness = 1.2;

  difference() {
    translate(globalMove) {
      difference() {

        union() {
          linear_extrude(8) {
            minkowski() {
              bodyProjection();
              circle(wallThickness);
            }
          }

          linear_extrude(9.4) {
            translate(-globalMove) {
              translate(sensolPosition) {
                circle(r=sensorRadius + wallThickness / 2, $fn=50);
              }
            }
          }
        }

        union() {
          translate([0, 0, -1]) {
            linear_extrude(8) {
              minkowski() {
                bodyProjection();
                circle(0.3);
              }
            }
          }
        }
      }
    }
    cuttingcubes();

    // sensor cutout
    translate([0, 0, 8.5]) {
      translate(sensolPosition) {
        linear_extrude(5) {
          circle(r=sensorRadius + 0.1, $fn=50);
        }
      }
    }

    // sensor cutout vertical
    translate([0, 0, 6]) {
      translate(sensolPosition) {
        linear_extrude(10) {
          circle(r=sensorRadius - 2, $fn=50);
        }
      }
    }

    // reset button cutout
    translate([0, 0, 6]) {
      translate(resetButtonPosition) {
        linear_extrude(10) {
          circle(r=resetButtonRadius, $fn=50);
        }
      }
    }

    //translate([-58, 20, 5]) {
    //  cube([12, 40, 11], center=true);
    //}
  }

  // reset button walls
  translate([0, 0, 3]) {
    translate(resetButtonPosition) {
      linear_extrude(4) {

        difference() {
          circle(r=resetButtonRadius + wallThickness, $fn=50);
          circle(r=resetButtonRadius, $fn=50);
        }
      }
    }
  }

  //cuttingcubes();

  //translate([-68, -5, -1]) {
  //cube([12, 5, 11], center=true);
  //}

  intersection() {
    difference() {
      translate([0, 0, -1]) {
        translate(globalMove) {
          difference() {
            linear_extrude(8) {
              minkowski() {
                import("outline.svg");
                circle(wallThickness);
              }
            }
          }
        }
      }
      cuttingcubes();
    }
    translate([0, 1, 0]) {
      translate(cube1Offset) {
        rotate(cube1Rotation) {
          cube([200, 200, 200], center=true);
        }
      }
    }
  }
}

if (false) {
  cover();
}

if (true) {
  // reset button
  translate([0, 0, 3]) {
    translate(resetButtonPosition) {
      linear_extrude(5) {
        circle(r=resetButtonRadius - 0.2, $fn=50);
      }
    }
  }

  translate([0, 0, 2]) {
    translate(resetButtonPosition) {
      linear_extrude(1) {
        circle(r=resetButtonRadius + wallThickness, $fn=50);
      }
    }
  }
}
