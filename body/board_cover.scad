/**

- [x] sitting pins
- [ ] reset button cut
- [x] usb-c cut
- [x] power button cut
- [x] board space
- [x] bottom plate that fits into horizontal cut

*/

$fn = 50;

cover_width = 20.5;
cover_length_1 = 60.71;
cover_length_2 = 49.5;

cover_height = 12;
cover_height_min = 4;

wall_thickness = 0.8;
sittin_pin_radius = 2.5 / 2;

module outer_shape() {
  linear_extrude(height=cover_height_min) {
    polygon([[0, 0], [0, -cover_length_1], [cover_width, -cover_length_2], [cover_width, 0]]);
  }

  difference() {
    linear_extrude(height=cover_height) {
      polygon([[0, 0], [0, -cover_length_1], [cover_width, -cover_length_2], [cover_width, 0]]);
    }

    translate([0, -60, 25])
      rotate([45, 0, 0])
        cube([100, 100, 50], center=true);
  }
}

module inner_shape() {
  difference() {
    translate([0, 0, -wall_thickness]) {
      linear_extrude(height=cover_height) {
        polygon([[wall_thickness, -wall_thickness], [wall_thickness, wall_thickness - cover_length_1], [cover_width - wall_thickness, wall_thickness - cover_length_2], [cover_width - wall_thickness, -wall_thickness]]);
      }
    }

    translate([0, -60, 25 - wall_thickness])
      rotate([45, 0, 0])
        cube([100, 100, 50], center=true);
  }

  translate([0, 0, -wall_thickness]) {
    linear_extrude(height=cover_height_min) {
      polygon([[wall_thickness, -wall_thickness], [wall_thickness, wall_thickness - cover_length_1], [cover_width - wall_thickness, wall_thickness - cover_length_2], [cover_width - wall_thickness, -wall_thickness]]);
    }
  }
}

module cuts() {

  // Power button cut
  translate([-5, -50.5, 0]) {
    rotate([0, 90, 0]) {
      hull() {
        cylinder(h=9, r=2.0, center=false);
        translate([0, 4, 0])
          cylinder(h=9, r=2.0, center=false);
      }
    }
  }

  // USB-C cut
  translate([14, -6, 8.6]) {
    rotate([0, 90, 90]) {
      hull() {
        cylinder(h=10, r=1.8, center=false);
        translate([0, 7, 0])
          cylinder(h=9, r=1.8, center=false);
      }
    }
  }

  translate([cover_width - 9.8, -cover_length_2 + 2.7, wall_thickness])
    cylinder(h=cover_height_min, r=sittin_pin_radius, center=false);
}

difference() {
  outer_shape();
  inner_shape();
  cuts();
}
/*
translate([3.6, -cover_length_1 + 5.7, -wall_thickness])
  cylinder(h=cover_height_min, r=sittin_pin_radius, center=false);

translate([cover_width - 4.5, -cover_length_2 + 3, -wall_thickness])
  cylinder(h=cover_height_min, r=sittin_pin_radius, center=false);
*/
translate([cover_width - 2.0, +2, /*-wall_thickness*/ 0])
  cylinder(h=cover_height_min, r=sittin_pin_radius-0.2, center=false);

translate([16.5, 0, 2]) {
  cube([4, 3.5, 3]);
}

