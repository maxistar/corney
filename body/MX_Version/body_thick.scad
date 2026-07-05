//hull() {
$fn = 20;

module body() {

  difference() {
    union() {

      difference() {
        linear_extrude(14) {
          minkowski() {
            import("outline.svg");
            circle(2);
          }
        }

        translate([0, 0, 7]) {
          linear_extrude(10) {
            minkowski() {
              import("outline.svg");
              circle(0.1);
            }
          }
        }

        translate([3, 225, 2.1]) {
          battery();
        }

        translate([6, 270, 11])
          cube([8, 30, 10]);
      }

      linear_extrude(10) {
        minkowski() {
          import("outline_empty.svg");
          circle(0.1);
        }
      }
    }

    translate([-40, 190, -5.5]) {
      rotate([0, -1, 0]) {
        cube([200, 200, 5]);
      }
    }

    translate([10, 230, 0.5]) {
      rotate([0, -1, 0]) {
        legs();
      }
    }
  }

  module battery() {
    cube([55, 63, 5]);
  }

  //import("outline.svg");

  //translate([10,230,0]) {
  //  legs();
  //}

  module legs() {

    rotate([0, 0, -40]) {
      translate([2, -7, 0]) {
        leg();
      }
      translate([2, 2, 0]) {
        leg();
      }
    }

    translate([104, 5.5, 0]) {
      leg();
    }
    translate([104, 14.5, 0]) {
      leg();
    }

    translate([-5, 56, 0]) {
      leg();
    }
    translate([-5, 46, 0]) {
      leg();
    }

    translate([104, 46, 0]) {
      leg();
    }
    translate([104, 55, 0]) {
      leg();
    }
  }

  module leg() {

    linear_extrude(2, center=true) {
      hull() {
        circle(4);

        translate([14, 0, 0]) {
          circle(4);
        }
      }
    }
  }
}

body();

