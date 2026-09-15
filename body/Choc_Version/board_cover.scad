use <body_thin.scad>

/**
* [ ] screw plates
* [x] side hooks
*
*
*
*
*
*/

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

cube2Rotation = [0, 0, 0];
cube2Offset = [-56.6, 0, 0];

resetButtonPosition = [56.15, -13.7, 0];
resetButtonRadius = 2;

hoverHeight = 13;

module cuttingcubes() {
  // cutting cube near main cluster
  translate(cube2Offset) {
    rotate([0, 0, 0]) {
      cube([206, 200, 200], center=true);
    }
  }

  // cutting cubes near the thumb cluster
  translate(cube1Offset) {
    rotate(cube1Rotation) {
      cube([200, 200, 200], center=true);
    }
  }
}

module verticalWall() {
  difference() {
    // cutting cube near main cluster
    translate([1, 0, 0]) {
      translate(cube2Offset) {
        rotate([0, 0, 0]) {
          cube([206, 200, 200], center=true);
        }
      }
    }

    // cutting cube near main cluster
    translate(cube2Offset) {
      rotate([0, 0, 0]) {
        cube([206, 210, 210], center=true);
      }
    }
  }
}

module cover() {

  difference() {
    translate(getGlobalMove()) {
      difference() {

        union() {
          linear_extrude(hoverHeight) {
            minkowski() {
              bodyProjection();
              circle(getWallThickness());
            }
          }

          linear_extrude(hoverHeight) {
            translate(-getGlobalMove()) {
              translate(getSensorPosition()) {
                circle(r=getSensorRadius() + getWallThickness() / 2);
              }
            }
          }
        }

        union() {
          translate([0, 0, -1.5]) {
            linear_extrude(hoverHeight) {
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
    translate([0, 0, hoverHeight]) {
      translate([0, 0, -1]) {
        translate(getSensorPosition()) {
          linear_extrude(5) {
            circle(r=getSensorRadius() + 0.1);
          }
        }
      }
    }

    // sensor cutout vertical
    translate([0, 0, 6]) {
      translate(getSensorPosition()) {
        linear_extrude(10) {
          circle(r=getSensorRadius() - 2);
        }
      }
    }

    // reset button cutout
    translate([0, 0, 6]) {
      translate(resetButtonPosition) {
        linear_extrude(10) {
          circle(r=resetButtonRadius);
        }
      }
    }

    //translate([-58, 20, 5]) {
    //  cube([12, 40, 11], center=true);
    //}

    // USB-C cut
    translate([60, 28, 3.5]) {
      rotate([0, 90, 90]) {
        hull() {
          cylinder(h=10, r=2.5, center=false);
          translate([0, 7, 0])
            cylinder(h=10, r=2.5, center=false);
        }
      }
    }
  }

  // reset button walls
  translate([0, 0, 3]) {
    translate(resetButtonPosition) {
      linear_extrude(9) {

        difference() {
          circle(r=resetButtonRadius + getWallThickness());
          circle(r=resetButtonRadius);
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
        translate(getGlobalMove()) {
          linear_extrude(hoverHeight) {
            minkowski() {
              import("outline.svg");
              circle(getWallThickness());
            }
          }
        }
      }
      cuttingcubes();
    }
    union() {
      translate([0, 1, 0]) {
        translate(cube1Offset) {
          rotate(cube1Rotation) {
            cube([200, 200, 200], center=true);
          }
        }
      }

      verticalWall();
    }
  }

  // round border
  translate([0, 0, -1]) {

    translate(getSensorPosition()) {

      difference() {
        linear_extrude(hoverHeight) {
          circle(r=getSensorRadius());
        }

        linear_extrude(hoverHeight * 3, center=true) {
          circle(r=getSensorRadius() - getWallThickness() / 2);
        }

        translate([-49, 0, 0]) {
          cube([100, 100, 100], center=true);
        }
      }
    }
  }

  // hooks
  intersection() {
    hooksBlock();
    cuttingcubes();
  }
}



module resetButton() {
  // reset button
  translate([0, 0, 0]) {
    translate(resetButtonPosition) {
      linear_extrude(13) {
        circle(r=resetButtonRadius - 0.2);
      }
    }
  }

  translate([0, 0, -1]) {
    translate(resetButtonPosition) {
      linear_extrude(1) {
        circle(r=resetButtonRadius + getWallThickness());
      }
    }
  }
}

if (true) {
  cover();
}

if (false) {
  resetButton();
}
