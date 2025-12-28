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

}