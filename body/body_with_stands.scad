$fn = 50;

magnet_radius = 3.93 / 2;

difference() {
import("corne_chocoflan_basic.stl");

translate([8.1,-16.1, 9.2]) 
  cylinder(h = 4.04, r = magnet_radius);

translate([8.1,-55.5, 9.2]) 
  cylinder(h = 4.04, r = magnet_radius);

translate([125.5,-96.7, 7.9]) 
  cylinder(h = 4.04, r = magnet_radius);

translate([98.4,-11.4, 8.2]) 
  cylinder(h = 4.04, r = magnet_radius);

//cube([250,50,100], center=true);

rotate([0,0.61,0])
 standCuts();

}

//translate([8.1,-16.1, 8.5]) 
//rotate([0,0.6,0]) 
//  cube([150,150,0.5]);

module innerStands1() {
  //cube([18.5,18.5,2]);
  translate([18.5,0,0])
    difference() {
      cylinder(h = 2, r = 6);
      cylinder(h = 3, r = 1);
    }
}

module innerStands2() {
  //cube([18.5,18.5,2]);
  translate([18.5,18.5,0])
    difference() {
      cylinder(h = 2, r = 6);
      cylinder(h = 3, r = 1);
    }
}

translate([6.1,-32.6,9.427]) 
  innerStands1();


translate([6.1,-70,9.427]) 
  innerStands2();


module standCut() {
  cylinder(h = 1, r = 3, center=true);
  difference() {
      cylinder(h = 1, r = 5.5, center=true);
      cylinder(h = 3, r = 3.5, center=true);
    }
}

module standCuts() {
  zoffset = 9;
  translate([24.5,-52,zoffset]) standCut();
  translate([24.5,-32.5,zoffset]) standCut();

  translate([120,-84,zoffset]) standCut();
  translate([130,-25,zoffset]) standCut();
}

