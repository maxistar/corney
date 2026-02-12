$fn = 50;

/**
  * Corne Chocoflan case body with stands and magnet holes
  * 
  * Based on corne_chocoflan_basic.stl
  * 
  * todo: 
  * - move two stands 0.5mm up
  * - make magnet holes
  *  
  */

magnet_radius = 4 / 2 + 0.1;

magnet1_pos = [8.8, -21.5, 9.4];
magnet2_pos = [8.8, -56.9, 9.4];
magnet3_pos = [91.5, -85.0, 9.4];
magnet4_pos = [101, -14.5, 9.2];
magnet5_pos = [132, -52.5, 9.2];

cuts_rotation = [0, 0.7, 0];
rubber_width = 5.2;

module magnets_holders() {

  rotate(cuts_rotation) {

    translate(magnet1_pos)
      cylinder(h=2.14, r=magnet_radius + 0.5);

    translate(magnet2_pos)
      cylinder(h=2.14, r=magnet_radius + 0.5);

    translate(magnet3_pos)
      cylinder(h=2.14, r=magnet_radius + 0.5);

    translate(magnet4_pos)
      cylinder(h=2.14, r=magnet_radius + 0.5);

    translate(magnet5_pos)
      cylinder(h=2.14, r=magnet_radius + 0.5);
  }
}

module magnets() {

  rotate(cuts_rotation) {
    translate(magnet1_pos)
      cylinder(h=4.04, r=magnet_radius);

    translate(magnet2_pos)
      cylinder(h=4.04, r=magnet_radius);

    translate(magnet3_pos)
      cylinder(h=4.04, r=magnet_radius);

    translate(magnet4_pos)
      cylinder(h=4.04, r=magnet_radius);

    translate(magnet5_pos)
      cylinder(h=4.04, r=magnet_radius);      
  }
}

module body() {
  difference() {

    union() {
      import("corne_chocoflan_basic.stl");
      thicknessCubes();
      innerStands();

      magnets_holders();
    }
    //import("corne_chocoflan_basic.stl");

    magnets();

    rotate(cuts_rotation)
      linearCutsV2();

    // standCuts();
    //cube([25,250,100], center=true);
  }
}

//rotate([0,1,0])
//linearCuts();

//translate([8.1,-16.1, 8.5]) 
//rotate([0,0.6,0]) 
//  cube([150,150,0.5]);

module stand() {
  difference() {
    union() {
      cylinder(h=3, r=1.8);
      cylinder(h=2, r1=4.5, r2=4);
    }
    cylinder(h=4, r=1);
  }
}

module innerStands1() {
  //cube([18.5,18.5,2]);
  translate([18.5, 0, 0])
    stand();
}

module innerStands2() {
  //cube([18.5,19,2]);
  translate([18.5, 19, 0])
    stand();
}

module innerStands3() {
  //cube([94.75,17,2]);
  translate([94.75, 0, 0])
    stand();
}

module innerStands4() {
  //cube([23.5,64.5,2]);
  translate([0, 0, 0])
    stand();
}

module innerStands() {
  translate([6.1, -32.6, 9.427])
    innerStands1();

  translate([6.1, -70.5, 9.427])
    innerStands2();

  translate([6.0, -29.2, 9.427])
    innerStands3();

  translate([114.4, -77.0, 9.427])
    innerStands4();
}

module bottomCube2() {
  //cube([3,3,1]);
  translate([0, -6, 0])
    cube([38, 6, 1.5]);
}

module bottomCube1() {
  //cube([10,10,1]);
  translate([0, 2, 0])
    cube([38, 52, 1]);
}

module thicknessCubes() {
  rotate(cuts_rotation)
    translate([6, -70, 9.3])
      bottomCube1();

  //translate([6, -18, 9.2])
  //bottomCube2();
}

module standCut() {
  cylinder(h=1, r=3, center=true);
  difference() {
    cylinder(h=1, r=5.5, center=true);
    cylinder(h=3, r=3.5, center=true);
  }
}

module linearCut() {
  cube([30, 1.6, 1.6]);
  translate([0, 1.6 * 2, 0])
    cube([30, 1.6, 1.6]);
}

module linearCuts() {

  depth = 8.3;

  translate([12, -23.5, depth])
    linearCut();

  translate([12, -59.5, depth])
    linearCut();

  translate([95, -80, depth])
    rotate([0, 0, -15])
      linearCut();

  translate([100, -25.5, depth])
    linearCut();
}

module linearCutV2() {
  cube([50, rubber_width, 1.8]);
  translate([0, 7, 0])
    cube([50, rubber_width, 1.8]);
  translate([0, 14, 0])
    cube([50, rubber_width, 1.8]);
}

module linearCutV2_long() {
  cube([65, rubber_width, 1.8]);
  translate([0, 7, 0])
    cube([66, rubber_width, 1.8]);
  translate([0, 14, 0])
    cube([67, rubber_width, 1.8]);
}

module linearCutsV2() {

  depth = 8.3;

  translate([11.5, -17, depth])
    rotate([0, 0, -90])
      linearCutV2();

  translate([113, -19, depth])
    rotate([0, 0, -95])
      linearCutV2_long();
}

//rotate(cuts_rotation)
//linearCutsV2();

module standCuts() {
  zoffset = 9;
  translate([24.5, -52, zoffset]) standCut();
  translate([24.5, -32.5, zoffset]) standCut();

  translate([120, -84, zoffset]) standCut();
  translate([130, -25, zoffset]) standCut();
}

module body_solid() {

  union() {
    //import("corne_shape.stl");

    translate([70, -50, 13.22])
      rotate(cuts_rotation)
        cube([200, 130, 10], center=true);

    rotate(cuts_rotation)
      linearCutsV2();

    rotate(cuts_rotation)
      translate([0, 0, -6.8]) {
        translate(magnet1_pos)
          cylinder(h=6.04, r=magnet_radius);

        translate(magnet2_pos)
          cylinder(h=6.04, r=magnet_radius);

        translate(magnet3_pos)
          cylinder(h=6.04, r=magnet_radius);

        translate(magnet4_pos)
          cylinder(h=6.04, r=magnet_radius);

        translate(magnet5_pos)
          cylinder(h=6.04, r=magnet_radius);          
      }
  }
}
