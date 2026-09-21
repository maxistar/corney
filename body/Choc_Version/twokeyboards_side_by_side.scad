use <board_assembled_touchpad.scad>
use <board_assembled.scad>
use <body_thin.scad>
use <_auxiliary.scad>

module twokeyboards(fullheight = false) {
  rotate(-getBoardTiltingAngle()) {
    panelbuttonsmoved_touchpad(fullheight=fullheight);
    // sensor
    showSensor = true;
    if (showSensor) {
      translate([0, 0, 25]) {
        flatSensor();
      }
    }
  }
  scale([1, 1, -1]) {
    rotate(-getBoardTiltingAngle()) {
      panelbuttonsmoved(fullheight=fullheight);
      //bottom_panel();
    }
  }
}

twokeyboards();
