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

use <Outline_thin.scad>

cut_angle = [-26, -15, 0];
magnet_radius = 4 / 2 + 0.1;
holder_thickness = 0.8;

cut_rotate = [-3, -2, 1];

module cut_centered() {
  rotate(cut_rotate) {
    translate([-2, -0, 40]) {
      rotate([0, 0, 180]){}
      scale([1, 1, -1]) {
        bottom_panel(hide_side_magnets=false);
      }
    }
  }
}

//two_cornes();

module shell() {
  difference() {
    minkowski() {
      outline();
      sphere(r=5.2);
    }

    minkowski() {
      outline();
      sphere(r=0.5);
    }
  }
  double_magnet_plane_half(magnet_radius + holder_thickness, 3);
}

module outline() {
  hull() {
    //rotate([0,0, 180]) {
    twokeyboards(fullheight=true);
    //}
  }
}

module one_half() {

  difference() {
    shell();

    rotate(cut_angle)
      translate([0, 0, -50])
        cube([400, 400, 100], center=true);

    //translate([5, 0, 20])
    cut_centered();

    double_angled_cuts();

    double_magnet_plane(magnet_radius, 2, 0.1);
  }

  //  translate([5, 0, 20])
  //cut_centered();

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
  translate([30, 54, 2.7])
    hull() {
      cylinder(h=0.5, r=magnet_radius + holder_thickness, $fn=50);

      translate([3, 15, 40])
        cylinder(h=0.5, r=magnet_radius + holder_thickness, $fn=50);
    }

  translate([-30, -54, 2.7])
    hull() {
      cylinder(h=0.5, r=magnet_radius + holder_thickness, $fn=50);

      translate([0, -10, 10])
        cylinder(h=0.5, r=magnet_radius + holder_thickness, $fn=50);
    }

  translate([-30, 37, 2.7])
    hull() {
      cylinder(h=0.5, r=magnet_radius + holder_thickness, $fn=50);

      translate([0, 5, 10])
        cylinder(h=0.5, r=magnet_radius + holder_thickness, $fn=50);
    }

  translate([30, -37, 2.7])
    hull() {
      cylinder(h=0.5, r=magnet_radius + holder_thickness, $fn=50);

      translate([0, -5, 8])
        cylinder(h=0.5, r=magnet_radius + holder_thickness, $fn=50);
    }
}

/*
translate([0, 0, 33]) {
  rotate(-cut_rotate) {
    panelbuttons();
  }
} 
*/

one_half();
//rubber_cuts();
//angled_cuts();
//twokeyboards();

/*
translate([0, 0, -30])
  rotate([0, 180, 0])
    scale([1, -1, 1])
      one_half();
*/

module rubber_cuts() {

  width = 2.2;
  depth = 2.5;

  translate([30, 58.2, 0])
    rotate([0, 0, 15])
      cube([20, width, depth], center=true);

  translate([-14, 46, 0])
    rotate([0, 0, 16])
      cube([20, width, depth], center=true);

  translate([-52.5, 33.5, 0])
    rotate([0, 0, 20])
      cube([20, width, depth], center=true);

  translate([59.6, 53, 0])
    rotate([0, 0, -56])
      cube([20, width, depth], center=true);

  translate([71.5, 23, 0])
    rotate([0, 0, -85])
      cube([20, width, depth], center=true);

  translate([72, -10, 0])
    rotate([0, 0, -92])
      cube([20, width, depth], center=true);
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

  translate([30, -36.9, voffset])
    cylinder(h=height * 2 + voffset * 2, r=radius, $fn=50, center=true);

  translate([30, 54, voffset])
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
