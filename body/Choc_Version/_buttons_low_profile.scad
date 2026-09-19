

module button() {
  rotate([0, 0, 45]) {
    cylinder(h=6, r1=12, r2=10, center=true, $fn=4);
  }
}

module button3() {
  translate([0, 18 * 2, 0]) {
    button();
  }

  translate([0, 18, 0]) {
    button();
  }
  button();
}

module buttons() {

  translate([0, 0, 22]) {

    translate([-19 * 2 - 19, -13.5, 0]) {
      button3();
    }

    translate([-19 - 19, -13.5, 0]) {
      button3();
    }

    translate([0 - 19, -9, 0]) {
      button3();
    }

    translate([18.5 - 19, -6, 0]) {
      button3();
    }

    translate([18.5 * 2 - 19, -8.5, 0]) {
      button3();
    }

    translate([18.5 * 3 - 19, -11, 0]) {
      button3();
    }

    translate([52, -35, 0]) {
      rotate([0, 0, -30]) {
        scale([1, 1.5, 1]) {
          button();
        }
      }
    }

    translate([30, -32, 0]) {
      rotate([0, 0, -13]) {
        button();
      }
    }

    translate([8.2, -29, 0]) {
      rotate([0, 0, -0]) {
        button();
      }
    }
  }
}
