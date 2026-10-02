/**
 * - [x] screen cut
 * - [x] usb cut
 * - [x] button cut
 * - [x] button
 * - [x] top screw plates
 * - [x] bottom screw holes
 * - [x] wall thighteners
 * - [x] tune positions with expection cuts
 * - clarify the screen cut
 */

$fn = 50;

breadboard_width = 30;
breadboard_length = 70;
breadboard_spacing = 0.25;
breatdboard_z_offset = -5;
inner_height = 25;
wall_thickness = 2;

wall_thickness_x2 = wall_thickness * 2;

x_cut_offset = 6;

resetButtonXOffset = -breadboard_width / 2 + 6 / 2 + 2.5;
resetButtonYOffset = breadboard_length / 2 - 6 / 2 - 11.5;
resetButtonSize = 6;

screwXOffset = breadboard_width / 2 - 1.5;
screwYOffset = breadboard_length / 2 - 1.5;

module thickShell(outerOffset = 0, innerOffset = 0) {
  difference() {
    cube([breadboard_width + wall_thickness_x2 + breadboard_spacing + outerOffset, breadboard_length + wall_thickness_x2 + breadboard_spacing + outerOffset, inner_height + wall_thickness_x2 + outerOffset], center=true);

    cube([breadboard_width + breadboard_spacing + innerOffset, breadboard_length + breadboard_spacing + innerOffset, inner_height + innerOffset], center=true);
  }
}

module usbCut() {
  translate([3, 32, 13])
    rotate([90, 0, 0])
      hull() {
        translate([3, 0, 0])
          cylinder(h=20, r=3, center=true);
        translate([-3, 0, 0])
          cylinder(h=20, r=3, center=true);
      }
}

module deviceModel() {

  translate([0, -15, 16])
    cube([28, 28, 1], center=true);

  // nice!nano
  translate([5, 15, 15])
    cube([18, 35, 1], center=true);

  // usb cut
  usbCut();

  // reset button
  translate([resetButtonXOffset, resetButtonYOffset, 2])
    cube([resetButtonSize, resetButtonSize, 4], center=true);

  difference() {
    cube([breadboard_width, breadboard_length, 1], center=true);

    translate([screwXOffset, screwYOffset, 0]) {
      cylinder(h=10, r=1, center=true);
    }

    translate([-screwXOffset, screwYOffset, 0]) {
      cylinder(h=10, r=1, center=true);
    }

    translate([screwXOffset, -screwYOffset, 0]) {
      cylinder(h=10, r=1, center=true);
    }

    translate([-screwXOffset, -screwYOffset, 0]) {
      cylinder(h=10, r=1, center=true);
    }
  }
}

module bottomShell() {
  cut_height = 50;
  cut_Move_z = -32;
  cutradius = 4;
  cutradius2 = 2.6;

  screwRadius = 1;

  // bottom shell
  difference() {

    union() {
      difference() {
        thickShell();
        translate([0, 0, inner_height - x_cut_offset - 2])
          cube([breadboard_width * 2, breadboard_length * 2, inner_height * 2], center=true);
      }

      difference() {
        thickShell(outerOffset=-2);
        translate([0, 0, inner_height - x_cut_offset])
          cube([breadboard_width * 2, breadboard_length * 2, inner_height * 2], center=true);
      }
    }

    translate([screwXOffset, screwYOffset, cut_Move_z]) {
      cylinder(h=cut_height, r=cutradius2, center=true);
    }

    translate([-screwXOffset, screwYOffset, cut_Move_z]) {
      cylinder(h=cut_height, r=cutradius2, center=true);
    }

    translate([screwXOffset, -screwYOffset, cut_Move_z]) {
      cylinder(h=cut_height, r=cutradius2, center=true);
    }

    translate([-screwXOffset, -screwYOffset, cut_Move_z]) {
      cylinder(h=cut_height, r=cutradius2, center=true);
    }
  }

  intersection() {
    smallOffset = 2;
    cube([breadboard_width + wall_thickness_x2 + breadboard_spacing - smallOffset, breadboard_length + wall_thickness_x2 + breadboard_spacing - smallOffset, inner_height + wall_thickness_x2], center=true);

    union() {
      translate([screwXOffset, screwYOffset, cut_Move_z + cut_height / 2]) {
        difference() {
          cylinder(h=1, r=cutradius, center=true);
          cylinder(h=cut_height, r=screwRadius, center=true);
        }
        //cylinder(h=1, r=cutradius, center=true);
      }

      translate([screwXOffset, -screwYOffset, cut_Move_z + cut_height / 2]) {
        difference() {
          cylinder(h=1, r=cutradius, center=true);
          cylinder(h=cut_height, r=screwRadius, center=true);
        }
      }

      translate([-screwXOffset, screwYOffset, cut_Move_z + cut_height / 2]) {
        difference() {
          cylinder(h=1, r=cutradius, center=true);
          cylinder(h=cut_height, r=screwRadius, center=true);
        }
      }

      translate([-screwXOffset, -screwYOffset, cut_Move_z + cut_height / 2]) {
        difference() {
          cylinder(h=1, r=cutradius, center=true);
          cylinder(h=cut_height, r=screwRadius, center=true);
        }
      }

      translate([screwXOffset, screwYOffset, cut_Move_z]) {
        difference() {
          cylinder(h=cut_height, r=cutradius, center=true);
          cylinder(h=cut_height + 1, r=cutradius2, center=true);
        }
      }

      translate([-screwXOffset, screwYOffset, cut_Move_z]) {
        difference() {
          cylinder(h=cut_height, r=cutradius, center=true);
          cylinder(h=cut_height + 1, r=cutradius2, center=true);
        }
      }

      translate([screwXOffset, -screwYOffset, cut_Move_z]) {
        difference() {
          cylinder(h=cut_height, r=cutradius, center=true);
          cylinder(h=cut_height + 1, r=cutradius2, center=true);
        }
      }

      translate([-screwXOffset, -screwYOffset, cut_Move_z]) {
        difference() {
          cylinder(h=cut_height, r=cutradius, center=true);
          cylinder(h=cut_height + 1, r=cutradius2, center=true);
        }
      }
    }
  }
}

