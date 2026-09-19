use <body_thin.scad>

/**
* [x] screw plates
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
coverHeight = 9;

function getResetButtonPosition() = resetButtonPosition;
function getResetButtonRadius() = resetButtonRadius;
function getCoverHeight() = hoverHeight;

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

module usbPort() {
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

module innerWalls(coverHeight) {
  intersection() {
    difference() {
      translate([0, 0, -1]) {
        translate(getGlobalMove()) {
          linear_extrude(coverHeight) {
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
}

module resetButtonWalls(coverHeight) {

  // reset button walls
  translate([0, 0, 3]) {
    translate(resetButtonPosition) {
      linear_extrude(coverHeight-3) {

        difference() {
          circle(r=resetButtonRadius + getWallThickness());
          circle(r=resetButtonRadius);
        }
      }
    }
  }
}

module cover() {

  difference() {
    translate(getGlobalMove()) {
      difference() {

        linear_extrude(coverHeight) {
          minkowski() {
            bodyProjection();
            circle(getWallThickness());
          }
        }

        union() {
          translate([0, 0, -1.5]) {
            linear_extrude(coverHeight) {
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

    usbPort();
  }

  resetButtonWalls(coverHeight);

  //cuttingcubes();

  //translate([-68, -5, -1]) {
  //cube([12, 5, 11], center=true);
  //}

  innerWalls(coverHeight);

  // hooks
  intersection() {
    hooksBlock();
    cuttingcubes();
  }
}

if (true) {
  cover();
}
