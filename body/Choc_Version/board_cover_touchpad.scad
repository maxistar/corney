use <body_thin.scad>
use <body_thin_touchpad.scad>
use <board_cover.scad>

coverHeight = 13;
resetButtonPosition = getResetButtonPosition();
resetButtonRadius = getResetButtonRadius();

module cover_touchpad() {

  difference() {

    difference() {

      linear_extrude(coverHeight) {
        minkowski() {
          bodyProjectionNormalizedTouchpad();
          circle(getWallThickness());
        }
      }

      translate([0, 0, -1.5]) {
        linear_extrude(coverHeight) {
          minkowski() {
            bodyProjectionNormalizedTouchpad();
            circle(0.3);
          }
        }
      }
    }

    cuttingcubes();

    // sensor cutout
    translate([0, 0, coverHeight]) {
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
      translate(getResetButtonPosition()) {
        linear_extrude(10) {
          circle(r=getResetButtonRadius());
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

  // round border
  translate([0, 0, -1]) {

    translate(getSensorPosition()) {

      difference() {
        linear_extrude(coverHeight) {
          circle(r=getSensorRadius());
        }

        linear_extrude(coverHeight * 3, center=true) {
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

  // screw holders
  screwHolders();
}

module screwHolders() {

  holderPosition1 = getHolderPositions()[0];
  holderPosition2 = getHolderPositions()[1];

  difference() {
    intersection() {
      linear_extrude(height=40, center=true) {
        translate(getSensorPosition()) {
          circle(r=getSensorRadius() + getWallThickness() / 2);
        }
      }
      union() {
        translate(holderPosition1) {
          cylinder(h=5, r=3, center=true);
        }

        hull() {
          translate([0, 0, 1]) {
            translate(holderPosition1) {
              cylinder(h=5, r=3, center=true);
            }
          }

          translate([73, -10, 10]) {
            cylinder(h=5, r=3, center=true);
          }
        }

        translate(holderPosition2) {
          cylinder(h=5, r=3, center=true);
        }

        hull() {
          translate([0, 0, 1]) {
            translate(holderPosition2) {
              cylinder(h=5, r=3, center=true);
            }
          }

          translate([73, 37, 10]) {
            cylinder(h=5, r=3, center=true);
          }
        }
      }
    }

    translate(holderPosition2) {
      cylinder(h=500, r=0.5, center=true);
    }
    translate(holderPosition1) {
      cylinder(h=500, r=0.5, center=true);
    }
  }
}

cover_touchpad();
