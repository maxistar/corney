use <body_thin.scad>


module cover() {

  module cuttingcubes() {
    translate([55, 0, 0]) {
      rotate([0, -10, 0]) {
        cube([200, 200, 200], center=true);
      }
    }

    translate([0, -107.5, 0]) {
      rotate([-15, 0, 24]) {
        cube([200, 200, 200], center=true);
      }
    }
  }

  difference() {
    translate([-67.25, -256, 0]) {
      difference() {
        linear_extrude(12) {
          minkowski() {
            import("outline.svg");
            circle(2);
          }
        }

        translate([0, 0, -1]) {
          linear_extrude(12) {
            minkowski() {
              import("outline.svg");
              circle(0.1);
            }
          }
        }
      }
    }
    cuttingcubes();
    translate([-58, 20, 5]) {
      cube([12, 40, 11], center=true);
    }

    translate([-68, -5, -1]) {
      cube([12, 5, 11], center=true);
    }
  }

  intersection() {
    difference() {
      translate([-65.35, -256, -1]) {
        difference() {
          linear_extrude(13) {
            minkowski() {
              import("outline.svg");
              circle(2);
            }
          }
        }
      }
      cuttingcubes();
    }
    translate([0, -102.7, 0]) {
      rotate([-15, 0, 24]) {
        cube([200, 200, 200], center=true);
      }
    }
  }
}


cover();