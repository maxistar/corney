$fn = 50;

difference() {
import("corne_chocoflan_basic.stl");

translate([10.1,-18, 9.2]) 
  cylinder(h = 4.04, r = 3.93);

translate([10.1,-66.5, 9.2]) 
  cylinder(h = 4.04, r = 3.93);

translate([124.8,-93.9, 8.2]) 
  cylinder(h = 4.04, r = 3.93);

translate([97,-13.3, 8.2]) 
  cylinder(h = 4.04, r = 3.99);

//cube([20,50,100], center=true);

}