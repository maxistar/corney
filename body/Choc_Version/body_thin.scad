//hull() {
$fn = 20;
magnet_radius = 4 / 2 + 0.1;

module bottom_panel(hide_side_magnets = false) {
  translate([-0, -0, 9.5]) {
    //translate([0, 0, 13]) {
    rotate([-3, -1, 0]) {
      translate([-20, 0, -12.8]) {
        cube([300, 200, 15], center=true);
      }

      translate([0, 0, 0]) {
        legs();
      }

      magnets2(hide_side_magnets1=hide_side_magnets);

      scale([1, -1, 1]) {
        magnets2(hide_side_magnets1=hide_side_magnets);
      }
    }
    //}
  }
}

module magnets2(hide_side_magnets1 = false) {
  leg1 = [65.0, -22, 15];
  translate(leg1)
    cylinder(h=40, r=magnet_radius, $fn=50, center=true);

  if (!hide_side_magnets1) {
    rotate([0, 0, 180])
      translate(leg1)
        cylinder(h=40, r=magnet_radius, $fn=50, center=true);
  }
  translate([0, -22, 15])
    cylinder(h=40, r=magnet_radius, $fn=50, center=true);
}

module battery() {
  cube([25, 43, 5]);
}

module legs() {
  legmove1 = [43, 21.0, 0];
  //legmove2 = [43, 11.0, 0];
  translate([0, 0, -5.0]) {
    translate(legmove1) {
      leg3();
    }

    scale([-1, 1, 1]) {
      translate(legmove1) {
        leg3();
      }
    }

    scale([-1, -1, 1]) {
      translate(legmove1) {
        leg3();
      }
    }

    scale([1, -1, 1]) {
      translate(legmove1) {
        leg3();
      }
    }

    //    scale([1, -1, 1]) {
    //  translate(legmove2) {
    //    leg();
    //  }
    //}
    //translate([102, 14.5, 0]) {
    //  leg();
    //}
  }

  module leg3() {
    leg();

    translate([0, 10, 0]) {
      leg();
    }

    translate([0, -10, 0]) {
      leg();
    }

    translate([0, 20, 0]) {
      leg();
    }
  }

  /*
  translate([-64, -25, -7.5]) {

    rotate([0, 0, -40]) {
      translate([2, -7, 0]) {
        leg();
      }
      translate([2, 2, 0]) {
        leg();
      }
    }

    translate([102, 5.5, 0]) {
      leg();
    }
    translate([102, 14.5, 0]) {
      leg();
    }

    translate([-5, 56, 0]) {
      leg();
    }
    translate([-5, 46, 0]) {
      leg();
    }

    translate([102, 46, 0]) {
      leg();
    }
    translate([102, 55, 0]) {
      leg();
    }
  } */
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

module body() {

  difference() {
    translate([-209.25, -99.5, 0]) {
      union() {

        difference() {
          linear_extrude(15.5) {
            minkowski() {
              import("outline.svg");
              circle(2);
            }
          }

          translate([0, 0, 9]) {
            linear_extrude(10) {
              minkowski() {
                import("outline.svg");
                circle(0.1);
              }
            }
          }

          //translate([3, 0, 6]) {
          //  battery();
          //}

          translate([6, 270, 13])
            cube([8, 30, 10]);
        }
        

        linear_extrude(12) {
          minkowski() {
            import("outline_empty.svg");
            circle(0.1);
          }
        }
      }
    }

    bottom_panel();
  }
}

//translate([0, 230, 7])
//  cube([3, 3, 3]);

//bottom_panel();

//difference() {



//bodyoutline();

//translate([-119.4, 0, 0]) {
//  cube([100, 100, 100], center=true);
//}

//translate([50,150,-10]) {
//  cube([100,100,100]);
//}
//}

module panel() {
  translate([-146, -122, 13]) {
    difference() {

      linear_extrude(1.5) {
        //minkowski() {
        import("plate_outline.svg");
        //circle(0.1);
        //}
      }

      translate([0, 0, -5])
        linear_extrude(10) {
          import("buttons.svg");
        }

      translate([0, 0, -5])
        linear_extrude(10) {
          import("holes.svg");
        }
    }
  }
}



//translate([0, 0, 15]) {
//  cover();
//}

//panelbuttons(fullheight=false);

//outline();

//cover();
body();