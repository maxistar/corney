// case
/**
features todo: 
- [ ] magnetic latch
- [ ] placing keybouad on top of case as tilted support
- [ ] magnetic fixation of the keyboard

*/

module column3() {
  translate([32, 120, 14])
    cube([18, 18, 5], center=true);

  translate([32, 120 - 19, 14])
    cube([18, 18, 5], center=true);

  translate([32, 120 - 19 * 2, 14])
    cube([18, 18, 5], center=true);
}

module corne() {

  import("corne-chocoflan-case.stl");

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

module corne_centered() {
  translate([-90, -95, 2])
    corne();
}

module outline() {
  hull() {
    corne_centered();

    rotate([0, 180, 0])
      scale([1, -1, 1])
        corne_centered();
  }
}

module shell() {
  difference() {
    minkowski() {
      outline();
      sphere(r=3);
    }

    minkowski() {
      outline();
      sphere(r=0.3);
    }
  }
}
/*
difference() {
  shell();

  rotate([0, 14, 0])
    translate([0, 0, -50])
      cube([400, 400, 100], center=true);
}
*/

//corne_centered();

/*
translate([0, 0, -40])
  rotate([0, 180, 0])
    scale([1, -1, 1])
      corne_centered();
*/

translate([0, 0, -40])
  intersection() {
    shell();

    rotate([0, 14, 0])
      translate([0, 0, -50])
        cube([400, 400, 100], center=true);
  }
