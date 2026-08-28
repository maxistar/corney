/**
 * [x] add screws islands
 * [x] add cover panel
 * [ ] design cover for shield
 * [x] mark buttons positions
 * create a travel case
 * 
 */

//hull() {
$fn = 20;
magnet_radius = 4 / 2 + 0.1;
globalMove = [-209.55, -99.5, 0];

holePosition1 = [161.15, 94.95, 0];
holePosition2 = [161.1, 113.9, 0];
holePosition3 = [237.1, 117.4, 0];
holePosition4 = [250.87, 70.05, 0];

module bottom_panel(hide_side_magnets = false) {
  translate([-0, -0, 10.5]) {
    //translate([0, 0, 13]) {
    rotate([-3, 1, 0]) {
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
  leg1 = [64.4, -21.0, 15];
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
  cube([20, 3.5, 1], center=true);
  /*
  linear_extrude(2, center=true) {
    hull() {
      circle(4);

      translate([14, 0, 0]) {
        circle(4);
      }
    }
  }
  */
}

module stand() {
  difference() {
    union() {
      cylinder(h=4, r=1.8);
      cylinder(h=3, r1=4.5, r2=4);
    }
    cylinder(h=5, r=1);
  }
}

module body() {

  difference() {
    translate(globalMove) {
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

  // power slider cutout
  translate([78, -15, 17]) {
    cube([30, 8, 8], center=true);
    translate([-10, 0, 0]) {
      rotate([0, -90, 0]) {
        rotate([0, 0, 45]) {
          cylinder(h=4, r1=10, r2=1, center=true, $fn=4);
        }
      }
    }
  }

  }

  translate([0, 0, 8.5]) {
    translate(globalMove) {
      stands();
    }
  }

  // control helpers
  //translate(globalMove) {
  //  linear_extrude(height=30) {
  //    import("controls.svg");
  //  }
  //}


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

module holesInPanel() {
  holeRadius = 1;

  translate([0, 0, 0]) {
    //linear_extrude(10) {
    //import("holes.svg");
    //}

    translate(holePosition1)
      cylinder(r=holeRadius, h=10, center=true);

    translate(holePosition2)
      cylinder(r=holeRadius, h=10, center=true);

    translate(holePosition3)
      cylinder(r=holeRadius, h=10, center=true);

    translate(holePosition4)
      cylinder(r=holeRadius, h=10, center=true);
  }
}

module stands() {
  holeRadius = 1;

  translate([0, 0, 0]) {

    translate(holePosition1)
      stand();

    translate(holePosition2)
      stand();

    translate(holePosition3)
      stand();

    translate(holePosition4)
      stand();
  }
}

module panel() {

  translate([0, 0, 14]) {
    translate(globalMove) {
      // holesInPanel();
      difference() {

        linear_extrude(1.5) {
          //minkowski() {
          import("outline.svg");
          //circle(0.1);
          //}
        }

        translate([0, 0, -5])
          linear_extrude(10) {
            import("buttons.svg");
          }

        holesInPanel();

        translate([268.4, 95, 0]) {
          difference() {
            cube([25, 90, 40], center=true);
            translate([-10, -60, 0]) {
              rotate([0, 0, 60]) {
                cube([80, 80, 50], center=true);
              }
            }
          }
        }
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
//panel();
//holesInPanel();
body();
