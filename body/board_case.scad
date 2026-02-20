// case
/**
features todo: 
- [x] magnetic latch
- [x] placing keyboad on top of case as tilted support
- [x] magnetic fixation of the keyboard
- [x] additional pads fixation for the keyboard
- [x] rubber perimeter for better stability
- [ ] make maglet holders 3dprint friendly

*/

cut_angle = [-6, 13, 0];
magnet_radius = 4 / 2 + 0.1;
holder_thickness = 0.8;

module column3() {
  translate([32, 120, 14])
    cube([18, 18, 5], center=true);

  translate([32, 120 - 19, 14])
    cube([18, 18, 5], center=true);

  translate([32, 120 - 19 * 2, 14])
    cube([18, 18, 5], center=true);
}

module corne() {

  translate([17, 143, -8.5])
    import("body_with_stands.stl");

  translate([0, 136, -84])
    rotate([90, 0, 0])
      import("tbenen-plate.stl");

  translate([157, 129, 6])
    scale([-1, 1, 1])
      import("board_cover.stl");

  column3();

  translate([19, 0, 0])
    column3();

  translate([19 * 2, 3, 0])
    column3();

  translate([19 * 3, 6, 0])
    column3();

  translate([19 * 4, 3, 0])
    column3();

  translate([19 * 5, 0, 0])
    column3();

  translate([99, 66, 14])
    cube([18, 18, 5], center=true);

  translate([120, 63, 14])
    rotate([0, 0, -15])
      cube([18, 18, 5], center=true);

  translate([142, 60, 14])
    rotate([0, 0, -30])
      cube([18, 18, 5], center=true);
}

//corne();

module corne_centered() {
  translate([-90, -95, 0.1])
    corne();
}

module base_centered() {
  translate([-90, -95, 2])
    hull() {
      import("corne-chocoflan-case.stl");
    }
}

module cut_centered() {
  rotate([1, -1.0, 0])
    translate([-70, 48, -8.35])
      //hull() {
      import("corne_shape_with_stands.stl");
  //} 
}

module two_cornes() {
  corne_centered();

  rotate([0, 180, 0])
    scale([1, -1, 1])
      corne_centered();
}

module outline() {
  hull() {
    two_cornes();
  }
}

//two_cornes();

module shell() {
  difference() {
    minkowski() {
      outline();
      sphere(r=4.2);
    }

    minkowski() {
      outline();
      sphere(r=0.5);
    }
  }
  double_magnet_plane_half(magnet_radius + holder_thickness, 3);
}

module one_half() {

  difference() {
    shell();

    rotate(cut_angle)
      translate([0, 0, -50])
        cube([400, 400, 100], center=true);

    translate([5, 0, 20])
      cut_centered();

    double_angled_cuts();

    double_magnet_plane(magnet_radius, 2, 0.1);
  }

  intersection() {
    minkowski() {
      outline();
      sphere(r=4);
    }
    rotate(cut_angle) {
      print_helpers();
    }
  }
}

module print_helpers() {
  translate([30, 35.6, 2.7])
    hull() {
      cylinder(h=0.5, r=magnet_radius + holder_thickness, $fn=50);

      translate([3, 15, 20])
        cylinder(h=0.5, r=magnet_radius + holder_thickness, $fn=50);
    }

  translate([-30, -35.6, 2.7])
    hull() {
      cylinder(h=0.5, r=magnet_radius + holder_thickness, $fn=50);

      translate([0, -10, 10])
        cylinder(h=0.5, r=magnet_radius + holder_thickness, $fn=50);
    }

  translate([-33, 50, 2.7])
    hull() {
      cylinder(h=0.5, r=magnet_radius + holder_thickness, $fn=50);

      translate([0, 5, 10])
        cylinder(h=0.5, r=magnet_radius + holder_thickness, $fn=50);
    }

  translate([33, -50, 2.7])
    hull() {
      cylinder(h=0.5, r=magnet_radius + holder_thickness, $fn=50);

      translate([0, -5, 10])
        cylinder(h=0.5, r=magnet_radius + holder_thickness, $fn=50);
    }
}

one_half();

/*
translate([0, 0, -30])
  rotate([0, 180, 0])
    scale([1, -1, 1])
      one_half();
*/

module rubber_cuts() {
  translate([10, 44.2, 0])
    rotate([0, 0, -16])
      cube([20, 1.8, 1.8], center=true);

  translate([-19, 50.5, 0])
    rotate([0, 0, -8])
      cube([20, 1.8, 1.8], center=true);

  translate([-52.5, 49.5, 0])
    rotate([0, 0, 22.9])
      cube([20, 1.8, 1.8], center=true);

  translate([44, 33.5, 0])
    rotate([0, 0, -22])
      cube([20, 1.8, 1.8], center=true);

  translate([71, 6, 0])
    rotate([0, 0, -88])
      cube([20, 1.8, 1.8], center=true);

  translate([71, -22, 0])
    rotate([0, 0, -91])
      cube([20, 1.8, 1.8], center=true);
}

module angled_cuts() {
  rotate(cut_angle)
    rubber_cuts();
}

module double_angled_cuts() {
  angled_cuts();

  rotate([0, 180, 0])
    scale([1, -1, 1])
      angled_cuts();
}

module magnets_plane(radius, height, voffset) {

  translate([33, -49.9, voffset])
    cylinder(h=height * 2 + voffset * 2, r=radius, $fn=50, center=true);

  translate([30, 35.6, voffset])
    cylinder(h=height * 2 + voffset * 2, r=radius, $fn=50, center=true);
}

module angled_magnet_plane(radius, height, voffset) {
  rotate(cut_angle)
    magnets_plane(radius, height, voffset);
}

module double_magnet_plane(radius, height, voffset = 0) {
  angled_magnet_plane(radius, height, voffset);

  rotate([0, 180, 0])
    scale([1, -1, 1])
      angled_magnet_plane(radius, height, voffset);
}

module double_magnet_plane_half(radius, height, voffset = 0) {
  difference() {
    double_magnet_plane(radius=radius, height=height, voffset=voffset);

    rotate(cut_angle)
      translate([0, 0, -50])
        cube([400, 400, 100], center=true);
  }
}

module test_magnet() {
  difference() {
    cylinder(h=3, r=magnet_radius + holder_thickness, $fn=50);
    translate([0, 0, -0.1])
      cylinder(h=2 + 0.1, r=magnet_radius, $fn=50);
  }
}

//test_magnet();
//difference() {

//}

//double_angled_cuts();

//translate([5, 0, 20])
//  cut_centered();

//corne_centered();

//translate([0, 0, -40])
//  rotate([0, 180, 0])
//    scale([1, -1, 1])
//      corne_centered();

/*
difference() {

  translate([0, 0, -40])
    intersection() {
      shell();

      rotate([0, 20, 0])
        translate([0, 0, -50])
          cube([400, 400, 100], center=true);
    }

  translate([-15, 0, -60])
    rotate([0, 180, 0])
      scale([1, -1, 1])
        cut_centered(); 
}

*/
//import("corne_shape.stl");
