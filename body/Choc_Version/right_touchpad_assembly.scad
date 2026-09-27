use <board_assembled_touchpad.scad>
use <_auxiliary.scad>

// Inspection assembly for controller/reset/sensor access on the right half.
module rightTouchpadAssembly(fullheight = false, showSensor = true) {
  panelbuttonsmoved_touchpad(fullheight=fullheight);

  if (showSensor) {
    translate([0, 0, 27]) {
      linear_extrude(1)
        flatSensor();
    }
  }
}

rightTouchpadAssembly();
