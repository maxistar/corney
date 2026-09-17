/**
 * [x] add screws islands
 * [x] add cover panel
 * [x] design cover for shield
 * [x] mark buttons positions
 * create a travel case
 * move battery location to the side
 * move sheld to the bottom
 * optimize the globalMove usage
 */

//hull() {
$fn = 150;
magnet_radius = 4 / 2 + 0.1;
globalMove = [-209.55, -99.5, 0];

holePosition1 = [161.15, 94.95, 0];
holePosition2 = [161.1, 113.9, 0];
holePosition3 = [237.1, 117.4, 0];
holePosition4 = [250.87, 70.05, 0];
wallThickness = 1.2;

sensorRadius = 20;
sensorPosition = [67, 13];

holderPosition1 = [73, -1, 0];
holderPosition2 = [73, 27, 0];

function getWallThickness() = wallThickness;
function getGlobalMove() = globalMove;
function getSensorPosition() = sensorPosition;
function getSensorRadius() = sensorRadius;
function getHolderPositions() = [holderPosition1, holderPosition2];

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
  translate(leg1) {
    cylinder(h=40, r=magnet_radius, center=true);
  }

  if (!hide_side_magnets1) {
    rotate([0, 0, 180]) {
      translate(leg1) {
        cylinder(h=40, r=magnet_radius, center=true);
      }
    }
  }
  translate([0, -22, 15]) {
    cylinder(h=40, r=magnet_radius, center=true);
  }
}

module battery() {
  cube([25, 43, 5]);
}

module legs() {
  legmove1 = [43, 21.0, 0];
  //legmove2 = [43, 11.0, 0];
  translate([0, 0, -5.0]) {

    translate([75, 15, 0]) {
      leg();
    }

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

module pcb() {
  translate([0, 0, 12.5]) {
    linear_extrude(1.5) {
      psbProjectionNormalized();
    }
  }
}

module bodyProjectionNormalized() {
  union() {
    psbProjectionNormalized();

    translate(sensorPosition) {
      circle(r=sensorRadius - wallThickness / 2);
    }
  }
}

module psbProjectionNormalized() {
  translate(globalMove) {
    import("outline.svg");
  }
}

module bodyProjection() {
  import("outline.svg");

  translate(-globalMove) {
    translate(sensorPosition) {
      circle(r=sensorRadius - wallThickness / 2);
    }
  }
}

module body() {

  difference() {
    translate(globalMove) {
      union() {

        difference() {
          linear_extrude(15.5) {
            minkowski() {
              bodyProjection();
              circle(wallThickness);
            }
          }

          translate([0, 0, 9]) {
            linear_extrude(10) {
              minkowski() {
                bodyProjection();
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
            import("outline_empty.svg");
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

  translate(holePosition1)
    stand();

  translate(holePosition2)
    stand();

  translate(holePosition3)
    stand();

  translate(holePosition4)
    stand();
}

module hooksBlock(offset = 0) {

  hookHeight = 0.6;

  translate([48, 30, -1 + hookHeight / 2])
    cylinder(h=0.6 + offset, r1=3 + offset, r2=2 + offset, center=true);

  translate([48, -10, -1 + hookHeight / 2])
    cylinder(h=0.6 + offset, r1=3 + offset, r2=2 + offset, center=true);

  translate([51, -17.5, -1 + hookHeight / 2])
    cylinder(h=0.6 + offset, r1=3 + offset, r2=2 + offset, center=true);

  translate([64, -25, -1 + hookHeight / 2])
    cylinder(h=0.6 + offset, r1=3 + offset, r2=2 + offset, center=true);
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
        translate(-globalMove) {
          translate([0, 0, 1]) {
            hooksBlock(0.2);
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

if (false) {
  panel();
}
//holesInPanel();

//difference() {
if (true) {
  union() {
    body();
  }
}

//pcb();

//translate([-100,0,0])
//inspection cube
//cube([100, 100, 100], center=true);

//}

// bodyProjection();

//linear_extrude(12) {
//minkowski() {
//import("outline_empty_sp.svg");
//circle(0.3);
//}
//        }
