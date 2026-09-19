use <body_thin.scad>

holderPosition1 = [73, -1, 0];
holderPosition2 = [73, 27, 0];
function getHolderPositions() = [holderPosition1, holderPosition2];

module bodyTouchpad() {
  globalMove = getGlobalMove();
  wallThickness = getWallThickness();

  difference() {
    translate(globalMove) {
      union() {

        difference() {
          linear_extrude(15.5) {
            minkowski() {
              bodyProjectionTouchpad();
              circle(wallThickness);
            }
          }

          translate([0, 0, 9]) {
            linear_extrude(10) {
              minkowski() {
                bodyProjectionTouchpad();
                circle(0.3);
              }
            }
          }

          //translate([3, 0, 6]) {
          //  battery();
          //}

          translate([6, 270, 13])
            cube([8, 30, 10]);
        }

        linear_extrude(11.5) {
          minkowski() {
            difference() {
              import("outline_empty.svg");

              translate(-getGlobalMove()) {
                translate(getSensorPosition()) {
                  circle(r=getSensorRadius());
                }
              }
            }
            circle(0.3);
          }
        }
      }
    }

    bottom_panel();

    // power slider cutout
    translate([77.2, -15, 17]) {
      cube([30, 8, 8], center=true);
      translate([-10, 0, 0]) {
        rotate([0, -90, 0]) {
          rotate([0, 0, 45]) {
            cylinder(h=3, r1=9, r2=1, center=true, $fn=4);
          }
        }
      }
    }

    // screw for cover
    translate(holderPosition1) {
      cylinder(h=200, r=3, center=true);
    }
    translate(holderPosition2) {
      cylinder(h=200, r=3, center=true);
    }
  }

  translate([0, 0, 8.5]) {
    translate(globalMove) {
      stands();
    }
  }

  //crew holder stands
  holderHeight = 8;
  translate([0, 0, 5]) {
    difference() {
      union() {
        translate(holderPosition1) {
          cylinder(h=holderHeight, r=3 + wallThickness);
        }
        translate(holderPosition2) {
          cylinder(h=holderHeight, r=3 + wallThickness);
        }
      }

      translate([0, 0, -1]) {
        union() {
          translate(holderPosition1) {
            cylinder(h=holderHeight, r=3);
          }
          translate(holderPosition2) {
            cylinder(h=holderHeight, r=3);
          }
        }
      }

      union() {
        translate(holderPosition1) {
          cylinder(h=100, r=1.5, center=true);
        }
        translate(holderPosition2) {
          cylinder(h=100, r=1.5, center=true);
        }
      }
    }
  }

  // control helpers
  if (false) {
    translate(globalMove) {
      linear_extrude(height=30) {
        import("controls.svg");
      }
    }
  }
}

bodyTouchpad();
