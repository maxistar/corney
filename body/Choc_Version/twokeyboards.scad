use <board_assembled_touchpad.scad>
use <board_assembled.scad>
use <_auxiliary.scad>

module twokeyboards(fullheight = false) {
  panelbuttonsmoved_touchpad(fullheight=fullheight);

  scale([1, 1, -1]) {
    rotate([0, 0, 180]) {
      panelbuttonsmoved(fullheight=fullheight);
      //bottom_panel();
    }
  }
}

twokeyboards();

// sensor
showSensor = true;
if (showSensor) {
  translate([0, 0, 25]) {
    flatSensor();
  }
}