use <body_thin.scad>

module cuttingcubes() {
  translate([-55, 0, 0]) {
    rotate([0, 10, 0]) {
      cube([200, 200, 200], center=true);
    }
  }

  translate([40, -120.5, 0]) {
    rotate([-15, 0, -24]) {
      cube([200, 200, 200], center=true);
    }
  }
}

module cover() {

  difference() {
  translate([-209.25, -99.5, 0]) {
    difference() {
      linear_extrude(8) {
        minkowski() {
          import("outline.svg");
          circle(2);
        }
      }

      translate([0, 0, -1]) {
        linear_extrude(8) {
          minkowski() {
            import("outline.svg");
            circle(0.1);
          }
        }
      }
    }
  }
cuttingcubes();
  //translate([-58, 20, 5]) {
  //  cube([12, 40, 11], center=true);
  //}


  }

    //cuttingcubes();


    //translate([-68, -5, -1]) {
    //cube([12, 5, 11], center=true);
  //}

  /*
  intersection() {
    difference() {
      translate([-146, -122, -1]) {
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
  } */
}

cover();