// upper shell
module upperShell() {
  difference() {
    union() {
      difference() {
        thickShell();
        translate([0, 0, -inner_height - x_cut_offset + 0.1])
          cube([breadboard_width * 2, breadboard_length * 2, inner_height * 2], center=true);
      }

      difference() {
        thickShell(innerOffset=2 + 0.1);
        translate([0, 0, -inner_height - x_cut_offset - 2])
          cube([breadboard_width * 2, breadboard_length * 2, inner_height * 2], center=true);
      }
    }

    // window cut
    windowsYOffset = -(breadboard_length / 2 - 15 / 2) + 15;
    translate([0, windowsYOffset, 0])
      cube([25, 15, 200], center=true);

    translate([0, 0, breatdboard_z_offset]) {
      usbCut();
    }
    translate([resetButtonXOffset, resetButtonYOffset, 0])
      cylinder(h=100, r=3, center=true);
  }

  translate([resetButtonXOffset, resetButtonYOffset, 9])
    difference() {
      cylinder(h=10, r=4, center=true);
      cylinder(h=21, r=3, center=true);
    }

  difference() {
    topPlates();

    translate([screwXOffset, screwYOffset, 0]) {
      cylinder(h=10, r=0.8, center=true);
    }

    translate([-screwXOffset, screwYOffset, 0]) {
      cylinder(h=10, r=0.8, center=true);
    }

    translate([screwXOffset, -screwYOffset, 0]) {
      cylinder(h=10, r=0.8, center=true);
    }

    translate([-screwXOffset, -screwYOffset, 0]) {
      cylinder(h=10, r=0.8, center=true);
    }
  }
}

module topPlates() {
  bottomScrewXOffset = screwXOffset + 4;
  bottomScrewYOffset = screwYOffset + 4;
  plateYOffset = -2;

  intersection() {
    cube([breadboard_width + wall_thickness_x2 + breadboard_spacing, breadboard_length + wall_thickness_x2 + breadboard_spacing, inner_height + wall_thickness_x2], center=true);

    union() {
      hull() {
        translate([bottomScrewXOffset, bottomScrewYOffset, 10]) {
          linear_extrude(height=1, center=true)
            circle(r=3);
        }
        translate([screwXOffset, screwYOffset, plateYOffset]) {
          linear_extrude(height=3, center=true)
            circle(r=3);
        }
      }

      hull() {
        translate([-bottomScrewXOffset, bottomScrewYOffset, 10]) {
          linear_extrude(height=1, center=true)
            circle(r=3);
        }
        translate([-screwXOffset, screwYOffset, plateYOffset]) {
          linear_extrude(height=3, center=true)
            circle(r=3);
        }
      }

      hull() {
        translate([bottomScrewXOffset, -bottomScrewYOffset, 10]) {
          linear_extrude(height=1, center=true)
            circle(r=3);
        }
        translate([screwXOffset, -screwYOffset, plateYOffset]) {
          linear_extrude(height=3, center=true)
            circle(r=3);
        }
      }

      hull() {
        translate([-bottomScrewXOffset, -bottomScrewYOffset, 10]) {
          linear_extrude(height=1, center=true)
            circle(r=3);
        }
        translate([-screwXOffset, -screwYOffset, plateYOffset]) {
          linear_extrude(height=3, center=true)
            circle(r=3);
        }
      }
    }
  }
}

module button() {
  translate([resetButtonXOffset, resetButtonYOffset, 9]) {
    cylinder(h=12, r=3 - 0.1, center=true);

    translate([0, 0, -5])
      cylinder(h=2, r=4, center=true);
  }
}

spacingZOffset = 30;

//difference() {
  union() {
    translate([0, 0, spacingZOffset])
      upperShell();

    translate([0, 0, breatdboard_z_offset])
      deviceModel();

    translate([0, 0, -spacingZOffset/2])
      bottomShell();
  }

  //translate([0, 120, 0])
  //  cube([200, 200, 200], center=true);
//}

button();
