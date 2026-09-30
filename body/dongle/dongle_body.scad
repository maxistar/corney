/**
 * - screen cut
 * - usb cut
 * - button cut
 * - button
 * - screw plates
 * - wall thighteners
 */


breadboard_width = 30;
breadboard_length = 70;
breadboard_spacing = 0.25;
inner_height = 30;
wall_thickness = 2;

wall_thickness_x2 = wall_thickness * 2;

x_cut_offset = 10;

module thickShell() {
  difference() {
    cube([breadboard_width + wall_thickness_x2 + breadboard_spacing, breadboard_length + wall_thickness_x2 + breadboard_spacing, inner_height + wall_thickness_x2], center=true);

    cube([breadboard_width + breadboard_spacing, breadboard_length + breadboard_spacing, inner_height], center=true);
  }
}

module deviceModel() {

  screwXOffset = breadboard_width / 2 - 1.5;
  screwYOffset = breadboard_length / 2 - 1.5;

  translate([0, -15, 16])
    cube([28, 28, 1], center=true);

  translate([5, 15, 15])
    cube([18, 35, 1], center=true);

  translate([-breadboard_width / 2 + 6 / 2 + 2.5, breadboard_length / 2 - 6 / 2 - 11.5, 2])
    cube([6, 6, 4], center=true);

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
  // bottom shell
  difference() {
    thickShell();
    translate([0, 0, inner_height-x_cut_offset])
      cube([breadboard_width * 2, breadboard_length * 2, inner_height * 2], center=true);
  }
}

// upper shell
module upperShell() {
  difference() {
    thickShell();
    translate([0, 0, -inner_height-x_cut_offset])
      cube([breadboard_width * 2, breadboard_length * 2, inner_height * 2], center=true);
  }
}

translate([0, 0, 50])
  upperShell();

deviceModel();

translate([0, 0, -50])
  bottomShell();
